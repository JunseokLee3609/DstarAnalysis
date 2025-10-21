#include "../Common/Analysis/DStarSingleBinRunner.h"
#include "../Common/Analysis/DatasetRegistry.h"
#include "../Common/Analysis/ExecutionProfiles.h"
#include "ConfigPP.h"

using namespace DStarAnalysis;

namespace {
struct AnalysisProfile {
    DStarFitOpt config;
    ParameterConfigOptions paramOpts;
    ExecutionOptions execOpts;
    KinematicBin bin;
};

AnalysisProfile BuildPPAnalysisProfile(bool doReFit, bool doDCA, bool plotFit, bool useCUDA,
                                       float pTMin, float pTMax, float cosMin, float cosMax,
                                       int centralityMin, int centralityMax,
                                       const std::string& parameterFile,
                                       bool isMC,
                                       bool usePrecomputedDcaYield,
                                       const std::string& dcaYieldHistFileOverride,
                                       const std::string& dcaYieldHistName) {
    AnalysisProfile profile;

    profile.config.SetYieldModeAutoTuning(false);
    profile.config.SetUseIndependentYields(true);

    auto dataset = DatasetRegistry::GetPPDataset();
    ConfigureFiles(profile.config, dataset);

    profile.config.SetOutputSubDir(SelectionCuts::SUB_DIR);
    ConfigureBaseFitOptions(profile.config, useCUDA, doReFit);

    profile.config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    profile.config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
    profile.config.SetUseAbsCosCuts(false);

    profile.bin = KinematicBin(pTMin, pTMax, cosMin, cosMax, centralityMin, centralityMax);
    profile.config.AddPtBin(pTMin, pTMax);
    profile.config.AddCosBin(cosMin, cosMax);

    profile.paramOpts.parameterFile = parameterFile;
    profile.paramOpts.ignoreCentralityInMatching = true;
    profile.paramOpts.applyFallbackParameters = ApplyDefaultDoubleGaussianParameters;

    profile.execOpts = MakePPExecutionOptions(doReFit, doDCA, plotFit,
                                              usePrecomputedDcaYield, isMC,
                                              dcaYieldHistFileOverride, dcaYieldHistName,
                                              FitMode::GaussianConstraintWithSideband);

    profile.execOpts.enableFixedFromMC = true;

    auto previousCustomize = profile.execOpts.customizeFitOpt;
    profile.execOpts.customizeFitOpt = [=](DStarFitOpt& cfg, FitOpt& fitOpt) {
        if (previousCustomize) {
            previousCustomize(cfg, fitOpt);
        }
        cfg.SetFitMethod(FitMethod::GaussianConstraint);
        fitOpt.fitMethod = FitMethod::GaussianConstraint;
    };

    return profile;
}
} // namespace

void DStarAnalysisPP(bool doReFit = false, bool doDCA = true, bool plotFit = true, bool useCUDA = true,
                     float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                     int centralityMin = 0, int centralityMax = 100,
                     const std::string& parameterFile = "", bool isMC = false,
                     bool usePrecomputedDcaYield = false,
                     const std::string& dcaYieldHistFileOverride = std::string(),
                     const std::string& dcaYieldHistName = "dataYieldHist") {
    ErrorHandlerManager::Instance().LogInfo("=== D* Meson Analysis V2 (PP) ===", "DStarAnalysisV2forpp");
    ErrorHandlerManager::Instance().LogInfo("Using new modular framework with MassFitterV2", "DStarAnalysisV2forpp");
    ErrorHandlerManager::Instance().LogInfo("Loading PP dataset with JSON support and debugging", "DStarAnalysisV2forpp");

    auto profile = BuildPPAnalysisProfile(doReFit, doDCA, plotFit, useCUDA,
                                               pTMin, pTMax, cosMin, cosMax,
                                               centralityMin, centralityMax,
                                               parameterFile, isMC,
                                               usePrecomputedDcaYield,
                                               dcaYieldHistFileOverride, dcaYieldHistName);

    RunSingleBinAnalysis(profile.config, profile.bin, profile.paramOpts, profile.execOpts);
}

#ifdef __CLING__
void DStarAnalysisPP_macro(bool doReFit = false, bool doDCA = true, bool plotFit = true, bool useCUDA = true,
                           float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                           int centralityMin = 0, int centralityMax = 100,
                           const char* parameterFile = "", bool isMC = false,
                           bool usePrecomputedDcaYield = false,
                           const char* dcaYieldHistFileOverride = "",
                           const char* dcaYieldHistName = "dataYieldHist") {
    const std::string paramFileStr = parameterFile ? std::string(parameterFile) : std::string();
    const std::string dcaFileStr = dcaYieldHistFileOverride ? std::string(dcaYieldHistFileOverride) : std::string();
    const std::string dcaHistNameStr = dcaYieldHistName ? std::string(dcaYieldHistName) : std::string("dataYieldHist");

    DStarAnalysisPP(doReFit, doDCA, plotFit, useCUDA,
                    pTMin, pTMax, cosMin, cosMax,
                    centralityMin, centralityMax,
                    paramFileStr, isMC,
                    usePrecomputedDcaYield,
                    dcaFileStr, dcaHistNameStr);
}

void DStarAnalysisPP_macro() {
    DStarAnalysisPP();
}
#endif
