#include <QCoreApplication>
#include <qhttpserver.hpp>
#include <qhttpserverconnection.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>
#include <QThread>

#include <signal.h>
#include <unistd.h>
#include "libinstaller.h"

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

    QString document_root("/nas/installer/docroot");

    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

    LibInstaller installer;
    if (!installer.listen(document_root, 8080)) {
        qDebug() << "unable to listen on localhost:8080";
        return -1;
    }

    qDebug() << "main thread:" << QThread::currentThreadId();
    return a.exec();
}
