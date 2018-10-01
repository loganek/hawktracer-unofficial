#ifndef HAWKTRACER_VIEWER_GRAPH_DIALOG_HPP
#define HAWKTRACER_VIEWER_GRAPH_DIALOG_HPP

#include <hawktracer/base_types.h>

#include <QtCore/QPoint>
#include <QtWidgets/QDialog>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

namespace Ui {
class GraphDialog;
}

namespace HawkTracer
{
namespace viewer
{

enum class RangeMode
{
    FIXED,
    FULL,
    LAST_DURATION
};

class GraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GraphDialog(const std::vector<QPointF>& data, QWidget *parent = 0);
    ~GraphDialog();

    void append_data(QPointF point);
    void set_data(const std::vector<QPointF>& data);

private slots:
    void on_unitsComboBox_currentIndexChanged(int index);

    void on_setMinPushButton_clicked();
    void on_setMaxPushButton_clicked();


    void on_lastDurationLineEdit_editingFinished();

private:
    void _setup_chart();

    void _on_range_changed(int offset);
    void _set_range(qreal min, qreal max);
    void _set_range_from_fixed_range();

    void _toggle_range_options(bool toggled);

    qreal _get_axis_min() const;
    qreal _get_axis_max() const;

    QT_CHARTS_NAMESPACE::QLineSeries* _series = nullptr;
    QT_CHARTS_NAMESPACE::QChart* _chart;
    Ui::GraphDialog *ui;

    std::pair<qreal, qreal> _limit_x;
    std::pair<qreal, qreal> _limit_y;
    qreal _multiplier = 1.0;

    RangeMode _range_mode = RangeMode::FULL;
};

} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_GRAPH_DIALOG_HPP
