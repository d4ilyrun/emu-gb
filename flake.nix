{
  description = "A very basic flake";

  inputs = {
    nixpkgs = {
      type = "github";
      owner = "NixOs";
      repo = "nixpkgs";
      ref = "nixos-22.05";
    };

    flake-utils = {
      type = "github";
      owner = "numtide";
      repo = "flake-utils";
      ref = "master";
    };
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        stdenv = pkgs.multiStdenv;
      in rec {

        defaultPackage = packages.emu-gb;
        
        packages = {
          emu-gb = stdenv.mkDerivation {
            pname = "emu-gb";
            version = "0.1.0";
            src = self;

            nativeBuildInputs = with pkgs; [ gcc cmake ];

            configurePhase = ''
              mkdir build && cd build
              cmake ..
            '';

            buildPhase = ''
              make emu-gb
            '';

            installPhase = ''
              mkdir -p $out/bin
              mv emu-gb $out/bin
            '';
          };
        };

        devShell = pkgs.mkShell {
          inputsFrom = [ packages.emu-gb ];
          buildInputs = with pkgs; [ doxygen valgrind ];
        };

      }
    );
}
