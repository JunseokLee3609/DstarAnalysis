# 빠른 시작 가이드 (Quick Start)

## 30초 안에 테스트하기

### 단계 1: ROOT 6.24 소싱
```bash
ROOT6.24
```

### 단계 2: 테스트 디렉토리로 이동
```bash
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Test/cosThetaCorrection/
```

### 단계 3: 테스트 실행
```bash
root test_HistBasedCorrection.C
```

**끝!** 결과는 `splot/<subDir>_cent0to10/` 디렉토리에 저장됩니다.

---

## 무엇이 실행되나?

이 테스트는 다음을 수행합니다:

1. **효율 맵 구성** (Efficiency Map Building)
   - MC 데이터셋 (PbPb_MC_Oct22)에서 Prompt/Nonprompt 효율 계산
   - Centrality 0-10% 범위로 필터링
   - 결과: `efficiency_map_*.png`

2. **Yield 보정** (Yield Correction)
   - Data 분석 결과에 효율 맵 적용
   - 각 pT/cos θ 빈에 대해 개별 보정
   - Prompt fraction = 1.0 (비활성화)

3. **ρ₀₀ 추출** (rho00 Extraction)
   - 보정된 yield로부터 핵 스핀 정렬 파라미터 추출
   - 함수 형태: `N₀(1 - ρ₀₀ + (3ρ₀₀ - 1)cos²θ)`
   - 결과: `rho00_vs_pT_prompt_nonprompt.png`

4. **요약 저장** (Summary Export)
   - 모든 결과를 CSV 파일로 저장
   - 결과: `yield_summary.csv`

---

## 출력 파일 설명

| 파일명 | 설명 |
|--------|------|
| `efficiency_map_prompt.png` | Prompt D* 효율 맵 (pT vs cos θ) |
| `efficiency_map_nonprompt.png` | Nonprompt D* 효율 맵 |
| `splot_prompt_corrected_yield_pt_*.png` | Prompt 보정된 yield |
| `splot_nonprompt_corrected_yield_pt_*.png` | Nonprompt 보정된 yield |
| `rho00_vs_pT_prompt_nonprompt.png` | ρ₀₀ vs pT 그래프 |
| `yield_summary.csv` | Centrality 0-10% 모든 결과 요약 |

---

## 다른 Centrality 범위로 테스트

### Centrality 30-50% 테스트
ROOT 대화형 모드에서:
```bash
root -l
root [0] .L HistBasedCorrection.cpp+
root [1] HistBasedCorrectionWithCentrality(false, true, 30, 50, "PbPb_MC_Oct22")
```

결과는 `splot/<subDir>_cent30to50/` 디렉토리에 저장됩니다.

---

## 주요 기능

✅ **Centrality-dependent 보정**
- 각 centrality 범위별로 별도 효율 맵 구성
- 중심 충돌도 의존성 포함

✅ **JSON Catalog 기반 파일 관리**
- MC 데이터셋 자동 로드
- `/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json` 참조

✅ **Prompt Fraction 비활성화**
- 현재 모든 yield는 prompt fraction = 1.0
- Prompt/Nonprompt 분리 없음

✅ **자동 플롯 생성**
- 모든 중간 결과 PNG로 저장
- 고해상도 분석 결과

---

## 문제 해결

### 컴파일 오류
```
error: HistBasedCorrection.cpp: No such file or directory
```
→ 현재 디렉토리 확인: `/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Test/cosThetaCorrection/`

### MC 파일 못 찾음
```
Error loading MC dataset: PbPb_MC_Oct22 from catalog ...
```
→ JSON catalog 확인:
```bash
cat /home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json
```

### 메모리 부족
→ ROOT 세션 재시작 후 다시 실행

---

## 다음 단계

테스트 완료 후:

1. **결과 검토**
   - `splot/<subDir>_cent0to10/` 디렉토리의 PNG 파일 확인
   - `yield_summary.csv` 데이터 검증

2. **다른 Centrality 범위 분석**
   - 0-10%, 10-20%, ... 등 차례대로 실행
   - 효율의 centrality 의존성 확인

3. **파라미터 최적화**
   - pT 빈 조정
   - cos θ 빈 조정
   - 필요시 코드 수정

---

## 더 자세한 정보

자세한 설명은 `TEST_GUIDE.md` 참조
