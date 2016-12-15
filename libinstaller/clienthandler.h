#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QProcess>
#include <QThread>

#include "libinstaller.h"
#include "disk.h"

namespace qhttp {
namespace server {
class QHttpRequest;
class QHttpResponse;
class QHttpServer;
}
}

class LargeFileReader;

void ClientLoadSlots();

class ClientHandler : public QObject {
Q_OBJECT
public:
    ClientHandler(LibInstaller *parent, qhttp::server::QHttpRequest *req, qhttp::server::QHttpResponse *res);

public slots:
    bool makeLabel(QJsonObject *reply, int handle, QString type);
    bool openDevice(QJsonObject *reply, QString device);
    bool openDisk(QJsonObject *reply, int device_handle);
    bool listPartitions(QJsonObject *reply, int disk_handle);
    bool createPartition(QJsonObject *reply, int disk_handle, int start, int size);
    bool testConfig(QJsonObject *reply, QString configuration);
private slots:
    void optionsBuilt(int exitCode, QProcess::ExitStatus exitStatus);
    void sendData(QByteArray);
private:
    void startOptionsBuild();
    bool closeDevice(int handle);
    bool closedDisk(int disk_handle);

    QProcess optionBuilder;
    LargeFileReader *worker;
    qhttp::server::QHttpResponse *res;
    LibInstaller *parent;
};

class LargeFileReader : public QThread {
Q_OBJECT
public:
    LargeFileReader(QObject * parent, QString source)
        :QThread(parent), source(source) {
    }

    void run();
signals:
    void doneReading(QByteArray);
private:
    QFile source;
};

#endif // CLIENTHANDLER_H
