# DStar Analysis Framework Documentation

## Overview
This directory contains the D* meson analysis framework for high-energy physics experiments, specifically designed for PbPb and pp collision analyses. The framework provides a modular, robust fitting system with comprehensive parameter management and visualization capabilities.

## Directory Structure

### 📁 Common/ - Shared Framework Components

#### 📁 Config/ - Configuration Management
- **`DStarFitConfig.h`** - Main configuration header defining PDF types, fitting options, and analysis parameters
- **`DStarFitOpt.h`** - Fitting optimization options and strategies
- **`FitCommonConfig.h`** - Common fitting configuration parameters
- **`Opt.h`** - Optimization settings and algorithms
- **`Params.h`** - Parameter definitions and structures

#### 📁 Core/ - Core Analysis Components
- **`DataLoader.h`** - Data loading and preprocessing utilities
- **`DCAFitter.h`** - DCA (Distance of Closest Approach) template fitting
- **`MassFitterV2.h`** - Advanced mass fitting algorithms (version 2)
- **`PDFFactory.h`** - Factory pattern for creating probability density functions
- **`PDFFactoryImpl.h`** - Implementation of PDF factory methods

#### 📁 Managers/ - Management Classes
- **`EnhancedPlotManager.h`** - Advanced plotting and visualization management
- **`ErrorHandler.h`** - Unified error handling and logging system
- **`FitManager.h`** - Main fitting workflow management
- **`FlexibleParameterManager.h`** - Dynamic parameter management
- **`ParameterManager.h`** - Base parameter management interface
- **`ResultManager.h`** - Analysis results storage and retrieval
- **`RobustParameterManager.h`** - Robust parameter handling with error recovery

#### 📁 Parameters/ - Parameter Configuration Files
- **JSON Parameter Files:**
  - `dstar_fit_parameters_dbcb_phenom_v1.json` - DBCrystalBall phenomenological parameters
  - `dstar_fit_parameters_dbcb_threshold_v2.json` - DBCrystalBall threshold parameters
  - `dstar_parameters_DBCrystalBall_DstD0_dca_v1.json` - DCA-specific DBCrystalBall parameters
  - `dstar_parameters_DoubleGaussian_Phenomenological.json` - Double Gaussian phenomenological
  - `dstar_parameters_grid_unified.json` - Unified grid parameters

