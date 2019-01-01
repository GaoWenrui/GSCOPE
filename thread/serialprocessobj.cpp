#include "thread/serialprocessobj.h"

const int DATA_LEN=67;
const double WYC_PI = 3.141592653589793;

SerialProcessObj::SerialProcessObj(QObject* parent)
    :QObject(parent)
{
    qDebug("File:%s Line:%d Function:%s", __FILE__, __LINE__, __FUNCTION__);
    memset(&mRespkg,0,sizeof(struct serialresolvepkg));
    memset(buffer,0,sizeof(buffer));
}

SerialProcessObj::~SerialProcessObj()
{

}

void SerialProcessObj::serial_obj_begin_resolve_slot(const SerialPkg& pkg)
{
    memcpy(buffer+len,pkg.ch,pkg.length);
    len+=pkg.length;
    if(len>=5*DATA_LEN){
        if(!truncateData(buffer,len)){
            if(0==resolveSingleCmd(buffer,len)){
                emit serial_obj_resolve_ok_signal(mRespkg);
                len=0;
                return;
            }
        }
        len=0;
    }
    else if(len>=DATA_LEN){
        if(!truncateData(buffer,len)){
            if(0==resolveSingleCmd(buffer,len)){
                emit serial_obj_resolve_ok_signal(mRespkg);
                len=0;
                return;
            }
        }
    }
    else{

    }

}

int SerialProcessObj::truncateData(unsigned char *dataBuffer, int &length)
{
    for(int i=length-1;i>=66;--i)
    {
      if(dataBuffer[i]==0x0A && dataBuffer[i-1]==0x0D &&
         dataBuffer[i-66]==0x3A && dataBuffer[i-65]==0x01 &&
         dataBuffer[i-64]==0x00 && dataBuffer[i-63]==0x09)
      {
        length=67;
        int startIndex=i-66;
        memmove(dataBuffer,dataBuffer+startIndex,67);
        return 0;
      }
    }
    return -1;
}

bool SerialProcessObj::sumCheck(unsigned char *data)
{
    uint16_t checksum=0;
    for(int i=1;i<63;++i)
    {
      checksum=checksum+data[i];
    }
    unsigned char LSB=(unsigned char)(checksum&0x00ff);
    unsigned char MSB=(unsigned char)((checksum&0xff00)>>8);
    if(LSB==data[63] && MSB==data[64])
      return true;
    return false;
}

int SerialProcessObj::resolveSingleCmd(unsigned char *data, int &length)
{
    bool check=sumCheck(data);
    if(!check || length!=67) {
      return -1;
    }

    mRespkg.linear_acceleration.x=little_endian_to_double(&data[23]);
    mRespkg.linear_acceleration.y=little_endian_to_double(&data[27]);
    mRespkg.linear_acceleration.z=little_endian_to_double(&data[31]);

    mRespkg.angular_velocity.x=little_endian_to_double(&data[11]);
    mRespkg.angular_velocity.y=little_endian_to_double(&data[15]);
    mRespkg.angular_velocity.z=little_endian_to_double(&data[19]);

    mRespkg.orientation.w=little_endian_to_double(&data[47]);
    mRespkg.orientation.x=little_endian_to_double(&data[51]);
    mRespkg.orientation.y=little_endian_to_double(&data[55]);
    mRespkg.orientation.z=little_endian_to_double(&data[59]);

    calculate_angular(mRespkg);

    qDebug("accl[x,y,z]=[%6.4f,%6.4f,%6.4f]",
           mRespkg.linear_acceleration.x,
           mRespkg.linear_acceleration.y,
           mRespkg.linear_acceleration.z);

    qDebug("gyro[x,y,z]=[%6.4f,%6.4f,%6.4f]",
           mRespkg.angular_velocity.x,
           mRespkg.angular_velocity.y,
           mRespkg.angular_velocity.z);

    qDebug("qutn[w,x,y,z]=[%6.4f,%6.4f,%6.4f,%6.4f]",
           mRespkg.orientation.w,
           mRespkg.orientation.x,
           mRespkg.orientation.y,
           mRespkg.orientation.z);

    qDebug("angl[x,y,z]=[%6.4f,%6.4f,%6.4f]",
           mRespkg.angular.roll,
           mRespkg.angular.pitch,
           mRespkg.angular.yaw);

    return 0;
}

double SerialProcessObj::little_endian_to_double(unsigned char *data)
{
    unsigned char buff[4]={data[0],data[1],data[2],data[3]};
    float transfloat= *reinterpret_cast<float*>(buff);
    double retdouble=static_cast<double>(transfloat);
    return retdouble;
}

void SerialProcessObj::calculate_angular(SerialResPkg &respkg)
{
    /*
    * roll=atan2(2(wx+yz),1-2(x^2+y^2))
    * pitch=arcsin(2(wy-zx))
    * yaw=atan2(2(wz+xy),1-2(y^2+z^2))
    */

    // roll (x-axis rotation)
    double roll,pitch,yaw;
    double sinr = +2.0*(respkg.orientation.w*respkg.orientation.x + respkg.orientation.y*respkg.orientation.z);
    double cosr = +1.0-2.0*(respkg.orientation.x*respkg.orientation.x+respkg.orientation.y*respkg.orientation.y);
    roll = atan2(sinr, cosr);

    // pitch (y-axis rotation)
    double sinp = +2.0*(respkg.orientation.w*respkg.orientation.y-respkg.orientation.z*respkg.orientation.x);
    if (fabs(sinp) >= 1)
        pitch = copysign(WYC_PI/2.0, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

    // yaw (z-axis rotation)
    double siny = +2.0*(respkg.orientation.w*respkg.orientation.z+respkg.orientation.x*respkg.orientation.y);
    double cosy = +1.0-2.0*(respkg.orientation.y*respkg.orientation.y+respkg.orientation.z*respkg.orientation.z);
    yaw = atan2(siny, cosy);

    // radian to degree
    roll=roll/WYC_PI*180.0;
    pitch=pitch/WYC_PI*180.0;
    yaw=yaw/WYC_PI*180.0;

    //add according to AHRSIMU
    if(yaw<-180.0) yaw=180.0-(-180.0-yaw);
    else if(yaw>180){
        yaw=-180.0-(180.0-yaw);
    }

    respkg.angular.roll=roll;
    respkg.angular.pitch=pitch;
    respkg.angular.yaw=yaw;
}
