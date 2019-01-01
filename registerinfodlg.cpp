#include <QtGui>
#include <QComboBox>
#include <QMessageBox>
#include "registerinfodlg.h"
#include <QDebug>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>

const QString reg_str[36]={("")
                         ,("FLASH_CNT"),("SUPPLY_OUT"),("XGYRO_OUT"),("YGYRO_OUT"),("ZGYRO_OUT")
                         ,("XACCL_OUT"),("YACCL_OUT"),("ZACCL_OUT"),("XTEMP_OUT"),("YTEMP_OUT")
                         ,("ZTEMP_OUT"),("AUX_ADC"),("XGYRO_OFF"),("YGYRO_OFF"),("ZGYRO_OFF")
                         ,("XACCL_OFF"),("YACCL_OFF"),("ZACCL_OFF"),("ALM_MAG1"),("ALM_MAG2")
                         ,("ALM_SMPL1"),("ALM_SMPL2"),("ALM_CTRL"),("AUX_DAC"),("GPIO_CTRL")
                         ,("MSC_CTRL"),("SMPL_PRD"),("SENS_AVG"),("SLP_CNT"),("DIAG_STAT")
                         ,("GLOB_CMD"),("LOT_ID1"),("LOT_ID2"),("PROD_ID"),("SERIAL_NUM")};

const unsigned char ch_base_addr[36]={0xFF,0x00,0x02,0x04,0x06,0x08,
                                  0x0A,0x0C,0x0E,0x10,0x12,
                                  0x14,0x16,0x1A,0x1C,0x1E,
                                  0x20,0x22,0x24,0x26,0x28,
                                  0x2A,0x2C,0x2E,0x30,0x32,
                                  0x34,0x36,0x38,0x3A,0x3C,
                                  0x3E,0x52,0x54,0x56,0x58};

const char ch_is_read[36]={0,1,1,1,1,1,
                             1,1,1,1,1,
                             1,1,1,1,1,
                             1,1,1,1,1,
                             1,1,1,1,1,
                             1,1,1,0,1,
                             0,1,1,1,1};

const char ch_is_write[36]={0,0,0,0,0,0,
                              0,0,0,0,0,
                              0,0,1,1,1,
                              1,1,1,1,1,
                              1,1,1,1,1,
                              1,1,1,1,0,
                              1,0,0,0,0};

const int n_default_val[36]={0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
                                    0x0000,0x0000,0x0000,0x0000,0x0000,
                                    0x0000,0x0000,0x0000,0x0000,0x0000,
                                    0x0000,0x0000,0x0000,0x0000,0x0000,
                                    0x0000,0x0000,0x0000,0x0000,0x0000,
                                    0x0006,0x0001,0x0402,0x0000,0x0000,
                                    0x0000,0x0000,0x0000,0x3FED,0x0000};

int RegisterInfoDlg::rightReadFlag=0;

RegisterInfoDlg::RegisterInfoDlg(QWidget *parent) : QDialog(parent)
{
    //setAttribute(Qt::WA_DeleteOnClose,true);

    initialWidget();

    QRegExp regExp("[0-9a-fA-F]{1,4}");
    lnedit_write_hex->setValidator(new QRegExpValidator(regExp, this));

    ///signal and slot
    connect(btn_read_hex,&QPushButton::clicked,this,&RegisterInfoDlg::btn_read_hex_clicked);
    connect(btn_write_hex,&QPushButton::clicked,this,&RegisterInfoDlg::btn_write_hex_clicked);
    //connect(combo_register_read_write,&QComboBox::currentIndexChanged,this,&RegisterInfoDlg::change_lnedit_btn_state);
    connect(combo_register_read_write,SIGNAL(currentIndexChanged(int)),this,SLOT(change_lnedit_btn_state()));

    connect(btn_user_read,SIGNAL(clicked(bool)),this,SLOT(btn_user_read_clicked()));
    connect(btn_user_print,SIGNAL(clicked(bool)),this,SLOT(btn_user_print_clicked()));
    connect(btn_user_save,SIGNAL(clicked(bool)),this,SLOT(btn_user_save_clicked()));

    setWindowTitle(tr("Register Information"));
    resize(750,400);
}

RegisterInfoDlg::~RegisterInfoDlg()
{

}

