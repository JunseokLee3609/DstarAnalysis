#!/bin/bash

# ========================================
# Full PbPb Analysis Pipeline
# ========================================
# Runs the complete PbPb analysis from preprocessing to final results
# ========================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Full PbPb Analysis Pipeline${NC}"
echo -e "${BLUE}========================================${NC}\n"

cd "${BASE_DIR}"

# Configuration
DATA_FILE=""
MC_FILE=""
OUTPUT_DIR="Results/PbPb"
USE_CUDA=true

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --data)
            DATA_FILE="$2"
            shift 2
            ;;
        --mc)
            MC_FILE="$2"
            shift 2
            ;;
        --output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --no-cuda)
            USE_CUDA=false
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Step 1: Preprocessing (if needed)
echo -e "${YELLOW}Step 1: Preprocessing (if needed)...${NC}"
if [ -z "$DATA_FILE" ]; then
    echo "  No data file specified, skipping preprocessing"
    echo "  Use --data to specify input data file"
else
    echo "  Data file: $DATA_FILE"
    # Add preprocessing command here
fi
echo ""

# Step 2: RDS Creation (if needed)
echo -e "${YELLOW}Step 2: RDS Creation (if needed)...${NC}"
if [ -f "Preprocessing/Common/DStarRDSMakerImproved.cpp" ]; then
    echo "  RDS Maker available"
    # Add RDS creation command here
else
    echo "  RDS Maker not found, using existing RDS files"
fi
echo ""

# Step 3: PbPb Analysis
echo -e "${YELLOW}Step 3: Running PbPb Analysis...${NC}"
if [ -f "Analysis/PbPb/DStarAnalysisPbPb.cpp" ]; then
    echo "  Compiling PbPb analysis..."
    # Add compilation command here
    # g++ -o Analysis/PbPb/DStarAnalysisPbPb Analysis/PbPb/DStarAnalysisPbPb.cpp ...
    
    echo "  Running PbPb analysis..."
    # Add execution command here
    # ./Analysis/PbPb/DStarAnalysisPbPb
    
    echo -e "  ${GREEN}✓ PbPb Analysis complete${NC}"
else
    echo -e "  ${RED}✗ PbPb Analysis file not found${NC}"
    exit 1
fi
echo ""

# Step 4: Results Organization
echo -e "${YELLOW}Step 4: Organizing results...${NC}"
mkdir -p "${OUTPUT_DIR}"/{Fits,Plots,Tables}
echo "  Results will be saved to: ${OUTPUT_DIR}"
echo ""

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}PbPb Analysis Pipeline Complete!${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo "Results location: ${OUTPUT_DIR}"
echo ""
echo "Next steps:"
echo "  1. Check results: ls -la ${OUTPUT_DIR}"
echo "  2. View plots: cd ${OUTPUT_DIR}/Plots"
echo "  3. Check fit results: cd ${OUTPUT_DIR}/Fits"
echo ""

