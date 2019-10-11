#!/bin/sh

# Install rust
wget https://sh.rustup.rs -O rustup-init.sh
chmod uga+rx ./rustup-init.sh
./rustup-init.sh -y

# Install deps for bindgen
sudo install llvm libclang-dev

# Setup env
. $HOME/.cargo/env

# Update rustup
rustup update
rustup self update

# Install code formatter
cargo install rustfmt --force

## Optional
#rustup toolchain install nightly
#rustup run nightly rustc --version
# cargo install cargo-script
