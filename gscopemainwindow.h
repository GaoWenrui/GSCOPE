#ifndef GSCOPEMAINWINDOW_H
#define GSCOPEMAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QToolButton>
#include <QToolBar>
#include <QThread>
#include <stack>
#include "datalogctrldlg.h"
#include "calibrationdlg.h"
#include "operationcontroldlg.h"
#include "registerinfodlg.h"
#include "serialportdlg.h"
#include "plot/axistag.h"
#include "thread/serialprocessobj.h"
#include "thread/serialdataformat.h"
#include "thread/serialloopthread.h"
#include "thread/threadsafequeue.hpp"
#include "thread/serialwriteobj.h"


namespace Ui {
class GScopeMainWindow;
}

class GScopeMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit GScopeMainWindow(QWidget *parent = 0);
    ~GScopeMainWindow();
protected:
    void closeEvent(QCloseEvent *event);

signals:
    void open_serial_port_msg(QString str);
    void open_port_success(QString str);
    void close_port_success_signal(QString str);
    void serial_changed_signal(serial::Serial* s);
    void configWrite_signal(const protocol::CfgData& d);

    /// 在QObject的子类槽函数中使用死循环，则该子类无法进行消息循环，因此信号与槽机制失效
    void serial_obj_to_stop_signal(bool stop);

private slots:
    void on_actionCalibration_triggered();
    void on_actionOperation_Control_triggered();
    void on_actionRegisters_triggered();
    void on_actionExit_triggered();
    void on_actionDatalog_triggered();
    void on_actionAlarms_triggered();
    void on_action16365_triggered();
    void on_actionUSB_triggered();

    void stk_push_data_slot(const SerialResPkg& respkg);
    void begin_plot_slot();
    void rotateOneStep();

    void gyroTimerSlot();
    void acclTimerSlot();
    void angleTimerSlot();

    void open_port_clicked_slot(const std::string &port,uint32_t baudrate,serial::bytesize_t data_bits,
                          serial::parity_t parity,serial::stopbits_t stop_bits,serial::flowcontrol_t flow_ctrl);
    void close_port_slot();

    void on_pushButton_start_clicked();
    void slpthreadStartSlot();

    void on_pushButton_clicked();

private:
    Ui::GScopeMainWindow *ui;

    CalibrationDlg* calibration_dlg;
    DatalogCtrlDlg* datalogctrl_dlg;
    OperationControlDlg* operationctrl_dlg;
    RegisterInfoDlg* registerinfo_dlg;
    SerialPortDlg* serialport_dlg;

    QToolBar *mainToolBar;
    void initial_main_toolbar();

    // gyro QCustomlot member
    QPointer<QCPGraph> gyroGraph1;
    QPointer<QCPGraph> gyroGraph2;
    QPointer<QCPGraph> gyroGraph3;
    AxisTag *gyroTag1;
    AxisTag *gyroTag2;
    AxisTag *gyroTag3;
    void initial_gyro_plot();

    QPointer<QCPGraph> acclGraph1;
    QPointer<QCPGraph> acclGraph2;
    QPointer<QCPGraph> acclGraph3;
    AxisTag *acclTag1;
    AxisTag *acclTag2;
    AxisTag *acclTag3;
    void initial_accl_plot();

    QPointer<QCPGraph> angleGraph1;
    QPointer<QCPGraph> angleGraph2;
    QPointer<QCPGraph> angleGraph3;
    AxisTag *angleTag1;
    AxisTag *angleTag2;
    AxisTag *angleTag3;
    void initial_angle_plot();

    // serial port operation
    serial::Serial* mSerial;
    SerialLoopThread *slpthread;
    SerialProcessObj *pSerialProcess;
    QThread *thd_serial_process;
    SerialWriteObj *pSerialwrite;
    QThread *thd_serial_write;
    threadsafe_queue<SerialPkg> thd_safe_que;

    // use stack to store the SerialResPkg
    std::stack<SerialResPkg> mStk;
    SerialResPkg mPkg;
    QTimer mPlotTimer;

    void initial_glwidget();
    void initial_menuToolBar();
    void disableWidget();

    // the following ptr have not been used
    QToolBar *menuToolBar;

    QAction *act_USB ;
    QAction *act_16365 ;
    QAction *act_calibration ;
    QAction *act_datalog ;
    QAction *act_operation_ctrl ;
    QAction *act_alarms ;
    QAction *act_registers ;
    QAction *act_exit ;

    QToolButton *tbtn_interface;
    QMenu *menu_interface;
    QToolButton *tbtn_device;
    QMenu *menu_device;
    QToolButton *tbtn_configuration;
    QMenu *menu_configuration;
    QToolButton *tbtn_datalog;
    QMenu *menu_datalog;
    QToolButton *tbtn_registers;
    QMenu *menu_registers;
    QToolButton *tbtn_exit;
    QMenu *menu_exit;

    QAction *act_menu_USB ;
    QAction *act_menu_16365 ;
    QAction *act_menu_calibration ;
    QAction *act_menu_datalog ;
    QAction *act_menu_operation_ctrl;
    QAction *act_menu_alarms ;
    QAction *act_menu_registers ;
    QAction *act_menu_exit ;
};

#endif // GSCOPEMAINWINDOW_H
