{
  description = "A lightweight RV32I instruction set simulator in C, featuring custom instructions for GeMM/SDPA";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
    git-hooks = {
      url = "github:cachix/git-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { flake-parts, ... }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ inputs.git-hooks.flakeModule ];
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      perSystem =
        { config, pkgs, ... }:
        {
          devShells.default = pkgs.mkShell {
            inputsFrom = [ config.pre-commit.devShell ];
            packages = with pkgs; [
              gcc
              gnumake
              clang-tools
              iverilog
              verilator
              pkgsCross.riscv32-embedded.buildPackages.gcc
              bear
              verible
              asmfmt
            ];
          };

          pre-commit = {
            check.enable = true;
            settings.hooks = {
              nixfmt.enable = true;
              shfmt.enable = true;
              prettier = {
                enable = true;
                excludes = [ "flake.lock" ];
              };
              clang-format.enable = true;
              checkmake.enable = true;
            };
          };
        };
    };
}
