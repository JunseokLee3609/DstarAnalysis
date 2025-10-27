#!/bin/bash

# ========================================
# PbPb Analysis Configuration Runner
# ========================================
# Quick test configurations for PbPb analysis
# ========================================

set -e  # Exit on error

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
SCRIPT="DStarAnalysisPbPb.cpp"
PARAM_DIR="Parameters"

# Check if ROOT is setup
if ! command -v root &> /dev/null; then
    echo -e "${RED}ERROR: ROOT not found!${NC}"
    echo -e "${YELLOW}Please run: ROOT6.24${NC}"
    exit 1
fi

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}PbPb Analysis Test Runner${NC}"
echo -e "${BLUE}========================================${NC}\n"

# ========================================
# Test Configurations
# ========================================

# Configuration 1: Quick Test (Single bin) - Data only
run_quick_test() {
    echo -e "${YELLOW}[Config 1] Quick Test - Single bin (Data only)${NC}"
    
    # Arguments same as PP but centrality is used
    root -l -b -q "${SCRIPT}(1,0,1,1,7,10,0,0.2,0,10,\"${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_v1.json\",0)" \
        2>&1 | tee logs/quick_test_$(date +%Y%m%d_%H%M%S).log
    
    echo -e "${GREEN}✓ Quick test completed${NC}\n"
}

# Configuration 1b: Quick Test (Single bin) - MC only
run_quick_test_mc() {
    echo -e "${YELLOW}[Config 1b] Quick Test - Single bin (MC only)${NC}"
    
    # Arguments same as PP but centrality is used
    root -l -b -q "${SCRIPT}(1,0,1,1,5,7,0,0.2,0,10,\"${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json\",1)" \
        2>&1 | tee logs/quick_test_mc_$(date +%Y%m%d_%H%M%S).log
    
    echo -e "${GREEN}✓ Quick test MC completed${NC}\n"
}

# Configuration 2: Centrality Scan
run_centrality_scan() {
    echo -e "${YELLOW}[Config 2] Centrality Scan${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json"
    PT_MIN=5
    PT_MAX=10
    
    # Centrality bins
    CENT_BINS=(
        "0  10"
        "10 30"
        "30 50"
        "50 90"
    )
    
    for bin in "${CENT_BINS[@]}"; do
        CENT_MIN=$(echo $bin | awk '{print $1}')
        CENT_MAX=$(echo $bin | awk '{print $2}')
        
        echo -e "  Running centrality: ${CENT_MIN}-${CENT_MAX}%"
        
        root -l -b -q "${SCRIPT}(1,1,1,1,${PT_MIN},${PT_MAX},-2,2,${CENT_MIN},${CENT_MAX},\"${PARAM_FILE}\",0)" \
            2>&1 | tee logs/cent_scan_${CENT_MIN}_${CENT_MAX}_$(date +%Y%m%d_%H%M%S).log &
        
        sleep 2
    done
    
    wait
    echo -e "${GREEN}✓ Centrality scan completed${NC}\n"
}

