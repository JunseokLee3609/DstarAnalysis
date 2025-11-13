#!/bin/bash

if [ $# -lt 3 ] || [ $# -gt 5 ]; then
    echo "Usage: $0 <filePath> <isMC> <prefix> [evtplane_calib] [chunkSize]"
    echo "  filePath: Path to input ROOT files directory"
    echo "  isMC: 0 for data, 1 for MC"
    echo "  prefix: Output prefix for naming"
    echo "  evtplane_calib: Event plane calibration file (optional, default: \"\")"
    echo "  chunkSize: Number of ROOT files per job (default: 100)"
    exit 1
fi

filePath=$1
isMC=$2
prefix=$3
evtplane_calib=${4:-""}
chunkSize=${5:-100}

echo "Starting condor submission process..."
echo "File path: $filePath"
echo "Is MC: $isMC"
echo "Prefix: $prefix"
echo "Event plane calib: $evtplane_calib"
echo "Chunk size: $chunkSize"

OUTPUT_DIR="file_lists"
mkdir -p "$OUTPUT_DIR"
mkdir -p "logs"

rm -f "$OUTPUT_DIR"/files*.txt
rm -f "combined_file_list.txt"

echo "Generating file lists..."
find "$filePath" -maxdepth 10 -type f -name "*.root" | awk -v n="$chunkSize" -v outdir="$OUTPUT_DIR" '{
    file = outdir "/files" int((NR-1)/n)+1 ".txt";
    print $0 > file
}'

find "$OUTPUT_DIR" -type f -name "files*.txt" | awk '{print $0, NR}' > "combined_file_list.txt"

file_count=$(wc -l < "combined_file_list.txt")
echo "Generated $file_count file lists in $OUTPUT_DIR directory"

cat > runSkim_temp.sh << EOF
#!/bin/bash

if [ -z "\$ROOTSYS" ]; then
	cd /u/user/jun502s/dstarana/skim/CMSSW_13_2_11/src && eval \`scramv1 runtime -sh\` && cd -
fi
cd /u/user/jun502s/dstarana/skim/DstarAnalysis/Preprocessing

inputFile=\$1
num=\$2
CUR_DIR="\$(pwd)"
if [[ "\$inputFile" != /* ]]; then inputFile="\$CUR_DIR/\$inputFile"; fi

echo "Processing: \$inputFile job \$num"

echo "Input list: \$inputFile"

root -l -b -q "Common/FlexibleFlattener.cpp($isMC,\"DStar\",\"PbPb\",\$num,0,-1,\"\$inputFile\",\"$prefix\",\"\",\"\",\"$evtplane_calib\")"
EOF

chmod +x runSkim_temp.sh

cat > condor_submit_temp.sub << EOF
executable         = runSkim_temp.sh
getenv = True
arguments          = \$(inputPath) \$(num) 
output             = logs/\$(Cluster)_\$(Process).out
error              = logs/\$(Cluster)_\$(Process).err
log                = logs/\$(Cluster)_\$(Process).log
request_memory = 4 GB
should_transfer_files = YES
when_to_transfer_output = ON_EXIT

transfer_input_files = combined_file_list.txt, \$(inputPath)

queue inputPath num from combined_file_list.txt
EOF

echo "Submitting to condor..."
condor_submit condor_submit_temp.sub

echo "Condor submission completed!"
echo "Monitor job status with: condor_q"
echo "View logs in the logs/ directory"
