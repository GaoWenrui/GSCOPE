#include "gscopemainwindow.h"
#include <QApplication>
#include <QString>
#include <QIODevice>

//Q_DECLARE_METATYPE(serial::Serial*)
int main(int argc, char *argv[])
{
    qRegisterMetaType<SerialPkg>("SerialPkg");
    qRegisterMetaType<SerialResPkg>("SerialResPkg");
    qRegisterMetaType<protocol::CfgData>("protocol::CfgData");
    qRegisterMetaType<serial::Serial*>("serial::Serial*");
    qRegisterMetaType<std::vector<uint8_t>>("std::vector<uint8_t>");


    QApplication a(argc, argv);
    GScopeMainWindow w;
    w.show();

    return a.exec();
}
