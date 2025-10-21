#!/bin/bash

# ========================================
# D* Analysis Environment Check
# ========================================
# Verifies that all required software and paths are correctly set up
# ========================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}D* Analysis Environment Check${NC}"
echo -e "${BLUE}========================================${NC}\n"

ERRORS=0
WARNINGS=0

# Check 1: ROOT availability
echo -e "${YELLOW}[1/8] Checking ROOT...${NC}"
if command -v root &> /dev/null; then
    ROOT_VERSION=$(root --version 2>&1 | head -1)
    echo -e "  ${GREEN}✓${NC} ROOT is available: $ROOT_VERSION"
    
    # Check if version is 6.24+
    if echo "$ROOT_VERSION" | grep -qE "(6\.24|6\.2[5-9]|6\.[3-9]|[7-9]\.)"; then
        echo -e "  ${GREEN}✓${NC} ROOT version is compatible"
    else
        echo -e "  ${YELLOW}⚠${NC} ROOT version might be too old (6.24+ recommended)"
        ((WARNINGS++))
    fi
else
    echo -e "  ${RED}✗${NC} ROOT not found!"
    echo -e "  ${YELLOW}Run: ROOT6.24${NC}"
    ((ERRORS++))
fi
echo ""

# Check 2: ROOT environment variables
echo -e "${YELLOW}[2/8] Checking ROOT environment...${NC}"
if [ -n "$ROOTSYS" ]; then
    echo -e "  ${GREEN}✓${NC} ROOTSYS: $ROOTSYS"
else
    echo -e "  ${YELLOW}⚠${NC} ROOTSYS not set (run ROOT6.24)"
    ((WARNINGS++))
fi

if [ -n "$LD_LIBRARY_PATH" ]; then
    if echo "$LD_LIBRARY_PATH" | grep -q root; then
        echo -e "  ${GREEN}✓${NC} ROOT libraries in LD_LIBRARY_PATH"
    else
        echo -e "  ${YELLOW}⚠${NC} ROOT not in LD_LIBRARY_PATH"
        ((WARNINGS++))
    fi
else
    echo -e "  ${YELLOW}⚠${NC} LD_LIBRARY_PATH not set"
    ((WARNINGS++))
fi
echo ""

# Check 3: RooFit availability
echo -e "${YELLOW}[3/8] Checking RooFit...${NC}"
if command -v root &> /dev/null; then
    ROOFIT_CHECK=$(root -l -b -q -e 'gSystem->Load("libRooFit"); cout << "OK" << endl;' 2>&1 | grep -o "OK")
    if [ "$ROOFIT_CHECK" == "OK" ]; then
        echo -e "  ${GREEN}✓${NC} RooFit library is accessible"
    else
        echo -e "  ${YELLOW}⚠${NC} RooFit might not be properly loaded"
        ((WARNINGS++))
    fi
else
    echo -e "  ${RED}✗${NC} Cannot check RooFit (ROOT not available)"
    ((ERRORS++))
fi
echo ""

# Check 4: Project directory
echo -e "${YELLOW}[4/8] Checking project directory...${NC}"
PROJECT_DIR="/home/jun502s/DstarAna/DStarAnalysis"
if [ -d "$PROJECT_DIR" ]; then
    echo -e "  ${GREEN}✓${NC} Project directory found: $PROJECT_DIR"
    
    # Check key subdirectories
    KEY_DIRS=("Analysis" "Preprocessing" "Corrections" "Tools" "Data")
    MISSING_DIRS=0
    for dir in "${KEY_DIRS[@]}"; do
        if [ ! -d "$PROJECT_DIR/$dir" ]; then
            echo -e "  ${YELLOW}⚠${NC} Missing directory: $dir"
            ((MISSING_DIRS++))
        fi
    done
    
    if [ $MISSING_DIRS -eq 0 ]; then
        echo -e "  ${GREEN}✓${NC} All key directories present"
    else
        echo -e "  ${YELLOW}⚠${NC} $MISSING_DIRS key directories missing"
        ((WARNINGS++))
    fi
else
    echo -e "  ${RED}✗${NC} Project directory not found: $PROJECT_DIR"
    ((ERRORS++))
fi
echo ""

# Check 5: Analysis scripts
echo -e "${YELLOW}[5/8] Checking analysis scripts...${NC}"
if [ -f "$PROJECT_DIR/Analysis/PP/DStarAnalysisPP.cpp" ]; then
    echo -e "  ${GREEN}✓${NC} PP analysis script found"
else
    echo -e "  ${YELLOW}⚠${NC} PP analysis script not found"
    ((WARNINGS++))
