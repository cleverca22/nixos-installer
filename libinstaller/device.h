#ifndef PARTED_DEVICE_H
#define PARTED_DEVICE_H

#include <QObject>

#include <parted/parted.h>

namespace parted {

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QString devnode);
    bool open();
    bool close();

    PedDevice *dev;
    QString devnode;
signals:

public slots:
private:
};

} // namespace parted

#endif // PARTED_DEVICE_H
