#include <algorithm>
#include <QDebug>

#include "libinstaller.h"

using namespace installer;

LibInstaller::LibInstaller()
{
}

bool sortByLength(const MountPoint a, const MountPoint b) {
    return a.mountpoint.length() > b.mountpoint.length();
}

bool LibInstaller::mountPaths(QList<MountPoint> paths) {
    QList<MountPoint> list2 = paths;
    std::sort(list2.begin(), list2.end(), sortByLength);
    foreach(MountPoint mp, list2) {
        if (!mp.mount()) {
            qDebug() << "failed to mount" << mp.mountpoint;
            return false;
        }
    }
    return true;
}

bool LibInstaller::umountPaths(QList<MountPoint> paths) {
    QList<MountPoint> list2 = paths;
    std::sort(list2.begin(), list2.end(), sortByLength);
    foreach(MountPoint mp, list2) {
        mp.umount();
    }
}
