#include "event_filter_proxy_model.hpp"

namespace HawkTracer
{
namespace viewer
{

EventFilterProxyModel::EventFilterProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent)
{
    setSourceModel(new QStandardItemModel());
}

EventFilterProxyModel::~EventFilterProxyModel()
{
    delete sourceModel();
}

QVariant EventFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return tr("Klass ID");
        case 1:
            return tr("Klass name");
        }
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

} // namespace viewer
} // namespace HawkTracer
