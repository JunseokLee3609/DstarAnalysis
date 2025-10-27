#!/bin/bash

# ================================================
# Modular Yield Correction Workflow Runner
# ================================================
# Orchestrates all three stages with dependency resolution
# Stage 1: Build reweighting function
# Stage 2: Build efficiency map  
# Stage 3: Apply corrections and generate plots
# ================================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

RESULTS_DIR="${1:-results/data_fits}"
VAR_TYPE="${2:-0}"  # 0=pT, 1=rapidity, 2=centrality
STAGE="${3:-all}"   # all, 1, 2, 3
NORM_PLOTS="${4:-1}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MACRO="${SCRIPT_DIR}/CorrectYieldsWithMVA_Modular.cpp"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Modular Yield Correction Workflow${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo -e "${YELLOW}Configuration:${NC}"
echo "  Results Directory: ${RESULTS_DIR}"
echo "  Variable: ${VAR_TYPE} (0=pT, 1=y, 2=centrality)"
echo "  Stages: ${STAGE}"
echo "  Normalized Plots: ${NORM_PLOTS}\n"

# Check if input directory exists
if [ ! -d "${RESULTS_DIR}" ]; then
    echo -e "${RED}ERROR: Results directory not found: ${RESULTS_DIR}${NC}"
    exit 1
fi

# Count fit result files
FIT_FILES=$(find "${RESULTS_DIR}" -name "fitresult_*.root" 2>/dev/null | wc -l)
if [ $FIT_FILES -eq 0 ]; then
    echo -e "${RED}ERROR: No fit result files found in ${RESULTS_DIR}${NC}"
    echo "Please run FitSingleBin.cpp first to generate fit results."
    exit 1
fi

echo -e "${GREEN}Found ${FIT_FILES} fit result files${NC}\n"

# Run stages
case $STAGE in
    all)
        echo -e "${YELLOW}[Stage 1] Building Reweighting Function...${NC}"
        root -l -b -q "${MACRO}(${VAR_TYPE}, \"${RESULTS_DIR}\", 1, 0, 0, ${NORM_PLOTS})"
        
        echo -e "\n${YELLOW}[Stage 2] Building Efficiency Map...${NC}"
        root -l -b -q "${MACRO}(${VAR_TYPE}, \"${RESULTS_DIR}\", 0, 1, 0, ${NORM_PLOTS})"
        
        echo -e "\n${YELLOW}[Stage 3] Applying Corrections & Plotting...${NC}"
        root -l -b -q "${MACRO}(${VAR_TYPE}, \"${RESULTS_DIR}\", 0, 0, 1, ${NORM_PLOTS})"
        ;;
    1)
        echo -e "${YELLOW}[Stage 1] Building Reweighting Function...${NC}"
        root -l -b -q "${MACRO}(${VAR_TYPE}, \"${RESULTS_DIR}\", 1, 0, 0, ${NORM_PLOTS})"
        ;;
    2)
        echo -e "${YELLOW}[Stage 2] Building Efficiency Map (with dep check)...${NC}"
        root -l -b -q "${MACRO}(${VAR_TYPE}, \"${RESULTS_DIR}\", 0, 1, 0, ${NORM_PLOTS})"
        ;;
    3)
        echo -e "${YELLOW}[Stage 3] Applying Corrections & Plotting (with dep check)...${NC}"
        root -l -b -q "${MACRO}(${VAR_TYPE}, \"${RESULTS_DIR}\", 0, 0, 1, ${NORM_PLOTS})"
        ;;
    *)
        echo -e "${RED}Invalid stage: ${STAGE}${NC}"
        echo "Usage: $0 <results_dir> <var_type> <stage> <norm_plots>"
        echo "  stage: all, 1, 2, or 3"
        exit 1
        ;;
esac

echo -e "\n${BLUE}========================================${NC}"
echo -e "${GREEN}Workflow Complete!${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo "Output saved in: results/aggregated/"
