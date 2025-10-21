#!/bin/bash

# ========================================
# Organize Correction Code
# ========================================
# Organizes correction-related files into proper structure
# ========================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Organizing Correction Code${NC}"
echo -e "${BLUE}========================================${NC}\n"

cd "${BASE_DIR}"

# Create Corrections directory structure
echo -e "${YELLOW}Creating Corrections directory structure...${NC}"
mkdir -p Corrections/{Common,Generators/{PP,PbPb,D0},Validation,scripts}
mkdir -p Data/Auxiliary/{EfficiencyMaps/{PP,PbPb},CorrectionFactors/{PP,PbPb},AcceptanceMaps/{PP,PbPb}}
echo -e "${GREEN}✓ Directory structure created${NC}\n"

# Move common correction utilities
echo -e "${YELLOW}Moving common correction utilities...${NC}"
cp Macro/Correction/AccEffHelpers.h Corrections/Common/ 2>/dev/null || true
cp Macro/Correction/EffHead.h Corrections/Common/ 2>/dev/null || true
cp Macro/interface/correctionmap.hxx Corrections/Common/ 2>/dev/null || true
echo -e "${GREEN}✓ Common utilities moved${NC}\n"

# Move correction generators
echo -e "${YELLOW}Moving correction generators...${NC}"

# PP generators
echo "  PP correction generators..."
cp Macro/Correction/EfficiencyMaps.Cpp Corrections/Generators/PP/EfficiencyMaps_PP.cpp 2>/dev/null || true
cp Macro/Correction/FlatEffAccCalculator.cpp Corrections/Generators/PP/FlatEffAccCalculator_PP.cpp 2>/dev/null || true
cp Macro/Correction/AcceptanceMap_noGenFilter.cpp Corrections/Generators/PP/ 2>/dev/null || true
cp Macro/Correction/D0pTEtaMap.cpp Corrections/Generators/PP/ 2>/dev/null || true

# PbPb generators
echo "  PbPb correction generators..."
cp Macro/Correction/FlatEffAccCalculator_PbPb.cpp Corrections/Generators/PbPb/ 2>/dev/null || true

# D0 generators
echo "  D0 correction generators..."
cp Macro/Correction/FlatEffAccCalculator_D0.cpp Corrections/Generators/D0/ 2>/dev/null || true
cp Macro/Correction/MVAEfficiency.cpp Corrections/Generators/D0/ 2>/dev/null || true

echo -e "${GREEN}✓ Generators moved${NC}\n"

# Move validation scripts
echo -e "${YELLOW}Moving validation scripts...${NC}"
cp Macro/Correction/PlotAcceptanceFromFile.cpp Corrections/Validation/ 2>/dev/null || true
cp Macro/test/cosThetaCorrection/HistBasedCorrection.cpp Corrections/Validation/ 2>/dev/null || true
cp Macro/Correction/test2.cpp Corrections/Validation/TestCorrection.cpp 2>/dev/null || true
echo -e "${GREEN}✓ Validation scripts moved${NC}\n"

# Move existing efficiency maps
echo -e "${YELLOW}Organizing existing efficiency map files...${NC}"
if [ -d "Macro/Correction/EfficiencyMaps" ]; then
    # Link instead of copy to save space
    ln -sf "$(pwd)/Macro/Correction/EfficiencyMaps" Data/Auxiliary/EfficiencyMaps/Legacy
    echo -e "${GREEN}✓ Linked legacy efficiency maps${NC}"
fi
echo ""

# Create helper utilities in Analysis/Common/Utils
echo -e "${YELLOW}Creating correction utilities for analysis...${NC}"
cat > Analysis/Common/Utils/CorrectionUtils.h << 'EOF'
#ifndef CORRECTION_UTILS_H
#define CORRECTION_UTILS_H

// ========================================
// Correction Utilities
// ========================================
// Helper functions for applying corrections in analysis
// ========================================

#include "TFile.h"
#include "TH3D.h"
#include <string>
#include <memory>

namespace CorrectionUtils {

/**
 * @brief Load efficiency map from file
 * @param filePath Path to efficiency map ROOT file
 * @param mapName Name of the histogram in the file
 * @return Unique pointer to TH3D efficiency map
 */
inline std::unique_ptr<TH3D> LoadEfficiencyMap(
    const std::string& filePath, 
    const std::string& mapName = "efficiency_map") 
{
    std::unique_ptr<TFile> file(TFile::Open(filePath.c_str()));
    if (!file || file->IsZombie()) {
        std::cerr << "ERROR: Cannot open efficiency file: " << filePath << std::endl;
        return nullptr;
    }
    
    TH3D* h_eff = dynamic_cast<TH3D*>(file->Get(mapName.c_str()));
    if (!h_eff) {
        std::cerr << "ERROR: Efficiency map '" << mapName << "' not found in file" << std::endl;
        return nullptr;
    }
    
    h_eff->SetDirectory(0);  // Detach from file
    return std::unique_ptr<TH3D>(h_eff);
}

/**
 * @brief Get efficiency weight for a given kinematic point
 * @param h_eff Efficiency map histogram
 * @param pT Transverse momentum
 * @param y Rapidity
 * @param cosTheta Cosine of helicity angle
 * @return Weight = 1/efficiency (or 0 if efficiency is too low)
 */
inline double GetEfficiencyWeight(
    TH3D* h_eff, 
    double pT, 
    double y, 
    double cosTheta,
    double minEfficiency = 1e-6) 
{
    if (!h_eff) return 1.0;
    
    int bin = h_eff->FindBin(pT, y, std::abs(cosTheta));
    double efficiency = h_eff->GetBinContent(bin);
    
    if (efficiency < minEfficiency) {
        return 0.0;  // Skip events with very low efficiency
    }
    
    return 1.0 / efficiency;
}

/**
 * @brief Load acceptance map from file
 */
inline std::unique_ptr<TH2D> LoadAcceptanceMap(
    const std::string& filePath, 
    const std::string& mapName = "acceptance_map") 
{
    std::unique_ptr<TFile> file(TFile::Open(filePath.c_str()));
    if (!file || file->IsZombie()) {
        std::cerr << "ERROR: Cannot open acceptance file: " << filePath << std::endl;
        return nullptr;
    }
    
    TH2D* h_acc = dynamic_cast<TH2D*>(file->Get(mapName.c_str()));
    if (!h_acc) {
        std::cerr << "ERROR: Acceptance map '" << mapName << "' not found in file" << std::endl;
        return nullptr;
    }
    
    h_acc->SetDirectory(0);
    return std::unique_ptr<TH2D>(h_acc);
}

} // namespace CorrectionUtils

