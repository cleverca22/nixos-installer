{ stdenv, fetchFromGitHub, qmakeHook, qtbase }:

let
  http-parser = fetchFromGitHub {
    owner = "nodejs";
    repo = "http-parser";
    rev = "feae95a3a69f111bc1897b9048d9acbc290992f9";
    sha256 = "0rl8xyb3fmypv5r3jajw8bkcxh41an3anj99qfy3mj38lc84d0zj";
  };
in
stdenv.mkDerivation {
  outputs = [ "dev" "out" ];
  name = "qhttp";
  src = fetchFromGitHub {
    owner = "azadkuh";
    repo = "qhttp";
    rev = "c22acdf2ed8dd135fcefc8f67a7b31ec46fb3d5f";
    sha256 = "1fr39dxni7s7pfkirj1r2cqr35w8h0mlmsn4vz6wn3g82whqrjmk";
  };
  preConfigure = ''
    mkdir -p 3rdparty
    cp -vr ${http-parser} 3rdparty/http-parser
  '';
  buildInputs = [ qmakeHook qtbase ];
  enableParallelBuilding = true;
  installPhase = ''
    mkdir -p $out/lib/
    cp -pd xbin/libqhttp* $out/lib/
    mkdir -p $dev/include/
    for x in qhttpserver.hpp qhttpserverconnection.hpp qhttpserverrequest.hpp qhttpserverresponse.hpp qhttpfwd.hpp qhttpabstracts.hpp
      do  cp src/$x $dev/include/
    done
  '';
}
