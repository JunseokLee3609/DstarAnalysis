#!/bin/bash
echo "Testing runConfig.sh script structure..."
echo ""
echo "ROOT check:"
./runConfig.sh 2>&1 | head -5
echo ""
echo "Script files:"
ls -lh runConfig*.sh README_runConfig.md
