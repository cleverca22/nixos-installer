#ifndef LIBINSTALLER_H
#define LIBINSTALLER_H

#include <QObject>
#include <QDir>

#include "libinstaller_global.h"
#include "mountpoint.h"
#include "clienthandler.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>

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
    bool mountPaths(QList<installer::MountPoint> paths);
    bool umountPaths(QList<installer::MountPoint> paths);

    QDir docroot;
private:
    qhttp::server::QHttpServer *server;
};

#endif // LIBINSTALLER_H
