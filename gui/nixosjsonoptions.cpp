#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "nixosjsonoptions.h"
#include "ui_nixosjsonoptions.h"

NixosJsonOptions::NixosJsonOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NixosJsonOptions),
    worker(new Worker(this))
{
    ui->setupUi(this);

    connect(&proc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(build_finished(int,QProcess::ExitStatus)));
    connect(worker,SIGNAL(finished()),this,SLOT(worker_finished()));

    QStringList args;
    args.append("<nixpkgs/nixos/release.nix>");
    args.append("-A");
    args.append("options");
    args.append("-o");
    args.append("/tmp/options");
    proc.start("nix-build",args);
    ui->status->setText("generating option list...");
    ui->stackedWidget->setCurrentIndex(0);
}

NixosJsonOptions::~NixosJsonOptions()
{
    delete ui;
}

void Worker::run() {
    QMap<QString,ConfigEntry> configs;
    QFile input("/tmp/options/share/doc/nixos/options.json");
    input.open(QFile::ReadOnly);
    QByteArray rawjson = input.readAll();
    QJsonObject root(QJsonDocument::fromJson(rawjson).object());
    for (auto i = root.begin(); i!= root.end(); ++i) {
        //qDebug() << i.key();
        //qDebug() << i.value();
        QJsonObject obj = i.value().toObject();
        ConfigEntry entry;
        entry.type = obj["type"].toVariant().toString();
        entry.description = obj["description"].toVariant().toString().trimmed();
        entry.def = obj["default"].toVariant();
        entry.example = obj["example"].toVariant();
        configs.insert(i.key(),entry);
    }
    parent->configs = configs;
}

void NixosJsonOptions::build_finished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << exitCode << exitStatus;
    qDebug() << proc.readAllStandardError();
    qDebug() << proc.readAllStandardOutput();
    ui->status->setText("loading option list...");
    worker->start();
}

class JsonModel : public QAbstractListModel {
public:
    QMap<QString,ConfigEntry> configs;
    int rowCount(const QModelIndex &) const {
        return configs.count();
    }
    QVariant data(const QModelIndex &index, int role) const {
        if (role == Qt::DisplayRole) {
            return configs.keys().at(index.row());
        } else return QVariant();
    }
    void update(QMap<QString,ConfigEntry> in) {
        int maxrow = std::max(configs.count(),in.count());
        bool adding = maxrow > configs.count();

        if (adding) beginInsertRows(QModelIndex(),0,maxrow);
        else beginRemoveRows(QModelIndex(),0,maxrow);
        configs = in;
        emit dataChanged(createIndex(0,0),createIndex(maxrow,0));
        if (adding) endInsertRows();
        else endRemoveRows();
    }
};

void NixosJsonOptions::worker_finished() {
    model = new JsonModel;
    ui->treeView->setModel(model);
    model->configs = configs;
    connect(ui->treeView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(update(QItemSelection,QItemSelection)));
    ui->stackedWidget->setCurrentIndex(1);
    emit config_loaded();
}

void NixosJsonOptions::refresh() {
    QString filter = ui->filter->text().trimmed();
    if (filter.length() == 0) {
        model->update(configs);
    } else {
        QMap<QString,ConfigEntry> out;
        for (auto i = configs.begin(); i != configs.end(); ++i) {
            if (i.key().contains(filter)) {
                out.insert(i.key(),i.value());
            }
        }
        model->update(out);
    }
}
void NixosJsonOptions::on_filter_textChanged(const QString &text) {
    refresh();
}
void NixosJsonOptions::update(QItemSelection,QItemSelection) {
    QModelIndexList l = ui->treeView->selectionModel()->selectedIndexes();
    if (l.count()) {
        int row = l.at(0).row();
        ConfigEntry en = model->configs.values().at(row);
        ui->type->setText(en.type);
        ui->description->setText(en.description);
        qDebug() << en.example;

        if (en.type == "boolean") {
            ui->example->setVisible(false);
            ui->label_3->setVisible(false);
        } else {
            ui->example->setVisible(true);
            ui->label_3->setVisible(true);
        }
        if (en.type == "boolean") {
            ui->propType->setCurrentIndex(0);
            ui->default_value->setText(en.def.toString());
            ui->boolean_value->setChecked(en.def.toBool());
            ui->boolean_value->setEnabled(false);
        } else if (en.type == "string") {
            ui->default_value->setText(en.def.toString());
            ui->propType->setCurrentIndex(1);
            ui->lineEdit->setText(en.def.toString());
            ui->lineEdit->setEnabled(false);
            ui->example->setText(en.example.toString());
        }
    } else {
        ui->type->setText("");
    }
}
