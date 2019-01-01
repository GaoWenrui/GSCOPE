#include <QtGui>
#include "datalogctrldlg.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

DatalogCtrlDlg::DatalogCtrlDlg(QWidget *parent) : QDialog(parent)
{
    pfile=NULL;
    delaySampleVec=std::vector<bool>(11,false);
    initialWidget();

    sampleTimes=100;
    delayTimeMs=1000;
    lnedit_samples_per_file->setText(QString("%1").arg(sampleTimes));
    lnedit_sample_delay_msec->setText(QString("%1").arg(delayTimeMs));
    delayTimer.setInterval(delayTimeMs);

    cbx_x_gyro->setChecked(true);
    cbx_y_gyro->setChecked(true);
    cbx_z_gyro->setChecked(true);
    cbx_x_accel->setChecked(true);
    cbx_y_accel->setChecked(true);
    cbx_z_accel->setChecked(true);

    QRegExp exp("[1-9]\\d{0,5}");
    lnedit_samples_per_file->setValidator(new QRegExpValidator(exp,this));
    QRegExp r("^0|([1-9]\\d{0,5})$");
    lnedit_sample_delay_msec->setValidator(new QRegExpValidator(r,this));

    connect(btn_start_datalog,&QPushButton::clicked,this,&DatalogCtrlDlg::startDatalogSlot);
    connect(btn_stop_datalog,&QPushButton::clicked,this,&DatalogCtrlDlg::stopDatalogSlot);
    connect(btn_file_path,&QPushButton::clicked,this,&DatalogCtrlDlg::getFilePathSlot);
    connect(&delayTimer,&QTimer::timeout,this,&DatalogCtrlDlg::delaySampleSlot);

    resize(350,350);
    this->setWindowTitle("Datalog Control");
}

DatalogCtrlDlg::~DatalogCtrlDlg()
{
    if(pfile!=NULL){
        if(pfile->isOpen()){
            pfile->close();
        }
        delete pfile;
    }
    delete lyt_total_pls_stusbar;
}

void DatalogCtrlDlg::closeEvent(QCloseEvent *event)
{
    if(delayTimer.isActive()){
        QMessageBox::StandardButton button;
        button=QMessageBox::information(this,"Stop Sampling","The device is sampling, confirm to stop?",
                                 QMessageBox::Ok|QMessageBox::Cancel);
        if(button==QMessageBox::Ok){
            event->accept();
        }else{
            event->ignore();
        }

    }
}

void DatalogCtrlDlg::resolveData_slot(const protocol::CfgData &d)
{
     qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
     if(!protocol::checkSum(d)){
         return;
     }

     if(d.cmdf!=0x01) return;
     if(d.name<0x60 || d.name>0x70) return;
     QString strarray[11]={"Power Supply:",
                          "X Gyro:","Y Gyro:","Z Gyro:",
                          "X Accel:","Y Accel:","Z Accel:",
                          "X Tempe:","Y Tempe:","Z Tempe:",
                          "AUX ADC:"};
     QString item=strarray[d.name-0x60];
     uint8_t buff[4]={d.val[0],d.val[1],d.val[2],d.val[3]};
     float flodata=*reinterpret_cast<float*>(buff);
     QTextStream out(pfile);
     out<<item<<QString("%1").arg(flodata,0,'g')<<"\n";
}

void DatalogCtrlDlg::startDatalogSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);

    QString times=lnedit_samples_per_file->text();
    bool ok;
    sampleTimes=times.toInt(&ok,10);
    if(!ok){
        QMessageBox::information(this,"Samples per File",
                                 "The input sample times is illeagal.");
        return;
    }

    QString delay=lnedit_sample_delay_msec->text();
    delayTimeMs=delay.toInt(&ok,10);
    if(!ok){
        QMessageBox::information(this,"Sample Delay msec",
                                 "Sample Delay msec is illeagal.");
        return;
    }

    // 文件路径
    QString filepath = lnedit_file_path->text();
    if(filepath.isEmpty()){
        QMessageBox::information(this,"File Path",
                                 "The file path is empty. Please select the file path.");
        return;
    }
    bool ed = filepath.endsWith(".txt",Qt::CaseInsensitive);
    if(!ed){
        filepath+=".txt";
    }

    // 打开文件
    pfile = new QFile(filepath);
    if(!pfile->open(QIODevice::WriteOnly | QIODevice::Text |QIODevice::Append)){
        QMessageBox::warning(this,tr("Save"),tr("Can not open file for writing."));
        delete pfile;
        pfile=NULL;
        return;
    }

    // the data logged begin with time
    QString d=QDate::currentDate().toString("yyyy-MM-dd ");
    QString t=QTime::currentTime().toString("hh:mm:ss");
    QTextStream out(pfile);
    out<<"\n"<<d<<t<<"\n";

    getCheckBoxVector();

    delayTimer.setInterval(delayTimeMs);
    QTimer::singleShot(1000*sampleTimes*delayTimeMs,this,&DatalogCtrlDlg::stopDatalogSlot);
    delayTimer.start();
    btn_start_datalog->setDisabled(true);
    status_bar->showMessage("The sample progress begins!",2000);
}

