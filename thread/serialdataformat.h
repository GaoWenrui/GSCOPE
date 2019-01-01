#ifndef SERIALDATAFORMAT_H
#define SERIALDATAFORMAT_H

#include <vector>
#include <stdint.h>

const int MAX_CHAR_NUM = 700;

typedef struct serialdatapkg{
    int length;
    unsigned char ch[MAX_CHAR_NUM];
}SerialPkg;

/*!
 * Defination of SerialResPkg
 */

typedef struct struct_qutn
{
    double x;
    double y;
    double z;
    double w;
}qutn;

typedef struct struct_gyro
{
    double x;
    double y;
    double z;
}gyro;

typedef struct struct_accl
{
    double x;
    double y;
    double z;
}accl;

typedef struct struct_angl
{
    double roll;
    double pitch;
    double yaw;
}angl;

typedef struct serialresolvepkg{
    qutn orientation;
    gyro angular_velocity;
    accl linear_acceleration;
    angl angular;
}SerialResPkg;


/*!
 * namespace protocol define the write data format
 * Enumeration defines the protocol databyte
 */

namespace protocol {

typedef enum : unsigned char{
    master = 0xAA,
    slave  = 0xBB
}head_t;

typedef unsigned char checkval_t;

typedef enum :unsigned char{
    set = 0x01,
    get = 0x02
}operation_t;

typedef enum :unsigned char{
    read_write_data = 0x01,
    manual_bias_cal = 0x02,
    auto_bias_null_cal =0x03,
    precsn_bias_cal =0x04,
    restore_fctry_cal =0x05,
    lnr_accl_bias_cmps =0x06,
    software_reset =0x07,
    inter_sampl_cmd =0x08,
    sleep_mode =0x09,
    dynamic_range_set =0x0A,
    self_test_seq =0x0B,
    memory_test =0x0C,
    read_status =0x0D,
    alarm_set =0x0E,
    product_id =0x0F,
    start_upload =0x10
}cmdfield_t;

typedef unsigned char paramname_t;

typedef unsigned char paramval_t[4];

typedef enum :unsigned char {
    tail1=0x0D
}tail1_t;

typedef enum :unsigned char{
    tail2=0x0A
}tail2_t;

typedef struct tagCfgData
{
    head_t head;
    checkval_t ck;
    operation_t op;
    cmdfield_t cmdf;
    paramname_t name;
    paramval_t val;
    tail1_t tl1;
    tail2_t tl2;
}CfgData;

void vec2Cfgdata(const std::vector<uint8_t> &v,protocol::CfgData &res);

void cfgdata2Vec(const protocol::CfgData& res,std::vector<uint8_t>& v);

bool checkSum(const std::vector<uint8_t> &v);

bool checkSum(const protocol::CfgData& res);

void calCheckSum(std::vector<uint8_t> &v);

void calCheckSum(protocol::CfgData& res);
}

#endif // SERIALDATAFORMAT_H
