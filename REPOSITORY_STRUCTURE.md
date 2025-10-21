# 📂 Repository Structure

> **Last Updated**: 2025-10-14  
> **Purpose**: D* Meson Analysis Framework for pp and PbPb collisions  
> **Structure Version**: 2.0 (Reorganized)

---

## 🎯 Overview

This repository is organized to **clearly separate PP and PbPb analyses** while maintaining common code in shared directories. The structure follows a modular design for maintainability and scalability.

## 📊 Directory Tree

```
DStarAnalysis/
│
├── Analysis/                          # 🎯 Main analysis code
│   ├── Common/                        # Shared analysis framework
│   │   ├── Core/                      # Core analysis engines
│   │   ├── Config/                    # Configuration management
│   │   ├── Managers/                  # Manager classes
│   │   ├── Utils/                     # Utility functions
│   │   └── Parameters/                # JSON parameter files
│   │       ├── pp/
│   │       │   ├── signal/
│   │       │   └── background/
│   │       └── PbPb/
│   │           ├── signal/
│   │           └── background/
│   │
│   ├── PP/                            # pp collision analysis
│   │   ├── DStarAnalysisPP.cpp
│   │   ├── ConfigPP.h
│   │   ├── scripts/
│   │   └── results/
│   │
│   ├── PbPb/                          # PbPb collision analysis
│   │   ├── DStarAnalysisPbPb.cpp
│   │   ├── ConfigPbPb.h
│   │   ├── scripts/
│   │   └── results/
│   │
│   └── Tests/                         # Unit tests
│
├── Preprocessing/                     # 🔧 Data preprocessing
│   ├── Common/                        # Shared preprocessing tools
│   │   ├── DStarRDSMakerImproved.cpp
│   │   ├── FlexibleFlattener.cpp
│   │   └── ImprovedDataProcessor.cpp
│   ├── PP/
│   │   └── scripts/
│   └── PbPb/
│       └── scripts/
│
├── Corrections/                       # 📐 Correction maps
│   ├── Common/                        # Common utilities
│   │   ├── AccEffHelpers.h
│   │   ├── EffHead.h
│   │   └── correctionmap.hxx
│   ├── Generators/                    # Map generation
│   │   ├── PP/
│   │   ├── PbPb/
│   │   └── D0/
│   ├── Validation/                    # Validation scripts
│   └── scripts/
│
├── Tools/                             # 🛠️ Common tools
│   ├── Kinematics/                    # Coordinate transformations
│   │   ├── Transformations.h
│   │   └── PhaseSpace.h
│   ├── Utils/                         # Utility functions
│   │   ├── FileManager.h
│   │   ├── CentralityUtils.h
│   │   └── GlobalCuts.h
│   ├── Plotting/                      # Plotting utilities
│   │   ├── CMS_lumi.h
│   │   ├── tdrStyle.C
│   │   └── ColorSchemes.h
│   └── Base/                          # Base classes
│       ├── BasicHeaders.h
│       └── DataProcessorBase.h
│
├── Data/                              # 💾 Data files
│   ├── Raw/                           # Raw ROOT files
│   │   ├── PP/{Data,MC}
│   │   └── PbPb/{Data,MC}
│   ├── Skimmed/                       # Skimmed flat files
│   │   ├── PP/{Data,MC}/{D0,DStar}
│   │   └── PbPb/{Data,MC}/{D0,DStar}
│   ├── RDS/                           # RooDataSet files
│   │   ├── PP/{Data,MC}
│   │   └── PbPb/{Data,MC}
│   └── Auxiliary/                     # Auxiliary data
│       ├── EfficiencyMaps/{PP,PbPb}
│       ├── CorrectionFactors/{PP,PbPb}
│       ├── AcceptanceMaps/{PP,PbPb}
│       └── Templates/
│
├── Results/                           # 📈 Analysis results (gitignore)
│   ├── PP/{Fits,Plots,Tables}
│   └── PbPb/{Fits,Plots,Tables}
│
├── Documentation/                     # 📖 Documentation
│   ├── README_OLD.md
│   ├── CHANGELOG.md
│   ├── UserGuide.md
│   └── DeveloperNotes.md
│
├── Scripts/                           # 🔨 Utility scripts
│   ├── run_full_pp_pipeline.sh
│   ├── run_full_pbpb_pipeline.sh
│   └── utilities/
│       ├── update_includes.sh
│       ├── cleanup_backup_files.sh
│       └── verify_migration.sh
│
└── Macro/                             # 🗃️ Legacy code (preserved)
    ├── fit/
    ├── skim/
    └── Tools/
```

---

## 📁 Detailed Directory Descriptions

### 1. Analysis/

