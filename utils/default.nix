# nix-build -o analyzer ; nix-build '<nixpkgs/nixos/release.nix>' -A options -o options ; ./analyzer/bin/option-analyzer ./options/share/doc/nixos/options.json

with import <nixpkgs> { config = {}; };

stdenv.mkDerivation {
  name = "utils";
  buildInputs = [ jsoncpp pugixml ];
  src = ./.;
  installPhase = ''
    mkdir -p $out/bin
    g++ option-analyzer.cpp -o $out/bin/option-analyzer -ljsoncpp -lpugixml
  '';
}