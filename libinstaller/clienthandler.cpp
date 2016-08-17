#include <qhttpserverconnection.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDateTime>
#include <QLocale>
#include <QDebug>
#include <QThread>

#include "clienthandler.h"

using namespace qhttp::server;
using namespace qhttp;

ClientHandler::ClientHandler(LibInstaller *parent, QHttpRequest *req, QHttpResponse *res)
    :QObject(req), res(res) {
    if (req->method() == THttpMethod::EHTTP_GET) {
        QUrl url = req->url();
        if (url.path().startsWith("/rpc/")) {
            QStringList parts = url.path().mid(1).split("/");
            qDebug() << parts << QDateTime::currentDateTime();
            QJsonObject reply;
            bool good = false;
            if (parts[1] == "devices") {
                QJsonArray devices;
                devices.append("/tmp/dummy.img");
                reply["devices"] = devices;
                good = true;
            }
            if (parts[1] == "options.json") {
                startOptionsBuild();
                return;
            }
            if (good) {
                QJsonDocument doc(reply);
                QByteArray data = doc.toJson(QJsonDocument::Compact);
                res->setStatusCode(ESTATUS_OK);
                res->addHeaderValue("content-length", data.size());
                res->addHeader("content-type","application/json");
                // TODO send cache control headers
                res->end(data);
            } else {
                QString message("404");
                res->setStatusCode(ESTATUS_NOT_FOUND);
                res->addHeaderValue("content-length", message.size());
                res->end(message.toUtf8());
            }
        } else {
            QFileInfo fi(parent->docroot,req->url().path().mid(1));
            if (fi.isDir()) fi = QFileInfo(fi.absoluteFilePath(),"index.html");
            qDebug() << req->url();
            qDebug() << fi.absoluteFilePath();
            qDebug() << "http thread:" << QThread::currentThreadId();
            if (fi.exists()) {
                QFile hnd(fi.absoluteFilePath());
                if (!hnd.open(QFile::ReadOnly)) {
                    qDebug() << "unable to open";
                    res->end("fail");
                    return;
                }
                QByteArray data = hnd.readAll();
                res->setStatusCode(ESTATUS_OK);
                res->addHeaderValue("content-length", data.size());
                // TODO send cache control headers
                // TODO if not localhost, gzip things?
                res->end(data);
            } else {
                QString message("404");
                res->setStatusCode(ESTATUS_NOT_FOUND);
                res->addHeaderValue("content-length", message.size());
                res->end(message.toUtf8());
            }
        }
    } else {
        req->collectData(1024);
        req->onEnd([this, req, res]() {
            qDebug("  connection (#%d): request from %s:%d\n  method: %s url: %s",
                   42,
                   req->remoteAddress().toUtf8().constData(),
                   req->remotePort(),
                   qhttp::Stringify::toString(req->method()),
                   qPrintable(req->url().toString())
                   );
            if ( req->collectedData().size() > 0 )
                qDebug("  body (#%d): %s",
                       42,
                       req->collectedData().constData()
                       );

            QString message =
                    QString("Hello World\n  packet count = %1\n  time = %2\n")
                    .arg(42)
                    .arg(QLocale::c().toString(QDateTime::currentDateTime(),"yyyy-MM-dd hh:mm:ss"));

                 res->setStatusCode(qhttp::ESTATUS_OK);
                 res->addHeaderValue("content-length", message.size());
                 res->end(message.toUtf8());
        });
    }
}

void ClientHandler::startOptionsBuild() {
    connect(&optionBuilder,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(optionsBuilt(int,QProcess::ExitStatus)));

    QStringList args;
    args.append("<nixpkgs/nixos/release.nix>");
    args.append("-A");
    args.append("options");
    args.append("-o");
    args.append("/tmp/options");
    optionBuilder.start("nix-build",args);
}

void ClientHandler::optionsBuilt(int exitCode,QProcess::ExitStatus exitStatus) {
    qDebug() << exitCode << exitStatus << QDateTime::currentDateTime();
    qDebug() << optionBuilder.readAllStandardError();
    qDebug() << optionBuilder.readAllStandardOutput();
    worker = new LargeFileReader(this,"/tmp/options/share/doc/nixos/options.json");
    connect(worker,SIGNAL(doneReading(QByteArray)),this,SLOT(sendData(QByteArray)));
    worker->start();
}

void LargeFileReader::run() {
    qDebug() << "read start" << QDateTime::currentDateTime();
    source.open(QFile::ReadOnly);
    QByteArray data = source.readAll();
    qDebug() << "read done" << QDateTime::currentDateTime();
    emit doneReading(data);
}

void ClientHandler::sendData(QByteArray data) {
    qDebug() << "send start" << QDateTime::currentDateTime();
    res->setStatusCode(ESTATUS_OK);
    res->addHeaderValue("content-length", data.size());
    res->addHeader("content-type","application/json");
    res->end(data);
    qDebug() << "send done" << QDateTime::currentDateTime();
}
