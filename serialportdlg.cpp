#include "serialportdlg.h"
#include <QLabel>
#include <QGridLayout>
#include <QDebug>
#include <QMessageBox>
#include <cstdio>
#include <QDebug>

bool SerialPortDlg::is_port_open=false;

SerialPortDlg::SerialPortDlg(threadsafe_queue<SerialPkg> *que,QWidget *parent)
    :QDialog(parent),mQueue(que)
{
    //setAttribute(Qt::WA_DeleteOnClose,true);

    initialWidget();

    QRegExp regExp("([0-9a-fA-F]{2}\\s){0,10}([0-9a-fA-F]{2}){0,1}");
    lnedit_send->setValidator(new QRegExpValidator(regExp, this));

    connect(btn_send,&QPushButton::clicked,this,&SerialPortDlg::btn_send_clicked_slot);
    connect(btn_open_port,SIGNAL(clicked(bool)),this,SLOT(btn_open_port_clicked()));
    connect(combo_port_name,SIGNAL(activated(int)),this,SLOT(display_serial_port_info(int)));

    // plntxt_content->document()->setMaximumBlockCount(10000);
    connect(&mTimer,&QTimer::timeout,this,&SerialPortDlg::refresh_txt_timer_slot);
    is_port_open = false;
}

SerialPortDlg::~SerialPortDlg()
{

}

void SerialPortDlg::closeEvent(QCloseEvent *e)
{
    if(!is_port_open){
        e->accept();
        return;
    }
    QMessageBox::information(this,"Close Window","Even if the window is closed, "
                                                 "the port is still working.");
    e->accept();
}

