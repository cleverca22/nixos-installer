with import <nixos/lib/testing.nix> { system = builtins.currentSystem; };
with import <nixos/lib/qemu-flags.nix>;
with pkgs.lib;
let
  iface = "virtio";
  qemuFlags = "-m 768 -cpu kvm64";
  hdFlags = ''hda => "vm-state-machine/machine.qcow2", hdaInterface => "${iface}", '';
  installer = import ./.;
in
makeTest {
  name = "gui-tester";
  nodes = {
    machine = { config, lib, pkgs, ... }:
    {
      imports = [ <nixos/modules/profiles/base.nix> <nixos/modules/profiles/installation-device.nix> ];
      virtualisation.diskSize = 8 * 1024;
      virtualisation.memorySize = 1024;
      virtualisation.writableStore = true;
      virtualisation.emptyDiskImages = [ 512 ];
      virtualisation.bootDevice = "/dev/vdb";
      virtualisation.qemu.diskInterface = "virtio";
      hardware.enableAllFirmware = mkForce false;
      nix.binaryCaches = mkForce [ ];
      environment.systemPackages = [ installer pkgs.ffmpeg ];
      services.xserver.enable = true;
      services.xserver.displayManager.slim.enable = true;
      services.xserver.displayManager.slim.defaultUser = "root";
      services.xserver.displayManager.slim.autoLogin = true;
      services.xserver.desktopManager.xterm.enable = true;
    };
  };
  testScript = ''
    $machine->start;
    $machine->succeed("echo hello");
    $machine->succeed("udevadm settle");
    $machine->succeed("ls -l /dev/vd* >&2");
    $machine->succeed("fdisk -l /dev/vda >&2");
    $machine->succeed("fdisk -l /dev/vdb >&2");
    $machine->waitForUnit("display-manager.service");
    $machine->sleep(10);
    $machine->succeed("XAUTHORITY=/var/run/slim.auth ffmpeg -video_size 1024x768 -framerate 25 -f x11grab -i :0.0+0,0 -t 200 /tmp/xchg/output.mp4 >&2 &");
    $machine->succeed("time XAUTHORITY=/var/run/slim.auth gui -test");
    #$machine->sleep(30);
    #$machine->screenshot("foo");
    $machine->succeed("sfdisk -d /dev/vda >&2");
    $machine->succeed("blkid /dev/vda* >&2");
    #$machine->sleep(10);
    #$machine->screenshot("bar");
    #$machine->succeed("nixos-install < /dev/null >&2");
    #$machine->succeed("nixos-install < /dev/null >&2");
    $machine->succeed("umount /mnt/boot || true");
    $machine->succeed("umount /mnt");
    $machine->succeed("sync");
    $machine->succeed("mount >&2");
    $machine->shutdown;
    $machine = createMachine({ ${hdFlags} qemuFlags => "${qemuFlags}" });
    $machine->start;
    sleep(60);
    $machine->screenshot("baz");
    $machine->waitForUnit("local-fs.target");
    my $foo = `pwd ; ls -l`;
    print $foo;
    $foo = `cp -v */xchg/output.mp4 \$out/`;
  '';
}
