#ifndef CALIBRATIONDLG_H
#define CALIBRATIONDLG_H

#include <QDialog>
#include <QOpenGLWidget>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <vector>
#include "thread/serialdataformat.h"

/*
QT_BEGIN_NAMESPACE
class QWidget;
//class QDialog;
class QLabel;
class QPushButton;
class QLineEdit;
class QVBoxLayout;
QT_END_NAMESPACE
*/

class CalibrationDlg : public QDialog
{
    Q_OBJECT
public:
    explicit CalibrationDlg(QWidget *parent = 0);
    ~CalibrationDlg();

private:
    // Automatic Features
    QLabel* lab_automatic_features;

    QLabel* lab_restore_factory;
    QLabel* lab_precision_auto_null;
    QLabel* lab_auto_null;
    QPushButton* btn_restore_run;
    QPushButton* btn_precision_run;
    QPushButton* btn_auto_run;

    // Manual Calibration
    QLabel* lab_manual_calibration;

    QLabel* lab_gyroscope;
    QLabel* lab_regisnter_content;

    QLabel* lab_x_offset;
    QLineEdit* lnedit_x_offset;
    QLabel* lab_x_deg_sec;
    QPushButton* btn_x_off_update;
    QLineEdit* lnedit_x_content;

    QLabel* lab_y_offset;
    QLineEdit* lnedit_y_offset;
    QLabel* lab_y_deg_sec;
    QPushButton* btn_y_off_update;
    QLineEdit* lnedit_y_content;

    QLabel* lab_z_offset;
    QLineEdit* lnedit_z_offset;
    QLabel* lab_z_deg_sec;
    QPushButton* btn_z_off_update;
    QLineEdit* lnedit_z_content;

    //Acceleration
    QLabel* lab_acceration;

    QLabel* lab_acc_x_offset;
    QLineEdit* lnedit_acc_x_offset;
    QLabel* lab_acc_x_g;
    QPushButton* btn_acc_x_off_update;
    QLineEdit* lnedit_acc_x_content;

    QLabel* lab_acc_y_offset;
    QLineEdit* lnedit_acc_y_offset;
    QLabel* lab_acc_y_g;
    QPushButton* btn_acc_y_off_update;
    QLineEdit* lnedit_acc_y_content;

    QLabel* lab_acc_z_offset;
    QLineEdit* lnedit_acc_z_offset;
    QLabel* lab_acc_z_g;
    QPushButton* btn_acc_z_off_update;
    QLineEdit* lnedit_acc_z_content;

    //button
    QPushButton* btn_close_window;
    QPushButton* btn_flash_memory_update;

    QVBoxLayout* total_layout;

    void initialWidget();
    void updateContent(uint8_t ch);
    void manualBiasSet(uint8_t ch,float flo);

signals:
    void configWrite_signal(const protocol::CfgData& d);
public slots:
    void restoreRunSlot();
    void precisionRunSlot();
    void autoRunSlot();

    void xUpdateSlot();
    void yUpdateSlot();
    void zUpdateSlot();

    void xAccUpdateSlot();
    void yAccUpdateSlot();
    void zAccUpdateSlot();

    void closeWindowSolt();
    void flashUpdateSlot();

    void configRes_slot(const protocol::CfgData& d);
};

#endif // CALIBRATIONDLG_H
