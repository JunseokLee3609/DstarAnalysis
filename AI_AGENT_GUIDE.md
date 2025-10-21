# 🤖 AI Agent Quick Reference Guide

> **For AI Assistants**: This is a condensed guide to navigate the repository efficiently.

---

## ⚠️ CRITICAL: Environment Setup

### Before Running ANY Analysis:

```bash
# 1. Setup ROOT (MANDATORY!)
ROOT6.24

# 2. Verify it worked
root --version
# Should show: ROOT Version 6.24 or later

# 3. Navigate to project
cd /home/jun502s/DstarAna/DStarAnalysis

# 4. Check environment (optional)
bash check_environment.sh
```

### Quick Test:
```bash
ROOT6.24 && root -l -b -q -e 'cout << "ROOT OK" << endl'
```

**Without ROOT setup, NOTHING will work!**

See [ENVIRONMENT.md](ENVIRONMENT.md) for details.

---

## 🎯 Repository Purpose

D* Meson physics analysis framework for **pp** and **PbPb** collisions at CMS experiment.

---

## 📂 Directory Cheat Sheet

| Directory | Purpose | When to Look Here |
|-----------|---------|-------------------|
| `Analysis/PP/` | **pp collision analysis** | User asks about pp analysis, ppRef |
| `Analysis/PbPb/` | **PbPb collision analysis** | User asks about PbPb, heavy-ion, centrality |
| `Analysis/Common/` | **Shared analysis code** | Core functionality, PDF models, fitting |
| `Preprocessing/` | **Data preparation** | TTree → RooDataSet conversion, skimming |
| `Corrections/` | **Efficiency/Acceptance** | Correction maps, efficiency calculation |
| `Tools/` | **Utilities** | Kinematics, plotting, file management |
| `Data/` | **Data files** | Organized by PP/PbPb → Raw/Skimmed/RDS |
| `Macro/` | **Legacy code** | Reference only, not actively maintained |

---

## 🔍 Quick File Lookup

### User wants to analyze PP data:
→ **`Analysis/PP/DStarAnalysisPP.cpp`**

### User wants to analyze PbPb data:
→ **`Analysis/PbPb/DStarAnalysisPbPb.cpp`**

### User asks about mass fitting:
→ **`Analysis/Common/Core/MassFitterV2.h`**

### User asks about DCA fitting (prompt/non-prompt):
→ **`Analysis/Common/Core/DCAFitter.h`**

### User wants to create RooDataSet:
→ **`Preprocessing/Common/DStarRDSMakerImproved.cpp`**

### User asks about efficiency correction:
→ **`Corrections/Generators/PP/` or `/PbPb/`**
→ **`Analysis/Common/Utils/CorrectionUtils.h`** (to apply)

### User asks about coordinate transformations:
→ **`Tools/Kinematics/Transformations.h`**

### User needs parameters:
→ **`Analysis/Common/Parameters/{pp,PbPb}/`** (JSON files)

---

## 🎨 Code Structure Pattern

```
Analysis/
├── Common/                    # ← Shared by PP and PbPb
│   ├── Core/                  # ← Fitting engines
│   ├── Config/                # ← Configuration
│   ├── Managers/              # ← Result/Parameter management
│   └── Utils/                 # ← Helper functions
│
├── PP/                        # ← PP-specific
│   ├── DStarAnalysisPP.cpp   # ← Main PP script
│   └── ConfigPP.h            # ← PP config
│
└── PbPb/                      # ← PbPb-specific
    ├── DStarAnalysisPbPb.cpp # ← Main PbPb script
    └── ConfigPbPb.h          # ← PbPb config
```

---

## 💡 Common Questions & Answers

### Q: "How do I run PP analysis?"
```bash
cd Analysis/PP
root -l -b -q DStarAnalysisPP.cpp
```

### Q: "How do I run PbPb analysis?"
```bash
cd Analysis/PbPb
root -l -b -q DStarAnalysisPbPb.cpp
```

### Q: "Where is the data?"
```
Data/
├── RDS/PP/Data/       # PP data (RooDataSet)
├── RDS/PbPb/Data/     # PbPb data (RooDataSet)
├── Skimmed/PP/        # Skimmed PP files
└── Skimmed/PbPb/      # Skimmed PbPb files
```

### Q: "How to create efficiency maps?"
```bash
cd Corrections/Generators/PP  # or PbPb
root -l -b -q EfficiencyMaps_PP.cpp
# Output: Data/Auxiliary/EfficiencyMaps/PP/
```

### Q: "How to apply corrections in analysis?"
```cpp
#include "../../Common/Utils/CorrectionUtils.h"

auto h_eff = CorrectionUtils::LoadEfficiencyMap("path/to/map.root");
double weight = CorrectionUtils::GetEfficiencyWeight(h_eff.get(), pT, y, cos);
```

### Q: "What's the difference between PP and PbPb?"
- **PP**: proton-proton collisions, no centrality
- **PbPb**: lead-lead collisions, includes centrality, higher multiplicity

### Q: "Where are PDF models defined?"
→ `Analysis/Common/Core/PDFFactory.h` and `PDFFactoryImpl.h`

Supported:
- Signal: Gaussian, Crystal Ball, Double CB, Voigtian
- Background: Exponential, Chebychev, Phenomenological, Threshold

### Q: "How to modify fit parameters?"
→ Edit JSON files in `Analysis/Common/Parameters/{pp,PbPb}/`

