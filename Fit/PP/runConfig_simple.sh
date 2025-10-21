#!/bin/bash

# ========================================
# Simple PP Analysis Runner
# ========================================
# Single configuration test - easy to modify
# ========================================

# Check ROOT
if ! command -v root &> /dev/null; then
    echo "ERROR: ROOT not found! Run: ROOT6.24"
    exit 1
fi

# ========================================
# EDIT THESE PARAMETERS
# ========================================

DO_REFIT=1          # 0 or 1
DO_DCA=1            # 0 or 1
PLOT_FIT=1          # 0 or 1
USE_CUDA=1          # 0 or 1

PT_MIN=5            # GeV
PT_MAX=7            # GeV

COS_MIN=0.0         # -1 to 1
COS_MAX=0.2         # -1 to 1

CENT_MIN=0          # Not used in PP
CENT_MAX=100        # Not used in PP

# Parameter file (relative to Common/Parameters/pp/)
PARAM_FILE="../Common/Parameters/pp/dstar_parameters_DBCrystalBall_Phenomenological2.json"

IS_MC=0             # 0 = Data, 1 = MC

# ========================================
# RUN ANALYSIS
# ========================================

echo "========================================"
echo "PP Analysis - Simple Runner"
echo "========================================"
echo "Configuration:"
echo "  pT:  ${PT_MIN} - ${PT_MAX} GeV"
echo "  cos: ${COS_MIN} to ${COS_MAX}"
echo "  DCA: ${DO_DCA}"
echo "  MC:  ${IS_MC}"
echo "========================================"
echo ""

# Create log directory
mkdir -p logs

# Run analysis
LOG_FILE="logs/run_$(date +%Y%m%d_%H%M%S).log"

echo "Running analysis... (log: $LOG_FILE)"

root -l -b -q "DStarAnalysisPP.cpp(${DO_REFIT},${DO_DCA},${PLOT_FIT},${USE_CUDA},${PT_MIN},${PT_MAX},${COS_MIN},${COS_MAX},${CENT_MIN},${CENT_MAX},\"${PARAM_FILE}\",${IS_MC})" \
    2>&1 | tee "$LOG_FILE"

echo ""
echo "========================================"
echo "Analysis Complete!"
echo "========================================"
echo "Log: $LOG_FILE"
echo "Results: ../../Results/PP/"
echo ""

