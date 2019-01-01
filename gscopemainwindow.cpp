#include <QDebug>
#include <QColor>
#include <QOpenGLWidget>
#include <QTimer>
#include <iostream>
#include "gscopemainwindow.h"
#include "ui_gscopemainwindow.h"

GScopeMainWindow::GScopeMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GScopeMainWindow)
{
    ui->setupUi(this);
    mainToolBar = new QToolBar(this);
    this->addToolBar(Qt::TopToolBarArea,mainToolBar);

    // serial port and thread member initialization
    mSerial = NULL ;
    pSerialProcess = NULL;
    slpthread = NULL;
    thd_serial_process = NULL;
    pSerialwrite = NULL;
    thd_serial_write = NULL;

    /// worker objects to implement mutithread
    thd_serial_write = new QThread();
    pSerialwrite = new SerialWriteObj();
    pSerialwrite->moveToThread(thd_serial_write);
    connect(thd_serial_write,&QThread::finished,pSerialwrite,&QObject::deleteLater);
    connect(this,&GScopeMainWindow::serial_changed_signal,pSerialwrite,&SerialWriteObj::serialChanged_slot);
    thd_serial_write->start();

    calibration_dlg = NULL;
    datalogctrl_dlg = NULL;
    operationctrl_dlg = NULL;
    registerinfo_dlg = NULL;
    serialport_dlg = NULL;

    initial_main_toolbar();

    initial_glwidget();
    initial_gyro_plot();
    initial_accl_plot();
    initial_angle_plot();

//    memset(&mPkg,0,sizeof(mPkg));
//    acclTimerSlot();
//    gyroTimerSlot();
//    angleTimerSlot();

    connect(&mPlotTimer,&QTimer::timeout,this,&GScopeMainWindow::begin_plot_slot);
    connect(this,&GScopeMainWindow::configWrite_signal,pSerialwrite,&SerialWriteObj::configWrite_slot);

    disableWidget();
    //setAttribute(Qt::WA_DeleteOnClose,true);
    setWindowTitle("GSCOPE");
    resize(700,500);
}

GScopeMainWindow::~GScopeMainWindow()
{
    close_port_slot();
    delete ui;
}

void GScopeMainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button=QMessageBox::question(this,"Exit","Confirm to exit?",
                                  QMessageBox::Ok|QMessageBox::Cancel);
    if(button==QMessageBox::Ok){
        event->accept();
    }else{
        event->ignore();
    }
}

void GScopeMainWindow::on_actionCalibration_triggered()
{
    qDebug()<<"Calibration";

    /*
    // for safe
    if(mSerial==NULL){
        QMessageBox::information(this,"Open port",
                                 "The serial port is not opened. Please open port first.");
        return;
    }
    */

    // Singleton
    if(calibration_dlg!=NULL){
        calibration_dlg->show();
        return;
    }
    calibration_dlg=new CalibrationDlg(this);
    connect(calibration_dlg,SIGNAL(configWrite_signal(protocol::CfgData)),
            pSerialwrite,SLOT(configWrite_slot(protocol::CfgData)));
    connect(pSerialwrite,&SerialWriteObj::configRes_singal,
            calibration_dlg,&CalibrationDlg::configRes_slot);
    calibration_dlg->show();
}

void GScopeMainWindow::on_actionOperation_Control_triggered()
{
    qDebug()<<"Operation Control";
    /*
    // for safe
    if(mSerial==NULL){
        QMessageBox::information(this,"Open port",
                                 "The serial port is not opened. Please open port first.");
        return;
    }
    */

    // Singleton
    if(operationctrl_dlg!=NULL){
        operationctrl_dlg->show();
        return;
    }
    operationctrl_dlg=new OperationControlDlg(this);
    connect(operationctrl_dlg,&OperationControlDlg::configWrite_signal,
            pSerialwrite,&SerialWriteObj::configWrite_slot);
    connect(pSerialwrite,&SerialWriteObj::configRes_singal,
            operationctrl_dlg,&OperationControlDlg::configRes_slot);
    operationctrl_dlg->show();
}

