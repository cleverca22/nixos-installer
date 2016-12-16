/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */
qx.Class.define("installer.DocLeaf",
{
  extend : qx.ui.tree.TreeFile,
  properties: {
    "key": {
      nullable: false,
      event: "changeKey"
    },
    "data": {
      nullable: false
    }
  },
  construct: function (name, value) {
    this.base(arguments);
    this.setKey(name);
    this.setData(value);
    this.bind("key", this, "label");
  },
  members: {
    set_value: function (val) {

    }
  }
});