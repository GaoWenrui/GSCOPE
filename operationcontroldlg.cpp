#include "operationcontroldlg.h"
#include <QtGui>
#include<QMessageBox>

OperationControlDlg::OperationControlDlg(QWidget *parent)
   : QDialog(parent)
{
    //setAttribute(Qt::WA_DeleteOnClose,true);

    initialWidget();

    rad_75_deg_sec->setChecked(true);

    lnedt_sample_rate->setDisabled(true);
    lnedit_measure_taps->setDisabled(true);
    rad_io0_input->setDisabled(true);
    rad_io0_output->setDisabled(true);
    rad_set_line_high->setDisabled(true);
    rad_set_line_low->setDisabled(true);
    rad_io1_input->setDisabled(true);
    rad_io1_output->setDisabled(true);
    rad_read_line_high->setDisabled(true);
    rad_read_line_low->setDisabled(true);
    rad_D1_o0->setDisabled(true);
    rad_D1_o1->setDisabled(true);
    rad_output_polarity_high->setDisabled(true);
    rad_output_polarity_low->setDisabled(true);
    rad_enable_on->setDisabled(true);
    rad_enable_off->setDisabled(true);
    lnedit_volts->setDisabled(true);
    lnedit_AUX_DAC_contents->setDisabled(true);
    btn_auxilliary_update->setDisabled(true);
    setWindowTitle("Operation Control");

    connect(btn_close_windows,&QPushButton::clicked,this,&OperationControlDlg::closeWindowSlot);
    connect(btn_flash_memory,&QPushButton::clicked,this,&OperationControlDlg::flashMemoryUpdateSlot);
    connect(btn_set_dynamic,&QPushButton::clicked,this,&OperationControlDlg::dynamicSetSlot);
    connect(btn_measure_update,&QPushButton::clicked,this,&OperationControlDlg::measureRangeSlot);
    connect(btn_sample_Update,&QPushButton::clicked,this,&OperationControlDlg::sampleRateUpdateSlot);
}

OperationControlDlg::~OperationControlDlg()
{
    delete lyt_total;
}

void OperationControlDlg::closeEvent(QCloseEvent *e)
{
    btn_sample_Update->setEnabled(true);
    btn_set_dynamic->setEnabled(true);
    btn_measure_update->setEnabled(true);
    e->accept();
}

void OperationControlDlg::configRes_slot(const protocol::CfgData &d)
{
     qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
     if(protocol::checkSum(d)==false){
         qDebug()<<"checkSum:false.";
         return;
     }

     if(d.cmdf==protocol::dynamic_range_set){
         btn_set_dynamic->setEnabled(true);
         btn_measure_update->setEnabled(true);
         uint8_t buff[2]={d.val[0],d.val[1]};
         // sh=0x0402 or 0x0202 or 0x0104
         uint16_t sh=*reinterpret_cast<uint16_t*>(buff);
         if(d.op==protocol::set){
             QString dispstr;
             if(sh & 0x0400){
                 dispstr="Dynamic range is set to 300 deg/s.";
                 lnedit_SENS_AVG_contents->setText(QString("%1").arg(sh));
             }else if(sh & 0x0200){
                 dispstr="Dynamic range is set to 150 deg/s.";
                 lnedit_SENS_AVG_contents->setText(QString("%1").arg(sh));
             }else if(sh & 0x0100){
                 dispstr="Dynamic range is set to 75 deg/s.";
                 lnedit_SENS_AVG_contents->setText(QString("%1").arg(sh));
             }else{
                 dispstr="Error";
             }
             QMessageBox::information(this,"Dynamic Range Set",dispstr);
             return;
         }
         if(d.op==protocol::get){
             if((sh&0x0400)||(sh&0x0200)||(sh&0x0100)){
                lnedit_SENS_AVG_contents->setText(QString("%1").arg(sh));
                return;
             }
         }
     }


     if(d.cmdf!=protocol::inter_sampl_cmd){
         btn_sample_Update->setEnabled(true);
         uint8_t buff[2]={d.val[0],d.val[1]};
         // sh=0x0402 or 0x0202 or 0x0104
         uint16_t sh=*reinterpret_cast<uint16_t*>(buff);
         char dispchar[10];
         snprintf(dispchar,5,"%04X",sh);
         lnedit_SMPL_PRD_contents->setText("0x"+QString(dispchar));
     }
}

void OperationControlDlg::closeWindowSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    this->close();
}

void OperationControlDlg::flashMemoryUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    sampleRateUpdateSlot();
    measureRangeSlot();
}