void GScopeMainWindow::on_actionRegisters_triggered()
{
    qDebug()<<"Registers";

    /*
    // for safe
    if(mSerial==NULL){
        QMessageBox::information(this,"Open port",
                                 "The serial port is not opened. Please open port first.");
        return;
    }
    */

    // Singleton
    if(registerinfo_dlg!=NULL){
        registerinfo_dlg->show();
        return;
    }
    registerinfo_dlg=new RegisterInfoDlg(this);
    connect(registerinfo_dlg,&RegisterInfoDlg::configWrite_signal,
            pSerialwrite,&SerialWriteObj::configWrite_slot);
    connect(pSerialwrite,&SerialWriteObj::configRes_singal,
            registerinfo_dlg,&RegisterInfoDlg::configRes_slot);
    registerinfo_dlg->show();
}

void GScopeMainWindow::on_actionExit_triggered()
{
    qDebug()<<"Exit";
    this->close();
}

void GScopeMainWindow::on_actionDatalog_triggered()
{
    qDebug()<<"Datalog";

    /*
    // for safe
    if(mSerial==NULL){
        QMessageBox::information(this,"Open port",
                                 "The serial port is not opened. Please open port first.");
        return;
    }
    */

    // Singleton
    if(datalogctrl_dlg!=NULL){
        datalogctrl_dlg->show();
        return;
    }
    datalogctrl_dlg=new DatalogCtrlDlg(this);
    connect(datalogctrl_dlg,&DatalogCtrlDlg::queryData_signal,
            pSerialwrite,&SerialWriteObj::configWrite_slot);
    connect(pSerialwrite,&SerialWriteObj::configRes_singal,
            datalogctrl_dlg,&DatalogCtrlDlg::resolveData_slot);
    datalogctrl_dlg->show();
}

void GScopeMainWindow::on_actionAlarms_triggered()
{
    qDebug()<<"Alarms";
    QMessageBox::information(this,"Alarm","The Alarm function is not supported for now.");
}

void GScopeMainWindow::on_action16365_triggered()
{
    qDebug()<<"16365";
    QMessageBox::information(this,"Sensor Select","The software only support Adis16365 for now.");
}

void GScopeMainWindow::on_actionUSB_triggered()
{
    qDebug()<<"USB";

    // make sure can only have one serialPortDlg
    if(serialport_dlg!=NULL){
        serialport_dlg->show();
        return;
    }
    serialport_dlg=new SerialPortDlg(&thd_safe_que,this);
    connect(serialport_dlg,SIGNAL(open_port_signal(std::string,uint32_t,
                                                   serial::bytesize_t,
                                                   serial::parity_t,
                                                   serial::stopbits_t,
                                                   serial::flowcontrol_t)),
            this,SLOT(open_port_clicked_slot(std::string,uint32_t,
                                           serial::bytesize_t,
                                           serial::parity_t,
                                           serial::stopbits_t,
                                           serial::flowcontrol_t)));
    connect(serialport_dlg,SIGNAL(close_port_signal()),this,SLOT(close_port_slot()));
    connect(this,SIGNAL(open_serial_port_msg(QString)),serialport_dlg,SLOT(show_msg_box(QString)));
    connect(this,SIGNAL(open_port_success(QString)),serialport_dlg,SLOT(open_port_seccess_slot(QString)));
    connect(this,SIGNAL(close_port_success_signal(QString)),serialport_dlg,SLOT(close_port_success_slot(QString)));
    connect(serialport_dlg,SIGNAL(configWrite_signal(const protocol::CfgData&)),
            pSerialwrite,SLOT(configWrite_slot(const protocol::CfgData&)));
    connect(serialport_dlg,&SerialPortDlg::bytesWrite_signal,pSerialwrite,&SerialWriteObj::bytesWrite_slot);
    serialport_dlg->show();
}

void GScopeMainWindow::stk_push_data_slot(const SerialResPkg &respkg)
{
    if(mStk.size()>100){
        mStk=std::stack<SerialResPkg>();
    }
    mStk.push(respkg);
}

void GScopeMainWindow::begin_plot_slot( )
{
    if(mStk.empty())
        return;
    mPkg=mStk.top();
    mStk=std::stack<SerialResPkg>();
    rotateOneStep();
    acclTimerSlot();
    gyroTimerSlot();
    angleTimerSlot();
}

void GScopeMainWindow::rotateOneStep()
{
    if (ui->openGLWidget)
        ui->openGLWidget->rotateBy(mPkg.angular.roll*16.0f,
                                   mPkg.angular.yaw*16.0f,mPkg.angular.pitch*16.0f);
}

