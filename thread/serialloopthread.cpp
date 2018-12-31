#ifndef SERIALLOOPTHREAD_H
#define SERIALLOOPTHREAD_H

#include <QThread>
#include <QDebug>
#include <QMutex>
#include "serial/serial.h"
#include "thread/serialdataformat.h"
#include "thread/threadsafequeue.hpp"

class SerialLoopThread : public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE{
        while(!pth_is_stop){
            usleep(10);
             mData.length=pthSerial->read(mData.ch,1);
            if( mData.length>0){
                mQueue->push(mData);
                emit read_byte_ready_signal(mData);
            }

            {
                QMutexLocker lk(&mutex);
                if(pth_is_stop) break;
            }
        }
    }
signals:
    void read_byte_ready_signal(const SerialPkg& pkg);
public:
    explicit SerialLoopThread(serial::Serial* mSerial,bool bstop = false,QObject * parent = 0)
        :QThread(parent),pthSerial(mSerial),pth_is_stop(bstop)
    {
        qDebug("File:%s Line:%d Function:%s\n", __FILE__, __LINE__, __FUNCTION__);
    }

    explicit SerialLoopThread(serial::Serial* mSerial,threadsafe_queue<SerialPkg>* que,bool bstop = false,QObject * parent = 0)
        :QThread(parent),pthSerial(mSerial),mQueue(que),pth_is_stop(bstop)
    {
        qDebug("File:%s Line:%d Function:%s\n", __FILE__, __LINE__, __FUNCTION__);
    }

    ~SerialLoopThread(){

    }
public:
    void slp_to_stop(bool is_stop){
        // to stop: true
        // to start: false
        qDebug("File:%s Line:%d Function:%s\n", __FILE__, __LINE__, __FUNCTION__);
        QMutexLocker locker(&mutex);
        pth_is_stop = is_stop;
    }

private:
    serial::Serial* pthSerial;
    threadsafe_queue<SerialPkg>* mQueue;
    bool pth_is_stop;
    mutable QMutex mutex;
    SerialPkg mData;
};

#endif // SERIALLOOPTHREAD_H
