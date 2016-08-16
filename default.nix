with import <nixpkgs> {};

rec {
  installer = qt5.callPackage ./installer.nix { inherit qhttp; };
  qhttp = qt5.callPackage ./qhttp.nix {};
}
