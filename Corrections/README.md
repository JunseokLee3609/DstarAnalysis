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
