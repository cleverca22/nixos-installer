#include "bootconfig.h"
#include "ui_bootconfig.h"

BootConfig::BootConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BootConfig)
{
    ui->setupUi(this);
}

BootConfig::~BootConfig()
{
    delete ui;
}
