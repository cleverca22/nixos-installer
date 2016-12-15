/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.Partitions",
{
  type : "singleton",
  extend : qx.ui.window.Window,
  construct: function () {
    var app = qx.core.Init.getApplication();
    this.base(arguments, "Partition Editor");
    this.setLayout(new qx.ui.layout.Grow());
    var tree = this.tree = new qx.ui.tree.Tree().set({
      width: 500,
      height: 400
    });
    this.add(tree);

    var root = new qx.ui.tree.TreeFolder("root");
    root.setOpen(true);
    tree.setRoot(root);
    tree.setHideRoot(true);
    var devices = new qx.ui.tree.TreeFolder("raw drives");
    devices.setOpen(true);
    root.add(devices);

    app.RPC("devices", null, function (data) {
      devices.removeAll();
      for (var i=0; i<data.devices.length; i++) {
        var node = new installer.Device(data.devices[i]);
        devices.add(node);
      }
    });
  }
});