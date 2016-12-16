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

      // Document is the application root
      var doc = this.getRoot();

      var tabview = new qx.ui.tabview.TabView();
      doc.add(tabview, { width: "100%", height: "100%" });

      var editor_tab = installer.Editor.getInstance();
      tabview.add(editor_tab);

      //var win3 = installer.BuildStatus.getInstance();
      //win3.open();
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
