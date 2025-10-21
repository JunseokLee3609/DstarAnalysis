#!/bin/bash

set -euo pipefail

print_usage() {
    cat <<'EOF'
사용법: ./run_rds_root.sh [옵션]

옵션:
  --data, -d          데이터 처리 (기본: MC)
  --mc, -m            MC 처리 (기본값)
  --d0, -0            D0 처리 (기본: D*)
  --dstar, -s         D* 처리 (기본값)
  --pp, -p            pp 충돌 (기본값)
  --pbpb, -b          PbPb 충돌
  --dataset, -k KEY   Data/datasets.json에 정의된 데이터셋 키
  --catalog, -c PATH  데이터셋 카탈로그 JSON 경로 (기본: repo/Data/datasets.json)
  --input, -i PATH    입력 ROOT 파일 경로 (dataset 사용 시 우선순위 낮음)
  --suffix, -x STR    출력 파일 접미사
  --eff, -e PATH      효율 맵 ROOT 파일 경로
  --effname, -n STR   효율 맵 히스토그램 이름
  --help, -h          도움말 표시

예시:
  ./run_rds_root.sh --data --dstar --pp \
      --input /path/to/input.root \
      --suffix test \
      --eff /path/to/eff.root \
      --effname efficiency_map
EOF
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMMON_DIR="${SCRIPT_DIR}/../Common"
MACRO_PATH="${COMMON_DIR}/DStarRDSMakerImproved.cpp"

if [[ ! -f "${MACRO_PATH}" ]]; then
    echo "ERROR: 매크로 파일을 찾을 수 없습니다: ${MACRO_PATH}" >&2
    exit 1
fi

if ! command -v root >/dev/null 2>&1; then
    echo "ERROR: ROOT 환경을 찾을 수 없습니다. 먼저 ROOT를 활성화해 주세요." >&2
    exit 1
fi

isMC=1
isD0=0
isPP=1
inputPath=""
suffix=""
effFile=""
effMapName=""
datasetKey=""
datasetCatalog="$(cd "${SCRIPT_DIR}/../.." && pwd)/Data/datasets.json"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --data|-d) isMC=0 ;;
        --mc|-m) isMC=1 ;;
        --d0|-0) isD0=1 ;;
        --dstar|-s) isD0=0 ;;
        --pp|-p) isPP=1 ;;
        --pbpb|-b) isPP=0 ;;
        --dataset|-k)
            [[ $# -ge 2 ]] || { echo "ERROR: --dataset 옵션에는 키가 필요합니다." >&2; exit 1; }
            datasetKey="$2"; shift
            ;;
        --catalog|-c)
            [[ $# -ge 2 ]] || { echo "ERROR: --catalog 옵션에는 경로가 필요합니다." >&2; exit 1; }
            datasetCatalog="$2"; shift
            ;;
        --input|-i)
            [[ $# -ge 2 ]] || { echo "ERROR: --input 옵션에는 경로가 필요합니다." >&2; exit 1; }
            inputPath="$2"; shift
            ;;
        --suffix|-x)
            [[ $# -ge 2 ]] || { echo "ERROR: --suffix 옵션에는 문자열이 필요합니다." >&2; exit 1; }
            suffix="$2"; shift
            ;;
        --eff|-e)
            [[ $# -ge 2 ]] || { echo "ERROR: --eff 옵션에는 경로가 필요합니다." >&2; exit 1; }
            effFile="$2"; shift
            ;;
        --effname|-n)
            [[ $# -ge 2 ]] || { echo "ERROR: --effname 옵션에는 이름이 필요합니다." >&2; exit 1; }
            effMapName="$2"; shift
            ;;
        --help|-h)
            print_usage
            exit 0
            ;;
        *)
            echo "ERROR: 알 수 없는 옵션: $1" >&2
            print_usage
            exit 1
            ;;
    esac
    shift
done

escape_arg() {
    local value="$1"
    value="${value//\\/\\\\}"
    value="${value//\"/\\\"}"
    printf '%s' "$value"
}

inputEscaped=$(escape_arg "$inputPath")
suffixEscaped=$(escape_arg "$suffix")
effFileEscaped=$(escape_arg "$effFile")
effMapEscaped=$(escape_arg "$effMapName")
datasetKeyEscaped=$(escape_arg "$datasetKey")
catalogEscaped=$(escape_arg "$datasetCatalog")

ROOT_CMD="${MACRO_PATH}(${isMC},${isD0},${isPP},\"${inputEscaped}\",\"${suffixEscaped}\",\"${effFileEscaped}\",\"${effMapEscaped}\",\"${datasetKeyEscaped}\",\"${catalogEscaped}\")"

echo ">>> ROOT 매크로 실행:"
echo "    root -l -q '${ROOT_CMD}'"

root -l -q "${ROOT_CMD}"
