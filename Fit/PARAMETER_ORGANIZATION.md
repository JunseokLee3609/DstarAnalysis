# Parameter Organization Summary

## ✓ 완료된 작업

### 1. 디렉토리 구조 개선

```
Fit/
├── PP/
│   ├── Parameters/                              ← NEW
│   │   ├── dstar_parameters_DBCrystalBall_Phenomenological2_PP.json    (새 생성)
│   │   ├── dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v1.json
│   │   ├── dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v2.json
│   │   └── [다른 기존 PP 파라미터]
│   ├── GenerateParameters_PP.py                 ← NEW (전용 생성기)
│   ├── DStarAnalysisPP.cpp
│   ├── runConfig.sh                            (경로 업데이트: Parameters/)
│   └── ...
│
├── PbPb/
│   ├── Parameters/                              ← NEW
│   │   ├── dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json  (새 생성)
│   │   ├── dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_v1.json
│   │   ├── dstar_parameters_DBCrystalBall_DstD0_dca_pbpb_v1.json
│   │   └── [다른 기존 PbPb 파라미터]
│   ├── GenerateParameters_PbPb.py               ← NEW (전용 생성기)
│   ├── DStarAnalysisPbPb.cpp
│   ├── runConfig.sh                            (경로 업데이트: Parameters/)
│   └── ...
│
└── Common/
    ├── Core/
    │   └── PDFFactory.h                        (Phenomenological2 개선)
    ├── Analysis/
    └── Parameters/                              (원본 유지, 참고용)
        ├── PP/
        ├── PbPb/
        └── GenerateJSONWithDCA.py             (공용 생성기)
```

### 2. 파라미터 파일 이동

**PP (3개 파일 복사)**
- `Common/Parameters/PP/*` → `PP/Parameters/`
- dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp.json
- dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v1.json
- dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v2.json

**PbPb (13개 파일 복사)**
- `Common/Parameters/PbPb/*` → `PbPb/Parameters/`
- dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_v1.json
- dstar_parameters_DBCrystalBall_DstD0_dca_pbpb_v1.json
- [그 외 기존 파일들]

### 3. 전용 파라미터 생성기 생성

**PP용 생성기: GenerateParameters_PP.py**
```bash
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PP
python3 GenerateParameters_PP.py \
  --pt-bins "5:7,7:10,10:15,15:20" \
  -o Parameters/dstar_parameters_DBCrystalBall_Phenomenological2_PP.json
```
- pT 그리드만 생성 (4개 bin)
- Phenomenological2 배경만 사용

**PbPb용 생성기: GenerateParameters_PbPb.py**
```bash
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb
python3 GenerateParameters_PbPb.py \
  --pt-bins "5:7,7:10,10:15,15:20" \
  --cent-bins "0:10,10:30,30:50,50:90" \
  -o Parameters/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json
```
- pT × Centrality 2D 그리드 생성 (4 × 4 = 16개 bin)
- Phenomenological2 배경만 사용

### 4. runConfig.sh 경로 업데이트

**PP runConfig.sh**
- 이전: `PARAM_DIR="../Common/Parameters/pp"`
- 현재: `PARAM_DIR="Parameters"`
- 파일 참조: `dstar_parameters_DBCrystalBall_Phenomenological2_PP.json`

**PbPb runConfig.sh**
- 이전: `PARAM_DIR="Parameters/PbPb"`
- 현재: `PARAM_DIR="Parameters"`
- 파일 참조: `dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json`

## 🚀 사용 방법

### PP 분석
```bash
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PP
bash runConfig.sh 1  # Quick test (새 파라미터 사용)
```

### PbPb 분석
```bash
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb
bash runConfig.sh 1  # Quick test (새 파라미터 사용)
```

### 파라미터 재생성 (필요시)
```bash
# PP
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PP
python3 GenerateParameters_PP.py

# PbPb
cd /home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb
python3 GenerateParameters_PbPb.py
```

## 📊 생성된 파라미터 구조

**PP 파일**: 4개 bin
```json
{
  "pT_5.0_7.0": {
    "kinematic": {"pT_min": 5.0, "pT_max": 7.0},
    "signal": {...},
    "background": {...},
    "background_type": "Phenomenological2",
    "yields": {...}
  },
  ...
}
```

**PbPb 파일**: 16개 bin
```json
{
  "pT_5.0_7.0_cent_0_10": {
    "kinematic": {
      "pT_min": 5.0, "pT_max": 7.0,
      "centrality_min": 0.0, "centrality_max": 10.0
    },
    "signal": {...},
    "background": {...},
    "background_type": "Phenomenological2",
    "yields": {...}
  },
  ...
}
```

## 💡 개선 사항

1. **명확한 분리**: PP와 PbPb 분석이 완전히 독립적 파라미터 사용
2. **빠른 재생성**: 각 분석 폴더에서 직접 생성기 실행
3. **버전 관리 용이**: `_PP`, `_PbPb` 접미사로 파일 구분
4. **유지보수 편의**: 공용 생성기(Common)는 참고용으로 유지

## ✅ 검증

```bash
# PP parameters
ls -1 /home/jun502s/DstarAna/DStarAnalysis/Fit/PP/Parameters/
  dstar_parameters_DBCrystalBall_Phenomenological2_PP.json
  dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v1.json
  dstar_parameters_DBCrystalBall_Phenomenological2_dca_pp_v2.json

# PbPb parameters
ls -1 /home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Parameters/
  dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json
  dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_v1.json
  dstar_parameters_DBCrystalBall_DstD0_dca_pbpb_v1.json
  ...
```

상태: ✅ **완료** - 모든 파라미터 구조 정리 및 생성기 분리 완료!
