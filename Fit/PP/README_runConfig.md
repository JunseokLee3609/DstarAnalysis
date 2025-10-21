# PP Analysis Run Configurations

Quick guide for running PP analysis with pre-configured settings.

---

## 🚀 Quick Start

### Option 1: Interactive Menu (Recommended)

```bash
cd Analysis/PP

# Show available configurations
./runConfig.sh

# Run specific configuration
./runConfig.sh 1    # Quick test
./runConfig.sh 2    # cosTheta scan
./runConfig.sh all  # Run all tests
```

### Option 2: Simple Script (Easy to edit)

```bash
cd Analysis/PP

# Edit parameters in the script
vim runConfig_simple.sh

# Run
./runConfig_simple.sh
```

---

## 📋 Available Configurations

### Config 1: Quick Test
**Purpose**: Fast single-bin test  
**Use Case**: Testing setup, debugging  
**Time**: ~2-5 minutes

```bash
./runConfig.sh 1
```

**Parameters**:
- pT: 5-7 GeV
- cos: 0.0-0.2
- DCA: ON
- Plots: ON

---

### Config 2: cosTheta Scan
**Purpose**: Full angular distribution analysis  
**Use Case**: Polarization measurement  
**Time**: ~30-60 minutes

```bash
./runConfig.sh 2
```

**Bins**: 8 bins from cos=-0.8 to cos=0.8
- Runs in parallel (8 jobs)
- Creates separate logs per bin

---

### Config 3: pT Scan
**Purpose**: Differential cross-section vs pT  
**Use Case**: pT-dependent analysis  
**Time**: ~20-40 minutes

```bash
./runConfig.sh 3
```

**Bins**: 
- 5-7 GeV
- 7-10 GeV
- 10-15 GeV
- 15-20 GeV
- 20-30 GeV

---

### Config 4: MC Test
**Purpose**: Monte Carlo validation  
**Use Case**: Efficiency, acceptance studies  
**Time**: ~5-10 minutes

```bash
./runConfig.sh 4
```

**Parameters**:
- Same as Quick Test but with MC flag ON

---

### Config 5: DCA Only
**Purpose**: Prompt/non-prompt separation  
**Use Case**: DCA template fitting  
**Time**: ~5-10 minutes

```bash
./runConfig.sh 5
```

**Parameters**:
- Full pT range (5-100 GeV)
- Full cos range (-2 to 2)
- Mass fit: OFF
- DCA fit: ON

---

### Config 6: Custom
**Purpose**: User-defined configuration  
**Use Case**: Special analysis needs  
**Time**: Varies

```bash
# Edit runConfig.sh, function run_custom()
vim runConfig.sh

# Run
./runConfig.sh 6
```

---

## 🎨 Parameter Guide

### Analysis Arguments

```cpp
DStarAnalysisPP.cpp(
    doReFit,        // 0=use existing, 1=refit
    doDCA,          // 0=skip DCA, 1=do DCA
    plotFit,        // 0=no plots, 1=make plots
    useCUDA,        // 0=CPU only, 1=use CUDA
    pTMin,          // Minimum pT (GeV)
    pTMax,          // Maximum pT (GeV)
    cosMin,         // Minimum cosTheta
    cosMax,         // Maximum cosTheta
    centralityMin,  // Not used in PP (set 0)
    centralityMax,  // Not used in PP (set 100)
    parameterFile,  // JSON parameter file path
    isMC            // 0=Data, 1=MC
)
```

### Example Values

```bash
# Quick Data Test
root -l -b -q 'DStarAnalysisPP.cpp(0,1,1,1,5,7,0,0.2,0,100,"../Common/Parameters/pp/params.json",0)'

# MC Validation
root -l -b -q 'DStarAnalysisPP.cpp(0,1,1,1,5,100,-2,2,0,100,"../Common/Parameters/pp/params.json",1)'

# DCA Only
root -l -b -q 'DStarAnalysisPP.cpp(0,1,0,1,5,100,-2,2,0,100,"../Common/Parameters/pp/params_dca.json",0)'
```

---

## 📊 Output Locations

### Logs
```
Analysis/PP/logs/
├── quick_test_20251014_153045.log
├── costheta_scan_-0.8_-0.6_20251014_153100.log
└── pt_scan_5_7_20251014_153200.log
```

### Results
```
Results/PP/
├── Fits/          # Fit results (.root)
├── Plots/         # Plots (.pdf, .png)
└── Tables/        # Summary tables (.csv)
```

---

## 🔧 Customization

### Method 1: Edit runConfig.sh

```bash
vim runConfig.sh

# Find run_custom() function
# Edit parameters
DO_REFIT=1
PT_MIN=10
PT_MAX=20
# ...

# Save and run
./runConfig.sh 6
```

### Method 2: Use runConfig_simple.sh

```bash
vim runConfig_simple.sh

# Edit parameters section
PT_MIN=10
PT_MAX=20
COS_MIN=-0.5
COS_MAX=0.5

# Save and run
./runConfig_simple.sh
```

### Method 3: Direct ROOT Command

```bash
root -l -b -q 'DStarAnalysisPP.cpp(1,1,1,1,10,20,-0.5,0.5,0,100,"../Common/Parameters/pp/params.json",0)'
```

---

## 💡 Tips

### Run Multiple Configs in Background

```bash
# Start multiple jobs
./runConfig.sh 1 &
./runConfig.sh 4 &
./runConfig.sh 5 &

# Check running jobs
jobs

# Wait for all to finish
wait
```

### Check Progress

```bash
# Monitor log file
tail -f logs/quick_test_*.log

# Check results
ls -lrt ../../Results/PP/Plots/ | tail -10
```

### Quick Validation

```bash
# Run quick test first
./runConfig.sh 1

# Check if it worked
ls ../../Results/PP/Plots/

# If OK, run full analysis
./runConfig.sh all
```

---

## 🐛 Troubleshooting

### "ROOT not found"
```bash
ROOT6.24
./runConfig.sh 1
```

### "Parameter file not found"
```bash
# Check parameter files
ls -la ../Common/Parameters/pp/

# Use absolute path if needed
PARAM_FILE="/full/path/to/parameters.json"
```

### "No output files"
```bash
# Check logs
cat logs/*.log | grep -i error

# Check if analysis ran
cat logs/*.log | grep -i "Analysis Complete"
```

---

## 📚 Related Documentation

- **[DStarAnalysisPP.cpp](DStarAnalysisPP.cpp)** - Main analysis script
- **[../Common/Parameters/pp/](../Common/Parameters/pp/)** - Parameter files
- **[ENVIRONMENT.md](../../ENVIRONMENT.md)** - Environment setup
- **[AI_AGENT_GUIDE.md](../../AI_AGENT_GUIDE.md)** - Quick reference

---

**Last Updated**: 2025-10-14  
**Maintained By**: D* PP Analysis Team

