#include "disk.h"

namespace parted {

Disk::Disk(Device *dev)
{
    disk = ped_disk_new(dev->dev);
}

} // namespace parted
