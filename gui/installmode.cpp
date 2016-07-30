#include <QDebug>

#include "installmode.h"
#include "ui_installmode.h"

InstallMode::InstallMode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InstallMode)
{
    ui->setupUi(this);
    connect(ui->install,SIGNAL(clicked()),this,SIGNAL(install()));
    connect(ui->modify,SIGNAL(clicked()),this,SIGNAL(modify()));
    connect(ui->recover,SIGNAL(clicked()),this,SIGNAL(recover()));
    setObjectName("installMode");
}

InstallMode::~InstallMode()
{
    delete ui;
}
