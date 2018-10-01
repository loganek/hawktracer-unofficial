#include "controller.hpp"
#include <hawktracer/parser/make_unique.hpp>

#include <hawktracer/client_utils/stream_factory.hpp>
#include <hawktracer/parser/event.hpp>
#include <hawktracer/mkcreflect.h>

#include <unistd.h>
#include <limits>

namespace HawkTracer
{
namespace viewer
{

#define INVOKE_UI(COMMAND) _invoke_ui([this] { COMMAND;})
#define INVOKE_UI_EX(COMMAND, ...) _invoke_ui([this, __VA_ARGS__] { COMMAND;})

QList<QStandardItem*> create_model_item(const std::string& name, HT_TimestampNs timestamp)
{
    QList<QStandardItem *> rowItems;

    rowItems << new QStandardItem(QString::fromStdString(name));
    rowItems << new QStandardItem(QString::fromStdString(std::to_string(timestamp)));

    return rowItems;
}


Controller::Controller() :
    _window(this)
{
    _max_ts = 0;
    _min_ts = std::numeric_limits<uint64_t>::max();

    _window.set_treeview_model(&_klasses_model);
}

ConnectionStatus Controller::get_connection_status() const
{
    if (!_connection_thread.valid())
    {
        return ConnectionStatus::NOT_CONNECTED;
    }

    if (_connection_thread.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        return _reader ? ConnectionStatus::CONNECTED : ConnectionStatus::NOT_CONNECTED;
    }

    return ConnectionStatus::PENDING;
}

void Controller::connect(const std::string& connection_string)
{
    auto stream = ClientUtils::make_stream_from_string(connection_string);
    if (!stream)
    {
        INVOKE_UI(_window.show_error(ErrorType::CONNECTION_ERROR));
        INVOKE_UI(_window.set_connection_state(ConnectionStatus::NOT_CONNECTED));
        return;
    }
    _reader = parser::make_unique<parser::ProtocolReader>(&_register, std::move(stream), true);
    _reader->register_events_listener([this](const parser::Event& event) { _process_event(event); });
    INVOKE_UI(_window.set_connection_state(ConnectionStatus::PENDING));

    INVOKE_UI(_klasses_model.clear());

    _connection_thread = std::async(std::launch::async, [this] {
        auto connection_status = _reader->start() ? ConnectionStatus::CONNECTED : ConnectionStatus::NOT_CONNECTED;
        _connection_cv.notify_one();
        INVOKE_UI_EX(_window.set_connection_state(connection_status), connection_status);
    });
}

void Controller::disconnect()
{
    INVOKE_UI(_window.set_connection_state(ConnectionStatus::NOT_CONNECTED));
    _reader->stop();
    _reader = nullptr;
}

void Controller::run()
{
    _window.show();
}

void Controller::_process_event(const parser::Event& event)
{
    std::unique_lock<std::mutex> l(_db_mtx);
    bool append_to_end = _db.insert(event);
    l.unlock();

    if (event.get_klass()->get_id() == _selected_klass_id)
    {
        l.lock();
        size_t count = _db.get_event_count(_selected_klass_id);
        l.unlock();
        INVOKE_UI_EX(_window.update_klass_event_count(count), count);
    }

    bool update_stats = false;

    if (event.get_timestamp() > _max_ts)
    {
        _max_ts = event.get_timestamp();
        update_stats = true;
    }
    if (event.get_timestamp() < _min_ts && event.get_timestamp() > 0)
    {
        _min_ts = event.get_timestamp();
        update_stats = true;
    }

    if (event.get_klass()->get_id() == (HT_EventKlassId)parser::WellKnownKlasses::EventKlassInfoEventKlass)
    {
        QList<QStandardItem *> rowItems;

        HT_EventKlassId klass_id = event.get_value<HT_EventKlassId>("info_klass_id");
        auto item = new QStandardItem(QString::number(klass_id));
        item->setData(QVariant(klass_id));
        rowItems << item;
        rowItems << new QStandardItem(QString::fromStdString(event.get_value<char*>("event_klass_name")));
        INVOKE_UI_EX(_klasses_model.invisibleRootItem()->appendRow(rowItems), rowItems);
    }

    if (update_stats)
    {
        INVOKE_UI(_window.set_timestamp_range(_min_ts, _max_ts));
    }

    if (append_to_end)
    {
        _graph_manager.update_graph(event);
    }
    else
    {
        l.lock();
        auto event_data = _db.get_data(event.get_klass()->get_id());
        l.unlock();
        _graph_manager.set_graph_data(event_data);
    }
}

void Controller::select_klass(HT_EventKlassId klass_id)
{
    auto klass = _register.get_klass(klass_id);
    if (klass)
    {
        _selected_klass_id = klass_id;
        INVOKE_UI_EX(_window.show_klass_info(*klass), klass);
        std::unique_lock<std::mutex> l(_db_mtx);
        size_t count = _db.get_event_count(_selected_klass_id);
        l.unlock();
        INVOKE_UI_EX(_window.update_klass_event_count(count), count);
    }
    else
    {
        INVOKE_UI(_window.show_error(ErrorType::KLASS_NOT_FOUND));
    }
}

void Controller::add_graph(const std::string& field_name)
{
    if (_selected_klass_id == _invalid_klass_id)
    {
        INVOKE_UI(_window.show_error(ErrorType::KLASS_NOT_SELECTED));
        return;
    }

    auto klass = _register.get_klass(_selected_klass_id);
    if (!klass)
    {
        INVOKE_UI(_window.show_error(ErrorType::KLASS_NOT_FOUND));
        return;
    }

    auto field = klass->get_field(field_name.c_str(), true);
    if (!field)
    {
        INVOKE_UI(_window.show_error(ErrorType::FIELD_NOT_FOUND));
        return;
    }

    if (!field->is_numeric())
    {
        INVOKE_UI(_window.show_error(ErrorType::NOT_NUMERIC_TYPE));
        return;
    }

    std::unique_lock<std::mutex> l(_db_mtx);
    auto event_data = _db.get_data(_selected_klass_id);
    l.unlock();
    auto ok = _graph_manager.add(klass->get_name() + "::" + field_name, _selected_klass_id, std::move(field), event_data);

    if (!ok)
    {
        INVOKE_UI(_window.show_error(ErrorType::NOT_NUMERIC_TYPE));
    }
}

} // namespace viewer
} // namespace HawkTracer
