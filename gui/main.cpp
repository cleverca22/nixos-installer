#include <QApplication>
#include <QDebug>
//#include <QPlastiqueStyle>
#include <QStyleFactory>
#include "bootconfig.h"
#include "installmode.h"
#include "mainwindow.h"

int main(int argc, char **argv) {
    //qDebug() << QStyleFactory::keys();
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication *app = new QApplication(argc,argv);
    //BootConfig *bc = new BootConfig();
    //bc->show();
    MainWindow *mw = new MainWindow();
    mw->show();
    app->exec();
    //do_test1(gpt);
    qDebug("shutting down");
    return 0;
}
