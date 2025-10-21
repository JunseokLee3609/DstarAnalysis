# 🚀 Quick Start Guide

## 바로 시작하기

### 1. 마이그레이션 실행 (처음 한 번만)

```bash
cd /home/jun502s/DstarAna/DStarAnalysis

# 스크립트 실행 권한 부여
chmod +x migrate_to_new_structure.sh
chmod +x Scripts/utilities/*.sh
chmod +x Scripts/*.sh

# 마이그레이션 실행 (백업 자동 생성)
./migrate_to_new_structure.sh
```

### 2. 마이그레이션 검증

```bash
# 마이그레이션이 제대로 되었는지 확인
./Scripts/utilities/verify_migration.sh
```

### 3. PP 분석 실행

```bash
# ROOT 환경 설정
source /path/to/root/bin/thisroot.sh

# PP 분석 실행
cd Analysis/PP/scripts
root -l -b -q ../DStarAnalysisPP.cpp
```

### 4. PbPb 분석 실행

```bash
# PbPb 분석 실행
cd Analysis/PbPb/scripts
root -l -b -q ../DStarAnalysisPbPb.cpp
```

## 📂 새로운 디렉토리 구조

```
DStarAnalysis/
├── Analysis/          # 🎯 분석 코드 (여기서 작업)
│   ├── Common/        # 공통 코드
│   ├── PP/            # pp 분석
│   └── PbPb/          # PbPb 분석
│
├── Preprocessing/     # 전처리
├── Tools/             # 도구
├── Data/              # 데이터 (충돌 시스템별 정리)
├── Results/           # 결과
└── Scripts/           # 유틸리티 스크립트
```

## 🔧 주요 명령어

### 코드 검색
```bash
# PP 분석 코드 찾기
cd Analysis/PP

# PbPb 분석 코드 찾기
cd Analysis/PbPb

# 공통 코드 찾기
cd Analysis/Common
```

### 백업 파일 정리
```bash
# 백업 파일 목록 확인
./Scripts/utilities/cleanup_backup_files.sh
```

### Include 경로 업데이트
```bash
# 자동 업데이트
./Scripts/utilities/update_includes.sh
```

## 📊 데이터 파일 위치

### PP 데이터
```
Data/
├── Skimmed/PP/Data/     # Skim된 pp 데이터
├── RDS/PP/Data/         # RooDataSet pp 데이터
└── RDS/PP/MC/           # RooDataSet pp MC
```

### PbPb 데이터
```
Data/
├── Skimmed/PbPb/Data/   # Skim된 PbPb 데이터
├── RDS/PbPb/Data/       # RooDataSet PbPb 데이터
└── RDS/PbPb/MC/         # RooDataSet PbPb MC
```

## ⚡ 빠른 팁

### 1. 새 분석 추가하기
```bash
# PP 분석에 새 기능 추가
cd Analysis/PP
cp DStarAnalysisPP.cpp MyNewAnalysisPP.cpp
# 수정...
```

### 2. 공통 코드 수정
```bash
# 모든 분석에 영향을 주는 수정
cd Analysis/Common/Core
vim MassFitterV2.h
```

### 3. 파라미터 변경
```bash
# PP 파라미터
vim Analysis/Common/Parameters/pp/signal/*.json

# PbPb 파라미터
vim Analysis/Common/Parameters/PbPb/signal/*.json
```

## 🆘 문제 해결

### Q: Include 에러가 발생합니다
```bash
# Include 경로 자동 업데이트
./Scripts/utilities/update_includes.sh
```

### Q: 데이터 파일을 찾을 수 없습니다
```bash
# 심볼릭 링크 확인
ls -la Data/RDS/PP/Data/
ls -la Data/Skimmed/PP/Data/
```

### Q: 마이그레이션을 취소하고 싶습니다
```bash
# 백업에서 복원
mv DStarAnalysis DStarAnalysis_failed
cp -r DStarAnalysis_backup_YYYYMMDD_HHMMSS DStarAnalysis
```

## 📚 더 자세한 정보

- 상세 가이드: `MIGRATION_GUIDE.md`
- 문서: `Documentation/` 폴더
- 기존 문서: `Documentation/README_OLD.md`

## 🎓 예제 워크플로우

### 전체 PP 분석 파이프라인
```bash
# 1. 데이터 전처리 (필요시)
cd Preprocessing/Common
root -l -b -q 'DStarRDSMakerImproved.cpp("pp")'

# 2. 분석 실행
cd ../../Analysis/PP
root -l -b -q DStarAnalysisPP.cpp

# 3. 결과 확인
cd ../../Results/PP/Plots
ls -la
```

### 전체 PbPb 분석 파이프라인
```bash
# 1. 데이터 전처리 (필요시)
cd Preprocessing/Common
root -l -b -q 'DStarRDSMakerImproved.cpp("PbPb")'

# 2. 분석 실행
cd ../../Analysis/PbPb
root -l -b -q DStarAnalysisPbPb.cpp

# 3. 결과 확인
cd ../../Results/PbPb/Plots
ls -la
```

## 💡 유용한 명령어

```bash
# 전체 구조 확인
tree -L 3 -d

# PP 관련 파일만 찾기
find Analysis/PP -name "*.cpp" -o -name "*.h"

# PbPb 관련 파일만 찾기
find Analysis/PbPb -name "*.cpp" -o -name "*.h"

# 특정 함수/클래스 찾기
grep -r "MassFitterV2" Analysis/

# 최근 수정된 파일
find Analysis/ -type f -mtime -7  # 7일 이내

# 디스크 사용량 확인
du -sh Data/*
```

## 🔄 업데이트 체크리스트

마이그레이션 후 확인할 사항:
- [ ] 새 구조 생성 확인
- [ ] PP 분석 실행 성공
- [ ] PbPb 분석 실행 성공
- [ ] 결과 파일 생성 확인
- [ ] 백업 파일 정리 완료
- [ ] Git commit 완료

