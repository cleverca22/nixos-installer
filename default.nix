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
    cfg = { pkgs, lib, ... }:
    {
      imports = [ <nixpkgs/nixos/modules/installer/netboot/netboot-minimal.nix> ];
      environment.systemPackages = [ installer pkgs.valgrind pkgs.gdb ];
      networking.firewall.allowedTCPPorts = [ 8080 ];
      users.users.root.openssh.authorizedKeys.keys = [ "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQC34wZQFEOGkA5b0Z6maE3aKy/ix1MiK1D0Qmg4E9skAA57yKtWYzjA23r5OCF4Nhlj1CuYd6P1sEI/fMnxf+KkqqgW3ZoZ0+pQu4Bd8Ymi3OkkQX9kiq2coD3AFI6JytC6uBi6FaZQT5fG59DbXhxO5YpZlym8ps1obyCBX0hyKntD18RgHNaNM+jkQOhQ5OoxKsBEobxQOEdjIowl2QeEHb99n45sFr53NFqk3UCz0Y7ZMf1hSFQPuuEC/wExzBBJ1Wl7E1LlNA4p9O3qJUSadGZS4e5nSLqMnbQWv2icQS/7J8IwY0M8r1MsL8mdnlXHUofPlG1r4mtovQ2myzOx clever@nixos" ];
      systemd.services.sshd.wantedBy = lib.mkForce [ "multi-user.target" ];
    };
    build = (import <nixpkgs/nixos> { configuration = cfg; }).config.system.build;
  in symlinkJoin {
    name="netboot";
    paths = [ build.netbootRamdisk build.kernel build.netbootIpxeScript ];
  };
}
