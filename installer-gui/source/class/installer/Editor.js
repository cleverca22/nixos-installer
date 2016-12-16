/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.Editor",
{
  type : "singleton",
  extend : qx.ui.tabview.Page,
  properties: {
    "model": {
      nullable: true,
      event: "changeModel"
    },
    "root": {
      nullable: false
    },
    "docTree": {
      nullable: false
    }
  },
  construct: function () {
    this.base(arguments, "Config Editor");
    this.setLayout(new qx.ui.layout.HBox(10));
    var tree = this.tree = new qx.ui.tree.Tree();
    this.add(tree, { flex: 1 });
    // maybe switch to http://www.qooxdoo.org/current/demobrowser/#data~JsonToTree.html at a later date?
    var root = new installer.JsonNode("config", "attrset");
    root.set_value("boot.loader.grub.enable", true);
    root.set_value("boot.loader.grub.version", 2);
    //root.set_value("boot.loader.grub.device", "/dev/sda");
    root.set_value("networking.hostName", "nixos");
    root.set_value("fileSystems.\"/\".device", "/dev/sda1");
    root.set_value("fileSystems.\"/\".fsType", "ext4");
    this.setRoot(root);

    //var config = this.get_config();
    //console.log(config);
    var app = qx.core.Init.getApplication();
    this.app = app;
    root.setOpen(true);
    tree.setRoot(root);

    var right_pane = new qx.ui.container.Composite();
    right_pane.setLayout(new qx.ui.layout.VBox(5));
    this.add(right_pane, { flex: 1 });

    var docs = new qx.ui.tree.Tree();
    right_pane.add(docs, { flex: 10 });
    this.setDocTree(docs);

    var bottom_pane = new qx.ui.container.Composite();
    bottom_pane.setLayout(new qx.ui.layout.VBox(5));
    right_pane.add(bottom_pane, { flex: 2 });

    var description = new qx.ui.form.TextArea();
    bottom_pane.add(description, { flex: 1 });

    var form = new qx.ui.form.Form();
    var def = new qx.ui.form.TextField();
    form.add(def, "Default");
    var formView = new qx.ui.form.renderer.Single(form);
    bottom_pane.add(formView);

    var but = new qx.ui.form.Button("Reload options");
    this.add(but);

    but.addListener("execute", function (e) {
      this.reloadConfig();
    }, this);
    this.reloadConfig();

    docs.addListener("changeSelection", function (e) {
      var node = e.getData()[0];
      var data = node.getData();
      if (!data) {
        description.setValue("");
      } else {
        console.log(JSON.stringify(data));
        description.setValue(data.description);
        if (data["default"]) def.setValue(JSON.stringify(data["default"]));
        else def.setValue("");
      }
    }, this);
  },
  members: {
    get_config: function() {
      return "{ ... }:\n\n{\n" + this.getRoot().to_nix() + "\n}\n";
    },
    reloadConfig: function () {
      this.app.RPC("options.json", null, function (data) {
        window.raw_options = data;
        var start = Date.now();
        var docroot = new installer.DocNode("root", {});
        var doctree = this.getDocTree();
        doctree.setRoot(docroot);
        docroot.setOpen(true);
        for (var key in data) {
          docroot.set_value(key, data[key]);
        }
        console.log("docs loaded in "+(Date.now() - start)+"ms");
      }.bind(this));
    }
  }
});