/*!
 * \brief RegisterInfoDlg::configRes_slot
 * receive the configRes_signal from SerialWriteObj and to process the
 * the data, set the value in the Register dialog
 * \param d
 * data received
 */
void RegisterInfoDlg::configRes_slot(const protocol::CfgData &d)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    if(protocol::checkSum(d)==false){
        qDebug()<<"checkSum:false.";
        return;
    }
    if(d.cmdf!=protocol::read_write_data){
        return;
    }

    uint8_t buff[2]={d.val[0],d.val[1]};
    uint16_t sh=*reinterpret_cast<uint16_t*>(buff);

    int index;
    if(false==searchIndex(d.name,index)){
        qDebug()<<"searchIndex: can't find register address.";
        return;
    }

    if(d.name==0x58){
        resetRightReadFlag();
    }

    // update right tableWidget
    qDebug("%04X",sh);
    qDebug("%ld",sizeof(sh));
    //QString str ="0x"+QString("%1").arg((uint16_t)sh,0,16).toUpper().rightJustified(4,'0');
    char dispstr[10];
    snprintf(dispstr,5,"%04X",sh);
    QString str="0x"+QString(dispstr);
    QTableWidgetItem* item_temp=new QTableWidgetItem(str);
    item_temp->setTextAlignment(Qt::AlignCenter);
    tab_user_registers->setItem(index-1,3,item_temp);

    // update left part
    int comboIndex=combo_register_read_write->currentIndex();
    if(index==comboIndex && d.op==protocol::get && rightReadFlag!=1){
        lnedit_read_hex->setText(str);
        QString decstr= QString::number((uint)sh,10);
        lnedit_deci_value->setText(decstr);
    }

}

void RegisterInfoDlg::sendPkg(protocol::operation_t op, char ch,uint16_t sh=0)
{
    protocol::CfgData cmd;
    memset(&cmd,0,sizeof(cmd));
    cmd.head = protocol::master;
    cmd.op   = op;
    cmd.cmdf = protocol::read_write_data;
    cmd.name = ch;
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

void RegisterInfoDlg::resetRightReadFlag()
{
    rightReadFlag=0;
}

/*!
 * \brief RegisterInfoDlg::searchIndex
 * search the char array ch_base_addr to find the index
 * of the given register address
 * \param n
 * the given register address
 * \param index
 * the index needed
 * \return
 * found n,return true, otherwise return false
 */
bool RegisterInfoDlg::searchIndex(protocol::paramname_t n, int &index)
{
    if(n>0x58) return false;
    for(int i=1;i<36;++i){
        if(ch_base_addr[i]==n){
            index=i;
            return true;
        }
    }
    return false;
}

void RegisterInfoDlg::btn_read_hex_clicked()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    int ind=combo_register_read_write->currentIndex();
    if(ind==0){
        QMessageBox::information(this,"Register information","Please select a Register.");
        return;
    }
    uint8_t ch=ch_base_addr[ind];
    sendPkg(protocol::get,ch);
}

void RegisterInfoDlg::btn_write_hex_clicked()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    int ind=combo_register_read_write->currentIndex();
    if(ind==0){
        QMessageBox::information(this,"Register information","Please select a Register.");
        return;
    }
    uint8_t ch=ch_base_addr[ind];
    QString str=lnedit_write_hex->text();
    if(str.isEmpty()){
        QMessageBox::information(this,"Register","The write data can't be empty.");
        return;
    }
    bool ok;
    uint16_t sh=str.toUShort(&ok,16);
    if(ok==false){
        QMessageBox::critical(this,"Error","Number transform error.");
        return;
    }
    sendPkg(protocol::set,ch,sh);
}

