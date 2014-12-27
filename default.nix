{ pkgs ? import <nixpkgs> {} }:

with pkgs;

stdenv.mkDerivation rec {
  name = "mpdas-${version}";
  version = "0.4-alpha";

  src = ./.;

  buildInputs = [ curl mpd_clientlib pkgconfig ];

  installPhase = ''
    install -d $out/bin
    install -d $out 
    install -m 755 mpdas $out/bin
    install -m 644 mpdas.1 $out/mpdas.1
  '';

  meta = {
    homepage = "http://50hz.ws/mpdas/";
    description = "A C++ client to submit tracks to audioscrobbler, supports new protocol";
  };
}
