#!/bin/bash

# ========================================
# Emergency Rollback Script
# ========================================
# Use this if migration fails and you need to quickly restore
# ========================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${RED}========================================${NC}"
echo -e "${RED}🚨 EMERGENCY ROLLBACK${NC}"
echo -e "${RED}========================================${NC}\n"

BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"
PARENT_DIR="/home/jun502s/DstarAna"

cd "$PARENT_DIR"

# Find latest backup
echo -e "${YELLOW}Searching for backups...${NC}"
BACKUPS=($(ls -td DStarAnalysis_backup_* 2>/dev/null))

if [ ${#BACKUPS[@]} -eq 0 ]; then
    echo -e "${RED}❌ No backup found!${NC}"
    echo -e "${YELLOW}Looking for manual backups (.tar.gz)...${NC}"
    
    TAR_BACKUPS=($(ls -t DStarAnalysis_manual_backup_*.tar.gz 2>/dev/null))
    
    if [ ${#TAR_BACKUPS[@]} -eq 0 ]; then
        echo -e "${RED}❌ No manual backups found either!${NC}"
        echo -e "${RED}Cannot rollback. No backups available.${NC}\n"
        exit 1
    else
        echo -e "${GREEN}Found ${#TAR_BACKUPS[@]} manual backup(s)${NC}\n"
        
        echo "Available backups:"
        for i in "${!TAR_BACKUPS[@]}"; do
            SIZE=$(ls -lh "${TAR_BACKUPS[$i]}" | awk '{print $5}')
            DATE=$(ls -lh "${TAR_BACKUPS[$i]}" | awk '{print $6, $7, $8}')
            echo "  [$i] ${TAR_BACKUPS[$i]} ($SIZE, $DATE)"
        done
        
        read -p "Select backup to restore [0]: " CHOICE
        CHOICE=${CHOICE:-0}
        
        SELECTED_BACKUP="${TAR_BACKUPS[$CHOICE]}"
        
        if [ ! -f "$SELECTED_BACKUP" ]; then
            echo -e "${RED}Invalid selection!${NC}"
            exit 1
        fi
        
        echo -e "\n${YELLOW}Selected: $SELECTED_BACKUP${NC}"
        read -p "Proceed with restoration? (yes/no): " -r
        
        if [[ ! $REPLY == "yes" ]]; then
            echo -e "${RED}Rollback cancelled.${NC}"
            exit 1
        fi
        
        # Backup current failed state
        if [ -d "DStarAnalysis" ]; then
            echo -e "\n${YELLOW}💾 Backing up current (failed) state...${NC}"
            FAILED_DIR="DStarAnalysis_failed_$(date +%Y%m%d_%H%M%S)"
            mv DStarAnalysis "$FAILED_DIR"
            echo -e "${GREEN}✓ Current state saved to: $FAILED_DIR${NC}"
        fi
        
        # Extract tar backup
        echo -e "\n${YELLOW}♻️  Extracting backup...${NC}"
        tar -xzf "$SELECTED_BACKUP"
        
        echo -e "\n${GREEN}✅ Rollback complete!${NC}"
        echo -e "${GREEN}Restored from: $SELECTED_BACKUP${NC}\n"
        exit 0
    fi
fi

# Directory backups found
echo -e "${GREEN}Found ${#BACKUPS[@]} backup(s)${NC}\n"

echo "Available backups:"
for i in "${!BACKUPS[@]}"; do
    SIZE=$(du -sh "${BACKUPS[$i]}" | awk '{print $1}')
    DATE=$(ls -ld "${BACKUPS[$i]}" | awk '{print $6, $7, $8}')
    echo "  [$i] ${BACKUPS[$i]} ($SIZE, $DATE)"
done

read -p "Select backup to restore [0 = latest]: " CHOICE
CHOICE=${CHOICE:-0}

LATEST_BACKUP="${BACKUPS[$CHOICE]}"

if [ ! -d "$LATEST_BACKUP" ]; then
    echo -e "${RED}Invalid selection!${NC}"
    exit 1
fi

echo -e "\n${GREEN}📦 Selected backup: $LATEST_BACKUP${NC}"

# Verify backup integrity
echo -e "\n${YELLOW}Verifying backup integrity...${NC}"
CRITICAL_FILES=(
    "Macro/fit/testRun/DStarAnalysisV2forpp.cpp"
    "Macro/fit/testRun/DStarAnalysisV2forPbPb.cpp"
    "Macro/skim/DStarRDSMakerImproved.cpp"
)

MISSING=0
for file in "${CRITICAL_FILES[@]}"; do
    if [ ! -f "$LATEST_BACKUP/$file" ]; then
        echo -e "  ${RED}✗ Missing in backup: $file${NC}"
        ((MISSING++))
    fi
done

if [ $MISSING -gt 0 ]; then
    echo -e "${RED}⚠️  Warning: Backup appears incomplete!${NC}"
    read -p "Continue anyway? (yes/no): " -r
    if [[ ! $REPLY == "yes" ]]; then
        echo -e "${RED}Rollback cancelled.${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}✓ Backup integrity OK${NC}"
fi

# Confirm rollback
echo -e "\n${RED}⚠️  This will replace current DStarAnalysis directory!${NC}"
read -p "Proceed with rollback? (yes/no): " -r
echo

if [[ ! $REPLY == "yes" ]]; then
    echo -e "${RED}Rollback cancelled.${NC}"
    exit 1
fi

# Backup current (failed) state
if [ -d "DStarAnalysis" ]; then
    echo -e "${YELLOW}💾 Backing up current (failed) state...${NC}"
    FAILED_DIR="DStarAnalysis_failed_$(date +%Y%m%d_%H%M%S)"
    mv DStarAnalysis "$FAILED_DIR"
    echo -e "${GREEN}✓ Current state saved to: $FAILED_DIR${NC}"
fi

# Restore from backup
echo -e "\n${YELLOW}♻️  Restoring from backup...${NC}"
cp -r "$LATEST_BACKUP" DStarAnalysis

# Verify restoration
echo -e "\n${YELLOW}Verifying restoration...${NC}"
cd DStarAnalysis
VERIFIED=0
for file in "${CRITICAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        ((VERIFIED++))
    fi
done

if [ $VERIFIED -eq ${#CRITICAL_FILES[@]} ]; then
    echo -e "${GREEN}✓ All critical files restored${NC}"
else
    echo -e "${YELLOW}⚠️  Only $VERIFIED/${#CRITICAL_FILES[@]} critical files found${NC}"
fi

# Summary
echo -e "\n${BLUE}========================================${NC}"
echo -e "${GREEN}✅ ROLLBACK COMPLETE!${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo "Restored from: $LATEST_BACKUP"
echo "Failed state backed up to: $FAILED_DIR"
echo ""
echo "Next steps:"
echo "  1. Verify files: ls -la Macro/fit/testRun/"
echo "  2. Test compilation: cd Macro/fit/testRun && root -l"
echo "  3. If issues persist, check: $FAILED_DIR"
echo ""

