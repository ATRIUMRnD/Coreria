#!/usr/bin/env bash
# Per-boot runtime setup for the Coreria environment.
# Starts a virtual X display so the OpenGL ChaosForge game can run headless
# (screenshots / smoke tests). The game binary itself is produced by
# .cursor/install.sh and persists via .gitignore, so it is ready without a
# rebuild here. Idempotent: no-op if the display is already running.
set -euo pipefail

DISPLAY_NUM=99

if ! xdpyinfo -display ":${DISPLAY_NUM}" >/dev/null 2>&1; then
  Xvfb ":${DISPLAY_NUM}" -screen 0 1024x768x24 >/tmp/xvfb.log 2>&1 &
  for _ in $(seq 1 20); do
    if xdpyinfo -display ":${DISPLAY_NUM}" >/dev/null 2>&1; then
      break
    fi
    sleep 0.25
  done
fi

if xdpyinfo -display ":${DISPLAY_NUM}" >/dev/null 2>&1; then
  echo "Virtual display available on DISPLAY=:${DISPLAY_NUM} (Xvfb)."
else
  echo "ERROR: failed to start Xvfb on :${DISPLAY_NUM}." >&2
  exit 1
fi
