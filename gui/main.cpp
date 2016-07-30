#include <QApplication>
#include <QDebug>
//#include <QPlastiqueStyle>
#include <QStyleFactory>
#include <parted/parted.h>
#include "bootconfig.h"


enum label_type {
    gpt, mbr
};

void do_test1(label_type labelType);

int main(int argc, char **argv) {
    //qDebug() << QStyleFactory::keys();
    //QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication *app = new QApplication(argc,argv);
    //BootConfig *bc = new BootConfig();
    //bc->show();
    //app->exec();
    do_test1(gpt);
    qDebug("shutting down");
    return 0;
}

void do_test1(enum label_type labelType) {
    PedDevice *dev;
    PedGeometry geom;
    PedDisk *disk;
    PedPartition *part;
    PedPartition *grub_partition = 0, *boot_partition = 0, *root_partition = 0;
    PedDiskType *type;
    PedFileSystemType *ext4 = ped_file_system_type_get("ext4");
    bool dirty = false;
    PedSector start = 0, end = 0;

    dev = ped_device_get("./dummy.img");
    qDebug() << dev;
    qDebug() << dev->path << dev->length << dev->sector_size;
    if (!ped_device_open(dev)) {
        qDebug() << "unable to open disk";
        return;
    }
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
        if (labelType == gpt) {
            type = ped_disk_type_get("gpt");
        } else if (labelType == mbr) {
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
            if (labelType == gpt) {
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
            if (labelType == gpt) {
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
            if (labelType == gpt) {
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