Main analysis code, separated by collision system.

#### Analysis/Common/
**Purpose**: Shared code used by both PP and PbPb analyses

##### Core/
- `MassFitterV2.h` - Mass fitting engine with multiple PDF support
- `DCAFitter.h` - DCA template fitting for prompt/non-prompt separation
- `DataLoader.h` - Unified data loading interface
- `PDFFactory.h` - Factory for creating PDF objects
- `PDFFactoryImpl.h` - PDF factory implementation

##### Config/
- `DStarFitConfig.h` - Main configuration class
- `DStarFitOpt.h` - Fit options and settings
- `FitCommonConfig.h` - Common fit configurations
- `Opt.h` - Legacy options
- `Params.h` - Parameter definitions

##### Managers/
- `ParameterManager.h` - Parameter management
- `ResultManager.h` - Result storage and retrieval
- `EnhancedPlotManager.h` - Plotting management
- `ErrorHandler.h` - Error handling and logging
- `FitManager.h` - Fit workflow management
- `FlexibleParameterManager.h` - Flexible parameter handling
- `RobustParameterManager.h` - Robust parameter management

##### Utils/
- `Helper.h` - General helper functions
- `JSONParameterUtils.h` - JSON parameter utilities
- `JSONParameterLoader.h` - JSON loading
- `SimpleParameterLoader.h` - Simple parameter loading
- `ParameterDebugUtils.h` - Parameter debugging
- `FitStrategy.h` - Fit strategy definitions
- `CorrectionUtils.h` - Correction application utilities

##### Parameters/
- `pp/signal/*.json` - PP signal PDF parameters
- `pp/background/*.json` - PP background PDF parameters
- `PbPb/signal/*.json` - PbPb signal PDF parameters
- `PbPb/background/*.json` - PbPb background PDF parameters

#### Analysis/PP/
**Purpose**: pp collision specific analysis

**Key Files**:
- `DStarAnalysisPP.cpp` - Main PP analysis script
- `ConfigPP.h` - PP-specific configuration

**Include Pattern**:
```cpp
#include "../../Common/Core/MassFitterV2.h"
#include "../../Common/Config/DStarFitConfig.h"
#include "../ConfigPP.h"
```

#### Analysis/PbPb/
**Purpose**: PbPb collision specific analysis

**Key Files**:
- `DStarAnalysisPbPb.cpp` - Main PbPb analysis script
- `ConfigPbPb.h` - PbPb-specific configuration

**Include Pattern**:
```cpp
#include "../../Common/Core/MassFitterV2.h"
#include "../../Common/Config/DStarFitConfig.h"
#include "../ConfigPbPb.h"
```

#### Analysis/Tests/
**Purpose**: Unit tests for analysis framework

**Key Files**:
- `TestDCAFit.cpp` - DCA fitter tests
- `testBinnedFit.cpp` - Binned fit tests
- `test_robust_framework.cpp` - Framework robustness tests

---

### 2. Preprocessing/

Data preprocessing and conversion.

#### Preprocessing/Common/
**Purpose**: Common preprocessing tools

**Key Files**:
- `DStarRDSMakerImproved.cpp` - ROOT TTree → RooDataSet converter (improved)
  - Supports multiple reference frames (CS, HX, EP)
  - Efficiency correction support
  - Bug fixes: no duplicate calculations, memory safe
- `FlexibleFlattener.cpp` - Flexible data flattening
- `ImprovedDataProcessor.cpp` - Enhanced data processing
- `Flat.cpp` - Basic flattening

**Usage**:
```bash
cd Preprocessing/Common
root -l -b -q DStarRDSMakerImproved.cpp
```

---

### 3. Corrections/

Efficiency and acceptance correction generation and validation.

#### Corrections/Common/
**Purpose**: Common correction utilities

**Key Files**:
- `AccEffHelpers.h` - Acceptance & efficiency helpers
- `EffHead.h` - Efficiency calculation headers
- `correctionmap.hxx` - Correction map interface

#### Corrections/Generators/
**Purpose**: Generate correction maps

