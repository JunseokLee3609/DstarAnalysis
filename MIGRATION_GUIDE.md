# 📚 마이그레이션 가이드

## 개요

이 가이드는 D* Analysis 레포지토리를 새로운 구조로 마이그레이션하는 방법을 설명합니다.

## 🎯 마이그레이션 목적

### 기존 문제점
- ❌ pp와 PbPb 분석 코드가 혼재 (`DStarAnalysisV2forpp.cpp`, `DStarAnalysisV2forPbPb.cpp`)
- ❌ 백업 파일이 산재 (`*_backup.h`, `*_old.*`, `*.bak`)
- ❌ 데이터 파일이 여러 위치에 분산
- ❌ 공통 코드와 특화 코드의 구분이 불명확
- ❌ 테스트 코드가 메인 코드와 섞임

### 개선 사항
- ✅ 충돌 시스템별 명확한 분리 (PP / PbPb)
- ✅ 공통 코드의 체계적 관리 (Common/)
- ✅ 데이터 파일의 계층적 구조
- ✅ 백업 파일 정리
- ✅ 테스트 코드 분리

## 📁 새로운 구조

```
DStarAnalysis/
│
├── Analysis/                    # 📊 분석 코드
│   ├── Common/                  # 공통 프레임워크
│   │   ├── Core/                # 핵심 분석 엔진
│   │   │   ├── MassFitterV2.h
│   │   │   ├── DCAFitter.h
│   │   │   ├── DataLoader.h
│   │   │   └── PDFFactory.h
│   │   ├── Config/              # 설정 관리
│   │   │   ├── DStarFitConfig.h
│   │   │   └── DStarFitOpt.h
│   │   ├── Managers/            # 매니저 클래스
│   │   │   ├── ParameterManager.h
│   │   │   ├── ResultManager.h
│   │   │   └── EnhancedPlotManager.h
│   │   ├── Utils/               # 유틸리티
│   │   │   ├── Helper.h
│   │   │   └── JSONParameterUtils.h
│   │   └── Parameters/          # JSON 파라미터
│   │       ├── pp/
│   │       └── PbPb/
│   │
│   ├── PP/                      # pp 전용
│   │   ├── DStarAnalysisPP.cpp
│   │   ├── ConfigPP.h
│   │   ├── scripts/
│   │   └── results/
│   │
│   ├── PbPb/                    # PbPb 전용
│   │   ├── DStarAnalysisPbPb.cpp
│   │   ├── ConfigPbPb.h
│   │   ├── scripts/
│   │   └── results/
│   │
│   └── Tests/                   # 테스트
│
├── Preprocessing/               # 🔧 전처리
│   ├── Common/
│   │   ├── DStarRDSMakerImproved.cpp
│   │   ├── FlexibleFlattener.cpp
│   │   └── ImprovedDataProcessor.cpp
│   ├── PP/
│   └── PbPb/
│
├── Tools/                       # 🛠️ 공통 도구
│   ├── Kinematics/             # 운동학 변환
│   ├── Utils/                   # 유틸리티
│   ├── Plotting/               # 플로팅
│   └── Base/                    # 기본 클래스
│
├── Data/                        # 💾 데이터
│   ├── Raw/                     # 원본
│   │   ├── PP/
│   │   └── PbPb/
│   ├── Skimmed/                 # Skim된 데이터
│   │   ├── PP/
│   │   └── PbPb/
│   ├── RDS/                     # RooDataSet
│   │   ├── PP/
│   │   └── PbPb/
│   └── Auxiliary/               # 보조 데이터
│
├── Results/                     # 📈 결과 (gitignore)
│   ├── PP/
│   └── PbPb/
│
├── Documentation/               # 📖 문서
│   ├── README_OLD.md
│   ├── CHANGELOG.md
│   └── UserGuide.md
│
└── Scripts/                     # 🔨 스크립트
    └── utilities/
```

## 🚀 마이그레이션 실행

### 1단계: 백업 생성 (필수!)

```bash
cd /home/jun502s/DstarAna/DStarAnalysis
chmod +x migrate_to_new_structure.sh
./migrate_to_new_structure.sh
```

스크립트가 자동으로 백업을 생성합니다:
- 위치: `DStarAnalysis_backup_YYYYMMDD_HHMMSS/`

### 2단계: 변경 사항 확인

```bash
# 새로운 구조 확인
tree -L 3 -d Analysis/
tree -L 3 -d Data/

# 파일이 제대로 복사되었는지 확인
ls -la Analysis/Common/Core/
ls -la Analysis/PP/
ls -la Analysis/PbPb/
```

### 3단계: Include 경로 업데이트

새 구조에서 include 경로를 업데이트해야 합니다:

#### 기존 코드:
```cpp
#include "../MassFitterV2.h"
#include "../DStarFitConfig.h"
#include "../../Tools/ConfigManagerPP.h"
```

#### 새 구조:
```cpp
#include "../../Common/Core/MassFitterV2.h"
#include "../../Common/Config/DStarFitConfig.h"
#include "../ConfigPP.h"
```

