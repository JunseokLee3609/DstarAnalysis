#!/bin/bash
# runProcess.sh

# Source the ROOT environment if not already set
if [ -z "$ROOTSYS" ]; then
    source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
fi
cd /home/jun502s/DstarAna/DStarAnalysis/Macro/skim/

inputFile=$1
num=$2



# Echoing for debugging
echo "Processing input File: $inputFile and $num" 

# The -l flag suppresses the splash screen, -b for batch mode, and -q to quit when done.
root -l -b -q "FlexibleFlattener.cpp(0,-1,$num,1,\"$inputFile\",\"dstar_Prompt_ppRef_MC_Jul28\")"
