#ifndef OPERATIONCONTROLDLG_H
#define OPERATIONCONTROLDLG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include "thread/serialdataformat.h"



/*
QT_BEGIN_NAMESPACE
class QWidget;
class QLabel;
class QPushButton;
class QLineEdit;
class QVBoxLayout;
class QRadioButton;
class QButtonGroup;
QT_END_NAMESPACE
*/

class OperationControlDlg : public QDialog
{
    Q_OBJECT    
public:
    explicit OperationControlDlg(QWidget *parent = 0);
    ~OperationControlDlg();
protected:
    void closeEvent(QCloseEvent *);
private:
    //sample rate
    QLabel* lab_sample_rate;
    QLineEdit* lnedt_sample_rate;
    QLabel* lab_SPS;
    QLabel* lab_SMPL_PRD_contents;
    QLineEdit* lnedit_SMPL_PRD_contents;
    QPushButton* btn_sample_Update;

    //Measurement Range and Digital Filtering
    QLabel* lab_measurement;

    QLabel* lab_select_gyro_range;
    QRadioButton* rad_300_deg_sec;
    QRadioButton* rad_150_deg_sec;
    QRadioButton* rad_75_deg_sec;
    QButtonGroup* bgp_measure_range;
    QPushButton* btn_set_dynamic;

    QLineEdit* lnedit_measure_taps;
    QLabel* lab_tabs;
    QLabel* lab_SENS_AVG_contents;
    QLineEdit* lnedit_SENS_AVG_contents;
    QPushButton* btn_measure_update;

    //Auxilliary Digital I/O Configuration
    QLabel* lab_Auxilliary_digital_cfg;

    ///configure as general
    QLabel* lab_configure_as_general;

    QLabel* lab_digital_IO_line0;
    QRadioButton* rad_io0_input;
    QRadioButton* rad_io0_output;
    QButtonGroup* bgp_io0_digital;

    QLabel* lab_set_line;
    QRadioButton* rad_set_line_high;
    QRadioButton* rad_set_line_low;
    QButtonGroup* bgp_set_line;

    QLabel* lab_digital_IO_line1;
    QRadioButton* rad_io1_input;
    QRadioButton* rad_io1_output;
    QButtonGroup* bgp_io1_digital;

    QLabel* lab_read_line;
    QRadioButton* rad_read_line_high;
    QRadioButton* rad_read_line_low;
    QButtonGroup* bgp_read_line;

    ///configure as a data ready line
    QLabel* lab_configure_as_data;

    QLabel* lab_select_io_line;
    QRadioButton* rad_D1_o0;
    QRadioButton* rad_D1_o1;
    QButtonGroup* bgp_select_io_line;

    QLabel* lab_output_polarity;
    QRadioButton* rad_output_polarity_high;
    QRadioButton* rad_output_polarity_low;
    QButtonGroup* bgp_output_polarity;

    QLabel* lab_enable;
    QRadioButton* rad_enable_on;
    QRadioButton* rad_enable_off;
    QButtonGroup* bgp_enable;

    //Auxilliary D/A Converter Output
    QLabel* lab_Auxilliary_da_converter;
    QLineEdit* lnedit_volts;
    QLabel* lab_volts;
    QLabel* lab_AUX_DAC_contents;
    QLineEdit* lnedit_AUX_DAC_contents;
    QPushButton* btn_auxilliary_update;

    //buttom
    QPushButton* btn_close_windows;
    QPushButton* btn_flash_memory;

    //layout;
    QVBoxLayout* lyt_total;
signals:
    void configWrite_signal(const protocol::CfgData& d);

public slots:
    void configRes_slot(const protocol::CfgData& d);
    void closeWindowSlot();
    void flashMemoryUpdateSlot();
    void sampleRateUpdateSlot();
    void dynamicSetSlot();
    void measureRangeSlot();

private:
     void sendPkg(protocol::operation_t op,protocol::cmdfield_t cmdfield,uint16_t sh=0);
    void initialWidget();
};

#endif // OPERATIONCONTROLDLG_H