void DatalogCtrlDlg::stopDatalogSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    if(!delayTimer.isActive()) return;

    delayTimer.stop();
    btn_start_datalog->setEnabled(true);
    status_bar->showMessage("The sample progress has stopped!",2000);

    if(pfile!=NULL){
        if(pfile->isOpen()){
            pfile->close();
        }
        int ret = QMessageBox::information(this,tr("Save"),
                                           tr("The file has been saved as \"%1\"").arg(pfile->fileName()),
                                           tr("Show in folder"),
                                           tr("Ok"));
        if(ret==0){
            QFileInfo fi=QFileInfo(*pfile);
            QString filePath=fi.absolutePath();
            QDesktopServices::openUrl(QUrl(filePath, QUrl::TolerantMode));
        }
    }
}

void DatalogCtrlDlg::getFilePathSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString path = QFileDialog::getSaveFileName(this,tr("Save File Path"),
                    qApp->applicationDirPath()+"/untitled.txt",tr("Files(*.txt)"));
    if(path.isEmpty()){
        return;
    }
    lnedit_file_path->setText(path);
}

/*!
 * \brief DatalogCtrlDlg::delaySampleSlot
 * 0x60         power supply
 * 0x61~0x63    XYZ gyro
 * 0x64~0x66    XYZ Accel
 * 0x67~0x69    XYZ Tempe
 * 0x70         AUX ADC
 */
void DatalogCtrlDlg::delaySampleSlot()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    if(sampleTimes<=0){
        stopDatalogSlot();
        return;
    }
    for(int i=0;i<11;++i){
        if(delaySampleVec[i]){
            // 0x60 power supply, start char
            emit sendPkg(protocol::get,static_cast<char>(0x60+i));
        }
    }
    sampleTimes--;
}

