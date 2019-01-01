#include <QtGui>
#include <QMessageBox>
#include "calibrationdlg.h"

CalibrationDlg::CalibrationDlg(QWidget *parent)
    :QDialog(parent)
{
    //setAttribute(Qt::WA_DeleteOnClose,true);
    initialWidget();

    // -50~50,小数点后4位
    QRegExp regExp("^((0|(-?[1-9])|(-?50)|(-?[1-4]\\d))((\\.\\d{1,4})?))$");
    lnedit_x_offset->setValidator(new QRegExpValidator(regExp, this));
    lnedit_y_offset->setValidator(new QRegExpValidator(regExp, this));
    lnedit_z_offset->setValidator(new QRegExpValidator(regExp, this));

    // -6700~6700, integer
    QRegExp r("^0|(-?[1-9]\\d{0,2})|(-?6700)|(-?6[0-6]\\d{2})|(-?[1-5]\\d{3})$");
    lnedit_acc_x_offset->setValidator(new QRegExpValidator(r,this));
    lnedit_acc_y_offset->setValidator(new QRegExpValidator(r,this));
    lnedit_acc_z_offset->setValidator(new QRegExpValidator(r,this));

    // connect
    connect(btn_restore_run,&QPushButton::clicked,this,&CalibrationDlg::restoreRunSlot);
    connect(btn_precision_run,&QPushButton::clicked,this,&CalibrationDlg::precisionRunSlot);
    connect(btn_auto_run,&QPushButton::clicked,this,&CalibrationDlg::autoRunSlot);

    connect(btn_x_off_update,&QPushButton::clicked,this,&CalibrationDlg::xUpdateSlot);
    connect(btn_y_off_update,&QPushButton::clicked,this,&CalibrationDlg::yUpdateSlot);
    connect(btn_z_off_update,&QPushButton::clicked,this,&CalibrationDlg::zUpdateSlot);

    connect(btn_acc_x_off_update,&QPushButton::clicked,this,&CalibrationDlg::xAccUpdateSlot);
    connect(btn_acc_y_off_update,&QPushButton::clicked,this,&CalibrationDlg::yAccUpdateSlot);
    connect(btn_acc_z_off_update,&QPushButton::clicked,this,&CalibrationDlg::zAccUpdateSlot);

    connect(btn_close_window,&QPushButton::clicked,this,&CalibrationDlg::closeWindowSolt);
    connect(btn_flash_memory_update,&QPushButton::clicked,this,&CalibrationDlg::flashUpdateSlot);

    setLayout(total_layout);
    setWindowTitle(tr("Calibration"));
    resize(450,500);
}

CalibrationDlg::~CalibrationDlg()
{
    // Automatic Features
    delete lab_automatic_features;

    delete lab_restore_factory;
    delete lab_precision_auto_null;
    delete lab_auto_null;
    delete btn_restore_run;
    delete btn_precision_run;
    delete btn_auto_run;

    // Manual Calibration
    delete lab_manual_calibration;

    delete lab_gyroscope;
    delete lab_regisnter_content;

    delete lab_x_offset;
    delete lnedit_x_offset;
    delete lab_x_deg_sec;
    delete btn_x_off_update;
    delete lnedit_x_content;

    delete lab_y_offset;
    delete lnedit_y_offset;
    delete lab_y_deg_sec;
    delete btn_y_off_update;
    delete lnedit_y_content;

    delete lab_z_offset;
    delete lnedit_z_offset;
    delete lab_z_deg_sec;
    delete btn_z_off_update;
    delete lnedit_z_content;

    //Acceleration
    delete lab_acceration;

    delete lab_acc_x_offset;
    delete lnedit_acc_x_offset;
    delete lab_acc_x_g;
    delete btn_acc_x_off_update;
    delete lnedit_acc_x_content;

    delete lab_acc_y_offset;
    delete lnedit_acc_y_offset;
    delete lab_acc_y_g;
    delete btn_acc_y_off_update;
    delete lnedit_acc_y_content;

    delete lab_acc_z_offset;
    delete lnedit_acc_z_offset;
    delete lab_acc_z_g;
    delete btn_acc_z_off_update;
    delete lnedit_acc_z_content;

    //button
    delete btn_close_window;
    delete btn_flash_memory_update;

    delete total_layout;
}

