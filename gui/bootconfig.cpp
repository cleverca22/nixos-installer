#include <QDebug>
#include <QFile>
#include <QTimer>

#include "bootconfig.h"
#include "ui_bootconfig.h"
#include <parted/parted.h>
#include "nixosjsonoptions.h"
#include "mainwindow.h"

#include "device.h"

enum class label_type {
    gpt, mbr
};

PartitionState do_test1(PedDevice *dev, label_type labelType);
void partition_drives(PartitionState state);

BootConfig::BootConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BootConfig)
{
    ui->setupUi(this);
    NixosJsonOptions *win = new NixosJsonOptions;
    connect(win,SIGNAL(config_loaded()),this,SLOT(config_loaded()));

    ui->container->layout()->addWidget(win);
    qDebug() << "id is" << ui->xterm->winId();
    xterm = new QProcess;
}

void BootConfig::config_loaded() {
    if (testing) {
        on_install_clicked();
    }
}

BootConfig::~BootConfig()
{
    qDebug() << xterm << xterm->pid();
    xterm->terminate();
    xterm->waitForFinished(1000);
    xterm->kill();
    delete ui;
}

void mount_things(PartitionState state) {
    QProcess::execute(QString("mount -v %1 /mnt/").arg(state.root_path));
    QProcess::execute("mkdir -pv /mnt/boot");
    QProcess::execute(QString("mount -v %1 /mnt/boot").arg(state.boot_path));
}

void DoInstall::run() {
    if (testing) device_path = "/dev/vda";

    parted::Device dev(device_path);

    if (!dev.open()) {
        qDebug() << "unable to open disk";
    }
    PartitionState state = do_test1(dev.dev, label_type::gpt);
    if (!dev.close()) {
        qDebug() << "unable to close disk";
    }
    partition_drives(state);

    hostWindow->mounts.clear();
    hostWindow->mounts.append(installer::MountPoint(state.root_path,"/mnt"));
    hostWindow->mounts.append(installer::MountPoint(state.boot_path,"/mnt/boot"));

    hostWindow->installer = new LibInstaller;
    if (!hostWindow->installer->mountPaths(hostWindow->mounts)) {
        qDebug() << "failed to mount all paths";
        abort();
    }
    QProcess::execute("nixos-generate-config --root /mnt");
    QFile config("/mnt/etc/nixos/configuration.nix");
    config.open(QFile::WriteOnly);
    QString cfg = QString("{ lib, config, pkgs, ...}:\n"
                          "{\n"
                          "  imports = [\n"
                          "    ./hardware-configuration.nix\n"
                          "    <nixpkgs/nixos/modules/testing/test-instrumentation.nix>\n"
                          "  ];\n"
                          "  users.mutableUsers = false;\n"
                          "  boot.loader.grub.device = \"/dev/vda\";\n"
                          "}");
    QByteArray bytes = cfg.toLocal8Bit();
    config.write(bytes.data(),bytes.size());
    config.close();
    emit start_install();
}

void BootConfig::start_install() {
    bool to_stdout = false;
    QStringList args;
    xterm->setProcessChannelMode(QProcess::ForwardedChannels);
    xterm->setStandardInputFile(QProcess::nullDevice());
    if (!to_stdout) {
        args.append("-into");
        args.append(QString("%1").arg(ui->xterm->winId()));
        args.append("-geometry");
        args.append("484x316");
        args.append("-e");
        args.append("nixos-install");
    }
    args.append("--root");
    args.append("/mnt");
    connect(xterm,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(install_finished(int,QProcess::ExitStatus)));
    if (to_stdout) {
        xterm->start("nixos-install",args);
    } else {
        xterm->start("xterm",args);
    }
    QTimer *foo = new QTimer();
    connect(foo,SIGNAL(timeout()),this,SLOT(debug1()));
    foo->setInterval(400 * 1000);
    foo->setSingleShot(true);
    foo->start();
}

void BootConfig::debug1() {
    QProcess::execute("ps -eH x");
}

void BootConfig::install_finished(int exitStatus, QProcess::ExitStatus status2) {
    qDebug() << __func__ << exitStatus << status2;
    if (exitStatus) {
        // auto-quit on failure too
        installer->umountPaths(mounts);
        if (testing) QApplication::exit(1);
    }
    if (testing) { // quit app on success
        installer->umountPaths(mounts);
        QApplication::exit(0);
    }
}

void BootConfig::on_install_clicked() {
    qDebug() << "do things";
    ui->tabWidget->setCurrentIndex(1);
    thread = new DoInstall();
    connect(thread,SIGNAL(start_install()),this,SLOT(start_install()));
    thread->device_path = ui->target->currentText();
    thread->hostWindow = this;
    thread->start();
}

