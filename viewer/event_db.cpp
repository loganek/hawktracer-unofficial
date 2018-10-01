#include "event_db.hpp"

#include <algorithm>

namespace HawkTracer
{
namespace viewer
{

bool EventDB::insert(parser::Event event)
{
    auto klass_id = event.get_klass()->get_id();
    if (_events.find(klass_id) == _events.end())
    {
        _events[klass_id] = std::vector<parser::Event>(1, std::move(event));
        return true;
    }
    else
    {
        std::vector<parser::Event>::reverse_iterator it;
        bool ret = true;
        for (it = _events[klass_id].rbegin(); it != _events[klass_id].rend(); ++it)
        {
            if (it->get_timestamp() < event.get_timestamp())
            {
                break;
            }
            ret = false;
        }
        _events[klass_id].insert(it.base(), std::move(event));
        return ret;
    }
}


} // namespace viewer
} // namespace HawkTracer
