#include "disk.h"

namespace parted {

Disk::Disk(Device *dev) : dev(dev)
{
    disk = ped_disk_new(dev->dev);
}

Disk::Disk(PedDisk *disk) : disk(disk) {
}

Disk::~Disk() {
    if (disk) ped_disk_destroy(disk);
}

} // namespace parted
