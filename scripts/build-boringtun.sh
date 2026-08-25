#!/usr/bin/env bash
set -e

# Requires: rustup target add x86_64-pc-windows-msvc && cargo install cargo-xwin
# Run this once before building the mod for the first time (or after a clean checkout).

BORINGTUN_DIR="${1:-$HOME/Projects/boringtun}"
OUT_DIR="$(dirname "$0")/../third_party/boringtun"

if [ ! -d "$BORINGTUN_DIR" ]; then
    git clone https://github.com/cloudflare/boringtun.git "$BORINGTUN_DIR"
fi

cd "$BORINGTUN_DIR"
cargo xwin build --release --target x86_64-pc-windows-msvc -p boringtun --features ffi-bindings

mkdir -p "$OUT_DIR/lib/win64" "$OUT_DIR/include"
cp "target/x86_64-pc-windows-msvc/release/boringtun.lib" "$OUT_DIR/lib/win64/"
cp "boringtun/src/wireguard_ffi.h" "$OUT_DIR/include/"

echo "boringtun.lib and wireguard_ffi.h copied into $OUT_DIR"
