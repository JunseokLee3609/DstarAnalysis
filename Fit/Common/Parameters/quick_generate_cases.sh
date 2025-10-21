#!/bin/bash

# Quick parameter generation for both PP and PbPb cases
# Usage: ./quick_generate_cases.sh

echo "=== Quick Parameter Generation for Both Cases ==="

# Generate PP parameters
echo "Generating PP parameters..."
python3 GenerateJSONWithDCA.py \
    --case PP \
    --output "PP/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v1.json" \
    --pt-bins 5:7 7:10 10:20 20:50 \
    --cent-bins 0:100 \
    --dca-bins 0.0:0.0012 0.0012:0.0023 0.0023:0.0039 0.0039:0.0059 0.0059:0.0085 0.0085:0.0160 0.0160:0.0281 0.0281:0.0476 0.0476:0.07

echo "✅ PP parameters generated"

# Generate PbPb parameters
echo "Generating PbPb parameters..."
python3 GenerateJSONWithDCA.py \
    --case PbPb \
    --output "PbPb/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_Oct21_v1.json" \
    --pt-bins 5:7 7:10 10:20 20:50 \
    --cent-bins 0:10 30:50 \
    --dca-bins 0.0:0.0012 0.0012:0.0023 0.0023:0.0039 0.0039:0.0059 0.0059:0.0085 0.0085:0.0160 0.0160:0.0281 0.0281:0.0476 0.0476:0.07
    
    # --background DstD0 \
    # --output PbPb/dstar_parameters_DBCrystalBall_DstD0_dca_pbpb_v3.json \


echo "✅ PbPb parameters generated"
echo "=== Complete ==="
echo "Generated files:"
echo "  PP/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v1.json"
echo "  PbPb/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_v1.json"
