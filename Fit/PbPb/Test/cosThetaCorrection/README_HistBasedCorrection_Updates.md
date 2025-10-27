# HistBasedCorrection.cpp Updates

## Summary
Updated `HistBasedCorrection.cpp` to support centrality-dependent efficiency corrections using JSON-based dataset loading pattern from `FlatEffAccCalculator_PbPb.cpp` and improved file management from `PlotRho00VsPtPbPb.cpp`.

## Key Changes

### 1. JSON Catalog Dataset Loading
Added `LoadDatasetFromJSON()` function (lines 579-620) to load MC files from JSON catalog:
```cpp
TFile *LoadDatasetFromJSON(const char *datasetKey,
                           const char *jsonPath,
                           const char *mode = "READ");
```
- Reads dataset configuration from `/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json`
- Supports dataset keys like "PbPb_MC_Oct22", "PbPb_MC_Aug22", etc.
- Returns TFile pointer ready for use

### 2. Centrality Support in BuildEfficiencyMap()
Updated function signature (line 622) to include centrality parameters:
```cpp
TH2D* BuildEfficiencyMap(ProductionType type,
                         const std::string& mcFilePath,
                         const std::vector<std::pair<double, double>>& ptBins,
                         const std::vector<double>& cosBins,
                         bool useAbsTheta,
                         double directionY,
                         std::vector<TH1D*>* genProjections,
                         std::vector<TH1D*>* matchProjections,
                         double centralityMin = 0.0,
                         double centralityMax = 100.0);
```

Added centrality cuts in:
- **GEN loop** (lines 722-726): Filters generated particles by centrality
- **RECO loop** (lines 751-755): Filters reconstructed particles by centrality

### 3. Centrality Support in ExtractMcGenRho00()
Updated function signature (line 162) to include centrality:
```cpp
std::map<ProductionType, McGenRho00Result>
ExtractMcGenRho00(const std::string& mcFilePath,
                  const std::vector<std::pair<double, double>>& ptBins,
                  const std::string& outputDir = std::string(),
                  double maxAbsRapidity = 1.0,
                  bool useAbsTheta = true,
                  double centralityMin = 0.0,
                  double centralityMax = 100.0);
```

Added centrality filtering (lines 275-279) in the GEN entry loop.

### 4. Updated Main Function HistBasedCorrection()
Modified signature (line 1295) to accept centrality and dataset parameters:
```cpp
void HistBasedCorrection(bool isMC = false, 
                         bool useAbsTheta = true,
                         const std::string& mcDatasetKey = "PbPb_MC_Oct22",
                         const std::string& datasetCatalog = "/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json",
                         double centralityMin = 0.0,
                         double centralityMax = 100.0);
```

Key features:
- Loads MC file from JSON catalog (lines 1341-1354)
- Creates centrality-specific output directory (lines 1307-1309)
- Passes centrality to efficiency map building (line 1381)
- Passes centrality to MC GEN rho00 extraction (line 1366)

### 5. New Wrapper Function
Added `HistBasedCorrectionWithCentrality()` (lines 2705-2722):
```cpp
void HistBasedCorrectionWithCentrality(bool isMC = false, 
                                       bool useAbsTheta = true,
                                       int centralityMin = 0,
                                       int centralityMax = 100,
                                       const std::string& mcDatasetKey = "PbPb_MC_Oct22");
```

Provides simplified interface for centrality-binned analysis with informative logging.

## Usage Examples

### Example 1: Run for centrality 0-10%
```cpp
HistBasedCorrectionWithCentrality(false, true, 0, 10, "PbPb_MC_Oct22");
```

### Example 2: Run for centrality 30-50%
```cpp
HistBasedCorrectionWithCentrality(false, true, 30, 50, "PbPb_MC_Oct22");
```

### Example 3: Run with direct main function
```cpp
HistBasedCorrection(false, true, "PbPb_MC_Oct22", 
                   "/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json",
                   0.0, 10.0);
```

## Output Structure
Results are saved to centrality-specific directories:
```
splot/
  └── <subDir>_cent0to10/
      ├── efficiency_map_prompt.png
      ├── efficiency_map_nonprompt.png
      ├── splot_prompt_corrected_yield_pt_*.png
      ├── splot_nonprompt_corrected_yield_pt_*.png
      ├── rho00_vs_pT_prompt_nonprompt.png
      └── yield_summary.csv
```

## Prompt Fraction Handling

**IMPORTANT**: Prompt fraction separation is currently **DISABLED**. All yields use `promptFraction = 1.0`:

- Lines 1649-1698: Prompt fraction scan is commented out
- Lines 1809-1907: Prompt fraction vs cos#theta plots are skipped
- Both Prompt and NonPrompt channels will have `fraction = 1.0` applied

This means:
- **Prompt channel**: Gets 100% of the raw yield (rawYield × 1.0)
- **NonPrompt channel**: Gets 0% of the raw yield (rawYield × 0.0)

To re-enable prompt/nonprompt separation, uncomment:
1. The `RunPromptFractionScan()` call block (lines 1654-1689)
2. The prompt fraction summary plot section (lines 1810-1906)

## Benefits

1. **Centrality-dependent corrections**: Properly accounts for centrality dependence in efficiency
2. **JSON-based file management**: Centralized dataset configuration, easier to maintain
3. **Pattern consistency**: Follows established patterns from other analysis codes
4. **Flexible interface**: Both low-level and high-level interfaces available
5. **Better organization**: Centrality-specific output directories prevent file conflicts
6. **Simplified workflow**: No DCA fitting needed when prompt fraction = 1.0

## Testing
- No compilation errors detected
- All function signatures are backward compatible (default parameters provided)
- Follows ROOT and C++ best practices

## Notes
- Centrality is expected in percentage (0-100)
- Input centrality variable from tree is multiplied by 0.5 to convert to percentage
- Default centrality range is 0-100% (no centrality selection)
- MC dataset keys must exist in `/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json`

