/* ************************************************************************

   Copyright:

   License:

   Authors:

************************************************************************ */

qx.Class.define("installer.Device",
{
  extend : qx.ui.tree.TreeFolder,
  construct: function (path) {
    this.base(arguments, path);
    var app = qx.core.Init.getApplication();
    app.RPC("openDevice", { device: path}, function (data) {
      this.device = data;
      this.updateLabel();
      this.openDisk();
    }.bind(this));
    this.app = app;
    this.path = path;
    var menu = new qx.ui.menu.Menu();
    this.setContextMenu(menu);
    var but = new qx.ui.menu.Button("Create GPT table");
    but.addListener("execute", function(e) {
      this.makeLabel("gpt");
    }.bind(this));
    menu.add(but);
    but = new qx.ui.menu.Button("Create MBR table");
    but.addListener("execute", function(e) {
      this.makeLabel("msdos");
    }.bind(this));
    menu.add(but);
  },
  members: {
    openDisk: function () {
      this.app.RPC("openDisk", { device_handle: this.device.handle }, function (data) {
        this.disk = data;
        this.updateLabel();
        if (data.valid) {
          this.listPartitions();
        }
      }.bind(this));
    },
    renderPartition: function (part) {
      var size_str = this.toMB(this.device.sector_size * part.length);
      var parts = [];
      parts.push("part#"+part.num);
      if (part.name.length > 0) parts.push("name: '"+part.name+"'");
      if (part.flags.length > 0) parts.push("flags: "+part.flags.join(","));
      parts.push("range: "+part.start+"-"+part.end);
      parts.push("size: " + size_str);
      var menu = new qx.ui.menu.Menu();
      var del = new qx.ui.menu.Button("Delete Partition");
      del.addListener("execute", function (e) {
        this.app.RPC("deletePartition", { disk_handle: this.disk.handle }, function (d) {

        }.bind(this));
      }, this);
      menu.add(del);

      var node = new qx.ui.tree.TreeFile(parts.join(" "));
      node.setContextMenu(menu);
      return node
    },
    listPartitions: function () {
      this.app.RPC("listPartitions", { disk_handle: this.disk.handle }, function (data) {
        this.partitions = data.partitions;
        this.findHoles();
      }.bind(this));
    },
    renderHole: function (start, size) {
      var hole = new qx.ui.tree.TreeFile("unused "+this.toMB(size * this.device.sector_size));
      var menu = new qx.ui.menu.Menu();
      hole.setContextMenu(menu);

      var create = new qx.ui.menu.Button("Create Partition");
      menu.add(create);
      create.addListener("execute", function (e) {
        var test = new installer.MakePartition(this.path, start, size);
        test.addListener("create", function (e) {
          var data = e.getData();
          console.log(data);
          var new_start = start + data.pre;
          var new_size = data.size;
          this.app.RPC("createPartition", { disk_handle: this.disk.handle, start:new_start, size: new_size }, function (d) {
            this.listPartitions();
          }.bind(this));
        }, this);
        test.open();
      }, this);

      this.add(hole);
    },
    findHoles: function () {
      var sorted = this.partitions.slice();
      sorted.sort(function (a,b) {
        if (a.start < b.start) return -1;
        else if (a.start > b.start) return +1;
        else return 0;
      });
      this.removeAll();
      this.setOpen(true);
      if (sorted.length > 0) {
        var first = sorted[0];
        var gap = first.start - 2048;
        if (gap > 0) {
          this.renderHole(2048, gap);
        }
        for (var i=0; i<(sorted.length-1); i++) {
          var a = sorted[i];
          var b = sorted[i+1];

          this.add(this.renderPartition(a));

          var gap = b.start - (a.end + 1);
          if (gap > 0) {
            this.renderHole(a.end+1, gap);
          }
        }
        if (this.partitions.length == 1) var b = this.partitions[0];
        if (b) {
          this.add(this.renderPartition(b));
          var gap = this.device.length - (b.end + 1);
          if (gap > 0) {
            this.renderHole(b.end + 1, gap);
          }
        }
      } else { // no partitions, all hole
        this.renderHole(2048, this.device.length);
      }
    },
    makeLabel: function (type) {
      this.app.RPC("makeLabel", { handle: this.device.handle, type: type }, function (data) {
        console.log("make label reply", data);
        this.disk = data;
        this.updateLabel();
        this.listPartitions();
      }.bind(this));
    },
    toMB: function (size) {
      var unit = 0;
      while (size > 1024) { size = size / 1024; unit++; }
      var units = [ "Byte", "K", "M", "G" ];
      return (Math.round(size * 1000)/1000) + units[unit];
    },
    updateLabel: function () {
      var size_str = this.toMB(this.device.sector_size * this.device.length);
      if (this.disk && this.disk.valid) {
        this.setLabel(this.path + " " + size_str + " " + this.disk.type);
      } else {
        this.setLabel(this.path + " " + size_str + " no valid partition tables");
      }
    }
  }
});