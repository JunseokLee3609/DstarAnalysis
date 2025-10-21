#!/bin/bash
# runProcess.sh

# Source the ROOT environment if not already set
if [ -z "$ROOTSYS" ]; then
    source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
fi
cd /home/jun502s/DstarAna/DStarAnalysis/Macro/skim

START=$1
END=$2
JOB_IDX=$3

# Echoing for debugging
echo "Processing range: $START to $END for job index $JOB_IDX"

# The -l flag suppresses the splash screen, -b for batch mode, and -q to quit when done.
root -l -b -q "FlatMix.cpp($START, $END,$JOBIDX)"
