#include "thread/serialloopthread.h"
#include <QDebug>

void SerialLoopThread::run()
{
    while(!pth_is_stop){
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

SerialLoopThread::SerialLoopThread(serial::Serial *mSerial, bool bstop, QObject *parent)
    :QThread(parent),pthSerial(mSerial),pth_is_stop(bstop)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
}

SerialLoopThread::SerialLoopThread(serial::Serial *mSerial, threadsafe_queue<SerialPkg> *que, bool bstop, QObject *parent)
    :QThread(parent),pthSerial(mSerial),mQueue(que),pth_is_stop(bstop)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
}

SerialLoopThread::~SerialLoopThread()
{

}

void SerialLoopThread::slp_to_stop(bool is_stop){
    // to stop: true
    // to start: false
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    QMutexLocker locker(&mutex);
    pth_is_stop = is_stop;
}

void SerialLoopThread::slp_port_changed_slot(serial::Serial* s)
{
    QMutexLocker locker(&mutex);
    pth_is_stop=false; // serial port changed, must restart
    pthSerial = s;
}