void GScopeMainWindow::acclTimerSlot()
{
    // calculate and add a new data point to each graph:
    acclGraph1->addData(acclGraph1->dataCount(), mPkg.linear_acceleration.x);
    acclGraph2->addData(acclGraph2->dataCount(), mPkg.linear_acceleration.y);
    acclGraph3->addData(acclGraph3->dataCount(), mPkg.linear_acceleration.z);

    // make key axis range scroll with the data:
    ui->accl_plot->xAxis->rescale();
    acclGraph1->rescaleValueAxis(false, true);
    acclGraph2->rescaleValueAxis(false, true);
    acclGraph3->rescaleValueAxis(false, true);
    ui->accl_plot->xAxis->setRange(ui->accl_plot->xAxis->range().upper, 100, Qt::AlignRight);

    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    double graph1Value = acclGraph1->dataMainValue(acclGraph1->dataCount()-1);
    double graph2Value = acclGraph2->dataMainValue(acclGraph2->dataCount()-1);
    double graph3Value = acclGraph3->dataMainValue(acclGraph3->dataCount()-1);
    acclTag1->updatePosition(graph1Value);
    acclTag2->updatePosition(graph2Value);
    acclTag3->updatePosition(graph3Value);
    acclTag1->setText(QString::number(graph1Value, 'f', 2));
    acclTag2->setText(QString::number(graph2Value, 'f', 2));
    acclTag3->setText(QString::number(graph3Value, 'f', 2));

    ui->accl_plot->replot();
}

void GScopeMainWindow::gyroTimerSlot()
{
    // calculate and add a new data point to each graph:
    gyroGraph1->addData(gyroGraph1->dataCount(), mPkg.angular_velocity.x);
    gyroGraph2->addData(gyroGraph2->dataCount(), mPkg.angular_velocity.y);
    gyroGraph3->addData(gyroGraph3->dataCount(), mPkg.angular_velocity.z);

    // make key axis range scroll with the data:
    ui->gyro_plot->xAxis->rescale();
    gyroGraph1->rescaleValueAxis(false, true);
    gyroGraph2->rescaleValueAxis(false, true);
    gyroGraph3->rescaleValueAxis(false, true);
    ui->gyro_plot->xAxis->setRange(ui->gyro_plot->xAxis->range().upper, 100, Qt::AlignRight);

    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    double graph1Value = gyroGraph1->dataMainValue(gyroGraph1->dataCount()-1);
    double graph2Value = gyroGraph2->dataMainValue(gyroGraph2->dataCount()-1);
    double graph3Value = gyroGraph3->dataMainValue(gyroGraph3->dataCount()-1);
    gyroTag1->updatePosition(graph1Value);
    gyroTag2->updatePosition(graph2Value);
    gyroTag3->updatePosition(graph3Value);
    gyroTag1->setText(QString::number(graph1Value, 'f', 2));
    gyroTag2->setText(QString::number(graph2Value, 'f', 2));
    gyroTag3->setText(QString::number(graph3Value, 'f', 2));

    ui->gyro_plot->replot();
}

void GScopeMainWindow::angleTimerSlot()
{
    // calculate and add a new data point to each graph:
    angleGraph1->addData(angleGraph1->dataCount(), mPkg.angular.roll);
    angleGraph2->addData(angleGraph2->dataCount(), mPkg.angular.pitch);
    angleGraph3->addData(angleGraph3->dataCount(), mPkg.angular.yaw);

    // make key axis range scroll with the data:
    ui->angle_plot->xAxis->rescale();
    angleGraph1->rescaleValueAxis(false, true);
    angleGraph2->rescaleValueAxis(false, true);
    angleGraph3->rescaleValueAxis(false, true);
    ui->angle_plot->xAxis->setRange(ui->angle_plot->xAxis->range().upper, 100, Qt::AlignRight);

    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    double graph1Value = angleGraph1->dataMainValue(angleGraph1->dataCount()-1);
    double graph2Value = angleGraph2->dataMainValue(angleGraph2->dataCount()-1);
    double graph3Value = angleGraph3->dataMainValue(angleGraph3->dataCount()-1);
    angleTag1->updatePosition(graph1Value);
    angleTag2->updatePosition(graph2Value);
    angleTag3->updatePosition(graph3Value);
    angleTag1->setText(QString::number(graph1Value, 'f', 2));
    angleTag2->setText(QString::number(graph2Value, 'f', 2));
    angleTag3->setText(QString::number(graph3Value, 'f', 2));

    ui->angle_plot->replot();
}

