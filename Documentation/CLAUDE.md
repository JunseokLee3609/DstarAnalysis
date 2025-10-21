# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a D* meson analysis framework for high-energy physics research, built on ROOT6.24 and RooFit. The framework performs comprehensive analysis of D* meson data from heavy-ion collision experiments, following a systematic workflow from data skimming to final physics results.

## Analysis Workflow

The analysis follows a structured 4-step process:

1. **Skim Ntuples** → 2. **Perform Fit** → 3. **Calculate Efficiency** → 4. **Apply Corrections**

Each step has dedicated directories and tools in the `Macro/` folder.

## Architecture & Directory Structure

### Core Directories

- **`skim/`** - Data processing and event selection
  - `FlexibleFlattener.cpp` - Main skimming tool for ROOT ntuples
  - `DStarRDSMaker.cpp` - Creates RooDataSet from skimmed data
  - `condor_submit.sub` - HTCondor batch job submission
  - `RunConfig.sh` - Configuration script with example commands

- **`fit/`** - Mass fitting and signal extraction
  - `MassFitter.h/.cpp` - Template-based fitting framework with RooFit
  - `Opt.h` - Configuration options and parameter management
  - `Params.h` - PDF parameter definitions (Gaussian, Crystal Ball, etc.)
  - `Helper.h` - Utility functions for fitting
  - `PlotManager.h` - Result visualization and plotting
  - `DataLoader.h` - RooDataSet loading and management
  - `DCAFitter.h` - Decay length significance fitting
  - `testRun/` - Example macros and condor scripts

- **`Correction/`** - Efficiency and acceptance corrections
  - `FlatEffAccCalculator.cpp` - Efficiency calculation from MC
  - `commonSelectionVar.h` - Selection variable definitions
  - Efficiency maps and correction ROOT files

- **`Tools/`** - Common utilities and styling
  - `BasicHeaders.h` - Standard ROOT/RooFit includes
  - `Style/` - CMS publication-ready plotting styles
  - `Parameters/` - Physics constants and analysis parameters

- **`interface/`** - Data interfaces and ROOT tree definitions
  - `correctionmap.hxx` - Correction factor mappings
  - `simpleDMC.h` - MC truth matching utilities

- **`plot/`** - Visualization and result plotting
- **`test/`** - Analysis validation and comparison tools

### Key Components

**MassFitter Class**: Template-based fitting framework supporting multiple PDF types:
- Signal PDFs: Gaussian, Double Gaussian, Crystal Ball, Double Crystal Ball, Voigtian
- Background PDFs: Exponential, Chebychev, Polynomial, Phenomenological

**FitOpt Structure**: Centralized configuration management for:
- Kinematic cuts (pT, eta, centrality, MVA, DCA)
- Variable definitions and binning
- Output file paths and naming conventions
- Fitting options (CUDA support, Minos, Hesse)

## Common Development Commands

### Running the Analysis

**IMPORTANT**: Always use ROOT6.24 command before running root:

```bash
# 1. Data skimming
cd Macro/skim/
ROOT6.24 && root -l -b -q 'FlexibleFlattener.cpp(0,-1,0,1,"input_path","output_name")'
# Parameters: (start_entry, end_entry, idx, isMC, input_path, output_name)

# 2. Create RooDataSet
ROOT6.24 && root -l -b -q 'DStarRDSMaker.cpp(true,false,false,"input_path")'
# Parameters: (isMC, isD0, isPP, input_path)

# 3. Run fitting
cd Macro/fit/testRun/
ROOT6.24 && root -l -b -q 'your_macro.cpp'

# 4. Debug compilation and test JSON parameters
ROOT6.24 && root -l -b -q 'DStarAnalysisV2.cpp'
```

### Batch Job Submission

```bash
# HTCondor submission for large datasets
cd Macro/skim/
condor_submit condor_submit.sub

cd Macro/fit/testRun/
condor_submit condor_submit_DStar.sub
```

### Example Macro Structure

Macros should include these headers and follow this pattern:
```cpp
#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"

void YourMacro(bool doReFit = false, bool plotFit = true, bool useCUDA = true) {
    FitOpt opt;
    ConfigureDStarDataFitOpt(opt); // or provide your own configuration
    
    // Set kinematic ranges
    opt.pTMin = 4.0; opt.pTMax = 100.0;
    opt.cosMin = -2.0; opt.cosMax = 2.0;
    
    // Define fitting parameters using parameter makers
    auto params = DStarParamMaker1({0},{0});
    
    // Load data
    DataLoader loader("path/to/data.root");
    loader.loadRooDataSet("datasetHX");
    
    // Perform fit
    MassFitter fitter(opt.name, opt.massVar, opt.massMin, opt.massMax);
    if(doReFit) {
        fitter.PerformFit(opt, loader.getDataSet(), true, "", "", 
                         params[{0,0}].first, params[{0,0}].second);
    }
    
    // Generate plots
    PlotManager plotManager(opt, opt.outputDir, opt.outputFile, "plots/");
    plotManager.DrawFittedModel(true);
}
```

## Configuration Management

### FitOpt Presets
- `ConfigureDStarDataFitOpt()` - Standard D* meson analysis baseline
- Customize further by overriding members on `FitOpt`

Key configuration options in `Opt.h`:
- Kinematic variables: `pTMin/Max`, `etaMin/Max`, `cosMin/Max`
- Selection cuts: `mvaMin`, `dcaMin/Max`, `cutExpr`
- Output paths: `outputDir`, `outputFile`, `subDir`
- Fitting flags: `useCUDA`, `doFit`, `useMinos`, `useHesse`

## Data Formats

- **Input**: ROOT ntuples with physics variables (pT, eta, mass, MVA scores)
- **Intermediate**: RooDataSet files for fitting framework
- **Output**: ROOT files with fit results, plots (PDF), efficiency maps

## Dependencies

- ROOT 6.24+ (required)
- RooFit (included with ROOT)
- HTCondor (for batch processing)
- CUDA (optional, for GPU-accelerated fitting)

## Important Notes

- This framework is designed for CMS heavy-ion physics analysis
- Uses template-based design for flexibility across different PDF combinations
- Supports both single-thread and batch processing via HTCondor
- Includes CMS publication-ready plotting styles and formatting
- CUDA acceleration available for computationally intensive fits
