#include "main_window.hpp"
#include "ui_main_window.h"

#include <hawktracer/parser/event_klass.hpp>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDebug>

namespace HawkTracer
{
namespace viewer
{

MainWindow::MainWindow(IController* controller, QWidget* parent) :
    QMainWindow(parent),
    _controller(controller),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->streamTypeComboBox->addItem(tr("File"));
    ui->streamTypeComboBox->addItem(tr("TCP Stream"));

    _stream_type_changed(0);

    connect(ui->streamTypeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::_stream_type_changed);

    ui->treeWidget->headerItem()->setText(0, tr("Field"));
    ui->treeWidget->headerItem()->setText(1, tr("Type"));
    ui->treeWidget->headerItem()->setText(2, tr("Type name"));

    connect(ui->connect_PushButton, &QPushButton::clicked, this, &MainWindow::_connect_clicked);
    connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(klass_selected(const QModelIndex&)));
    connect(ui->filePathSelectPushButton, &QPushButton::clicked, this, &MainWindow::_select_file_stream);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_treeview_model(QAbstractItemModel* model)
{
    ui->treeView->setModel(model);
}

void MainWindow::set_connection_state(ConnectionStatus status)
{
    switch (status)
    {
    case ConnectionStatus::CONNECTED:
        ui->treeWidget->clear();
        ui->connect_PushButton->setText(tr("Disconnect"));
        break;
    case ConnectionStatus::NOT_CONNECTED:
        ui->connect_PushButton->setText(tr("Connect"));
        break;
    case ConnectionStatus::PENDING:
        ui->connect_PushButton->setText(tr("Connecting... (cancel)"));
        break;
    }
}

void MainWindow::set_timestamp_range(uint64_t min_ts, uint64_t max_ts)
{
    double ts = max_ts - min_ts;
    QString units[] = {"ns", "us", "ms", "s"};

    size_t i = 0;
    while (ts >= 1000 && i < sizeof(units)/sizeof(units[0]) - 1)
    {
        ts /= 1000;
        i++;
    }

    ui->statusBar->showMessage(tr("Timestamp range: <0, %1> [%2]").arg(ts, 0, 'f', 2).arg(units[i]));
}

void MainWindow::update_klass_event_count(size_t count)
{
    ui->numberEventsLabel->setText(QString::number(count));
}

void MainWindow::show_error(ErrorType type)
{
    QString error_message;
    switch (type)
    {
    case ErrorType::CONNECTION_ERROR:
        error_message = tr("Connection failed");
        break;
    case ErrorType::NOT_NUMERIC_TYPE:
        error_message = tr("Expected numeric type!");
        break;
    default:
        error_message = tr("Unknown error");
        break;
    }

    QMessageBox::critical(this, tr("HawkTracer error"), error_message, QMessageBox::Ok);
}

void MainWindow::_connect_clicked()
{
    switch (_controller->get_connection_status())
    {
    case ConnectionStatus::CONNECTED:
    case ConnectionStatus::PENDING:
        _controller->disconnect();
        break;
    case ConnectionStatus::NOT_CONNECTED:
    {
        std::string connection_string;
        switch (ui->streamTypeComboBox->currentIndex())
        {
        case 0:
            connection_string = ui->filePathLineEdit->text().toStdString();
            break;
        default:
            connection_string = QString("%1:%2").arg(ui->tcpIpLineEdit->text()).arg(ui->portSpinBox->value()).toStdString();
            break;
        }
        _controller->connect(connection_string);
        break;
    }
    }
}

QString MainWindow::_type_to_string(parser::FieldTypeId type_id)
{
    switch (type_id)
    {
#define TYPE_TO_STRING(UNDERLYING_TYPE, USER_DATA) case parser::FieldTypeId::UNDERLYING_TYPE: return #UNDERLYING_TYPE;
    MKCREFLECT_FOREACH(TYPE_TO_STRING, 0, UINT8, INT8, UINT16, INT16, UINT32, INT32, UINT64, INT64, POINTER, STRING, STRUCT)
#undef TYPE_TO_STRING
    default:
        return tr("UNKNOWN");
    }
}

void MainWindow::_add_field_info(const std::shared_ptr<parser::EventKlassField>& field)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);

    treeItem->setText(0, QString::fromStdString(field->get_name()));
    QString underlying_type_str;

    treeItem->setText(1, _type_to_string(field->get_type_id()));
    treeItem->setText(2, QString::fromStdString(field->get_type_name()));
}

void MainWindow::_stream_type_changed(int index)
{
    switch (index)
    {
    case 0:
        ui->fileStreamFrame->show();
        ui->tcpIpStreamFrame->hide();
        break;
    default:
        ui->fileStreamFrame->hide();
        ui->tcpIpStreamFrame->show();
        break;
    }
}

void MainWindow::klass_selected(const QModelIndex& index)
{
    auto model = index.model();
    bool parse_ok = true;

    HT_EventKlassId klass_id = model->data(model->index(index.row(), 0)).toULongLong(&parse_ok);

    if (parse_ok)
    {
        _controller->select_klass(klass_id);
    }
    else
    {
        // TODO
    }
}

void MainWindow::show_klass_info(const parser::EventKlass& klass)
{
    ui->treeWidget->clear();

    std::function<void(const parser::EventKlass& klass)> show_fnc =
            [this, &show_fnc] (const parser::EventKlass& klass) {
        for (const auto& field : klass.get_fields())
        {
            if (field->get_klass())
            {
                show_fnc(*field->get_klass());
            }
            else
            {
                _add_field_info(field);
            }
        }
    };

    show_fnc(klass);
}

void MainWindow::_select_file_stream()
{
    ui->filePathLineEdit->setText(
                QFileDialog::getOpenFileName(
                    this, tr("Open HawkTracer dump file"), QString(),
                    tr("HawkTracer dump files (*.htdump);;All files (*)")));
}

void MainWindow::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    auto item = ui->treeWidget->itemAt(pos);
    if (!item)
    {
        return;
    }

    QMenu menu(this);
    menu.addAction(tr("Show on a graph"), [this, item] {
        _controller->add_graph(item->text(0).toStdString());
    });

    menu.exec(ui->treeWidget->mapToGlobal(pos));
}

} // namespace viewer
} // namespace HawkTracer
