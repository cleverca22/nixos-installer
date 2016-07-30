#ifndef INSTALLMODE_H
#define INSTALLMODE_H

#include <QWidget>

namespace Ui {
class InstallMode;
}

class InstallMode : public QWidget
{
    Q_OBJECT

public:
    explicit InstallMode(QWidget *parent = 0);
    ~InstallMode();

signals:
    void install();
    void modify();
    void recover();

private:
    Ui::InstallMode *ui;
};

#endif // INSTALLMODE_H