#endif // CORRECTION_UTILS_H
EOF
echo -e "${GREEN}✓ Created CorrectionUtils.h${NC}\n"

# Create README for Corrections directory
cat > Corrections/README.md << 'EOF'
# Corrections Directory

이 디렉토리는 효율성(efficiency) 및 수용도(acceptance) 보정과 관련된 모든 코드를 포함합니다.

## 📁 구조

### Common/
공통 유틸리티 및 헬퍼 클래스
- `AccEffHelpers.h` - Acceptance & Efficiency 계산 헬퍼
- `EffHead.h` - Efficiency 계산 헤더
- `correctionmap.hxx` - Correction map 인터페이스

### Generators/
Correction map을 생성하는 코드

#### PP/
pp 충돌 시스템용 correction 생성
- `EfficiencyMaps_PP.cpp` - Efficiency map 생성
- `FlatEffAccCalculator_PP.cpp` - Flat tree에서 효율 계산
- `AcceptanceMap_noGenFilter.cpp` - Acceptance map 생성

#### PbPb/
PbPb 충돌 시스템용 correction 생성
- `FlatEffAccCalculator_PbPb.cpp` - PbPb 효율 계산

#### D0/
D0 메손 특화 correction
- `FlatEffAccCalculator_D0.cpp` - D0 효율 계산
- `MVAEfficiency.cpp` - MVA 기반 효율 계산

### Validation/
Correction의 유효성을 검증하는 코드
- `PlotAcceptanceFromFile.cpp` - Acceptance map 시각화
- `HistBasedCorrection.cpp` - 히스토그램 기반 보정 검증
- `TestCorrection.cpp` - 보정 테스트

### scripts/
Correction 생성 및 검증 실행 스크립트

## 🚀 사용법

### 1. Efficiency Map 생성
```bash
cd Corrections/Generators/PP
root -l -b -q EfficiencyMaps_PP.cpp
```

생성된 파일은 자동으로 `Data/Auxiliary/EfficiencyMaps/PP/`에 저장됩니다.

### 2. Acceptance Map 생성
```bash
cd Corrections/Generators/PP
root -l -b -q AcceptanceMap_noGenFilter.cpp
```

### 3. Correction 검증
```bash
cd Corrections/Validation
root -l -b -q PlotAcceptanceFromFile.cpp
```

### 4. 분석에서 사용
```cpp
#include "../../Common/Utils/CorrectionUtils.h"

// Efficiency map 로드
auto h_eff = CorrectionUtils::LoadEfficiencyMap(
    "Data/Auxiliary/EfficiencyMaps/PP/efficiency_map.root"
);

// 이벤트별 가중치 계산
double weight = CorrectionUtils::GetEfficiencyWeight(
    h_eff.get(), pT, y, cosTheta
);
```

## 📊 출력 위치

생성된 correction map 파일들:
- Efficiency maps → `Data/Auxiliary/EfficiencyMaps/{PP,PbPb}/`
- Acceptance maps → `Data/Auxiliary/AcceptanceMaps/{PP,PbPb}/`
- Correction factors → `Data/Auxiliary/CorrectionFactors/{PP,PbPb}/`

## 📝 노트

- PP와 PbPb의 correction은 별도로 생성되어야 합니다
- 각 correction map은 kinematic bin별로 다를 수 있습니다
- 분석 전에 항상 적절한 correction map이 적용되었는지 확인하세요
EOF

echo -e "${GREEN}✓ Created README.md${NC}\n"

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Correction organization complete!${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo "New structure:"
echo "  Corrections/Common/       - Common utilities"
echo "  Corrections/Generators/   - Map generation code"
echo "  Corrections/Validation/   - Validation scripts"
echo "  Data/Auxiliary/           - Generated maps"
echo ""
echo "Integration with analysis:"
echo "  Analysis/Common/Utils/CorrectionUtils.h - Use this in your analysis"
echo ""
echo "Next steps:"
echo "  1. Review moved files: ls -la Corrections/"
echo "  2. Check README: cat Corrections/README.md"
echo "  3. Update include paths in generator scripts if needed"
echo ""

