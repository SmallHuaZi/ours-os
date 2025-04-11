#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Start debug"
${SCRIPT_DIR}/mkimg.sh
${SCRIPT_DIR}/runvm.sh
echo "Vm shutdown"