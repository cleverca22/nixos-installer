#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BootConfig;
class InstallMode;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_installMode_install();
    void on_installMode_recover();
    void on_installMode_modify();
private:
    BootConfig *bootConfig;
    InstallMode *installMode;
};

#endif // MAINWINDOW_H