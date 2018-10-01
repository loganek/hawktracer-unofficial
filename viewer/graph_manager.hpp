#ifndef HAWKTRACER_VIEWER_GRAPH_MANAGER_HPP
#define HAWKTRACER_VIEWER_GRAPH_MANAGER_HPP

#include "graph_dialog.hpp"

#include <hawktracer/base_types.h>
#include <hawktracer/parser/event_klass.hpp>

#include <memory>
#include <unordered_map>
#include <mutex>

namespace HawkTracer
{
namespace viewer
{

class GraphManager
{
public:
    bool add(const std::string& graph_name,
             HT_EventKlassId parent_klass_id,
             std::shared_ptr<const parser::EventKlassField> field,
             const std::vector<parser::Event>& init_data);

    void update_graph(const parser::Event& event);
    void set_graph_data(const std::vector<parser::Event>& data);

private:
    bool _create_new_graph(const std::string& graph_name,
                           HT_EventKlassId parent_klass_id,
                           std::shared_ptr<const parser::EventKlassField> field,
                           const std::vector<parser::Event>& init_data);
    bool _get_points_from_events(const std::vector<parser::Event>& data,
                                 const std::shared_ptr<const parser::EventKlassField>& field,
                                 std::vector<QPointF>& out_points);

    struct GraphInfo
    {
        std::shared_ptr<const parser::EventKlassField> field;
        HT_EventKlassId parent_klass_id;
        std::unique_ptr<GraphDialog> dialog;
    };

    std::unordered_map<std::string, GraphInfo> _graphs;
    std::mutex _graphs_mutex;
};

} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_GRAPH_MANAGER_HPP
