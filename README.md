# GTC — Geode Traffic Controller

A Geode mod for Geometry Dash that gives visibility and control over the game's network traffic.

GTC intercepts outbound connections made by the game and its ecosystem — Geode's own mod updater, Globed's online multiplayer traffic, and in-game music/asset downloads — and lets you inspect and route them through your own proxy, without touching your system-wide network configuration.

## Features

- **Traffic visibility.** Every outbound connection made by the game is logged in real time, with destination IP and port.
- **Per-category monitoring.** Globed Online, Geode Mods Update, and GD Music Download are tracked separately, each with a live latency check.
- **Tunnel engine.** A WireGuard client is embedded directly in the mod via [BoringTun](https://github.com/cloudflare/boringtun), Cloudflare's Rust implementation of the WireGuard protocol. No external WireGuard client or system-level VPN is required.
- **VLESS support.** Planned alongside WireGuard as a second tunnel backend.
- **In-game configuration.** WireGuard and VLESS credentials are entered and stored directly from a settings popup — no config files to edit by hand.

## Status

Early development. The network hook layer, traffic UI, and WireGuard handshake engine are functional and tested against a real server. Full traffic tunneling (as opposed to handshake-only testing) is in progress. See [changelog.md](changelog.md) for details.

## Requirements

- [Geode](https://geode-sdk.org) 5.9.0 or later
- Geometry Dash 2.2081, Windows build (native or via Proton)

## Building from source

```sh
git clone https://github.com/PA3MA3AH/GTC.git
cd GTC
./scripts/build-boringtun.sh
geode build
```

`build-boringtun.sh` cross-compiles the WireGuard engine (BoringTun) for `x86_64-pc-windows-msvc` using [cargo-xwin](https://github.com/rust-cross/cargo-xwin) and places the resulting static library where the mod's build expects it. It requires a Rust toolchain with the `x86_64-pc-windows-msvc` target installed.

## License

GPL-3.0 — see [LICENSE](LICENSE).
