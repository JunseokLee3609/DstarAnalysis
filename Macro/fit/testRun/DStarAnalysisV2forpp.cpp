#include "../DStarFitConfig.h"
#include "../MassFitterV2.h"
#include "../DataLoader.h"
// #include "../PlotManager.h"  // Disabled - has compilation issues
#include "../EnhancedPlotManager.h"  // Use our enhanced plotter instead
#include "../SimpleParameterLoader.h"  // External parameter loading
#include "../JSONParameterUtils.h"     // JSON parameter utilities (includes JSONParameterLoader)
#include "../../Tools/ConfigManager.h"
#include "../ParameterDebugUtils.h"     // Centralized parameter printing utilities
#include "../DCAFitter.h"               // DCA template fitter
#include <fstream>
#include <algorithm>
#include <cctype>

// ===== PARAMETER PRINTING UTILITY FUNCTIONS =====

// Template function to get PDF type names
template<typename T> std::string GetPDFTypeName() { return "Unknown"; }

// Signal PDF type names
template<> std::string GetPDFTypeName<PDFParams::GaussianParams>() { return "Gaussian"; }
template<> std::string GetPDFTypeName<PDFParams::DoubleGaussianParams>() { return "DoubleGaussian"; }
template<> std::string GetPDFTypeName<PDFParams::CrystalBallParams>() { return "CrystalBall"; }
template<> std::string GetPDFTypeName<PDFParams::DBCrystalBallParams>() { return "DBCrystalBall"; }
template<> std::string GetPDFTypeName<PDFParams::VoigtianParams>() { return "Voigtian"; }
template<> std::string GetPDFTypeName<PDFParams::BreitWignerParams>() { return "BreitWigner"; }

// Background PDF type names
template<> std::string GetPDFTypeName<PDFParams::ExponentialBkgParams>() { return "Exponential"; }
template<> std::string GetPDFTypeName<PDFParams::ChebychevBkgParams>() { return "Chebychev"; }
template<> std::string GetPDFTypeName<PDFParams::PhenomenologicalParams>() { return "Phenomenological"; }
template<> std::string GetPDFTypeName<PDFParams::Phenomenological2Params>() { return "Phenomenological2"; }
template<> std::string GetPDFTypeName<PDFParams::PolynomialBkgParams>() { return "Polynomial"; }
template<> std::string GetPDFTypeName<PDFParams::ThresholdFuncParams>() { return "ThresholdFunction"; }
template<> std::string GetPDFTypeName<PDFParams::ExpErfBkgParams>() { return "ExpErf"; }
template<> std::string GetPDFTypeName<PDFParams::DstBkgParams>() { return "DstBg"; }
template<> std::string GetPDFTypeName<PDFParams::DstD0Params>() { return "DstD0Bg"; }


/**
 * @brief Modern D* meson analysis using the new modular framework
 * 
 * This macro demonstrates how to use the improved MassFitterV2 with 
 * configurable kinematic bins and centrality support (dummy for now).
 */

// ===== PARAMETER DEBUGGING FUNCTIONS =====

// Function to print PDF parameters for debugging
void PrintBinParameters(const KinematicBin& bin, const DStarBinParameters& params, const std::string& title = "") {
    // Thin wrapper to centralized utility for consistency
    ParameterDebug::PrintBinParameters(bin, params, title);
}

// Function to print all parameters in the config
void PrintAllConfigParameters(const DStarFitConfig& config, const std::string& title = "") {
    if (!title.empty()) {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "📋 " << title << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }
    
    auto allBins = config.GetAllKinematicBins();
    std::cout << "Total bins: " << allBins.size() << std::endl;
    
    for (const auto& bin : allBins) {
        try {
            auto params = config.GetParametersForBin(bin);
            ParameterDebug::PrintBinParameters(bin, params);
        } catch (const std::exception& e) {
            std::cout << "❌ Error getting parameters for bin " << bin.GetBinName() 
                      << ": " << e.what() << std::endl;
        }
    }
}

