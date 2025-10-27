# HistBasedCorrection 테스트 가이드 (Centrality 0-10%)

## 테스트 환경 설정

### 1. ROOT 소싱
먼저 ROOT 6.24를 소싱하세요:

```bash
source ROOT6.24
```

또는 alias가 설정되어 있다면:

```bash
ROOT6.24
```

### 2. 테스트 디렉토리로 이동
```bash
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Test/cosThetaCorrection/
```

## 테스트 실행 방법

### 방법 1: ROOT 매크로 사용 (권장)
```bash
root test_HistBasedCorrection.C
```

이 매크로는 다음을 수행합니다:
- HistBasedCorrection.cpp 컴파일 및 로드
- Centrality 0-10% 범위로 분석 실행
- MC 데이터셋: PbPb_MC_Oct22 (JSON catalog에서 로드)
- 완료 후 결과 파일 위치 출력

### 방법 2: ROOT 대화형 모드
```bash
root -l
root [0] .L HistBasedCorrection.cpp+
root [1] HistBasedCorrectionWithCentrality(false, true, 0, 10, "PbPb_MC_Oct22")
```

### 방법 3: 다른 centrality 범위로 테스트
예: Centrality 30-50%
```bash
root -l
root [0] .L HistBasedCorrection.cpp+
root [1] HistBasedCorrectionWithCentrality(false, true, 30, 50, "PbPb_MC_Oct22")
```

## 예상 출력

테스트 실행 시 다음과 같은 로그가 출력됩니다:

```
========================================
HistBasedCorrection 테스트 시작
Centrality: 0-10%
========================================

[INFO] HistBasedCorrection.cpp 로드 완료

[PARAMS] 분석 설정:
  - isMC: false
  - useAbsTheta: true
  - Centrality: 0-10%
  - MC Dataset: PbPb_MC_Oct22

[START] HistBasedCorrectionWithCentrality 실행 중...

[INFO] Using analysis output base directory: ...
[INFO] Loaded MC file: ...
[MC GEN] Processing ... GEN entries from ...
Built Prompt efficiency map for centrality 0-10%
Built Nonprompt efficiency map for centrality 0-10%
...
========================================
✓ 테스트 완료!
========================================

[OUTPUT] 결과 파일 위치:
  - splot/<subDir>_cent0to10/
  - efficiency_map_*.png
  - splot_*_corrected_yield_pt_*.png
  - rho00_vs_pT_prompt_nonprompt.png
  - yield_summary.csv
```

## 생성되는 파일

테스트 완료 후 다음 디렉토리에 파일이 생성됩니다:

```
splot/<subDir>_cent0to10/
├── efficiency_map_prompt.png           # Prompt 효율 맵
├── efficiency_map_nonprompt.png        # Nonprompt 효율 맵
├── mc_gen/                             # MC 생성 입자 분석
│   ├── prompt/
│   └── nonprompt/
├── splot_prompt_raw_yield_pt_*.png     # Raw yield (Prompt)
├── splot_prompt_corrected_yield_pt_*.png  # 보정된 yield (Prompt)
├── splot_nonprompt_raw_yield_pt_*.png  # Raw yield (Nonprompt)
├── splot_nonprompt_corrected_yield_pt_*.png # 보정된 yield (Nonprompt)
├── rho00_vs_pT_prompt_nonprompt.png    # ρ₀₀ vs pT 그래프
├── rho00_vs_pT_prompt_nonprompt.pdf
└── yield_summary.csv                   # Yield 요약 (CSV 형식)
```

## 주요 설정값

현재 테스트에서 사용되는 설정:

| 파라미터 | 값 | 설명 |
|---------|-----|------|
| isMC | false | Data 분석 (MC 아님) |
| useAbsTheta | true | \|cos θ\| 사용 |
| Centrality Min | 0% | 최소 centrality |
| Centrality Max | 10% | 최대 centrality |
| MC Dataset | PbPb_MC_Oct22 | 효율 맵용 MC 데이터 |
| Prompt Fraction | 1.0 | 비활성화 (전체 yield 사용) |

## 문제 해결

### 컴파일 오류 발생
- ROOT 버전 확인: `root -v`가 6.24인지 확인
- `.L HistBasedCorrection.cpp+` 대신 `.L HistBasedCorrection.cpp++` 사용 가능

### 파일을 찾을 수 없음
- MC 데이터셋이 JSON catalog에 있는지 확인:
  ```bash
  cat /home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json
  ```
- "PbPb_MC_Oct22" 엔트리가 존재하는지 확인

### 메모리 부족
- 더 작은 pT 범위로 테스트 시도
- ROOT 세션 재시작

## 분석 파라미터 변경

### 다른 MC 데이터셋 사용
```bash
root [0] .L HistBasedCorrection.cpp+
root [1] HistBasedCorrectionWithCentrality(false, true, 0, 10, "PbPb_MC_Aug22")
```

### MC 분석으로 실행
```bash
root [0] .L HistBasedCorrection.cpp+
root [1] HistBasedCorrectionWithCentrality(true, true, 0, 10, "PbPb_MC_Oct22")
```

### cos θ (절대값 아님) 사용
```bash
root [0] .L HistBasedCorrection.cpp+
root [1] HistBasedCorrectionWithCentrality(false, false, 0, 10, "PbPb_MC_Oct22")
```

## 추가 정보

### Centrality 계산
- 입력 centrality 변수는 0-200 범위
- 내부적으로 `centralityPercent = centrality × 0.5`로 변환
- 따라서 centrality 변수가 0-20이면 0-10%에 해당

### 효율 맵 (Efficiency Map)
- MC 파일에서 GEN 입자와 RECO 입자를 사용하여 구성
- 각 centrality 범위별로 별도로 계산
- 2D 히스토그램: pT vs cos θ

### Yield 보정 (Correction)
- Raw yield를 efficiency map으로 나누어 보정
- 각 pT/cos θ 빈에 대해 개별적으로 수행
- Prompt fraction = 1.0 (비활성화)

## 추가 도움말

자세한 내용은 다음 파일을 참조하세요:
- `README_HistBasedCorrection_Updates.md`: 코드 변경 사항
- `HistBasedCorrection.cpp`: 전체 소스 코드
