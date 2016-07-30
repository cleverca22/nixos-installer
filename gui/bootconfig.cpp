#include <QDebug>

#include "bootconfig.h"
#include "ui_bootconfig.h"
#include <parted/parted.h>

enum class label_type {
    gpt, mbr
};

void do_test1(PedDevice *dev, label_type labelType);

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

void BootConfig::on_install_clicked() {
    qDebug() << "do things";
    qDebug() << ui->target->currentText();
    PedDevice *dev;
    dev = ped_device_get(qPrintable(ui->target->currentText()));
    if (!ped_device_open(dev)) {
        qDebug() << "unable to open disk";
    }
    do_test1(dev, label_type::gpt);
    if (!ped_device_close(dev)) {
        qDebug() << "unable to close disk";
    }
}

void do_test1(PedDevice *dev, label_type labelType) {
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
                ped_partition_set_flag(root_partition,PED_PARTITION_ROOT,1);
            }
            if (!ped_disk_add_partition(disk,root_partition,constraint)) {
                qDebug() << "error adding partition";
            }
            dirty = true;
        }
        ped_constraint_destroy(constraint);
    }
    if (dirty) ped_disk_commit(disk);
}
