#include <QProcess>

#include "mountpoint.h"

namespace installer {

MountPoint::MountPoint(QString dev, QString mountpoint):
    device(dev), mountpoint(mountpoint), mounted(false)
{
}

bool MountPoint::mount() {
    return !QProcess::execute(QString("mount -v \"%1\" \"%2\"").arg(device).arg(mountpoint));
}

bool MountPoint::umount() {
    return !QProcess::execute(QString("umount -v \"%1\"").arg(mountpoint));
}

} // namespace installer

