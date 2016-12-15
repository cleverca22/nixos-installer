/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.JsonLeaf",
{
  extend : qx.ui.tree.TreeFile,
  properties: {
    "key": {
      nullable: false,
      event: "changeKey"
    },
    "value": {
      nullable: false,
      event: "changeValue"
    },
    "type": {
      nullable: false,
      check: [ "string", "bool", "int" ],
      event: "changeType"
    }
  },
  construct: function (name, type, value) {
    this.setKey(name);
    this.setValue(value);
    this.setType(type);
    this.base(arguments, this.formatLabel());
  },
  members: {
    formatLabel: function () {
      switch (this.getType()) {
      case "string":
        return this.getKey()+" = \"" + this.getValue() + "\"";
      case "bool":
      case "int":
        return this.getKey()+" = " + this.getValue();
      }
    },
    to_nix: function (indent) {
      return this.formatLabel()+";";
    }
  }
});