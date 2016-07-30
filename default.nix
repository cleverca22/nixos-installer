(import <nixpkgs> {}).qt5.callPackage

({ stdenv, makeQtWrapper, qttools, qtbase, qmakeHook, qt5Full, parted, enableDebugging }:

stdenv.mkDerivation {
  name = "installer";
  src = ./.;
  nativeBuildInputs = [ qmakeHook makeQtWrapper ];
  buildInputs = [ qmakeHook qtbase (enableDebugging parted) ];
  dontStrip = true;
  NIX_CFLAGS_COMPILE = "-ggdb -Og";
#  postInstall = ''
#    wrapQtProgram $out/bin/gui
#  '';
}) {}
