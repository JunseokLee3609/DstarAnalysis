#!/bin/bash

# ========================================
# D* Analysis Repository Migration Script
# ========================================
# This script reorganizes the repository structure
# to separate PP and PbPb analyses clearly
# ========================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Base directory
BASE_DIR="/home/jun502s/DstarAna/DStarAnalysis"
BACKUP_DIR="${BASE_DIR}_backup_$(date +%Y%m%d_%H%M%S)"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}D* Analysis Repository Migration${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Step 0: Create backup
echo -e "${YELLOW}Step 0: Creating backup...${NC}"
echo "Backup location: ${BACKUP_DIR}"
read -p "Create backup? (y/n): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    cp -r "${BASE_DIR}" "${BACKUP_DIR}"
    echo -e "${GREEN}✓ Backup created${NC}\n"
else
    echo -e "${YELLOW}⚠ Skipping backup (not recommended)${NC}\n"
fi

cd "${BASE_DIR}"

# Step 1: Create new directory structure
echo -e "${YELLOW}Step 1: Creating new directory structure...${NC}"

mkdir -p Analysis/Common/{Core,Config,Managers,Utils}
mkdir -p Analysis/Common/Parameters/{pp/{signal,background},PbPb/{signal,background}}
mkdir -p Analysis/{PP/{scripts,results},PbPb/{scripts,results},Tests}
mkdir -p Preprocessing/{Common,PP/scripts,PbPb/scripts}
mkdir -p Tools/{Kinematics,Utils,Plotting,Base}
mkdir -p Data/Raw/{PP/{Data,MC},PbPb/{Data,MC}}
mkdir -p Data/Skimmed/{PP/{Data/{D0,DStar},MC/{D0,DStar}},PbPb/{Data/{D0,DStar},MC/{D0,DStar}}}
mkdir -p Data/RDS/{PP/{Data,MC},PbPb/{Data,MC}}
mkdir -p Data/Auxiliary/{EfficiencyMaps,CorrectionFactors,Templates}
mkdir -p Results/{PP/{Fits,Plots,Tables},PbPb/{Fits,Plots,Tables}}
mkdir -p Documentation
mkdir -p Scripts/utilities

echo -e "${GREEN}✓ Directory structure created${NC}\n"

# Step 2: Move Common Analysis Code
echo -e "${YELLOW}Step 2: Moving common analysis code...${NC}"

# Core analysis classes
echo "  Moving core classes..."
cp Macro/fit/MassFitterV2.h Analysis/Common/Core/
cp Macro/fit/DCAFitter.h Analysis/Common/Core/
cp Macro/fit/DataLoader.h Analysis/Common/Core/
cp Macro/fit/PDFFactory.h Analysis/Common/Core/
cp Macro/fit/PDFFactoryImpl.h Analysis/Common/Core/

# Config files
echo "  Moving config files..."
cp Macro/fit/DStarFitConfig.h Analysis/Common/Config/
cp Macro/fit/DStarFitOpt.h Analysis/Common/Config/
cp Macro/fit/FitCommonConfig.h Analysis/Common/Config/
cp Macro/fit/Opt.h Analysis/Common/Config/
cp Macro/fit/Params.h Analysis/Common/Config/

# Managers
echo "  Moving managers..."
cp Macro/fit/ParameterManager.h Analysis/Common/Managers/
cp Macro/fit/ResultManager.h Analysis/Common/Managers/
cp Macro/fit/EnhancedPlotManager.h Analysis/Common/Managers/
cp Macro/fit/ErrorHandler.h Analysis/Common/Managers/
cp Macro/fit/FitManager.h Analysis/Common/Managers/
cp Macro/fit/FlexibleParameterManager.h Analysis/Common/Managers/
cp Macro/fit/RobustParameterManager.h Analysis/Common/Managers/

# Utils
echo "  Moving utilities..."
cp Macro/fit/Helper.h Analysis/Common/Utils/
cp Macro/fit/JSONParameterUtils.h Analysis/Common/Utils/
cp Macro/fit/JSONParameterLoader.h Analysis/Common/Utils/
cp Macro/fit/SimpleParameterLoader.h Analysis/Common/Utils/
cp Macro/fit/ParameterDebugUtils.h Analysis/Common/Utils/
cp Macro/fit/FitStrategy.h Analysis/Common/Utils/

# Parameters
echo "  Moving parameter files..."
cp -r Macro/fit/parameters/*.json Analysis/Common/Parameters/
# Organize by collision system
mv Analysis/Common/Parameters/*pp*.json Analysis/Common/Parameters/pp/ 2>/dev/null || true
mv Analysis/Common/Parameters/*PbPb*.json Analysis/Common/Parameters/PbPb/ 2>/dev/null || true
mv Analysis/Common/Parameters/*pbpb*.json Analysis/Common/Parameters/PbPb/ 2>/dev/null || true

echo -e "${GREEN}✓ Common code moved${NC}\n"

# Step 3: Move PP-specific code
echo -e "${YELLOW}Step 3: Moving PP-specific analysis code...${NC}"

cp Macro/fit/testRun/DStarAnalysisV2forpp.cpp Analysis/PP/DStarAnalysisPP.cpp
cp Macro/Tools/ConfigManagerPP.h Analysis/PP/ConfigPP.h

# PP scripts
cp Macro/fit/testRun/condor_submit_DStarV2_pp.sub Analysis/PP/scripts/ 2>/dev/null || true
cp Macro/fit/testRun/runCondor_DStarV2.sh Analysis/PP/scripts/run_pp_analysis.sh 2>/dev/null || true

echo -e "${GREEN}✓ PP code moved${NC}\n"

# Step 4: Move PbPb-specific code
echo -e "${YELLOW}Step 4: Moving PbPb-specific analysis code...${NC}"

cp Macro/fit/testRun/DStarAnalysisV2forPbPb.cpp Analysis/PbPb/DStarAnalysisPbPb.cpp
cp Macro/Tools/ConfigManager.h Analysis/PbPb/ConfigPbPb.h

# PbPb scripts
cp Macro/fit/testRun/condor_submit_DStarV2.sub Analysis/PbPb/scripts/ 2>/dev/null || true

echo -e "${GREEN}✓ PbPb code moved${NC}\n"

# Step 5: Move Preprocessing code
echo -e "${YELLOW}Step 5: Moving preprocessing code...${NC}"

# Common preprocessing
cp Macro/skim/DStarRDSMakerImproved.cpp Preprocessing/Common/
cp Macro/skim/FlexibleFlattener.cpp Preprocessing/Common/
cp Macro/skim/ImprovedDataProcessor.cpp Preprocessing/Common/
cp Macro/skim/Flat.cpp Preprocessing/Common/

# Preprocessing scripts
cp Macro/skim/RunConfig.sh Preprocessing/scripts/ 2>/dev/null || true
cp Macro/skim/run.sh Preprocessing/scripts/ 2>/dev/null || true

echo -e "${GREEN}✓ Preprocessing code moved${NC}\n"

# Step 6: Reorganize Tools
echo -e "${YELLOW}Step 6: Reorganizing Tools directory...${NC}"

# Kinematics
cp Macro/Tools/Transformations.h Tools/Kinematics/
cp Macro/Tools/Parameters/PhaseSpace.h Tools/Kinematics/ 2>/dev/null || true

# Utils
cp Macro/Tools/FileManager.h Tools/Utils/ 2>/dev/null || true
cp Macro/Tools/CentralityUtils.h Tools/Utils/ 2>/dev/null || true
cp Macro/Tools/GlobalCuts.h Tools/Utils/ 2>/dev/null || true
cp Macro/Tools/Helper.h Tools/Utils/ 2>/dev/null || true
cp Macro/Tools/VariableHandler.h Tools/Utils/ 2>/dev/null || true

# Plotting
cp Macro/Tools/Style/CMS_lumi.h Tools/Plotting/ 2>/dev/null || true
cp Macro/Tools/Style/CMS_lumi.C Tools/Plotting/ 2>/dev/null || true
cp Macro/Tools/Style/tdrStyle.C Tools/Plotting/ 2>/dev/null || true
cp Macro/Tools/Style/ColorSchemes.h Tools/Plotting/ 2>/dev/null || true
cp Macro/Tools/Style/Legends.h Tools/Plotting/ 2>/dev/null || true

# Base
cp Macro/Tools/BasicHeaders.h Tools/Base/ 2>/dev/null || true
cp Macro/Tools/DataProcessorBase.h Tools/Base/ 2>/dev/null || true

echo -e "${GREEN}✓ Tools reorganized${NC}\n"

# Step 7: Reorganize Data directory
echo -e "${YELLOW}Step 7: Organizing Data directory...${NC}"
echo "  Note: Large data files should be moved manually if needed"
echo "  Creating symbolic links to existing data..."

# Link to existing RDS files
if [ -d "Data/RDS_Physics" ]; then
    ln -sf "$(pwd)/Data/RDS_Physics" Data/RDS/PP/Data/DStar_old
    ln -sf "$(pwd)/Data/RDS_Physics" Data/RDS/PbPb/Data/DStar_old
fi

if [ -d "Data/RDS_MC" ]; then
    ln -sf "$(pwd)/Data/RDS_MC" Data/RDS/PP/MC/DStar_old
    ln -sf "$(pwd)/Data/RDS_MC" Data/RDS/PbPb/MC/DStar_old
fi

# Link to skimmed data
if [ -d "Macro/skim/Data/FlatSample" ]; then
    ln -sf "$(pwd)/Macro/skim/Data/FlatSample/ppData" Data/Skimmed/PP/Data/
    ln -sf "$(pwd)/Macro/skim/Data/FlatSample/ppMC" Data/Skimmed/PP/MC/
    ln -sf "$(pwd)/Macro/skim/Data/FlatSample/PbPbData" Data/Skimmed/PbPb/Data/
    ln -sf "$(pwd)/Macro/skim/Data/FlatSample/PbPbMC" Data/Skimmed/PbPb/MC/ 2>/dev/null || true
fi

echo -e "${GREEN}✓ Data directory organized${NC}\n"

# Step 8: Move test files
echo -e "${YELLOW}Step 8: Moving test files...${NC}"

cp Macro/fit/testRun/TestDCAFit.cpp Analysis/Tests/ 2>/dev/null || true
cp Macro/fit/testRun/testBinnedFit.cpp Analysis/Tests/ 2>/dev/null || true
cp Macro/fit/testRun/test_robust_framework.cpp Analysis/Tests/ 2>/dev/null || true

echo -e "${GREEN}✓ Test files moved${NC}\n"

# Step 9: Create documentation
echo -e "${YELLOW}Step 9: Creating documentation...${NC}"

cp README.md Documentation/README_OLD.md 2>/dev/null || true
cp Macro/CLAUDE.md Documentation/ 2>/dev/null || true
cp Macro/D_STAR_ANALYSIS_RESULTS.md Documentation/ 2>/dev/null || true
cp Macro/DETAILED_CHANGES_LOG.md Documentation/ 2>/dev/null || true
cp Macro/IMPROVEMENT_LOG_KOR.md Documentation/ 2>/dev/null || true

# Create new README
cat > README.md << 'EOF'
# D* Meson Analysis Framework

Modernized and reorganized D* meson analysis framework for pp and PbPb collisions.

## 📁 Directory Structure

```
DStarAnalysis/
├── Analysis/           # Analysis code
│   ├── Common/         # Shared analysis framework
│   ├── PP/             # pp collision analysis
│   ├── PbPb/           # PbPb collision analysis
│   └── Tests/          # Unit tests
├── Preprocessing/      # Data preprocessing
├── Tools/              # Common utilities
├── Data/               # Data files (organized by collision system)
├── Results/            # Analysis results
├── Documentation/      # Documentation
└── Scripts/            # Helper scripts
```

## 🚀 Quick Start

### PP Analysis
```bash
cd Analysis/PP/scripts
./run_pp_analysis.sh
```

### PbPb Analysis
```bash
cd Analysis/PbPb/scripts
./run_pbpb_analysis.sh
```

## 📖 Documentation

See `Documentation/` for detailed guides and notes.

## 🔧 Migration

This repository was recently reorganized. Old structure is backed up in `${BACKUP_DIR}`
EOF

echo -e "${GREEN}✓ Documentation created${NC}\n"

# Step 10: Create .gitignore
echo -e "${YELLOW}Step 10: Creating .gitignore...${NC}"

cat > .gitignore << 'EOF'
# Compiled files
*.o
*.so
*.d
*.pcm
*.pyc
__pycache__/

# ROOT files (large data)
*.root
!*Template*.root
!*template*.root

# Results
Results/
results/
plots/
Plots/

# Logs
logs/
*.log

# Backup files
*_backup.*
*.backup
*_old.*
*~
*.bak
*.swp

# IDE
.vscode/
.idea/
*.code-workspace

# Condor
*.out
*.err
*.log
condor_*.log

# Temporary files
tmp/
temp/
.cache/

# OS files
.DS_Store
Thumbs.db
EOF

echo -e "${GREEN}✓ .gitignore created${NC}\n"

# Step 11: List files to clean up (backup files)
echo -e "${YELLOW}Step 11: Identifying backup files for cleanup...${NC}"

cat > Scripts/utilities/list_backup_files.sh << 'EOF'
#!/bin/bash
# List all backup files that can be safely removed

echo "Backup files in the repository:"
find . -type f \( -name "*_backup.*" -o -name "*.backup" -o -name "*_old.*" -o -name "*.bak" \) | sort

echo ""
echo "To remove all backup files, run:"
echo "  find . -type f \( -name \"*_backup.*\" -o -name \"*.backup\" -o -name \"*_old.*\" -o -name \"*.bak\" \) -delete"
EOF

chmod +x Scripts/utilities/list_backup_files.sh

echo "  Created: Scripts/utilities/list_backup_files.sh"
echo -e "${YELLOW}  Run this script to see all backup files${NC}\n"

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Migration Complete!${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo -e "${GREEN}✓ New directory structure created${NC}"
echo -e "${GREEN}✓ Code organized by collision system${NC}"
echo -e "${GREEN}✓ Documentation updated${NC}"
echo -e "${GREEN}✓ .gitignore created${NC}\n"

echo -e "${YELLOW}Next Steps:${NC}"
echo "1. Review the new structure"
echo "2. Test PP analysis: cd Analysis/PP/scripts && ./run_pp_analysis.sh"
echo "3. Test PbPb analysis: cd Analysis/PbPb/scripts && ./run_pbpb_analysis.sh"
echo "4. Update include paths in code if needed"
echo "5. Remove backup files: Scripts/utilities/list_backup_files.sh"
echo "6. Commit changes to git"
echo ""
echo -e "${YELLOW}Backup location: ${BACKUP_DIR}${NC}\n"

