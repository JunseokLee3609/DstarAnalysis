# 🔄 롤백(복구) 가이드

## ⚠️ 마이그레이션 전 필수 확인사항

### 1. Git 상태 확인
```bash
cd /home/jun502s/DstarAna/DStarAnalysis

# 현재 변경사항 확인
git status

# 커밋되지 않은 변경사항이 있다면 커밋 또는 stash
git add .
git commit -m "Before migration - backup commit"

# 또는
git stash save "Before migration"
```

### 2. 수동 백업 생성 (추가 안전장치)
```bash
# 전체 디렉토리 백업 (타임스탬프 포함)
cd /home/jun502s/DstarAna/
tar -czf DStarAnalysis_manual_backup_$(date +%Y%m%d_%H%M%S).tar.gz DStarAnalysis/

# 백업 위치 확인
ls -lh DStarAnalysis_manual_backup_*.tar.gz
```

### 3. 중요한 파일만 백업 (빠른 백업)
```bash
cd /home/jun502s/DstarAna/DStarAnalysis

# 분석 코드만 백업
tar -czf backup_analysis_code_$(date +%Y%m%d_%H%M%S).tar.gz Macro/fit/testRun/*.cpp

# 데이터 경로 파일 백업 (용량이 크지 않음)
cp -r Macro/fit/parameters/ backup_parameters_$(date +%Y%m%d_%H%M%S)/
```

## 🔙 롤백 방법

### 방법 1: 스크립트 자동 백업에서 복원 (가장 쉬움)

```bash
# 1. 현재 디렉토리 이름 변경
cd /home/jun502s/DstarAna/
mv DStarAnalysis DStarAnalysis_failed_$(date +%Y%m%d_%H%M%S)

# 2. 백업에서 복원
# 백업 디렉토리 이름 확인
ls -ld DStarAnalysis_backup_*

# 가장 최근 백업으로 복원
LATEST_BACKUP=$(ls -td DStarAnalysis_backup_* | head -1)
echo "Restoring from: $LATEST_BACKUP"
cp -r "$LATEST_BACKUP" DStarAnalysis

# 3. 확인
cd DStarAnalysis
ls -la
```

### 방법 2: Git으로 복원 (Git 사용 시)

```bash
cd /home/jun502s/DstarAna/DStarAnalysis

# 모든 변경사항 취소
git reset --hard HEAD

# Stash한 경우 복원
git stash pop

# 특정 커밋으로 돌아가기
git log --oneline  # 커밋 해시 확인
git reset --hard <commit-hash>
```

### 방법 3: 수동 백업에서 복원

```bash
cd /home/jun502s/DstarAna/

# tar.gz 백업 목록 확인
ls -lh DStarAnalysis_manual_backup_*.tar.gz

# 복원할 백업 선택 후 압축 해제
tar -xzf DStarAnalysis_manual_backup_YYYYMMDD_HHMMSS.tar.gz

# 기존 디렉토리 이름 변경 후 복원
mv DStarAnalysis DStarAnalysis_failed
mv DStarAnalysis_restored/DStarAnalysis ./
```

### 방법 4: 부분 복원 (특정 파일만)

```bash
# 백업에서 특정 파일만 복원
BACKUP_DIR="DStarAnalysis_backup_20251014_153045"

# 특정 파일 복원
cp "$BACKUP_DIR/Macro/fit/testRun/DStarAnalysisV2forpp.cpp" \
   DStarAnalysis/Macro/fit/testRun/

# 특정 폴더 복원
cp -r "$BACKUP_DIR/Macro/fit/parameters/" \
      DStarAnalysis/Macro/fit/
```

## 🧪 안전한 마이그레이션 테스트 방법

### 1. 테스트 환경에서 먼저 실행
```bash
# 복사본 생성
cd /home/jun502s/DstarAna/
cp -r DStarAnalysis DStarAnalysis_test

# 테스트 환경에서 마이그레이션 실행
cd DStarAnalysis_test
./migrate_to_new_structure.sh

# 문제 없으면 실제 환경에서 실행
```

### 2. Dry-run (실행 시뮬레이션)
```bash
# 마이그레이션 스크립트를 수정하여 dry-run 모드 추가
# 실제로 파일을 복사하지 않고 명령어만 출력
```

## 📋 롤백 체크리스트

