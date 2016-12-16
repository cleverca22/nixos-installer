/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */
qx.Class.define("installer.DocNode",
{
  extend : qx.ui.tree.TreeFolder,
  properties: {
    "key": {
      nullable: false,
      event: "changeKey"
    },
    "data": {
      nullable: true
    }
  },
  construct: function (name, type) {
    this.base(arguments);
    this.setKey(name);
    this.bind("key", this, "label");

    this.addListener("changeOpen", function (e) {
      var next;
      while (next = this._defered.shift()) {
        this.set_value(next.key, next.value, true);
      }
    }, this);
    this._cache = {};
    this._defered = [];
  },
  members: {
    //_cache: {},
    //_defered: [],
    find_child: function(key) {
      return this._cache[key];
      var children = this.getChildren();
      for (var i=0; i<children.length; i++) {
        if (children[i].getKey() == key) return children[i];
      }
      return null;
    },
    set_value: function (key, value, forceAdd) {
      var parent = this.getParent();
      if ((!forceAdd) && (parent && (!parent.getOpen() || this.getChildren().length > 1))) {
        this._defered.push({key:key, value:value });
        return;
      }
      var parts = key.split(".");
      if (parts.length == 1) {
        //console.log("one part left");
        var child = this.find_child(parts[0]);
        if (!child) {
          var child = new installer.DocLeaf(parts[0], value);
          this.add(child);
        } else {
          child.set_value(value);
        }
      } else {
        //console.log("else");
        var remain = parts.slice(1).join(".");
        var child = this.find_child(parts[0]);
        if (!child) {
          var child = new installer.DocNode(parts[0], "attrset");
          //console.log("new node", key);
          this._cache[parts[0]] = child;
          this.add(child);
        }
        child.set_value(remain, value);
      }
    }
  }
});