Example:
```json
{
  "pT_10_15": {
    "signal": {
      "mean": 0.1455,
      "sigma": 0.001,
      "alphaLow": 1.5
    }
  }
}
```

### Q: "What reference frames are supported?"
- **HX** (Helicity): Most common, boosted to D* rest frame
- **CS** (Collins-Soper): Beam axis reference
- **EP** (Event Plane): For flow analysis

Code: `Tools/Kinematics/Transformations.h`

---

## 🔗 Include Path Patterns

### From PP/PbPb Analysis:
```cpp
#include "../../Common/Core/MassFitterV2.h"      // Core
#include "../../Common/Config/DStarFitConfig.h"   // Config
#include "../../Common/Utils/Helper.h"            // Utils
#include "../ConfigPP.h"                          // PP-specific
#include "../../../Tools/Kinematics/Transformations.h"  // Tools
```

### From Preprocessing:
```cpp
#include "../../Tools/Kinematics/Transformations.h"
#include "../../Tools/Utils/FileManager.h"
```

### From Corrections:
```cpp
#include "../Common/AccEffHelpers.h"
#include "../../Tools/Kinematics/Transformations.h"
```

---

## 🔧 Key Classes

### MassFitterV2
**Location**: `Analysis/Common/Core/MassFitterV2.h`  
**Purpose**: Mass spectrum fitting with multiple PDF models  
**Key Methods**:
- `Fit()` - Perform fit
- `GetSignalYield()` - Extract signal yield
- `Plot()` - Create fit plots

### DCAFitter
**Location**: `Analysis/Common/Core/DCAFitter.h`  
**Purpose**: Separate prompt and non-prompt components  
**Key Methods**:
- `FitDCA()` - Fit DCA distribution
- `GetPromptFraction()` - Extract prompt fraction

### DStarRDSMakerImproved
**Location**: `Preprocessing/Common/DStarRDSMakerImproved.cpp`  
**Purpose**: Convert TTree to RooDataSet  
**Features**:
- Multiple reference frames (HX, CS, EP)
- Efficiency correction support
- Improved memory management

### CorrectionUtils
**Location**: `Analysis/Common/Utils/CorrectionUtils.h`  
**Purpose**: Apply corrections in analysis  
**Key Functions**:
- `LoadEfficiencyMap()` - Load efficiency map
- `GetEfficiencyWeight()` - Calculate weight
- `LoadAcceptanceMap()` - Load acceptance map

---

## 🎯 Workflow Shortcuts

### Full PP Pipeline:
```bash
./Scripts/run_full_pp_pipeline.sh
```

### Full PbPb Pipeline:
```bash
./Scripts/run_full_pbpb_pipeline.sh
```

### Verify Structure:
```bash
./Scripts/utilities/verify_migration.sh
```

---

## 🚨 Common Pitfalls

1. **Wrong Include Paths**: Use relative paths from current file
2. **Missing Efficiency Maps**: Generate before analysis
3. **Centrality Only in PbPb**: Don't use centrality cuts in PP
4. **Reference Frame**: HX is default, specify if different

---

## 📚 Documentation Files

- `REPOSITORY_STRUCTURE.md` - Detailed structure (this is more detailed)
- `MIGRATION_GUIDE.md` - Migration from old structure
- `QUICK_START.md` - Quick start guide
- `ROLLBACK_GUIDE.md` - How to rollback changes
- `README.md` - Main README

---

## 🔄 Data Flow

```
Raw Data (.root)
    ↓
Skimming (Preprocessing/)
    ↓
Flat Trees (.root)
    ↓
RDS Maker (Preprocessing/Common/DStarRDSMakerImproved.cpp)
    ↓
RooDataSet (Data/RDS/)
    ↓
Analysis (Analysis/PP/ or Analysis/PbPb/)
    ↓
Results (Results/PP/ or Results/PbPb/)
```

---

## 🎓 Tips for Assisting Users

### User mentions "pp":
- Direct to `Analysis/PP/`
- Check `Analysis/Common/Parameters/pp/`
- Use PP-specific configs

### User mentions "PbPb" or "centrality":
- Direct to `Analysis/PbPb/`
- Check `Analysis/Common/Parameters/PbPb/`
- Consider centrality-dependent parameters

### User asks about "fitting":
- `Analysis/Common/Core/MassFitterV2.h` for mass fits
- `Analysis/Common/Core/DCAFitter.h` for DCA fits

### User asks about "efficiency" or "corrections":
- Generation: `Corrections/Generators/{PP,PbPb}/`
- Application: `Analysis/Common/Utils/CorrectionUtils.h`
- Storage: `Data/Auxiliary/EfficiencyMaps/`

### User asks about "transformations" or "helicity":
- `Tools/Kinematics/Transformations.h`
- Functions: `DstarDau1Vector_Helicity()`, etc.

### User mentions "DStarRDSMaker":
- Use improved version: `Preprocessing/Common/DStarRDSMakerImproved.cpp`
- Bug-free, memory-safe, efficient

---

## 🏷️ Version Info

- **Structure Version**: 2.0
- **Last Updated**: 2025-10-14
- **Migration Status**: Complete
- **Legacy Code**: Preserved in `Macro/`

---

**Quick Help Command**:
```bash
cat AI_AGENT_GUIDE.md  # This file
cat REPOSITORY_STRUCTURE.md  # Detailed structure
```

