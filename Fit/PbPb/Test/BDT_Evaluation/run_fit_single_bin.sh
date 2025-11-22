#!/bin/bash
# Wrapper script for FitSingleBin.cpp
# Usage: ./run_fit_single_bin.sh <kinVarInt> <varMin> <varMax> [<generateHistograms>]
# Processes: inclusive, prompt (dca3D <= 0.01), nonprompt (dca3D > 0.01)

set -e

KINVAR=$1
VAR_MIN=$2
VAR_MAX=$3
GEN_HISTS=${4:-0}
DCA_CUT=0.01

# Setup ROOT environment
source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh

# Change to script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Build expected cache file paths
case "$KINVAR" in
  0) VNAME="pT";;
  1) VNAME="y";;
  2) VNAME="centrality";;
  *) VNAME="pT";;
 esac

printf -v VMIN_FMT "%.2f" "$VAR_MIN"
printf -v VMAX_FMT "%.2f" "$VAR_MAX"
MC_CACHE="results/histogram_cache/mc_histograms_${VNAME}_${VMIN_FMT}_${VMAX_FMT}.root"
DATA_CACHE="results/histogram_cache/data_histograms_${VNAME}_${VMIN_FMT}_${VMAX_FMT}.root"

# Ensure cache dir exists
mkdir -p results/histogram_cache

echo "=========================================="
echo "Processing: $VNAME[$VAR_MIN, $VAR_MAX]"
echo "DCA cut: $DCA_CUT"
echo "=========================================="

# Process three categories: inclusive (dcaCut=-1), prompt (dcaMode=1), nonprompt (dcaMode=2)
for CATEGORY in "incl" "prompt" "nonprompt"; do
  echo ""
  echo "--- Processing: $CATEGORY ---"
  
  case "$CATEGORY" in
    incl)
      DCA_MODE=0
      DCA_THRESHOLD=-1
      ;;
    prompt)
      DCA_MODE=1
      DCA_THRESHOLD=$DCA_CUT
      ;;
    nonprompt)
      DCA_MODE=2
      DCA_THRESHOLD=$DCA_CUT
      ;;
  esac
  
  if [[ "$GEN_HISTS" -eq 1 ]]; then
    # Generate and fit
    echo "Generating histograms and fitting..."
    root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 1, 1, $DCA_MODE, $DCA_THRESHOLD)" 2>&1 | tail -20
  elif [[ -f "$MC_CACHE" && -f "$DATA_CACHE" ]]; then
    # Fit using existing caches
    echo "Fitting using existing caches..."
    root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 0, 1, $DCA_MODE, $DCA_THRESHOLD)" 2>&1 | tail -20
  else
    # Fallback: generate caches only, then fit
    echo "Cache missing, generating first..."
    root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 1, 0, $DCA_MODE, $DCA_THRESHOLD)" 2>&1 | tail -20
    root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 0, 1, $DCA_MODE, $DCA_THRESHOLD)" 2>&1 | tail -20
  fi
  
  echo "✓ $CATEGORY completed"
done

echo ""
echo "=========================================="
echo "Completed: $VNAME[$VAR_MIN, $VAR_MAX]"
echo "=========================================="
