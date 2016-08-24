with import <nixpkgs> {};

rec {
  installer = qt5.callPackage ./installer.nix { inherit qhttp websocketpp7; };
  qhttp = qt5.callPackage ./qhttp.nix {};
  websocketpp7 = websocketpp.overrideDerivation (oldAttrs: rec {
    name = "websocket++-${version}";
    version = "0.7.0";
    src = fetchFromGitHub {
      owner = "zaphoyd";
      repo = "websocketpp";
      rev = version;
      sha256 = "1i64sps52kvy8yffysjbmmbb109pi28kqai0qdxxz1dcj3xfckqd";
    };
  });
  docroot = runCommand "docroot" {} ''
    cp -vir ${./docroot} $out
    chmod +w $out
    ${installer}/bin/headless --dump-methods > $out/methods.js 2>&1
  '';
  runner = writeScriptBin "headless" ''
    #!${stdenv.shell}
    ${installer}/bin/headless --docroot ${docroot}
  '';
}
