# socks5-enhance

A custom C SOCKS5 proxy experiment and learning project. It is intended to benchmark the author's C and programming knowledge. It currently has a TCP listener, accepts clients, applies a read timeout, and contains an unfinished attempt at SOCKS5 method selection and username/password authentication.

**Status: work in progress. This is not yet a working SOCKS5 proxy.** The current source does not reliably complete authentication, does not parse the SOCKS5 request, does not connect to a destination, and does not relay traffic. The current build also fails; see [Known issues](#known-issues).

## What the program currently does

1. Parses command-line options for bind host, port, timeout, credentials, backlog, and verbosity.
2. Resolves the bind address with `getaddrinfo()`, creates a TCP socket, binds it, and listens.
3. Accepts one client at a time.
4. Sets receive and send timeouts on the accepted socket.
5. Reads bytes from the client. The networking code contains an initial SOCKS5 greeting check and a partial username/password check, but these are not yet safe or complete.

The observed `nc` tests confirmed that the TCP listener accepted clients and received data. That verifies the basic transport path only; it does not verify SOCKS5 authentication or proxying.

## Build

The Makefile builds into `build/`:

```sh
make
```

Force a complete rebuild:

```sh
make -B
```

Remove generated build output:

```sh
make clean
```

The current source does not build successfully. In `src/networking.c`, diagnostic paths use `printf(sprintf(NULL, ...))`; `sprintf()` needs a valid destination buffer, and its integer return value is not a format string for `printf()`. Replace those diagnostics with an appropriate direct output call before expecting `make` to pass.

The Makefile forces 64-bit compilation with `-m64`. It discovers C/C++ sources in the repository root and `src/`, includes headers from `include/`, and writes objects and the executable under `build/`.

## Run

After the build errors are fixed:

```sh
./build/socks5-enhance --host 127.0.0.1 --port 6969 --timeout 3
```

Available options in `src/main.c`:

| Option | Default | Purpose |
| --- | --- | --- |
| `-H`, `--host` | `127.0.0.1` | Address to bind |
| `-p`, `--port` | `6969` | TCP listening port |
| `-t`, `--timeout` | `3` | Client socket timeout in seconds |
| `-U`, `--username` | `user` | Configured username |
| `-P`, `--password` | A development default | Configured password |
| `-b`, `--backlog` | `128` | Listen backlog |
| `-v`, `--verbose` | off | Additional listener messages |
| `-h`, `--help` | n/a | Show parser help |

The username/password options currently configure values, but the server's authentication implementation is incomplete. Do not expose this service outside localhost. The current program also prints the password at startup; remove that before using real credentials.

## Python HTTPS test

`src/test.py` uses Requests and PySocks to request an HTTPS page through `socks5h://127.0.0.1:6969`. `socks5h` asks the proxy to resolve the destination hostname.

Create a virtual environment and install the test dependencies without modifying the system Python:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install 'requests[socks]'
```

Run the test with:

```sh
.venv/bin/python src/test.py
```

The test uses the same development credential defaults as `main.c`. It can be overridden with `SOCKS5_USERNAME` and `SOCKS5_PASSWORD` environment variables. The request will not succeed until the server completes SOCKS5 authentication, implements the CONNECT request, connects to the destination, and relays data.

## SOCKS5 protocol milestones

A complete authenticated CONNECT session has these stages:

1. **Greeting:** The client sends version `05`, a method count, and that many method IDs. Parse the method list instead of assuming a fixed three- or four-byte greeting. For mandatory username/password authentication, select method `02` only if offered. Reply `05 02`; if it was not offered, reply `05 FF` and close.
2. **Username/password authentication:** The client sends auth version `01`, a one-byte username length, username bytes, a one-byte password length, and password bytes. Reply `01 00` for success or `01 01` for failure. Close after failure.
3. **SOCKS request:** Parse the command and destination address. For HTTP(S) proxy use, the common command is CONNECT. Support IPv4, IPv6, and domain-name address types as desired.
4. **Destination connection:** Resolve/connect to the requested target and send the SOCKS5 request reply with the result.
5. **Relay:** Forward bytes in both directions until one side closes or a configured timeout/policy ends the connection.

TCP is a byte stream: a `read()` may return fewer bytes than requested or combine bytes from adjacent fields. Read and validate each protocol field according to its declared length. Do not use C string functions on raw protocol fields unless you create and bounds-check a terminated string; length-aware comparisons are more appropriate.

A client that offers both no-auth and username/password commonly sends a greeting like `05 02 00 02`. The `NMETHODS` byte is `02`, meaning two methods follow. The server must choose `02`, not reject the greeting just because it also contains `00`.

## Source layout

| Path | Role |
| --- | --- |
| `src/main.c` | CLI configuration, validation, startup, and the server loop |
| `src/networking.c` | Address resolution, socket setup, accept, timeout, and current partial protocol handler |
| `src/argparse.c` | Bundled command-line parser implementation |
| `src/test.py` | Requests/PySocks HTTPS smoke test |
| `include/argparse.h` | Parser types, option definitions, and API |
| `include/networking.h` | Listener and client-handler declarations |
| `include/libs.h` | Shared system and project includes |
| `include/types.h` | `i64` and `i32` integer aliases |
| `socks5-auth-guide.html` | Standalone visual guide to SOCKS5 greeting and username/password authentication |
| `Makefile` | Build and clean rules |

Generated/local directories include `build/`, `.venv/`, and `src/__pycache__/`.

## Known issues and next work

- **Current build blocker:** invalid `printf(sprintf(NULL, ...))` expressions in `src/networking.c`.
- The greeting parser only checks a couple of fixed packet shapes; it must use `NMETHODS` and parse the offered-method list safely.
- The handler relies on one `read()` containing a complete greeting or credential request. Handle short/fragmented reads and validate buffer lengths before indexing.
- The username/password arrays are not null-terminated, but are compared with `strcmp()`. Use length-aware byte comparisons or safely terminated copies.
- Check the result of `setsockopt()` and `send()`. A send may write fewer bytes than requested.
- After sending authentication success, transition to a SOCKS request state. The current code does not parse requests or implement CONNECT/relay.
- `socks_listen()` currently uses only the first `getaddrinfo()` result and does not try later IPv4/IPv6 candidates if setup fails.
- `include/types.h` includes `libs.h`, while `libs.h` includes `types.h`; remove this circular dependency as the headers are cleaned up.
- The server currently prints the configured password. Never log real credentials. SOCKS5 username/password authentication is not encrypted, so keep the listener on loopback while learning.
- Add tests for greeting variants, unsupported methods, valid/invalid credentials, fragmented reads, authentication replies, CONNECT failures, and relay behavior.

## Work completed so far

This project has been developed iteratively by the user with assistant support.

- The user built the C socket listener, added IPv4/IPv6 address resolution, a client accept/read loop, a configurable timeout, initial SOCKS5 greeting/authentication experiments, and manual `nc` tests.
- Assistant contributions included the Makefile source discovery/build-directory setup, CLI scaffolding, the Requests/PySocks test client and virtual-environment guidance, and the `socks5-auth-guide.html` protocol walkthrough. The assistant also helped diagnose socket, timeout, and protocol issues and organized the shared include list by header-name length at the user's request.
- The SOCKS5 negotiation, credential validation, destination connection, and data relay remain unfinished. The assistant has not implemented a complete proxy.
