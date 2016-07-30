#include <QLayout>
#include <QDebug>

#include "mainwindow.h"
#include "installmode.h"
#include "bootconfig.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), installMode(0), bootConfig(0)
{
    QLayout *layout = this->layout();
    installMode = new InstallMode(this);
    setCentralWidget(installMode);
    QMetaObject::connectSlotsByName(this);
}

MainWindow::~MainWindow()
{
    delete bootConfig;
}

void MainWindow::on_installMode_install() {
    qDebug() << __func__;
    bootConfig = new BootConfig(this);
    setCentralWidget(bootConfig);
}

void MainWindow::on_installMode_recover() {
    qDebug() << __func__;
}

void MainWindow::on_installMode_modify() {
    qDebug() << __func__;
}
