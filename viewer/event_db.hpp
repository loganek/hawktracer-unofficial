#ifndef HAWKTRACER_VIEWER_EVENT_DB_HPP
#define HAWKTRACER_VIEWER_EVENT_DB_HPP

#include <hawktracer/parser/event.hpp>

namespace HawkTracer
{
namespace viewer
{

class EventDB
{
public:
    bool insert(parser::Event event);

    size_t get_event_count(HT_EventKlassId klass_id) const
    {
        auto it = _events.find(klass_id);
        return it != _events.end() ? it->second.size() : 0;
    }

    std::vector<parser::Event> get_data(HT_EventKlassId klass_id) const
    {
        auto it = _events.find(klass_id);
        return (it != _events.end()) ? it->second : std::vector<parser::Event>{};
    }

private:
    std::unordered_map<HT_EventKlassId, std::vector<parser::Event>> _events;
};
} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_EVENT_DB_HPP
