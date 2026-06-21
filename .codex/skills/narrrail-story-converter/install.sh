#!/usr/bin/env bash
set -euo pipefail

REPO="${NARRRAIL_SKILL_REPO:-Courtshipfy/NarrRail}"
REF="${NARRRAIL_SKILL_REF:-main}"
PATH_IN_REPO=".codex/skills/narrrail-story-converter"
CODEX_HOME="${CODEX_HOME:-$HOME/.codex}"
INSTALLER="$CODEX_HOME/skills/.system/skill-installer/scripts/install-skill-from-github.py"

if [[ ! -f "$INSTALLER" ]]; then
  echo "Codex skill installer not found: $INSTALLER" >&2
  echo "Install or update Codex, then retry." >&2
  exit 1
fi

python "$INSTALLER" --repo "$REPO" --ref "$REF" --path "$PATH_IN_REPO"
echo "Restart Codex to pick up the narrrail-story-converter skill."

