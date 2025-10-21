# 🔧 Environment Setup Guide

> **For AI Agents & Users**: Essential environment configuration for D* analysis

---

## 🎯 Quick Setup

```bash
# 1. Setup ROOT (REQUIRED!)
ROOT6.24

# 2. Verify ROOT is working
root --version
# Expected: ROOT Version 6.24 or later

# 3. Navigate to repository
cd /home/jun502s/DstarAna/DStarAnalysis

# 4. You're ready!
```

---

## 📋 Prerequisites

### Required Software

| Software | Version | Purpose |
|----------|---------|---------|
| **ROOT** | 6.24+ | Data analysis framework |
| **RooFit** | (included in ROOT) | Fitting framework |
| **C++ Compiler** | C++17 compatible | Code compilation |
| **Python** | 3.x (optional) | Helper scripts |
| **bash** | Any | Shell scripts |

### System Requirements
- OS: Linux (CentOS, Ubuntu, etc.)
- Disk Space: ~50GB for data files
- RAM: 8GB+ recommended

---

## 🚀 ROOT Setup (CRITICAL!)

### Method 1: Using Alias (Recommended on lxplus/CMS)

```bash
# This is the standard way on this system
ROOT6.24

# Verify
root --version
which root
```

**What `ROOT6.24` does**:
- Sources ROOT environment (`thisroot.sh`)
- Sets up `$ROOTSYS`, `$PATH`, `$LD_LIBRARY_PATH`
- Enables ROOT libraries and RooFit

### Method 2: Manual Setup (Alternative)

```bash
# If ROOT6.24 alias is not available
source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.24.00/x86_64-centos7-gcc48-opt/bin/thisroot.sh

# Or your local ROOT installation
source /path/to/root/bin/thisroot.sh
```

### Method 3: Automatic in Shell (Add to ~/.bashrc)

```bash
# Add this to your ~/.bashrc for automatic setup
alias ROOT6.24='source /path/to/root/bin/thisroot.sh'

# Then reload
source ~/.bashrc
```

### Verify ROOT Setup

```bash
# Check ROOT version
root --version

# Check ROOT environment variables
echo $ROOTSYS
echo $PATH | grep root
echo $LD_LIBRARY_PATH | grep root

# Test ROOT
root -l -b -q
# Should enter ROOT prompt without errors
```

---

## 🔍 Environment Variables

### Essential Variables (Set by ROOT6.24)

```bash
# ROOT installation directory
export ROOTSYS=/path/to/root

# Add ROOT to PATH
export PATH=$ROOTSYS/bin:$PATH

# Add ROOT libraries to library path
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

# Python bindings (if needed)
export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH
```

### Project-Specific Variables (Optional)

```bash
# Project root
export DSTAR_ANALYSIS_DIR=/home/jun502s/DstarAna/DStarAnalysis

# Data directory
export DSTAR_DATA_DIR=$DSTAR_ANALYSIS_DIR/Data

# Results directory
export DSTAR_RESULTS_DIR=$DSTAR_ANALYSIS_DIR/Results
```

---

## 📝 Compilation & Execution

### Method 1: ROOT Interactive (Most Common)

```bash
# Setup ROOT first
ROOT6.24

# Run analysis script
cd Analysis/PP
root -l -b -q DStarAnalysisPP.cpp

# With arguments
root -l -b -q 'DStarAnalysisPP.cpp(true, false, "pp")'
```

### Method 2: ACLiC (Compiled)

```bash
# Compile and execute
root -l -b -q 'DStarAnalysisPP.cpp++'
# The '++' compiles the code first
```

### Method 3: Standalone Compilation

```bash
# Using g++ directly (if needed)
g++ -o myAnalysis DStarAnalysisPP.cpp \
    `root-config --cflags --libs` \
    -lRooFit -lRooFitCore

# Run
./myAnalysis
```

---

## 🎨 Common ROOT Commands

### Running Analysis

```bash
# Interactive mode (with graphics)
root -l DStarAnalysisPP.cpp

# Batch mode (no graphics, faster)
root -l -b -q DStarAnalysisPP.cpp

# With output redirection
root -l -b -q DStarAnalysisPP.cpp > output.log 2>&1
```

### Working with ROOT Files

```bash
# Open ROOT file
root -l Data/RDS/PP/Data/myfile.root

# Inside ROOT:
# .ls              - list contents
# tree->Print()    - print tree structure
# .q               - quit
```

### Debugging

```bash
# Run with more verbose output
root -l DStarAnalysisPP.cpp 2>&1 | tee debug.log

# Check for memory leaks (if valgrind available)
valgrind --leak-check=full root -l -b -q DStarAnalysisPP.cpp
```

---

## 🔧 Troubleshooting

### Issue: "ROOT6.24: command not found"

**Solution 1**: Setup manually
```bash
source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.24.00/x86_64-centos7-gcc48-opt/bin/thisroot.sh
```

**Solution 2**: Find ROOT installation
```bash
# Search for ROOT
find /cvmfs -name "thisroot.sh" 2>/dev/null | grep 6.24

# Or
ls /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/
```

### Issue: "Error in &lt;TFile::TFile&gt;: file does not exist"

**Check**:
```bash
# Verify file path
ls -la Data/RDS/PP/Data/*.root

# Check working directory
pwd
```

