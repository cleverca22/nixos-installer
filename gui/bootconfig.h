#ifndef BOOTCONFIG_H
#define BOOTCONFIG_H

#include <QWidget>
#include <QProcess>
#include <QThread>

#include "libinstaller.h"

namespace Ui {
class BootConfig;
}

class LibInstaller;
class BootConfig;

class DoInstall : public QThread {
    Q_OBJECT
    void run();

signals:
    void start_install();
public:
    QString device_path;
    BootConfig *hostWindow;
};

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

    QList<installer::MountPoint> mounts;
    LibInstaller *installer;
private slots:
    void config_loaded();
    void on_install_clicked();
    void start_install();
    void install_finished(int exitStatus, QProcess::ExitStatus status2);
    void debug1();

private:
    Ui::BootConfig *ui;
    QProcess *xterm;
    DoInstall *thread;
};

#endif // BOOTCONFIG_H
