#include "chart_view.hpp"

namespace HawkTracer
{
namespace viewer
{

void ChartView::mousePressEvent(QMouseEvent* event)
{
    _init_pos = event->pos();
    _is_dragging = true;
}

void ChartView::mouseMoveEvent(QMouseEvent* event)
{
    if (_is_dragging)
    {
        emit rangeRequest(_init_pos.x() - event->pos().x());
        _init_pos = event->pos();
    }
}

void ChartView::mouseReleaseEvent(QMouseEvent*)
{
    _is_dragging = false;
}

void ChartView::wheelEvent(QWheelEvent* event)
{
    qreal xcenter = event->pos().x() - chart()->plotArea().x();

    if (event->angleDelta().y() > 0)
    {
        zoom(2, xcenter);
    }
    else if (event->angleDelta().y() < 0)
    {
        zoom(0.5, xcenter);
    }
}

void ChartView::zoom(qreal factor, qreal xcenter)
{
    QRectF rect = chart()->plotArea();
    qreal scale = xcenter / rect.width();
    rect.setWidth(rect.width() / factor);

    qreal leftOffset = xcenter - rect.width() * scale;

    rect.moveLeft(rect.x() + leftOffset);
    chart()->zoomIn(rect);

    emit rangeRequest(0);
}

} // namespace viewer
} // namespace HawkTracer
