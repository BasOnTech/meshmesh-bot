#!/bin/sh
# Every check that runs on this computer alone: tests/run.sh (needs .pio/libdeps/m9: pio run -e m9 first).
# tests/web/*_check.cjs need data from a board (a history dump, .mmmap files) and the tools/hw/ checks need the boards.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd);B=$(mktemp -d);trap 'rm -rf "$B"' EXIT
sh "$ROOT/tests/protocol/protocol_check.sh"
sh "$ROOT/tests/chess/rules_check.sh"
sh "$ROOT/tests/chess/rating_check.sh"
node "$ROOT/tests/chess/companion_check.cjs"
KEEP="$B/duel" sh "$ROOT/tools/ui_preview/build.sh" m9 "$B/preview" en >/dev/null
python3 "$ROOT/tests/duel_check.py" "$B/duel" --games 4
echo "PASS all host checks"
