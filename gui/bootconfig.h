#ifndef BOOTCONFIG_H
#define BOOTCONFIG_H

#include <QWidget>
#include <QProcess>
#include <QThread>

namespace Ui {
class BootConfig;
}

class DoInstall : public QThread {
    Q_OBJECT
    void run();

signals:
    void start_install();
public:
    QString device_path;
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

private slots:
    void config_loaded();
    void on_install_clicked();
    void start_install();
    void install_finished(int exitStatus, QProcess::ExitStatus status2);

private:
    Ui::BootConfig *ui;
    QProcess *xterm;
    DoInstall *thread;
};

#endif // BOOTCONFIG_H