int RegisterInfoDlg::change_lnedit_btn_state()
{
    int index=combo_register_read_write->currentIndex();
    qDebug()<<"Item:"<<combo_register_read_write->currentText()<<",Select index:"<<index;

    if(0==index)
    {
        lnedit_base_addr->setText(QString(""));
        lnedit_num_bytes->setText(QString(""));
        lnedit_write_hex->setText(QString(""));
        lnedit_read_hex->setText(QString(""));
        lnedit_read_flag->setText(QString(""));
        lnedit_write_flag->setText(QString(""));
        lnedit_deci_value->setText(QString(""));
        btn_read_hex->setEnabled(false);
        btn_write_hex->setEnabled(false);
    }
    else
    {
        lnedit_base_addr->setText("0x"+QString::number((uint)ch_base_addr[index],16).toUpper().rightJustified(2,'0'));
        //lnedit_deci_value->setText("0x"+QString::number((uint)n_default_val[index],16).toUpper().rightJustified(4,'0'));
        lnedit_num_bytes->setText(QString("2"));
        QString qstr_flag = ch_is_read[index]==0 ? "False":"True";
        lnedit_read_flag->setText(qstr_flag);
        qstr_flag = ch_is_write[index]==0 ? "False":"True";
        lnedit_write_flag->setText(qstr_flag);
        lnedit_deci_value->setText("");
        lnedit_read_hex->setText("");
        btn_read_hex->setEnabled(ch_is_read[index]);

        bool b_write_btn = static_cast<bool>(ch_is_write[index]);
        btn_write_hex->setEnabled(b_write_btn);
    }

    return 0;
}

int RegisterInfoDlg::btn_user_read_clicked()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    // set the flag to indicate the left read lineEdit should't be updated
    rightReadFlag=1;
    for(int i=1;i<36;++i){
        char ch=ch_base_addr[i];
        sendPkg(protocol::get,ch);
    }
    QTimer::singleShot(10000,this,&RegisterInfoDlg::resetRightReadFlag);
    return 0;
}

int RegisterInfoDlg::btn_user_print_clicked()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);

    QString strStream;
    QTextStream out(&strStream);

    const int rowCount = tab_user_registers->rowCount();
    const int columnCount = tab_user_registers->columnCount();
    QString strTitle="untitled";

    out <<  "<html>\n"
        "<head>\n"
        "<meta Content=\"Text/html; charset=utf-8\">\n"
        <<  QString("<title>%1</title>\n").arg(strTitle)
        <<  "</head>\n"
        "<body bgcolor=#ffffff link=#5000A0>\n"
        "<table border=1 cellspacing=0 cellpadding=2>\n";

    // headers
    out << "<thead><tr bgcolor=#f0f0f0>";
    QString headstr[]={"Name","Addr","Default Value","Hex Value"};
    for (int column = 0; column < 4; column++)
            out << QString("<th>%1</th>").arg(headstr[column]);
    out << "</tr></thead>\n";

    // data table
    for (int row = 0; row < rowCount; row++) {
        out << "<tr>";
        for (int column = 0; column < columnCount; column++) {
            QTableWidgetItem* it = tab_user_registers->item(row,column);
            if (it) {
                QString data = it->text();
                out << QString("<td bkcolor=0>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }
    out <<  "</table>\n"
        "</body>\n"
        "</html>\n";

    QTextDocument *document = new QTextDocument();
    document->setHtml(strStream);

    QPrinter printer;

    QPrintDialog *dialog = new QPrintDialog(&printer, NULL);
    if (dialog->exec() == QDialog::Accepted) {
        document->print(&printer);
        QMessageBox::information(this,"Print","The file has been printed.");
    }

    delete document;


    return 0;
}

int RegisterInfoDlg::btn_user_save_clicked()
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Excel file"),
                       qApp->applicationDirPath()+"/untitled.csv",tr("Files (*.csv)"));
    if(fileName.isEmpty()){
        return 1;
    }

    qDebug()<<fileName;
    bool ed=fileName.endsWith(".csv",Qt::CaseInsensitive);
    if(!ed) {
        fileName+=".csv";
    }

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(this,tr("Save"),tr("Can not open file for writing."));
        return 1;
    }

    QTextStream out(&file);
    out << tr("Name,") << tr("Addr,")<< tr("Default Value,") << tr("Hex Value,")<<"\n";//表头
    int row = tab_user_registers->rowCount();
    int col = tab_user_registers->columnCount();
    for(int i=0;i<row;++i){
        for(int j=0;j<col;++j){
            QTableWidgetItem* it=tab_user_registers->item(i,j);
            if(it){
                QString str = it->text();
                out<<str<<",";
            }
        }
        out<<"\n";
    }
    file.close();

    int ret = QMessageBox::information(this,tr("Save"),
                                       tr("The file has been saved as \"%1\"").arg(fileName),
                                       tr("Show in folder"),
                                       tr("Ok"));
    // show in folder
    if(ret==0){
        QFileInfo fi=QFileInfo(fileName);
        QString filePath;
        filePath=fi.absolutePath();
        QDesktopServices::openUrl(QUrl(filePath, QUrl::TolerantMode));
    }
    return 0;
}

