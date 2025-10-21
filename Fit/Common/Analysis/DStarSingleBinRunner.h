#ifndef DSTAR_SINGLE_BIN_RUNNER_H
#define DSTAR_SINGLE_BIN_RUNNER_H

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../Config/DStarFitConfig.h"
#include "../Core/DataLoader.h"
#include "../Core/DCAFitter.h"
#include "../Core/MassFitterV2.h"
#include "../Managers/EnhancedPlotManager.h"
#include "../Managers/ErrorHandler.h"
#include "../Utils/Helper.h"
#include "../Utils/JSONParameterUtils.h"
#include "../Utils/ParameterDebugUtils.h"
#include "../Utils/SimpleParameterLoader.h"

namespace DStarAnalysis {

enum class FitMode {
    GaussianConstraint,
    GaussianConstraintWithSideband,
    FixedFromMC,
    FixedFromMCWithSideband,
    Plain
};

// Diagnostic helpers mirrored from legacy analysis implementation
inline void PrintBinParameters(const KinematicBin& bin,
                               const DStarBinParameters& params,
                               const std::string& title = "") {
    ParameterDebug::PrintBinParameters(bin, params, title);
}

template<typename T> inline std::string GetPDFTypeName() { return "Unknown"; }
template<> inline std::string GetPDFTypeName<PDFParams::GaussianParams>() { return "Gaussian"; }
template<> inline std::string GetPDFTypeName<PDFParams::DoubleGaussianParams>() { return "DoubleGaussian"; }
template<> inline std::string GetPDFTypeName<PDFParams::CrystalBallParams>() { return "CrystalBall"; }
template<> inline std::string GetPDFTypeName<PDFParams::DBCrystalBallParams>() { return "DBCrystalBall"; }
template<> inline std::string GetPDFTypeName<PDFParams::DoubleDBCrystalBallParams>() { return "DoubleDBCrystalBall"; }
template<> inline std::string GetPDFTypeName<PDFParams::VoigtianParams>() { return "Voigtian"; }
template<> inline std::string GetPDFTypeName<PDFParams::BreitWignerParams>() { return "BreitWigner"; }
template<> inline std::string GetPDFTypeName<PDFParams::ExponentialBkgParams>() { return "Exponential"; }
template<> inline std::string GetPDFTypeName<PDFParams::ChebychevBkgParams>() { return "Chebychev"; }
template<> inline std::string GetPDFTypeName<PDFParams::PhenomenologicalParams>() { return "Phenomenological"; }
template<> inline std::string GetPDFTypeName<PDFParams::Phenomenological2Params>() { return "Phenomenological2"; }
template<> inline std::string GetPDFTypeName<PDFParams::PolynomialBkgParams>() { return "Polynomial"; }
template<> inline std::string GetPDFTypeName<PDFParams::ThresholdFuncParams>() { return "ThresholdFunction"; }
template<> inline std::string GetPDFTypeName<PDFParams::ExpErfBkgParams>() { return "ExpErf"; }
template<> inline std::string GetPDFTypeName<PDFParams::DstBkgParams>() { return "DstBg"; }
template<> inline std::string GetPDFTypeName<PDFParams::DstD0Params>() { return "DstD0Bg"; }

struct DatasetConfig {
    std::string dataFile;
    std::string mcFile;
    std::string datasetName = "datasetHX";
};

struct ParameterConfigOptions {
    std::string parameterFile;
    bool ignoreCentralityInMatching = true;
    std::function<void(DStarFitOpt&, const KinematicBin&)> applyFallbackParameters;
};

struct ExecutionOptions {
    bool doReFit = false;
    bool doDCA = true;
    bool plotFit = true;
    bool usePrecomputedDcaYield = false;
    bool isMC = false;
    bool dropCentralityFromCuts = false;
    bool isPP = false;
    // Fit-mode switches (set combinations as needed):
    //   • Gaussian constraint only        → enableGaussianConstraint=true,  enableGaussianConstraintSideband=false, enableFixedFromMC=false
    //   • Gaussian constraint + sideband  → enableGaussianConstraint=true,  enableGaussianConstraintSideband=true
    //   • Fixed-from-MC                   → enableFixedFromMC=true (others optional)
    //   • Plain unconstrained fit         → set all three flags to false
    bool enableGaussianConstraint = true;
    bool enableGaussianConstraintSideband = true;
    bool enableFixedFromMC = true;
    bool enableSidebandPrefitOnly = false;
    bool enableFixedSignalWithSB = false;

