# Execute all recipes in strict bash mode.

set shell := ["bash", "-euo", "pipefail", "-c"]
repo_root := `git rev-parse --show-toplevel`

# Show available recipes.
default:
    just --list --unsorted

# Update all flake inputs and refresh flake.lock.
update:
    cd {{ repo_root }} && nix flake update
