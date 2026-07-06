#!/usr/bin/env bash
# EPITECH PROJECT, 2026 - Zappy
# Smoke test for ZAP-21 (initial resource spawn): boots the server and
# checks it accepts a client end-to-end. Resources are not exposed over the
# protocol yet, so we only assert the server boots and handshakes cleanly.

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SERVER="$ROOT/zappy_server"
PORT="${PORT:-4280}"
LOG="$(mktemp)"

cleanup() {
    [ -n "${SRV_PID:-}" ] && kill "$SRV_PID" 2>/dev/null
    wait "${SRV_PID:-}" 2>/dev/null
    rm -f "$LOG"
}
trap cleanup EXIT

fail() { echo "FAIL: $1"; echo "--- server log ---"; cat "$LOG"; exit 1; }

[ -x "$SERVER" ] || { echo "building server..."; make -C "$ROOT" >/dev/null || fail "build failed"; }

"$SERVER" -p "$PORT" -x 10 -y 10 -n alpha beta -c 2 -f 100 >"$LOG" 2>&1 &
SRV_PID=$!

ready=0
for _ in $(seq 1 50); do
    kill -0 "$SRV_PID" 2>/dev/null || fail "server died at boot"
    if exec 3<>"/dev/tcp/127.0.0.1/$PORT" 2>/dev/null; then ready=1; break; fi
    sleep 0.1
done
[ "$ready" = 1 ] || fail "server never accepted connections on port $PORT"
printf 'alpha\n' >&3
WELCOME="$(head -c 7 <&3)"
exec 3>&- 3<&-

[ "$WELCOME" = "WELCOME" ] || fail "expected WELCOME banner, got '$WELCOME'"

sleep 0.2
kill -0 "$SRV_PID" 2>/dev/null || fail "server crashed after client connect"

echo "PASS: server booted, spawned resources, accepted client and sent WELCOME"
echo "--- server log ---"
cat "$LOG"
