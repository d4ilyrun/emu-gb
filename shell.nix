{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    wla-dx
    pre-commit
    doxygen
  ];

  shellHook = ''
  '';
}
