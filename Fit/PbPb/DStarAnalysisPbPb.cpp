#include <cstdlib>
#include "../Common/Analysis/DStarSingleBinRunner.h"
#include "../Common/Analysis/DatasetRegistry.h"
#include "../Common/Analysis/ExecutionProfiles.h"
#include "../../Interface/ConfigPbPb.h"

using namespace DStarAnalysis;

namespace {
struct AnalysisProfile {
    DStarFitOpt config;
    ParameterConfigOptions paramOpts;
    ExecutionOptions execOpts;
    KinematicBin bin;
};

AnalysisProfile BuildPbPbAnalysisProfile(bool doReFit, bool doDCA, bool plotFit, bool useCUDA,
                                         float pTMin, float pTMax, float cosMin, float cosMax,
                                         int centralityMin, int centralityMax,
                                         const std::string& parameterFile,
                                         bool isMC,
                                         bool usePrecomputedDcaYield,
                                         const std::string& dcaYieldHistFileOverride,
                                         const std::string& dcaYieldHistName,
                                         const std::string& customDatasetName = "") {
    AnalysisProfile profile;

    profile.config.SetYieldModeAutoTuning(false);
    profile.config.SetUseIndependentYields(true);

    // 데이터셋 설정 파일 로드 - 새로운 인스턴스 생성
    SimpleDatasetManager localDatasetManager("../Common/Analysis/datasets.json");
    
    // 커스텀 데이터셋 사용: Data는 PbPb_Data_EPtransformation, MC는 PbPb_MC
    auto dataInfo = localDatasetManager.GetDataset("PbPb_Data_EPtransformation");
    auto mcInfo = localDatasetManager.GetDataset("PbPb_MC");
    
    DatasetConfig dataset;
    dataset.dataFile = dataInfo.file;
    dataset.mcFile = mcInfo.file;
    dataset.datasetName = dataInfo.dataset_name;
    // Data dataset name 설정 (for Opt.h cos variable selection)
    profile.config.SetDatasetName(dataInfo.dataset_name);
    
    // MC dataset name 설정
    profile.config.SetMCdatasetName(mcInfo.dataset_name);
    
    ErrorHandlerManager::Instance().LogInfo("Using custom dataset configuration:", "DStarAnalysisPbPb");
    ErrorHandlerManager::Instance().LogInfo("  Data: " + dataset.dataFile, "DStarAnalysisPbPb");
    ErrorHandlerManager::Instance().LogInfo("  MC: " + dataset.mcFile, "DStarAnalysisPbPb");
    ErrorHandlerManager::Instance().LogInfo("  Dataset Name: " + dataset.datasetName, "DStarAnalysisPbPb");
    
    ConfigureFiles(profile.config, dataset);

    profile.config.SetOutputSubDir(SelectionCuts::SubDir());
    ConfigureBaseFitOptions(profile.config, useCUDA, doReFit);
    profile.config.SetFitMethod(FitMethod::GaussianConstraint);

    profile.config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    profile.config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
    profile.config.SetMVACut(0.999);
    profile.config.SetUseAbsCosCuts(false);

    profile.bin = KinematicBin(pTMin, pTMax, cosMin, cosMax, centralityMin, centralityMax);
    profile.config.ClearCentralityBins();
    profile.config.AddPtBin(pTMin, pTMax);
    profile.config.AddCosBin(cosMin, cosMax);
    profile.config.AddCentralityBin(centralityMin, centralityMax);

    profile.paramOpts.parameterFile = parameterFile;
    profile.paramOpts.ignoreCentralityInMatching = false;
    profile.paramOpts.applyFallbackParameters = ApplyDefaultDoubleGaussianParameters;

    profile.execOpts = MakePbPbExecutionOptions(doReFit, doDCA, plotFit,
                                                usePrecomputedDcaYield, isMC,
                                                dcaYieldHistFileOverride, dcaYieldHistName,
                                                FitMode::GaussianConstraintWithSideband);
    profile.execOpts.enableGaussianConstraint = true;
    profile.execOpts.enableGaussianConstraintSideband = true;
    profile.execOpts.enableSidebandPrefitOnly = false;
    profile.execOpts.sidebandLowMin = 0.140;
    profile.execOpts.sidebandLowMax = 0.143;
    profile.execOpts.sidebandHighMin = 0.149;
    profile.execOpts.sidebandHighMax = 0.155;
    profile.execOpts.sidebandSigmaScale = 1.0;

    auto previousCustomize = profile.execOpts.customizeFitOpt;
    profile.execOpts.customizeFitOpt = [=](DStarFitOpt& cfg, FitOpt& fitOpt) {
        if (previousCustomize) {
            previousCustomize(cfg, fitOpt);
        }
        fitOpt.isPP = false;
        const char* useIntegratedEnv = std::getenv("USE_INTEGRATED_MC");
        if (useIntegratedEnv && std::string(useIntegratedEnv) != "0") {
            fitOpt.useIntegratedMC = true;
            ErrorHandlerManager::Instance().LogInfo("[PbPb] Using integrated MC constraints (USE_INTEGRATED_MC=1)", "DStarAnalysisPbPb");
            fitOpt.cosLegend.clear();
        } else {
            fitOpt.useIntegratedMC = false;
            ErrorHandlerManager::Instance().LogInfo("[PbPb] Using cos-binned MC constraints (USE_INTEGRATED_MC=0)", "DStarAnalysisPbPb");
        }
        fitOpt.centLegend = Form("%.0d-%.0d%%", centralityMin, centralityMax);
        fitOpt.pTLegend = Form("%.1f < p_{T} < %.1f GeV/c", pTMin, pTMax);
        if (!fitOpt.useIntegratedMC) {
            fitOpt.cosLegend = Form("%.1f < cos#theta_{HX} < %.1f", cosMin, cosMax);
        } else {
            fitOpt.cosLegend.clear();
        }
        fitOpt.drawMvaLegend = true;
        fitOpt.yLegend = Form("|y| < %.1f", 1.0);
        fitOpt.mvaLegend = Form("MVA > %.3f", 0.999);
    };

    auto previousCallback = profile.execOpts.preFitCallback;
    profile.execOpts.preFitCallback = [previousCallback](MassFitterV2& fitter, const FitOpt& fitOpt) {
        if (previousCallback) {
            previousCallback(fitter, fitOpt);
        }
        
        // Configure Sideband Prefit Background Constraint
        const double sbLoMin = 0.140, sbLoMax = 0.143;
        const double sbHiMin = 0.149, sbHiMax = 0.155;
        const double sigmaScale = 1.0;
        
        ErrorHandlerManager::Instance().LogInfo(
            "[PbPb] Enabling Sideband Prefit Background Constraint (no MC signal constraint)",
            "DStarAnalysisPbPb");
        ErrorHandlerManager::Instance().LogInfo(
            "[PbPb] Sideband ranges: [" + std::to_string(sbLoMin) + ", " + std::to_string(sbLoMax) +
            "] U [" + std::to_string(sbHiMin) + ", " + std::to_string(sbHiMax) + "]",
            "DStarAnalysisPbPb");
        ErrorHandlerManager::Instance().LogInfo(
            "[PbPb] Sigma scale: " + std::to_string(sigmaScale),
            "DStarAnalysisPbPb");
            
        // Configure sideband constraint parameters for background fitting
        // These parameters will be used by the fitting strategy
        ErrorHandlerManager::Instance().LogInfo(
            "[PbPb] Sideband constraint configured for background parameter estimation",
            "DStarAnalysisPbPb");
    };

    return profile;
}
} // namespace

