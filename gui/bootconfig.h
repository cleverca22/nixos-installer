#ifndef BOOTCONFIG_H
#define BOOTCONFIG_H

#include <QWidget>

namespace Ui {
class BootConfig;
}

class BootConfig : public QWidget
{
    Q_OBJECT

public:
    explicit BootConfig(QWidget *parent = 0);
    ~BootConfig();

private:
    Ui::BootConfig *ui;
};

#endif // BOOTCONFIG_H
