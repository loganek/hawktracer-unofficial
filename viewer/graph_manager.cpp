#include "graph_manager.hpp"

#include <hawktracer/parser/make_unique.hpp>
#include <hawktracer/parser/event.hpp>

#include <QtCore/QPointF>

#define NUMERIC_TYPE_LIST (UINT8, uint8_t), (UINT16, uint16_t), (UINT32, uint32_t), (UINT64, uint64_t)
#define FOREACH_TYPE_(TYPES, FNC_NAME) FNC_NAME TYPES

namespace HawkTracer
{
namespace viewer
{

template<typename T>
std::vector<QPointF> _create_data_set(
        const std::vector<parser::Event>& events,
        const std::shared_ptr<const parser::EventKlassField>& field)
{
    std::vector<QPointF> out;
    if (events.empty())
    {
        return out;
    }

    for (const auto& event : events)
    {
        out.push_back(QPointF(event.get_timestamp(), event.get_value<T>(field->get_name())));
    }

    return out;
}

bool GraphManager::_get_points_from_events(const std::vector<parser::Event>& data,
                                           const std::shared_ptr<const parser::EventKlassField>& field,
                                           std::vector<QPointF>& out_points)
{
    out_points.clear();

    switch(field->get_type_id())
    {
#define CREATE_DATASET(TYPE, C_TYPE) case parser::FieldTypeId::TYPE: out_points = _create_data_set<C_TYPE>(data, field); return true;
    MKCREFLECT_FOREACH(FOREACH_TYPE_, CREATE_DATASET, NUMERIC_TYPE_LIST)
            default:
        return false;
    }
}

bool GraphManager::_create_new_graph(const std::string& graph_name,
                                     HT_EventKlassId parent_klass_id,
                                     std::shared_ptr<const parser::EventKlassField> field,
                                     const std::vector<parser::Event>& init_data)
{
    GraphInfo info;
    std::vector<QPointF> points;

    if (!_get_points_from_events(init_data, field, points))
    {
        return false;
    }

    info.field = std::move(field);
    info.parent_klass_id = parent_klass_id;
    info.dialog = parser::make_unique<GraphDialog>(points, nullptr);
    info.dialog->setWindowTitle(QString::fromStdString(graph_name));
    info.dialog->show();

    _graphs[graph_name] = std::move(info);
    return true;
}

bool GraphManager::add(const std::string& graph_name,
                       HT_EventKlassId parent_klass_id,
                       std::shared_ptr<const parser::EventKlassField> field,
                       const std::vector<parser::Event>& init_data)
{
    std::lock_guard<std::mutex> l(_graphs_mutex);
    auto it = _graphs.find(graph_name);
    if (it == _graphs.end())
    {
        return _create_new_graph(graph_name, parent_klass_id, std::move(field), init_data);
    }

    it->second.dialog->show();
    it->second.dialog->activateWindow();
    return true;
}

qreal get_event_value(const parser::Event& event, const parser::EventKlassField& field)
{
    switch(field.get_type_id())
    {
#define GET_VALUE(TYPE, C_TYPE) \
    case parser::FieldTypeId::TYPE: return (qreal)event.get_value<C_TYPE>(field.get_name());
    MKCREFLECT_FOREACH(FOREACH_TYPE_, GET_VALUE, NUMERIC_TYPE_LIST)
            default:
        return 0;
    }
}

void GraphManager::set_graph_data(const std::vector<parser::Event>& data)
{
    if (data.empty())
    {
        return;
    }

    auto klass_id = data.front().get_klass()->get_id();

    std::lock_guard<std::mutex> l(_graphs_mutex);
    for (auto& graph : _graphs)
    {
        if (graph.second.parent_klass_id == klass_id)
        {
            std::vector<QPointF> points;
            _get_points_from_events(data, graph.second.field, points);
            auto dlg = graph.second.dialog.get();
            QMetaObject::invokeMethod(dlg, [points, dlg] { dlg->set_data(points); });
        }
    }
}

void GraphManager::update_graph(const parser::Event& event)
{
    std::lock_guard<std::mutex> l(_graphs_mutex);
    for (auto& graph : _graphs)
    {
        if (graph.second.parent_klass_id == event.get_klass()->get_id())
        {
            QPointF pt(event.get_timestamp(), get_event_value(event, *graph.second.field));
            auto dlg = graph.second.dialog.get();
            QMetaObject::invokeMethod(dlg, [pt, dlg] { dlg->append_data(pt); });
        }
    }
}


} // namespace viewer
} // namespace HawkTracer
