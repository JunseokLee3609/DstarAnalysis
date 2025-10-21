#!/bin/bash

# Generate JSON parameters for PP and PbPb cases
# Usage: ./generate_parameters.sh [PP|PbPb] [version]

set -e

# Default values
CASE=${1:-"PP"}
VERSION=${2:-"v1"}
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PYTHON_SCRIPT="$SCRIPT_DIR/GenerateJSONWithDCA.py"

# Check if Python script exists
if [ ! -f "$PYTHON_SCRIPT" ]; then
    echo "Error: GenerateJSONWithDCA.py not found at $PYTHON_SCRIPT"
    exit 1
fi

echo "=== Generating JSON Parameters ==="
echo "Case: $CASE"
echo "Version: $VERSION"
echo "Script: $PYTHON_SCRIPT"
echo ""

# Common parameters
PT_BINS="5:7 7:10 10:20 20:50"
CENT_BINS="0:100"
DCA_BINS="0.0:0.0012 0.0012:0.0023 0.0023:0.0039 0.0039:0.0059 0.0059:0.0085 0.0085:0.0160 0.0160:0.0281 0.0281:0.0476 0.0476:0.07"

if [ "$CASE" = "PP" ]; then
    echo "--- Generating PP Parameters ---"
    
    # PP case: symmetric cos bins
    COS_BINS="-1.0:-0.8 -0.8:-0.6 -0.6:-0.4 -0.4:-0.2 -0.2:0.0 0.0:0.2 0.2:0.4 0.4:0.6 0.6:0.8 0.8:1.0"
    OUTPUT_FILE="dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_${VERSION}.json"
    
    echo "Output file: $OUTPUT_FILE"
    echo "PT bins: $PT_BINS"
    echo "COS bins: $COS_BINS"
    echo "CENT bins: $CENT_BINS"
    echo "DCA bins: $DCA_BINS"
    echo ""
    
    python3 "$PYTHON_SCRIPT" \
        --output "$OUTPUT_FILE" \
        --pt-bins $PT_BINS \
        --cos-bins $COS_BINS \
        --cent-bins $CENT_BINS \
        --dca-bins $DCA_BINS
    
    echo ""
    echo "✅ PP parameters generated: $OUTPUT_FILE"
    
elif [ "$CASE" = "PbPb" ]; then
    echo "--- Generating PbPb Parameters ---"
    
    # PbPb case: positive cos bins only
    COS_BINS="0:0.2 0.2:0.4 0.4:0.6 0.6:0.8 0.8:1.0"
    OUTPUT_FILE="dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_${VERSION}.json"
    
    echo "Output file: $OUTPUT_FILE"
    echo "PT bins: $PT_BINS"
    echo "COS bins: $COS_BINS"
    echo "CENT bins: $CENT_BINS"
    echo "DCA bins: $DCA_BINS"
    echo ""
    
    python3 "$PYTHON_SCRIPT" \
        --output "$OUTPUT_FILE" \
        --pt-bins $PT_BINS \
        --cos-bins $COS_BINS \
        --cent-bins $CENT_BINS \
        --dca-bins $DCA_BINS
    
    echo ""
    echo "✅ PbPb parameters generated: $OUTPUT_FILE"
    
else
    echo "Error: Invalid case '$CASE'. Use 'PP' or 'PbPb'"
    echo ""
    echo "Usage: $0 [PP|PbPb] [version]"
    echo ""
    echo "Examples:"
    echo "  $0 PP v1        # Generate PP parameters v1"
    echo "  $0 PbPb v2      # Generate PbPb parameters v2"
    echo "  $0 PP           # Generate PP parameters (default v1)"
    exit 1
fi

echo ""
echo "=== Generation Complete ==="
echo "Generated file: $OUTPUT_FILE"
echo "Total bins: $(echo $PT_BINS $COS_BINS $CENT_BINS $DCA_BINS | wc -w) combinations"
