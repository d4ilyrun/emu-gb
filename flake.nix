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
        
        packages = let
          nativeBuildInputs = with pkgs; [ gcc cmake ];
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
              make -j4 emu-gb
            '';

            installPhase = ''
              mkdir -p $out/bin
              mv emu-gb $out/bin
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
              make -j4
            '';

            installPhase = ''
              # Run tests and save output
              cd tests

              # Move test to reuse it later
              mkdir -p $out/
              mv tests/*_test $out/
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
