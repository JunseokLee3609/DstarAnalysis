# PbPb RDS Maker

PbPb 충돌 데이터를 위한 DStarRDSMaker Improved 버전

## 📁 파일 구조

```
PbPb/
├── RunConfig_RDS.sh              # 기존 스크립트 (참고용)
├── RunConfig_RDS_Improved.sh      # 개선된 전체 실행 스크립트
├── run_pbpb_examples.sh          # 사용 예시 스크립트
├── Data/                         # PbPb 데이터 저장 폴더
└── README.md                     # 이 파일
```

## 🚀 빠른 시작

### 1. 전체 PbPb 데이터 처리
```bash
# 모든 PbPb 설정으로 데이터 처리
./RunConfig_RDS_Improved.sh
```

### 2. 개별 예시 실행
```bash
# 대화형 예시 실행
./run_pbpb_examples.sh
```

## 📋 사용법

### RunConfig_RDS_Improved.sh
전체 PbPb 데이터를 자동으로 처리하는 스크립트입니다.

**처리 내용:**
- PbPb MC D* (기본 + 효율성 맵)
- PbPb Data D* (기본 + 효율성 맵)
- PbPb MC D0 (기본)
- PbPb Data D0 (기본)

**실행:**
```bash
./RunConfig_RDS_Improved.sh
```

### run_pbpb_examples.sh
대화형으로 다양한 설정을 선택하여 실행할 수 있습니다.

**사용 예시:**
1. **PbPb MC D* (기본)**
2. **PbPb Data D* (기본)**
3. **PbPb MC D0 (기본)**
4. **PbPb Data D0 (기본)**
5. **PbPb MC D* (효율성 맵 사용)**
6. **PbPb Data D* (효율성 맵 사용)**
7. **사용자 정의 입력 파일**
8. **모든 PbPb 데이터 처리**

**실행:**
```bash
./run_pbpb_examples.sh
```

## 📊 출력 파일

### 파일명 규칙
```
RDS_Physics_[MC/Data]_[DStar/D0]_PbPb[_suffix].root
```

### 데이터셋 구조
- `dataset`: 기본 데이터셋
- `datasetHX`: Helicity 프레임 (D*만)
- `datasetEP`: Event Plane 프레임 (D*만)

### 출력 디렉토리
- **기본 저장**: `/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/` (MC)
- **기본 저장**: `/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/` (Data)
- **PbPb 전용**: `/home/jun502s/DstarAna/DStarAnalysis/Preprocessing/PbPb/Data/` (복사본)

## 🎯 주요 기능

### 1. PbPb 특화 변수
- **centrality**: 중앙성 (0-200)
- **Centrality**: 중앙성/2 (0-100)
- **mva**: MVA 점수 (0.9-1.0)

### 2. 물리 프레임
- **Helicity**: HX 프레임 (D*만)
- **Event Plane**: EP 프레임 (D*만)
- **Collins-Soper**: CS 프레임 (비활성화)

### 3. 효율성 가중치
- TH3D 효율성 맵 지원
- Prompt/Non-Prompt 구분
- 자동 가중치 계산

## 📝 사용 예시

### ROOT 프롬프트에서 직접 실행
```bash
cd /home/jun502s/DstarAna/DStarAnalysis/Preprocessing/Common
root -l
```

```cpp
.L DStarRDSMaker.cpp

// PbPb MC D* 처리
DStarRDSMakerImproved(
    true,   // isMC
    false,  // isD0 (D*)
    false,  // isPP (PbPb)
    "/path/to/input.root",
    "suffix"
);

// PbPb Data D* 처리 (효율성 맵 사용)
DStarRDSMakerImproved(
    false,  // isMC (Data)
    false,  // isD0 (D*)
    false,  // isPP (PbPb)
    "/path/to/input.root",
    "suffix",
    "/path/to/efficiency.root",
    "efficiency_map_name"
);
```

### 스크립트 실행
```bash
# 전체 처리
./RunConfig_RDS_Improved.sh

# 개별 처리
./run_pbpb_examples.sh
```

## 🔧 설정 옵션

### 입력 파일 경로
- **MC D***: `/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbMC/DStar/`
- **Data D***: `/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbData/`
- **MC D0**: `/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbMC/D0/`
- **Data D0**: `/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbData/D0/`

### 효율성 맵 파일
- **Prompt**: `pt_y_cos_pr_pass_ratio`
- **Non-Prompt**: `pt_y_cos_np_pass_ratio`

## 🐛 문제 해결

### ROOT 환경 확인
```bash
# ROOT 버전 확인
root-config --version

# 환경 설정
source /path/to/root/bin/thisroot.sh
```

### 파일 권한 확인
```bash
# 실행 권한 부여
chmod +x *.sh

# 파일 확인
ls -la *.sh
```

### 메모리 부족
- 큰 파일 처리 시 메모리 부족 가능
- `maxEntries` 옵션으로 이벤트 수 제한

## 📈 성능 최적화

### 1. 병렬 처리
```bash
# 백그라운드 실행
nohup ./RunConfig_RDS_Improved.sh > pbpb_processing.log 2>&1 &
```

### 2. 메모리 관리
- 큰 파일은 청크 단위로 처리
- 불필요한 변수 제거
- 효율성 맵 최적화

### 3. 디스크 공간
- 출력 파일 크기 모니터링
- 임시 파일 정리
- 압축 저장 고려

## 🔄 버전 관리

- **v1.0**: 기본 PbPb 처리
- **v2.0**: Improved 버전 (현재)
  - 개선된 에러 처리
  - 체계적인 파일 관리
  - 효율성 맵 지원

## 📞 지원

문제가 발생하면 다음을 확인하세요:
1. ROOT 환경 설정
2. 입력 파일 경로
3. 출력 디렉토리 권한
4. 메모리 사용량
