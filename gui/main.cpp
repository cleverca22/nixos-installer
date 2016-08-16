#include <QApplication>
#include <QDebug>
//#include <QPlastiqueStyle>
#include <QStyleFactory>
#include "bootconfig.h"
#include "installmode.h"
#include "mainwindow.h"

#include "nixosjsonoptions.h"

int main(int argc, char **argv) {
    qDebug() << QStyleFactory::keys();
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication *app = new QApplication(argc,argv);
    if (app->arguments().contains("-test")) testing = true;
    //BootConfig *bc = new BootConfig();
    //bc->show();
    MainWindow *mw = new MainWindow();
    mw->show();
    //NixosJsonOptions *win = new NixosJsonOptions();
    //win->show();
    app->exec();
    delete mw;
    //do_test1(gpt);
    qDebug("shutting down");
    delete app;
    return 0;
}
