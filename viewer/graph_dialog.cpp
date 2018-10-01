#include "graph_dialog.hpp"
#include "ui_graph_dialog.h"
#include "chart_view.hpp"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtCharts/QValueAxis>

namespace HawkTracer
{
namespace viewer
{

QT_CHARTS_USE_NAMESPACE


qreal GraphDialog::_get_axis_min() const
{
    return static_cast<QValueAxis*>(_chart->axisX())->min();
}

qreal GraphDialog::_get_axis_max() const
{
    return static_cast<QValueAxis*>(_chart->axisX())->max();
}

GraphDialog::GraphDialog(const std::vector<QPointF>& data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphDialog)
{
    ui->setupUi(this);

    _setup_chart();
    set_data(data);

    ui->unitsComboBox->addItems({"s", "ms", "us", "ns"});

    connect(ui->fullRangeRadioButton, &QRadioButton::toggled, this, &GraphDialog::_toggle_range_options);
    connect(ui->lastDurationRadioButton, &QRadioButton::toggled, this, &GraphDialog::_toggle_range_options);
    connect(ui->fixedRangeRadioButton, &QRadioButton::toggled, this, &GraphDialog::_toggle_range_options);
    connect(ui->fromLineEdit, &QLineEdit::editingFinished, this, &GraphDialog::_set_range_from_fixed_range);
    connect(ui->toLineEdit, &QLineEdit::editingFinished, this, &GraphDialog::_set_range_from_fixed_range);

    on_unitsComboBox_currentIndexChanged(0); // TODO separate function
}

void GraphDialog::_setup_chart()
{
    _chart = new QChart();
    _chart->legend()->hide();

    auto chart_view = new ChartView(_chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    ui->frame->layout()->addWidget(chart_view);
    connect(chart_view, &ChartView::rangeRequest, this, &GraphDialog::_on_range_changed);
}

GraphDialog::~GraphDialog()
{
    delete ui;
}

void GraphDialog::_on_range_changed(int offset)
{
    ui->fixedRangeRadioButton->setChecked(true);

    auto change = (_get_axis_max() - _get_axis_min()) / _chart->plotArea().width() * offset;
    _set_range(_get_axis_min() + change, _get_axis_max() + change);
}

void GraphDialog::_set_range(qreal min, qreal max)
{
    ui->fromLineEdit->setText(QString::number(min / _multiplier, 'f'));
    ui->toLineEdit->setText(QString::number(max / _multiplier, 'f'));
    _chart->axisX()->setRange(min, max);

    if (_limit_y.first == _limit_y.second)
    {
        _chart->axisY()->setRange(_limit_y.first - 5, _limit_y.second + 5);//TODO
    }
    else
    {
        _chart->axisY()->setRange(_limit_y.first, _limit_y.second);//TODO
    }
}

static void update_limit(std::pair<qreal, qreal>& limit, qreal value)
{
    limit.first = std::min(limit.first, value);
    limit.second = std::max(limit.second, value);
}

void GraphDialog::set_data(const std::vector<QPointF>& data)
{
    if (_series)
    {
        _chart->removeSeries(_series);
        delete _series;
    }
    _series = new QLineSeries();

    _limit_x = _limit_y = std::make_pair(std::numeric_limits<qreal>::max(), std::numeric_limits<qreal>::lowest());

    for (const auto& p : data)
    {
        update_limit(_limit_x, p.x());
        update_limit(_limit_y, p.y());
        _series->append(p.x(), p.y());
    }

    _chart->addSeries(_series);
    _chart->createDefaultAxes();
}

void GraphDialog::append_data(QPointF point)
{
    update_limit(_limit_x, point.x());
    update_limit(_limit_y, point.y());

    if (_range_mode == RangeMode::FULL)
    {
        _set_range(_limit_x.first, _limit_x.second);
    }
    else if (_range_mode == RangeMode::LAST_DURATION)
    {
        bool ok = false;
        double value = ui->lastDurationLineEdit->text().toDouble(&ok);
        if (ok)
        {
            _set_range(_limit_x.second - value * _multiplier, _limit_x.second);
        }
    }

    _series->append(point.x(), point.y());
}

void GraphDialog::on_unitsComboBox_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        _multiplier = 1000 * 1000 * 1000; // seconds
        break;
    case 1:
        _multiplier = 1000 * 1000; // milliseconds
        break;
    case 2:
        _multiplier = 1000; // microseconds
        break;
    case 3:
        _multiplier = 1; // nanoseconds
        break;
    default:
        break;
    }

    _set_range(_get_axis_min(), _get_axis_max());
    bool ok;
    qreal val = ui->lastDurationLineEdit->text().toDouble(&ok);
    if (ok)
    {
        ui->lastDurationLineEdit->setText(QString::number(val));
    }
}

void GraphDialog::_toggle_range_options(bool toggled)
{
    if (!toggled)
    {
        return;
    }

    auto set_fixed_range_disabled = [this] (bool disabled){
        ui->fromLineEdit->setDisabled(disabled);
        ui->toLineEdit->setDisabled(disabled);
        ui->setMaxPushButton->setDisabled(disabled);
        ui->setMinPushButton->setDisabled(disabled);
    };

    auto disable_all = [this, &set_fixed_range_disabled] {
        set_fixed_range_disabled(true);
        ui->lastDurationLineEdit->setDisabled(true);
    };
    disable_all();

    if (ui->fixedRangeRadioButton->isChecked())
    {
        _range_mode = RangeMode::FIXED;
        set_fixed_range_disabled(false);
    }
    else if (ui->lastDurationRadioButton->isChecked())
    {
        _range_mode = RangeMode::LAST_DURATION;
        ui->lastDurationLineEdit->setDisabled(false);
    }
    if (ui->fullRangeRadioButton->isChecked())
    {
        _range_mode = RangeMode::FULL;
        _set_range(_limit_x.first, _limit_x.second);
    }
}

void GraphDialog::on_setMinPushButton_clicked()
{
    _set_range(_limit_x.first, _get_axis_max());
}

void GraphDialog::on_setMaxPushButton_clicked()
{
    _set_range(_get_axis_min(), _limit_x.second);
}

void GraphDialog::_set_range_from_fixed_range()
{
    bool ok1 = false;
    bool ok2 = false;
    qreal from_val = ui->fromLineEdit->text().toDouble(&ok1);
    qreal to_val = ui->toLineEdit->text().toDouble(&ok2);

    if (ok1 && ok2)
    {
        _set_range(from_val * _multiplier, to_val * _multiplier);
    }
}

void GraphDialog::on_lastDurationLineEdit_editingFinished()
{
    bool ok;
    qreal val = ui->lastDurationLineEdit->text().toDouble(&ok);

    if (ok)
    {
        _set_range(_limit_x.second - val * _multiplier, _limit_x.second);
    }
}

} // namespace viewer
} // namespace HawkTracer

