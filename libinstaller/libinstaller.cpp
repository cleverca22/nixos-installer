#include <algorithm>
#include <QDebug>
#include <qhttpserver.hpp>

#include "libinstaller.h"
#include "clienthandler.h"

using namespace qhttp::server;
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
    bool ret = true;
    QList<MountPoint> list2 = paths;
    std::sort(list2.begin(), list2.end(), sortByLength);
    foreach(MountPoint mp, list2) {
        if (!mp.umount()) {
            ret = false;
            qDebug() << "failed to umount:" << mp.mountpoint;
        }
    }
    return ret;
}

bool LibInstaller::listen(QString document_root, int port) {
    listen(document_root, "127.0.0.1", port);
}

bool LibInstaller::listen(QString document_root, QString host, int port) {
    docroot = document_root;
    server = new QHttpServer(this);
    server->listen(QHostAddress(host),port,[&](QHttpRequest *req, QHttpResponse *res) {
        new ClientHandler(this,req,res);
    });

    return server->isListening();
}
