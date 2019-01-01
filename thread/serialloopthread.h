#ifndef SERIALLOOPTHREAD_H
#define SERIALLOOPTHREAD_H

#include <QThread>
#include <QMutex>
#include "serial/serial.h"
#include "thread/serialdataformat.h"
#include "thread/threadsafequeue.hpp"

class SerialLoopThread : public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE;
signals:
    void read_byte_ready_signal(const SerialPkg& pkg);
public:
    explicit SerialLoopThread(serial::Serial* mSerial,bool bstop = false,QObject * parent = 0);

    explicit SerialLoopThread(serial::Serial* mSerial,threadsafe_queue<SerialPkg>* que,bool bstop = false,QObject * parent = 0);

    ~SerialLoopThread();
public:
    void slp_to_stop(bool is_stop);
    void slp_port_changed_slot( serial::Serial* s);

private:
    serial::Serial* pthSerial;
    threadsafe_queue<SerialPkg>* mQueue;
    bool pth_is_stop;
    mutable QMutex mutex;
    SerialPkg mData;
};

#endif // SERIALLOOPTHREAD_H