void CalibrationDlg::initialWidget()
{
    // Automatic Features
    lab_automatic_features =new QLabel(tr("<u>Automatic Features</u>"));
    lab_automatic_features->setFont(QFont("Times",15,QFont::Bold));

    lab_restore_factory =new QLabel(tr("Restore Factory Calibration"));
    lab_precision_auto_null =new QLabel(tr("Precision Auto Null"));
    lab_auto_null =new QLabel(tr("Auto Null"));

    /*
     QVBoxLayout * autolyt_left = new QVBoxLayout;
     autolyt_left->addStretch();
     autolyt_left->addWidget(lab_restore_factory);
     autolyt_left->addWidget(lab_precision_auto_null);
     autolyt_left->addWidget(lab_auto_null);
     autolyt_left->addStretch();
     setLayout( autolyt_left);
    */

    btn_restore_run =new QPushButton(tr("Run"),this);
    btn_precision_run =new QPushButton(tr("Run"),this);
    btn_auto_run =new QPushButton(tr("Run"),this);

    /*
    QVBoxLayout* autolyt_right = QVBoxLayout;
    autolyt_right->addStretch();
    autolyt_right->addWidget(btn_restore_run);
    autolyt_right->addWidget(btn_precison_run);
    autolyt_right->addWidget(btn_auto_run);
    */

    QGridLayout* autogrid_lyt=new QGridLayout;
    autogrid_lyt->addWidget(lab_restore_factory,0,0);
    autogrid_lyt->addWidget(lab_precision_auto_null,1,0);
    autogrid_lyt->addWidget(lab_auto_null,2,0);

    autogrid_lyt->addWidget(btn_restore_run,0,1);
    autogrid_lyt->addWidget(btn_precision_run,1,1);
    autogrid_lyt->addWidget(btn_auto_run,2,1);

    QVBoxLayout* lyt_auto_total=new QVBoxLayout;
    lyt_auto_total->addWidget(lab_automatic_features);
    lyt_auto_total->addLayout(autogrid_lyt);
    autogrid_lyt->setParent(lyt_auto_total);

    // Manual Calibration
    lab_manual_calibration=new QLabel(tr("<u>Manual Calibration Adjustment</u>"));
    lab_manual_calibration->setFont(QFont("Times",15,QFont::Bold));

    lab_gyroscope=new QLabel(tr("Gyroscopes"));
    lab_gyroscope->setFont(QFont("Times",13,QFont::Bold));

    lab_regisnter_content=new QLabel(tr("Regisnter\nContents"));
    lab_regisnter_content->setFont(QFont("Times",13,QFont::Bold));

    lab_x_offset=new QLabel(tr("X-Axis Offset"));
    lnedit_x_offset=new QLineEdit;
    lab_x_deg_sec=new QLabel(tr("deg/sec"));
    btn_x_off_update=new QPushButton(tr("Update"));
    lnedit_x_content=new QLineEdit;lnedit_x_content->setDisabled(true);

    lab_y_offset=new QLabel(tr("Y-Axis Offset"));
    lnedit_y_offset=new QLineEdit;
    lab_y_deg_sec=new QLabel(tr("deg/sec"));
    btn_y_off_update=new QPushButton(tr("Update"));
    lnedit_y_content=new QLineEdit;lnedit_y_content->setDisabled(true);

    lab_z_offset=new QLabel(tr("Z-Axis Offset"));
    lnedit_z_offset=new QLineEdit;
    lab_z_deg_sec=new QLabel(tr("deg/sec"));
    btn_z_off_update=new QPushButton(tr("Update"));
    lnedit_z_content=new QLineEdit;lnedit_z_content->setDisabled(true);

    //Acceleration
    lab_acceration=new QLabel(tr("Acceleration"));
    lab_acceration->setFont(QFont("Times",13,QFont::Bold));

    lab_acc_x_offset=new QLabel(tr("X-Axis Offset"));
    lnedit_acc_x_offset=new QLineEdit;
    lab_acc_x_g=new QLabel(tr("mg"));
    btn_acc_x_off_update=new QPushButton(tr("Update"));
    lnedit_acc_x_content=new QLineEdit;lnedit_acc_x_content->setDisabled(true);

    lab_acc_y_offset=new QLabel(tr("Y-Axis Offset"));
    lnedit_acc_y_offset=new QLineEdit;
    lab_acc_y_g=new QLabel(tr("mg"));
    btn_acc_y_off_update=new QPushButton(tr("Update"));
    lnedit_acc_y_content=new QLineEdit;lnedit_acc_y_content->setDisabled(true);

    lab_acc_z_offset=new QLabel(tr("Z-Axis Offset"));
    lnedit_acc_z_offset=new QLineEdit;
    lab_acc_z_g=new QLabel(tr("mg"));
    btn_acc_z_off_update=new QPushButton(tr("Update"));
    lnedit_acc_z_content=new QLineEdit;lnedit_acc_z_content->setDisabled(true);

    QGridLayout* mangrid_lyt=new QGridLayout;
    mangrid_lyt->addWidget(lab_gyroscope,0,0);
    mangrid_lyt->addWidget(lab_regisnter_content,0,4);

    mangrid_lyt->addWidget(lab_x_offset,1,0);
    mangrid_lyt->addWidget(lnedit_x_offset,1,1);
    mangrid_lyt->addWidget(lab_x_deg_sec,1,2);
    mangrid_lyt->addWidget(btn_x_off_update,1,3);
    mangrid_lyt->addWidget(lnedit_x_content,1,4);

    mangrid_lyt->addWidget(lab_y_offset,2,0);
    mangrid_lyt->addWidget(lnedit_y_offset,2,1);
    mangrid_lyt->addWidget(lab_y_deg_sec,2,2);
    mangrid_lyt->addWidget(btn_y_off_update,2,3);
    mangrid_lyt->addWidget(lnedit_y_content,2,4);

    mangrid_lyt->addWidget(lab_z_offset,3,0);
    mangrid_lyt->addWidget(lnedit_z_offset,3,1);
    mangrid_lyt->addWidget(lab_z_deg_sec,3,2);
    mangrid_lyt->addWidget(btn_z_off_update,3,3);
    mangrid_lyt->addWidget(lnedit_z_content,3,4);

    mangrid_lyt->addWidget(lab_acceration,4,0);

    mangrid_lyt->addWidget(lab_acc_x_offset,5,0);
    mangrid_lyt->addWidget(lnedit_acc_x_offset,5,1);
    mangrid_lyt->addWidget(lab_acc_x_g,5,2);
    mangrid_lyt->addWidget(btn_acc_x_off_update,5,3);
    mangrid_lyt->addWidget(lnedit_acc_x_content,5,4);

    mangrid_lyt->addWidget(lab_acc_y_offset,6,0);
    mangrid_lyt->addWidget(lnedit_acc_y_offset,6,1);
    mangrid_lyt->addWidget(lab_acc_y_g,6,2);
    mangrid_lyt->addWidget(btn_acc_y_off_update,6,3);
    mangrid_lyt->addWidget(lnedit_acc_y_content,6,4);

    mangrid_lyt->addWidget(lab_acc_z_offset,7,0);
    mangrid_lyt->addWidget(lnedit_acc_z_offset,7,1);
    mangrid_lyt->addWidget(lab_acc_z_g,7,2);
    mangrid_lyt->addWidget(btn_acc_z_off_update,7,3);
    mangrid_lyt->addWidget(lnedit_acc_z_content,7,4);

    QVBoxLayout* man_total_lyt=new QVBoxLayout;
    man_total_lyt->addWidget(lab_manual_calibration);
    man_total_lyt->addLayout(mangrid_lyt);

    //button
    btn_close_window=new QPushButton(tr("Close Window"));
    btn_flash_memory_update=new QPushButton(tr("Flash Memory\nRegister Update"));

    QHBoxLayout* bottom_2btn_lyt=new QHBoxLayout;
    bottom_2btn_lyt->addStretch();
    bottom_2btn_lyt->addWidget(btn_close_window);
    bottom_2btn_lyt->addWidget(btn_flash_memory_update);
    btn_close_window->setMinimumHeight(40);
    btn_flash_memory_update->setMinimumHeight(40);

    //total layout
    total_layout=new QVBoxLayout;
    total_layout->addLayout(lyt_auto_total);
    total_layout->addStretch();
    total_layout->addLayout(man_total_lyt);
    total_layout->addStretch();
    total_layout->addLayout(bottom_2btn_lyt);
}

