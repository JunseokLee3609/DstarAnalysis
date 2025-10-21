#!/bin/bash

# ========================================
# Include Path Update Script
# ========================================
# Automatically updates #include paths to match new directory structure
# ========================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Include Path Update Script${NC}"
echo -e "${BLUE}========================================${NC}\n"

cd "${BASE_DIR}"

# Function to update includes in a file
update_file_includes() {
    local file="$1"
    local backup="${file}.include_backup"
    
    # Create backup
    cp "$file" "$backup"
    
    # Update common includes
    sed -i 's|#include "../MassFitterV2.h"|#include "../../Common/Core/MassFitterV2.h"|g' "$file"
    sed -i 's|#include "../DCAFitter.h"|#include "../../Common/Core/DCAFitter.h"|g' "$file"
    sed -i 's|#include "../DataLoader.h"|#include "../../Common/Core/DataLoader.h"|g' "$file"
    sed -i 's|#include "../PDFFactory.h"|#include "../../Common/Core/PDFFactory.h"|g' "$file"
    
    sed -i 's|#include "../DStarFitConfig.h"|#include "../../Common/Config/DStarFitConfig.h"|g' "$file"
    sed -i 's|#include "../DStarFitOpt.h"|#include "../../Common/Config/DStarFitOpt.h"|g' "$file"
    sed -i 's|#include "../FitCommonConfig.h"|#include "../../Common/Config/FitCommonConfig.h"|g' "$file"
    
    sed -i 's|#include "../Helper.h"|#include "../../Common/Utils/Helper.h"|g' "$file"
    sed -i 's|#include "../JSONParameterUtils.h"|#include "../../Common/Utils/JSONParameterUtils.h"|g' "$file"
    sed -i 's|#include "../SimpleParameterLoader.h"|#include "../../Common/Utils/SimpleParameterLoader.h"|g' "$file"
    sed -i 's|#include "../ParameterDebugUtils.h"|#include "../../Common/Utils/ParameterDebugUtils.h"|g' "$file"
    
    sed -i 's|#include "../EnhancedPlotManager.h"|#include "../../Common/Managers/EnhancedPlotManager.h"|g' "$file"
    sed -i 's|#include "../ErrorHandler.h"|#include "../../Common/Managers/ErrorHandler.h"|g' "$file"
    sed -i 's|#include "../ParameterManager.h"|#include "../../Common/Managers/ParameterManager.h"|g' "$file"
    
    # Update Tools includes
    sed -i 's|#include "../../Tools/ConfigManagerPP.h"|#include "../ConfigPP.h"|g' "$file"
    sed -i 's|#include "../../Tools/ConfigManager.h"|#include "../ConfigPbPb.h"|g' "$file"
    sed -i 's|#include "../../Tools/Transformations.h"|#include "../../../Tools/Kinematics/Transformations.h"|g' "$file"
    
    # Check if file was actually modified
    if ! diff -q "$file" "$backup" > /dev/null 2>&1; then
        echo -e "  ${GREEN}✓${NC} Updated: $file"
        return 0
    else
        # No changes, remove backup
        rm "$backup"
        return 1
    fi
}

# Update PP analysis files
echo -e "${YELLOW}Updating PP analysis files...${NC}"
if [ -f "Analysis/PP/DStarAnalysisPP.cpp" ]; then
    update_file_includes "Analysis/PP/DStarAnalysisPP.cpp"
fi

# Update PbPb analysis files
echo -e "\n${YELLOW}Updating PbPb analysis files...${NC}"
if [ -f "Analysis/PbPb/DStarAnalysisPbPb.cpp" ]; then
    update_file_includes "Analysis/PbPb/DStarAnalysisPbPb.cpp"
fi

# Update preprocessing files
echo -e "\n${YELLOW}Updating preprocessing files...${NC}"
for file in Preprocessing/Common/*.cpp; do
    if [ -f "$file" ]; then
        update_file_includes "$file" || true
    fi
done

# Summary
echo -e "\n${BLUE}========================================${NC}"
echo -e "${GREEN}Include path update complete!${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo -e "${YELLOW}Backup files created with .include_backup extension${NC}"
echo -e "To remove backups: find . -name '*.include_backup' -delete\n"

