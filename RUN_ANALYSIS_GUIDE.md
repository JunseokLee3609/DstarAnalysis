# 🚀 Quick Analysis Run Guide

> **Fast track**: From zero to results in 5 minutes

---

## ⚡ Super Quick Start

```bash
# 1. Setup environment
ROOT6.24

# 2. Run PP analysis
cd Analysis/PP
./runConfig.sh 1

# 3. Check results
ls -la ../../Results/PP/Plots/
```

Done! 🎉

---

## 📋 Available Run Configurations

### PP Analysis (`Analysis/PP/`)

```bash
cd Analysis/PP

# Show menu
./runConfig.sh

# Quick test (5 min)
./runConfig.sh 1

# Full cosTheta scan (30-60 min)
./runConfig.sh 2

# pT scan (20-40 min)
./runConfig.sh 3

# MC validation (5-10 min)
./runConfig.sh 4

# DCA only (5-10 min)
./runConfig.sh 5

# Custom (edit script first)
./runConfig.sh 6

# Run all tests
./runConfig.sh all
```

### PbPb Analysis (`Analysis/PbPb/`)

```bash
cd Analysis/PbPb

# Show menu
./runConfig.sh

# Quick test (5 min)
./runConfig.sh 1

# Centrality scan (20-30 min)
./runConfig.sh 2

# cosTheta scan (0-10% centrality, 30-60 min)
./runConfig.sh 3

# pT scan (0-10% centrality, 15-25 min)
./runConfig.sh 4

# MC validation (5-10 min)
./runConfig.sh 5

# Full 2D scan (Cent × pT, 60-120 min)
./runConfig.sh 6

# Custom (edit script first)
./runConfig.sh 7
```

---

## 🎨 Simple Configuration (Easy to Edit)

### PP Simple Runner

```bash
cd Analysis/PP

# Edit parameters
vim runConfig_simple.sh

# Change these:
PT_MIN=5
PT_MAX=7
COS_MIN=0.0
COS_MAX=0.2

# Run
./runConfig_simple.sh
```

### PbPb Simple Runner

Coming soon... (or copy from PP and modify centrality)

---

## 📊 Configuration Comparison

| Config | PP | PbPb | Purpose | Time |
|--------|----|----|---------|------|
| 1 | ✓ | ✓ | Quick test | 5 min |
| 2 | cosθ scan | Centrality scan | Differential | 30 min |
| 3 | pT scan | cosθ scan (0-10%) | Kinematic | 20-60 min |
| 4 | MC test | pT scan (0-10%) | Validation | 5-20 min |
| 5 | DCA only | MC test | Separation | 5-10 min |
| 6 | Custom | 2D scan (Cent×pT) | Full analysis | Varies |

---

## 🔍 What Each Config Does

### Config 1: Quick Test
- **Purpose**: Verify setup is working
- **Output**: Single fit plot, DCA plot
- **Time**: 5 minutes
- **Best for**: Testing, debugging

### Config 2: PP cosTheta Scan / PbPb Centrality Scan
- **PP**: 8 angular bins
- **PbPb**: 4 centrality bins
- **Purpose**: Main physics analysis
- **Output**: Multiple fit plots
- **Time**: 20-30 minutes

### Config 3-6: See README_runConfig.md

---

## 📁 Output Locations

```
Results/
├── PP/
│   ├── Fits/           # .root files with fit results
│   ├── Plots/          # .pdf, .png plots
│   └── Tables/         # .csv summary tables
└── PbPb/
    ├── Fits/
    ├── Plots/
    └── Tables/

Analysis/PP/logs/       # PP run logs
Analysis/PbPb/logs/     # PbPb run logs
```

---

## 💡 Usage Tips

### Parallel Runs

```bash
# Run multiple configs in background
cd Analysis/PP
./runConfig.sh 1 &
./runConfig.sh 4 &
./runConfig.sh 5 &

# Check running jobs
jobs

# Wait for all
wait

# Check results
ls -lrt ../../Results/PP/Plots/ | tail -20
```

