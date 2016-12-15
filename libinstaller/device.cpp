#include "device.h"

namespace parted {

Device::Device(QString devnode) : devnode(devnode) {
    dev = ped_device_get(qPrintable(devnode));
}

bool Device::open() {
    return ped_device_open(dev);
}

bool Device::close() {
    return ped_device_close(dev);
}

} // namespace parted

