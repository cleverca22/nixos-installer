{ writeScript, stdenv, qemu, buildEnv, config }:

let
  env = buildEnv {
    name = "runtime_deps";
    paths = [ qemu ];
  };
in writeScript "demo" ''
#!${stdenv.shell}
export PATH=${env}/bin:$PATH
[ -f root.qcow ] || qemu-img create -f qcow2 -o cluster_size=2M root.qcow 8G

qemu-system-x86_64 -smp 4 -m 1024 -drive file=root.qcow,index=0,media=disk,discard=unmap -kernel ${config.system.build.kernel}/bzImage -initrd ${config.system.build.netbootRamdisk}/initrd -append "init=${builtins.unsafeDiscardStringContext config.system.build.toplevel}/init ${toString config.boot.kernelParams}"
''
