/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.JsonNode",
{
  extend : qx.ui.tree.TreeFolder,
  properties: {
    "key": {
      nullable: false,
      event: "changeKey"
    }
  },
  construct: function (name, type) {
    this.base(arguments);
    this.setKey(name);
    this.bind("key", this, "label");
    var menu = new qx.ui.menu.Menu();

    var newattr = new qx.ui.menu.Button("New Attrset");
    newattr.addListener("execute", function (e) {
      var prompt = new installer.ModalPrompt("attrset");
      prompt.addListener("create", function (e) {
        var value = e.getData();
        var child = new installer.JsonNode(value);
        child.setOpen(true);
        this.add(child);
      }, this);
      prompt.open();
    }, this);
    menu.add(newattr);

    var newstring = new qx.ui.menu.Button("New String");
    newstring.addListener("execute", function (e) {
      var prompt = new installer.ModalPrompt("string");
      prompt.addListener("create", function (e) {
        var data = e.getData();
        var child = new installer.JsonLeaf(data.name, "string", data.value);
        this.add(child);
      }, this);
      prompt.open();
    }, this);
    menu.add(newstring);

    var newbool = new qx.ui.menu.Button("New Boolean");
    menu.add(newbool);

    this.setContextMenu(menu);
  },
  members: {
    to_nix: function () {
      var children = this.getChildren();
      var lines = [];
      if (children.length == 1) {
        lines.push(this.getKey() + "." + children[0].to_nix(0));
      } else {
        lines.push(this.getKey() + " = {");
        for (var i=0; i<children.length; i++) {
          lines = lines.concat(children[i].to_nix().split("\n"));
        }
        lines.push("};");
      }
      return lines.join("\n");
    },
    find_child: function(key) {
      var children = this.getChildren();
      console.log("children:",children);
      for (var i=0; i<children.length; i++) {
        if (children[i].getKey() == key) return children[i];
      }
      return null;
    },
    set_value: function (key, value) {
      var parts = key.split(".");
      console.log(parts, value);
      if (parts.length == 1) {
        console.log("one part left");
        var child = this.find_child(parts[0]);
        if (!child) {
          var type = null;
          switch (typeof value) {
          case "string":
            type = "string";
            break;
          case "boolean":
            type = "bool";
            break;
          case "number":
            type = "int";
            break;
          }
          var child = new installer.JsonLeaf(parts[0], type, value);
          this.add(child);
        } else {
          child.set_value(type, value);
        }
      } else {
        var remain = parts.slice(1).join(".");
        var child = this.find_child(parts[0]);
        if (!child) {
          var child = new installer.JsonNode(parts[0], "attrset");
          child.setOpen(true);
          this.add(child);
        }
        child.set_value(remain, value);
      }
    }
  }
});