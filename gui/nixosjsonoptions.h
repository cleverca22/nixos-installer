#ifndef NIXOSJSONOPTIONS_H
#define NIXOSJSONOPTIONS_H

#include <QWidget>
#include <QProcess>
#include <QThread>
#include <QVariant>
#include <QModelIndex>
#include <QItemSelection>

namespace Ui {
class NixosJsonOptions;
}
class NixosJsonOptions;
class Worker;
class JsonModel;

class ConfigEntry {
public:
    QVariant def;
    QString description;
    QString type;
    QVariant example;
};

class NixosJsonOptions : public QWidget
{
    Q_OBJECT

public:
    explicit NixosJsonOptions(QWidget *parent = 0);
    ~NixosJsonOptions();

    QMap<QString,ConfigEntry> configs;

signals:
    void config_loaded();

private slots:
    void build_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void worker_finished();
    void on_filter_textChanged(const QString &text);
    void update(QItemSelection, QItemSelection);

private:
    void refresh();

    Ui::NixosJsonOptions *ui;
    QProcess proc;
    Worker *worker;
    JsonModel *model;
};

class Worker : public QThread {
public:
    Worker(NixosJsonOptions *parent) : QThread(parent), parent(parent) {
    }

    void run();
private:
    NixosJsonOptions *parent;
};

#endif // NIXOSJSONOPTIONS_H