void GScopeMainWindow::open_port_clicked_slot(const std::string &port, uint32_t baudrate,
                                            serial::bytesize_t data_bits, serial::parity_t parity,
                                            serial::stopbits_t stop_bits, serial::flowcontrol_t flow_ctrl)
{
    if(mSerial) {
        delete mSerial;
        mSerial = NULL;
    }
    try{
        mSerial = new serial::Serial(port,baudrate,serial::Timeout::simpleTimeout(1000),
                                     data_bits,parity,stop_bits,flow_ctrl);
    }
    catch(std::exception &e){
        QString str = QString(e.what());
        qDebug()<<str;
        emit open_serial_port_msg(str);
        delete mSerial;
        mSerial = NULL;
    }
    if(mSerial) {
        emit open_port_success(QString(mSerial->getPort().c_str()));

        /// serialWriteObj part
        emit serial_changed_signal(mSerial);

        /// the loop read thread (subclass QThread and reimplement run())
        slpthread = new SerialLoopThread(mSerial,&thd_safe_que,false,this);
        // qRegisterMetaType() to register the SerialPkg
        //connect(slpthread,&SerialLoopThread::read_byte_ready_signal,serialport_dlg,&SerialPortDlg::read_byte_ready_slot);
        /* // override 后的run()没有event loop，因此finished()信号不会被发射
        connect(serialThread, &SerialPortThread::finished, serialThread, &QObject::deleteLater);*/

        //slpthread->start();

        /// worker objects to implement mutithread
        thd_serial_process = new QThread();
        pSerialProcess = new SerialProcessObj();
        pSerialProcess->moveToThread(thd_serial_process);
        connect(thd_serial_process,&QThread::finished,pSerialProcess,&QObject::deleteLater);
        //connect(this,&GScopeMainWindow::serial_obj_to_stop_signal,pSerialProcess,&SerialProcessObj::serial_obj_to_stop_slot);
        connect(slpthread,&SerialLoopThread::read_byte_ready_signal,pSerialProcess,&SerialProcessObj::serial_obj_begin_resolve_slot);
        connect(pSerialProcess,&SerialProcessObj::serial_obj_resolve_ok_signal,this,&GScopeMainWindow::stk_push_data_slot);
        thd_serial_process->start();

        mPlotTimer.start(40);
    }
}

void GScopeMainWindow::close_port_slot()
{
    qDebug()<<"GScopeMainWindow::close_port_slot()";

    if(slpthread!=NULL && slpthread->isRunning()){
        slpthread->slp_to_stop(true);
        slpthread->quit();
        slpthread->wait();
        slpthread = NULL;
    }

    if(thd_serial_process!=NULL && thd_serial_process->isRunning()){
        emit serial_obj_to_stop_signal(true);
        delete pSerialProcess;
        pSerialProcess = NULL;

        thd_serial_process->quit();
        thd_serial_process->wait();
        delete thd_serial_process;
        thd_serial_process=NULL;
    }

    QString portname;
    if(mSerial) {
        portname=QString(mSerial->getPort().c_str());
        delete mSerial;
        mSerial = NULL;
        emit serial_changed_signal(mSerial);
    }

    emit close_port_success_signal(portname);
}

