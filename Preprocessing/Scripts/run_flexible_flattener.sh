#!/bin/bash

# Script to run FlexibleFlattener for various data types

print_help() {
  cat <<'EOF'
Usage: ./run_flexible_flattener.sh <type> <particle> <collision> [jobIdx] [inputfile] [suffix]

Required arguments:
  type        - Data type: 0=Data, 1=MC, 2=Mix
  particle    - Particle type: D0 or DStar
  collision   - Collision system: PbPb or pp

Optional arguments:
  jobIdx      - Job index for output directory naming (default: 0)
                If suffix is provided, this is ignored and suffix is used instead
  inputfile   - Custom input file path (optional; single ROOT file, file list, or directory)
  suffix      - Output suffix for custom directory naming (default: empty)

Examples:
  ./run_flexible_flattener.sh 0 DStar PbPb
  ./run_flexible_flattener.sh 1 DStar PbPb 5
  ./run_flexible_flattener.sh 0 DStar PbPb 0 /path/to/input.root
  ./run_flexible_flattener.sh 1 DStar PbPb 0 "" "MyCustomSuffix"
  ./run_flexible_flattener.sh 1 DStar PbPb 0 /path/to/input.root "Nov2025"

Output:
  Files are saved to: Data/FlatSample/{type}{collision}/{particle}/{suffix or job_{jobIdx}}/

Tips:
  -h, --help   Show this help message and exit
EOF
}

# Show help if requested
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
  print_help
  exit 0
fi

# Validate argument count: require 3..6 args
if [[ $# -lt 3 || $# -gt 6 ]]; then
  echo "ERROR: Invalid number of arguments: $# (expected 3-6)."
  echo "Hint: use -h for help."
  exit 1
fi

# Set ROOT environment
if [ -f /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh ]; then
    source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
elif [ -f /software/ROOT/root_v6.32.06.Linux-ubuntu22.04-x86_64-gcc11.4/bin/thisroot.sh ]; then
    source /software/ROOT/root_v6.32.06.Linux-ubuntu22.04-x86_64-gcc11.4/bin/thisroot.sh
else
    echo "WARNING: ROOT environment not found, proceeding without sourcing"
fi

# Get absolute path to DStarAnalysis directory
DSTAR_ANALYSIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"
FLAT_SAMPLE_DIR="${DSTAR_ANALYSIS_DIR}/Data/FlatSample"

# Default values
TYPE="$1"  # 0=Data, 1=MC, 2=Mix
PARTICLE="$2"
COLLISION="$3"
JOB_IDX=${4:-0}
INPUTFILE=${5:-""}  # Optional: custom input file
SUFFIX=${6:-""}     # Optional: output suffix

# Validate inputs
if [[ ! "$TYPE" =~ ^[012]$ ]]; then
    echo "ERROR: type must be 0 (Data), 1 (MC), or 2 (Mix)"
    exit 1
fi

if [[ "$PARTICLE" != "D0" && "$PARTICLE" != "DStar" ]]; then
    echo "ERROR: particle must be D0 or DStar"
    exit 1
fi

if [[ "$COLLISION" != "PbPb" && "$COLLISION" != "pp" ]]; then
    echo "ERROR: collision must be PbPb or pp"
    exit 1
fi

# Determine output directory based on type and collision system
case "$TYPE" in
    0)  # Data
        SUFFIX_DIR="${COLLISION}Data"
        ;;
    1)  # MC
        SUFFIX_DIR="${COLLISION}MC"
        ;;
    2)  # Mix
        SUFFIX_DIR="${COLLISION}Mix"
        ;;
esac

# Build output paths to mirror the macro
OUTPUT_BASE="${FLAT_SAMPLE_DIR}/${SUFFIX_DIR}"
if [[ "$TYPE" -ne 2 ]]; then
    OUTPUT_BASE="${OUTPUT_BASE}/${PARTICLE}"
fi
# SUFFIX가 없으면 job 인덱스 기반 폴더를 사용
if [[ -n "$SUFFIX" ]]; then
    OUTPUT_DIR="${OUTPUT_BASE}/${SUFFIX}"
else
    OUTPUT_DIR="${OUTPUT_BASE}/job_${JOB_IDX}"
fi
mkdir -p "$OUTPUT_DIR"

# Build ROOT macro call with optional parameters
if [[ -z "$INPUTFILE" && -z "$SUFFIX" ]]; then
    MACRO_CALL="FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX)"
elif [[ -z "$SUFFIX" ]]; then
    MACRO_CALL="FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX, 0, -1, \"$INPUTFILE\")"
else
    MACRO_CALL="FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX, 0, -1, \"$INPUTFILE\", \"$SUFFIX\")"
fi

echo "=========================================="
echo "Running FlexibleFlattener"
echo "=========================================="
echo "DStarAnalysis Dir: $DSTAR_ANALYSIS_DIR"
echo "Type: $TYPE ($([ $TYPE -eq 0 ] && echo 'Data' || ([ $TYPE -eq 1 ] && echo 'MC' || echo 'Mix')))"
echo "Particle: $PARTICLE"
echo "Collision: $COLLISION"
echo "Job Index: $JOB_IDX"
if [[ ! -z "$INPUTFILE" ]]; then
    echo "Input File: $INPUTFILE"
    if [[ "$INPUTFILE" == *.root ]]; then
        echo "Input Type: single ROOT file"
    elif [[ -f "$INPUTFILE" ]]; then
        echo "Input Type: file list"
    elif [[ -d "$INPUTFILE" ]]; then
        echo "Input Type: directory (recursive search)"
    else
        echo "WARNING: input path not found; relying on macro defaults"
    fi
fi
if [[ ! -z "$SUFFIX" ]]; then
    echo "Output Suffix: $SUFFIX"
fi
echo "Output Base (Absolute): $OUTPUT_BASE"
echo "Output directory: $OUTPUT_DIR"
echo "Macro Call: $MACRO_CALL"
echo "=========================================="

# Change to DStarAnalysis directory first
cd "$DSTAR_ANALYSIS_DIR" || exit 1

# Run ROOT macro with the correct include path resolution
# ROOT will execute from DStarAnalysis directory where includes can be resolved properly
if [[ -z "$INPUTFILE" && -z "$SUFFIX" ]]; then
    ROOT_CALL="root -l -b -q 'Preprocessing/Common/FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX)'"
elif [[ -z "$SUFFIX" ]]; then
    ROOT_CALL="root -l -b -q 'Preprocessing/Common/FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX, 0, -1, \"$INPUTFILE\")'"
else
    ROOT_CALL="root -l -b -q 'Preprocessing/Common/FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX, 0, -1, \"$INPUTFILE\", \"$SUFFIX\")'"
fi

mkdir -p "$OUTPUT_DIR"
eval $ROOT_CALL

if [ $? -eq 0 ]; then
    echo "✓ Job completed successfully"
    echo "Output saved to: $OUTPUT_DIR"
else
    echo "✗ Job failed with error"
    exit 1
fi
