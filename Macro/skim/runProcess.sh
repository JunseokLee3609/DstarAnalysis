#!/bin/bash
# runProcess.sh

# Source the ROOT environment if not already set
if [ -z "$ROOTSYS" ]; then
    source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
fi
cd /home/jun502s/DstarAna/DStarAnalysis/Macro/skim

# $1 is the input file; $2 is an optional flag for isMC (default true)
#INPUTFILE=$1
INPUTFILE="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/MCNPD0KPi_DpT8_0220/d0ana_tree_3.root"
NUM=${2:-1}

# The -l flag suppresses the splash screen, -b for batch mode, and -q to quit when done.
root -l -b -q "skimMC_ForCondor.cpp(\"$INPUTFILE\", $NUM)"
