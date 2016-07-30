#ifndef BOOTCONFIG_H
#define BOOTCONFIG_H

#include <QWidget>
#include <QProcess>

namespace Ui {
class BootConfig;
}

class PartitionState {
public:
    QString boot_path, root_path;
};

class BootConfig : public QWidget
{
    Q_OBJECT

public:
    explicit BootConfig(QWidget *parent = 0);
    ~BootConfig();

private slots:
    void config_loaded();
    void on_install_clicked();

private:
    Ui::BootConfig *ui;
    QProcess *xterm;
};

#endif // BOOTCONFIG_H
