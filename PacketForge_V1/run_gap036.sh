#!/usr/bin/env bash
set -u
ROOT="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)}"
ROOT="$(cd "$ROOT" && pwd)"
SERVER="$ROOT/bin/packetforge_server"
CLIENT="$ROOT/bin/packetforge_integration_success_client"
LOG_DIR="$ROOT/logs/gap036"
CYCLES=3
mkdir -p "$LOG_DIR"
[[ -x "$SERVER" ]] || { echo "GAP-036: FAIL - server missing"; exit 1; }
[[ -x "$CLIENT" ]] || { echo "GAP-036: FAIL - client missing"; exit 1; }
overall=0
for cycle in $(seq 1 "$CYCLES"); do
  server_log="$LOG_DIR/cycle_${cycle}_server.log"
  client_log="$LOG_DIR/cycle_${cycle}_client.log"
  rm -f "$server_log" "$client_log"
  echo "--- GAP-036 Cycle $cycle ---"
  printf '\n' | "$SERVER" >"$server_log" 2>&1 &
  server_pid=$!
  ready=0
  for _ in $(seq 1 50); do
    if grep -q "Listening on 127.0.0.1:9090" "$server_log" 2>/dev/null; then ready=1; break; fi
    if ! kill -0 "$server_pid" 2>/dev/null; then break; fi
    sleep 0.1
  done
  if [[ $ready -ne 1 ]]; then echo "Server startup/listening: FAIL"; cat "$server_log"; overall=1; kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true; continue; fi
  echo "Server startup/listening: PASS"
  "$CLIENT" >"$client_log" 2>&1; client_rc=$?
  wait "$server_pid"; server_rc=$?
  [[ $client_rc -eq 0 ]] && echo "Client workflow: PASS (exit $client_rc)" || { echo "Client workflow: FAIL (exit $client_rc)"; overall=1; }
  [[ $server_rc -eq 0 ]] && echo "Server termination: PASS (exit $server_rc)" || { echo "Server termination: FAIL (exit $server_rc)"; overall=1; }
  if grep -q "Connect: PASS" "$client_log" && grep -q "Disconnect: PASS" "$client_log"; then echo "Client lifecycle evidence: PASS"; else echo "Client lifecycle evidence: REVIEW"; overall=1; fi
  if grep -q "Client connected successfully" "$server_log" && grep -q "Packet received successfully" "$server_log" && grep -q "Response packet sent successfully" "$server_log" && grep -q "PacketForge server stopped successfully" "$server_log"; then echo "Server lifecycle evidence: PASS"; else echo "Server lifecycle evidence: REVIEW"; overall=1; fi
  echo "Client log:"; cat "$client_log"; echo "Server log:"; cat "$server_log"; echo
done
echo "============================================================"
if [[ $overall -eq 0 ]]; then echo "GAP-036 repeated connect/disconnect: PASS"; else echo "GAP-036 repeated connect/disconnect: FAIL / REVIEW"; fi
echo "Logs: $LOG_DIR"
exit "$overall"