// Function to compare parameters before and after JSON loading
void CompareParameters(const DStarFitConfig& configBefore, const DStarFitConfig& configAfter, 
                      const std::string& binName = "") {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "🔍 PARAMETER COMPARISON" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    auto binsBefore = configBefore.GetAllKinematicBins();
    auto binsAfter = configAfter.GetAllKinematicBins();
    
    if (binsBefore.size() != binsAfter.size()) {
        std::cout << "⚠️  Warning: Different number of bins! Before: " << binsBefore.size() 
                  << ", After: " << binsAfter.size() << std::endl;
    }
    
    for (size_t i = 0; i < std::min(binsBefore.size(), binsAfter.size()); ++i) {
        const auto& bin = binsBefore[i];
        
        if (!binName.empty() && bin.GetBinName().find(binName) == std::string::npos) {
            continue;
        }
        
        std::cout << "\n📍 Bin: " << bin.GetBinName() << std::endl;
        
        try {
            auto paramsBefore = configBefore.GetParametersForBin(bin);
            auto paramsAfter = configAfter.GetParametersForBin(bin);
            
            bool changed = false;
            
            // Compare PDF types
            if (paramsBefore.signalPdfType != paramsAfter.signalPdfType) {
                std::cout << "  🎯 Signal PDF: " << static_cast<int>(paramsBefore.signalPdfType) 
                          << " → " << static_cast<int>(paramsAfter.signalPdfType) << std::endl;
                changed = true;
            }
            
            if (paramsBefore.backgroundPdfType != paramsAfter.backgroundPdfType) {
                std::cout << "  🔲 Background PDF: " << static_cast<int>(paramsBefore.backgroundPdfType) 
                          << " → " << static_cast<int>(paramsAfter.backgroundPdfType) << std::endl;
                changed = true;
            }
            
            // Compare yield ratios
            if (paramsBefore.nsig_ratio != paramsAfter.nsig_ratio) {
                std::cout << "  📊 nsig_ratio: " << paramsBefore.nsig_ratio 
                          << " → " << paramsAfter.nsig_ratio << std::endl;
                changed = true;
            }
            
            if (paramsBefore.nbkg_ratio != paramsAfter.nbkg_ratio) {
                std::cout << "  📊 nbkg_ratio: " << paramsBefore.nbkg_ratio 
                          << " → " << paramsAfter.nbkg_ratio << std::endl;
                changed = true;
            }
            
            // Compare signal parameters (example for DoubleGaussian)
            if (paramsAfter.signalPdfType == PDFType::DoubleGaussian) {
                const auto& before = paramsBefore.doubleGaussianParams;
                const auto& after = paramsAfter.doubleGaussianParams;
                if (before.mean != after.mean) {
                    std::cout << "  🎯 mean: " << before.mean << " → " << after.mean << std::endl;
                    changed = true;
                }
                if (before.sigma1 != after.sigma1) {
                    std::cout << "  🎯 sigma1: " << before.sigma1 << " → " << after.sigma1 << std::endl;
                    changed = true;
                }
                if (before.sigma2 != after.sigma2) {
                    std::cout << "  🎯 sigma2: " << before.sigma2 << " → " << after.sigma2 << std::endl;
                    changed = true;
                }
                if (before.fraction != after.fraction) {
                    std::cout << "  🎯 fraction: " << before.fraction << " → " << after.fraction << std::endl;
                    changed = true;
                }
            }
            
            // Compare background parameters (example for ThresholdFunction)
            if (paramsAfter.backgroundPdfType == PDFType::ThresholdFunction) {
                const auto& before = paramsBefore.thresholdFuncParams;
                const auto& after = paramsAfter.thresholdFuncParams;
                
                if (before.p0_init != after.p0_init) {
                    std::cout << "  🔲 p0: " << before.p0_init << " → " << after.p0_init << std::endl;
                    changed = true;
                }
                if (before.p1_init != after.p1_init) {
                    std::cout << "  🔲 p1: " << before.p1_init << " → " << after.p1_init << std::endl;
                    changed = true;
                }
            }
            
            if (!changed) {
                std::cout << "  ✅ No changes detected" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "  ❌ Error comparing parameters: " << e.what() << std::endl;
        }
    }
    
    std::cout << std::string(80, '=') << std::endl;
}

// Simplified function to load parameters from JSON file using improved utilities
void LoadParametersFromJSON(DStarFitConfig& config, const std::string& jsonFile) {
    JSONParameterLoader jsonLoader;
    // pp analysis: ignore centrality when matching JSON bins
    jsonLoader.setIgnoreCentralityInMatching(true);
    jsonLoader.loadFromFile(jsonFile);
    
    std::cout << "[JSON Loader] Loading parameters from: " << jsonFile << std::endl;
    
    auto configBins = config.GetAllKinematicBins();
    
    for (const auto& bin : configBins) {
        // Use improved bin identifier for automatic matching
        BinIdentifier binId;
        binId.ptMin = bin.pTMin;
        binId.ptMax = bin.pTMax;
        binId.cosMin = bin.cosMin;
        binId.cosMax = bin.cosMax;
        binId.centralityMin = bin.centralityMin;
        binId.centralityMax = bin.centralityMax;
        
        std::cout << "[JSON Loader] Loading parameters for bin: " << bin.GetBinName() << std::endl;
        
        try {
            // Use utility function for automatic parameter and fixed flags loading (C++14-compatible)
            auto loaded = LoadBinParametersFromJSONWithFixedInfo(jsonLoader, binId);
            DStarBinParameters binParams = loaded.first;
            ParameterFixedInfo fixedInfo = loaded.second;

            // Apply parameters and fixed flags to config
            config.SetParametersForBin(bin, binParams);
            config.SetFixedFlagsForBin(bin, fixedInfo.fixedFlags);

            std::cout << "[JSON Loader] ✅ Successfully loaded parameters and fixed flags for bin: " << bin.GetBinName() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[JSON Loader] ❌ Failed to load parameters for bin " << bin.GetBinName() 
                      << ": " << e.what() << ". Using defaults." << std::endl;
        }
    }
}
void DStarAnalysisV2forpp(bool doReFit = false, bool doDCA = true, bool plotFit = true, bool useCUDA = true,
                     float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                     int centralityMin = 0, int centralityMax = 100, 
                     const std::string& parameterFile = "", bool isMC = false) {
    
    std::cout << "=== D* Meson Analysis V2 ===" << std::endl;
    std::cout << "Using new modular framework with MassFitterV2" << std::endl;
    
    // Create main configuration
    DStarFitConfig config;
    // Enable auto-tuning: switching yield mode adjusts fit method and model
    config.SetYieldModeAutoTuning(false);
    
    // Configure file paths for pp (no centrality)
    config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_ppRef_Aug01_v1.root");
    config.SetMCFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_np_pp_08Sep25_v1.root");
    config.SetDatasetName("datasetHX");
    // Use a pp-specific output subdirectory
    config.SetOutputSubDir("/DStar_ppRef_Analysis_V2/");
    // config.SetFitMethod(FitMethod::Extended);  // Use NLL for better stability with low stats
    // config.SetFitMethod(FitMethod::FixedFromMC);  // Use NLL for better stability with low stats
    config.SetFitMethod(FitMethod::GaussianConstraint);  // Use NLL for better stability with low stats
    
    // Configure fit options
    // Enable auto-tuned Fraction yield mode (auto-switches to NLL and coeff model)
    config.SetUseCUDA(useCUDA);
    config.SetVerbose(false);
    config.SetDoRefit(doReFit);
    
    // Configure selection cuts
    config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
    // config.SetMVACut(0.999);
    // Use |cos(theta*)| for cuts if desired
    config.SetUseAbsCosCuts(true);
    config.SetUseIndependentYields(true);
    
    // Create single kinematic bin for analysis
    KinematicBin currentBin(pTMin, pTMax, cosMin, cosMax, centralityMin, centralityMax);
    
    // Add kinematic ranges (pp: do not add centrality bins)
    config.AddPtBin(pTMin, pTMax);
    config.AddCosBin(cosMin, cosMax);
    // Do NOT add centrality bins for pp analysis
    
    // ===== PARAMETER LOADING SYSTEM =====
    std::cout << "Setting up parameters..." << std::endl;
    
    // Check if external parameter file is provided
    if (!parameterFile.empty()) {
        std::cout << "Loading parameters from external file: " << parameterFile << std::endl;
        
        try {
            // Check file extension to determine loader type
            std::string extension = parameterFile.substr(parameterFile.find_last_of(".") + 1);
            
            if (extension == "json") {
                std::cout << "Using JSON parameter loader..." << std::endl;
                LoadParametersFromJSON(config, parameterFile);
                // Ensure the exact key for currentBin is populated (pp uses arbitrary cent ranges)
                try {
                    JSONParameterLoader directLoader;
                    directLoader.setIgnoreCentralityInMatching(true);
                    directLoader.loadFromFile(parameterFile);
                    BinIdentifier bid;
                    bid.ptMin = currentBin.pTMin;
                    bid.ptMax = currentBin.pTMax;
                    bid.cosMin = currentBin.cosMin;
                    bid.cosMax = currentBin.cosMax;
                    bid.centralityMin = currentBin.centralityMin;
                    bid.centralityMax = currentBin.centralityMax;
                    auto loaded = LoadBinParametersFromJSONWithFixedInfo(directLoader, bid);
                    config.SetParametersForBin(currentBin, loaded.first);
                    config.SetFixedFlagsForBin(currentBin, loaded.second.fixedFlags);
                    std::cout << "[JSON Loader] Populated parameters for EXACT current bin key: "
                              << currentBin.GetBinName() << std::endl;
                } catch (const std::exception& e2) {
                    std::cout << "[JSON Loader] Could not set exact current bin key: " << e2.what() << std::endl;
                }
            } else {
                std::cout << "Using legacy parameter loader..." << std::endl;
                ParameterLoaderUtils::LoadParametersToConfig(config, parameterFile);
            }
            std::cout << "✓ External parameters loaded successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "⚠ Failed to load external parameters: " << e.what() << std::endl;
            std::cout << "Falling back to hardcoded parameters..." << std::endl;
            // Will use hardcoded parameters below
        }
    } else {
        std::cout << "No parameter file provided, using hardcoded parameters" << std::endl;
    }
        
    // Store copy of config before JSON loading for comparison  
    DStarFitConfig configBeforeJSON = config;
    
    // ===== SINGLE BIN ANALYSIS =====
    std::cout << "\n🎯 Single Bin Analysis Mode" << std::endl;
    std::cout << "Target bin: pT[" << pTMin << ", " << pTMax << "]"
              << ", cos[" << cosMin << ", " << cosMax << "] (pp)" << std::endl;
    
    // ===== PARAMETER DEBUGGING: BEFORE LOADING =====
    std::cout << "\n🔍 DEBUGGING: Initial Parameters (Before Loading)" << std::endl;
    PrintBinParameters(currentBin, config.GetParametersForBin(currentBin), "INITIAL PARAMETERS FOR TARGET BIN");
    
    // Check if we need to set hardcoded parameters (only if no param file)
    bool useHardcodedParams = parameterFile.empty();
    
    // DEBUG: Check if JSON loading actually worked
    if (!parameterFile.empty()) {
        std::cout << "\n🔍 DEBUG: Checking if JSON parameters were loaded..." << std::endl;
        try {
            auto testParams = config.GetParametersForBin(currentBin);
            std::cout << "✅ Parameters found for target bin - JSON loading successful!" << std::endl;
            std::cout << "   Test nsig_ratio: " << testParams.nsig_ratio << std::endl;
            useHardcodedParams = false;  // JSON loading was successful
        } catch (const std::exception& e) {
            std::cout << "❌ JSON loading failed: " << e.what() << std::endl;
            useHardcodedParams = true;
        }
        std::cout << "useHardcodedParams = " << (useHardcodedParams ? "true" : "false") << std::endl;
    }
    
    if (useHardcodedParams) {
        // Use hardcoded parameters as fallback for single bin
        std::cout << "\n🔧 Setting hardcoded parameters for target bin..." << std::endl;
        
        // Create custom parameters for the current bin
        DStarBinParameters binParams;
            
            // ===== SIGNAL PDF CONFIGURATION =====
            binParams.signalPdfType = PDFType::DoubleGaussian;
            
            // Configure DoubleGaussian signal parameters
            binParams.doubleGaussianParams.mean = 0.1455;
            binParams.doubleGaussianParams.mean_min = 0.1452;
            binParams.doubleGaussianParams.mean_max = 0.1458;
            binParams.doubleGaussianParams.sigma1 = 0.0005;
            binParams.doubleGaussianParams.sigma1_min = 0.0001;
            binParams.doubleGaussianParams.sigma1_max = 0.01;
            binParams.doubleGaussianParams.sigma2 = 0.001;
            binParams.doubleGaussianParams.sigma2_min = 0.0001;
            binParams.doubleGaussianParams.sigma2_max = 0.01;
            binParams.doubleGaussianParams.fraction = 0.7;
            binParams.doubleGaussianParams.fraction_min = 0.0;
            binParams.doubleGaussianParams.fraction_max = 1.0;
            
            // ===== BACKGROUND PDF CONFIGURATION =====
            binParams.backgroundPdfType = PDFType::ThresholdFunction;
            
            // Configure ThresholdFunction background parameters
            binParams.thresholdFuncParams.p0_init = 1.0;
            binParams.thresholdFuncParams.p0_min = -10.0;
            binParams.thresholdFuncParams.p0_max = 10.0;
            binParams.thresholdFuncParams.p1_init = -1.0;
            binParams.thresholdFuncParams.p1_min = -10.0;
            binParams.thresholdFuncParams.p1_max = 10.0;
            binParams.thresholdFuncParams.m_pi_value = 0.13957;  // Charged pion mass
            
            // Customize yield ratios
            binParams.nsig_ratio = 0.005;
            binParams.nsig_min_ratio = 0.0001;
            binParams.nsig_max_ratio = 0.05;
            binParams.nbkg_ratio = 0.02;
            binParams.nbkg_min_ratio = 0.001;
            binParams.nbkg_max_ratio = 0.5;
            
        // Apply these parameters to the current bin
        config.SetParametersForBin(currentBin, binParams);
        std::cout << "✅ Hardcoded parameters applied to target bin." << std::endl;
    }
    
    // ===== PARAMETER DEBUGGING: AFTER LOADING =====
    std::cout << "\n🔍 DEBUGGING: Final Parameters (After Loading)" << std::endl;
    PrintBinParameters(currentBin, config.GetParametersForBin(currentBin), "FINAL PARAMETERS FOR TARGET BIN");
    
    // ===== PARAMETER COMPARISON =====
    if (!parameterFile.empty()) {
        std::cout << "\n🔍 PARAMETER COMPARISON FOR TARGET BIN" << std::endl;
        try {
            auto paramsBefore = configBeforeJSON.GetParametersForBin(currentBin);
            auto paramsAfter = config.GetParametersForBin(currentBin);
            
            std::cout << "📍 Target Bin: " << currentBin.GetBinName() << std::endl;
            
            // Compare yield ratios
            if (paramsBefore.nsig_ratio != paramsAfter.nsig_ratio) {
                std::cout << "  📊 nsig_ratio: " << paramsBefore.nsig_ratio 
                          << " → " << paramsAfter.nsig_ratio << std::endl;
            }
            if (paramsBefore.nbkg_ratio != paramsAfter.nbkg_ratio) {
                std::cout << "  📊 nbkg_ratio: " << paramsBefore.nbkg_ratio 
                          << " → " << paramsAfter.nbkg_ratio << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error comparing parameters: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n🎯 Single Bin Analysis Starting..." << std::endl;
    
    // Process the single kinematic bin
    const auto& bin = currentBin;
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Processing bin (pp):" << std::endl;
    std::cout << "  pT range: [" << bin.pTMin << ", " << bin.pTMax << "] GeV/c" << std::endl;
    std::cout << "  cos(θ*) range: [" << bin.cosMin << ", " << bin.cosMax << "]" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    try {
        // Create optimized fitter for this bin
        auto fitter = CreateDStarFitter(bin, config);
        // Coefficient model is auto-enabled by config when Fraction mode is selected
        
        // Get bin-specific parameters
        auto binParams = config.GetParametersForBin(bin);
        auto fitOpt = config.CreateFitOpt(bin);
        
        // In pp reference analysis, Centrality is not a dataset variable.
        // Ensure the cut expression does NOT include a centrality cut even if the bin name carries cent ranges.
        {
            const std::string subdir = config.GetOutputSubDir();
            const bool isPP = (subdir.find("ppRef") != std::string::npos) || (subdir.find("_pp") != std::string::npos) || (subdir.find("/pp") != std::string::npos);
            if (isPP) {
                std::string kinCut = std::string("pT > ") + std::to_string(bin.pTMin) + " && pT < " + std::to_string(bin.pTMax);
                if (bin.cosMin > -2.0 || bin.cosMax < 2.0) {
                    const bool useAbs = config.GetUseAbsCosCuts();
                    const std::string cosExpr = useAbs ? "abs(cosThetaHX)" : "cosThetaHX";
                    kinCut += std::string(" && ") + cosExpr + " > " + std::to_string(bin.cosMin)
                           +  " && " + cosExpr + " < " + std::to_string(bin.cosMax);
                }
                // Rebuild cuts without any Centrality term
                fitOpt.cutExpr   = config.GetFullCutString() + " && " + kinCut;
                fitOpt.cutMCExpr = fitOpt.cutExpr + " && matchGEN==1";
                std::cout << "[pp] Overriding cutExpr to drop centrality: " << fitOpt.cutExpr << std::endl;
            }
        }
        const std::string ppBinName = fitOpt.GetBinName();
        
        std::cout << "Cut expression: " << fitOpt.cutExpr << std::endl;
        std::cout << "Expected signal ratio: " << binParams.nsig_ratio << std::endl;
        std::cout << "Expected background ratio: " << binParams.nbkg_ratio << std::endl;
        
        // Load data
        std::cout << "Loading data from: " << config.GetDataFilePath() << std::endl;
        DataLoader loader(config.GetDataFilePath());
        
        if (!loader.loadRooDataSet(config.GetDatasetName())) {
            std::cerr << "Failed to load dataset: " << config.GetDatasetName() << std::endl;
            return;
        }
            
        auto dataset = loader.getDataSet();
        if (!dataset) {
            std::cerr << "Dataset is null!" << std::endl;
            return;
        }
            
            std::cout << "Original dataset size: " << dataset->numEntries() << " events" << std::endl;
            
            // Perform the fit
            std::cout << "Performing fit..." << std::endl;
            
            bool fitSuccess = false;
            if (doReFit) {
                // Show active parameter types (C++14-compatible)
                std::cout << "\n=== PARAMETER CONFIGURATION ===" << std::endl;
                binParams.ApplyToSignalParams([&](const auto& signalParams){
                    typedef typename std::decay<decltype(signalParams)>::type ParamType;
                    std::cout << "SIGNAL PDF PARAMETERS (" << GetPDFTypeName<ParamType>() << "):" << std::endl;
                });
                binParams.ApplyToBackgroundParams([&](const auto& backgroundParams){
                    typedef typename std::decay<decltype(backgroundParams)>::type ParamType;
                    std::cout << "\nBACKGROUND PDF PARAMETERS (" << GetPDFTypeName<ParamType>() << "):" << std::endl;
                });
                std::cout << "===============================\n" << std::endl;

                // Perform fit by dispatching on active parameter types (no std::visit)
                // Add routing for GaussianConstraint / FixedFromMC using JSON fixed flags
                // 1) Collect fixed flags for this bin and derive prefixes
                auto fixedFlags = config.GetFixedFlagsForBin(bin);
                std::vector<std::string> prefixesToFix;
                prefixesToFix.reserve(fixedFlags.size());
                for (const auto& kv : fixedFlags) {
                    if (!kv.second) continue;
                    const std::string& key = kv.first; // e.g., mean_dbcb, sigma1_doublegauss
                    auto pos = key.find('_');
                    if (pos != std::string::npos && pos > 0) {
                        std::string pref = key.substr(0, pos);
                        if (std::find(prefixesToFix.begin(), prefixesToFix.end(), pref) == prefixesToFix.end()) {
                            prefixesToFix.push_back(pref);
                        }
                    }
                }
                if (!prefixesToFix.empty()) {
                    std::ostringstream oss;
                    for (size_t i=0;i<prefixesToFix.size();++i) { if (i) oss << ", "; oss << prefixesToFix[i]; }
                    std::cout << "[forpp] Prefixes to fix derived from JSON: [" << oss.str() << "]" << std::endl;
                } else {
                    std::cout << "[forpp] No prefixes to fix derived from JSON (list empty)" << std::endl;
                }
                // 2) Build data-side parameter names for Gaussian constraints (prefix_signal)
                std::vector<std::string> dataParamNames;
                dataParamNames.reserve(prefixesToFix.size());
                for (const auto& p : prefixesToFix) dataParamNames.push_back(p + std::string("_signal"));
                if (!dataParamNames.empty()) {
                    std::ostringstream oss2;
                    for (size_t i=0;i<dataParamNames.size();++i) { if (i) oss2 << ", "; oss2 << dataParamNames[i]; }
                    std::cout << "[forpp] Constraint target names: [" << oss2.str() << "]" << std::endl;
                }

                binParams.ApplyToSignalParams([&](const auto& signalParams){
                    binParams.ApplyToBackgroundParams([&](const auto& backgroundParams){
                        if (fitOpt.fitMethod == FitMethod::GaussianConstraint) {
                            // Load MC dataset for constraints
                            DataLoader mcLoader(config.GetMCFilePath());
                            if (!mcLoader.loadRooDataSet(config.GetDatasetName())) {
                                std::cerr << "[GC] Failed to load MC dataset: " << config.GetDatasetName() << std::endl;
                                fitSuccess = false;
                            } else {
                                RooDataSet* mcDataset = mcLoader.getDataSet();
                                // Default to mean/sigma if no flags present
                                std::vector<std::string> toConstrain = dataParamNames.empty()
                                    ? std::vector<std::string>{"mean_signal","sigma_signal"}
                                    : dataParamNames;
                                fitSuccess = fitter->PerformGaussianConstraintFitWithMC(
                                    fitOpt, dataset, mcDataset,
                                    signalParams, backgroundParams,
                                    toConstrain,
                                    ppBinName);
                            }
                        } else if (fitOpt.fitMethod == FitMethod::FixedFromMC) {
                            // Load MC dataset, fit MC, fix selected params to MC values, fit data
                            DataLoader mcLoader(config.GetMCFilePath());
                            if (!mcLoader.loadRooDataSet(config.GetDatasetName())) {
                                std::cerr << "[FFMC] Failed to load MC dataset: " << config.GetDatasetName() << std::endl;
                                fitSuccess = false;
                            } else {
                                RooDataSet* mcDataset = mcLoader.getDataSet();
                                fitSuccess = fitter->PerformFixedParameterFitWithMC(
                                    fitOpt, dataset, mcDataset,
                                    signalParams, backgroundParams,
                                    prefixesToFix,
                                    ppBinName);
                            }
                        } else {
                            // Default path: free-parameter fit
                            fitSuccess = fitter->PerformFit(fitOpt, dataset,
                                                           signalParams,
                                                           backgroundParams,
                                                           ppBinName);
                        }
                    });
                });
            } else {
                // Check if results already exist
                std::cout << "Skipping fit (doReFit=false). Loading existing results..." << std::endl;
                // In practice, you would load existing results here
                fitSuccess = true;  // Assume success for demo
            }
            
            if (fitSuccess) {
                std::cout << "\n✓ Fit successful for bin: " << ppBinName << std::endl;
                
                // Print fit results
                fitter->PrintSummary(ppBinName);
                
                // Get fit quality metrics
                double signalYield = fitter->GetSignalYield(ppBinName);
                double signalError = fitter->GetSignalYieldError(ppBinName);
                double backgroundYield = fitter->GetBackgroundYield(ppBinName);
                double chiSquare = fitter->GetReducedChiSquare(ppBinName);
                bool isGoodFit = fitter->IsGoodFit(ppBinName);
                
                std::cout << "Fit Results Summary:" << std::endl;
                std::cout << "  Signal yield: " << signalYield << " ± " << signalError << std::endl;
                std::cout << "  Background yield: " << backgroundYield << std::endl;
                std::cout << "  Reduced χ²: " << chiSquare << std::endl;
                std::cout << "  Fit quality: " << (isGoodFit ? "GOOD" : "POOR") << std::endl;
                
                // Calculate significance
                double significance = fitter->CalculateSignificance(ppBinName);
                double purity = fitter->CalculatePurity(ppBinName);
                
                std::cout << "  Significance: " << significance << " σ" << std::endl;
                std::cout << "  Purity: " << purity * 100 << "%" << std::endl;
                

                // Save fit results
                std::cout << "Saving fit results..." << std::endl;
                std::string category = isMC ? "/MC" : "/Data";
                std::string fullOutDir = fitOpt.outputDir + fitOpt.subDir + category;
                createDir(Form("%s/", fullOutDir.c_str()));
                fitter->SaveResult(ppBinName, fullOutDir, fitOpt.outputFile + ".root", true);
                
                // Export results to text/JSON for further analysis
                fitter->ExportResults("json", fullOutDir + "/" + fitOpt.outputFile + "_results.json");

                // Additionally perform MC fit and save results under MC subDir
                try {
                    std::cout << "\n[MC] Performing MC fit for comparison..." << std::endl;
                    // Load MC dataset
                    DataLoader mcLoader(config.GetMCFilePath());
                    if (!mcLoader.loadRooDataSet(config.GetDatasetName())) {
                        std::cerr << "[MC] Failed to load MC dataset: " << config.GetDatasetName() << std::endl;
                    } else {
                        auto mcDataset = mcLoader.getDataSet();
                        if (mcDataset) {
                            bool mcFitSuccess = false;
                            auto mcBinParams = config.GetParametersForBin(bin);
                            mcBinParams.ApplyToSignalParams([&](const auto& signalParams){
                                mcFitSuccess = fitter->PerformMCFit(fitOpt, mcDataset, signalParams, ppBinName + std::string("_MC"));
                            });

                            if (mcFitSuccess) {
                                std::cout << "[MC] ✓ MC fit successful" << std::endl;
                                std::string mcOutDir = fitOpt.outputDir + fitOpt.subDir + "/MC";
                                createDir(Form("%s/", mcOutDir.c_str()));
                                fitter->SaveResult(ppBinName + "_MC", mcOutDir, fitOpt.outputFile + ".root", true);
                                fitter->ExportResults("json", mcOutDir + "/" + fitOpt.outputFile + "_results.json");
                                // MC plots
                                try {
                                    std::string mcPlotDir = mcOutDir + "/plots";
                                    EnhancedPlotManager mcPlotManager(fitOpt, mcOutDir, fitOpt.outputFile + ".root", mcPlotDir, false, true);
                                    if (mcPlotManager.IsValid()) {
                                        mcPlotManager.PrintSummary();
                                        mcPlotManager.DrawRawDistribution("raw_MC_" + ppBinName);
                                        mcPlotManager.DrawFittedModel(true, "fitted_MC_" + ppBinName);
                                    }
                                } catch (const std::exception& e) {
                                    std::cerr << "[MC] Plotting error: " << e.what() << std::endl;
                                }
                            } else {
                                std::cout << "[MC] ✗ MC fit failed" << std::endl;
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[MC] Exception during MC fit: " << e.what() << std::endl;
                }
                
                // Create plots if requested - using EnhancedPlotManager
                if (plotFit) {
                    std::cout << "Creating plots using EnhancedPlotManager..." << std::endl;
                    
                    try {
                        // Create EnhancedPlotManager with the saved fit results
                        std::string inputDir = fitOpt.outputDir + fitOpt.subDir + (isMC ? "/MC" : "/Data");
                        std::string inputFile = fitOpt.outputFile + ".root";
                        std::string plotDir = inputDir + "/plots";
                        
                        EnhancedPlotManager plotManager(fitOpt, inputDir, inputFile, plotDir, false, true);
                        
                        if (plotManager.IsValid()) {
                            // Print summary
                            plotManager.PrintSummary();
                            
                            // Draw raw distribution
                            bool rawSuccess = plotManager.DrawRawDistribution("raw_" + bin.GetBinName());
                            if (rawSuccess) {
                                std::cout << "✓ Raw distribution plot created" << std::endl;
                            }
                            
                            // Draw fitted model with pull plot
                            bool fitSuccess = plotManager.DrawFittedModel(true, "fitted_" + bin.GetBinName());
                            if (fitSuccess) {
                                std::cout << "✓ Fitted model plot created" << std::endl;
                            }
                            
                        } else {
                            std::cout << "⚠ PlotManager initialization failed, falling back to basic plotting" << std::endl;
                            
                            // Fallback to basic plotting
                            PlotOptions plotOptions;
                            plotOptions.title = "D* Meson Fit - " + bin.GetBinName();
                            plotOptions.xAxisTitle = "Δm = m(Kππ) - m(Kπ) [GeV/c²]";
                            plotOptions.yAxisTitle = "Events / (0.4 MeV/c²)";
                            plotOptions.drawComponents = true;
                            plotOptions.drawResiduals = true;
                            plotOptions.nbins = 100;
                            
                            auto canvas = fitter->CreateCanvas(bin.GetBinName(), plotOptions);
                            if (canvas) {
                                std::string outBase = fitOpt.outputDir + fitOpt.subDir + (isMC ? "/MC" : "/Data");
                                std::string plotName = outBase + "/plots/" + fitOpt.outputFile + "_plot.pdf";
                                createDir(Form("%s/plots/", outBase.c_str()));
                                canvas->SaveAs(plotName.c_str());
                                std::cout << "Plot saved: " << plotName << std::endl;
                            }
                        }
                        
                    } catch (const std::exception& e) {
                        std::cerr << "Error during plotting: " << e.what() << std::endl;
                        std::cout << "Continuing without plots..." << std::endl;
                    }
                }
                
                // --- DCA Analysis (template fit) ---
                if(doDCA){
                try {
                    std::cout << "\n[ DCA ] Starting DCA template fit..." << std::endl;

                    // Prepare DCA-specific fit options (use D0 mass for sideband-driven templates)
                    FitOpt dcaOpt = fitOpt;
                    dcaOpt.massVar = "massDaugther1";   // D0 mass variable in the dataset
                    dcaOpt.massMin = 1.75;
                    dcaOpt.massMax = 2.00;
                    // Ensure DCA variable name matches dataset/workspace branch
                    dcaOpt.dcaVar = "dca3D";

                    // Instantiate DCAFitter with kinematic/cut context from FitOpt
                    DCAFitter dcaFitter(dcaOpt, "DCAFitter", dcaOpt.massVar,
                                         dcaOpt.dcaMin, dcaOpt.dcaMax, 100);

                    // Prefer MC mass-fit result workspace (reduced by pt/cos/cent)
                    {
                        std::string mcOutDirDC = fitOpt.outputDir + fitOpt.subDir + "/MC";
                        std::string mcResultFile = mcOutDirDC + "/" + fitOpt.outputFile + ".root";
                        dcaFitter.setMCResultFile(mcResultFile, "workspace", "");
                    }
                    // Use mass-fit result (workspace) for sliced data input
                    {
                        std::string categoryDC = isMC ? "/MC" : "/Data";
                        std::string fullOutDirDC = fitOpt.outputDir + fitOpt.subDir + categoryDC;
                        std::string dcaDataResult = fullOutDirDC + "/" + fitOpt.outputFile + ".root";
                        dcaFitter.setDataResultFile(dcaDataResult, "workspace", "");
                    }

                    // Branch configuration
                    dcaFitter.setDCABranchName("dca3D");
                    dcaFitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_");
                    dcaFitter.setPromptPdgIds({4, 2});
                    dcaFitter.setNonPromptPdgIds({5});

                    // Apply same kinematic and selection cuts
                    dcaFitter.setMCCuts(dcaOpt.cutMCExpr);
                    dcaFitter.setDataCuts(dcaOpt.cutExpr);

                    // Output setup (save DCA ROOTs under dedicated folder)
                    std::string dcaRootDir = fitOpt.outputDir + fitOpt.subDir + "/Data/dcaroot";
                    createDir(dcaRootDir);
                    std::string dcaOutFile = dcaRootDir + "/DStar_DCA_" + bin.GetBinName() + ".root";
                    dcaFitter.setOutputFile(dcaOutFile);

                    // Run DCA workflow
                    if (dcaFitter.createTemplatesFromMC()) {
                        // Save DCA plots under a dedicated 'dcaplot' folder within the same subdir
                        std::string dcaPlotDir = fitOpt.outputDir + fitOpt.subDir + "/Data/dcaplot";
                        createDir(dcaPlotDir);
                        std::string plotPrefix = dcaPlotDir + "/DCA_" + bin.GetBinName();
                        dcaFitter.plotRawDataDistribution(plotPrefix + "_templates");

                        if (dcaFitter.loadDataFromResult() && dcaFitter.buildModelwSideband()) {
                        // if (dcaFitter.loadDataFromResult() && dcaFitter.buildModel()) {
                            RooFitResult* dcaFitResult = dcaFitter.performFit(true);
                            if (dcaFitResult) {
                                dcaFitter.plotResults(dcaFitResult, plotPrefix + "_fit");
                                dcaFitter.saveResults(dcaFitResult);
                                delete dcaFitResult;
                                std::cout << "[ DCA ] Completed successfully." << std::endl;
                            } else {
                                std::cout << "[ DCA ] Fit returned null result." << std::endl;
                            }
                        } else {
                            std::cout << "[ DCA ] Failed to load data or build model." << std::endl;
                        }
                    } else {
                        std::cout << "[ DCA ] Template creation from MC failed." << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[ DCA ] Exception: " << e.what() << std::endl;
                }
            }

                
            } else {
                std::cerr << "✗ Fit failed for bin: " << bin.GetBinName() << std::endl;
                
                // Run diagnostics to understand why fit failed
                std::cout << "Running fit diagnostics..." << std::endl;
                fitter->RunDiagnostics(fitOpt.outputDir + fitOpt.subDir + (isMC ? "/MC" : "/Data") + "/" + fitOpt.outputFile + "_diagnostics.txt");
            }
            
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred for bin " << bin.GetBinName() << ": " << e.what() << std::endl;
    }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "D* Analysis V2 completed!" << std::endl;
    std::cout << "Results saved in: results/" << config.GetOutputSubDir() << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}
