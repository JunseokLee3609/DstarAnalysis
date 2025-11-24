#!/bin/bash

# Script to run FlexibleFlattener for various data types

# Function to display usage
show_usage() {
    cat <<EOF
Usage: ./run_flexible_flattener.sh <type> <particle> <collision> [options]

Required Arguments:
  type        - Data type: 0=Data, 1=MC, 2=Mix
  particle    - Particle type: D0 or DStar
  collision   - Collision system: PbPb or pp

Optional Arguments:
  --job-idx NUM               - Job index (default: 0)
  --start NUM                 - Start entry index (default: 0)
  --end NUM                   - End entry index, -1 for all (default: -1)
  --input FILE                - Input file/directory path (optional)
  --suffix NAME               - Output suffix for custom directory naming (optional)
  --evtplane-calib-file FILE  - Event plane calibration file (optional)
  --help                      - Show this help message

Examples:
  # Basic usage (Data, DStar, PbPb, job 0)
  ./run_flexible_flattener.sh 0 DStar PbPb

  # MC with custom job index
  ./run_flexible_flattener.sh 1 DStar PbPb --job-idx 5

  # MC with entry range
  ./run_flexible_flattener.sh 1 DStar PbPb --start 0 --end 100000

  # MC with custom input and suffix
  ./run_flexible_flattener.sh 1 DStar PbPb --input /path/to/input.root --suffix MyCustom

  # PbPb DStar with event plane calibration
  ./run_flexible_flattener.sh 1 DStar PbPb --evtplane-calib-file /path/to/calib.root

  # Full example
  ./run_flexible_flattener.sh 1 DStar PbPb --job-idx 2 --start 0 --end 50000 --input /data/mc.root --suffix Nov2025 --evtplane-calib-file /calib.root

Output:
  Files are saved to: Data/FlatSample/{type}{collision}/{particle}/{suffix or job_{jobIdx}}/
EOF
}

# Check if required arguments are provided
if [ $# -lt 3 ]; then
    echo "ERROR: Missing required arguments"
    echo ""
    show_usage
    exit 1
fi

# Display help if requested
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    show_usage
    exit 0
fi

# Default values
TYPE="$1"
PARTICLE="$2"
COLLISION="$3"
JOB_IDX=0
START=0
END=-1
INPUTFILE=""
SUFFIX=""
EVTPLANE_CALIB_FILE=""

# Parse optional arguments
shift 3  # Remove the first 3 required arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --job-idx)
            JOB_IDX="$2"
            shift 2
            ;;
        --start)
            START="$2"
            shift 2
            ;;
        --end)
            END="$2"
            shift 2
            ;;
        --input)
            INPUTFILE="$2"
            shift 2
            ;;
        --suffix)
            SUFFIX="$2"
            shift 2
            ;;
        --evtplane-calib-file)
            EVTPLANE_CALIB_FILE="$2"
            shift 2
            ;;
        --help|-h)
            show_usage
            exit 0
            ;;
        *)
            echo "ERROR: Unknown option '$1'"
            show_usage
            exit 1
            ;;
    esac
done

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

# Validate numeric parameters
if ! [[ "$JOB_IDX" =~ ^[0-9]+$ ]]; then
    echo "ERROR: job-idx must be a non-negative integer"
    exit 1
fi

if ! [[ "$START" =~ ^[0-9]+$ ]]; then
    echo "ERROR: start must be a non-negative integer"
    exit 1
fi

if ! [[ "$END" =~ ^-?[0-9]+$ ]]; then
    echo "ERROR: end must be an integer (-1 for all)"
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

# Setup logging
LOG_DIR="${DSTAR_ANALYSIS_DIR}/log"
mkdir -p "$LOG_DIR"
LOG_FILE="${LOG_DIR}/flexible_flattener.log"

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

# Build ROOT macro call with supported parameters
MACRO_CALL="FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX, $START, $END, \"$INPUTFILE\", \"$SUFFIX\", \"$EVTPLANE_CALIB_FILE\")"

echo "=========================================="
echo "Running FlexibleFlattener"
echo "=========================================="
echo "DStarAnalysis Dir: $DSTAR_ANALYSIS_DIR"
echo "Type: $TYPE ($([ $TYPE -eq 0 ] && echo 'Data' || ([ $TYPE -eq 1 ] && echo 'MC' || echo 'Mix')))"
echo "Particle: $PARTICLE"
echo "Collision: $COLLISION"
echo "Job Index: $JOB_IDX"
echo "Start Entry: $START"
echo "End Entry: $END"
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
if [[ ! -z "$EVTPLANE_CALIB_FILE" ]]; then
    echo "Event Plane Calib File: $EVTPLANE_CALIB_FILE"
fi
echo "Output Base (Absolute): $OUTPUT_BASE"
echo "Output directory: $OUTPUT_DIR"
echo "Macro Call: $MACRO_CALL"
echo "=========================================="

# Change to DStarAnalysis directory first
cd "$DSTAR_ANALYSIS_DIR" || exit 1

# Run ROOT macro with supported parameters
ROOT_CALL="root -l -b -q 'Preprocessing/Common/FlexibleFlattener.cpp($TYPE, \"$PARTICLE\", \"$COLLISION\", $JOB_IDX, $START, $END, \"$INPUTFILE\", \"$SUFFIX\", \"$EVTPLANE_CALIB_FILE\")'"

mkdir -p "$OUTPUT_DIR"

# Log the command with timestamp
echo "[$(date '+%Y-%m-%d %H:%M:%S')] $0 $@" >> "$LOG_FILE"

eval $ROOT_CALL

if [ $? -eq 0 ]; then
    echo "✓ Job completed successfully"
    echo "Output saved to: $OUTPUT_DIR"
else
    echo "✗ Job failed with error"
    exit 1
fi
