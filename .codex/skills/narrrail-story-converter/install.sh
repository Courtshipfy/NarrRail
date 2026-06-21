#!/usr/bin/env bash
set -euo pipefail

REPO="${NARRRAIL_SKILL_REPO:-Courtshipfy/NarrRail}"
REF="${NARRRAIL_SKILL_REF:-main}"
PATH_IN_REPO=".codex/skills/narrrail-story-converter"
CODEX_HOME="${CODEX_HOME:-$HOME/.codex}"
INSTALLER="$CODEX_HOME/skills/.system/skill-installer/scripts/install-skill-from-github.py"
PYTHON_BIN="${PYTHON_BIN:-}"

if [[ ! -f "$INSTALLER" ]]; then
  echo "Codex skill installer not found: $INSTALLER" >&2
  echo "Install or update Codex, then retry." >&2
  exit 1
fi

if [[ -z "$PYTHON_BIN" ]]; then
  if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN="python3"
  elif command -v python >/dev/null 2>&1; then
    PYTHON_BIN="python"
  else
    echo "Python not found. Install Python 3 or set PYTHON_BIN=/path/to/python3." >&2
    exit 1
  fi
fi

"$PYTHON_BIN" "$INSTALLER" --repo "$REPO" --ref "$REF" --path "$PATH_IN_REPO"
echo "Restart Codex to pick up the narrrail-story-converter skill."
