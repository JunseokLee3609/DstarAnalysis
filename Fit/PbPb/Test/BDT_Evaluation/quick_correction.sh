#!/bin/bash

# ================================================
# Quick Start: Modular Yield Correction
# ================================================
# 이 스크립트는 새로운 모듈화 워크플로우의 기본 사용법을 보여줍니다.

cd "$(dirname "$0")"

echo "================================================"
echo "Modular Yield Correction Quick Start"
echo "================================================"
echo ""

# 단계별 메뉴
if [ $# -eq 0 ]; then
    echo "Available Commands:"
    echo ""
    echo "  ./quick_correction.sh stage1        # Build reweighting"
    echo "  ./quick_correction.sh stage2        # Build efficiency"
    echo "  ./quick_correction.sh stage3        # Apply corrections"
    echo "  ./quick_correction.sh all           # Run all stages"
    echo "  ./quick_correction.sh test          # Test with sample data"
    echo "  ./quick_correction.sh help          # Show detailed help"
    echo ""
    exit 0
fi

case "$1" in
    stage1)
        echo "[Stage 1] Building Reweighting Function..."
        echo "  - Extracts MC gen-pT distribution (matchGEN==1)"
        echo "  - Compares with data fit yields at baseline MVA"
        echo "  - Creates weight function w(x) = N_data / N_mc"
        echo ""
        echo "Running..."
        bash runCorrection_Modular.sh results/data_fits 0 1 1
        echo ""
        echo "Output: results/aggregated/reweight_pT.png"
        ;;
        
    stage2)
        echo "[Stage 2] Building Efficiency Map..."
        echo "  - Computes reconstruction efficiency per MVA threshold"
        echo "  - Auto-runs Stage 1 if needed"
        echo ""
        echo "Running..."
        bash runCorrection_Modular.sh results/data_fits 0 2 1
        echo ""
        echo "Output: results/aggregated/efficiency_pT_*.png"
        ;;
        
    stage3)
        echo "[Stage 3] Applying Corrections & Plotting..."
        echo "  - Applies weight to data yields"
        echo "  - Generates corrected vs raw comparison plots"
        echo "  - Auto-runs Stage 1 if needed"
        echo ""
        echo "Running..."
        bash runCorrection_Modular.sh results/data_fits 0 3 1
        echo ""
        echo "Output:"
        echo "  - results/aggregated/corrected_yield_vs_pT_raw.png"
        echo "  - results/aggregated/corrected_yield_vs_pT_norm.png"
        echo "  - results/aggregated/corrected_yield_ratio_vs_pT_raw.png"
        ;;
        
    all)
        echo "[Workflow] Running All Stages..."
        echo ""
        bash runCorrection_Modular.sh results/data_fits 0 all 1
        echo ""
        echo "All stages completed!"
        echo "Output directory: results/aggregated/"
        ;;
        
    test)
        echo "[Test] Quick validation with 1 variable..."
        echo ""
        echo "Checking inputs..."
        
        # Check if FitSingleBin output exists
        if [ ! -d "results/data_fits" ]; then
            echo "ERROR: No fit results found!"
            echo ""
            echo "Please run FitSingleBin.cpp first:"
            echo "  root -l -b -q 'FitSingleBin.cpp(0, 5, 7, 1, 1)'"
            exit 1
        fi
        
        FILE_COUNT=$(find results/data_fits -name "fitresult_*.root" 2>/dev/null | wc -l)
        if [ $FILE_COUNT -eq 0 ]; then
            echo "ERROR: No fit result files in results/data_fits/"
            exit 1
        fi
        
        echo "Found $FILE_COUNT fit result files ✓"
        echo ""
        echo "Running complete workflow..."
        bash runCorrection_Modular.sh results/data_fits 0 all 1
        echo ""
        echo "Test completed successfully! ✓"
        echo "Check results in: results/aggregated/"
        ;;
        
    help)
        cat << 'EOF'

=== Modular Yield Correction Workflow ===

Three independent stages:

Stage 1: Reweighting
  - Uses MC dataset (matchGEN==1)
  - Compares MC gen distribution with data fit yields
  - Produces weight function: w(x) = N_data(x) / N_mc,gen(x)
  - Clips weights to [0.2, 5.0] for stability
  
  Command: ./quick_correction.sh stage1
  Output: results/aggregated/reweight_pT.png/root

Stage 2: Efficiency Map
  - Builds efficiency ε(x, MVA) for each MVA threshold
  - Uses reweighting from Stage 1 (auto-runs if missing)
  - Computes: ε(x) = N_reco(x) / N_gen(x)
  
  Command: ./quick_correction.sh stage2
  Output: results/aggregated/efficiency_pT_*.png/root

Stage 3: Corrections & Plotting
  - Applies weight correction to data yields
  - Creates comparison plots: corrected vs raw
  - Shows normalized and raw distributions
  - Uses Stage 1 (auto-runs if missing)
  
  Command: ./quick_correction.sh stage3
  Output: results/aggregated/corrected_yield_vs_pT_*.png

Full Workflow:
  ./quick_correction.sh all
  - Automatically runs Stage 1 → Stage 2 → Stage 3
  - Skips stages that already have outputs
  - Perfect for first-time run

Test Mode:
  ./quick_correction.sh test
  - Validates all inputs
  - Runs complete workflow
  - Shows success/error messages

Variable Types (advanced):
  0 = pT (transverse momentum)
  1 = rapidity
  2 = centrality

Advanced Usage:
  bash runCorrection_Modular.sh <dir> <vartype> <stage> <norm>
  Example: bash runCorrection_Modular.sh results/data_fits 2 all 1
  (runs centrality analysis with normalized plots)

Dependency Resolution:
  - Stage 2 automatically runs Stage 1 if reweight_*.root missing
  - Stage 3 automatically runs Stage 1 if reweight_*.root missing
  - No need to manually track dependencies!

Output Structure:
  results/aggregated/
  ├── reweight_pT.root
  ├── reweight_pT.png
  ├── efficiency_pT_mva*.root
  ├── efficiency_pT_mva*.png
  ├── corrected_yield_vs_pT_raw.png
  ├── corrected_yield_vs_pT_norm.png
  └── corrected_yield_ratio_vs_pT_raw.png

Requirements:
  - FitSingleBin.cpp must be run first (generates fit results)
  - ROOT6.24 environment
  - MC and Data fit results in results/data_fits/

Troubleshooting:
  If you see "INPUT MISSING" messages:
    → The script automatically runs previous stages
    → This is normal and expected!
    → Check results in results/aggregated/ afterward

  If you see "No fit result files":
    → Run FitSingleBin.cpp first
    → Then run this workflow again

EOF
        ;;
        
    *)
        echo "Unknown command: $1"
        echo "Use './quick_correction.sh help' for details"
        exit 1
        ;;
esac

echo ""
echo "================================================"
echo "Done!"
echo "================================================"
