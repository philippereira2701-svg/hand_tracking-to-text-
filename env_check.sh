#!/bin/bash
set -e

echo "== Build Environment Check =="

warn=0

echo "- g++ path: $(command -v g++ || echo 'not found')"
echo "- ld path:  $(command -v ld || echo 'not found')"
echo "- pkg-config path: $(command -v pkg-config || echo 'not found')"

if [ -n "${CONDA_PREFIX:-}" ] || [ -n "${CONDA_DEFAULT_ENV:-}" ]; then
  echo "! Warning: Conda environment appears active."
  warn=1
fi

if [ -n "${LD_LIBRARY_PATH:-}" ]; then
  echo "! Warning: LD_LIBRARY_PATH is set."
  warn=1
fi

if [ -n "${LIBRARY_PATH:-}" ]; then
  echo "! Warning: LIBRARY_PATH is set."
  warn=1
fi

if [ -n "${CPATH:-}" ] || [ -n "${C_INCLUDE_PATH:-}" ] || [ -n "${CPLUS_INCLUDE_PATH:-}" ]; then
  echo "! Warning: include path environment variables are set."
  warn=1
fi

if [ -n "${PKG_CONFIG_PATH:-}" ]; then
  echo "! Warning: PKG_CONFIG_PATH is set."
  warn=1
fi

if pkg-config --exists opencv4; then
  echo "- opencv4 via pkg-config: OK"
else
  echo "! Warning: opencv4 not found via pkg-config."
  warn=1
fi

if [ "$warn" -eq 0 ]; then
  echo "Environment check: OK (no common contamination found)."
else
  echo "Environment check: WARNINGS found."
fi
