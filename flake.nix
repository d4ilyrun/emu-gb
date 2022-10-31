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

    nixGL.url = "github:guibou/nixGL";
  };

  outputs = { self, nixpkgs, flake-utils, nixGL, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        stdenv = pkgs.multiStdenv;
      in rec {

        defaultPackage = packages.emu-gb;

        packages = let
          nativeBuildInputs = with pkgs; [ gcc cmake SDL2 SDL2.dev ];
        in {
          emu-gb = stdenv.mkDerivation {
            pname = "emu-gb";
            version = "0.1.0";
            src = self;

            inherit nativeBuildInputs;

            configurePhase = ''
              mkdir build && cd build
              cmake ..
            '';

            buildPhase = ''
              make -j12 emu-gb
            '';

            installPhase = ''
              mkdir -p $out/bin
              mv emu-gb $out/bin
            '';
          };

          wrapped =
          let
            pkgs = import nixpkgs {
              inherit system;
              overlays = [ nixGL.overlay ];
            };
          in
          pkgs.writeShellApplication {
            name = "emu-gb";
            runtimeInputs = [ packages.emu-gb ];
            text = ''
              ${pkgs.nixgl.nixGLIntel}/bin/nixGLIntel ${packages.emu-gb}/bin/emu-gb "$@"
            '';
          };

          unit-tests = stdenv.mkDerivation {
            pname = "unit-tests";
            version = "0.1.0";
            src = self;

            inherit nativeBuildInputs;
            buildInputs = with pkgs; [ gtest ];

            configurePhase = ''
              mkdir build && cd build
              cmake -DBUILD_TESTS=ON ..
            '';

            buildPhase = ''
              make -j12
            '';

            installPhase = ''
              for test in $(find tests -type f -and -executable); do
                mkdir -p "$out/$(dirname $test)"
                mv $test "$out/$test"
              done
            '';
          };
        };

        devShell = pkgs.mkShell {
          name = "emu-gb";
          inputsFrom = [ packages.emu-gb ];
          buildInputs = with pkgs; [ doxygen valgrind gtest ];
        };
      }
    );
}