void SerialPortDlg::initialWidget()
{
    QFrame* frm_above_part=new QFrame(this);
    frm_above_part->setLineWidth(1);
    frm_above_part->setFrameStyle(QFrame::Box|QFrame::Plain);
    //frm_above_part->setMaximumWidth(250);

    // plntxt_content=new QPlainTextEdit(this);
    plntxt_content=new QTextBrowser(this);

    QLabel* lab_port_name =new QLabel("Port Name:",frm_above_part);
    QLabel* lab_boud_rate =new QLabel("Baud Rate:",frm_above_part);
    QLabel* lab_data_bits =new QLabel("Data Bits:",frm_above_part);
    QLabel* lab_parity    =new QLabel("Parity:",frm_above_part);
    QLabel* lab_stop_bits =new QLabel("Stop Bits:",frm_above_part);
    QLabel* lab_flow_ctrl =new QLabel("Flow Control:",frm_above_part);

    combo_port_name = new QComboBox(frm_above_part);
    combo_baud_rate = new QComboBox(frm_above_part);
    combo_data_bits = new QComboBox(frm_above_part);
    combo_stop_bits = new QComboBox(frm_above_part);
    combo_parity    = new QComboBox(frm_above_part);
    combo_flow_ctrl = new QComboBox(frm_above_part);

    QStringList qstrlst_port_name;
    std::vector<serial::PortInfo> devices_found = serial::list_ports();
    std::vector<serial::PortInfo>::iterator iter = devices_found.begin();
    QString str_port;

    while( iter != devices_found.end() )
    {
        serial::PortInfo device = *iter++;
        str_port=QString(device.port.c_str());
        qstrlst_port_name<<str_port;
        qvec_device_port.push_back(str_port);
        qvec_device_desp.push_back(device.description.c_str());
        qvec_device_id.push_back(device.hardware_id.c_str());
    }

    QStringList qstrlst_baud_rate,qstrlst_data_bits,qstrlst_parity,
            qstrlst_stop_bits,qstrlst_flow_ctrl;
    qstrlst_baud_rate<<tr("Custom")<<tr("110")<<tr("300")<<tr("600")
                     <<tr("1200")<<tr("2400")<<tr("4800")<<tr("9600")
                     <<tr("14400")<<tr("19200")<<tr("38400")<<tr("56000")
                     <<tr("57600")<<tr("115200")<<tr("128000")<<tr("256000");
    qstrlst_data_bits<<tr("5")<<tr("6")<<tr("7")<<tr("8");
    qstrlst_parity<<tr("None")<<tr("Odd")<<tr("Even")<<tr("Mark")<<tr("Space");
    qstrlst_stop_bits<<tr("1")<<tr("2")<<tr("1.5");
    qstrlst_flow_ctrl<<tr("None")<<tr("Software")<<tr("Hardware");

    combo_port_name->addItems(qstrlst_port_name);
    combo_baud_rate->addItems(qstrlst_baud_rate);
    combo_data_bits->addItems(qstrlst_data_bits);
    combo_parity->addItems(qstrlst_parity);
    combo_stop_bits->addItems(qstrlst_stop_bits);
    combo_flow_ctrl->addItems(qstrlst_flow_ctrl);

    btn_open_port = new QPushButton("Open Port",frm_above_part);

    QGridLayout* lyt_grid_frm =new QGridLayout;
    lyt_grid_frm->addWidget(lab_port_name,0,0);
    lyt_grid_frm->addWidget(lab_boud_rate,1,0);
    lyt_grid_frm->addWidget(lab_data_bits,2,0);
    lyt_grid_frm->addWidget(lab_stop_bits,3,0);
    lyt_grid_frm->addWidget(lab_parity,   4,0);
    lyt_grid_frm->addWidget(lab_flow_ctrl,5,0);

    lyt_grid_frm->addWidget(combo_port_name,0,1);
    lyt_grid_frm->addWidget(combo_baud_rate,1,1);
    lyt_grid_frm->addWidget(combo_data_bits,2,1);
    lyt_grid_frm->addWidget(combo_stop_bits,3,1);
    lyt_grid_frm->addWidget(combo_parity,   4,1);
    lyt_grid_frm->addWidget(combo_flow_ctrl,5,1);

    lyt_grid_frm->addWidget(btn_open_port,6,0);

    frm_above_part->setLayout(lyt_grid_frm);

    lnedit_send = new QLineEdit(this);
    btn_send = new QPushButton(tr("Send"),this);

    QHBoxLayout* lyt_send = new QHBoxLayout;
    lyt_send->addWidget(lnedit_send);;
    lyt_send->addWidget(btn_send);

    QVBoxLayout* lyt_left = new QVBoxLayout;
    lyt_left->addLayout(lyt_send);
    lyt_left->addWidget(plntxt_content);

    QHBoxLayout* lyt_total =new QHBoxLayout;
    lyt_total->addWidget(frm_above_part);
    lyt_total->addLayout(lyt_left);

    stsbar_serial = new QStatusBar(this);

    QVBoxLayout *lyt_total_pls_stusbar= new QVBoxLayout;
    lyt_total_pls_stusbar->addLayout(lyt_total);
    lyt_total_pls_stusbar->addWidget(stsbar_serial);

    lyt_total_pls_stusbar->setMargin(2);
    setLayout(lyt_total_pls_stusbar);

    combo_baud_rate->setCurrentIndex(13);//9600
    combo_data_bits->setCurrentIndex(3);//8
    combo_flow_ctrl->setCurrentIndex(0);//None
    combo_parity->setCurrentIndex(0);//None
    combo_stop_bits->setCurrentIndex(0);//1
    setWindowTitle(tr("Serial Port Information"));
}

void SerialPortDlg::btn_open_port_clicked()
{
    qDebug()<<"SerialPortDlg::btn_open_port_clicked()";

    if(is_port_open)
    {
        emit close_port_signal();
        qDebug()<<"SerialPortDlg::close_port_signal() emitted.";
        return;
    }

    const uint32_t mBaudrate[16]={0,110,300,600,
                            1200,2400,4800,9600,
                            14400,19200,38400,56000,
                            57600,115200,128000,256000};

    const serial::bytesize_t mDatabits[4]={serial::fivebits,
                                     serial::sixbits,
                                     serial::sevenbits,
                                     serial::eightbits};

    const serial::parity_t mParity[5]={serial::parity_none,
                                 serial::parity_odd,
                                 serial::parity_even,
                                 serial::parity_mark,
                                 serial::parity_space
                                 };

    const serial::stopbits_t mStopbits[3]={serial::stopbits_one,
                                     serial::stopbits_two,
                                     serial::stopbits_one_point_five};

    const serial::flowcontrol_t mFlowctrl[3]={serial::flowcontrol_none,
                                        serial::flowcontrol_software,
                                        serial::flowcontrol_hardware};
    std::string str_cur_port=qvec_device_port[combo_port_name->currentIndex()].toStdString();
    emit open_port_signal(str_cur_port,
                          mBaudrate[combo_baud_rate->currentIndex()],
                          mDatabits[combo_data_bits->currentIndex()],
                          mParity[combo_parity->currentIndex()],
                          mStopbits[combo_stop_bits->currentIndex()],
            mFlowctrl[combo_flow_ctrl->currentIndex()]);
    qDebug()<<"SerialPortDlg::open_port_signal(...) emitted.";
}