자동 업데이트 스크립트를 제공합니다 (아래 참조).

### 4단계: 테스트

```bash
# PP 분석 테스트
cd Analysis/PP/scripts
./run_pp_analysis.sh

# PbPb 분석 테스트
cd ../../PbPb/scripts
./run_pbpb_analysis.sh
```

## 🔧 Include 경로 자동 업데이트

포함 경로를 자동으로 업데이트하는 스크립트:

```bash
cd /home/jun502s/DstarAna/DStarAnalysis
./Scripts/utilities/update_includes.sh
```

## 📝 파일 매핑표

### 분석 코드

| 기존 위치 | 새 위치 |
|----------|---------|
| `Macro/fit/testRun/DStarAnalysisV2forpp.cpp` | `Analysis/PP/DStarAnalysisPP.cpp` |
| `Macro/fit/testRun/DStarAnalysisV2forPbPb.cpp` | `Analysis/PbPb/DStarAnalysisPbPb.cpp` |
| `Macro/fit/MassFitterV2.h` | `Analysis/Common/Core/MassFitterV2.h` |
| `Macro/fit/DStarFitConfig.h` | `Analysis/Common/Config/DStarFitConfig.h` |

### 전처리 코드

| 기존 위치 | 새 위치 |
|----------|---------|
| `Macro/skim/DStarRDSMakerImproved.cpp` | `Preprocessing/Common/DStarRDSMakerImproved.cpp` |
| `Macro/skim/FlexibleFlattener.cpp` | `Preprocessing/Common/FlexibleFlattener.cpp` |

### 도구

| 기존 위치 | 새 위치 |
|----------|---------|
| `Macro/Tools/Transformations.h` | `Tools/Kinematics/Transformations.h` |
| `Macro/Tools/Style/CMS_lumi.h` | `Tools/Plotting/CMS_lumi.h` |

## 🗑️ 정리할 파일들

### 백업 파일
```bash
# 백업 파일 목록 확인
./Scripts/utilities/list_backup_files.sh

# 백업 파일 삭제 (주의!)
find . -type f \( -name "*_backup.*" -o -name "*.backup" -o -name "*_old.*" -o -name "*.bak" \) -delete
```

### 제거 권장 파일들
- `Macro/fit/*_backup.h`
- `Macro/fit/*_temp.h`
- `Macro/fit/testRun/*_cpp.d`
- `Macro/skim/*_copy.*`

## ⚠️ 주의사항

1. **백업 필수**: 마이그레이션 전에 반드시 백업을 생성하세요
2. **Include 경로**: 코드에서 include 경로를 수동으로 확인하고 업데이트하세요
3. **데이터 파일**: 큰 데이터 파일(*.root)은 심볼릭 링크로 연결됩니다
4. **테스트**: 마이그레이션 후 모든 분석 파이프라인을 테스트하세요
5. **Git**: 변경사항을 커밋하기 전에 충분히 테스트하세요

## 🔄 롤백 방법

문제가 발생하면 백업에서 복원할 수 있습니다:

```bash
# 1. 현재 디렉토리 이름 변경
mv DStarAnalysis DStarAnalysis_failed

# 2. 백업 복원
cp -r DStarAnalysis_backup_YYYYMMDD_HHMMSS DStarAnalysis

# 3. 확인
cd DStarAnalysis
ls -la
```

## 📞 문제 해결

### Q: Include 에러가 발생합니다
A: Include 경로를 새 구조에 맞게 업데이트하세요. 자동 업데이트 스크립트를 사용하거나 수동으로 수정하세요.

### Q: 데이터 파일을 찾을 수 없습니다
A: 심볼릭 링크가 제대로 생성되었는지 확인하세요:
```bash
ls -la Data/RDS/PP/Data/
ls -la Data/Skimmed/PP/Data/
```

### Q: 기존 코드도 사용하고 싶습니다
A: 백업 디렉토리의 코드를 사용하거나, 기존 `Macro/` 디렉토리를 그대로 유지할 수 있습니다.

## 📊 마이그레이션 체크리스트

- [ ] 백업 생성 완료
- [ ] 새 디렉토리 구조 생성
- [ ] 공통 코드 이동 완료
- [ ] PP 코드 이동 완료
- [ ] PbPb 코드 이동 완료
- [ ] 전처리 코드 이동 완료
- [ ] Tools 재구성 완료
- [ ] 데이터 디렉토리 정리 완료
- [ ] Include 경로 업데이트 완료
- [ ] PP 분석 테스트 성공
- [ ] PbPb 분석 테스트 성공
- [ ] 백업 파일 정리 완료
- [ ] .gitignore 업데이트 완료
- [ ] 문서 업데이트 완료
- [ ] Git 커밋 완료

## 📅 마이그레이션 로그

마이그레이션 날짜: __________
마이그레이션 수행자: __________
백업 위치: __________
문제 사항: __________
해결 방법: __________

