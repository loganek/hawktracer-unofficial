#ifndef HAWKTRACER_VIEWER_ICONTROLLER_HPP
#define HAWKTRACER_VIEWER_ICONTROLLER_HPP

#include <hawktracer/base_types.h>

#include <string>

namespace HawkTracer
{
namespace viewer
{

enum class ConnectionStatus
{
    NOT_CONNECTED,
    PENDING,
    CONNECTED
};

class IController
{
public:
    virtual void connect(const std::string& connection_string) = 0;
    virtual void disconnect() = 0;
    virtual ConnectionStatus get_connection_status() const = 0;

    virtual void select_klass(HT_EventKlassId klass_id) = 0;

    virtual void add_graph(const std::string& field_name) = 0;

    virtual ~IController() {}
};

} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_ICONTROLLER_HPP
