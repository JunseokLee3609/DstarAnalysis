#!/bin/bash
# Wrapper script for FitSingleBin.cpp
# Usage: ./run_fit_single_bin.sh <kinVarInt> <varMin> <varMax> [<generateHistograms>]

set -e

KINVAR=$1
VAR_MIN=$2
VAR_MAX=$3
GEN_HISTS=${4:-0}

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

if [[ "$GEN_HISTS" -eq 1 ]]; then
  # Generate and fit
  root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 1, 1)"
elif [[ -f "$MC_CACHE" && -f "$DATA_CACHE" ]]; then
  # Fit using existing caches
  root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 0, 1)"
else
  # Fallback: generate caches only, then fit
  echo "Cache missing, generating first..."
  root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 1, 0)"
  root -l -b -q "FitSingleBin.cpp($KINVAR, $VAR_MIN, $VAR_MAX, 0, 1)"
fi

echo "Completed: $VNAME[$VAR_MIN, $VAR_MAX]"
