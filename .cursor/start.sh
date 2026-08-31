#!/usr/bin/env bash
# Per-boot runtime setup for the Coreria environment.
#
# 1. Rebuilds the ChaosForge C game. Its binary and .o files are committed to
#    git, so a fresh agent's checkout restores the stale prebuilt Windows/MinGW
#    objects on top of whatever `install` produced. Recompiling here (a
#    sub-second job) guarantees every boot has a correct native Linux ELF.
# 2. Starts a virtual X display so the OpenGL game can run headless.
# Idempotent: safe to run repeatedly.
set -euo pipefail

# --- Rebuild the OpenGL game for the current Linux host --------------------
if [ -d chaosforge-game ]; then
  if make -C chaosforge-game clean && make -C chaosforge-game; then
    chmod +x chaosforge-game/chaosforge 2>/dev/null || true
    echo "ChaosForge game rebuilt (native Linux ELF)."
  else
    echo "WARNING: ChaosForge game rebuild failed; continuing boot." >&2
  fi
fi

# --- Virtual display for headless OpenGL ----------------------------------
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