마이그레이션 전:
- [ ] Git 커밋 또는 stash 완료
- [ ] 수동 백업 생성 완료 (.tar.gz)
- [ ] 중요 파일 별도 백업
- [ ] 백업 파일 크기 확인 (손상 여부)
- [ ] 디스크 공간 충분한지 확인 (최소 10GB 권장)

롤백 시:
- [ ] 현재 디렉토리 백업 (실패한 버전도 보관)
- [ ] 백업 파일 무결성 확인
- [ ] 복원 후 주요 파일 존재 여부 확인
- [ ] 복원 후 분석 코드 실행 테스트

## 🆘 긴급 복구 스크립트

급하게 복구가 필요한 경우:

```bash
cat > emergency_rollback.sh << 'EOF'
#!/bin/bash
set -e

echo "🚨 Emergency Rollback Starting..."

# Find latest backup
LATEST_BACKUP=$(ls -td /home/jun502s/DstarAna/DStarAnalysis_backup_* | head -1)

if [ -z "$LATEST_BACKUP" ]; then
    echo "❌ No backup found!"
    exit 1
fi

echo "📦 Found backup: $LATEST_BACKUP"

# Backup current state
echo "💾 Backing up current (failed) state..."
mv /home/jun502s/DstarAna/DStarAnalysis \
   /home/jun502s/DstarAna/DStarAnalysis_failed_$(date +%Y%m%d_%H%M%S)

# Restore from backup
echo "♻️  Restoring from backup..."
cp -r "$LATEST_BACKUP" /home/jun502s/DstarAna/DStarAnalysis

echo "✅ Rollback complete!"
echo "Restored from: $LATEST_BACKUP"
EOF

chmod +x emergency_rollback.sh
```

실행:
```bash
./emergency_rollback.sh
```

## 💡 복구 후 확인 사항

```bash
# 1. 디렉토리 구조 확인
ls -la Macro/fit/testRun/

# 2. 중요 파일 존재 확인
test -f Macro/fit/testRun/DStarAnalysisV2forpp.cpp && echo "✓ PP file OK"
test -f Macro/fit/testRun/DStarAnalysisV2forPbPb.cpp && echo "✓ PbPb file OK"
test -f Macro/skim/DStarRDSMakerImproved.cpp && echo "✓ RDS Maker OK"

# 3. 분석 코드 컴파일 테스트
cd Macro/fit/testRun/
root -l -b -q 'DStarAnalysisV2forpp.cpp++' > /dev/null 2>&1 && echo "✓ Compilation OK"

# 4. Git 상태 확인 (Git 사용 시)
git status
```

## 📞 문제 해결

### Q: 백업이 손상되었습니다
```bash
# tar.gz 파일 무결성 검사
tar -tzf backup_file.tar.gz > /dev/null && echo "OK" || echo "Corrupted"

# Git history에서 복구
git reflog
git checkout <commit-hash>
```

### Q: 디스크 공간이 부족합니다
```bash
# 용량 확인
df -h /home/jun502s/DstarAna/

# 불필요한 백업 삭제
rm -rf DStarAnalysis_backup_20251001*  # 오래된 백업만

# 큰 파일 찾기
find . -type f -size +100M -exec ls -lh {} \;
```

### Q: 일부 파일만 복원하고 싶습니다
```bash
# 백업에서 특정 디렉토리만 복원
cp -r DStarAnalysis_backup_*/Macro/fit/ DStarAnalysis/Macro/

# 또는 Git에서
git checkout HEAD -- Macro/fit/
```

## 🎯 권장 복구 순서

1. **침착하게 상황 파악**
   - 어떤 파일이 문제인지 확인
   - 전체 복구가 필요한지, 부분 복구로 충분한지 판단

2. **현재 상태 보존**
   - 실패한 상태도 백업 (나중에 비교 가능)

3. **백업 확인**
   - 백업 파일이 손상되지 않았는지 확인

4. **복원 실행**
   - 위의 방법 중 하나 선택

5. **검증**
   - 복원 후 주요 기능 테스트

6. **문서화**
   - 무엇이 잘못되었는지 기록
   - 다음에 같은 실수 방지

## 📝 복구 로그 템플릿

```
날짜: __________
문제: __________
시도한 해결책: __________
사용한 백업: __________
복구 방법: __________
결과: __________
교훈: __________
```

