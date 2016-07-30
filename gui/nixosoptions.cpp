#include <QHeaderView>
#include <QDebug>

#include "nixosoptions.h"
#include "ui_nixosoptions.h"
#include "nixosoptionlist.h"

NixosOptionList *hax = 0;

class NixosOptionModel : public QAbstractItemModel {
public:
    NixosOptionModel() {
        hax = new NixosOptionList;
    }

    int rowCount(const QModelIndex& in) const {
        //qDebug() << __func__ << in << in.isValid();
        int ret = 0;

        //if (in.row() == -1) ret = 10;
        //else if ((in.row() == 3) && (in.internalPointer() == (void*)&hax->root)) ret = 4;
        //else ret = 0;

        if (in.internalPointer()) { // non-root node
            WrappedValue *parent = (WrappedValue*)in.internalPointer();
            if (parent->parent) {
                //qDebug() << "rowcount of" << in.row() << parent->parent->listNodes();
            }
            ret = parent->listNodes().count();
            /*if (parent->v.type == nix::tAttrs) {
                WrappedValue *child = parent->nth(in.row());
                child->force();
                qDebug() << "part 2" << parent << child << child->v.type;
                if (child->v.type == nix::tAttrs) {
                    QStringList nodes = child->listNodes();
                    ret = nodes.count();
                }
            }*/
        } else {
            QStringList nodes = hax->root->listNodes();
            ret = nodes.count();
        }
        //qDebug() << "returning" << ret;
        return ret;
    }
    int columnCount(const QModelIndex& in) const {
        //qDebug() << in;
        return 3;
    }
    QModelIndex index(int row, int column, const QModelIndex& index) const {
        WrappedValue *parent;
        if (index.internalPointer()) {
            parent = (WrappedValue*)index.internalPointer();
        } else parent = hax->root;
        parent->force();
        WrappedValue *child = parent->nth(row);
        child->force();
        QModelIndex out = createIndex(row,column,(void*)child);
        return out;
    }
    QModelIndex parent(const QModelIndex &index) const {
        if (index.internalPointer()) {
            WrappedValue *parent = (WrappedValue*)index.internalPointer();
            if (parent->parent) {
                return createIndex(parent->parent->row,0,parent->parent);
            } else return QModelIndex();
        } else return QModelIndex();
    }
    QVariant data(const QModelIndex& index, int role) const {
        if ((index.column() == 0) && (role == Qt::DisplayRole)) {
            //qDebug() << __func__ << index << role;
        }
        if (role == Qt::DisplayRole) {
            if (index.internalPointer()) {
                WrappedValue *v = (WrappedValue*)index.internalPointer();
                //QStringList nodes = v->listNodes();
                if (index.column() == 0) {
                    return v->name;
                } else if (index.column() == 1) {
                    return QString("0x%1").arg((size_t)v,8,16);
                } //else if (index.column() == 2) {
                    //if (v->name != "_definedNames") {
                        //QStringList children = v->listNodes();
                        //return children.at(0);
                    //}
                    //return v->listNodes().count();
                //}
                return QString("test");
            } else return QVariant();
        } else return QVariant();
    }
    QVariant headerData(int column, Qt::Orientation, int role) const {
        if (role == Qt::DisplayRole) {
            switch (column) {
            case 0: return "Attribute Path";
            case 1: return "Pointer";
            }
        }
        return QVariant();
    }
};

NixOSOptions::NixOSOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NixOSOptions)
{
    ui->setupUi(this);
    nix::initNix();
    nix::initGC();
    nix::store = nix::openStore();
    ui->treeView->setModel(new NixosOptionModel);
}

NixOSOptions::~NixOSOptions()
{
    delete ui;
}
