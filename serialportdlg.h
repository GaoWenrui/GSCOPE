#ifndef SERIALPORTDLG_H
#define SERIALPORTDLG_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QLineEdit>
#include <QStatusBar>
#include <QString>
#include <QTimer>
#include "serial/serial.h"
#include "thread/serialdataformat.h"
#include "thread/threadsafequeue.hpp"

class SerialPortDlg : public QDialog
{
    Q_OBJECT
public:
    explicit SerialPortDlg(threadsafe_queue<SerialPkg>* que,QWidget *parent=0);
    ~SerialPortDlg();
protected:
    void closeEvent(QCloseEvent *e);
private:
    QComboBox* combo_port_name;
    QComboBox* combo_baud_rate;
    QComboBox* combo_data_bits;
    QComboBox* combo_parity;
    QComboBox* combo_stop_bits;
    QComboBox* combo_flow_ctrl;

    QPushButton* btn_open_port;

    QLineEdit* lnedit_send;
    QPushButton* btn_send;
    QTextBrowser* plntxt_content;

    QStatusBar *stsbar_serial;

    QVector<QString> qvec_device_port;
    QVector<QString> qvec_device_desp;
    QVector<QString> qvec_device_id;

    threadsafe_queue<SerialPkg>* mQueue;
    QTimer mTimer;
    static bool is_port_open;

    void initialWidget();

signals:
    void open_port_signal(const std::string &port,uint32_t baudrate,serial::bytesize_t data_bits,
                          serial::parity_t parity,serial::stopbits_t stop_bits,serial::flowcontrol_t flow_ctrl);
    void close_port_signal();
    void configWrite_signal(const protocol::CfgData& cfgwt);
    void bytesWrite_signal(const std::vector<uint8_t>& v);
public slots:
    void btn_open_port_clicked();
    void btn_send_clicked_slot();
    void display_serial_port_info(int index);
    void show_msg_box(QString str);
    void open_port_seccess_slot(QString str);
    void close_port_success_slot(QString str);
    void read_byte_ready_slot(const SerialPkg& tpkg);
    void refresh_txt_timer_slot();
};

#endif // SERIALPORTDLG_H