# Configuration 3: cosTheta Scan (Central collisions)
run_costheta_scan() {
    echo -e "${YELLOW}[Config 3] cosTheta Scan (0-10% centrality)${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json"
    PT_MIN=5
    PT_MAX=7
    CENT_MIN=0
    CENT_MAX=10
    
    # cosTheta bins
    COS_BINS=(
        "-1.0 -0.8"
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
        
        root -l -b -q "${SCRIPT}(1,1,1,1,${PT_MIN},${PT_MAX},${COS_MIN},${COS_MAX},${CENT_MIN},${CENT_MAX},\"${PARAM_FILE}\",0,0)" \
            2>&1 | tee logs/costheta_scan_${COS_MIN}_${COS_MAX}_$(date +%Y%m%d_%H%M%S).log &
        
        sleep 2
    done
    
    wait
    echo -e "${GREEN}✓ cosTheta scan completed${NC}\n"
}

# Configuration 4: pT Scan (Central collisions)
run_pt_scan() {
    echo -e "${YELLOW}[Config 4] pT Scan (0-10% centrality)${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json"
    CENT_MIN=0
    CENT_MAX=10
    
    # pT bins
    PT_BINS=(
        "5  7"
        "7  10"
        "10 15"
        "15 20"
    )
    
    for bin in "${PT_BINS[@]}"; do
        PT_MIN=$(echo $bin | awk '{print $1}')
        PT_MAX=$(echo $bin | awk '{print $2}')
        
        echo -e "  Running pT: ${PT_MIN} to ${PT_MAX} GeV"
        
        root -l -b -q "${SCRIPT}(1,1,1,1,${PT_MIN},${PT_MAX},-2,2,${CENT_MIN},${CENT_MAX},\"${PARAM_FILE}\",0)" \
            2>&1 | tee logs/pt_scan_${PT_MIN}_${PT_MAX}_$(date +%Y%m%d_%H%M%S).log
    done
    
    echo -e "${GREEN}✓ pT scan completed${NC}\n"
}

# Configuration 5: MC Test
run_mc_test() {
    echo -e "${YELLOW}[Config 5] MC Test${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json"
    
    root -l -b -q "${SCRIPT}(0,1,1,1,5,7,0,0.2,0,10,\"${PARAM_FILE}\",0)" \
        2>&1 | tee logs/mc_test_$(date +%Y%m%d_%H%M%S).log
    
    echo -e "${GREEN}✓ MC test completed${NC}\n"
}

# Configuration 6: Full 2D Scan (Centrality vs pT)
run_2d_scan() {
    echo -e "${YELLOW}[Config 6] 2D Scan (Centrality vs pT)${NC}"
    
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json"
    
    # Centrality bins
    CENT_BINS=("0 10" "10 30" "30 50")
    PT_BINS=("5 7" "7 10" "10 15")
    
    for cent_bin in "${CENT_BINS[@]}"; do
        CENT_MIN=$(echo $cent_bin | awk '{print $1}')
        CENT_MAX=$(echo $cent_bin | awk '{print $2}')
        
        for pt_bin in "${PT_BINS[@]}"; do
            PT_MIN=$(echo $pt_bin | awk '{print $1}')
            PT_MAX=$(echo $pt_bin | awk '{print $2}')
            
            echo -e "  Running Cent: ${CENT_MIN}-${CENT_MAX}%, pT: ${PT_MIN}-${PT_MAX} GeV"
            
            root -l -b -q "${SCRIPT}(1,1,1,1,${PT_MIN},${PT_MAX},-2,2,${CENT_MIN},${CENT_MAX},\"${PARAM_FILE}\",0)" \
                2>&1 | tee logs/2d_scan_cent${CENT_MIN}_${CENT_MAX}_pt${PT_MIN}_${PT_MAX}_$(date +%Y%m%d_%H%M%S).log &
            
            sleep 2
        done
    done
    
    wait
    echo -e "${GREEN}✓ 2D scan completed${NC}\n"
}

# Configuration 7: Custom (User editable)
run_custom() {
    echo -e "${YELLOW}[Config 7] Custom Configuration${NC}"
    
    # Edit these parameters as needed
    DO_REFIT=1
    DO_DCA=1
    PLOT_FIT=1
    USE_CUDA=1
    PT_MIN=5
    PT_MAX=10
    COS_MIN=-2
    COS_MAX=2
    CENT_MIN=0    # PbPb specific
    CENT_MAX=10   # PbPb specific
    PARAM_FILE="${PARAM_DIR}/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json"
    IS_MC=0
    
    echo -e "  Parameters:"
    echo -e "    pT: ${PT_MIN}-${PT_MAX} GeV"
    echo -e "    cos: ${COS_MIN} to ${COS_MAX}"
    echo -e "    Centrality: ${CENT_MIN}-${CENT_MAX}%"
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
    echo "  1. Quick Test           - Single bin test (fastest)"
    echo "  2. Centrality Scan      - Multiple centrality bins"
    echo "  3. cosTheta Scan        - Full angular distribution (0-10%)"
    echo "  4. pT Scan              - Multiple pT bins (0-10%)"
    echo "  5. MC Test              - Monte Carlo validation"
    echo "  6. Full 2D Scan         - Centrality × pT (comprehensive)"
    echo "  7. Custom               - Edit script for custom config"
    echo "  all. Run All            - Run configs 1-5"
    echo ""
    echo -e "${YELLOW}Usage:${NC}"
    echo "  $0 <config_number>"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  $0 1              # Quick test"
    echo "  $0 2              # Centrality scan"
    echo "  $0 6              # Full 2D scan"
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
    1b)
        run_quick_test_mc
        ;;
    2)
        run_centrality_scan
        ;;
    3)
        run_costheta_scan
        ;;
    4)
        run_pt_scan
        ;;
    5)
        run_mc_test
        ;;
    6)
        run_2d_scan
        ;;
    7)
        run_custom
        ;;
    all)
        run_quick_test
        run_centrality_scan
        run_pt_scan
        run_mc_test
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
echo "Results saved in: ../../Results/PbPb/"
echo ""