void GScopeMainWindow::initial_main_toolbar()
{
#if defined(_WIN32)
    act_USB = new QAction(QIcon("://images/interface.png"),tr("Interface"),mainToolBar);
    act_16365 =new QAction(QIcon("://images/adis16365.png"),tr("ADIS16365"),mainToolBar);
    act_calibration = new QAction(QIcon("://images/calibration.png"),tr("Calibration"),mainToolBar);
    act_datalog = new QAction(QIcon("://images/datalog.png"),tr("Datalog"),mainToolBar);
    act_operation_ctrl = new QAction(QIcon("://images/operation.png"),tr("Operation Control"),mainToolBar);
    act_alarms = new QAction(QIcon("://images/alarms.png"),tr("Alarms"),mainToolBar);
    act_registers = new QAction(QIcon("://images/register.png"),tr("Register Information"),mainToolBar);
    act_exit = new QAction(QIcon("://images/exit.png"),tr("Exit"),mainToolBar);
#elif !defined(_WIN32)
    act_USB = new QAction(QIcon("./images/interface.png"),tr("Interface"),mainToolBar);
    act_16365 =new QAction(QIcon("./images/adis16365.png"),tr("ADIS16365"),mainToolBar);
    act_calibration = new QAction(QIcon("./images/calibration.png"),tr("Calibration"),mainToolBar);
    act_datalog = new QAction(QIcon("./images/datalog.png"),tr("Datalog"),mainToolBar);
    act_operation_ctrl = new QAction(QIcon("./images/operation.png"),tr("Operation Control"),mainToolBar);
    act_alarms = new QAction(QIcon("./images/alarms.png"),tr("Alarms"),mainToolBar);
    act_registers = new QAction(QIcon("./images/register.png"),tr("Register Information"),mainToolBar);
    act_exit = new QAction(QIcon("./images/exit.png"),tr("Exit"),mainToolBar);
#endif

    //act_registers->setFont(QFont("Times",8));
    //act_registers->setIconVisibleInMenu(true);
    //ui->mainTool->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    mainToolBar->addAction(act_USB);
    mainToolBar->addAction(act_16365);
    mainToolBar->addAction(act_calibration);
    mainToolBar->addAction(act_datalog);
    mainToolBar->addAction(act_operation_ctrl);
    mainToolBar->addAction(act_alarms);
    mainToolBar->addAction(act_registers);
    mainToolBar->addAction(act_exit);

    connect(act_USB,&QAction::triggered,this,&GScopeMainWindow::on_actionUSB_triggered);
    connect(act_16365,&QAction::triggered,this,&GScopeMainWindow::on_action16365_triggered);
    connect(act_calibration,&QAction::triggered,this,&GScopeMainWindow::on_actionCalibration_triggered);
    connect(act_datalog,&QAction::triggered,this,&GScopeMainWindow::on_actionDatalog_triggered);
    connect(act_operation_ctrl,&QAction::triggered,this,&GScopeMainWindow::on_actionOperation_Control_triggered);
    connect(act_alarms,&QAction::triggered,this,&GScopeMainWindow::on_actionAlarms_triggered);
    connect(act_registers,&QAction::triggered,this,&GScopeMainWindow::on_actionRegisters_triggered);
    connect(act_exit,&QAction::triggered,this,&GScopeMainWindow::on_actionExit_triggered);
}

void GScopeMainWindow::initial_accl_plot()
{
    // configure plot to have two right axes:
    ui->accl_plot->yAxis->setTickLabels(false);
    ui->accl_plot->xAxis->setTickLabels(false);

    // left axis only mirrors inner right axis
    connect(ui->accl_plot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->accl_plot->yAxis, SLOT(setRange(QCPRange)));

    ui->accl_plot->yAxis2->setVisible(true);
    ui->accl_plot->axisRect()->addAxis(QCPAxis::atRight);
    ui->accl_plot->axisRect()->addAxis(QCPAxis::atRight);
    ui->accl_plot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(30); // add some padding to have space for tags
    ui->accl_plot->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(30); // add some padding to have space for tags
    ui->accl_plot->axisRect()->axis(QCPAxis::atRight, 2)->setPadding(30); // add some padding to have space for tags

    // create graphs:
    acclGraph1 = ui->accl_plot->addGraph(ui->accl_plot->xAxis, ui->accl_plot->axisRect()->axis(QCPAxis::atRight, 0));
    acclGraph2 = ui->accl_plot->addGraph(ui->accl_plot->xAxis, ui->accl_plot->axisRect()->axis(QCPAxis::atRight, 1));
    acclGraph3 = ui->accl_plot->addGraph(ui->accl_plot->xAxis, ui->accl_plot->axisRect()->axis(QCPAxis::atRight, 2));
    acclGraph1->setPen(QPen(QColor(250, 120, 0)));
    acclGraph2->setPen(QPen(QColor(0, 180, 60)));
    acclGraph3->setPen(QPen(QColor(28, 28, 28)));

    // create tags with newly introduced AxisTag class (see axistag.h/.cpp):
    acclTag1 = new AxisTag(acclGraph1->valueAxis());
    acclTag1->setPen(acclGraph1->pen());
    acclTag2 = new AxisTag(acclGraph2->valueAxis());
    acclTag2->setPen(acclGraph2->pen());
    acclTag3 = new AxisTag(acclGraph3->valueAxis());
    acclTag3->setPen(acclGraph3->pen());

    acclTag1->setText(QString::number(0.0, 'f', 2));
    acclTag2->setText(QString::number(0.0, 'f', 2));
    acclTag3->setText(QString::number(0.0, 'f', 2));
}

