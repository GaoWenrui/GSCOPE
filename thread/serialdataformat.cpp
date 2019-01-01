#include "thread/serialdataformat.h"
#include <QDebug>

namespace protocol {
void vec2Cfgdata(const std::vector<uint8_t> &v, protocol::CfgData &res)
{
    res.head=static_cast<protocol::head_t>(v[0]);
    res.ck=static_cast<protocol::checkval_t>(v[1]);
    res.op=static_cast<protocol::operation_t>(v[2]);
    res.cmdf=static_cast<protocol::cmdfield_t>(v[3]);
    res.name=static_cast<protocol::paramname_t>(v[4]);
    res.val[0]=v[5]; res.val[1]=v[6];  res.val[2]=v[7];  res.val[3]=v[8];
    res.tl1=static_cast<protocol::tail1_t>(v[9]);
    res.tl2=static_cast<protocol::tail2_t>(v[10]);
}

void cfgdata2Vec(const protocol::CfgData &res, std::vector<uint8_t> &v)
{
    v[0]= res.head;
    v[1]= res.ck;
    v[2]= res.op;
    v[3]= res.cmdf;
    v[4]= res.name;
    v[5]= res.val[0];
    v[6]= res.val[1];
    v[7]= res.val[2];
    v[8]= res.val[3];
    v[9]= res.tl1;
    v[10]= res.tl2;
}

bool checkSum(const std::vector<uint8_t> &v)
{
    uint8_t ckval=0;
    for(int i=2;i<11;++i){
        ckval+=v[i];
    }
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    qDebug("v[1]=%02X",v[1]);
    qDebug("ckval=%02X",ckval);
    return v[1]==ckval;
}

bool checkSum(const CfgData &res)
{
    std::vector<uint8_t> tv(11,0);
    cfgdata2Vec(res,tv);
    return checkSum(tv);
}

void calCheckSum(std::vector<uint8_t> &v)
{
    uint8_t ckval=0;
    for(int i=2;i<11;++i){
        ckval+=v[i];
    }
    v[1]=ckval;
}

void calCheckSum(CfgData &res)
{
    std::vector<uint8_t> tv(11,0);
    cfgdata2Vec(res,tv);
    calCheckSum(tv);
    res.ck = static_cast<checkval_t>(tv[1]);
}

}// end of namespace protocol
