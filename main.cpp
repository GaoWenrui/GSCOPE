#include "gscopemainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GScopeMainWindow w;
    w.show();

    return a.exec();
}
