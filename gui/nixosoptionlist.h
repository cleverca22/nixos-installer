#ifndef NIXOSOPTIONLIST_H
#define NIXOSOPTIONLIST_H

#include <QStringList>
#include <QDebug>
#include <QMap>

#include "shared.hh"
#include "eval.hh"
#include "eval-inline.hh"
#include "store-api.hh"

#undef debug

class WrappedValue;

class NixosOptionList {
public:
    NixosOptionList();
    void force(nix::Value *v);
    WrappedValue *root;
    nix::EvalState state;

private:
    nix::StaticEnv staticEnv;
    nix::Env *env;
};

class WrappedValue {
public:
    WrappedValue(nix::Value v, WrappedValue *parent, QString name, int row): v(v), parent(parent), name(name), row(row) {
        assert(v.type <= 17);
        //qDebug() << __func__ << this << v.type;
    }
    QStringList listNodes();
    void force();
    WrappedValue *nth(int n);
    WrappedValue *select(QString key);

    nix::Value v;
    QMap<QString,WrappedValue*> selectCache;
    WrappedValue *parent;
    QString name;
    int row;
};

extern NixosOptionList *hax;

#endif // NIXOSOPTIONLIST_H

