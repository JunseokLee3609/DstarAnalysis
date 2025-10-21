# D* Meson Analysis Framework

<div align="center">

**현대화되고 재구성된 D* 메손 분석 프레임워크**

[![ROOT](https://img.shields.io/badge/ROOT-6.24+-blue.svg)](https://root.cern.ch/)
[![C++](https://img.shields.io/badge/C++-17-orange.svg)](https://isocpp.org/)
[![RooFit](https://img.shields.io/badge/RooFit-enabled-green.svg)](https://root.cern.ch/roofit)

</div>

---

## 📋 목차

- [개요](#-개요)
- [새로운 구조](#-새로운-구조)
- [빠른 시작](#-빠른-시작)
- [기능](#-기능)
- [마이그레이션](#-마이그레이션)
- [사용법](#-사용법)
- [문서](#-문서)

## 🎯 개요

이 레포지토리는 pp 및 PbPb 충돌에서의 D* 메손 분석을 위한 통합 프레임워크입니다.

### 주요 특징

- ✨ **충돌 시스템별 분리**: PP와 PbPb 분석 코드 명확히 분리
- 🔧 **모듈화된 구조**: 공통 코드와 특화 코드 체계적 관리
- 📊 **개선된 데이터 관리**: 계층적 데이터 구조
- 🚀 **최신 C++ 기능**: C++17 기반 현대적 코딩
- 📝 **자동화된 도구**: 마이그레이션, 검증, 정리 스크립트

### 최근 개선 사항 (2025년)

- ✅ 레포지토리 구조 전면 개편
- ✅ `DStarRDSMakerImproved.cpp` - 버그 수정 및 성능 최적화
- ✅ 백업 파일 정리 및 .gitignore 개선
- ✅ 포괄적인 문서화

## 📁 새로운 구조

```
DStarAnalysis/
│
├── 📊 Analysis/                 # 분석 코드
│   ├── 🔄 Common/               # 공통 분석 프레임워크
│   │   ├── Core/                # 핵심 엔진 (MassFitterV2, DCAFitter)
│   │   ├── Config/              # 설정 관리
│   │   ├── Managers/            # 매니저 클래스
│   │   ├── Utils/               # 유틸리티
│   │   └── Parameters/          # JSON 파라미터
│   │       ├── pp/
│   │       └── PbPb/
│   │
│   ├── 🔵 PP/                   # pp 충돌 분석
│   │   ├── DStarAnalysisPP.cpp
│   │   ├── ConfigPP.h
│   │   ├── scripts/
│   │   └── results/
│   │
│   ├── 🔴 PbPb/                 # PbPb 충돌 분석
│   │   ├── DStarAnalysisPbPb.cpp
│   │   ├── ConfigPbPb.h
│   │   ├── scripts/
│   │   └── results/
│   │
│   └── 🧪 Tests/                # 테스트 코드
│
├── 🔧 Preprocessing/            # 데이터 전처리
│   ├── Common/                  # 공통 전처리 (RDS Maker, Flattener)
│   ├── PP/
│   └── PbPb/
│
├── 🛠️ Tools/                    # 공통 도구
│   ├── Kinematics/              # 좌표계 변환
│   ├── Utils/                   # 유틸리티 함수
│   ├── Plotting/                # CMS 스타일 플로팅
│   └── Base/                    # 기본 클래스
│
├── 💾 Data/                     # 데이터 파일
│   ├── Raw/                     # 원본 ROOT 파일
│   │   ├── PP/{Data,MC}
│   │   └── PbPb/{Data,MC}
│   ├── Skimmed/                 # Skim된 Flat 파일
│   │   ├── PP/{Data,MC}/{D0,DStar}
│   │   └── PbPb/{Data,MC}/{D0,DStar}
│   ├── RDS/                     # RooDataSet 파일
│   │   ├── PP/{Data,MC}
│   │   └── PbPb/{Data,MC}
│   └── Auxiliary/               # 보조 데이터 (효율, 보정)
│
├── 📈 Results/                  # 분석 결과 (gitignore)
│   ├── PP/{Fits,Plots,Tables}
│   └── PbPb/{Fits,Plots,Tables}
│
├── 📖 Documentation/            # 문서
│   ├── README_OLD.md
│   ├── CHANGELOG.md
│   └── UserGuide.md
│
└── 🔨 Scripts/                  # 실행 스크립트
    ├── run_full_pp_pipeline.sh
    ├── run_full_pbpb_pipeline.sh
    └── utilities/               # 유틸리티 스크립트
```

## 🚀 빠른 시작

### 1. 마이그레이션 (처음 한 번만)

```bash
cd /home/jun502s/DstarAna/DStarAnalysis

# 실행 권한 부여
chmod +x migrate_to_new_structure.sh
chmod +x Scripts/**/*.sh

# 마이그레이션 실행 (백업 자동 생성)
./migrate_to_new_structure.sh

# 검증
./Scripts/utilities/verify_migration.sh
```

### 2. PP 분석

```bash
# PP 분석 실행
cd Analysis/PP
root -l -b -q DStarAnalysisPP.cpp

# 결과 확인
ls -la ../../Results/PP/
```

### 3. PbPb 분석

```bash
# PbPb 분석 실행
cd Analysis/PbPb
root -l -b -q DStarAnalysisPbPb.cpp

# 결과 확인
ls -la ../../Results/PbPb/
```

## ✨ 기능

### 분석 프레임워크

- **MassFitterV2**: 개선된 질량 피팅 엔진
- **DCAFitter**: DCA 템플릿 피팅
- **다양한 PDF**: Gaussian, Crystal Ball, Double Crystal Ball, Voigtian 등
- **배경 모델**: Exponential, Chebychev, Phenomenological 등
- **자동 파라미터 로딩**: JSON 기반 설정

### 데이터 처리

- **DStarRDSMakerImproved**: 개선된 RooDataSet 생성기
  - 버그 수정 (중복 계산, 메모리 누수)
  - 효율성 보정 지원
  - 다중 참조 프레임 (CS, HX, EP)
- **FlexibleFlattener**: 유연한 데이터 평탄화
- **ImprovedDataProcessor**: 향상된 데이터 처리

### 분석 도구

- **Kinematics**: Helicity, Collins-Soper, Event Plane 변환
- **Plotting**: CMS 스타일 플로팅 도구
- **Utilities**: 파일 관리, 중앙성 처리 등

## 🔄 마이그레이션

### 기존 구조의 문제점

```
❌ Macro/fit/testRun/DStarAnalysisV2forpp.cpp  (혼재)
❌ Macro/fit/testRun/DStarAnalysisV2forPbPb.cpp (혼재)
❌ Macro/fit/*_backup.h                         (백업 파일 산재)
❌ Data/여러/위치/에/분산                        (데이터 분산)
```

### 새 구조의 장점

```
✅ Analysis/PP/DStarAnalysisPP.cpp              (명확한 분리)
✅ Analysis/PbPb/DStarAnalysisPbPb.cpp          (명확한 분리)
✅ 백업 파일 정리                                (Git history 사용)
✅ Data/충돌시스템/타입/입자                     (계층적 구조)
```

### 마이그레이션 가이드

자세한 내용은 [`MIGRATION_GUIDE.md`](MIGRATION_GUIDE.md) 참조.

## 📖 사용법

### PP 분석 예제

```cpp
// Analysis/PP/DStarAnalysisPP.cpp
#include "../../Common/Core/MassFitterV2.h"
#include "../../Common/Config/DStarFitConfig.h"
#include "../ConfigPP.h"

void DStarAnalysisPP() {
    // 설정
    DStarFitOpt config;
    config.SetDataFilePath("../../Data/RDS/PP/Data/DStar.root");
    config.SetMCFilePath("../../Data/RDS/PP/MC/DStar.root");
    
    // 분석 실행
    // ...
}
```

### 파라미터 설정

```json
// Analysis/Common/Parameters/pp/signal/dbcb.json
{
  "pT_10_15": {
    "signal": {
      "mean": 0.1455,
      "sigma": 0.001,
      "alphaLow": 1.5,
      "nLow": 3.0
    }
  }
}
```

## 📚 문서

- **[빠른 시작](QUICK_START.md)**: 바로 시작하기
- **[마이그레이션 가이드](MIGRATION_GUIDE.md)**: 상세 마이그레이션 절차
- **[API 문서](Documentation/)**: 클래스 및 함수 설명
- **[변경 로그](Documentation/CHANGELOG.md)**: 버전별 변경사항

## 🛠️ 개발

### 새 기능 추가

```bash
# PP 분석에 새 기능
cd Analysis/PP
vim MyNewFeature.cpp

# 공통 코드 수정 (모든 분석에 영향)
cd ../Common/Core
vim NewModule.h
```

### 테스트

```bash
# 테스트 실행
cd Analysis/Tests
root -l -b -q TestMassFitter.cpp
```

## 🤝 기여

1. 백업 생성
2. 기능 브랜치 생성
3. 변경 사항 커밋
4. 테스트 실행
5. Pull Request

## 📝 라이선스

[여기에 라이선스 정보 추가]

## 👥 저자

[여기에 저자 정보 추가]

## 🙏 감사의 말

- ROOT 및 RooFit 프레임워크
- CMS 실험
- [기타 기여자들]

---

<div align="center">

**마지막 업데이트**: 2025년 10월

**질문이나 문제**: [GitHub Issues](링크) 또는 이메일

</div>

