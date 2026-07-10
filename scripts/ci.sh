#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

NODE_BIN="${NODE:-}"
if [[ -z "$NODE_BIN" ]]; then
  for candidate in node nodejs node.exe; do
    if command -v "$candidate" >/dev/null 2>&1 && "$candidate" --version >/dev/null 2>&1; then
      NODE_BIN="$candidate"
      break
    fi
  done
fi
if [[ -z "$NODE_BIN" ]]; then
  echo "node executable not found in this shell environment" >&2
  exit 127
fi

"$NODE_BIN" scripts/check-format.mjs
"$NODE_BIN" scripts/build.mjs
"$NODE_BIN" --test --test-concurrency=1 "tests/node/*.test.js"
