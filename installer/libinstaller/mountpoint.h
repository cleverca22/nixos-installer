#ifndef INSTALLER_MOUNTPOINT_H
#define INSTALLER_MOUNTPOINT_H


namespace installer {

class MountPoint
{
public:
    MountPoint(QString dev, QString mountpoint);
    bool mount();
    bool umount();

    QString device, mountpoint;
    bool mounted;
};

} // namespace installer

#endif // INSTALLER_MOUNTPOINT_H