fi

if [ -f "$PROJECT_DIR/Analysis/PbPb/DStarAnalysisPbPb.cpp" ]; then
    echo -e "  ${GREEN}✓${NC} PbPb analysis script found"
else
    echo -e "  ${YELLOW}⚠${NC} PbPb analysis script not found"
    ((WARNINGS++))
fi
echo ""

# Check 6: Data directory
echo -e "${YELLOW}[6/8] Checking data directory...${NC}"
if [ -d "$PROJECT_DIR/Data" ]; then
    echo -e "  ${GREEN}✓${NC} Data directory exists"
    
    # Count ROOT files
    ROOT_FILES=$(find "$PROJECT_DIR/Data" -name "*.root" 2>/dev/null | wc -l)
    if [ $ROOT_FILES -gt 0 ]; then
        echo -e "  ${GREEN}✓${NC} Found $ROOT_FILES ROOT data files"
    else
        echo -e "  ${YELLOW}⚠${NC} No ROOT files found in Data/"
        ((WARNINGS++))
    fi
else
    echo -e "  ${YELLOW}⚠${NC} Data directory not found"
    ((WARNINGS++))
fi
echo ""

# Check 7: Compiler
echo -e "${YELLOW}[7/8] Checking C++ compiler...${NC}"
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -1)
    echo -e "  ${GREEN}✓${NC} g++ is available: $GCC_VERSION"
else
    echo -e "  ${YELLOW}⚠${NC} g++ not found (might not be needed for ROOT scripts)"
    ((WARNINGS++))
fi
echo ""

# Check 8: Disk space
echo -e "${YELLOW}[8/8] Checking disk space...${NC}"
if [ -d "$PROJECT_DIR" ]; then
    AVAILABLE=$(df -BG "$PROJECT_DIR" | tail -1 | awk '{print $4}' | sed 's/G//')
    USED=$(du -sg "$PROJECT_DIR" | awk '{print $1}')
    
    echo -e "  ${GREEN}✓${NC} Disk space available: ${AVAILABLE}GB"
    echo -e "  ${GREEN}✓${NC} Project size: ${USED}GB"
    
    if [ $AVAILABLE -lt 10 ]; then
        echo -e "  ${YELLOW}⚠${NC} Low disk space (< 10GB available)"
        ((WARNINGS++))
    fi
fi
echo ""

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Summary${NC}"
echo -e "${BLUE}========================================${NC}"

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}✓ All checks PASSED!${NC}"
    echo -e "${GREEN}Environment is properly configured.${NC}\n"
    
    echo -e "${BLUE}Ready to run analysis:${NC}"
    echo -e "  ${YELLOW}cd $PROJECT_DIR/Analysis/PP${NC}"
    echo -e "  ${YELLOW}root -l -b -q DStarAnalysisPP.cpp${NC}\n"
    exit 0
    
elif [ $ERRORS -eq 0 ]; then
    echo -e "${YELLOW}⚠ Checks passed with $WARNINGS warning(s)${NC}"
    echo -e "${YELLOW}Environment is functional but has minor issues.${NC}\n"
    
    if [ $WARNINGS -gt 0 ]; then
        echo -e "${YELLOW}Recommendations:${NC}"
        
        if [ -z "$ROOTSYS" ]; then
            echo -e "  1. Setup ROOT: ${YELLOW}ROOT6.24${NC}"
        fi
        
        if [ $ROOT_FILES -eq 0 ]; then
            echo -e "  2. Add data files to Data/ directory"
        fi
    fi
    echo ""
    exit 0
    
else
    echo -e "${RED}✗ Checks FAILED!${NC}"
    echo -e "${RED}Found $ERRORS error(s) and $WARNINGS warning(s)${NC}\n"
    
    echo -e "${YELLOW}Required actions:${NC}"
    
    if ! command -v root &> /dev/null; then
        echo -e "  1. ${RED}Setup ROOT:${NC}"
        echo -e "     ${YELLOW}ROOT6.24${NC}"
        echo -e "     or"
        echo -e "     ${YELLOW}source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.24.00/.../thisroot.sh${NC}"
    fi
    
    if [ ! -d "$PROJECT_DIR" ]; then
        echo -e "  2. ${RED}Navigate to correct directory:${NC}"
        echo -e "     ${YELLOW}cd $PROJECT_DIR${NC}"
    fi
    
    echo ""
    echo -e "For more help, see: ${YELLOW}ENVIRONMENT.md${NC}"
    echo ""
    exit 1
fi

