#include "serialwriteobj.h"
#include <QDebug>

SerialWriteObj::SerialWriteObj(QObject *parent)
    : QObject(parent)
{
    mSerial = nullptr;
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
}

SerialWriteObj::SerialWriteObj(serial::Serial *s, QObject *parent)
    :QObject(parent),mSerial(s)
{

}

void SerialWriteObj::configWrite(const protocol::head_t &head, const protocol::checkval_t &ck,
                            const protocol::operation_t &op, const protocol::cmdfield_t &cmdf,
                            const protocol::paramname_t &name, const protocol::paramval_t &val,
                            const protocol::tail1_t &tl1, const protocol::tail2_t &tl2)
{
    std::vector<uint8_t> senddata(11,0);

    senddata[0]=head;  senddata[1]=ck;  senddata[2]=op;  senddata[3]=cmdf;senddata[4]=name;
    senddata[5]=val[0];senddata[6]=val[1];  senddata[7]=val[2];  senddata[8]=val[3];
    senddata[9]=tl1;  senddata[10]=tl2;
    writeReadVector(senddata);
}

void SerialWriteObj::writeReadVector(const std::vector<uint8_t> &writeV)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    std::vector<uint8_t> revdata;

    // comment the following line
    //mSerial->write(writeV);
    if(mSerial==NULL){
        qDebug("mSerial=NULL,please open the port");
        return;
    }

    // 3次内数据错误，重写数据，超过3次，结束函数
    {QMutexLocker locker(&mutex);
    for(int cnt=0;cnt<3;++cnt){
        mSerial->write(writeV);
        while(revdata.size()<11)
        {
            mSerial->read(revdata,1);
        }
        qDebug("revdata size=%ld",revdata.size());
        for(int i=0;i<11;++i){
            qDebug("%02X",revdata[i]);
        }
        bool rest=protocol::checkSum(revdata);
        qDebug("check sum=%s",rest==true?"true":"false");
        if(rest && revdata[0]!=0x00) {
            qDebug("checkSum ok!");
            break;
        }
        if(cnt==2) {
            qDebug("cnt=2,return.");
            return;
        }
    }
    protocol::CfgData res;
    protocol::vec2Cfgdata(revdata,res);
    emit configRes_singal(res);
    }
}

void SerialWriteObj::configWrite_slot(const protocol::CfgData &cfgwt)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    std::vector<uint8_t> vec(11,0);
    protocol::cfgdata2Vec(cfgwt,vec);
    for(int i=0;i<11;++i){
        qDebug("%02X",vec[i]);
    }

    // to test the datalog dialog
    /*
    if(cfgwt.name>=0x60 && cfgwt.name<=0x70){
        protocol::CfgData reply;
        memcpy(&reply,&cfgwt,sizeof(cfgwt));
        reply.head=protocol::slave;
        float tp=113.14259;
        char *pch=reinterpret_cast<char*>(&tp);
        reply.val[0]=*pch;pch++;
        reply.val[1]=*pch;pch++;
        reply.val[2]=*pch;pch++;
        reply.val[3]=*pch;
        protocol::calCheckSum(reply);
        emit  configRes_singal(reply);
        return;
    }
    */

    qDebug("mSerial==NULL?%s",mSerial==NULL?"true":"false");
    writeReadVector(vec);
}

void SerialWriteObj::serialChanged_slot(serial::Serial* s)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    mSerial=s;
    qDebug("mSerial==NULL? %s",mSerial==NULL?"true":"false");
}

void SerialWriteObj::bytesWrite_slot(const std::vector<uint8_t> &v)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    qDebug("mSerial==NULL? %s",mSerial==NULL?"true":"false");
    if(mSerial==NULL)
        return;
    qDebug("v.size()=%ld",v.size());
    for(size_t i=0;i<v.size();++i){
        qDebug("%02X",v[i]);
    }

    size_t t=mSerial->write(v);
    qDebug("write size=%ld",t);
}
