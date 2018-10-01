#ifndef HAWKTRACER_VIEWER_EVENT_FILTER_PROXY_MODEL_HPP
#define HAWKTRACER_VIEWER_EVENT_FILTER_PROXY_MODEL_HPP

#include <QtCore/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>

namespace HawkTracer
{
namespace viewer
{

class EventFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    EventFilterProxyModel(QObject* parent = 0);
    ~EventFilterProxyModel() override;

    QStandardItemModel* get_source_model() const { return static_cast<QStandardItemModel*>(sourceModel()); }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_EVENT_FILTER_PROXY_MODEL_HPP
