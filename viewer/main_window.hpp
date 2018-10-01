#ifndef HAWKTRACER_VIEWER_MAIN_WINDOW_HPP
#define HAWKTRACER_VIEWER_MAIN_WINDOW_HPP

#include "icontroller.hpp"

#include <hawktracer/parser/event_klass.hpp>

#include <QMainWindow>
#include <QtCore/QAbstractItemModel>

namespace Ui
{
class MainWindow;
}

namespace HawkTracer
{
namespace viewer
{

enum class ErrorType
{
    OK,
    CONNECTION_ERROR,
    KLASS_NOT_FOUND,
    FIELD_NOT_FOUND,
    KLASS_NOT_SELECTED,
    NOT_NUMERIC_TYPE
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(IController* controller, QWidget *parent = 0);
    ~MainWindow();

    void set_treeview_model(QAbstractItemModel* model);
    void show_error(ErrorType type);
    void show_klass_info(const parser::EventKlass& klass);

    void set_connection_state(ConnectionStatus status);
    void set_timestamp_range(uint64_t min_ts, uint64_t max_ts);
    void update_klass_event_count(size_t count);

private slots:
    void klass_selected(const QModelIndex& index);

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

private:
    void _connect_clicked();
    void _stream_type_changed(int index);
    void _select_file_stream();

    static QString _type_to_string(parser::FieldTypeId type_id);
    void _add_field_info(const std::shared_ptr<parser::EventKlassField>& field);

    IController* _controller;

    Ui::MainWindow *ui;
};

} // namespace viewer
} // namespace HawkTracer

#endif // HAWKTRACER_VIEWER_MAIN_WINDOW_HPP
