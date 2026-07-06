
## What this is

`zappy_server` is the referee of the Zappy game. It listens on TCP, accepts AI clients (autonomous players) and GUI clients (viewers), simulates the world, and arbitrates the match. It is single threaded and uses non blocking I/O multiplexed with `poll()`. There is no global state: even SIGINT is delivered through a `signalfd` and handled as a normal loop event, so shutdown is clean.

## Layered architecture

```
src/server/      application logic (CLI, clients, game, dispatchers)
libs/protocol/   Zappy protocol parsing and serialization (libzappy_protocol)
libs/net/        low level network engine (libzappy_net)
libs/posix/      RAII wrappers over POSIX resources (libzappy_posix)
```

The three libraries are built as static archives and linked into the server. `libs/protocol` is pure logic with no network dependency, so it can be unit tested in isolation.

## Reading order for this documentation set

1. Overview (this file)
2. POSIX and Network layer
3. Protocol layer
4. Server core and orchestration
5. Game logic
6. AI command handling
7. GUI protocol and notifications

## Build and run

```sh
make            # builds zappy_server, zappy_gui, zappy_ai
make debug      # debug build with AddressSanitizer
make tests_run  # Criterion unit tests (libs and server)
make tests_cov  # tests plus gcovr coverage
make re         # full rebuild

./zappy_server -p 4242 -x 10 -y 10 -n team1 team2 -c 5 -f 100
```

## Command line arguments

| Flag | Required | Meaning | Constraint |
|------|----------|---------|-----------|
| `-p` | yes | TCP port | 1 to 65535 |
| `-x` / `-y` | yes | world width and height in tiles | greater than 0 |
| `-n name...` | yes | team names | unique, `GRAPHIC` is reserved |
| `-c` | yes | clients allowed per team | greater than 0 |
| `-f` | no | inverse of the time unit for actions | greater than 0, default 100 |
| `--help` | no | print usage and exit | |

Bonus flags are parsed but only partially wired: `--enable-events`, `--enable-biomes`, `--enable-admin`, `--admin-password`. Any invalid or missing argument prints the usage and exits with code `84`.

## Quick manual test

```sh
nc 127.0.0.1 4242
# server sends: WELCOME
# reply "team1"   -> remaining slots, then world dimensions (AI client)
# reply "GRAPHIC" -> connect as a GUI client
```

## End to end flow at a glance

1. A connection is accepted, the client starts in the `HANDSHAKE` state, and receives `WELCOME`.
2. The first line promotes the client to `AI` or `GUI`, otherwise the connection is dropped.
3. AI commands are queued and scheduled in time, then applied to the world.
4. Every world mutation is broadcast to GUI clients as protocol lines.
5. Timers (food, resource refill, incantation end, fork) are driven by a scheduler tied into the poll loop.
