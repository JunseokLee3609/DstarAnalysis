#!/bin/bash

# ========================================
# PP Analysis Configuration Runner
# ========================================
# Quick test configurations for PP analysis
# ========================================

set -e  # Exit on error

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
SCRIPT="DStarAnalysisPP.cpp"
PARAM_DIR="Parameters"

# Check if ROOT is setup
if ! command -v root &> /dev/null; then
    echo -e "${RED}ERROR: ROOT not found!${NC}"
    echo -e "${YELLOW}Please run: ROOT6.24${NC}"
    exit 1
fi

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}PP Analysis Test Runner${NC}"
echo -e "${BLUE}========================================${NC}\n"

# ========================================
# Test Configurations
# ========================================

# Configuration 1: Quick Test (Single bin)
run_quick_test() {
    echo -e "${YELLOW}[Config 1] Quick Test - Single pT bin${NC}"
    
    # Arguments:
    # doReFit, doDCA, plotFit, useCUDA, pTMin, pTMax, cosMin, cosMax, 
    # centralityMin, centralityMax, parameterFile, isMC
    
    root -l -b -q "${SCRIPT}(1,0,1,1,5,7,0,0.2,0,100,\"${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PP.json\",0)" \
        2>&1 | tee logs/quick_test_$(date +%Y%m%d_%H%M%S).log
    
    echo -e "${GREEN}✓ Quick test completed${NC}\n"
}

# Configuration 2: Full cosTheta Scan
run_costheta_scan() {
    echo -e "${YELLOW}[Config 2] Full cosTheta Scan${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_dca_v2.json"
    PT_MIN=5
    PT_MAX=7
    
    # cosTheta bins
    COS_BINS=(
        "-0.8 -0.6"
        "-0.6 -0.4"
        "-0.4 -0.2"
        "-0.2  0.0"
        " 0.0  0.2"
        " 0.2  0.4"
        " 0.4  0.6"
        " 0.6  0.8"
    )
    
    for bin in "${COS_BINS[@]}"; do
        COS_MIN=$(echo $bin | awk '{print $1}')
        COS_MAX=$(echo $bin | awk '{print $2}')
        
        echo -e "  Running cos: ${COS_MIN} to ${COS_MAX}"
        
        root -l -b -q "${SCRIPT}(1,1,1,1,${PT_MIN},${PT_MAX},${COS_MIN},${COS_MAX},0,100,\"${PARAM_FILE}\",0,0)" \
            2>&1 | tee logs/costheta_scan_${COS_MIN}_${COS_MAX}_$(date +%Y%m%d_%H%M%S).log &
        
        # Wait between jobs to avoid overwhelming system
        sleep 2
    done
    
    wait
    echo -e "${GREEN}✓ cosTheta scan completed${NC}\n"
}

# Configuration 3: pT Scan
run_pt_scan() {
    echo -e "${YELLOW}[Config 3] pT Scan${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PP.json"
    
    # pT bins
    PT_BINS=(
        "5  7"
        "7  10"
        "10 15"
        "15 20"
        "20 30"
    )
    
    for bin in "${PT_BINS[@]}"; do
        PT_MIN=$(echo $bin | awk '{print $1}')
        PT_MAX=$(echo $bin | awk '{print $2}')
        
        echo -e "  Running pT: ${PT_MIN} to ${PT_MAX} GeV"
        
        root -l -b -q "${SCRIPT}(1,1,1,1,${PT_MIN},${PT_MAX},-2,2,0,100,\"${PARAM_FILE}\",0)" \
            2>&1 | tee logs/pt_scan_${PT_MIN}_${PT_MAX}_$(date +%Y%m%d_%H%M%S).log
    done
    
    echo -e "${GREEN}✓ pT scan completed${NC}\n"
}

# Configuration 4: MC Test
run_mc_test() {
    echo -e "${YELLOW}[Config 4] MC Test${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PP.json"
    
    root -l -b -q "${SCRIPT}(0,1,1,1,5,7,0,0.2,0,100,\"${PARAM_FILE}\",1)" \
        2>&1 | tee logs/mc_test_$(date +%Y%m%d_%H%M%S).log
    
    echo -e "${GREEN}✓ MC test completed${NC}\n"
}

# Configuration 5: DCA Only (No mass fit)
run_dca_only() {
    echo -e "${YELLOW}[Config 5] DCA Only Test${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_dca_v2.json"
    
    root -l -b -q "${SCRIPT}(0,1,0,1,5,100,-2,2,0,100,\"${PARAM_FILE}\",0)" \
        2>&1 | tee logs/dca_only_$(date +%Y%m%d_%H%M%S).log
    
    echo -e "${GREEN}✓ DCA only test completed${NC}\n"
}

# Configuration 6: Custom (User editable)
run_custom() {
    echo -e "${YELLOW}[Config 6] Custom Configuration${NC}"
    
    # Edit these parameters as needed
    DO_REFIT=1
    DO_DCA=1
    PLOT_FIT=1
    USE_CUDA=1
    PT_MIN=5
    PT_MAX=100
    COS_MIN=-2
    COS_MAX=2
    CENT_MIN=0
    CENT_MAX=100
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PP.json"
    IS_MC=0
    
    echo -e "  Parameters:"
    echo -e "    pT: ${PT_MIN}-${PT_MAX} GeV"
    echo -e "    cos: ${COS_MIN} to ${COS_MAX}"
    echo -e "    MC: ${IS_MC}"
    
    root -l -b -q "${SCRIPT}(${DO_REFIT},${DO_DCA},${PLOT_FIT},${USE_CUDA},${PT_MIN},${PT_MAX},${COS_MIN},${COS_MAX},${CENT_MIN},${CENT_MAX},\"${PARAM_FILE}\",${IS_MC})" \
        2>&1 | tee logs/custom_$(date +%Y%m%d_%H%M%S).log
    
    echo -e "${GREEN}✓ Custom test completed${NC}\n"
}

# ========================================
# Main Menu
# ========================================

# Create logs directory
mkdir -p logs

if [ $# -eq 0 ]; then
    echo -e "${BLUE}Available configurations:${NC}"
    echo ""
    echo "  1. Quick Test        - Single bin test (fastest)"
    echo "  2. cosTheta Scan     - Full angular distribution"
    echo "  3. pT Scan           - Multiple pT bins"
    echo "  4. MC Test           - Monte Carlo validation"
    echo "  5. DCA Only          - DCA fitting only"
    echo "  6. Custom            - Edit script for custom config"
    echo "  all. Run All         - Run configs 1-5"
    echo ""
    echo -e "${YELLOW}Usage:${NC}"
    echo "  $0 <config_number>"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  $0 1              # Quick test"
    echo "  $0 2              # cosTheta scan"
    echo "  $0 all            # Run all configs"
    echo ""
    exit 0
fi

# Parse arguments
CONFIG=$1

case $CONFIG in
    1)
        run_quick_test
        ;;
    2)
        run_costheta_scan
        ;;
    3)
        run_pt_scan
        ;;
    4)
        run_mc_test
        ;;
    5)
        run_dca_only
        ;;
    6)
        run_custom
        ;;
    all)
        run_quick_test
        run_costheta_scan
        run_pt_scan
        run_mc_test
        run_dca_only
        ;;
    *)
        echo -e "${RED}Invalid configuration: $CONFIG${NC}"
        echo "Run without arguments to see available configurations"
        exit 1
        ;;
esac

echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Analysis Complete!${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo "Logs saved in: $(pwd)/logs/"
echo "Results saved in: ../../Results/PP/"
echo ""

