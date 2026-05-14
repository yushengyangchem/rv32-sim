{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  packages = with pkgs; [
    just
    clang-tools
    iverilog
    pkgsCross.riscv32-embedded.buildPackages.gcc
    bear
    verible
    asmfmt
    nixfmt
    shfmt
    prettier
  ];
}
