#include <QCoreApplication>
#include <qhttpserver.hpp>
#include <qhttpserverconnection.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>
#include <QDateTime>
#include <signal.h>
#include <unistd.h>

using namespace qhttp::server;

class ClientHandler : public QObject {
public:
    ClientHandler(QHttpRequest *req, QHttpResponse *res) : QObject(req) {
        req->collectData(1024);
        req->onEnd([this, req, res]() {
            qDebug("  connection (#%llu): request from %s:%d\n  method: %s url: %s",
                   42,
                   req->remoteAddress().toUtf8().constData(),
                   req->remotePort(),
                   qhttp::Stringify::toString(req->method()),
                   qPrintable(req->url().toString())
                   );
            if ( req->collectedData().size() > 0 )
                 qDebug("  body (#%llu): %s",
                         42,
                         req->collectedData().constData()
                         );

             QString message =
                 QString("Hello World\n  packet count = %1\n  time = %2\n")
                 .arg(42)
                 .arg(QLocale::c().toString(QDateTime::currentDateTime(),
                                            "yyyy-MM-dd hh:mm:ss")
                 );

             res->setStatusCode(qhttp::ESTATUS_OK);
             res->addHeaderValue("content-length", message.size());
             res->end(message.toUtf8());

        });
    }
};

void catchUnixSignals(const std::vector<int>& quitSignals,
                      const std::vector<int>& ignoreSignals = std::vector<int>()) {

    auto handler = [](int sig) ->void {
        printf("\nquit the application (user request signal = %d).\n", sig);
        QCoreApplication::quit();
    };

    for ( int sig : ignoreSignals )
        signal(sig, SIG_IGN);

    for ( int sig : quitSignals )
        signal(sig, handler);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

    QString port("8080");

    QHttpServer server(&a);
    server.listen(port,[&](QHttpRequest *req, QHttpResponse *res) {
        new ClientHandler(req,res);
    });

    if (!server.isListening()) {
        qDebug() << QString("cant listen on %1").arg(port);
        return -1;
    }

    return a.exec();
}
