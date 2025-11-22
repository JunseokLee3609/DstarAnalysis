#!/bin/bash
# Wrapper: create massPion histogram for combined kinematic bin and MVA with cosThetaEP
# Usage: run_make_hist_bin.sh <ptMin> <ptMax> <yAbsMin> <yAbsMax> <centMin> <centMax> <mva> <dcaModeInt> <cosEPMin> <cosEPMax>
set -e
PTMIN=$1
PTMAX=$2
YMIN=$3
YMAX=$4
CENTMIN=$5
CENTMAX=$6
MVA=$7
DCAMODE=$8
COSEPMIN=${9:--1.0}   # Default to -1.0 if not provided
COSEPMAX=${10:-1.0}   # Default to 1.0 if not provided

# ROOT 6.24
source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

mkdir -p results/reduced_hist
# Mass window is fixed inside macro: 0.140 - 0.155
root -l -b -q "MakeHistKinematicBin.cpp($PTMIN, $PTMAX, $YMIN, $YMAX, $CENTMIN, $CENTMAX, $MVA, \"PbPb_Data_FlatPsiAngle_MVA0p9_14Nov\", $DCAMODE, $COSEPMIN, $COSEPMAX)"

echo "Done: pt[$PTMIN,$PTMAX] y[$YMIN,$YMAX] cent[$CENTMIN,$CENTMAX] mva=$MVA dcaMode=$DCAMODE cosEP[$COSEPMIN,$COSEPMAX]"