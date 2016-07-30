(import <nixpkgs> {}).qt5.callPackage

({ stdenv, makeQtWrapper, qttools, qtbase, qmakeHook, qt5Full, parted, enableDebugging, nix }:

stdenv.mkDerivation {
  name = "installer";
  src = ./.;
  nativeBuildInputs = [ qmakeHook makeQtWrapper ];
  buildInputs = [ qmakeHook qtbase parted nix ];
  dontStrip = true;
  NIX_CFLAGS_COMPILE = "-ggdb -Og -I${nix}/include/nix -DNIX_VERSION=\"${(builtins.parseDrvName nix.name).version}\"";
  NIX_LDFLAGS = "-lnixexpr -lnixmain";
  enableParallelBuilding = true;
#  postInstall = ''
#    wrapQtProgram $out/bin/gui
#  '';
}) {}
