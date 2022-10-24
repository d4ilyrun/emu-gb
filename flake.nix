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
              make -j12 emu-gb
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
              make -j12
            '';

            installPhase = ''
              for test in $(find tests -type f -and -executable); do
                mkdir -p "$out/$(dirname $test)"
                mv $test "$out/$test"
              done
            '';
          };

          pre-commit = stdenv.mkDerivation {
            pname = "pre-commit";
            version = "1.0.0";
            src = self;

            buildInputs = with pkgs; [ pre-commit shellcheck clang-tools ];

            installPhase =
            let
              run_pre_commit = pkgs.writeShellScript "run_pre_commit" ''
                ${pkgs.pre-commit}/bin/pre-commit run
              '';
            in ''
              ${pkgs.pre-commit}/bin/pre-commit install --install-hooks
              mv ${run_pre_commit} $out/bin
            '';
          };
        };

        devShell = pkgs.mkShell {
          name = "emu-gb";
          inputsFrom = [ packages.emu-gb packages.pre-commit ];
          buildInputs = with pkgs; [ doxygen valgrind gtest ];
        };
      }
    );
}
