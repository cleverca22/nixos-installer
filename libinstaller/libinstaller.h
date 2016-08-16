#ifndef LIBINSTALLER_H
#define LIBINSTALLER_H

#include <QObject>

#include "libinstaller_global.h"
#include "mountpoint.h"

class LIBINSTALLERSHARED_EXPORT LibInstaller : public QObject
{
    Q_OBJECT

public:
    LibInstaller();
    bool mountPaths(QList<installer::MountPoint> paths);
    bool umountPaths(QList<installer::MountPoint> paths);
};

#endif // LIBINSTALLER_H
