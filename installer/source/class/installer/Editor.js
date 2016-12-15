/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.Editor",
{
  type : "singleton",
  extend : qx.ui.window.Window,
  properties: {
    "model": {
      nullable: true,
      event: "changeModel"
    },
    "root": {
      nullable: false
    }
  },
  construct: function () {
    this.base(arguments, "Config Editor");
    this.setLayout(new qx.ui.layout.Grow());
    var tree = this.tree = new qx.ui.tree.Tree().set({
      width: 500,
      height: 400
    });
    this.add(tree);
    // maybe switch to http://www.qooxdoo.org/current/demobrowser/#data~JsonToTree.html at a later date?
    var root = new installer.JsonNode("config", "attrset");
    root.set_value("boot.loader.grub.enable", true);
    root.set_value("boot.loader.grub.version", 2);
    //root.set_value("boot.loader.grub.device", "/dev/sda");
    root.set_value("networking.hostName", "nixos");
    root.set_value("fileSystems.\"/\".device", "/dev/sda1");
    root.set_value("fileSystems.\"/\".fsType", "ext4");
    this.setRoot(root);

    var config = this.get_config();
    console.log(config);
    var app = qx.core.Init.getApplication();
    root.setOpen(true);
    tree.setRoot(root);
  },
  members: {
    get_config: function() {
      return "{ ... }:\n\n{\n" + this.getRoot().to_nix() + "\n}\n";
    }
  }
});