- **📁 PbPb/** - PbPb-specific parameters
  - `background/` - Background parameter configurations
  - `signal/` - Signal parameter configurations

- **📁 pp/** - pp-specific parameters
  - `background/` - Background parameter configurations
  - `signal/` - Signal parameter configurations
  - `dstar_fit_parameters_pp.json` - Main pp fitting parameters
  - `dstar_parameters_DBCrystalBall_Phenomenological2_dca_v1.json` - DCA parameters for pp
  - `dstar_parameters_grid_unified_pp.json` - Unified pp grid parameters

#### 📁 Utils/ - Utility Classes
- **`CorrectionUtils.h`** - Correction factor calculations
- **`FitStrategy.h`** - Fitting strategy definitions
- **`Helper.h`** - General utility functions
- **`JSONParameterLoader.h`** - JSON parameter file loading
- **`JSONParameterUtils.h`** - JSON parameter manipulation utilities
- **`ParameterDebugUtils.h`** - Parameter debugging and diagnostics
- **`SimpleParameterLoader.h`** - Simple parameter loading interface

### 📁 PbPb/ - Lead-Lead Collision Analysis

#### Main Analysis Files
- **`DStarAnalysisPbPb.cpp`** - Main PbPb analysis implementation (760 lines)
- **`ConfigPbPb.h`** - PbPb-specific configuration and cuts
- **`DStarAnalysisPbPb.cpp.include_backup`** - Backup of include statements

#### Configuration & Scripts
- **`runConfig.sh`** - PbPb analysis execution script
- **📁 scripts/** - Execution scripts
  - `condor_submit_DStarV2.sub` - Condor job submission script
- **📁 results/** - Analysis results directory

#### PbPb-Specific Features
- **Selection Cuts:**
  - Daughter particle cuts: pT_D1 ≥ 0, pT_D2 ≥ 0.3, |η| ≤ 2.4/1.6
  - Grand-daughter cuts: pT ≥ 1.5, |η| ≤ 1.6
  - Parent particle cuts: pT ∈ [1.0, 100.0], |y| ≤ 1.0
  - Track quality: NHit ≥ 10, pT error ≤ 0.1

### 📁 PP/ - Proton-Proton Collision Analysis

#### Main Analysis Files
- **`DStarAnalysisPP.cpp`** - Main pp analysis implementation (733 lines)
- **`ConfigPP.h`** - pp-specific configuration and cuts
- **`DStarAnalysisPP.cpp.include_backup`** - Backup of include statements

#### Configuration & Scripts
- **`runConfig.sh`** - Main pp analysis execution script
- **`runConfig_simple.sh`** - Simplified pp analysis script
- **`test_runconfig.sh`** - Test configuration script
- **📁 scripts/** - Execution scripts
  - `condor_submit_DStarV2_pp.sub` - Condor job submission for pp
  - `run_pp_analysis.sh` - pp analysis runner script
- **📁 results/** - Analysis results with subdirectories:
  - `DStar_ppRef_PU_NoCorrection_Sep20_pTGrand_1p0_EtaGrand_2p4_pTD2_0p4/`
    - `Data/` - Data analysis results
    - `MC/` - Monte Carlo analysis results
- **📁 logs/** - Analysis log files
- **`README_runConfig.md`** - pp analysis documentation

#### PP-Specific Features
- **Selection Cuts:**
  - Daughter particle cuts: pT_D1 ≥ 0, pT_D2 ≥ 0.4, |η| ≤ 2.4
  - Grand-daughter cuts: pT ≥ 1.0, |η| ≤ 2.4
  - Parent particle cuts: pT ∈ [1.0, 50.0], |y| ≤ 1.0
  - Track quality: NHit ≥ 10, pT error ≤ 0.1

### 📁 Tests/ - Testing Framework

#### Test Files
- **`test_robust_framework.cpp`** - Tests the robust fitting framework
- **`testBinnedFit.cpp`** - Tests binned fitting functionality
- **`TestDCAFit.cpp`** - Tests DCA fitting algorithms

## Key Features

### 🔧 PDF Types Supported
- **Signal PDFs:** Gaussian, DoubleGaussian, CrystalBall, DBCrystalBall, DoubleDBCrystalBall, Voigtian, BreitWigner
- **Background PDFs:** Exponential, Chebychev, Phenomenological, Phenomenological2, Polynomial, ThresholdFunction, ExpErf, DstBg, DstD0

### 🎯 Analysis Capabilities
- **Mass Fitting:** Advanced mass fitting with multiple PDF options
- **DCA Fitting:** Distance of closest approach template fitting
- **Parameter Management:** Robust parameter handling with JSON configuration
- **Visualization:** Enhanced plotting and result visualization
- **Error Handling:** Comprehensive error handling and logging
- **Testing:** Automated testing framework for validation

### 📊 Output Structure
- **ROOT Files:** Fitted histograms and results
- **Plots:** PDF plots, DCA templates, fitted distributions
- **Logs:** Analysis execution logs
- **Parameters:** JSON configuration files for different analysis scenarios

## Usage

### ROOT Environment Setup
The system provides multiple ROOT versions through aliases. To use ROOT 6.24:

```bash
# Activate ROOT 6.24 environment
ROOT6.24

# Verify ROOT is active
root --version
# Output: ROOT Version: 6.24/09

# Available ROOT versions:
# ROOT6.24  - ROOT 6.24/09 (default for this analysis)
# ROOT6.30  - ROOT 6.30
# ROOT6.32_cuda - ROOT 6.32 with CUDA support
# ROOT6.36_cuda - ROOT 6.36 with CUDA support
```

### Running PbPb Analysis
```bash
# First activate ROOT environment
ROOT6.24

# Navigate to PbPb directory
cd PbPb/

# Run the analysis
./runConfig.sh
```

### Running PP Analysis
```bash
# First activate ROOT environment
ROOT6.24

# Navigate to PP directory
cd PP/

# Run the analysis
./runConfig.sh
# or for simple analysis:
./runConfig_simple.sh
```

### Testing Framework
```bash
# First activate ROOT environment
ROOT6.24

# Navigate to Tests directory
cd Tests/

# Compile and run test files
# (Test files need to be compiled with ROOT environment active)
```

### ROOT Interactive Usage
```bash
# Start ROOT interactive session
ROOT6.24
root

# Load and run analysis macros
.L DStarAnalysisPbPb.cpp
.L DStarAnalysisPP.cpp

# Or run specific functions
gROOT->ProcessLine(".L DStarAnalysisPbPb.cpp");
```

## Dependencies
- ROOT framework
- JSON parameter files
- Condor job submission system (optional)
- C++17 or later for variant support

## Notes
- Both PbPb and PP analyses share the same core framework in `Common/`
- Configuration differences are handled through separate config headers
- Parameter files are organized by collision system and analysis type
- The framework supports both data and Monte Carlo analysis
- Results are automatically organized by analysis type and kinematic bins
