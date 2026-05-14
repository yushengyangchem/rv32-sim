# Execute all recipes in strict bash mode.

set shell := ["bash", "-euo", "pipefail", "-c"]
repo_root := `git rev-parse --show-toplevel`

# Show available recipes.
default:
    just --list --unsorted

# Generate a compile_commands.json database using Bear for IDE/LSP integration.
bear:
    cd {{ repo_root }} && bear -- make && make clean
