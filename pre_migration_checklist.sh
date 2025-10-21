#!/bin/bash

# ========================================
# Pre-Migration Safety Checklist
# ========================================
# Run this before migration to ensure safety
# ========================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Pre-Migration Safety Checklist${NC}"
echo -e "${BLUE}========================================${NC}\n"

WARNINGS=0
ERRORS=0

# Check 1: Disk space
echo -e "${YELLOW}[1/8] Checking disk space...${NC}"
AVAILABLE=$(df -BG "$BASE_DIR" | tail -1 | awk '{print $4}' | sed 's/G//')
CURRENT_SIZE=$(du -sg "$BASE_DIR" | awk '{print $1}')
REQUIRED=$((CURRENT_SIZE + 5))

if [ "$AVAILABLE" -lt "$REQUIRED" ]; then
    echo -e "  ${RED}✗ Insufficient disk space${NC}"
    echo -e "  Available: ${AVAILABLE}GB, Required: ${REQUIRED}GB"
    ((ERRORS++))
else
    echo -e "  ${GREEN}✓ Sufficient disk space${NC} (Available: ${AVAILABLE}GB)"
fi
echo ""

# Check 2: Git status
echo -e "${YELLOW}[2/8] Checking Git status...${NC}"
cd "$BASE_DIR"
if git rev-parse --git-dir > /dev/null 2>&1; then
    UNCOMMITTED=$(git status --porcelain | wc -l)
    if [ "$UNCOMMITTED" -gt 0 ]; then
        echo -e "  ${YELLOW}⚠ Warning: $UNCOMMITTED uncommitted changes${NC}"
        echo -e "  Recommendation: git commit or git stash before migration"
        ((WARNINGS++))
    else
        echo -e "  ${GREEN}✓ No uncommitted changes${NC}"
    fi
else
    echo -e "  ${YELLOW}⚠ Not a git repository${NC}"
    echo -e "  Recommendation: Initialize git for version control"
    ((WARNINGS++))
fi
echo ""

# Check 3: Important files exist
echo -e "${YELLOW}[3/8] Checking important files...${NC}"
CRITICAL_FILES=(
    "Macro/fit/testRun/DStarAnalysisV2forpp.cpp"
    "Macro/fit/testRun/DStarAnalysisV2forPbPb.cpp"
    "Macro/skim/DStarRDSMakerImproved.cpp"
    "Macro/fit/MassFitterV2.h"
    "Macro/fit/DStarFitConfig.h"
)

MISSING=0
for file in "${CRITICAL_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "  ${RED}✗ Missing: $file${NC}"
        ((MISSING++))
    fi
done

if [ $MISSING -eq 0 ]; then
    echo -e "  ${GREEN}✓ All critical files present${NC}"
else
    echo -e "  ${RED}✗ $MISSING critical files missing${NC}"
    ((ERRORS++))
fi
echo ""

# Check 4: Backup directory doesn't exist
echo -e "${YELLOW}[4/8] Checking for existing backups...${NC}"
EXISTING_BACKUPS=$(ls -d ../DStarAnalysis_backup_* 2>/dev/null | wc -l)
if [ "$EXISTING_BACKUPS" -gt 0 ]; then
    echo -e "  ${GREEN}✓ Found $EXISTING_BACKUPS existing backup(s)${NC}"
    ls -lhd ../DStarAnalysis_backup_* | tail -3
else
    echo -e "  ${YELLOW}⚠ No existing backups found${NC}"
    echo -e "  A new backup will be created during migration"
fi
echo ""

# Check 5: Write permissions
echo -e "${YELLOW}[5/8] Checking write permissions...${NC}"
if [ -w "$BASE_DIR" ]; then
    echo -e "  ${GREEN}✓ Write permission OK${NC}"
else
    echo -e "  ${RED}✗ No write permission${NC}"
    ((ERRORS++))
fi
echo ""

# Check 6: Running processes
echo -e "${YELLOW}[6/8] Checking for running ROOT processes...${NC}"
ROOT_PROCS=$(ps aux | grep -i root | grep -v grep | wc -l)
if [ "$ROOT_PROCS" -gt 0 ]; then
    echo -e "  ${YELLOW}⚠ Warning: $ROOT_PROCS ROOT process(es) running${NC}"
    echo -e "  Recommendation: Close ROOT before migration"
    ps aux | grep -i root | grep -v grep | head -5
    ((WARNINGS++))
else
    echo -e "  ${GREEN}✓ No ROOT processes running${NC}"
fi
echo ""

# Check 7: Data files
echo -e "${YELLOW}[7/8] Checking data files...${NC}"
DATA_SIZE=$(du -sh Data/ 2>/dev/null | awk '{print $1}')
if [ -n "$DATA_SIZE" ]; then
    echo -e "  ${GREEN}✓ Data directory found${NC} (Size: $DATA_SIZE)"
    echo -e "  Note: Data files will be linked, not copied"
else
    echo -e "  ${YELLOW}⚠ No Data directory found${NC}"
    ((WARNINGS++))
fi
echo ""

# Check 8: Shell script permissions
echo -e "${YELLOW}[8/8] Checking script permissions...${NC}"
if [ -f "migrate_to_new_structure.sh" ]; then
    if [ -x "migrate_to_new_structure.sh" ]; then
        echo -e "  ${GREEN}✓ Migration script is executable${NC}"
    else
        echo -e "  ${YELLOW}⚠ Migration script not executable${NC}"
        echo -e "  Run: chmod +x migrate_to_new_structure.sh"
        ((WARNINGS++))
    fi
else
    echo -e "  ${RED}✗ Migration script not found${NC}"
    ((ERRORS++))
fi
echo ""

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Summary${NC}"
echo -e "${BLUE}========================================${NC}"

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}✓ ALL CHECKS PASSED!${NC}"
    echo -e "${GREEN}Safe to proceed with migration.${NC}\n"
    
    echo -e "${BLUE}Recommended next steps:${NC}"
    echo -e "1. Create manual backup (optional but recommended):"
    echo -e "   ${YELLOW}cd /home/jun502s/DstarAna${NC}"
    echo -e "   ${YELLOW}tar -czf DStarAnalysis_manual_backup_\$(date +%Y%m%d_%H%M%S).tar.gz DStarAnalysis/${NC}\n"
    
    echo -e "2. Run migration:"
    echo -e "   ${YELLOW}cd DStarAnalysis${NC}"
    echo -e "   ${YELLOW}./migrate_to_new_structure.sh${NC}\n"
    
    exit 0
elif [ $ERRORS -eq 0 ]; then
    echo -e "${YELLOW}⚠ Checks passed with $WARNINGS warning(s)${NC}"
    echo -e "${YELLOW}Review warnings above before proceeding.${NC}\n"
    
    read -p "Do you want to continue anyway? (yes/no): " -r
    if [[ $REPLY == "yes" ]]; then
        echo -e "${YELLOW}Proceeding with migration...${NC}"
        exit 0
    else
        echo -e "${RED}Migration cancelled.${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ CHECKS FAILED!${NC}"
    echo -e "${RED}Found $ERRORS error(s) and $WARNINGS warning(s)${NC}"
    echo -e "${RED}Please fix errors before proceeding.${NC}\n"
    exit 1
fi

