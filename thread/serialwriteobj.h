#ifndef SERIALWRITEOBJ_H
#define SERIALWRITEOBJ_H

#include <QObject>
#include <QMutex>
#include <vector>
#include "thread/serialdataformat.h"
#include "serial/serial.h"

class SerialWriteObj : public QObject
{
    Q_OBJECT
public:
    explicit SerialWriteObj(QObject *parent = nullptr);
    explicit SerialWriteObj(serial::Serial *s,QObject *parent = nullptr);

signals:
    void configRes_singal(const protocol::CfgData &res);

public slots:
    void configWrite_slot(const protocol::CfgData& cfgwt);
    void serialChanged_slot(serial::Serial* s);
    void bytesWrite_slot(const std::vector<uint8_t>& v);

private:
    void configWrite(const protocol::head_t& head,const protocol::checkval_t& ck,
                const protocol::operation_t& op,const protocol::cmdfield_t& cmdf,
                const protocol::paramname_t& name,const protocol::paramval_t& val,
                const protocol::tail1_t& tl1,const protocol::tail2_t& tl2);

    void writeReadVector(const std::vector<uint8_t>& writeV);
    void resolveConfig(const protocol::CfgData& cfgwt);

    serial::Serial *mSerial;
    mutable QMutex mutex;
};

#endif // SERIALWRITEOBJ_H
