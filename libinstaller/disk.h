#ifndef PARTED_DISK_H
#define PARTED_DISK_H

#include <parted/parted.h>

#include "device.h"

namespace parted {

class Disk
{
public:
    Disk(Device *dev);
    Disk(PedDisk *disk);
    ~Disk();

    PedDisk *disk;
    Device *dev;
};

} // namespace parted

#endif // PARTED_DISK_H
