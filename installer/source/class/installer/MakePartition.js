/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.MakePartition",
{
  extend : qx.ui.window.Window,
  events: {
    "create": "qx.event.type.Data"
  },
  construct: function (device, start, size) {
    this.base(arguments, "Create Partition");
    this.setLayout(new qx.ui.layout.VBox(10));
    this.setModal(true);
    this.setShowMaximize(false);
    this.setShowMinimize(false);
    this.device = device;
    this.start = start;
    this.size = size;
    this.createFields();
  },
  members: {
    createFields: function () {
        var form = new qx.ui.form.Form();

        var pre = new qx.ui.form.Spinner(0, 0, this.size);
        pre.setWidth(80);
        pre.setRequired(true);
        pre.addListener("changeValue", function (e) {
            //size.setMaximum(this.size - (pre.getValue() + post.getValue()));
            post.setValue(this.size - (pre.getValue() + size.getValue()));
        }, this)
        form.add(pre, "Space before");

        var size = new qx.ui.form.Spinner(0, this.size, this.size);
        size.setRequired(true);
        size.addListener("changeValue", function (e) {
            post.setValue(this.size - (pre.getValue() + size.getValue()));
        }, this);
        form.add(size, "Size");

        var post = new qx.ui.form.Spinner(0, 0, this.size);
        post.setRequired(true);
        post.addListener("changeValue", function (e) {
            //size.setMaximum(this.size - (pre.getValue() + post.getValue()));
        }, this)
        form.add(post, "Space after");

        var formView = new qx.ui.form.renderer.Single(form);
        this.add(formView);

        var create = new qx.ui.form.Button("Create");
        create.addListener("execute", function (e) {
            console.log(pre.getValue(), size.getValue(), post.getValue());
            this.fireDataEvent("create", { pre: pre.getValue(), size: size.getValue(), post: post.getValue() });
            this.destroy();
        }, this);
        this.add(create);
    }
  }
});