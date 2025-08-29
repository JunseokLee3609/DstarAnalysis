#!/bin/bash
# runProcess.sh

# Source the ROOT environment if not already set
if [ -z "$ROOTSYS" ]; then
	cd /u/user/jun502s/dstarana/skim/CMSSW_13_2_11/src && eval `scramv1 runtime -sh` && cd -
fi
cd /u/user/jun502s/dstarana/skim/DstarAnalysis/Macro/skim

inputFile=$1
num=$2



# Echoing for debugging
echo "Processing input File: $inputFile and $num" 

# The -l flag suppresses the splash screen, -b for batch mode, and -q to quit when done.
#root -l -b -q "FlexibleFlattener.cpp(0,-1,$num,0,\"$inputFile\",\"dstar_Prompt_ppRef_MC_Aug14\")"
#root -l -b -q "FlexibleFlattener.cpp(0,-1,$num,1,\"$inputFile\",\"dstar_NonPrompt_ppRef_MC_Aug18\")"
#root -l -b -q "FlexibleFlattener.cpp(0,-1,$num,1,0,\"$inputFile\",\"dstar_PbPb_MC_Aug21\")"
root -l -b -q "FlexibleFlattener.cpp(0,-1,$num,0,0,\"$inputFile\",\"dstar_PbPb_MC_wEvtplane_mva0p99_Aug25\")"
