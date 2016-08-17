#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QProcess>
#include <QThread>

#include "libinstaller.h"

namespace qhttp {
namespace server {
class QHttpRequest;
class QHttpResponse;
class QHttpServer;
}
}

class LargeFileReader;

class ClientHandler : public QObject {
Q_OBJECT
public:
    ClientHandler(LibInstaller *parent, qhttp::server::QHttpRequest *req, qhttp::server::QHttpResponse *res);

private slots:
    void optionsBuilt(int exitCode, QProcess::ExitStatus exitStatus);
    void sendData(QByteArray);
private:
    void startOptionsBuild();

    QProcess optionBuilder;
    LargeFileReader *worker;
    qhttp::server::QHttpResponse *res;
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
