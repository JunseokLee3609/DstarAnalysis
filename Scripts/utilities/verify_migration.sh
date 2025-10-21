#!/bin/bash

# ========================================
# Migration Verification Script
# ========================================
# Verifies that migration was successful
# ========================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Migration Verification${NC}"
echo -e "${BLUE}========================================${NC}\n"

cd "${BASE_DIR}"

ERRORS=0

# Function to check if file exists
check_file() {
    local file="$1"
    local description="$2"
    
    if [ -f "$file" ]; then
        echo -e "  ${GREEN}✓${NC} $description"
        return 0
    else
        echo -e "  ${RED}✗${NC} $description - NOT FOUND"
        ((ERRORS++))
        return 1
    fi
}

# Function to check if directory exists
check_dir() {
    local dir="$1"
    local description="$2"
    
    if [ -d "$dir" ]; then
        echo -e "  ${GREEN}✓${NC} $description"
        return 0
    else
        echo -e "  ${RED}✗${NC} $description - NOT FOUND"
        ((ERRORS++))
        return 1
    fi
}

# Check directory structure
echo -e "${YELLOW}Checking directory structure...${NC}"
check_dir "Analysis/Common/Core" "Common Core directory"
check_dir "Analysis/Common/Config" "Common Config directory"
check_dir "Analysis/Common/Managers" "Common Managers directory"
check_dir "Analysis/Common/Utils" "Common Utils directory"
check_dir "Analysis/PP" "PP Analysis directory"
check_dir "Analysis/PbPb" "PbPb Analysis directory"
check_dir "Preprocessing/Common" "Preprocessing Common directory"
check_dir "Tools/Kinematics" "Tools Kinematics directory"
check_dir "Data/RDS" "Data RDS directory"
echo ""

# Check core analysis files
echo -e "${YELLOW}Checking core analysis files...${NC}"
check_file "Analysis/Common/Core/MassFitterV2.h" "MassFitterV2.h"
check_file "Analysis/Common/Core/DCAFitter.h" "DCAFitter.h"
check_file "Analysis/Common/Core/DataLoader.h" "DataLoader.h"
check_file "Analysis/Common/Core/PDFFactory.h" "PDFFactory.h"
echo ""

# Check config files
echo -e "${YELLOW}Checking config files...${NC}"
check_file "Analysis/Common/Config/DStarFitConfig.h" "DStarFitConfig.h"
check_file "Analysis/Common/Config/DStarFitOpt.h" "DStarFitOpt.h"
echo ""

# Check PP/PbPb analysis files
echo -e "${YELLOW}Checking PP/PbPb analysis files...${NC}"
check_file "Analysis/PP/DStarAnalysisPP.cpp" "PP Analysis file"
check_file "Analysis/PP/ConfigPP.h" "PP Config file"
check_file "Analysis/PbPb/DStarAnalysisPbPb.cpp" "PbPb Analysis file"
check_file "Analysis/PbPb/ConfigPbPb.h" "PbPb Config file"
echo ""

# Check preprocessing files
echo -e "${YELLOW}Checking preprocessing files...${NC}"
check_file "Preprocessing/Common/DStarRDSMakerImproved.cpp" "DStarRDSMakerImproved.cpp"
check_file "Preprocessing/Common/FlexibleFlattener.cpp" "FlexibleFlattener.cpp"
echo ""

# Check Tools
echo -e "${YELLOW}Checking Tools...${NC}"
check_file "Tools/Kinematics/Transformations.h" "Transformations.h"
echo ""

# Check documentation
echo -e "${YELLOW}Checking documentation...${NC}"
check_file "README.md" "README.md"
check_file "MIGRATION_GUIDE.md" "Migration Guide"
check_file ".gitignore" ".gitignore"
echo ""

# Summary
echo -e "${BLUE}========================================${NC}"
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ Migration verification PASSED!${NC}"
    echo -e "${GREEN}All expected files and directories are present.${NC}"
else
    echo -e "${RED}✗ Migration verification FAILED!${NC}"
    echo -e "${RED}Found $ERRORS missing files or directories.${NC}"
    echo -e "\n${YELLOW}Please review the errors above and rerun the migration script if needed.${NC}"
fi
echo -e "${BLUE}========================================${NC}\n"

exit $ERRORS

