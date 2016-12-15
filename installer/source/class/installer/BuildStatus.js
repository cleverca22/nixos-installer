/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.BuildStatus",
{
  type : "singleton",
  extend : qx.ui.window.Window,
  properties: {
    field: {
      nullable: false
    }
  },
  construct: function () {
    this.base(arguments);
    var app = qx.core.Init.getApplication();
    this.setLayout(new qx.ui.layout.VBox(10));
    var field = new qx.ui.form.TextArea();
    this.setField(field);
    this.add(field, { flex: 1 });
    var dryrun = new qx.ui.form.Button("dry-run");
    this.add(dryrun);
    dryrun.addListener("execute", function () {
      var editor = installer.Editor.getInstance();
      var config = editor.get_config();
      app.RPC("testConfig", { configuration : config }, function (data) {
        console.log(data);
        field.setValue(data.output);
      }.bind(this));
    }, this);
  },
  members: {
  }
});