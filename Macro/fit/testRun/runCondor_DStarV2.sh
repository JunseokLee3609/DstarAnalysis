#!/bin/bash
# runCondor_DStarV2.sh

set -euo pipefail

# Source the ROOT environment if not already set
#if [ -z "${ROOTSYS:-}" ]; then
  source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
 # source /software/ROOT/ROOT-v6.32/root-6.32-install/bin/thisroot.sh
#fi

# Move to project testRun directory for stable relative paths
cd /home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/

# Arguments
pTMin=$1
pTMax=$2
cosMin=$3
cosMax=$4
centMin=$5
centMax=$6


mkdir -p logs

# echo "[Condor DStarV2] doReFit=${doReFit}, plotFit=${plotFit}, useCUDA=${useCUDA}"
echo "[Condor DStarV2] pT=[${pTMin}, ${pTMax}], cos=[${cosMin}, ${cosMax}], cent=[${centMin}, ${centMax}]"
# echo "[Condor DStarV2] paramFile='${parameterFile}', isMC=${isMC}"

# Ensure C++17 is used by Cling/ACLiC (needed for std::variant/visit)
export ROOT_CXXFLAGS="${ROOT_CXXFLAGS:-} -std=c++17"

# Execute ROOT macro with C++17 standard explicitly set
root -l -b -q "DStarAnalysisV2.cpp(true,true,true,${pTMin},${pTMax},${cosMin},${cosMax},${centMin},${centMax},\"/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/parameters/dstar_parameters_grid_unified.json\",false)"