void RegisterInfoDlg::initialWidget()
{
    ///the left frame
    frm_register_read_write =new QFrame(this);
    frm_register_read_write->setLineWidth(1);
    frm_register_read_write->setFrameStyle(QFrame::Box|QFrame::Plain);
    frm_register_read_write->setMaximumWidth(250);

    gpb_register_read_write =new QGroupBox("Register Read/Write",frm_register_read_write);

    combo_register_read_write = new QComboBox(this);

    lab_base_addr = new QLabel("Base Addr (Hex)",this);
    lab_num_bytes = new QLabel("Num Bytes",this);
    lab_read_flag = new QLabel("Read Flag",this);
    lab_write_flag = new QLabel("Write Flag",this);
    lab_deci_value = new QLabel("Deci Value",this);

    btn_read_hex = new QPushButton("Read (Hex)",this);
    btn_write_hex = new QPushButton("Write (Hex)",this);

    lnedit_base_addr = new QLineEdit(this);
    lnedit_num_bytes = new QLineEdit(this);
    lnedit_read_flag = new QLineEdit(this);
    lnedit_write_flag = new QLineEdit(this);
    lnedit_deci_value = new QLineEdit(this);
    lnedit_read_hex = new QLineEdit(this);
    lnedit_write_hex = new QLineEdit(this);

    QGridLayout* lyt_grid_gpb =new QGridLayout;
    lyt_grid_gpb->addWidget(lab_base_addr,0,0);
    lyt_grid_gpb->addWidget(lab_num_bytes,1,0);
    lyt_grid_gpb->addWidget(lab_read_flag,2,0);
    lyt_grid_gpb->addWidget(lab_write_flag,3,0);
    lyt_grid_gpb->addWidget(lab_deci_value,4,0);
    lyt_grid_gpb->addWidget(btn_read_hex,5,0);
    lyt_grid_gpb->addWidget(btn_write_hex,6,0);

    lyt_grid_gpb->addWidget(lnedit_base_addr,0,1);
    lyt_grid_gpb->addWidget(lnedit_num_bytes,1,1);
    lyt_grid_gpb->addWidget(lnedit_read_flag,2,1);
    lyt_grid_gpb->addWidget(lnedit_write_flag,3,1);
    lyt_grid_gpb->addWidget(lnedit_deci_value,4,1);
    lyt_grid_gpb->addWidget(lnedit_read_hex,5,1);
    lyt_grid_gpb->addWidget(lnedit_write_hex,6,1);

    QVBoxLayout* lyt_vbox_gpb_left_total = new QVBoxLayout;
    lyt_vbox_gpb_left_total->addWidget(combo_register_read_write);
    lyt_vbox_gpb_left_total->addLayout(lyt_grid_gpb);
    lyt_vbox_gpb_left_total->addStretch();
    gpb_register_read_write->setLayout(lyt_vbox_gpb_left_total);

    QVBoxLayout* lyt_vbox_register_rw_frm_total=new QVBoxLayout;
    lyt_vbox_register_rw_frm_total->addWidget(gpb_register_read_write);
    frm_register_read_write->setLayout(lyt_vbox_register_rw_frm_total);

    ///the right frame
    frm_user_registers =new QFrame(this);
    gpb_user_registers =new QGroupBox("User Registers(Hex Values)",frm_user_registers);
    frm_user_registers->setLineWidth(1);
    frm_user_registers->setFrameStyle(QFrame::Box|QFrame::Plain);

    tab_user_registers =new QTableWidget(35,4,gpb_user_registers);
    btn_user_print =new QPushButton("Print",gpb_user_registers);
    btn_user_read =new QPushButton("Read",gpb_user_registers);
    btn_user_save =new QPushButton("Save",gpb_user_registers);

    QHBoxLayout* lyt_hbox_btn =new QHBoxLayout;
    lyt_hbox_btn->addStretch();
    lyt_hbox_btn->addWidget(btn_user_read);
    lyt_hbox_btn->addStretch();
    lyt_hbox_btn->addWidget(btn_user_print);
    lyt_hbox_btn->addStretch();
    lyt_hbox_btn->addWidget(btn_user_save);
    lyt_hbox_btn->addStretch();

    QVBoxLayout* lyt_vbox_gpb_right_total=new QVBoxLayout;
    lyt_vbox_gpb_right_total->addWidget(tab_user_registers);
    lyt_vbox_gpb_right_total->addLayout(lyt_hbox_btn);
    gpb_user_registers->setLayout(lyt_vbox_gpb_right_total);

    QVBoxLayout* lyt_vbox_frm_total =new QVBoxLayout;
    lyt_vbox_frm_total->addWidget(gpb_user_registers);
    frm_user_registers->setLayout(lyt_vbox_frm_total);

    QHBoxLayout* lyt_total =new QHBoxLayout;
    lyt_total->addWidget(frm_register_read_write);
    lyt_total->addWidget(frm_user_registers);

    ///left frame initialize
    QStringList qstrlst_combo;
    qstrlst_combo<<tr("Please Select")
                <<tr("FLASH_CNT")<<tr("SUPPLY_OUT")<<tr("XGYRO_OUT")<<tr("YGYRO_OUT")<<tr("ZGYRO_OUT")
                <<tr("XACCL_OUT")<<tr("YACCL_OUT")<<tr("ZACCL_OUT")<<tr("XTEMP_OUT")<<tr("YTEMP_OUT")
                <<tr("ZTEMP_OUT")<<tr("AUX_ADC")<<tr("XGYRO_OFF")<<tr("YGYRO_OFF")<<tr("ZGYRO_OFF")
                <<tr("XACCL_OFF")<<tr("YACCL_OFF")<<tr("ZACCL_OFF")<<tr("ALM_MAG1")<<tr("ALM_MAG2")
                <<tr("ALM_SMPL1")<<tr("ALM_SMPL2")<<tr("ALM_CTRL")<<tr("AUX_DAC")<<tr("GPIO_CTRL")
                <<tr("MSC_CTRL")<<tr("SMPL_PRD")<<tr("SENS_AVG")<<tr("SLP_CNT")<<tr("DIAG_STAT")
                <<tr("GLOB_CMD")<<tr("LOT_ID1")<<tr("LOT_ID2")<<tr("PROD_ID")<<tr("SERIAL_NUM");
    combo_register_read_write->addItems(qstrlst_combo);

    btn_read_hex->setDisabled(true);
    btn_write_hex->setDisabled(true);

    ///right frame initialize
    for(int i=0;i<35;++i)
    {
        QTableWidgetItem *tab_item_temp=new QTableWidgetItem(reg_str[i+1]);
        tab_item_temp->setTextAlignment(Qt::AlignCenter);
        tab_user_registers->setItem(i,0,tab_item_temp);

        QString str_base_temp =QString("0x"+QString::number((uint)ch_base_addr[i+1],16).toUpper().rightJustified(2,'0'));
        QTableWidgetItem* item_temp=new QTableWidgetItem(str_base_temp);
        item_temp->setTextAlignment(Qt::AlignCenter);
        tab_user_registers->setItem(i,1,item_temp);

        str_base_temp = QString("0x"+QString::number((uint)n_default_val[i+1],16).toUpper().rightJustified(4,'0'));
        item_temp=new QTableWidgetItem(str_base_temp);
        item_temp->setTextAlignment(Qt::AlignCenter);
        tab_user_registers->setItem(i,2,item_temp);
    }
    tab_user_registers->setFont(QFont("Times"));
    tab_user_registers->resizeColumnsToContents();
    tab_user_registers->setColumnWidth(2,100);
    tab_user_registers->resizeRowsToContents();
    /*
    tab_user_registers->setColumnWidth(0,60);
    tab_user_registers->setColumnWidth(1,60);
    tab_user_registers->setColumnWidth(2,80);
    tab_user_registers->setColumnWidth(3,80);
    */
    QStringList qstr_h_header;
    qstr_h_header<<"Name"<<"Addr"<<"Default Value"<<"Hex Value";
    tab_user_registers->setHorizontalHeaderLabels(qstr_h_header);
    tab_user_registers->horizontalHeader()->setStretchLastSection(true);

    setLayout(lyt_total);
}