void GScopeMainWindow::initial_gyro_plot()
{
    // configure plot to have two right axes:
    ui->gyro_plot->yAxis->setTickLabels(false);
    ui->gyro_plot->xAxis->setTickLabels(false);

    // left axis only mirrors inner right axis
    connect(ui->gyro_plot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->gyro_plot->yAxis, SLOT(setRange(QCPRange)));

    ui->gyro_plot->yAxis2->setVisible(true);
    ui->gyro_plot->axisRect()->addAxis(QCPAxis::atRight);
    ui->gyro_plot->axisRect()->addAxis(QCPAxis::atRight);
    ui->gyro_plot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(30); // add some padding to have space for tags
    ui->gyro_plot->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(30); // add some padding to have space for tags
    ui->gyro_plot->axisRect()->axis(QCPAxis::atRight, 2)->setPadding(30); // add some padding to have space for tags

    // create graphs:
    gyroGraph1 = ui->gyro_plot->addGraph(ui->gyro_plot->xAxis, ui->gyro_plot->axisRect()->axis(QCPAxis::atRight, 0));
    gyroGraph2 = ui->gyro_plot->addGraph(ui->gyro_plot->xAxis, ui->gyro_plot->axisRect()->axis(QCPAxis::atRight, 1));
    gyroGraph3 = ui->gyro_plot->addGraph(ui->gyro_plot->xAxis, ui->gyro_plot->axisRect()->axis(QCPAxis::atRight, 2));
    gyroGraph1->setPen(QPen(QColor(250, 120, 0)));
    gyroGraph2->setPen(QPen(QColor(0, 180, 60)));
    gyroGraph3->setPen(QPen(QColor(28, 28, 28)));

    // create tags with newly introduced AxisTag class (see axistag.h/.cpp):
    gyroTag1 = new AxisTag(gyroGraph1->valueAxis());
    gyroTag1->setPen(gyroGraph1->pen());
    gyroTag2 = new AxisTag(gyroGraph2->valueAxis());
    gyroTag2->setPen(gyroGraph2->pen());
    gyroTag3 = new AxisTag(gyroGraph3->valueAxis());
    gyroTag3->setPen(gyroGraph3->pen());

    gyroTag1->setText(QString::number(0.0, 'f', 2));
    gyroTag2->setText(QString::number(0.0, 'f', 2));
    gyroTag3->setText(QString::number(0.0, 'f', 2));
}

void GScopeMainWindow::initial_angle_plot()
{
    // configure plot to have three right axes:
    ui->angle_plot->yAxis->setTickLabels(false);
    ui->angle_plot->xAxis->setTickLabels(false);

    // left axis only mirrors inner right axis
    connect(ui->angle_plot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui->angle_plot->yAxis, SLOT(setRange(QCPRange)));

    ui->angle_plot->yAxis2->setVisible(true);
    ui->angle_plot->axisRect()->addAxis(QCPAxis::atRight);
    ui->angle_plot->axisRect()->addAxis(QCPAxis::atRight);
    ui->angle_plot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(25); // add some padding to have space for tags
    ui->angle_plot->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(25); // add some padding to have space for tags
    ui->angle_plot->axisRect()->axis(QCPAxis::atRight, 2)->setPadding(25); // add some padding to have space for tags

    // create graphs:
    angleGraph1 = ui->angle_plot->addGraph(ui->angle_plot->xAxis, ui->angle_plot->axisRect()->axis(QCPAxis::atRight, 0));
    angleGraph2 = ui->angle_plot->addGraph(ui->angle_plot->xAxis, ui->angle_plot->axisRect()->axis(QCPAxis::atRight, 1));
    angleGraph3 = ui->angle_plot->addGraph(ui->angle_plot->xAxis, ui->angle_plot->axisRect()->axis(QCPAxis::atRight, 2));
    angleGraph1->setPen(QPen(QColor(250, 120, 0)));
    angleGraph2->setPen(QPen(QColor(0, 180, 60)));
    angleGraph3->setPen(QPen(QColor(28, 28, 28)));

    // create tags with newly introduced AxisTag class (see axistag.h/.cpp):
    angleTag1 = new AxisTag(angleGraph1->valueAxis());
    angleTag1->setPen(angleGraph1->pen());
    angleTag2 = new AxisTag(angleGraph2->valueAxis());
    angleTag2->setPen(angleGraph2->pen());
    angleTag3 = new AxisTag(angleGraph3->valueAxis());
    angleTag3->setPen(angleGraph3->pen());

    angleTag1->setText(QString::number(0.0, 'f', 2));
    angleTag2->setText(QString::number(0.0, 'f', 2));
    angleTag3->setText(QString::number(0.0, 'f', 2));
}

