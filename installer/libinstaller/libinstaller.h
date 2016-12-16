#ifndef LIBINSTALLER_H
#define LIBINSTALLER_H

#include <QObject>
#include <QDir>
#include <QTemporaryDir>

#include "libinstaller_global.h"
#include "mountpoint.h"
#include "clienthandler.h"

namespace qhttp {
namespace server {
class QHttpServer;
}
}

typedef websocketpp::server<websocketpp::config::asio> server;

class runner : public QThread {
public:
    server serv;
    ClientHandler handler;
    void run();
    QString docroot;
    quint16 port;
};

class LIBINSTALLERSHARED_EXPORT LibInstaller : public QObject
{
    Q_OBJECT

public:
    LibInstaller();
    bool listen(QString document_root, quint16 port);
    bool listen(QString document_root, QString host, quint16 port);
    bool mountPaths(QList<installer::MountPoint> paths);
    bool umountPaths(QList<installer::MountPoint> paths);

    QDir docroot;
    QTemporaryDir tempdir;
private:
    qhttp::server::QHttpServer *server;
};

#endif // LIBINSTALLER_H
