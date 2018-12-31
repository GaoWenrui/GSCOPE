#ifndef SERIALPROCESSOBJ_H
#define SERIALPROCESSOBJ_H

#include <QObject>
#include <QDebug>
#include "serial/serial.h"
#include "serialloopthread.h"

class SerialProcessObj : public QObject
{
    Q_OBJECT
public:
    explicit SerialProcessObj(serial::Serial* mSerial,SerialPkg* pkg,bool bstop = false)
    {
        qDebug()<<"SerialProcessObj::SerialProcessObj(serial::Serial* mSerial)";
        slpthread=new SerialLoopThread(mSerial,pkg,this,bstop);
        slpthread->start();
    }
    ~SerialProcessObj()
    {

    }

signals:

public slots:
    void serial_obj_to_stop_slot(bool stop)
    {
        // to stop: true
        // to start: false
        slpthread->slp_to_stop(stop);
    }

private:
    SerialLoopThread *slpthread;
};

#endif // SERIALPROCESSOBJ_H