void GScopeMainWindow::initial_glwidget()
{
     QColor clearColor;
     clearColor.setHsv(0,255, 63);
     //ui->openGLWidget =new GLWidget;
     ui->openGLWidget->setClearColor(clearColor);
     //ui->openGLWidget->rotateBy(+42 * 16, +42 * 16, -21 * 16);
}

void GScopeMainWindow::disableWidget()
{
    ui->lineEdit_powersupply->setDisabled(true);
    ui->lineEdit_xgyro->setDisabled(true);
    ui->lineEdit_ygyro->setDisabled(true);
    ui->lineEdit_zgyro->setDisabled(true);
    ui->lineEdit_xaccl->setDisabled(true);
    ui->lineEdit_yaccl->setDisabled(true);
    ui->lineEdit_zaccl->setDisabled(true);
    ui->lineEdit_xtemp->setDisabled(true);
    ui->lineEdit_ytemp->setDisabled(true);
    ui->lineEdit_ztemp->setDisabled(true);
    ui->lineEdit_auxadc->setDisabled(true);
    ui->lineEdit_loopdelay->setDisabled(true);
    ui->pushButton_read->setDisabled(true);
    ui->checkBox_loop->setDisabled(true);

    ui->checkBox_xgyro->setDisabled(true);
    ui->checkBox_ygyro->setDisabled(true);
    ui->checkBox_zgyro->setDisabled(true);
    ui->checkBox_xaccl->setDisabled(true);
    ui->checkBox_yaccl->setDisabled(true);
    ui->checkBox_zaccl->setDisabled(true);
    ui->tbtn_x_gyro->setDisabled(true);
    ui->tbtn_y_gyro->setDisabled(true);
    ui->tbtn_z_gyro->setDisabled(true);
    ui->tbtn_x_accl->setDisabled(true);
    ui->tbtn_y_accl->setDisabled(true);
    ui->tbtn_z_accl->setDisabled(true);

    ui->pushButton_readstatus->setDisabled(true);
    ui->lineEdit_powersupplylow->setDisabled(true);
    ui->lineEdit_powersupplyhigh->setDisabled(true);
    ui->lineEdit_controlwrite->setDisabled(true);
    ui->lineEdit_spiwrite->setDisabled(true);
    ui->lineEdit_alarm1->setDisabled(true);
    ui->lineEdit_alarm2->setDisabled(true);

    ui->radioButton_off->setDisabled(true);
    ui->radioButton_on->setDisabled(true);
    ui->lineEdit_set->setDisabled(true);
    ui->pushButton_run->setDisabled(true);
    ui->lineEdit_elapsed->setDisabled(true);
}

void GScopeMainWindow::on_pushButton_start_clicked()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    if(mSerial==NULL){
        QMessageBox::information(this,"Open port",
                                 "The serial port is not opened. Please open port first.");
        return;
    }
    slpthreadStartSlot();

    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head =protocol::master;
    cmd.op   =protocol::set;
    cmd.cmdf =protocol::start_upload;
    cmd.tl1  =protocol::tail1;
    cmd.tl2  =protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);
}



void GScopeMainWindow::on_pushButton_clicked()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    if(mSerial==NULL){
        QMessageBox::information(this,"Open port",
                                 "The serial port is not opened. Please open port first.");
        return;
    }

    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head =protocol::master;
    cmd.op   =protocol::get;
    cmd.cmdf =protocol::product_id;
    cmd.tl1  =protocol::tail1;
    cmd.tl2  =protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);

}

void GScopeMainWindow::slpthreadStartSlot()
{
    if(slpthread==NULL){
        QMessageBox::information(this,"slpthread",
                                 "slpthread=NULL!");
        return;
    }
    slpthread->start();
}

