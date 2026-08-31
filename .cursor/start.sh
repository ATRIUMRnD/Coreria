#!/usr/bin/env bash
# Per-boot runtime setup for the Coreria environment.
# Starts a virtual X display so the OpenGL ChaosForge game can run headless
# (e.g. for screenshots / smoke tests). Idempotent: no-op if already running.
set -euo pipefail

DISPLAY_NUM=99

if ! xdpyinfo -display ":${DISPLAY_NUM}" >/dev/null 2>&1; then
  Xvfb ":${DISPLAY_NUM}" -screen 0 1024x768x24 >/tmp/xvfb.log 2>&1 &
  # Give the server a moment to come up so downstream GL clients can connect.
  for _ in $(seq 1 20); do
    if xdpyinfo -display ":${DISPLAY_NUM}" >/dev/null 2>&1; then
      break
    fi
    sleep 0.25
  done
fi

echo "Virtual display available on DISPLAY=:${DISPLAY_NUM} (Xvfb)."
