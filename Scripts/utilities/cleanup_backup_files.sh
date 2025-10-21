#!/bin/bash

# ========================================
# Backup File Cleanup Script
# ========================================
# Lists and optionally removes backup files
# ========================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Backup File Cleanup Utility${NC}"
echo -e "${BLUE}========================================${NC}\n"

cd "${BASE_DIR}"

# Find all backup files
echo -e "${YELLOW}Searching for backup files...${NC}\n"

BACKUP_FILES=$(find . -type f \( \
    -name "*_backup.*" -o \
    -name "*.backup" -o \
    -name "*_old.*" -o \
    -name "*.bak" -o \
    -name "*_temp.*" -o \
    -name "*_copy.*" -o \
    -name "* copy.*" \
\) ! -path "*/DStarAnalysis_backup_*/*" | sort)

if [ -z "$BACKUP_FILES" ]; then
    echo -e "${GREEN}No backup files found!${NC}"
    exit 0
fi

echo -e "${YELLOW}Found backup files:${NC}"
echo "$BACKUP_FILES" | nl
echo ""

# Count files
FILE_COUNT=$(echo "$BACKUP_FILES" | wc -l)
echo -e "${BLUE}Total: $FILE_COUNT files${NC}\n"

# Calculate total size
TOTAL_SIZE=$(echo "$BACKUP_FILES" | xargs du -ch 2>/dev/null | tail -1 | awk '{print $1}')
echo -e "${BLUE}Total size: $TOTAL_SIZE${NC}\n"

# Ask for confirmation
echo -e "${RED}WARNING: This will permanently delete all backup files!${NC}"
read -p "Do you want to delete these files? (yes/no): " -r
echo

if [[ $REPLY == "yes" ]]; then
    echo -e "${YELLOW}Deleting backup files...${NC}\n"
    
    echo "$BACKUP_FILES" | while read -r file; do
        if [ -f "$file" ]; then
            rm "$file"
            echo -e "  ${GREEN}✓${NC} Deleted: $file"
        fi
    done
    
    echo -e "\n${GREEN}Cleanup complete!${NC}"
    echo -e "${GREEN}Deleted $FILE_COUNT files ($TOTAL_SIZE)${NC}\n"
else
    echo -e "${YELLOW}Cleanup cancelled.${NC}\n"
    echo "To delete specific files manually:"
    echo "  rm path/to/file_backup.ext"
    echo ""
    echo "To delete all backup files:"
    echo "  find . -type f \\( -name \"*_backup.*\" -o -name \"*.backup\" -o -name \"*_old.*\" -o -name \"*.bak\" \\) -delete"
    echo ""
fi