    double sidebandLowMin = 0.0;
    double sidebandLowMax = 0.0;
    double sidebandHighMin = 0.0;
    double sidebandHighMax = 0.0;
    double sidebandSigmaScale = 2.0;

    std::function<void(DStarFitOpt&, FitOpt&)> customizeFitOpt;
    std::function<void(MassFitterV2&, const FitOpt&)> preFitCallback;

    std::string systemLabel = "pp";
    std::string logPrefix = "[pp]";
    std::string loggerLabel = "DStarAnalysis";
    std::string dcaYieldHistFileOverride;
    std::string dcaYieldHistName = "dataYieldHist";
};

inline void ApplyFitMode(FitMode mode, ExecutionOptions& execOpts) {
    execOpts.enableGaussianConstraint = false;
    execOpts.enableGaussianConstraintSideband = false;
    execOpts.enableFixedFromMC = false;
    execOpts.enableFixedSignalWithSB = false;

    auto modeSetter = [mode](DStarFitOpt& cfg, FitOpt& fitOpt) {
        switch (mode) {
            case FitMode::GaussianConstraint:
            case FitMode::GaussianConstraintWithSideband:
                cfg.SetFitMethod(FitMethod::GaussianConstraint);
                fitOpt.fitMethod = FitMethod::GaussianConstraint;
                break;
            case FitMode::FixedFromMC:
                cfg.SetFitMethod(FitMethod::FixedFromMC);
                fitOpt.fitMethod = FitMethod::FixedFromMC;
                break;
            case FitMode::FixedFromMCWithSideband:
                cfg.SetFitMethod(FitMethod::FixedFromMCWithSideband);
                fitOpt.fitMethod = FitMethod::FixedFromMCWithSideband;
                break;
            case FitMode::Plain:
            default:
                cfg.SetFitMethod(FitMethod::Extended);
                fitOpt.fitMethod = FitMethod::Extended;
                break;
        }
    };

    switch (mode) {
        case FitMode::GaussianConstraint:
            execOpts.enableGaussianConstraint = true;
            break;
        case FitMode::GaussianConstraintWithSideband:
            execOpts.enableGaussianConstraint = true;
            execOpts.enableGaussianConstraintSideband = true;
            break;
        case FitMode::FixedFromMC:
            execOpts.enableFixedFromMC = true;
            break;
        case FitMode::FixedFromMCWithSideband:
            execOpts.enableFixedFromMC = true;
            execOpts.enableFixedSignalWithSB = true;
            break;
        case FitMode::Plain:
        default:
            break;
    }

    auto previous = execOpts.customizeFitOpt;
    execOpts.customizeFitOpt = [modeSetter, previous](DStarFitOpt& cfg, FitOpt& fitOpt) {
        modeSetter(cfg, fitOpt);
        if (previous) {
            previous(cfg, fitOpt);
        }
    };
}

inline void ConfigureFiles(DStarFitOpt& config, const DatasetConfig& dataset) {
    if (!dataset.dataFile.empty()) {
        config.SetDataFilePath(dataset.dataFile);
    }
    if (!dataset.mcFile.empty()) {
        config.SetMCFilePath(dataset.mcFile);
    }
    if (!dataset.datasetName.empty()) {
        config.SetDatasetName(dataset.datasetName);
    }
}

inline void ConfigureBaseFitOptions(DStarFitOpt& config, bool useCUDA, bool doReFit) {
    config.SetUseCUDA(useCUDA);
    config.SetVerbose(false);
    config.SetDoRefit(doReFit);
}

inline void ApplyDefaultDoubleGaussianParameters(DStarFitOpt& config, const KinematicBin& bin) {
    DStarBinParameters binParams;

    binParams.signalPdfType = PDFType::DoubleGaussian;
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

    binParams.backgroundPdfType = PDFType::ThresholdFunction;
    binParams.thresholdFuncParams.p0_init = 1.0;
    binParams.thresholdFuncParams.p0_min = -10.0;
    binParams.thresholdFuncParams.p0_max = 10.0;
    binParams.thresholdFuncParams.p1_init = -1.0;
    binParams.thresholdFuncParams.p1_min = -10.0;
    binParams.thresholdFuncParams.p1_max = 10.0;
    binParams.thresholdFuncParams.m_pi_value = 0.13957;

    binParams.nsig_ratio = 0.005;
    binParams.nsig_min_ratio = 0.0001;
    binParams.nsig_max_ratio = 0.05;
    binParams.nbkg_ratio = 0.02;
    binParams.nbkg_min_ratio = 0.001;
    binParams.nbkg_max_ratio = 0.5;

    config.SetParametersForBin(bin, binParams);
}

inline void PrintBinSummary(const KinematicBin& bin, const ExecutionOptions& execOpts) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Processing bin (" << execOpts.systemLabel << "):" << std::endl;
    std::cout << "  pT range: [" << bin.pTMin << ", " << bin.pTMax << "] GeV/c" << std::endl;
    std::cout << "  cos(θ*) range: [" << bin.cosMin << ", " << bin.cosMax << "]" << std::endl;
    std::cout << "  centrality range: [" << bin.centralityMin << ", " << bin.centralityMax << "]%" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

inline void RunSingleBinAnalysis(DStarFitOpt& config,
                                 const KinematicBin& currentBin,
                                 const ParameterConfigOptions& paramOpts,
                                 const ExecutionOptions& execOpts) {
    const std::string loggerLabel = execOpts.loggerLabel.empty() ? "DStarAnalysis" : execOpts.loggerLabel;
    const std::string logPrefix = execOpts.logPrefix.empty() ? "[" + execOpts.systemLabel + "]" : execOpts.logPrefix;

    DStarFitOpt configBeforeJSON = config;

    std::cout << "\n🎯 Single Bin Analysis Mode" << std::endl;
    std::cout << "Target bin: pT[" << currentBin.pTMin << ", " << currentBin.pTMax << "]"
              << ", cos[" << currentBin.cosMin << ", " << currentBin.cosMax << "]"
              << ", centrality[" << currentBin.centralityMin << ", " << currentBin.centralityMax << "]% ("
              << execOpts.systemLabel << ")" << std::endl;

    std::cout << "\n🔍 DEBUGGING: Initial Parameters (Before Loading)" << std::endl;
    PrintBinParameters(currentBin, config.GetParametersForBin(currentBin), "INITIAL PARAMETERS FOR TARGET BIN");

    bool useHardcodedParams = paramOpts.parameterFile.empty();

    if (!paramOpts.parameterFile.empty()) {
        std::cout << "Loading parameters from external file: " << paramOpts.parameterFile << std::endl;
        try {
            auto dotPos = paramOpts.parameterFile.find_last_of('.');
            std::string extension = (dotPos == std::string::npos)
                ? std::string()
                : paramOpts.parameterFile.substr(dotPos + 1);
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){ return std::tolower(c); });

            if (extension == "json") {
                std::cout << "Using JSON parameter loader..." << std::endl;
                LoadParametersFromJSONToConfig(config, paramOpts.parameterFile, paramOpts.ignoreCentralityInMatching, &currentBin);
            } else {
                std::cout << "Using legacy parameter loader..." << std::endl;
                ParameterLoaderUtils::LoadParametersToConfig(config, paramOpts.parameterFile);
            }
            std::cout << "✓ External parameters loaded successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "⚠ Failed to load external parameters: " << e.what() << std::endl;
            std::cout << "Falling back to hardcoded parameters..." << std::endl;
            useHardcodedParams = true;
        }