void OperationControlDlg::sampleRateUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    sendPkg(protocol::get,protocol::inter_sampl_cmd,0);
    btn_sample_Update->setDisabled(true);
}

/*!
 * \brief OperationControlDlg::dynamicSetSlot
 * 300deg/s，4抽头：0x0402==>0xB904,0xB802
 * 150deg/s，4抽头：0x0202==>0xB902,0xB802
 * 75deg/s，16抽头：0x0104==>0xB901,0xB804
 */
void OperationControlDlg::dynamicSetSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    int id=bgp_measure_range->checkedId();
    switch (id) {
    case 1:
        sendPkg(protocol::set,protocol::dynamic_range_set,0x0402);
        break;
    case 2:
        sendPkg(protocol::set,protocol::dynamic_range_set,0x0202);
        break;
    case 3:
        sendPkg(protocol::set,protocol::dynamic_range_set,0x0104);
        break;
    default:
        break;
    }
    btn_set_dynamic->setDisabled(true);
    btn_measure_update->setDisabled(true);
}

void OperationControlDlg::measureRangeSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    sendPkg(protocol::get,protocol::dynamic_range_set);
    btn_measure_update->setDisabled(true);
}

void OperationControlDlg::sendPkg(protocol::operation_t op, protocol::cmdfield_t cmdfield, uint16_t sh)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head = protocol::master;
    cmd.op   = op;
    cmd.cmdf = cmdfield;
    //cmd.name = 0x00;default
    if(op==protocol::set){
        char *pch=reinterpret_cast<char*>(&sh);
        cmd.val[0]=*pch;pch++;
        cmd.val[1]=*pch;
    }
    cmd.tl1  = protocol::tail1;
    cmd.tl2  = protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);
}

