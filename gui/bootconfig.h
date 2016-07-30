#ifndef BOOTCONFIG_H
#define BOOTCONFIG_H

#include <QWidget>
#include <QProcess>

namespace Ui {
class BootConfig;
}

class BootConfig : public QWidget
{
    Q_OBJECT

public:
    explicit BootConfig(QWidget *parent = 0);
    ~BootConfig();

private slots:
    void on_install_clicked();
private:
    Ui::BootConfig *ui;
    QProcess *xterm;
};

#endif // BOOTCONFIG_H
