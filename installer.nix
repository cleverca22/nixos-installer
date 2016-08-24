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
  buildInputs = [ qmakeHook qtbase parted websocketpp7 boost ];
  dontStrip = true;
  #NIX_CFLAGS_COMPILE = "-ggdb -Og -I${nix}/include/nix -DNIX_VERSION=\"${(builtins.parseDrvName nix.name).version}\"";
  #NIX_LDFLAGS = "-lnixexpr -lnixmain";
  enableParallelBuilding = true;
  postInstall = ''
    # todo, switch it to stdout
    $out/bin/headless --dump-methods > $out/methods.js 2>&1
  '';
}
