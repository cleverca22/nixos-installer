#include "nixosoptionlist.h"

NixosOptionList::NixosOptionList() :
    state(nix::Strings()/*, nix::openStore()*/),
    staticEnv(false,&state.staticBaseEnv) {
    env = &state.allocEnv(32768);
    env->up = &state.baseEnv;
    nix::Expr *e = state.parseExprFromString("(import <nixos> { configuration = {}; }).options","/",staticEnv);
    nix::Value v;
    e->eval(state,*env,v);
    qDebug() << "creating root";
    root = new WrappedValue(v,0,"root",0);
}
QStringList WrappedValue::listNodes() {
    QStringList attrs;
    //qDebug() << "listNodes" << v << v->type;
    force();
    if (v.type != nix::tAttrs) return attrs;
    hax->state.forceAttrs(v);
    for (auto &i : *v.attrs) {
        std::string name = i.name;
        attrs.append(name.c_str());
    }
    return attrs;
}
/*WrappedValue *WrappedValue::select(QString key) {
    auto i = selectCache.find(key);
    if (i != selectCache.end()) return i.value();

    nix::Symbol sym = hax->state.symbols.create(qPrintable(key));
    hax->state.forceAttrs(v);
    auto a = v.attrs->find(sym);
    if (a != v.attrs->end()) {
        nix::Value *v = a->value;
        std::string name = a->name;
        qDebug() << v << "is" << name.c_str();
        hax->state.forceValue(*v);
        WrappedValue *wrapper = new WrappedValue(*v,this,key);
        selectCache.insert(key,wrapper);
        return wrapper;
    }
}*/
void NixosOptionList::force(nix::Value *v) {
    state.forceValue(*v);
}
void WrappedValue::force() {
    hax->state.forceValue(v);
}
WrappedValue *WrappedValue::nth(int n) {
    //qDebug() << __func__ << this << v.type << n << listNodes();
    force();
    auto a = (*v.attrs)[n];

    std::string key = a.name;
    auto i = selectCache.find(key.c_str());
    if (i != selectCache.end()) return i.value();

    WrappedValue *wrapper = new WrappedValue(*a.value,this,key.c_str(),n);
    selectCache.insert(key.c_str(),wrapper);
    return wrapper;
}
