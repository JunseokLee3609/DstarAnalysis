#!/bin/bash
# runProcess.sh

# Source the ROOT environment if not already set
if [ -z "$ROOTSYS" ]; then
    source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
fi
cd /home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/

pTMin=$1
pTMax=$2


# Echoing for debugging
echo "Processing range: $pTMin to $pTMax "

# The -l flag suppresses the splash screen, -b for batch mode, and -q to quit when done.
root -l -b -q "MCMacroDstarOO_condor.cpp(true,true,true,$pTMin,$pTMax)"