void SerialPortDlg::btn_send_clicked_slot()
{

    if(!is_port_open){
        QString str("Serial port is not opened!");
        QMessageBox::information(this,"Serial Port", str);
        return;
    }

    QString getstr=lnedit_send->text();
    if(getstr.isEmpty())
        return;

    QStringList strlist=getstr.toLower().split(' ',QString::SkipEmptyParts);
    std::vector<uint8_t> vec;
    bool ok;
    for(int i=0;i<strlist.size();++i){
        uint8_t ch=static_cast<uint8_t>(strlist[i].toInt(&ok,16));
        vec.push_back(ch);
    }

    emit bytesWrite_signal(vec);
}

void SerialPortDlg::display_serial_port_info(int index)
{
    qDebug()<<"SerialPortDlg::display_serial_port_info(int index)";
    QString str_display=qvec_device_port[index]+":"+qvec_device_desp[index];
    stsbar_serial->showMessage(str_display,2000);
}

void SerialPortDlg::show_msg_box(QString str)
{
    QMessageBox::information(this,"Open Serial Port", str);
}

void SerialPortDlg::open_port_seccess_slot(QString str)
{
    qDebug()<<"SerialPortDlg::open_port_seccess_slot()";
    QString showstr = "Port:"+str+" open success!";
    stsbar_serial->showMessage(showstr,2000);
    is_port_open = true;
    mTimer.start(20);
    if(btn_open_port->text()!="Close Port")
        btn_open_port->setText("Close Port");
}

void SerialPortDlg::close_port_success_slot(QString str)
{
    qDebug()<<"SerialPortDlg::close_port_success_slot()";
    QString showstr = "Port:"+str+" close success!";
    stsbar_serial->showMessage(showstr,2000);
    is_port_open = false;
    mTimer.stop();
    if(btn_open_port->text()!="Open Port")
        btn_open_port->setText("Open Port");
}

void SerialPortDlg::read_byte_ready_slot(const SerialPkg& tpkg)
{
    QString plntxt_append_str;
    char str[10];
    for(int i=0;i<tpkg.length;++i){
        sprintf(str,"%02X ",tpkg.ch[i]);
        plntxt_append_str.append(str);
    }
    qDebug()<<plntxt_append_str;
    //plntxt_content->appendPlainText(plntxt_append_str);
//    plntxt_content->moveCursor(QTextCursor::End);
//    plntxt_content->insertPlainText(plntxt_append_str);
   // plntxt_content->appendPlainText(plntxt_append_str);


//        qDebug()<<plntxt_append_str<<endl;
//        /*
//        QTextCursor text_cursor = QTextCursor(plntxt_content->document());
//        text_cursor.movePosition(QTextCursor::End);
//        text_cursor.insertText(plntxt_append_str);
//        plntxt_content->moveCursor(QTextCursor::End);
//        //plntxt_content->appendPlainText(plntxt_append_str);
//        */
//    }

}

void SerialPortDlg::refresh_txt_timer_slot()
{
    SerialPkg tpkg;
    while(mQueue->try_pop(tpkg)){
        QString plntxt_append_str;
        char str[10];
        for(int i=0;i<tpkg.length;++i){
            sprintf(str,"%02X ",tpkg.ch[i]);
            plntxt_append_str.append(str);
        }
        qDebug()<<plntxt_append_str;
        //plntxt_content->insertPlainText(plntxt_append_str);
    }

}