### Monitor Progress

```bash
# Watch log file
tail -f logs/quick_test_*.log

# Check latest results
ls -lrt ../../Results/PP/Plots/ | tail -10

# Count completed fits
ls ../../Results/PP/Plots/*.pdf | wc -l
```

### Quick Validation

```bash
# 1. Quick test first
./runConfig.sh 1

# 2. Check if it worked
ls ../../Results/PP/Plots/

# 3. If OK, run full analysis
./runConfig.sh all
```

---

## 🐛 Troubleshooting

### "ROOT not found"
```bash
ROOT6.24
./runConfig.sh 1
```

### No output files
```bash
# Check log for errors
cat logs/*.log | grep -i error

# Verify analysis completed
cat logs/*.log | grep -i "complete"
```

### "Parameter file not found"
```bash
# Check parameter files exist
ls -la ../Common/Parameters/pp/
ls -la ../Common/Parameters/PbPb/

# Use absolute path if needed
vim runConfig.sh  # Edit PARAM_FILE path
```

---

## 📚 Detailed Documentation

- **[Analysis/PP/README_runConfig.md](Analysis/PP/README_runConfig.md)** - PP config details
- **[ENVIRONMENT.md](ENVIRONMENT.md)** - Environment setup
- **[QUICK_START.md](QUICK_START.md)** - General quick start
- **[AI_AGENT_GUIDE.md](AI_AGENT_GUIDE.md)** - Quick reference

---

## 🎯 Common Workflows

### New to the Code? Start Here:

```bash
# 1. Setup
ROOT6.24
cd Analysis/PP

# 2. Quick test (verify everything works)
./runConfig.sh 1

# 3. Check results
ls -la ../../Results/PP/Plots/

# 4. Read the plot!
```

### Production Run:

```bash
# PP full analysis
cd Analysis/PP
./runConfig.sh all > full_run.log 2>&1 &

# PbPb full analysis
cd Analysis/PbPb
./runConfig.sh all > full_run.log 2>&1 &
```

### Custom Analysis:

```bash
# Method 1: Edit simple script
cd Analysis/PP
vim runConfig_simple.sh
./runConfig_simple.sh

# Method 2: Edit run_custom function
vim runConfig.sh  # Find run_custom()
./runConfig.sh 6

# Method 3: Direct ROOT command
root -l -b -q 'DStarAnalysisPP.cpp(1,1,1,1,10,20,-0.5,0.5,0,100,"../Common/Parameters/pp/params.json",0)'
```

---

## ⏱️ Estimated Run Times

| Configuration | PP Time | PbPb Time |
|---------------|---------|-----------|
| Quick Test | 5 min | 5 min |
| cosθ Scan | 30-60 min | 30-60 min (0-10%) |
| pT Scan | 20-40 min | 15-25 min (0-10%) |
| Centrality Scan | N/A | 20-30 min |
| 2D Scan | N/A | 60-120 min |
| All configs | 60-120 min | 80-150 min |

*Times vary based on data size and system load*

---

## 🎓 Advanced: Understanding Arguments

```cpp
DStarAnalysisPP.cpp(
    doReFit,        // 0 or 1
    doDCA,          // 0 or 1
    plotFit,        // 0 or 1
    useCUDA,        // 0 or 1
    pTMin,          // GeV
    pTMax,          // GeV
    cosMin,         // -1 to 1
    cosMax,         // -1 to 1
    centralityMin,  // 0-100 (PbPb only, use 0 for PP)
    centralityMax,  // 0-100 (PbPb only, use 100 for PP)
    parameterFile,  // JSON file path
    isMC            // 0 or 1
)
```

**Example**:
```bash
# Data fit, pT 5-7 GeV, cos 0-0.2, with DCA
root -l -b -q 'DStarAnalysisPP.cpp(0,1,1,1,5,7,0,0.2,0,100,"../Common/Parameters/pp/params.json",0)'
```

---

**Last Updated**: 2025-10-14  
**Quick Help**: Run `./runConfig.sh` without arguments to see available options

