#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMMON_DIR="${SCRIPT_DIR}/../Common"
EXECUTABLE="${COMMON_DIR}/DStarRDSMaker"

if [[ ! -x "${EXECUTABLE}" ]]; then
    echo "ERROR: 실행 파일을 찾을 수 없습니다: ${EXECUTABLE}" >&2
    echo "먼저 Common 디렉터리에서 'make' 또는 './build.sh'를 실행해 주세요." >&2
    exit 1
fi

echo ">>> 실행: ${EXECUTABLE} $*"
"${EXECUTABLE}" "$@"
