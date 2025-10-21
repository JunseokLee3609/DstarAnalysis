#!/bin/bash
# List all backup files that can be safely removed

echo "Backup files in the repository:"
find . -type f \( -name "*_backup.*" -o -name "*.backup" -o -name "*_old.*" -o -name "*.bak" \) | sort

echo ""
echo "To remove all backup files, run:"
echo "  find . -type f \( -name \"*_backup.*\" -o -name \"*.backup\" -o -name \"*_old.*\" -o -name \"*.bak\" \) -delete"
