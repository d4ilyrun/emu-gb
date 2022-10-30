{
  description = "A very basic flake";

  inputs = {
    nixpkgs = {
      type = "github";
      owner = "NixOs";
      repo = "nixpkgs";
      ref = "master";
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

        inherit (pkgs) stdenv;

        # Build inputs for targets
        nativeBuildInputs = with pkgs; [ cmake ];
        preCommitInputs = with pkgs; [ pre-commit shellcheck clang-tools nixpkgs-fmt python39Packages.setuptools ];
        gtestInputs = with pkgs; [ gtest.dev ];
        checksInputs = preCommitInputs ++ gtestInputs;
      in
      rec {

        defaultPackage = packages.emu-gb;

        packages = {
          emu-gb = stdenv.mkDerivation {
            pname = "emu-gb";
            version = "0.1.0";
            src = self;

            inherit nativeBuildInputs;
            buildInputs = [ ];

            cmakeFlags = [ "-DENABLE_INSTALL=ON" ];
          };

          unit-tests = stdenv.mkDerivation {
            pname = "emu-gb";
            version = "0.1.0";
            src = self;

            inherit nativeBuildInputs;
            buildInputs = gtestInputs;

            cmakeFlags = [
              "-DENABLE_INSTALL=ON"
              "-DENABLE_TESTING=ON"
            ];
          };

          pre-commit = pkgs.writeShellApplication {
            name = "pre-commit";
            runtimeInputs = preCommitInputs;
            text = ''
              ${pkgs.pre-commit}/bin/pre-commit install --install-hooks
              ${pkgs.pre-commit}/bin/pre-commit run --all-files
            '';
          };
        };

        checks = {
          inherit (packages) pre-commit;
        };

        devShell = pkgs.mkShell {
          name = "emu-gb";
          inputsFrom = [ packages.emu-gb ];
          buildInputs = with pkgs; [ doxygen gdb ] ++ nativeBuildInputs ++ checksInputs;
        };
      }
    );
}
