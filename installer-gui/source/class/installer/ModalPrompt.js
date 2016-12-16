/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.ModalPrompt",
{
  extend : qx.ui.window.Window,
  events: {
    "create": "qx.event.type.Data"
  },
  construct: function (type) {
    this.base(arguments, "Create "+type);
    this.setLayout(new qx.ui.layout.VBox(10));
    this.setModal(true);
    this.setShowMaximize(false);
    this.setShowMinimize(false);
    this._type = type;
    this.createFields();
  },
  members: {
    _type: null,
    createFields: function () {
        var form = new qx.ui.form.Form();
        var formView = new qx.ui.form.renderer.Single(form);
        this.add(formView);
        var autoShow = function (elem) {
            this.addListener("appear", function (e) {
                elem.focus();
            });
        }.bind(this);
        var enterCompletes = function (elem) {
            elem.addListener("keyup", function (e) {
                if (e.getKeyIdentifier() == "Enter") {
                    switch (this._type) {
                    case "attrset":
                        var data = name.getValue();
                        break;
                    case "string":
                        var data = { name: name.getValue(), value: value.getValue() };
                        break;
                    }
                    this.fireDataEvent("create", data);
                    this.destroy();
                }
            }, this);
        }.bind(this);

        console.log(this._type);
        switch (this._type) {
        case "attrset":
            var name = new qx.ui.form.TextField("");
            form.add(name, "Name");
            autoShow(name);
            enterCompletes(name);
            break;
        case "string":
            var name = new qx.ui.form.TextField("");
            form.add(name, "Name");
            autoShow(name);
            var value = new qx.ui.form.TextField("");
            form.add(value, "Value");
            enterCompletes(value);
            break;
        }

        var create = new qx.ui.form.Button("Create");
        create.addListener("execute", function (e) {
            switch (this._type) {
            case "attrset":
                var data = { name: name.getValue() };
                break;
            }
            this.fireDataEvent("create", data);
            this.destroy();
        }, this);
        //this.add(create);
    }
  }
});