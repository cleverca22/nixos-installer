with import <nixpkgs> {};

rec {
  installer = qt5.callPackage ./installer.nix { inherit qhttp; };
  qhttp = qt5.callPackage ./qhttp.nix {};
  qemu_demo = callPackage ./demo.nix { inherit config; };
  configuration = { pkgs, ... }: {
    imports = [ <nixpkgs/nixos/modules/installer/netboot/netboot-minimal.nix> ];
    virtualisation.emptyDiskImages = [ (1024*8) ];
    environment.systemPackages = with pkgs; [ installer pkgs.tcpdump pkgs.gdb valgrind ];
    virtualisation.writableStore = true;
    virtualisation.graphics = false;
    virtualisation.qemu.networkingOptions = [
      "-net nic,vlan=0,model=virtio"
      "-net user,hostfwd=tcp::8080-:8080,vlan=0\${QEMU_NET_OPTS:+,$QEMU_NET_OPTS}"
    ];
    networking.firewall.allowedTCPPorts = [ 8080 ];
  };
  eval = (import <nixpkgs/nixos> { inherit configuration; });
  config = eval.config;
  qooxdoo = callPackage ./qooxdoo.nix {};
  netboot = let
    cfg = { ... }:
    {
      imports = [ <nixpkgs/nixos/modules/installer/netboot/netboot-minimal.nix> ];
      environment.systemPackages = [ installer ];
      networking.firewall.allowedTCPPorts = [ 8080 ];
    };
    build = (import <nixpkgs/nixos> { configuration = cfg; }).config.system.build;
  in symlinkJoin {
    name="netboot";
    paths = [ build.netbootRamdisk build.kernel build.netbootIpxeScript ];
  };
}
