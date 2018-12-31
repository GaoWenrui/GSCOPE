#ifndef SERIALWRITEOBJ_H
#define SERIALWRITEOBJ_H

#include <QObject>

class SerialWriteObj : public QObject
{
    Q_OBJECT
public:
    explicit SerialWriteObj(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SERIALWRITEOBJ_H