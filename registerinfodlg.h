#ifndef REGISTERINFODLG_H
#define REGISTERINFODLG_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QFrame>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
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
class QFrame;
class QComboBox;
class QTableWidget;
QT_END_NAMESPACE
*/

class RegisterInfoDlg : public QDialog
{
    Q_OBJECT
private:
    QFrame* frm_register_read_write;
    QFrame* frm_user_registers;
    QGroupBox* gpb_register_read_write;
    QGroupBox* gpb_user_registers;

    QComboBox* combo_register_read_write;
    QLabel* lab_base_addr;
    QLabel* lab_num_bytes;
    QLabel* lab_read_flag;
    QLabel* lab_write_flag;
    QLabel* lab_deci_value;
    QPushButton* btn_read_hex;
    QPushButton* btn_write_hex;

    QLineEdit* lnedit_base_addr;
    QLineEdit* lnedit_num_bytes;
    QLineEdit* lnedit_read_flag;
    QLineEdit* lnedit_write_flag;
    QLineEdit* lnedit_deci_value;
    QLineEdit* lnedit_read_hex;
    QLineEdit* lnedit_write_hex;

    ///User Registers(Hex Values)
    QPushButton* btn_user_read;
    QPushButton* btn_user_print;
    QPushButton* btn_user_save;

    QTableWidget* tab_user_registers;

public:
    explicit RegisterInfoDlg(QWidget *parent = 0);
    ~RegisterInfoDlg();
signals:
    void configWrite_signal(const protocol::CfgData& d);
public slots:
    void configRes_slot(const protocol::CfgData& d);

    void btn_read_hex_clicked();
    void btn_write_hex_clicked();

    int change_lnedit_btn_state();
    int btn_user_read_clicked();
    int btn_user_print_clicked();
    int btn_user_save_clicked();
private:
    void initialWidget();
    void sendPkg(protocol::operation_t op,char ch,uint16_t sh);
    void resetRightReadFlag();
    bool searchIndex(protocol::paramname_t n,int &index);
    static int rightReadFlag;
    QTimer rightReadTimer;
};

#endif // REGISTERINFODLG_H
