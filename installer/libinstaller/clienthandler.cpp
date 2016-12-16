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

#include <assert.h>
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

static bool invoke(ClientHandler *self, QMetaMethod method, QList<QGenericArgument> &args) {
    QList<QByteArray> names = method.parameterNames();
    qDebug() << "calling with" << args.count() << names.count();
    //qDebug() << *((QString*)args[2].data());
    bool ret = false;
    switch (names.count()) {
    case 2:
        qDebug() << "two";
        method.invoke(self, Qt::DirectConnection, Q_RETURN_ARG(bool, ret), args[0], args[1]);
        return ret;
    case 3:
        qDebug() << "three";
        method.invoke(self, Qt::DirectConnection, Q_RETURN_ARG(bool, ret), args[0], args[1], args[2]);
        return ret;
    case 4:
        method.invoke(self, Qt::DirectConnection, Q_RETURN_ARG(bool, ret), args[0], args[1], args[2], args[3]);
        return ret;
    default:
        assert(false);
    }
    return false;
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
    return 0;
}

ClientHandler::ClientHandler(LibInstaller *parent, QHttpRequest *req, QHttpResponse *res)
    :QObject(req), res(res), parent(parent) {
    if (req->method() == THttpMethod::EHTTP_GET) {
        QUrl url = req->url();
        if (url.path().startsWith("/rpc/")) {
            QStringList parts = url.path().mid(1).split("/");
            qDebug() << parts << QDateTime::currentDateTime();
            QJsonObject reply;
            bool good = false;
            if (parts[1] == "devices") {
                QJsonArray devices;
                ped_device_probe_all();
                for (PedDevice *dev = ped_device_get_next(NULL); dev; dev = ped_device_get_next(dev)) {
                    qDebug() << dev->model << dev->path;
                    devices.append(dev->path);
                }
                if (devices.count() == 0) {
                    devices.append("/tmp/dummy1.img");
                    devices.append("/tmp/dummy2.img");
                    devices.append("/tmp/dummy3.img");
                    devices.append("/tmp/dummy4.img");
                    devices.append("/tmp/dummy5.img");
                    devices.append("/tmp/dummy6.img");
                }
                reply["devices"] = devices;
                good = true;
            }
            if (parts[1] == "options.json") {
                startOptionsBuild();
                return;
            }
            //if (parts[1] == "openDevice") {
            //    int handle = openDevice("/"+parts.mid(2).join("/"));
            //    reply["handle"] = handle;
            //    good = true;
            //}
            if (parts[1] == "closeDevice") {
                reply["status"] = closeDevice(parts[2].toInt());
                good = true;
            }
            /*if (parts[1] == "openDisk") {
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
            }*/
            //if (parts[1] == "makeLabel") {
            //    makeLabel(parts[2].toInt());
            //}
            /*if (methods.contains(parts[1])) {
                QMetaMethod meth = methods[parts[1]];
                QList<QByteArray> names = meth.parameterNames();
                qDebug() << "which wants" << names.count() << "arguments";
                QList<QGenericArgument> args;
                //arg1 = Q_ARG(QJsonObject*,&reply);
                meth.method.invoke(this, args);
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
                    QJsonObject *ref = &reply;
                    args.append(Q_ARG(QJsonObject*, ref));
                    qDebug() << "reply is at" << &reply;

                    // Q_ARG maintains a reference to its arguments
                    // function return values usualy break things badly
                    //
                    // these numbers must be large enough to contain all
                    // arguments defined in the header
                    QString strings[10];
                    int ints[10];
                    int string_count = 0, int_count = 0;
                    for (int i=0; i < names.count(); i++) {
                        QString name(names[i]);
                        if (name == "reply") continue;
                        int type = meth.parameterType(i);
                        qDebug() << name << obj[name];
                        switch (type) {
                        case QMetaType::Int:
                            qDebug() << "adding int";
                            ints[int_count] = obj[name].toInt();
                            args.append(Q_ARG(int, ints[int_count]));
                            int_count++;
                            break;
                        case QMetaType::QString:
                            strings[string_count] = obj[name].toString();
                            qDebug() << "adding string" << strings[string_count];
                            args.append(Q_ARG(QString,strings[string_count]));
                            string_count++;
                            break;
                        }
                    }
                    bool ret = invoke(this, meth, args);
                    QJsonDocument doc(reply);
                    if (ret) {
                        res->setStatusCode(ESTATUS_OK);
                        QByteArray data = doc.toJson(QJsonDocument::Compact);
                        res->addHeaderValue("content-length", data.size());
                        res->addHeader("content-type","application/json");
                        res->addHeader("connection", "keep-alive");
                        res->end(data);
                        return;
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

bool ClientHandler::createPartition(QJsonObject *reply, int disk_handle, int start, int size) {
    qDebug() << __func__ << disk_handle << start << size;
    PedFileSystemType *ext4 = ped_file_system_type_get("ext4");
    Disk *disk = open_disks[disk_handle];
    if (!disk) return false;
    
    PedConstraint *constraint = ped_constraint_any(disk->dev->dev);

    auto partition = ped_partition_new(disk->disk, PED_PARTITION_NORMAL, ext4, start, start+size);
    if (QString("gpt") == disk->disk->type->name) {
        ped_partition_set_name(partition, "test name");
        ped_partition_set_flag(partition, PED_PARTITION_BIOS_GRUB, 1);
    }
    if (!ped_disk_add_partition(disk->disk, partition, constraint)) {
        qDebug() << "error adding partition";
        return false;
    }
    ped_disk_commit(disk->disk);
    return true;
}

bool ClientHandler::testConfig(QJsonObject *reply, QString configuration) {
    QJsonObject &out = *reply;
    qDebug() << parent->tempdir.path();
    QFile config(parent->tempdir.path()+"/configuration.nix");
    config.open(QFile::WriteOnly);
    QByteArray bytes = configuration.toLocal8Bit();
    config.write(bytes.data(),bytes.size());
    config.close();
    QProcess dryRun;
    QStringList args;
    args << "-I" << QString("nixos-config=%1").arg(parent->tempdir.path()+"/configuration.nix");
    args << "dry-build";
    dryRun.setProcessChannelMode(QProcess::MergedChannels);
    dryRun.setStandardInputFile(QProcess::nullDevice());
    dryRun.start("nixos-rebuild", args);
    dryRun.waitForFinished();
    out["output"] = QString(dryRun.readAll());
    return true;
}

bool ClientHandler::makeLabel(QJsonObject *reply, int handle, QString type) {
    QJsonObject &out = *reply;
    qDebug() << "invoked" << handle << type;
    PedDiskType *tbl_type = ped_disk_type_get(qPrintable(type));
    qDebug() << "type:" << tbl_type;
    if (!tbl_type) return false;

    Device *dev = open_devices[handle];

    // close the old disk handle
    QMapIterator<int, Disk*> i(open_disks);
    while (i.hasNext()) {
        i.next();
        if (i.value()->dev == dev) {
            delete i.value();
            open_disks.remove(i.key());
            break;
        }
    }

    // and then make a new empty disk handle
    auto disk = ped_disk_new_fresh(dev->dev, tbl_type);
    ped_disk_commit(disk);
    Disk *disk2 = new Disk(disk);
    int ret = handleCounter++;
    open_disks[ret] = disk2;
    out["handle"] = ret;
    out["valid"] = true;
    if (disk2->disk->type == ped_disk_type_get("gpt")) out["type"] = "gpt";
    else if (disk2->disk->type == ped_disk_type_get("msdos")) out["type"] = "msdos";
    return true;
}

/*HttpReply HandleRPCCall(ClientHandler *handler, QString function, QByteArray body) {
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
}*/


bool ClientHandler::openDisk(QJsonObject *reply, int device_handle) {
    QJsonObject &out = *reply;
    if (open_devices.contains(device_handle)) {
        Device *dev = open_devices[device_handle];
        int handle = -1;
        Disk *disk = NULL;

        QMapIterator<int, Disk*> i(open_disks);
        while (i.hasNext()) {
            i.next();
            if (i.value()->dev == dev) {
                disk = i.value();
                handle = i.key();
                break;
            }
        }

        if (!disk) {
            disk = new Disk(dev);
            handle = handleCounter++;
            open_disks[handle] = disk;
        }

        out["handle"] = handle;
        if (disk->disk) {
            out["valid"] = true;
            if (disk->disk->type == ped_disk_type_get("gpt")) out["type"] = "gpt";
            else if (disk->disk->type == ped_disk_type_get("msdos")) out["type"] = "msdos";
            else out["type"] = disk->disk->type->name;
        } else {
            out["valid"] = false;
            open_disks.remove(handle);
            delete disk;
        }
        return true;
    } else return false;
}

bool ClientHandler::listPartitions(QJsonObject *reply, int disk_handle) {
    QJsonObject &out = *reply;
    PedPartition *part;
    Disk *disk = open_disks[disk_handle];
    if (!disk) return false;
    QJsonArray partitions;

    for (part = ped_disk_next_partition(disk->disk, NULL);
            part;
            part = ped_disk_next_partition(disk->disk, part)) {
        if (!ped_partition_is_active(part)) continue;
        QJsonObject part_out;
        QString name(ped_partition_get_name(part));
        qDebug() << "partition" << part->num << name;
        part_out["name"] = name;
        part_out["start"] = part->geom.start;
        part_out["end"] = part->geom.end;
        part_out["length"] = part->geom.length;
        part_out["num"] = part->num;
        QJsonArray flags;
        for (int f = PED_PARTITION_FIRST_FLAG; f < PED_PARTITION_LAST_FLAG; f++) {
            if (ped_partition_get_flag(part,(PedPartitionFlag)f)) {
                QString flag_name(ped_partition_flag_get_name((PedPartitionFlag)f));
                qDebug() << "flag" << flag_name << "is set";
                flags.append(flag_name);
            }
        }
        part_out["flags"] = flags;
        partitions.append(part_out);
    }
    out["partitions"] = partitions;
    return true;
}

bool ClientHandler::openDevice(QJsonObject *reply, QString device) {
    QJsonObject &out = *reply;
    Device *dev = NULL;
    int handle = -1;

    QMapIterator<int, Device*> i(open_devices);
    while (i.hasNext()) {
        i.next();
        if (i.value()->devnode == device) {
            dev = i.value();
            handle = i.key();
            break;
        }
    }

    if (!dev) {
        dev = new Device(device);

        if (!dev->open()) {
            qDebug() << "unable to open disk";
            delete dev;
            return false;
        }

        handle = handleCounter++;
        qDebug() << "new handle" << handle;
        open_devices[handle] = dev;
    }

    out["handle"] = handle;
    out["sector_size"] = dev->dev->sector_size;
    out["phys_sector_size"] = dev->dev->phys_sector_size;
    out["length"] = dev->dev->length;
    QJsonDocument doc(out);

    return true;
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
    // a bug in http-parser/qhttp causes the connection to terminate early if this returns
    // all async code has been neutered to make the bug happy
    //connect(&optionBuilder,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(optionsBuilt(int,QProcess::ExitStatus)));

    QStringList args;
    args.append("<nixpkgs/nixos/release.nix>");
    args.append("-A");
    args.append("options");
    args.append("-o");
    args.append(parent->tempdir.path() + "/options");
    optionBuilder.start("nix-build",args);
    optionBuilder.waitForFinished();
    optionsBuilt(optionBuilder.exitCode(), optionBuilder.exitStatus());
}

void ClientHandler::optionsBuilt(int exitCode,QProcess::ExitStatus exitStatus) {
    qDebug() << exitCode << exitStatus << QDateTime::currentDateTime();
    qDebug() << optionBuilder.readAllStandardError();
    qDebug() << optionBuilder.readAllStandardOutput();
    worker = new LargeFileReader(this,parent->tempdir.path() + "/options/share/doc/nixos/options.json");
    connect(worker,SIGNAL(doneReading(QByteArray)),this,SLOT(sendData(QByteArray)));
    //worker->start();
    worker->run();
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
