{ stdenv, makeQtWrapper, qttools, qtbase, qmakeHook, qt5Full, parted, enableDebugging, nix, utillinux, qhttp, websocketpp7, boost }:

let
  utillinux2 = utillinux.overrideDerivation (oldAttrs: {
    src = /home/clever/x/util-linux-2.28;
  });
  parted2 = parted.override { utillinux = utillinux2; };
in stdenv.mkDerivation {
  name = "installer";
  src = ./installer;
  nativeBuildInputs = [ qmakeHook makeQtWrapper qhttp ];
  buildInputs = [ qmakeHook qtbase parted ];
  postInstall = ''
    mkdir -pv $out/share/
    cp -r ${./docroot} $out/share/docroot
    chmod -R +w $out/share
    rm -rf $out/share/docroot/rpc
  '';
  dontStrip = true;
  NIX_CFLAGS_COMPILE = "-ggdb -Og";
  #NIX_LDFLAGS = "-lnixexpr -lnixmain";
  enableParallelBuilding = true;
}
