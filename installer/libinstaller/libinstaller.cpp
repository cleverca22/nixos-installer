#include <algorithm>
#include <QDebug>
#include <qhttpserver.hpp>

#include "libinstaller.h"
#include "clienthandler.h"


using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef server::message_ptr message_ptr;

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

void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
    if (msg->get_payload() == "stop-listening") {
        s->stop_listening();
        return;
    }

    try {
        s->send(hdl, msg->get_payload(), msg->get_opcode());
    } catch (const websocketpp::lib::error_code& e) {
        std::cout << "Echo failed because: " << e
                  << "(" << e.message() << ")" << std::endl;
    }
}

void on_http(server* s, class runner *parent, websocketpp::connection_hdl hdl) {
    server::connection_ptr con = s->get_con_from_hdl(hdl);

    std::string res = con->get_request_body();
    QString url = con->get_uri()->get_resource().c_str();
    std::string method = con->get_request().get_method();

    if (method == "GET") {
        QFileInfo fi(parent->docroot,url.mid(1));
        if (fi.isDir()) fi = QFileInfo(fi.absoluteFilePath(),"index.html");
        //qDebug() << url;
        //qDebug() << fi.absoluteFilePath();
        //qDebug() << "http thread:" << QThread::currentThreadId();
        if (fi.exists()) {
            QFile hnd(fi.absoluteFilePath());
            if (!hnd.open(QFile::ReadOnly)) {
                qDebug() << "unable to open";
                con->set_body("fail");
                con->set_status(websocketpp::http::status_code::internal_server_error);
                return;
            }
            QByteArray data = hnd.readAll();
            // TODO send cache control headers
            // TODO if not localhost, gzip things?
            con->set_body(std::string(data.data(),data.size()));
            con->set_status(websocketpp::http::status_code::ok);
            return;
        } else {
            std::string message("404");
            con->set_body(message);
            con->set_status(websocketpp::http::status_code::not_found);
        }
    } else if (method == "POST") {
        if (url.startsWith("/rpc/")) {
            QStringList parts = url.mid(1).split("/");
            qDebug() << parts;
            HandleRPCCall(&parent->handler,parts[1],con->get_request_body().c_str());
        }
    }

    std::stringstream ss;
    ss << "got HTTP request with " << res.size() << " bytes of body data." << method;

    con->set_body(ss.str());
    con->set_status(websocketpp::http::status_code::ok);
}

void runner::run() {
    try {
        serv.set_access_channels(websocketpp::log::alevel::all);
        serv.clear_access_channels(websocketpp::log::alevel::frame_payload);
        serv.init_asio();
        serv.set_message_handler(bind(&on_message,&serv,::_1,::_2));
        serv.set_http_handler(bind(&on_http,&serv,this,::_1));
        serv.listen(port);
        serv.start_accept();
        serv.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}

bool LibInstaller::listen(QString document_root, quint16 port) {
    docroot = document_root;
    runner * foo = new runner();
    foo->start();
    foo->port = port;
    foo->docroot = document_root;
    /*
    server = new QHttpServer(this);
    server->listen(QHostAddress("127.0.0.1"),port,[&](QHttpRequest *req, QHttpResponse *res) {
        new ClientHandler(this,req,res);
    });

    return server->isListening();
    */
    return true;
}