void OperationControlDlg::initialWidget()
{

    ///sample rate
    lab_sample_rate=new QLabel(tr("<u>Sample Rate</u>"),this);
    lab_sample_rate->setFont(QFont("Times",15,QFont::Bold));
    lnedt_sample_rate=new QLineEdit(this);
    lab_SPS=new QLabel(tr("SPS"),this);
    lab_SMPL_PRD_contents=new QLabel(tr("SMPL_PRD Contents"),this);
    lnedit_SMPL_PRD_contents=new QLineEdit(this);
    lnedit_SMPL_PRD_contents->setDisabled(true);
    btn_sample_Update=new QPushButton(tr("Update"),this);

    ///Measurement Range and Digital Filtering
    lab_measurement=new QLabel(tr("<u>Measurement Range and Digital Filtering</u>"),this);
    lab_measurement->setFont(QFont("Times",15,QFont::Bold));
    lab_select_gyro_range=new QLabel(tr("Select Gyro Range"),this);
    rad_300_deg_sec=new QRadioButton(tr("300 deg/sec"),this);
    rad_150_deg_sec=new QRadioButton(tr("150 deg/sec"),this);
    rad_75_deg_sec=new QRadioButton(tr("75 deg/sec"),this);
    btn_set_dynamic=new QPushButton(tr("Set"),this);
    bgp_measure_range=new QButtonGroup(this);
    bgp_measure_range->addButton(rad_300_deg_sec,1);
    bgp_measure_range->addButton(rad_150_deg_sec,2);
    bgp_measure_range->addButton(rad_75_deg_sec,3);

    lnedit_measure_taps=new QLineEdit(this);
    lab_tabs=new QLabel(tr("Taps"),this);
    lab_SENS_AVG_contents=new QLabel(tr("SENS/AVG Contents"),this);
    lnedit_SENS_AVG_contents=new QLineEdit(this);
    lnedit_SENS_AVG_contents->setDisabled(true);
    btn_measure_update=new QPushButton(tr("Update"),this);

    ///Auxilliary Digital I/O Configuration
    lab_Auxilliary_digital_cfg=new QLabel(tr("<u>Auxilliary Digital I/O Configuration</u>"),this);
    lab_Auxilliary_digital_cfg->setFont(QFont("Times",15,QFont::Bold));

    //configure as general
    lab_configure_as_general=new QLabel(tr("<u>Configure as a general purpose I/O line</u>"),this);
    lab_configure_as_general->setFont(QFont("Times",13,QFont::Bold));

    lab_digital_IO_line0=new QLabel(tr("Digital I/O Line 0:"),this);
    rad_io0_input=new QRadioButton(tr("Input"),this);
    rad_io0_output=new QRadioButton(tr("Output"),this);
    bgp_io0_digital=new QButtonGroup(this);
    bgp_io0_digital->addButton(rad_io0_input,1);
    bgp_io0_digital->addButton(rad_io0_output,2);

    lab_set_line=new QLabel(tr("Set Line 0 Level"),this);
    rad_set_line_high=new QRadioButton(tr("High"),this);
    rad_set_line_low=new QRadioButton(tr("Low"),this);
    bgp_set_line=new QButtonGroup(this);
    bgp_set_line->addButton(rad_set_line_high,1);
    bgp_set_line->addButton(rad_set_line_low,2);

    lab_digital_IO_line1=new QLabel(tr("Digital I/O Line 1"),this);
    rad_io1_input=new QRadioButton(tr("Input"),this);
    rad_io1_output=new QRadioButton(tr("Output"),this);
    bgp_io1_digital=new QButtonGroup(this);
    bgp_io1_digital->addButton(rad_io1_input,1);
    bgp_io1_digital->addButton(rad_io1_output,2);

    lab_read_line=new QLabel(tr("Read Line 1 Level"),this);
    rad_read_line_high=new QRadioButton(tr("High"),this);
    rad_read_line_low=new QRadioButton(tr("Low"),this);
    bgp_read_line=new QButtonGroup(this);
    bgp_read_line->addButton(rad_read_line_high,1);
    bgp_read_line->addButton(rad_read_line_low,2);

    //configure as a data ready line
    lab_configure_as_data=new QLabel(tr("<u>Configure as a data ready line</u>"),this);
    lab_configure_as_data->setFont(QFont("Times",13,QFont::Bold));

    lab_select_io_line=new QLabel(tr("Select I/O Line"),this);
    rad_D1_o0=new QRadioButton(tr("D1/O0"),this);
    rad_D1_o1=new QRadioButton(tr("D1/O1"),this);
    bgp_select_io_line=new QButtonGroup(this);
    bgp_select_io_line->addButton(rad_D1_o0,1);
    bgp_select_io_line->addButton(rad_D1_o1,2);

    lab_output_polarity=new QLabel(tr("Output Polarity"),this);
    rad_output_polarity_high=new QRadioButton(tr("High"),this);
    rad_output_polarity_low=new QRadioButton(tr("Low"),this);
    bgp_output_polarity=new QButtonGroup(this);
    bgp_output_polarity->addButton(rad_output_polarity_high,1);
    bgp_output_polarity->addButton(rad_output_polarity_low,2);

    lab_enable=new QLabel(tr("Enable"),this);
    rad_enable_on=new QRadioButton(tr("ON"),this);
    rad_enable_off=new QRadioButton(tr("OFF"),this);
    bgp_enable=new QButtonGroup(this);
    bgp_enable->addButton(rad_enable_on,1);
    bgp_enable->addButton(rad_enable_off,2);

    ///Auxilliary D/A Converter Output
    lab_Auxilliary_da_converter=new QLabel(tr("<u>Auxilliary D/A Converter Output</u>"),this);
    lab_Auxilliary_da_converter->setFont(QFont("Times",15,QFont::Bold));
    lnedit_volts=new QLineEdit(this);
    lab_volts=new QLabel(tr("Volts"),this);
    lab_AUX_DAC_contents=new QLabel(tr("AUX_DAC Contents"),this);
    lnedit_AUX_DAC_contents=new QLineEdit(this);
    lnedit_AUX_DAC_contents->setDisabled(true);
    btn_auxilliary_update=new QPushButton(tr("Update"),this);

    ///buttom
    btn_close_windows=new QPushButton(tr("Close Windows"),this);
    btn_flash_memory=new QPushButton(tr("Flash Memory\nRegister Update"),this);

    ///layout;
    //sample rate layout
    QHBoxLayout* lyt_hbox_sample=new QHBoxLayout;
    lyt_hbox_sample->addWidget(lnedt_sample_rate);
    lyt_hbox_sample->addWidget(lab_SPS);
    lyt_hbox_sample->addStretch();
    lyt_hbox_sample->addWidget(lab_SMPL_PRD_contents);
    lyt_hbox_sample->addWidget(lnedit_SMPL_PRD_contents);
    lyt_hbox_sample->addStretch();
    lyt_hbox_sample->addWidget(btn_sample_Update);

    QVBoxLayout* lyt_vbox_sample_total=new QVBoxLayout;
    lyt_vbox_sample_total->addWidget(lab_sample_rate);
    lyt_vbox_sample_total->addLayout(lyt_hbox_sample);

    //Measurement Range
#ifdef MEASURE_RANGE_GRID_LAYOUT
    QGridLayout* lyt_grid_measure=new QGridLayout;
    lyt_grid_measure->addWidget(lab_select_gyro_range,0,0);
    lyt_grid_measure->addWidget(rad_300_deg_sec,0,2);
    lyt_grid_measure->addWidget(rad_150_deg_sec,0,3);
    lyt_grid_measure->addWidget(rad_75_deg_sec,0,4);

    lyt_grid_measure->addWidget(lnedit_measure_taps,1,0);
    lyt_grid_measure->addWidget(lab_tabs,1,1);
    lyt_grid_measure->addWidget(lab_SENS_AVG_contents,1,2);
    lyt_grid_measure->addWidget(lnedit_SENS_AVG_contents,1,3);
    lyt_grid_measure->addWidget(btn_measure_update,1,4);
#else
    QHBoxLayout* lyt_hbox_measure_select=new QHBoxLayout;
    lyt_hbox_measure_select->addWidget(lab_select_gyro_range);
    lyt_hbox_measure_select->addStretch();
    lyt_hbox_measure_select->addWidget(rad_300_deg_sec);
    lyt_hbox_measure_select->addWidget(rad_150_deg_sec);
    lyt_hbox_measure_select->addWidget(rad_75_deg_sec);
    lyt_hbox_measure_select->addStretch();
    lyt_hbox_measure_select->addWidget(btn_set_dynamic);

    QHBoxLayout* lyt_hbox_measure_tabs=new QHBoxLayout;
    lyt_hbox_measure_tabs->addWidget(lnedit_measure_taps);
    lyt_hbox_measure_tabs->addWidget(lab_tabs);
    lyt_hbox_measure_tabs->addStretch();
    lyt_hbox_measure_tabs->addWidget(lab_SENS_AVG_contents);
    lyt_hbox_measure_tabs->addWidget(lnedit_SENS_AVG_contents);
    lyt_hbox_measure_tabs->addStretch();
    lyt_hbox_measure_tabs->addWidget(btn_measure_update);
#endif

    QVBoxLayout* lyt_vbox_measure_total=new QVBoxLayout;
    lyt_vbox_measure_total->addWidget(lab_measurement);
#ifdef MEASURE_RANGE_GRID_LAYOUT
    lyt_vbox_measure_total->addLayout(lyt_grid_measure);
#else
    lyt_vbox_measure_total->addLayout(lyt_hbox_measure_select);
    lyt_vbox_measure_total->addLayout(lyt_hbox_measure_tabs);
#endif

    //Auxilliary Digital I/O Configuration
#define AUXILLIARY_DIGITAL_GRID_LAYOUT
#ifdef AUXILLIARY_DIGITAL_GRID_LAYOUT
    QGridLayout* lyt_grid_digital=new QGridLayout;
    lyt_grid_digital->addWidget(lab_digital_IO_line0,0,0);
    lyt_grid_digital->addWidget(rad_io0_input,0,1);
    lyt_grid_digital->addWidget(rad_io0_output,0,2);
    lyt_grid_digital->addWidget(lab_set_line,0,4);
    lyt_grid_digital->addWidget(rad_set_line_high,0,5);
    lyt_grid_digital->addWidget(rad_set_line_low,0,6);

    lyt_grid_digital->addWidget(lab_digital_IO_line1,1,0);
    lyt_grid_digital->addWidget(rad_io1_input,1,1);
    lyt_grid_digital->addWidget(rad_io1_output,1,2);
    lyt_grid_digital->addWidget(lab_read_line,1,4);
    lyt_grid_digital->addWidget(rad_read_line_high,1,5);
    lyt_grid_digital->addWidget(rad_read_line_low,1,6);

    QGridLayout* lyt_grid_select=new QGridLayout;
    lyt_grid_select->addWidget(lab_select_io_line,0,0);
    lyt_grid_select->addWidget(rad_D1_o0,0,1);
    lyt_grid_select->addWidget(rad_D1_o1,0,2);
    lyt_grid_select->addWidget(lab_output_polarity,0,4);
    lyt_grid_select->addWidget(rad_output_polarity_high,0,5);
    lyt_grid_select->addWidget(rad_output_polarity_low,0,6);

    lyt_grid_select->addWidget(lab_enable,1,0);
    lyt_grid_select->addWidget(rad_enable_on,1,1);
    lyt_grid_select->addWidget(rad_enable_off,1,2);
#else
    QHBoxLayout* lyt_hbox_digital_0=new QHBoxLayout;
    lyt_hbox_digital_0->addWidget(lab_digital_IO_line0);
    lyt_hbox_digital_0->addWidget(rad_io0_input);
    lyt_hbox_digital_0->addWidget(rad_io0_output);
    lyt_hbox_digital_0->addStretch();
    lyt_hbox_digital_0->addWidget(lab_set_line);
    lyt_hbox_digital_0->addWidget(rad_set_line_high);
    lyt_hbox_digital_0->addWidget(rad_set_line_low);

    QHBoxLayout* lyt_hbox_digital_1=new QHBoxLayout;
    lyt_hbox_digital_1->addWidget(lab_digital_IO_line1);
    lyt_hbox_digital_1->addWidget(rad_io1_input);
    lyt_hbox_digital_1->addWidget(rad_io1_output);
    lyt_hbox_digital_1->addStretch();
    lyt_hbox_digital_1->addWidget(lab_read_line);
    lyt_hbox_digital_1->addWidget(rad_read_line_high);
    lyt_hbox_digital_1->addWidget(rad_read_line_low);

    QHBoxLayout* lyt_hbox_select=new QHBoxLayout;
    lyt_hbox_select->addWidget(lab_select_io_line);
    lyt_hbox_select->addWidget(rad_D1_o0);
    lyt_hbox_select->addWidget(rad_D1_o1);
    lyt_hbox_select->addStretch();
    lyt_hbox_select->addWidget(lab_output_polarity);
    lyt_hbox_select->addWidget(rad_output_polarity_high);
    lyt_hbox_select->addWidget(rad_output_polarity_low);

    QHBoxLayout* lyt_hbox_enable=new QHBoxLayout;
    lyt_hbox_enable->addWidget(lab_enable);
    lyt_hbox_enable->addWidget(rad_enable_on);
    lyt_hbox_enable->addWidget(rad_enable_off);
    lyt_hbox_enable->addStretch();
#endif

    QVBoxLayout* lyt_vbox_auxilliary_total=new QVBoxLayout;
    lyt_vbox_auxilliary_total->addWidget(lab_Auxilliary_digital_cfg);
    lyt_vbox_auxilliary_total->addWidget(lab_configure_as_general);

#ifdef AUXILLIARY_DIGITAL_GRID_LAYOUT
    lyt_vbox_auxilliary_total->addLayout(lyt_grid_digital);
#else
    lyt_vbox_auxilliary_total->addLayout(lyt_hbox_digital_0);
    lyt_vbox_auxilliary_total->addLayout(lyt_hbox_digital_1);
#endif

    lyt_vbox_auxilliary_total->addWidget(lab_configure_as_data);
#ifdef AUXILLIARY_DIGITAL_GRID_LAYOUT
    lyt_vbox_auxilliary_total->addLayout(lyt_grid_select);
#else
    lyt_vbox_auxilliary_total->addLayout(lyt_hbox_select);
    lyt_vbox_auxilliary_total->addLayout(lyt_hbox_enable);
#endif

    //Auxilliary D/A Converter
    QHBoxLayout* lyt_hbox_converter=new QHBoxLayout;
    lyt_hbox_converter->addWidget(lnedit_volts);
    lyt_hbox_converter->addWidget(lab_volts);
    lyt_hbox_converter->addStretch();
    lyt_hbox_converter->addWidget(lab_AUX_DAC_contents);
    lyt_hbox_converter->addWidget(lnedit_AUX_DAC_contents);
    lyt_hbox_converter->addStretch();
    lyt_hbox_converter->addWidget(btn_auxilliary_update);

    QVBoxLayout* lyt_vbox_converter_total=new QVBoxLayout;
    lyt_vbox_converter_total->addWidget(lab_Auxilliary_da_converter);
    lyt_vbox_converter_total->addLayout(lyt_hbox_converter);

    //buttons
    QHBoxLayout* lyt_hbox_2_btm_total=new QHBoxLayout;
    lyt_hbox_2_btm_total->addStretch();
    lyt_hbox_2_btm_total->addWidget(btn_close_windows);
    lyt_hbox_2_btm_total->addWidget(btn_flash_memory);
    btn_close_windows->setMinimumHeight(45);
    btn_flash_memory->setMinimumHeight(45);

    lyt_total=new QVBoxLayout;
    lyt_total->addLayout(lyt_vbox_sample_total);
    lyt_total->addStretch();
    lyt_total->addLayout(lyt_vbox_measure_total);
    lyt_total->addStretch();
    lyt_total->addLayout(lyt_vbox_auxilliary_total);
    lyt_total->addStretch();
    lyt_total->addLayout(lyt_vbox_converter_total);
    lyt_total->addStretch();
    lyt_total->addLayout(lyt_hbox_2_btm_total);

    setLayout(lyt_total);
}

