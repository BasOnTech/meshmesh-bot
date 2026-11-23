#!/bin/sh
# Host check of the MM/1 packet code (lib/MeshProtocol): bounds, wire roundtrip, AAD, UTF-8, dedup, malformed input
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd);B=$(mktemp -d)
c++ -std=gnu++17 -O2 -Wall -Wextra -I"$ROOT/lib/MeshProtocol" "$ROOT/tests/protocol/protocol_check.cpp" -o "$B/protocol"
"$B/protocol";rc=$?;rm -rf "$B";exit $rc
