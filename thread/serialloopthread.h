#ifndef SERIALLOOPTHREAD_H
#define SERIALLOOPTHREAD_H

#include <QThread>
#include <QDebug>
#include <QMutex>
#include "serial/serial.h"
#include "serialdataformat.h"

class SerialLoopThread : public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE{
        QMutexLocker locker(&mutex);
        int bFlag = pth_is_stop;
        locker.unlock();

        while(!bFlag){
            size=pthSerial->read(buffer,67);
            if(size>0){
                locker.relock();
                mData->length=size;
                memcpy(mData->ch,buffer,size);
                mData->isupdate=true;
                locker.unlock();
            }

            locker.relock();
            bFlag = pth_is_stop;
            locker.unlock();
        }
        pthSerial->close();
    }
signals:

public:
    explicit SerialLoopThread(serial::Serial* mSerial,SerialPkg* pkg,QObject * parent = 0,bool bstop = false)
        :QThread(parent),pthSerial(mSerial),mData(pkg),pth_is_stop(bstop)
    {
        qDebug()<<"SerialLoopThread::SerialPLoopThread(serial::Serial* mSerial,QObject*)";
    }
public:
    void slp_to_stop(bool is_stop){
         qDebug()<<"SerialLoopThread::pth_to_stop(bool)";
         QMutexLocker locker(&mutex);
         pth_is_stop = is_stop;
    }

private:
    unsigned char buffer[670];
    unsigned long size;

    serial::Serial* pthSerial;
    SerialPkg* mData;
    bool pth_is_stop;
    mutable QMutex mutex;

};

#endif // SERIALLOOPTHREAD_H
