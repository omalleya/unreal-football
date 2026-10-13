#!/bin/bash
set -euo pipefail
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UE_ROOT="${UE_ROOT:-/Users/Shared/Epic Games/UE_5.8}"
exec "$UE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh" FootballEditor Mac Development "$PROJECT_ROOT/Football.uproject" -WaitMutex -NoHotReloadFromIDE