void partition_drives(PartitionState state) {
    QProcess::execute(QString("mkfs.ext4 %1").arg(state.boot_path));
    QProcess::execute(QString("mkfs.ext4 %1").arg(state.root_path));
}

PartitionState do_test1(PedDevice *dev, label_type labelType) {
    PartitionState state;
    PedGeometry geom;
    PedDisk *disk;
    PedPartition *part;
    PedPartition *grub_partition = 0, *boot_partition = 0, *root_partition = 0;
    PedDiskType *type;
    PedFileSystemType *ext4 = ped_file_system_type_get("ext4");
    bool dirty = false;
    PedSector start = 0, end = 0;

    /*if (!ped_geometry_init(&geom,dev,0,dev->length)) {
        qDebug() << "unable to init geom";
        return;
    }*/

    disk = ped_disk_new(dev);
    /*type = ped_disk_probe(dev);
    if (type) {
        qDebug() << "current partition type:" << type->name;
        disk = type->ops->alloc(dev);
        if (!type->ops->read(disk)) {
            qDebug() << "failed to read gpt tables";
            return;
        }
    }*/
    if (!disk) {
        qDebug() << "no tables detected";
        if (labelType == label_type::gpt) {
            type = ped_disk_type_get("gpt");
        } else if (labelType == label_type::mbr) {
            type = ped_disk_type_get("msdos");
        }
        disk = ped_disk_new_fresh(dev,type);
        ped_disk_commit(disk);
    }
    if (disk) {
        for (part = ped_disk_next_partition(disk,NULL);
             part;
             part = ped_disk_next_partition(disk,part)) {
            if (!ped_partition_is_active(part)) continue;
            QString name(ped_partition_get_name(part));
            qDebug() << "partition" << part->num << name;
            if (name == "boot") boot_partition = part;
            if (name == "root") root_partition = part;
            if (ped_partition_get_flag(part,PED_PARTITION_BIOS_GRUB)) grub_partition = part;
            for (int f = PED_PARTITION_FIRST_FLAG; f < PED_PARTITION_LAST_FLAG; f++) {
                if (ped_partition_get_flag(part,(PedPartitionFlag)f)) {
                    QString flag_name(ped_partition_flag_get_name((PedPartitionFlag)f));
                    qDebug() << "flag" << flag_name << "is set";
                }
            }
        }

        PedConstraint *constraint = ped_constraint_any(dev);
        if (!grub_partition) {
            start = (1024*1024) / dev->sector_size;
            end = ((1024*1024) / dev->sector_size) + start;
            qDebug() << "creating" << start << end;
            grub_partition = ped_partition_new(disk,PED_PARTITION_NORMAL,ext4,start,end);
            if (labelType == label_type::gpt) {
                ped_partition_set_name(grub_partition,"bios boot");
                ped_partition_set_flag(grub_partition,PED_PARTITION_BIOS_GRUB,1);
            }
            if (!ped_disk_add_partition(disk,grub_partition,constraint)) {
                qDebug() << "error adding partition";
            }
            dirty = true;
        }

        if (!boot_partition) {
            start = (1024*1024*2) / dev->sector_size;
            end = ((1024*1024*128) / dev->sector_size) + start;
            qDebug() << "creating" << start << end;
            boot_partition = ped_partition_new(disk,PED_PARTITION_NORMAL,NULL,start,end);
            if (labelType == label_type::gpt) {
                ped_partition_set_name(boot_partition,"boot");
            }
            //ped_partition_set_flag(boot_partition,PED_PARTITION_BOOT,1);
            if (!ped_disk_add_partition(disk,boot_partition,constraint)) {
                qDebug() << "error adding partition";
            }
            dirty = true;
        }
        if (!root_partition) {
            start = (1024*1024*129) / dev->sector_size;
            end = dev->length;
            qDebug() << "creating" << start << end;
            root_partition = ped_partition_new(disk,PED_PARTITION_NORMAL,ext4,start,end);
            if (labelType == label_type::gpt) {
                ped_partition_set_name(root_partition,"root");
                //ped_partition_set_flag(root_partition,PED_PARTITION_ROOT,1);
            }
            if (!ped_disk_add_partition(disk,root_partition,constraint)) {
                qDebug() << "error adding partition";
            }
            dirty = true;
        }
        ped_constraint_destroy(constraint);
    }
    if (dirty) ped_disk_commit(disk);
    state.boot_path = ped_partition_get_path(boot_partition);
    state.root_path = ped_partition_get_path(root_partition);
    return state;
}
