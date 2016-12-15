/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

/**
 * This is the main application class of your custom application "installer"
 *
 * @asset(installer/*)
 */
qx.Class.define("installer.Application",
{
  extend : qx.application.Standalone,


  /*
  *****************************************************************************
     MEMBERS
  *****************************************************************************
  */

  members :
  {
    /**
     * This method contains the initial application code and gets called 
     * during startup of the application
     * 
     * @lint ignoreDeprecated(alert)
     */
    main : function()
    {
      // Call super class
      this.base(arguments);

      //qx.core.Environment.set("qx.debug.dispose-level",1);

      // Enable logging in debug variant
      if (qx.core.Environment.get("qx.debug"))
      {
        // support native logging capabilities, e.g. Firebug for Firefox
        qx.log.appender.Native;
        // support additional cross-browser console. Press F7 to toggle visibility
        qx.log.appender.Console;
      }

      /*
      -------------------------------------------------------------------------
        Below is your actual application code...
      -------------------------------------------------------------------------
      */
      var responseData = [
        {
          method: "GET",
          url: /\/rpc\/devices/,
          response: function(request) {
            var status = 200;
            var headers = { "Content-Type": "application/json" };
            var responseData = {
              devices: [ "/tmp/dummy1.img", "/tmp/dummy2.img" ]
            };
            var body = JSON.stringify(responseData);
            request.respond(status, headers, body);
          }
        },
        {
          method: "POST",
          url: /\/rpc\//,
          response: function(request) {
            console.log(request.requestBody);
            var status = 200;
            var headers = { "Content-Type": "application/json" };
            switch (request.url) {
            case "/rpc/openDevice":
              var responseData = {
                devices: [ "/tmp/dummy1.img", "/tmp/dummy2.img" ]
              };
              break;
            }
            var body = JSON.stringify(responseData);
            request.respond(status, headers, body);
          }
        }
      ];
      //qx.dev.FakeServer.getInstance().configure(responseData);

      // Create a button
      var button1 = new qx.ui.form.Button("First Button", "installer/test.png");

      // Document is the application root
      var doc = this.getRoot();

      // Add button to document at fixed coordinates
      //doc.add(button1, {left: 100, top: 50});

      // Add an event listener
      button1.addListener("execute", function(e) {
      });

      var desktop = new qx.ui.window.Desktop();
      doc.add(desktop, { width: "100%", height: "100%" });

      var button2 = new qx.ui.form.Button("second button", "installer/test.png");
      var win = installer.Partitions.getInstance();
      win.open();

      var win2 = installer.Editor.getInstance();
      win2.open();

      var win3 = installer.BuildStatus.getInstance();
      win3.open();
    },
    RPC: function(method, arguments, callback) {
      var req = new qx.io.request.Xhr("/rpc/"+method, arguments ? "POST" : "GET");
      req.addListener("success", function (e) {
        var req = e.getTarget();
        callback(req.getResponse());
      }, this);
      if (arguments) req.setRequestData(JSON.stringify(arguments));
      req.send();
    }
  }
});