**Solution**: Use absolute paths or check `cd` to correct directory

### Issue: "undefined reference to RooFit"

**Solution**: Ensure RooFit libraries are linked
```bash
# In ROOT
.L RooFit.h
gSystem->Load("libRooFit");
gSystem->Load("libRooFitCore");
```

### Issue: "X11 connection rejected"

**Solution**: Use batch mode
```bash
# Add -b flag
root -l -b -q DStarAnalysisPP.cpp
```

### Issue: Segmentation fault

**Debug**:
```bash
# Run with gdb
gdb --args root -l -b DStarAnalysisPP.cpp
# Inside gdb: run, then bt (backtrace) after crash
```

---

## 🐍 Python Environment (Optional)

If using Python scripts:

```bash
# Setup Python environment
python3 -m venv venv
source venv/bin/activate

# Install dependencies (if needed)
pip install numpy matplotlib uproot awkward
```

---

## 🔄 Typical Workflow Commands

### Full Analysis Pipeline

```bash
# 1. Setup environment
ROOT6.24

# 2. Navigate to project
cd /home/jun502s/DstarAna/DStarAnalysis

# 3. Run preprocessing (if needed)
cd Preprocessing/Common
root -l -b -q DStarRDSMakerImproved.cpp

# 4. Generate corrections (if needed)
cd ../../Corrections/Generators/PP
root -l -b -q EfficiencyMaps_PP.cpp

# 5. Run analysis
cd ../../../Analysis/PP
root -l -b -q DStarAnalysisPP.cpp

# 6. Check results
ls -la ../../Results/PP/Plots/
```

---

## 📊 Environment Check Script

Create this script to check your environment:

```bash
#!/bin/bash
# check_environment.sh

echo "=== D* Analysis Environment Check ==="
echo ""

# Check ROOT
echo "ROOT:"
if command -v root &> /dev/null; then
    echo "  ✓ ROOT is available"
    root --version | head -1
else
    echo "  ✗ ROOT not found! Run: ROOT6.24"
fi
echo ""

# Check environment variables
echo "Environment Variables:"
if [ -n "$ROOTSYS" ]; then
    echo "  ✓ ROOTSYS: $ROOTSYS"
else
    echo "  ✗ ROOTSYS not set"
fi

if [ -n "$LD_LIBRARY_PATH" ]; then
    if echo "$LD_LIBRARY_PATH" | grep -q root; then
        echo "  ✓ ROOT in LD_LIBRARY_PATH"
    else
        echo "  ✗ ROOT not in LD_LIBRARY_PATH"
    fi
fi
echo ""

# Check project directory
echo "Project Directory:"
if [ -d "/home/jun502s/DstarAna/DStarAnalysis" ]; then
    echo "  ✓ Found: /home/jun502s/DstarAna/DStarAnalysis"
else
    echo "  ✗ Project directory not found"
fi
echo ""

# Check data
echo "Data Directory:"
if [ -d "Data/RDS/PP" ]; then
    DATA_COUNT=$(find Data/RDS -name "*.root" 2>/dev/null | wc -l)
    echo "  ✓ Found $DATA_COUNT ROOT files"
else
    echo "  ✗ Data directory not found"
fi
echo ""

echo "=== Check Complete ==="
```

Usage:
```bash
chmod +x check_environment.sh
./check_environment.sh
```

---

## 💡 Tips for AI Agents

### Before Running Any Analysis:

1. **Always setup ROOT first**:
   ```bash
   ROOT6.24
   ```

2. **Check current directory**:
   ```bash
   pwd
   ```

3. **Use batch mode** (no X11 needed):
   ```bash
   root -l -b -q script.cpp
   ```

4. **Redirect output for logging**:
   ```bash
   root -l -b -q script.cpp > output.log 2>&1
   ```

### Common Patterns:

```bash
# Pattern 1: Run analysis
ROOT6.24 && cd Analysis/PP && root -l -b -q DStarAnalysisPP.cpp

# Pattern 2: Check results
ls -la Results/PP/Plots/*.pdf | tail -10

# Pattern 3: Quick test
ROOT6.24 && root -l -b -q -e 'cout << "ROOT is working!" << endl'
```

---

## 🎯 Quick Reference Card

```
┌─────────────────────────────────────────────────────┐
│  D* Analysis Environment Quick Reference            │
├─────────────────────────────────────────────────────┤
│  Setup ROOT:        ROOT6.24                        │
│  Check ROOT:        root --version                  │
│  Run Analysis:      root -l -b -q script.cpp        │
│  Project Dir:       /home/jun502s/DstarAna/...      │
│  Batch Mode:        Always use -b flag              │
│  PP Analysis:       Analysis/PP/DStarAnalysisPP.cpp │
│  PbPb Analysis:     Analysis/PbPb/...PbPb.cpp       │
└─────────────────────────────────────────────────────┘
```

---

## 📚 Related Documentation

- **[AI_AGENT_GUIDE.md](AI_AGENT_GUIDE.md)** - AI agent navigation guide
- **[QUICK_START.md](QUICK_START.md)** - Quick start guide
- **[REPOSITORY_STRUCTURE.md](REPOSITORY_STRUCTURE.md)** - Repository structure

---

**Last Updated**: 2025-10-14  
**System**: lxplus / CMS computing cluster  
**ROOT Version**: 6.24+

