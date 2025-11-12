#!/bin/bash

if [ $# -lt 3 ] || [ $# -gt 4 ]; then
    echo "Usage: $0 <filePath> <isMC> <prefix> [chunkSize]"
    echo "  filePath: Path to input ROOT files directory"
    echo "  isMC: 0 for data, 1 for MC"
    echo "  prefix: Output prefix for naming"
    echo "  chunkSize: Number of ROOT files per job (default: 100)"
    echo "  exmaple : /eos/cms/store/group/phys_heavyions/junseok/DstarAnalysis/HIPhysicsRawPrime12 0 PbPbStage 100" 
    exit 1
fi

filePath=$1
isMC=$2
prefix=$3
chunkSize=${4:-100}

echo "Starting condor submission process..."
echo "File path: $filePath"
echo "Is MC: $isMC"
echo "Prefix: $prefix"
echo "Chunk size: $chunkSize"

OUTPUT_DIR="file_lists"
mkdir -p "$OUTPUT_DIR"
mkdir -p "logs"

rm -f "$OUTPUT_DIR"/files*.txt
rm -f "combined_file_list.txt"

echo "Generating file lists (XRootD paths)..."
find "$filePath" -maxdepth 10 -type f -name "*.root" | sed 's|^/eos/cms|root://eoscms.cern.ch//eos/cms|' | awk -v n="$chunkSize" -v outdir="$OUTPUT_DIR" '{
    file = outdir "/files" int((NR-1)/n)+1 ".txt";
    print $0 > file
}'

# Use basenames in combined list to avoid EOS paths in submit
find "$OUTPUT_DIR" -type f -name "files*.txt" | awk -F'/' '{print $NF, NR}' > "combined_file_list.txt"

file_count=$(wc -l < "combined_file_list.txt")
echo "Generated $file_count file lists in $OUTPUT_DIR directory"

# Stage submission to AFS/local (HTCondor cannot submit from EOS)
USER_INITIAL="${USER:0:1}" 2>/dev/null || USER_INITIAL="${LOGNAME:0:1}"
STAGE_BASE="/afs/cern.ch/user/${USER_INITIAL}/${USER}/condor_stage"
[ -d "$STAGE_BASE" ] || mkdir -p "$STAGE_BASE"
STAGE_DIR="$STAGE_BASE/${prefix}_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$STAGE_DIR/file_lists"
# Copy macro sources and file lists
cp ../Common/FlexibleFlattener.cpp "$STAGE_DIR/" || true
cp ../Common/FlexibleFlattenerForCondor.cpp "$STAGE_DIR/" || true
cp combined_file_list.txt "$STAGE_DIR/"
cp file_lists/files*.txt "$STAGE_DIR/file_lists/"
echo "Staged submission directory: $STAGE_DIR"

cat > "$STAGE_DIR/runSkim_temp.sh" << EOF
#!/bin/bash

if [ -z "\$ROOTSYS" ]; then
	cd /u/user/jun502s/dstarana/skim/CMSSW_13_2_11/src && eval \`scramv1 runtime -sh\` && cd -
fi
# Stay in execution directory (staged) so macros are found
# cd /u/user/jun502s/dstarana/skim/DstarAnalysis/Macro/skim

inputFile=\$1
num=\$2

echo "Processing input File: \$inputFile and \$num" 

while read -r f; do
  [ -z "\$f" ] && continue
  echo "  -> Running on \$f"
  root -l -b -q "Preprocessing/Common/FlexibleFlattenerForCondor.cpp(\"$inputFile\",$isMC,\"$prefix\",$num,\"DStar\",\"PbPb\")"
done < "\$inputFile"
EOF

chmod +x "$STAGE_DIR/runSkim_temp.sh"

cat > "$STAGE_DIR/condor_submit_temp.sub" << EOF
executable         = runSkim_temp.sh
getenv = True
arguments          = \$(inputPath) \$(num) 
output             = logs/\$(Cluster)_\$(Process).out
error              = logs/\$(Cluster)_\$(Process).err
log                = logs/\$(Cluster)_\$(Process).log
request_memory = 4 GB
should_transfer_files = YES
when_to_transfer_output = ON_EXIT

transfer_input_files = FlexibleFlattener.cpp, FlexibleFlattenerForCondor.cpp, combined_file_list.txt, file_lists/\$(inputPath)

queue inputPath num from combined_file_list.txt
EOF

echo "Submitting to condor..."
(cd "$STAGE_DIR" && condor_submit condor_submit_temp.sub)

echo "Condor submission completed!"
echo "Monitor job status with: condor_q"
echo "View logs in the logs/ directory"

#rm -f runSkim_temp.sh condor_submit_temp.sub
