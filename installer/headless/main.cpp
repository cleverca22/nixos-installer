#include <QCoreApplication>
#include <qhttpserver.hpp>
#include <qhttpserverconnection.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>
#include <QThread>

#include <signal.h>
#include <unistd.h>
#include "libinstaller.h"
#include "clienthandler.h"

using namespace qhttp::server;

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

    //QString document_root(OUT"/share/docroot");
    QString document_root("/home/clever/apps/nixos-installer/docroot");
    qDebug() << document_root;

    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
    ClientLoadSlots();

    if ((a.arguments().count() >= 2) && (a.arguments()[1] == "--dump-methods")) {
        return ClientDumpMethods();
    }

    if ((a.arguments().count() == 3) && (a.arguments()[1] == "--docroot")) document_root = a.arguments()[2];

    LibInstaller installer;
    if (!installer.listen(document_root, "0.0.0.0", 8080)) {
        qDebug() << "unable to listen on localhost:8080";
        return -1;
    }

    qDebug() << "main thread:" << QThread::currentThreadId();
    return a.exec();
}
