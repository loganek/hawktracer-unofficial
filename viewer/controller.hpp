#ifndef HAWKTRACER_VIEWER_CONTROLLER_HPP
#define HAWKTRACER_VIEWER_CONTROLLER_HPP

#include "icontroller.hpp"
#include "main_window.hpp"
#include "event_db.hpp"
#include "graph_manager.hpp"

#include <QtGui/QStandardItemModel>

#include <hawktracer/parser/klass_register.hpp>
#include <hawktracer/parser/stream.hpp>
#include <hawktracer/parser/protocol_reader.hpp>
#include <hawktracer/parser/event.hpp>

#include <condition_variable>
#include <future>

namespace HawkTracer
{
namespace viewer
{

class Controller : public IController
{
public:
    Controller();

    void connect(const std::string& connection_string) override;
    void disconnect() override;

    ConnectionStatus get_connection_status() const override;

    void add_graph(const std::string& field_name) override;
    void select_klass(HT_EventKlassId klass_id) override;

    void run();

private:
    void _process_event(const parser::Event& event);

    template<typename Func>
    void _invoke_ui(Func fnc)
    {
        QMetaObject::invokeMethod(&_window, std::move(fnc));
    }

    MainWindow _window;
    parser::KlassRegister _register;
    std::unique_ptr<parser::ProtocolReader> _reader;
    uint64_t _min_ts;
    uint64_t _max_ts;
    GraphManager _graph_manager;

    std::mutex _db_mtx;
    EventDB _db;

    std::condition_variable _connection_cv;
    std::future<void> _connection_thread;

    static const HT_EventKlassId _invalid_klass_id = static_cast<HT_EventKlassId>(-1);
    HT_EventKlassId _selected_klass_id = _invalid_klass_id;
    QStandardItemModel _klasses_model;
};

} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_CONTROLLER_HPP