        if (!useHardcodedParams) {
            std::cout << "\n🔍 DEBUG: Checking if JSON parameters were loaded..." << std::endl;
            try {
                auto testParams = config.GetParametersForBin(currentBin);
                std::cout << "✅ Parameters found for target bin - JSON loading successful!" << std::endl;
                std::cout << "   Test nsig_ratio: " << testParams.nsig_ratio << std::endl;
            } catch (const std::exception& e) {
                std::cout << "❌ JSON loading failed: " << e.what() << std::endl;
                useHardcodedParams = true;
            }
            std::cout << "useHardcodedParams = " << (useHardcodedParams ? "true" : "false") << std::endl;
        }
    }

    if (useHardcodedParams && paramOpts.applyFallbackParameters) {
        std::cout << "\n🔧 Setting hardcoded parameters for target bin..." << std::endl;
        paramOpts.applyFallbackParameters(config, currentBin);
        std::cout << "✅ Hardcoded parameters applied to target bin." << std::endl;
    }

    std::cout << "\n🔍 DEBUGGING: Final Parameters (After Loading)" << std::endl;
    PrintBinParameters(currentBin, config.GetParametersForBin(currentBin), "FINAL PARAMETERS FOR TARGET BIN");

    if (!paramOpts.parameterFile.empty()) {
        std::cout << "\n🔍 PARAMETER COMPARISON FOR TARGET BIN" << std::endl;
        try {
            auto paramsBefore = configBeforeJSON.GetParametersForBin(currentBin);
            auto paramsAfter = config.GetParametersForBin(currentBin);
            std::cout << "📍 Target Bin: " << currentBin.GetBinName() << std::endl;
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

    PrintBinSummary(currentBin, execOpts);

    try {
        auto fitter = CreateDStarFitter(currentBin, config);
        auto binParams = config.GetParametersForBin(currentBin);
        auto fitOpt = config.CreateFitOpt(currentBin);
        if (execOpts.customizeFitOpt) {
            execOpts.customizeFitOpt(config, fitOpt);
        }
        fitOpt.isPP = execOpts.isPP;

        if (fitOpt.cutMCExpr.empty()) {
            fitOpt.cutMCExpr = fitOpt.cutExpr.empty()
                                   ? std::string("matchGEN==1")
                                   : fitOpt.cutExpr + " && matchGEN==1";
        }

        if (execOpts.dropCentralityFromCuts) {
            const std::string subdir = config.GetOutputSubDir();
            const bool looksLikePP = (subdir.find("ppRef") != std::string::npos) ||
                                     (subdir.find("_pp") != std::string::npos) ||
                                     (subdir.find("/pp") != std::string::npos);
            std::cout << logPrefix << " isPP flag (from output subdir) = " << (looksLikePP ? "true" : "false") << std::endl;
            std::string kinCut = std::string("pT > ") + std::to_string(currentBin.pTMin) +
                                 " && pT < " + std::to_string(currentBin.pTMax);
            if (currentBin.cosMin > -2.0 || currentBin.cosMax < 2.0) {
                const bool useAbs = config.GetUseAbsCosCuts();
                const std::string cosExpr = useAbs ? "abs(cosThetaHX)" : "cosThetaHX";
                kinCut += std::string(" && ") + cosExpr + " > " + std::to_string(currentBin.cosMin)
                       +  " && " + cosExpr + " < " + std::to_string(currentBin.cosMax);
            }
            fitOpt.cutExpr   = config.GetFullCutString() + " && " + kinCut;
            fitOpt.cutMCExpr = fitOpt.cutExpr + " && matchGEN==1";
            std::cout << logPrefix << " Overriding cutExpr to drop centrality: " << fitOpt.cutExpr << std::endl;
        }

        const std::string binName = fitOpt.GetBinName();

        std::cout << "Cut expression: " << fitOpt.cutExpr << std::endl;
        std::cout << "Expected signal ratio: " << binParams.nsig_ratio << std::endl;
        std::cout << "Expected background ratio: " << binParams.nbkg_ratio << std::endl;

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

        const std::string mcDatasetName = !config.GetMCdatasetName().empty()
                                         ? config.GetMCdatasetName()
                                         : config.GetDatasetName();

        if (execOpts.preFitCallback) {
            execOpts.preFitCallback(*fitter, fitOpt);
        }

        std::cout << "Performing fit..." << std::endl;
        bool fitSuccess = false;

        if (execOpts.doReFit) {
            std::cout << "\n=== PARAMETER CONFIGURATION ===" << std::endl;
            binParams.ApplyToSignalParams([&](const auto& signalParams){
                using ParamType = typename std::decay<decltype(signalParams)>::type;
                std::cout << "SIGNAL PDF PARAMETERS (" << GetPDFTypeName<ParamType>() << ")" << std::endl;
            });
            binParams.ApplyToBackgroundParams([&](const auto& backgroundParams){
                using ParamType = typename std::decay<decltype(backgroundParams)>::type;
                std::cout << "\nBACKGROUND PDF PARAMETERS (" << GetPDFTypeName<ParamType>() << ")" << std::endl;
            });
            std::cout << "===============================\n" << std::endl;

            auto fixedFlags = config.GetFixedFlagsForBin(currentBin);
            std::vector<std::string> prefixesToFix;
            prefixesToFix.reserve(fixedFlags.size());
            for (const auto& kv : fixedFlags) {
                if (!kv.second) continue;
                const std::string& key = kv.first;
                auto pos = key.find('_');
                if (pos != std::string::npos && pos > 0) {
                    std::string prefix = key.substr(0, pos);
                    if (std::find(prefixesToFix.begin(), prefixesToFix.end(), prefix) == prefixesToFix.end()) {
                        prefixesToFix.push_back(prefix);
                    }
                }
            }
            if (!prefixesToFix.empty()) {
                std::ostringstream oss;
                for (size_t i = 0; i < prefixesToFix.size(); ++i) {
                    if (i) oss << ", ";
                    oss << prefixesToFix[i];
                }
                std::cout << logPrefix << " Prefixes to fix derived from JSON: [" << oss.str() << "]" << std::endl;
            } else {
                std::cout << logPrefix << " No prefixes to fix derived from JSON (list empty)" << std::endl;
            }

            std::vector<std::string> dataParamNames;
            dataParamNames.reserve(prefixesToFix.size());
            for (const auto& prefix : prefixesToFix) {
                dataParamNames.push_back(prefix + std::string("_signal"));
            }
            if (!dataParamNames.empty()) {
                std::ostringstream oss;
                for (size_t i = 0; i < dataParamNames.size(); ++i) {
                    if (i) oss << ", ";
                    oss << dataParamNames[i];
                }
                std::cout << logPrefix << " Constraint target names: [" << oss.str() << "]" << std::endl;
            }

            binParams.ApplyToSignalParams([&](const auto& signalParams){
                binParams.ApplyToBackgroundParams([&](const auto& backgroundParams){
                    if (fitOpt.fitMethod == FitMethod::GaussianConstraint && execOpts.enableGaussianConstraint) {
                        DataLoader mcLoader(config.GetMCFilePath());
                        if (!mcLoader.loadRooDataSet(mcDatasetName)) {
                            std::cerr << "[GC] Failed to load MC dataset: " << mcDatasetName << std::endl;
                            fitSuccess = false;
                        } else {
                            RooDataSet* mcDataset = mcLoader.getDataSet();
                            std::vector<std::string> toConstrain = dataParamNames.empty()
                                ? std::vector<std::string>{"mean_signal", "sigma_signal"}
                                : dataParamNames;

                            const double sbLoMin = 0.140;
                            const double sbLoMax = 0.143;
                            const double sbHiMin = 0.149;
                            const double sbHiMax = 0.155;
                            const double sigScale = 2.0;
                            const double bkgScale = 2.0;

                            if (execOpts.enableGaussianConstraintSideband) {
                                fitSuccess = fitter->PerformGaussianConstraintFitWithMCAndBkgSB(
                                    fitOpt, dataset, mcDataset,
                                    signalParams, backgroundParams,
                                    toConstrain,
                                    sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                    sigScale, bkgScale,
                                    binName);

                                if (!fitSuccess) {
                                    ErrorHandlerManager::Instance().LogWarning("[GC+SB] Combined constraints failed; fallback to signal-only GC.", loggerLabel);
                                }
                            }

                            if (!execOpts.enableGaussianConstraintSideband || !fitSuccess) {
                                fitSuccess = fitter->PerformGaussianConstraintFitWithMC(
                                    fitOpt, dataset, mcDataset,
                                    signalParams, backgroundParams,
                                    toConstrain,
                                    binName);
                            }
                        }
                    } else if (fitOpt.fitMethod == FitMethod::FixedFromMC && execOpts.enableFixedFromMC) {
                        DataLoader mcLoader(config.GetMCFilePath());
                        if (!mcLoader.loadRooDataSet(mcDatasetName)) {
                            std::cerr << "[FixedFromMC] Failed to load MC dataset: " << mcDatasetName << std::endl;
                            fitSuccess = false;
                        } else {
                            RooDataSet* mcDataset = mcLoader.getDataSet();
                            fitSuccess = fitter->PerformFixedParameterFitWithMC(
                                fitOpt, dataset, mcDataset,
                                signalParams, backgroundParams,
                                prefixesToFix,
                                binName);
                        }
                    } else if (fitOpt.fitMethod == FitMethod::FixedFromMCWithSideband && execOpts.enableFixedSignalWithSB) {
                        DataLoader mcLoader(config.GetMCFilePath());
                        if (!mcLoader.loadRooDataSet(mcDatasetName)) {
                            std::cerr << "[FixedFromMC+SB] Failed to load MC dataset: " << mcDatasetName << std::endl;
                            fitSuccess = false;
                        } else {
                            RooDataSet* mcDataset = mcLoader.getDataSet();
                            double sbLoMin = execOpts.sidebandLowMin;
                            double sbLoMax = execOpts.sidebandLowMax;
                            double sbHiMin = execOpts.sidebandHighMin;
                            double sbHiMax = execOpts.sidebandHighMax;
                            if (sbLoMin >= sbLoMax || sbHiMin >= sbHiMax) {
                                sbLoMin = 0.140;
                                sbLoMax = 0.143;
                                sbHiMin = 0.149;
                                sbHiMax = 0.155;
                            }
                            double sigmaScale = execOpts.sidebandSigmaScale <= 0.0 ? 2.0 : execOpts.sidebandSigmaScale;
                            fitSuccess = fitter->PerformFixedSignalWithMCAndBkgSB(
                                fitOpt, dataset, mcDataset,
                                signalParams, backgroundParams,
                                prefixesToFix,
                                sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                sigmaScale,
                                binName);
                        }
                    } else if (execOpts.enableSidebandPrefitOnly) {
                        double sbLoMin = execOpts.sidebandLowMin;
                        double sbLoMax = execOpts.sidebandLowMax;
                        double sbHiMin = execOpts.sidebandHighMin;
                        double sbHiMax = execOpts.sidebandHighMax;
                        double sigmaScale = execOpts.sidebandSigmaScale <= 0.0 ? 2.0 : execOpts.sidebandSigmaScale;

                        if (sbLoMin >= sbLoMax || sbHiMin >= sbHiMax) {
                            sbLoMin = 0.140;
                            sbLoMax = 0.143;
                            sbHiMin = 0.149;
                            sbHiMax = 0.155;
                        }

                        ErrorHandlerManager::Instance().LogInfo(
                            "[SidebandPrefit] Applying background-only constraint",
                            loggerLabel);
                        ErrorHandlerManager::Instance().LogInfo(
                            "[SidebandPrefit] Ranges: [" + std::to_string(sbLoMin) + ", " + std::to_string(sbLoMax) +
                            "] U [" + std::to_string(sbHiMin) + ", " + std::to_string(sbHiMax) + "]",
                            loggerLabel);

                        fitSuccess = fitter->PerformSidebandPrefitBackgroundConstraintFit(
                            fitOpt, dataset,
                            signalParams, backgroundParams,
                            sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                            sigmaScale,
                            binName);
                    } else {
                        if (!execOpts.enableGaussianConstraintSideband && fitOpt.fitMethod == FitMethod::GaussianConstraint) {
                            fitOpt.fitMethod = FitMethod::Extended;
                        }
                        if (!execOpts.enableFixedFromMC && fitOpt.fitMethod == FitMethod::FixedFromMC) {
                            fitOpt.fitMethod = FitMethod::Extended;
                        }
                        if (!execOpts.enableFixedSignalWithSB && fitOpt.fitMethod == FitMethod::FixedFromMCWithSideband) {
                            fitOpt.fitMethod = FitMethod::Extended;
                        }
                        fitSuccess = fitter->PerformFit(
                            fitOpt, dataset,
                            signalParams, backgroundParams,
                            binName);
                    }
                });
            });
        } else {
            std::cout << "Skipping fit (doReFit=false). Loading existing results..." << std::endl;
            fitSuccess = true;
        }

        if (fitSuccess) {
            ErrorHandlerManager::Instance().LogInfo(std::string("Fit successful for bin: ") + binName, loggerLabel);
            fitter->PrintSummary(binName);

            double signalYield = fitter->GetSignalYield(binName);
            double signalError = fitter->GetSignalYieldError(binName);
            double backgroundYield = fitter->GetBackgroundYield(binName);
            double chiSquare = fitter->GetReducedChiSquare(binName);
            bool isGoodFit = fitter->IsGoodFit(binName);

            {
                std::ostringstream oss;
                oss << "Fit Results Summary:\n"
                    << "  Signal yield: " << signalYield << " ± " << signalError << "\n"
                    << "  Background yield: " << backgroundYield << "\n"
                    << "  Reduced chi2: " << chiSquare << "\n"
                    << "  Fit quality: " << (isGoodFit ? "GOOD" : "POOR");
                ErrorHandlerManager::Instance().LogInfo(oss.str(), loggerLabel);
            }

            double significance = fitter->CalculateSignificance(binName);
            double purity = fitter->CalculatePurity(binName);

            std::cout << "  Significance: " << significance << " σ" << std::endl;
            std::cout << "  Purity: " << purity * 100 << "%" << std::endl;

            std::cout << "Saving fit results..." << std::endl;
            std::string category = execOpts.isMC ? "/MC" : "/Data";
            std::string fullOutDir = fitOpt.outputDir + fitOpt.subDir + category;
            createDir(Form("%s/", fullOutDir.c_str()));
            fitter->SaveResult(binName, fullOutDir, fitOpt.outputFile + ".root", true);
            fitter->ExportResults("json", fullOutDir + "/" + fitOpt.outputFile + "_results.json");

            try {
                std::cout << "\n[MC] Performing MC fit for comparison..." << std::endl;
                DataLoader mcLoader(config.GetMCFilePath());
                if (!mcLoader.loadRooDataSet(mcDatasetName)) {
                    std::cerr << "[MC] Failed to load MC dataset: " << mcDatasetName << std::endl;
                } else {
                    auto mcDataset = mcLoader.getDataSet();
                    if (mcDataset) {
                        bool mcFitSuccess = false;
                        auto mcBinParams = config.GetParametersForBin(currentBin);
                        mcBinParams.ApplyToSignalParams([&](const auto& signalParams){
                            mcFitSuccess = fitter->PerformMCFit(fitOpt, mcDataset, signalParams, binName + std::string("_MC"));
                        });

                        if (mcFitSuccess) {
                            std::cout << "[MC] ✓ MC fit successful" << std::endl;
                            std::string mcOutDir = fitOpt.outputDir + fitOpt.subDir + "/MC";
                            ensureDir(Form("%s/", mcOutDir.c_str()));
                            fitter->SaveResult(binName + "_MC", mcOutDir, fitOpt.outputFile + ".root", true);
                            fitter->ExportResults("json", mcOutDir + "/" + fitOpt.outputFile + "_results.json");
                            try {
                                std::string mcPlotDir = mcOutDir + "/plots";
                                FitOpt mcPlotOpt = fitOpt;
                                mcPlotOpt.datasetName = mcDatasetName;
                                if (mcPlotOpt.useIntegratedMC) {
                                    mcPlotOpt.cosLegend.clear();
                                }
                                EnhancedPlotManager mcPlotManager(mcPlotOpt, mcOutDir, fitOpt.outputFile + ".root", mcPlotDir, execOpts.isPP, true);
                                if (mcPlotManager.IsValid()) {
                                    mcPlotManager.PrintSummary();
                                    mcPlotManager.DrawRawDistribution("raw_MC_" + binName);
                                    mcPlotManager.DrawFittedModel(true, "fitted_MC_" + binName);
                                    mcPlotManager.DrawFittedModel(true, "fitted_MC_noParamPad_" + binName, false);
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

            if (execOpts.plotFit) {
                std::cout << "Creating plots using EnhancedPlotManager..." << std::endl;
                try {
                    std::string inputDir = fitOpt.outputDir + fitOpt.subDir + (execOpts.isMC ? "/MC" : "/Data");
                    std::string inputFile = fitOpt.outputFile + ".root";
                    std::string plotDir = inputDir + "/plots";

                    EnhancedPlotManager plotManager(fitOpt, inputDir, inputFile, plotDir, execOpts.isPP, true);

                    if (plotManager.IsValid()) {
                        plotManager.PrintSummary();
                        if (plotManager.DrawRawDistribution("raw_" + currentBin.GetBinName())) {
                            std::cout << "✓ Raw distribution plot created" << std::endl;
                        }
                        if (plotManager.DrawFittedModel(true, "fitted_" + currentBin.GetBinName())) {
                            std::cout << "✓ Fitted model plot created" << std::endl;
                        }
                        if (plotManager.DrawFittedModel(true, "fitted_noParamPad_" + currentBin.GetBinName(), false)) {
                            std::cout << "✓ Fitted model (no parameter pad) plot created" << std::endl;
                        }
                    } else {
                        std::cout << "⚠ PlotManager initialization failed, falling back to basic plotting" << std::endl;
                        PlotOptions plotOptions;
                        plotOptions.title = "D* Meson Fit - " + currentBin.GetBinName();
                        plotOptions.xAxisTitle = "Δm = m(Kππ) - m(Kπ) [GeV/c²]";
                        plotOptions.yAxisTitle = "Events / (0.4 MeV/c²)";
                        plotOptions.drawComponents = true;
                        plotOptions.drawResiduals = true;
                        plotOptions.nbins = 100;

                        auto canvas = fitter->CreateCanvas(currentBin.GetBinName(), plotOptions);
                        if (canvas) {
                            std::string outBase = fitOpt.outputDir + fitOpt.subDir + (execOpts.isMC ? "/MC" : "/Data");
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

            if (execOpts.doDCA) {
                try {
                    std::cout << "\n[ DCA ] Starting DCA template fit..." << std::endl;

                    FitOpt dcaOpt = fitOpt;
                    dcaOpt.isPP = execOpts.isPP;
                    dcaOpt.dcaVar = "dca3D";

                    DCAFitter dcaFitter(dcaOpt, "DCAFitter", dcaOpt.massVar,
                                         dcaOpt.dcaMin, dcaOpt.dcaMax, 100);

                    if (!paramOpts.parameterFile.empty()) {
                        auto dotPos = paramOpts.parameterFile.find_last_of('.');
                        if (dotPos != std::string::npos) {
                            std::string ext = paramOpts.parameterFile.substr(dotPos + 1);
                            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
                            if (ext == "json") {
                                dcaFitter.setMassParamJSON(paramOpts.parameterFile);
                            }
                        }
                    }

                    std::cout << logPrefix << " [DCA] EnableTestComponent=true, base=nonprompt, exp lambda=2.0" << std::endl;

                    {
                        std::string mcOutDirDC = fitOpt.outputDir + fitOpt.subDir + "/MC";
                        std::string mcResultFile = mcOutDirDC + "/" + fitOpt.outputFile + ".root";
                        dcaFitter.setMCResultFile(mcResultFile, "workspace", "");
                    }
                    {
                        std::string categoryDC = execOpts.isMC ? "/MC" : "/Data";
                        std::string fullOutDirDC = fitOpt.outputDir + fitOpt.subDir + categoryDC;
                        std::string dcaDataResult = fullOutDirDC + "/" + fitOpt.outputFile + ".root";
                        dcaFitter.setDataResultFile(dcaDataResult, "workspace", "");
                    }

                    dcaFitter.setDCABranchName("dca3D");
                    dcaFitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_");
                    dcaFitter.setPromptPdgIds({4, 2});
                    dcaFitter.setNonPromptPdgIds({5});

                    dcaFitter.setMCCuts(dcaOpt.cutMCExpr);
                    const std::string dataCutsForFit = (execOpts.isMC && !dcaOpt.cutMCExpr.empty())
                                                        ? dcaOpt.cutMCExpr
                                                        : dcaOpt.cutExpr;
                    dcaFitter.setDataCuts(dataCutsForFit);

                    std::string dcaYieldDir = fitOpt.outputDir + fitOpt.subDir + "/Data/dcahist";
                    createDir(dcaYieldDir);
                    std::string defaultYieldFile = dcaYieldDir + "/DStar_DCA_Yield_" + currentBin.GetBinName() + ".root";
                    std::string yieldFile = execOpts.dcaYieldHistFileOverride.empty() ? defaultYieldFile : execOpts.dcaYieldHistFileOverride;
                    std::string yieldHistName = execOpts.dcaYieldHistName.empty() ? "dataYieldHist" : execOpts.dcaYieldHistName;

                    if (execOpts.usePrecomputedDcaYield) {
                        dcaFitter.EnableMassFits(false);
                        dcaFitter.setDataYieldHistInput(yieldFile, yieldHistName);
                        std::cout << "[ DCA ] Using precomputed mass-fit yield histogram: "
                                  << yieldFile << " (hist=" << yieldHistName << ")" << std::endl;
                    } else {
                        dcaFitter.EnableMassFits(true);
                        dcaFitter.setDataYieldHistOutput(yieldFile, yieldHistName);
                        std::cout << "[ DCA ] Mass-fit yields will be saved to: "
                                  << yieldFile << " (hist=" << yieldHistName << ")" << std::endl;
                    }

                    std::string dcaRootDir = fitOpt.outputDir + fitOpt.subDir + "/Data/dcaroot";
                    ensureDir(dcaRootDir);
                    std::string dcaOutFile = dcaRootDir + "/DStar_DCA_" + currentBin.GetBinName() + ".root";
                    dcaFitter.setOutputFile(dcaOutFile);

                    if (dcaFitter.createTemplatesFromMC()) {
                        std::string dcaPlotDir = fitOpt.outputDir + fitOpt.subDir + "/Data/dcaplot/";
                        ensureDir(dcaPlotDir);
                        std::string plotPrefix = dcaPlotDir + "/DCA_" + currentBin.GetBinName();
                        dcaFitter.plotRawDataDistribution(plotPrefix + "_templates");

                        if (dcaFitter.loadDataFromResult() && dcaFitter.buildModel()) {
                            RooFitResult* dcaFitResult = dcaFitter.performFit(true);
                            if (dcaFitResult) {
                                dcaFitter.plotResults(dcaFitResult,
                                                      plotPrefix + "_fit",
                                                      true,
                                                      dcaFitter.GetLatestDetailedPlotDir());
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
            std::cerr << "✗ Fit failed for bin: " << currentBin.GetBinName() << std::endl;
            std::cout << "Running fit diagnostics..." << std::endl;
            fitter->RunDiagnostics(fitOpt.outputDir + fitOpt.subDir + (execOpts.isMC ? "/MC" : "/Data") + "/" + fitOpt.outputFile + "_diagnostics.txt");
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred for bin " << currentBin.GetBinName() << ": " << e.what() << std::endl;
    }

    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "D* Analysis V2 completed!" << std::endl;
    std::cout << "Results saved in: results/" << config.GetOutputSubDir() << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

} // namespace DStarAnalysis

#endif // DSTAR_SINGLE_BIN_RUNNER_H
