#!/bin/bash

# ========================================
# PbPb RDS Maker - 테스트 스크립트
# ========================================
# ROOT 없이 스크립트 구조와 경로를 테스트

echo "========================================"
echo "    PbPb RDS Maker - Test Script"
echo "========================================"

# 공통 경로 설정
COMMON_DIR="/home/jun502s/DstarAna/DStarAnalysis/Preprocessing/Common"
DATA_DIR="/home/jun502s/DstarAna/DStarAnalysis/Data"
PBPB_DATA_DIR="/home/jun502s/DstarAna/DStarAnalysis/Preprocessing/PbPb/Data"

echo "Testing paths and structure..."

# 1. 경로 확인
echo "1. Checking paths..."
echo "   Common dir: $COMMON_DIR"
if [ -d "$COMMON_DIR" ]; then
    echo "   ✓ Common directory exists"
else
    echo "   ✗ Common directory not found"
fi

echo "   Data dir: $DATA_DIR"
if [ -d "$DATA_DIR" ]; then
    echo "   ✓ Data directory exists"
else
    echo "   ✗ Data directory not found"
fi

echo "   PbPb Data dir: $PBPB_DATA_DIR"
if [ -d "$PBPB_DATA_DIR" ]; then
    echo "   ✓ PbPb Data directory exists"
else
    echo "   ✗ PbPb Data directory not found"
fi

# 2. 필요한 파일 확인
echo ""
echo "2. Checking required files..."
cd "$COMMON_DIR"

if [ -f "DStarRDSMaker.h" ]; then
    echo "   ✓ DStarRDSMaker.h found"
else
    echo "   ✗ DStarRDSMaker.h not found"
fi

if [ -f "DStarRDSMaker.cpp" ]; then
    echo "   ✓ DStarRDSMaker.cpp found"
else
    echo "   ✗ DStarRDSMaker.cpp not found"
fi

if [ -f "DStarRDSMakerMain.cpp" ]; then
    echo "   ✓ DStarRDSMakerMain.cpp found"
else
    echo "   ✗ DStarRDSMakerMain.cpp not found"
fi

# 3. 입력 파일 경로 확인
echo ""
echo "3. Checking input file paths..."

# PbPb MC D* 파일
MC_DSTAR_FILE="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbMC/DStar/flatSkimForBDT_DStar_dstar_PbPb_MC_wEvtplane_Aug22.root"
echo "   MC D* file: $MC_DSTAR_FILE"
if [ -f "$MC_DSTAR_FILE" ]; then
    echo "   ✓ MC D* file exists"
else
    echo "   ✗ MC D* file not found"
fi

# PbPb Data D* 파일
DATA_DSTAR_FILE="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbData/0822/flatSkimForBDT_DStar_dstar_PbPb_Data_wEvtplane_mva0p99_Aug25.root"
echo "   Data D* file: $DATA_DSTAR_FILE"
if [ -f "$DATA_DSTAR_FILE" ]; then
    echo "   ✓ Data D* file exists"
else
    echo "   ✗ Data D* file not found"
fi

# 4. ROOT 환경 확인
echo ""
echo "4. Checking ROOT environment..."
if command -v root &> /dev/null; then
    echo "   ✓ ROOT is available"
    root --version 2>/dev/null | head -1
else
    echo "   ✗ ROOT is not available"
    echo "   Please source ROOT environment first"
fi

# 5. 출력 디렉토리 생성 테스트
echo ""
echo "5. Testing output directory creation..."
mkdir -p "$PBPB_DATA_DIR"
if [ -d "$PBPB_DATA_DIR" ]; then
    echo "   ✓ PbPb Data directory created successfully"
    echo "   Directory: $PBPB_DATA_DIR"
else
    echo "   ✗ Failed to create PbPb Data directory"
fi

# 6. 스크립트 실행 권한 확인
echo ""
echo "6. Checking script permissions..."
cd /home/jun502s/DstarAna/DStarAnalysis/Preprocessing/PbPb

if [ -x "RunConfig_RDS_Improved.sh" ]; then
    echo "   ✓ RunConfig_RDS_Improved.sh is executable"
else
    echo "   ✗ RunConfig_RDS_Improved.sh is not executable"
fi

if [ -x "run_pbpb_examples.sh" ]; then
    echo "   ✓ run_pbpb_examples.sh is executable"
else
    echo "   ✗ run_pbpb_examples.sh is not executable"
fi

echo ""
echo "========================================"
echo "Test completed!"
echo "========================================"
echo ""
echo "Next steps:"
echo "1. Source ROOT environment: source /path/to/root/bin/thisroot.sh"
echo "2. Run: ./RunConfig_RDS_Improved.sh"
echo "3. Or run: ./run_pbpb_examples.sh"
echo "========================================"