void DStarAnalysisPbPb(bool doReFit = false, bool doDCA = true, bool plotFit = true, bool useCUDA = true,
                       float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                       int centralityMin = 0, int centralityMax = 100,
                       const std::string& parameterFile = "", bool isMC = false,
                       bool usePrecomputedDcaYield = false,
                       const std::string& dcaYieldHistFileOverride = std::string(),
                       const std::string& dcaYieldHistName = "dataYieldHist") {
    ErrorHandlerManager::Instance().LogInfo("=== D* Meson Analysis V2 (PbPb) ===", "DStarAnalysisPbPb");
    ErrorHandlerManager::Instance().LogInfo("Using new modular framework with MassFitterV2", "DStarAnalysisPbPb");
    ErrorHandlerManager::Instance().LogInfo("Loading PbPb dataset with JSON support and debugging", "DStarAnalysisPbPb");

    auto profile = BuildPbPbAnalysisProfile(doReFit, doDCA, plotFit, useCUDA,
                                                pTMin, pTMax, cosMin, cosMax,
                                                centralityMin, centralityMax,
                                                parameterFile, isMC,
                                                usePrecomputedDcaYield,
                                                dcaYieldHistFileOverride, dcaYieldHistName);

    RunSingleBinAnalysis(profile.config, profile.bin, profile.paramOpts, profile.execOpts);
}

#ifdef __CLING__
void DStarAnalysisPbPb_macro(bool doReFit = false, bool doDCA = true, bool plotFit = true, bool useCUDA = true,
                             float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                             int centralityMin = 0, int centralityMax = 100,
                             const char* parameterFile = "", bool isMC = false,
                             bool usePrecomputedDcaYield = false,
                             const char* dcaYieldHistFileOverride = "",
                             const char* dcaYieldHistName = "dataYieldHist") {
    const std::string paramFileStr = parameterFile ? std::string(parameterFile) : std::string();
    const std::string dcaFileStr = dcaYieldHistFileOverride ? std::string(dcaYieldHistFileOverride) : std::string();
    const std::string dcaHistNameStr = dcaYieldHistName ? std::string(dcaYieldHistName) : std::string("dataYieldHist");

    DStarAnalysisPbPb(doReFit, doDCA, plotFit, useCUDA,
                      pTMin, pTMax, cosMin, cosMax,
                      centralityMin, centralityMax,
                      paramFileStr, isMC,
                      usePrecomputedDcaYield,
                      dcaFileStr, dcaHistNameStr);
}

void DStarAnalysisPbPb_macro() {
    DStarAnalysisPbPb();
}
#endif
