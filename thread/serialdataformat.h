#ifndef SERIALDATAFORMAT_H
#define SERIALDATAFORMAT_H

typedef struct serialdatapkg{
    bool isupdate;
    int length;
    unsigned char ch[700];
}SerialPkg;

#endif // SERIALDATAFORMAT_H