**PP/**:
- `EfficiencyMaps_PP.cpp` - Generate PP efficiency maps
- `FlatEffAccCalculator_PP.cpp` - Calculate PP efficiency from flat trees
- `AcceptanceMap_noGenFilter.cpp` - Generate acceptance maps
- `D0pTEtaMap.cpp` - D0 kinematic maps

**PbPb/**:
- `FlatEffAccCalculator_PbPb.cpp` - Calculate PbPb efficiency

**D0/**:
- `FlatEffAccCalculator_D0.cpp` - Calculate D0 efficiency
- `MVAEfficiency.cpp` - MVA-based efficiency calculation

**Output Location**: `Data/Auxiliary/EfficiencyMaps/{PP,PbPb}/`

#### Corrections/Validation/
**Purpose**: Validate correction maps

**Key Files**:
- `PlotAcceptanceFromFile.cpp` - Visualize acceptance maps
- `HistBasedCorrection.cpp` - Histogram-based validation
- `TestCorrection.cpp` - Correction tests

---

### 4. Tools/

Common utilities shared across all analyses.

#### Tools/Kinematics/
**Purpose**: Coordinate system transformations

**Key Files**:
- `Transformations.h` - Frame transformations (Helicity, Collins-Soper, Event Plane)
- `PhaseSpace.h` - Phase space calculations

**Functions**:
```cpp
TVector3 DstarDau1Vector_Helicity(TLorentzVector dstar, TLorentzVector dau1);
TVector3 DstarDau1Vector_CollinsSoper(TLorentzVector dstar, TLorentzVector dau1);
TVector3 DstarDau1Vector_EventPlane(TLorentzVector dstar, TLorentzVector dau1, double psi);
```

#### Tools/Utils/
**Purpose**: General utility functions

**Key Files**:
- `FileManager.h` - File management utilities
- `CentralityUtils.h` - Centrality handling
- `GlobalCuts.h` - Global selection cuts
- `Helper.h` - General helpers
- `VariableHandler.h` - Variable handling

#### Tools/Plotting/
**Purpose**: Plotting utilities with CMS style

**Key Files**:
- `CMS_lumi.h` - CMS luminosity label
- `tdrStyle.C` - TDR plotting style
- `ColorSchemes.h` - Color scheme definitions
- `Legends.h` - Legend utilities

#### Tools/Base/
**Purpose**: Base classes and headers

**Key Files**:
- `BasicHeaders.h` - Common includes
- `DataProcessorBase.h` - Base data processor class

---

### 5. Data/

Data files organized by collision system and processing stage.

#### Data/Raw/
**Purpose**: Raw ROOT files from production

**Structure**:
```
Raw/
├── PP/
│   ├── Data/    # pp data
│   └── MC/      # pp MC
└── PbPb/
    ├── Data/    # PbPb data
    └── MC/      # PbPb MC
```

#### Data/Skimmed/
**Purpose**: Skimmed and flattened data

**Structure**:
```
Skimmed/
├── PP/
│   ├── Data/
│   │   ├── D0/      # D0 meson data
│   │   └── DStar/   # D* meson data
│   └── MC/
│       ├── D0/
│       └── DStar/
└── PbPb/
    ├── Data/
    │   ├── D0/
    │   └── DStar/
    └── MC/
        ├── D0/
        └── DStar/
```

#### Data/RDS/
**Purpose**: RooDataSet files ready for fitting

**Structure**:
```
RDS/
├── PP/
│   ├── Data/    # PP data RDS
│   └── MC/      # PP MC RDS
└── PbPb/
    ├── Data/    # PbPb data RDS
    └── MC/      # PbPb MC RDS
```

**File Naming Convention**:
```
RDS_Physics_Data_DStar_ppRef_<tag>_v<version>.root
RDS_Physics_MC_DStar_PbPb_<tag>_v<version>.root
```

#### Data/Auxiliary/
**Purpose**: Auxiliary data for analysis

**EfficiencyMaps/**:
- PP efficiency maps
- PbPb efficiency maps
- Format: 3D histograms (pT, y, |cosθ|)

**CorrectionFactors/**:
- PP correction factors
- PbPb correction factors

**AcceptanceMaps/**:
- PP acceptance maps
- PbPb acceptance maps
- Format: 2D histograms (η, φ)

**Templates/**:
- DCA templates
- Background templates

---

### 6. Results/

Analysis output (excluded from git).

**Structure**:
```
Results/
├── PP/
│   ├── Fits/      # Fit results (.root, .txt)
│   ├── Plots/     # Plots (.pdf, .png)
│   └── Tables/    # Tables (.csv, .tex)
└── PbPb/
    ├── Fits/
    ├── Plots/
    └── Tables/
```

**Gitignored**: All files in `Results/` are ignored by git.

---

### 7. Documentation/

All documentation files.

**Key Files**:
- `README_OLD.md` - Original repository README
- `CLAUDE.md` - AI assistant development notes
- `D_STAR_ANALYSIS_RESULTS.md` - Analysis results documentation
- `DETAILED_CHANGES_LOG.md` - Detailed changelog
- `IMPROVEMENT_LOG_KOR.md` - Korean improvement log

---

### 8. Scripts/

Helper scripts for common tasks.

#### Pipeline Scripts:
- `run_full_pp_pipeline.sh` - Complete PP analysis pipeline
- `run_full_pbpb_pipeline.sh` - Complete PbPb analysis pipeline

#### Utility Scripts:
- `utilities/update_includes.sh` - Update include paths
- `utilities/cleanup_backup_files.sh` - Clean backup files
- `utilities/verify_migration.sh` - Verify migration success

---

### 9. Macro/ (Legacy)

**Purpose**: Original code structure (preserved for reference)

**Status**: Preserved as-is, not actively maintained

**Use**: Reference only, use new structure for active development

---

## 🔄 Workflow Examples

### PP Analysis Workflow

```bash
# 1. Preprocessing (if needed)
cd Preprocessing/Common
root -l -b -q 'DStarRDSMakerImproved.cpp("pp")'

# 2. Generate efficiency map (once)
cd ../../Corrections/Generators/PP
root -l -b -q EfficiencyMaps_PP.cpp

# 3. Run analysis
cd ../../Analysis/PP
root -l -b -q DStarAnalysisPP.cpp

# 4. Check results
ls -la ../../Results/PP/Plots/
```

### PbPb Analysis Workflow

```bash
# 1. Preprocessing
cd Preprocessing/Common
root -l -b -q 'DStarRDSMakerImproved.cpp("PbPb")'

# 2. Generate efficiency map
cd ../../Corrections/Generators/PbPb
root -l -b -q EfficiencyMaps_PbPb.cpp

# 3. Run analysis
cd ../../Analysis/PbPb
root -l -b -q DStarAnalysisPbPb.cpp

# 4. Check results
ls -la ../../Results/PbPb/Plots/
```

---

## 📝 Include Path Patterns

### From PP Analysis:
```cpp
// Common core
#include "../../Common/Core/MassFitterV2.h"
#include "../../Common/Core/DCAFitter.h"

// Common config
#include "../../Common/Config/DStarFitConfig.h"

// Common utils
#include "../../Common/Utils/Helper.h"
#include "../../Common/Utils/CorrectionUtils.h"

// PP-specific
#include "../ConfigPP.h"

// Tools
#include "../../../Tools/Kinematics/Transformations.h"
#include "../../../Tools/Plotting/CMS_lumi.h"
```

### From Preprocessing:
```cpp
// Tools
#include "../../Tools/Kinematics/Transformations.h"
#include "../../Tools/Utils/FileManager.h"
```

### From Corrections:
```cpp
// Common
#include "../Common/AccEffHelpers.h"

// Tools
#include "../../Tools/Kinematics/Transformations.h"
```

---

## 🎯 Quick Reference

### Find PP Code:
```bash
find Analysis/PP -name "*.cpp" -o -name "*.h"
```

### Find PbPb Code:
```bash
find Analysis/PbPb -name "*.cpp" -o -name "*.h"
```

### Find Common Code:
```bash
find Analysis/Common -name "*.h"
```

### Find Correction Code:
```bash
find Corrections -type f \( -name "*.cpp" -o -name "*.h" \)
```

### Locate Data Files:
```bash
# PP data
ls Data/RDS/PP/Data/

# PbPb data
ls Data/RDS/PbPb/Data/

# Efficiency maps
ls Data/Auxiliary/EfficiencyMaps/
```

---

## 🔍 Search Patterns

### Find Usage of a Class:
```bash
grep -r "MassFitterV2" Analysis/
```

### Find Include of a Header:
```bash
grep -r "DCAFitter.h" Analysis/
```

### Find Parameter Files:
```bash
find Analysis/Common/Parameters -name "*.json"
```

---

## 📚 Additional Documentation

- **Migration Guide**: `MIGRATION_GUIDE.md` - How to migrate from old structure
- **Quick Start**: `QUICK_START.md` - Quick start guide
- **Rollback Guide**: `ROLLBACK_GUIDE.md` - How to rollback if needed
- **README**: `README.md` - Main repository README

---

## 🏷️ Version History

- **v2.0** (2025-10-14): Reorganized structure with PP/PbPb separation
- **v1.x** (2024-): Original structure (see `Macro/`)

---

## 💡 Tips for AI Agents

1. **Analysis Code**: Check `Analysis/{PP,PbPb}/` first
2. **Common Framework**: Look in `Analysis/Common/`
3. **Preprocessing**: `Preprocessing/Common/`
4. **Corrections**: `Corrections/Generators/{PP,PbPb}/`
5. **Data Location**: `Data/{Skimmed,RDS}/{PP,PbPb}/`
6. **Legacy Code**: Preserved in `Macro/` (reference only)

---

**Last Modified**: 2025-10-14  
**Maintained By**: D* Analysis Team

