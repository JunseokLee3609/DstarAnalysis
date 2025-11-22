#!/bin/bash
# Quick test of cosThetaEP binning implementation
# Tests a single bin to verify everything works

set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

echo "=== Testing cosThetaEP Binning ==="
echo ""
echo "Test parameters:"
echo "  pT: [30, 50] GeV/c (merged bin)"
echo "  |y|: [0.0, 0.3]"
echo "  Centrality: [0, 10]%"
echo "  MVA: 0.990"
echo "  DCA mode: 0 (inclusive)"
echo "  cosThetaEP: [-1.0, -0.8] (first bin)"
echo ""

source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh

echo "Running MakeHistKinematicBin (histogram only, no fit)..."
root -l -b -q 'MakeHistKinematicBin.cpp(30, 50, 0.0, 0.3, 0, 10, 0.990, "PbPb_Data_Oct29_mva0p9", 0, -1.0, -0.8, 0)' 2>&1 | tail -30

echo ""
echo "=== Test Complete ==="
echo "Check output:"
ls -lh results/reduced_hist/*pt30p0_50p0*cosEP-1p00_-0p80* 2>/dev/null | tail -3

echo ""
echo "If you see histogram files above, the implementation is working!"
echo "To submit all 810 jobs, run: ./submit_condor_jobs.sh"
