#ifndef LIBINSTALLER_H
#define LIBINSTALLER_H

#include <QObject>
#include <QDir>
#include <QTemporaryDir>

#include "libinstaller_global.h"
#include "mountpoint.h"

namespace qhttp {
namespace server {
class QHttpServer;
}
}

class LIBINSTALLERSHARED_EXPORT LibInstaller : public QObject
{
    Q_OBJECT

public:
    LibInstaller();
    bool listen(QString document_root, int port);
    bool listen(QString document_root, QString host, int port);
    bool mountPaths(QList<installer::MountPoint> paths);
    bool umountPaths(QList<installer::MountPoint> paths);

    QDir docroot;
    QTemporaryDir tempdir;
private:
    qhttp::server::QHttpServer *server;
};

#endif // LIBINSTALLER_H