// the following function will not be called
void GScopeMainWindow::initial_menuToolBar()
{
    ///menuToolBar
    tbtn_interface = new QToolButton(menuToolBar);
    menu_interface = new QMenu(this);
    tbtn_device = new QToolButton(menuToolBar);
    menu_device = new QMenu(this);
    tbtn_configuration = new QToolButton(menuToolBar);
    menu_configuration = new QMenu(this);
    tbtn_datalog = new QToolButton(menuToolBar);
    menu_datalog = new QMenu(this);
    tbtn_registers = new QToolButton(menuToolBar);
    menu_registers = new QMenu(this);
    tbtn_exit = new QToolButton(menuToolBar);
    menu_exit = new QMenu(this);

    ///interface
    tbtn_interface->setText(tr("Interface"));
    //tbtn_interface->setToolButtonStyle(Qt::ToolButtonTextOnly);
    tbtn_interface->setPopupMode(QToolButton::InstantPopup);

    //QAction *act_menu_USB = new QAction(tr("USB"),menu_interface);
    act_menu_USB = menu_interface->addAction(tr("USB"),this,SLOT(on_actionUSB_triggered()));
    act_menu_USB->setCheckable(true);
    act_menu_USB->setChecked(true);
    act_menu_USB->setIconVisibleInMenu(true);

    tbtn_interface->setMenu(menu_interface);
    menuToolBar->addWidget(tbtn_interface);

    ///Device
    tbtn_device->setText(tr("Device"));
    tbtn_device->setPopupMode(QToolButton::InstantPopup);
    act_menu_16365 = menu_device->addAction(tr("16365"),this,SLOT(on_action16365_triggered()));
    act_menu_16365->setCheckable(true);
    act_menu_16365->setChecked(true);

    tbtn_device->setMenu(menu_device);
    menuToolBar->addWidget(tbtn_device);

    ///Configuration
    tbtn_configuration->setText(tr("Configuration"));
    tbtn_configuration->setPopupMode(QToolButton::InstantPopup);

    act_menu_calibration = menu_configuration->addAction(QIcon(":/res_images/register"),
                              tr("Calibration"),this,SLOT(on_actionCalibration_triggered()));
    act_menu_operation_ctrl = menu_configuration->addAction(QIcon(":/res_images/register"),
                              tr("Operation Control"),this,SLOT(on_actionOperation_Control_triggered()));
    act_menu_alarms = menu_configuration->addAction(QIcon(":/res_images/register"),
                              tr("Alarms"),this,SLOT(on_actionAlarms_triggered()));
    act_menu_calibration->setIconVisibleInMenu(true);
    act_menu_operation_ctrl->setIconVisibleInMenu(true);
    act_menu_alarms->setIconVisibleInMenu(true);

    tbtn_configuration->setMenu(menu_configuration);
    menuToolBar->addWidget(tbtn_configuration);

    ///Datalog
    tbtn_datalog->setText(tr("Datalog"));
    tbtn_datalog->setPopupMode(QToolButton::InstantPopup);

    act_menu_datalog = menu_datalog->addAction(QIcon(":/res_images/register"),
                            tr("Datalog"),this,SLOT(on_actionDatalog_triggered()));
    act_menu_datalog->setIconVisibleInMenu(true);

    tbtn_datalog->setMenu(menu_datalog);
    menuToolBar->addWidget(tbtn_datalog);

    ///Registers
    tbtn_registers->setText(tr("Registers"));
    tbtn_registers->setPopupMode(QToolButton::InstantPopup);

    act_menu_registers = menu_registers->addAction(QIcon(":/res_images/register"),
                             tr("Registers"),this,SLOT(on_actionRegisters_triggered()));
    act_menu_registers->setIconVisibleInMenu(true);

    tbtn_registers->setMenu(menu_registers);
    menuToolBar->addWidget(tbtn_registers);

    ///Exit
    tbtn_exit->setText(tr("Exit"));
    tbtn_exit->setPopupMode(QToolButton::InstantPopup);

    menu_exit->addAction(tr("Exit"),this,SLOT(on_actionExit_triggered()));

    tbtn_exit->setMenu(menu_exit);
    menuToolBar->addWidget(tbtn_exit);

    menuToolBar->setMovable(false);
    menuToolBar->setStyleSheet("QToolButton::menu-indicator {image: None;}");
}

