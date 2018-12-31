#include "gscopemainwindow.h"
#include "ui_gscopemainwindow.h"

GScopeMainWindow::GScopeMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GScopeMainWindow)
{
    ui->setupUi(this);
}

GScopeMainWindow::~GScopeMainWindow()
{
    delete ui;
}
