#ifndef GSCOPEMAINWINDOW_H
#define GSCOPEMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class GScopeMainWindow;
}

class GScopeMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GScopeMainWindow(QWidget *parent = 0);
    ~GScopeMainWindow();

private:
    Ui::GScopeMainWindow *ui;
};

#endif // GSCOPEMAINWINDOW_H
