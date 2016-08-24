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
#include <QMetaMethod>
#include <QFileInfo>

#include "clienthandler.h"
#include "device.h"
#include "disk.h"
#include "libinstaller.h"

Q_DECLARE_METATYPE(QJsonObject*)

using namespace qhttp::server;
using namespace qhttp;
using namespace parted;

int handleCounter = 0;

QMap<int,Device*> open_devices;
QMap<int,Disk*> open_disks;

QMap<QString,QMetaMethod> methods;

void ClientLoadSlots() {
    int id = qRegisterMetaType<QJsonObject*>();
    const QMetaObject &moc = ClientHandler::staticMetaObject;
    for (int i = moc.methodOffset(); i < moc.methodCount(); i++) {
        const QMetaMethod &meth = moc.method(i);
        //qDebug() << meth.methodSignature() << meth.parameterCount();
        methods[QString(meth.name())] = meth;
    }
}

int ClientDumpMethods() {
    const QMetaObject &moc = ClientHandler::staticMetaObject;
    QStringList scripts;
    for (int i = moc.methodOffset(); i < moc.methodCount(); i++) {
        const QMetaMethod &meth = moc.method(i);
        QList<QByteArray> names = meth.parameterNames();
        QStringList args;
        args.append("cb");
        for (int i=1; i < names.count(); i++) {
            args.append(QString(names[i]));
        }
        QString part = QString("function %1(%2) {\n  var obj = {\n").arg(QString(meth.name())).arg(args.join(", "));
        QStringList items;
        for (int i=1; i < names.count(); i++) {
            items.append(QString("    %1: %1").arg(QString(names[i])));
        }
        part = part + items.join(",\n");
        part += QString("\n  };\n  RPC(obj,'%1',cb);\n}").arg(QString(meth.name()));
        scripts.append(part);
    }
    qDebug() << qPrintable(scripts.join("\n"));
}

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
            if (parts[1] == "openDevice") {
                int handle = openDevice("/"+parts.mid(2).join("/"));
                reply["handle"] = handle;
                good = true;
            }
            if (parts[1] == "closeDevice") {
                reply["status"] = closeDevice(parts[2].toInt());
                good = true;
            }
            if (parts[1] == "openDisk") {
                Disk *disk;
                int handle = openDisk(parts[2].toInt(), &disk);
                reply["handle"] = handle;
                bool valid = disk->disk != NULL;
                reply["valid"] = valid;
                if (valid) {
                    if (disk->disk->type == ped_disk_type_get("gpt")) reply["type"] = "gpt";
                    else if (disk->disk->type == ped_disk_type_get("msdos")) reply["type"] = "msdos";
                }
                good = true;
            }
            //if (parts[1] == "makeLabel") {
            //    makeLabel(parts[2].toInt());
            //}
            /*if (methods.contains(parts[1])) {
                RPCMethod meth = methods[parts[1]];
                QGenericArgument arg1;
                arg1 = Q_ARG(QJsonObject*,&reply);
                if (meth.has_arg) {
                    int x;
                    if (parts.count() != 3) {
                        reply["error"] = "missing argument";
                        good = true;
                    } else {
                        QGenericArgument arg2;
                        switch (meth.arg_type) {
                        case QMetaType::Int:
                            x = parts[2].toInt();
                            arg2 = Q_ARG(int,x);
                            break;
                        }

                        meth.method.invoke(this,arg1,arg2);
                        good = true;
                    }
                } else meth.method.invoke(this,arg1);
            }*/
            if (good) {
                QJsonDocument doc(reply);
                QByteArray data = doc.toJson(QJsonDocument::Compact);
                res->setStatusCode(ESTATUS_OK);
                res->addHeaderValue("content-length", data.size());
                res->addHeader("content-type","application/json");
                res->addHeader("connection", "keep-alive");
                // TODO send cache control headers
                res->end(data);
            } else {
                QString message("404");
                res->setStatusCode(ESTATUS_NOT_FOUND);
                res->addHeaderValue("content-length", message.size());
                res->addHeader("connection", "keep-alive");
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
                res->addHeader("connection", "keep-alive");
                // TODO send cache control headers
                // TODO if not localhost, gzip things?
                res->end(data);
            } else {
                QString message("404");
                res->setStatusCode(ESTATUS_NOT_FOUND);
                res->addHeaderValue("content-length", message.size());
                res->addHeader("connection", "keep-alive");
                res->end(message.toUtf8());
            }
        }
    } else if (req->method() == THttpMethod::EHTTP_POST) {
        req->collectData(-1);
        req->onEnd([this,req,res]() {
            QJsonObject reply;
            QUrl url = req->url();
            if (url.path().startsWith("/rpc/")) {
                QByteArray rawpostbody = req->collectedData();
                QJsonDocument doc = QJsonDocument::fromJson(rawpostbody);
                QJsonObject obj = doc.object();
                QStringList parts = url.path().mid(1).split("/");
                qDebug() << rawpostbody << parts;
                qDebug() << methods.keys();
                if (methods.contains(parts[1])) {
                    qDebug() << "its a valid RPC method";
                    QMetaMethod meth = methods[parts[1]];
                    QList<QByteArray> names = meth.parameterNames();
                    qDebug() << "which wants" << names.count() << "arguments";
                    QList<QGenericArgument> args;
                    args.append(Q_ARG(QJsonObject*,&reply));
                    QStringList temp;
                    QString t2;
                    for (int i=0; i < names.count(); i++) {
                        QString name(names[i]);
                        if (name == "reply") continue;
                        int type = meth.parameterType(i);
                        qDebug() << name << obj[name];
                        switch (type) {
                        case QMetaType::Int:
                            qDebug() << "adding int";
                            args.append(Q_ARG(int,obj[name].toInt()));
                            break;
                        case QMetaType::QString:
                            temp.append(t2 = obj[name].toString());
                            qDebug() << "adding string" << t2;
                            args.append(Q_ARG(QString,t2));
                            break;
                        }
                    }
                    qDebug() << "calling with" << args.count();
                    qDebug() << *((QString*)args[2].data());
                    switch (names.count()) {
                    case 3:
                        meth.invoke(this,args[0],args[1],args[2]);
                        break;
                    }
                }
            }
            res->end(QByteArray());
        });
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

HttpReply HandleRPCCall(ClientHandler *handler, QString function, QByteArray body) {
    HttpReply resp;

    QJsonObject reply;
    QJsonDocument doc = QJsonDocument::fromJson(body);
    QJsonObject obj = doc.object();
    qDebug() << body;
    qDebug() << methods.keys();
    if (methods.contains(function)) {
        qDebug() << "its a valid RPC method";
        QMetaMethod meth = methods[function];
        QList<QByteArray> names = meth.parameterNames();
        qDebug() << "which wants" << names.count() << "arguments";
        QList<QGenericArgument> args;
        args.append(Q_ARG(QJsonObject*,&reply));
        QStringList temp;
        QString t2;
        for (int i=0; i < names.count(); i++) {
            QString name(names[i]);
            if (name == "reply") continue;
            int type = meth.parameterType(i);
            qDebug() << name << obj[name];
            switch (type) {
            case QMetaType::Int:
                qDebug() << "adding int";
                args.append(Q_ARG(int,obj[name].toInt()));
                break;
            case QMetaType::QString:
                temp.append(t2 = obj[name].toString());
                qDebug() << "adding string" << t2;
                args.append(Q_ARG(QString,t2));
                break;
            }
        }
        qDebug() << "calling with" << args.count();
        qDebug() << *((QString*)args[2].data());
        switch (names.count()) {
        case 3:
            meth.invoke(handler,args[0],args[1],args[2]);
            break;
        }
    }
    resp.reply_body = "";
    return resp;
}

bool ClientHandler::makeLabel(QJsonObject *, int handle, QString type) {
    qDebug() << "invoked" << handle << type;
    return true;
}

int ClientHandler::openDisk(int device_handle, Disk **out) {
    if (open_devices.contains(device_handle)) {
        Device *dev = open_devices[device_handle];
        Disk *disk = new Disk(dev);
        int ret = handleCounter++;
        open_disks[ret] = disk;
        *out = disk;
        return ret;
    } else return -1;
}

int ClientHandler::openDevice(QString device) {
    Device *dev = new Device(device);
    int ret = -1;

    if (!dev->open()) {
        qDebug() << "unable to open disk";
        delete dev;
        return -1;
    }

    ret = handleCounter++;
    open_devices[ret] = dev;

    return ret;
}

bool ClientHandler::closeDevice(int handle) {
    if (open_devices.contains(handle)) {
        Device *dev = open_devices[handle];
        if (!dev->close()) {
            qDebug() << "unable to close disk";
            return false;
        }
        open_devices.remove(handle);
        return true;
    } else return false;
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
    res->addHeader("connection", "keep-alive");
    res->end(data);
    qDebug() << "send done" << QDateTime::currentDateTime();
}