/*!
 * \brief CalibrationDlg::updateContent
 * update the content in the QLineEdit of the register
 * \param          addr      ch
 * XGYRO_OFF  ==== 0x1A === 0x01
 * YGYRO_OFF  ==== 0x1C === 0x02
 * ZGYRO_OFF  ==== 0x1E === 0x03
 *
 * XACCL_OFF  ==== 0x20 === 0x04
 * YACCL_OFF  ==== 0x22 === 0x05
 * ZACCL_OFF  ==== 0x24 === 0x06
 * flash write          === 0x07
 */
void CalibrationDlg::updateContent(uint8_t ch)
{
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head = protocol::master;
    cmd.op   = protocol::get;
    cmd.cmdf = protocol::manual_bias_cal;
    cmd.name = ch;
    cmd.tl1  = protocol::tail1;
    cmd.tl2  = protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);
}

/*!
 * \brief CalibrationDlg::manualBiasSet
 * send the package of bias data
 * \param ch
 * register address of
 * \param v
 * the parameter of bias float stored in 4 char
 */
void CalibrationDlg::manualBiasSet(uint8_t ch, float flo=0.0)
{
    char *pch=reinterpret_cast<char*>(&flo);
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head  = protocol::master;
    cmd.op    = protocol::set;
    cmd.cmdf  = protocol::manual_bias_cal;
    cmd.name  = ch;
    cmd.val[0]=*pch;pch++;
    cmd.val[1]=*pch;pch++;
    cmd.val[2]=*pch;pch++;
    cmd.val[3]=*pch;
    cmd.tl1   = protocol::tail1;
    cmd.tl2   = protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);
}

