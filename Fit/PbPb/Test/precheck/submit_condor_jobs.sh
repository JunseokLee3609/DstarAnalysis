#!/bin/bash
# Submit Condor jobs for MakeHistKinematicBin.cpp with cosThetaEP binning (all DCA modes: inclusive, prompt, nonprompt)
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BINS_FILE="kinematic_bins_queue_cosEP_dedup.txt"
if [[ ! -f "$BINS_FILE" ]]; then echo "Error: $BINS_FILE not found"; exit 1; fi

MVA_LIST=("0.90" "0.990" "0.999")
CONDOR_FILE="make_hist_bins.condor"

mkdir -p logs
cat > "$CONDOR_FILE" << 'CONDOR_EOF'
Universe = vanilla
getenv = True
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_output_files = results
executable = run_make_hist_bin.sh
output = logs/job_$(Process).out
error  = logs/job_$(Process).err
log    = logs/job_$(Process).log
CONDOR_EOF

# Append jobs per combined bin and per MVA
while read -r ptmin ptmax yamin yamax cmin cmax dcamode cosepmin cosepmax; do
  [[ "$ptmin" =~ ^# ]] && continue
  [[ -z "$ptmin" ]] && continue
  for mva in "${MVA_LIST[@]}"; do
    {
      echo "transfer_input_files = run_make_hist_bin.sh, MakeHistKinematicBin.cpp, KinematicBinConfig.h"
      echo "arguments = \"$ptmin $ptmax $yamin $yamax $cmin $cmax $mva $dcamode $cosepmin $cosepmax\""
      echo "queue"
      echo ""
    } >> "$CONDOR_FILE"
  done
done < "$BINS_FILE"

chmod +x run_make_hist_bin.sh

jobs=$(grep -c '^arguments' "$CONDOR_FILE")
echo "Submitting $jobs jobs..."
condor_submit "$CONDOR_FILE"
echo "Submitted. Condor file: $CONDOR_FILE"