void DatalogCtrlDlg::initialWidget()
{
    ///FILE SETUP
    lab_samples_per_file =new QLabel(tr("Samples per File"),this);
    lab_sample_delay_msec =new QLabel(tr("Sample Delay msec"),this);
    //lab_files_per_session =new QLabel(tr("Files per Session"),this);

    lnedit_samples_per_file =new QLineEdit(this);
    lnedit_sample_delay_msec =new QLineEdit(this);
    //lnedit_files_per_session =new QLineEdit(this);

    gpb_file_setup = new QGroupBox(tr("FILE SETUP"),this);

    ///FILE INFORMATION
//    lab_directory =new QLabel(tr("Directory"),this);
//    lab_file_name =new QLabel(tr("File Name"),this);
//    lab_file_num =new QLabel(tr("File Number"),this);
//    lab_txt =new QLabel(tr(".txt"),this);

//    lnedit_directory =new QLineEdit(this);
//    lnedit_file_name =new QLineEdit(this);
//    lnedit_file_num =new QLineEdit(this);
    lab_file_path = new QLabel(tr("File Path"),this);
    lnedit_file_path = new QLineEdit(this);
    btn_file_path = new QPushButton(tr("Path..."),this);

    gpb_file_information = new QGroupBox(tr("FILE INFORMATION"),this);

    ///DATA SELECTION
    cbx_power_supply =new QCheckBox(tr("Power Supply"),this);
    cbx_x_gyro =new QCheckBox(tr("X Gyro"),this);
    cbx_y_gyro =new QCheckBox(tr("Y Gyro"),this);
    cbx_z_gyro =new QCheckBox(tr("Z Gyro"),this);
    cbx_x_accel =new QCheckBox(tr("X Accel"),this);
    cbx_y_accel =new QCheckBox(tr("Y Accel"),this);
    cbx_z_accel =new QCheckBox(tr("Z Accel"),this);
    cbx_x_temper =new QCheckBox(tr("X Temperature"),this);
    cbx_y_temper =new QCheckBox(tr("Y Temperature"),this);
    cbx_z_temper =new QCheckBox(tr("Z Temperature"),this);
    cbx_aux_adc =new QCheckBox(tr("Aux Adc"),this);

    gpb_data_selection = new QGroupBox(tr("DATA SELECTION"),this);

    ///BUTTON
    btn_start_datalog = new QPushButton(tr("Start Datalog"),this);
    btn_stop_datalog = new QPushButton(tr("Stop Datalog"),this);

    lyt_total=new QHBoxLayout;

    ///layout
    QGridLayout* lyt_file_setup=new QGridLayout;
    lyt_file_setup->addWidget(lab_samples_per_file,0,0);
    lyt_file_setup->addWidget(lab_sample_delay_msec,1,0);
    //lyt_file_setup->addWidget(lab_files_per_session,2,0);

    lyt_file_setup->addWidget(lnedit_samples_per_file,0,1);
    lyt_file_setup->addWidget(lnedit_sample_delay_msec,1,1);
    //lyt_file_setup->addWidget(lnedit_files_per_session,2,1);

    gpb_file_setup->setLayout(lyt_file_setup);

    QGridLayout* lyt_file_information=new QGridLayout;
//    lyt_file_information->addWidget(lab_directory,0,0);
//    lyt_file_information->addWidget(lab_file_name,1,0);
//    lyt_file_information->addWidget(lab_file_num,2,0);
    lyt_file_information->addWidget(lab_file_path,0,0);

//    lyt_file_information->addWidget(lnedit_directory,0,1);
//    lyt_file_information->addWidget(lnedit_file_name,1,1);
    QHBoxLayout *lyt_file_num_fmt=new QHBoxLayout;
    lyt_file_num_fmt->addWidget(lnedit_file_path);
    lyt_file_num_fmt->addWidget(btn_file_path);
    lyt_file_information->addLayout(lyt_file_num_fmt,1,0);

    gpb_file_information->setLayout(lyt_file_information);

    QVBoxLayout* lyt_data_selection=new QVBoxLayout;
    lyt_data_selection->addWidget(cbx_power_supply);
    lyt_data_selection->addWidget(cbx_x_gyro);
    lyt_data_selection->addWidget(cbx_y_gyro);
    lyt_data_selection->addWidget(cbx_z_gyro);
    lyt_data_selection->addWidget(cbx_x_accel);
    lyt_data_selection->addWidget(cbx_y_accel);
    lyt_data_selection->addWidget(cbx_z_accel);
    lyt_data_selection->addWidget(cbx_x_temper);
    lyt_data_selection->addWidget(cbx_y_temper);
    lyt_data_selection->addWidget(cbx_z_temper);
    lyt_data_selection->addWidget(cbx_aux_adc);

    gpb_data_selection->setLayout(lyt_data_selection);

    QHBoxLayout* lyt_button=new QHBoxLayout;
    lyt_button->addStretch();
    lyt_button->addWidget(btn_start_datalog);
    lyt_button->addStretch();
    lyt_button->addWidget(btn_stop_datalog);
    lyt_button->addStretch();

    QVBoxLayout* lyt_left_half =new QVBoxLayout;
    lyt_left_half->addWidget(gpb_file_setup);
    lyt_left_half->addStretch();
    lyt_left_half->addWidget(gpb_file_information);
    lyt_left_half->addStretch();
    lyt_left_half->addLayout(lyt_button);
    lyt_left_half->addStretch();

    lyt_total->addLayout(lyt_left_half);
    lyt_total->addWidget(gpb_data_selection);

    status_bar = new QStatusBar(this);

    lyt_total_pls_stusbar= new QVBoxLayout;
    lyt_total_pls_stusbar->addLayout(lyt_total);
    lyt_total_pls_stusbar->addWidget(status_bar);

    lyt_total_pls_stusbar->setMargin(2);
    setLayout(lyt_total_pls_stusbar);
}

void DatalogCtrlDlg::getCheckBoxVector()
{
    delaySampleVec[0] = cbx_power_supply->isChecked();
    delaySampleVec[1] = cbx_x_gyro->isChecked();
    delaySampleVec[2] = cbx_y_gyro->isChecked();
    delaySampleVec[3] = cbx_z_gyro->isChecked();
    delaySampleVec[4] = cbx_x_accel->isChecked();
    delaySampleVec[5] = cbx_y_accel->isChecked();
    delaySampleVec[6] = cbx_z_accel->isChecked();
    delaySampleVec[7] = cbx_x_temper->isChecked();
    delaySampleVec[8] = cbx_y_temper->isChecked();
    delaySampleVec[9] = cbx_z_temper->isChecked();
    delaySampleVec[10] = cbx_aux_adc->isChecked();
}

void DatalogCtrlDlg::sendPkg(protocol::operation_t op, char ch, float fl)
{
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head = protocol::master;
    cmd.op   = op;
    cmd.cmdf = protocol::read_write_data;
    cmd.name = ch;
    if(op==protocol::set){
        char *pch=reinterpret_cast<char*>(&fl);
        cmd.val[0]=*pch;pch++;
        cmd.val[1]=*pch;pch++;
        cmd.val[2]=*pch;pch++;
        cmd.val[3]=*pch;
    }
    cmd.tl1  = protocol::tail1;
    cmd.tl2  = protocol::tail2;
    protocol::calCheckSum(cmd);
    emit queryData_signal(cmd);
}

