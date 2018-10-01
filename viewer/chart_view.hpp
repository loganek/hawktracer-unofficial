#ifndef HAWKTRACER_VIEWER_CHART_VIEW_HPP
#define HAWKTRACER_VIEWER_CHART_VIEW_HPP

#include <QtCharts/QChartView>

namespace HawkTracer
{
namespace viewer
{

class ChartView : public QT_CHARTS_NAMESPACE::QChartView
{
    Q_OBJECT

public:
    ChartView(QT_CHARTS_NAMESPACE::QChart* chart, QWidget* parent = nullptr) :
        QChartView(chart, parent)
    {
    }

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void zoom(qreal factor, qreal xcenter);

signals:
    void rangeRequest(int offset);

private:
    QPoint _init_pos;
    bool _is_dragging = false;
};

} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_CHART_VIEW_HPP
