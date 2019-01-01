#ifndef DATALOGCTRLDLG_H
#define DATALOGCTRLDLG_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QStatusBar>
#include <vector>
#include <QFile>
#include <QTimer>
#include "thread/serialdataformat.h"

/*
QT_BEGIN_NAMESPACE
class QWidget;
class QLabel;
class QPushButton;
class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QCheckBox;
class QGroupBox;
QT_END_NAMESPACE
*/

class DatalogCtrlDlg : public QDialog
{
    Q_OBJECT
private:
    QGroupBox* gpb_file_setup;

    QLabel* lab_samples_per_file;
    QLabel* lab_sample_delay_msec;
    //QLabel* lab_files_per_session;

    QLineEdit* lnedit_samples_per_file;
    QLineEdit* lnedit_sample_delay_msec;
    //QLineEdit* lnedit_files_per_session;

    QGroupBox* gpb_file_information;
//    QLabel* lab_directory;
//    QLabel* lab_file_name;
//    QLabel* lab_file_num;
//    QLabel* lab_txt;
    QLabel* lab_file_path;
    QLineEdit* lnedit_file_path;
    QPushButton* btn_file_path;

//    QLineEdit* lnedit_directory;
//    QLineEdit* lnedit_file_name;
//    QLineEdit* lnedit_file_num;

    QGroupBox* gpb_data_selection;
    QCheckBox* cbx_power_supply;
    QCheckBox* cbx_x_gyro;
    QCheckBox* cbx_y_gyro;
    QCheckBox* cbx_z_gyro;
    QCheckBox* cbx_x_accel;
    QCheckBox* cbx_y_accel;
    QCheckBox* cbx_z_accel;
    QCheckBox* cbx_x_temper;
    QCheckBox* cbx_y_temper;
    QCheckBox* cbx_z_temper;
    QCheckBox* cbx_aux_adc;

    QPushButton* btn_start_datalog;
    QPushButton* btn_stop_datalog;

    QHBoxLayout* lyt_total;

    QVBoxLayout *lyt_total_pls_stusbar;
    QStatusBar* status_bar;

    QFile* pfile;
    int sampleTimes;
    int delayTimeMs;
    QTimer delayTimer;

    std::vector<bool>delaySampleVec;

public:
    explicit DatalogCtrlDlg(QWidget *parent = 0);

    ~DatalogCtrlDlg();
protected:
    void closeEvent(QCloseEvent *event);
signals:
    void queryData_signal(const protocol::CfgData& d);
public slots:
    void resolveData_slot(const protocol::CfgData& d);
    void startDatalogSlot();
    void stopDatalogSlot();
    void getFilePathSlot();
    void delaySampleSlot();

private:
    void initialWidget();
    void getCheckBoxVector();
    void sendPkg(protocol::operation_t op,char ch,float fl=0.0);

};

#endif // DATALOGCTRLDLG_H
