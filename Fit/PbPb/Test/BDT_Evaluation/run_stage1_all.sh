#!/bin/bash

source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh

cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Test/BDT_Evaluation

echo "=========================================="
echo "Running Stage 1 for all modes"
echo "=========================================="

echo ""
echo "1. Inclusive mode..."
root -l -b -q 'CorrectYieldsWithMVA_Stage1_Independent.cpp(0, "results/data_fits", "incl")'

echo ""
echo "2. Prompt mode..."
root -l -b -q 'CorrectYieldsWithMVA_Stage1_Independent.cpp(0, "results/data_fits", "prompt")'

echo ""
echo "3. Nonprompt mode..."
root -l -b -q 'CorrectYieldsWithMVA_Stage1_Independent.cpp(0, "results/data_fits", "nonprompt")'

echo ""
echo "=========================================="
echo "All plots generated!"
echo "Check results/aggregated/ directory"
echo "=========================================="
