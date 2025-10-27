#!/bin/bash
# Submit Condor jobs for FitSingleBin.cpp analysis
# Reads kinematic bins from kinematic_bins.txt

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BINS_FILE="kinematic_bins.txt"

if [ ! -f "$BINS_FILE" ]; then
    echo "❌ Error: $BINS_FILE not found!"
    exit 1
fi

# Parse kinematic bins from file
echo "📖 Reading kinematic bins from $BINS_FILE..."
BINS_ARRAY=()
while IFS=' ' read -r kinvar varname varmin varmax; do
    # Skip comments and empty lines
    [[ "$kinvar" =~ ^#.*$ ]] && continue
    [ -z "$kinvar" ] && continue
    
    BINS_ARRAY+=("$kinvar $varmin $varmax")
    echo "  ✓ Added: kinVar=$kinvar, $varname [$varmin, $varmax]"
done < "$BINS_FILE"

if [ ${#BINS_ARRAY[@]} -eq 0 ]; then
    echo "❌ Error: No valid bins found in $BINS_FILE"
    exit 1
fi

echo ""
echo "Total bins to process: ${#BINS_ARRAY[@]}"
echo ""

# Create temporary Condor submit file
CONDOR_FILE="fit_single_bin.condor"

cat > "$CONDOR_FILE" << 'CONDOR_EOF'
# Condor submit file for FitSingleBin.cpp batch processing
Universe           = vanilla
getenv             = True
should_transfer_files = YES
when_to_transfer_output = ON_EXIT

# Output files to transfer back
transfer_output_files = results

# Executable wrapper script
executable         = run_fit_single_bin.sh

# Output/Error/Log files
output             = logs/job_$(Cluster)_$(Process).out
error              = logs/job_$(Cluster)_$(Process).err
log                = logs/job_$(Cluster)_$(Process).log

CONDOR_EOF

# Add bins to condor file
for bin in "${BINS_ARRAY[@]}"; do
    read kinvar varmin varmax <<< "$bin"

    case "$kinvar" in
      0) vname="pT";;
      1) vname="y";;
      2) vname="centrality";;
      *) vname="pT";;
    esac

    printf -v vmin_fmt "%.2f" "$varmin"
    printf -v vmax_fmt "%.2f" "$varmax"
    mc_cache="results/histogram_cache/mc_histograms_${vname}_${vmin_fmt}_${vmax_fmt}.root"
    data_cache="results/histogram_cache/data_histograms_${vname}_${vmin_fmt}_${vmax_fmt}.root"

    base_inputs="run_fit_single_bin.sh, FitSingleBin.cpp, BDTParameterLoader.h, bdt_fit_parameters_phenom2.json"
    inputs="$base_inputs"
    if [[ -f "$mc_cache" ]]; then inputs="$inputs, $mc_cache"; fi
    if [[ -f "$data_cache" ]]; then inputs="$inputs, $data_cache"; fi

    {
      echo "transfer_input_files = $inputs"
      echo "arguments = \"$kinvar $varmin $varmax 0\""
      echo "queue"
      echo ""
    } >> "$CONDOR_FILE"

done

echo "✅ Created Condor submit file: $CONDOR_FILE"
echo ""
echo "=== CONDOR JOB QUEUE ==="
tail -n $((${#BINS_ARRAY[@]} * 2 + 3)) "$CONDOR_FILE"
echo ""

echo "✅ Created Condor submit file: $CONDOR_FILE"
echo ""
echo "ℹ️  Each Condor job will:"
echo "  1. Generate histograms for its kinematic bin"
echo "  2. Perform fitting"
echo ""

# Create logs directory
mkdir -p logs
chmod +x run_fit_single_bin.sh

echo ""
echo "📤 Submitting jobs to Condor..."
condor_submit "$CONDOR_FILE"

echo ""
echo "✅ Jobs submitted successfully!"
echo ""
echo "Monitor jobs with:"
echo "  condor_q"
echo "  condor_q <cluster_id>"