void CalibrationDlg::restoreRunSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head = protocol::master;
    cmd.op   = protocol::set;
    cmd.cmdf = protocol::restore_fctry_cal;
    cmd.tl1  = protocol::tail1;
    cmd.tl2  = protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);
}

void CalibrationDlg::precisionRunSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head = protocol::master;
    cmd.op   = protocol::set;
    cmd.cmdf = protocol::precsn_bias_cal;
    cmd.tl1  = protocol::tail1;
    cmd.tl2  = protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);
    QMessageBox::information(this,"","Precision Auto Null is been conducted.\n Pleade wait 35 seconds.");
}

void CalibrationDlg::autoRunSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head = protocol::master;
    cmd.op   = protocol::set;
    cmd.cmdf = protocol::auto_bias_null_cal;
    cmd.tl1  = protocol::tail1;
    cmd.tl2  = protocol::tail2;
    protocol::calCheckSum(cmd);
    emit configWrite_signal(cmd);
}

void CalibrationDlg::xUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString str=lnedit_x_offset->text();
    if(str.isEmpty()){
        QMessageBox::information(this,"X axis gyro update","Input can't empty.");
        return;
    }
    float flo=str.toFloat();
    manualBiasSet(0x01,flo);
}

void CalibrationDlg::yUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString str=lnedit_y_offset->text();
    if(str.isEmpty())
        return;
    float flo=str.toFloat();
    manualBiasSet(0x02,flo);
}

void CalibrationDlg::zUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString str=lnedit_z_offset->text();
    if(str.isEmpty())
        return;
    float flo=str.toFloat();
    qDebug("%f",flo);
    manualBiasSet(0x03,flo);
}

void CalibrationDlg::xAccUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString str=lnedit_acc_x_offset->text();
    if(str.isEmpty())
        return;
    float flo=str.toFloat();
    manualBiasSet(0x04,flo);
}

void CalibrationDlg::yAccUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString str=lnedit_acc_y_offset->text();
    if(str.isEmpty())
        return;
    float flo=str.toFloat();
    manualBiasSet(0x05,flo);
}

void CalibrationDlg::zAccUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString str=lnedit_acc_z_offset->text();
    if(str.isEmpty())
        return;
    float flo=str.toFloat();
    manualBiasSet(0x06,flo);
}

void CalibrationDlg::closeWindowSolt()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    this->close();
}

void CalibrationDlg::flashUpdateSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    manualBiasSet(0x07);
}

void CalibrationDlg::configRes_slot(const protocol::CfgData &d)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    if(protocol::checkSum(d)==false){
        qDebug()<<"checkSum:false.";
        return;
    }

    if(d.cmdf==protocol::restore_fctry_cal){
        QMessageBox::information(this,"Calibration Result","Restore factory calibration success.");
        return;
    }

    if(d.cmdf==protocol::precsn_bias_cal){
        QMessageBox::information(this,"Calibration Result","Precision auto null calibration success.");
        return;
    }

    if(d.cmdf==protocol::auto_bias_null_cal){
        QMessageBox::information(this,"Calibration Result","Auto bias null calibration success.");
        return;
    }

    if(d.cmdf==protocol::manual_bias_cal){
        uint8_t buff[4]={d.val[0],d.val[1],d.val[2],d.val[3]};
        float flo = *reinterpret_cast<float*>(buff);
        double dou = static_cast<double>(flo);
        QString str=QString("%1").arg(dou,0,'f',3);

        switch (d.name) {
        case 0x01:
            lnedit_x_content->setText(str);
            break;
        case 0x02:
            lnedit_y_content->setText(str);
            break;
        case 0x03:
            lnedit_z_content->setText(str);
            break;
        case 0x04:
            lnedit_acc_x_content->setText(str);
            break;
        case 0x05:
            lnedit_acc_y_content->setText(str);
            break;
        case 0x06:
            lnedit_acc_z_content->setText(str);
            break;
        case 0x07:
            QMessageBox::information(this,"Flash write","Write flash successfully.");
            return;
        default:
            break;
        }// end of switch

    }

}
