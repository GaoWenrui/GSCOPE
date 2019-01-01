#ifndef SERIALPROCESSOBJ_H
#define SERIALPROCESSOBJ_H

#include <QObject>
#include <QDebug>
#include "thread/serialdataformat.h"

class SerialProcessObj : public QObject
{
    Q_OBJECT
public:
    explicit SerialProcessObj(QObject* parent=0);
    ~SerialProcessObj();

signals:
    void serial_obj_resolve_ok_signal(const SerialResPkg& respkg);

public slots:
    // to stop: true
    // to start: false
    //void serial_obj_to_stop_slot(const bool stop);
    void serial_obj_begin_resolve_slot(const SerialPkg& pkg);

private:
    SerialResPkg mRespkg;
    int len;
    unsigned char buffer[1000];

    int truncateData(unsigned char* dataBuffer,int &length);
    bool sumCheck(unsigned char* data);
    int resolveSingleCmd(unsigned char* data,int &length);
    double little_endian_to_double(unsigned char *data);
    void calculate_angular(SerialResPkg& respkg);
};

#endif // SERIALPROCESSOBJ_H
