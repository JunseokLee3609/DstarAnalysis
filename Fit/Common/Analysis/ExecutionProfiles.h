#ifndef DSTAR_EXECUTION_PROFILES_H
#define DSTAR_EXECUTION_PROFILES_H

#include "DStarSingleBinRunner.h"
#include "DatasetRegistry.h"

namespace DStarAnalysis {

inline ExecutionOptions MakePPExecutionOptions(bool doReFit,
                                               bool doDCA,
                                               bool plotFit,
                                               bool usePrecomputedDcaYield,
                                               bool isMC,
                                               const std::string& dcaYieldHistFileOverride,
                                               const std::string& dcaYieldHistName,
                                               FitMode mode) {
    ExecutionOptions exec;
    exec.doReFit = doReFit;
    exec.doDCA = doDCA;
    exec.plotFit = plotFit;
    exec.usePrecomputedDcaYield = usePrecomputedDcaYield;
    exec.isMC = isMC;
    exec.dropCentralityFromCuts = true;
    exec.isPP = true;
    exec.systemLabel = "pp";
    exec.logPrefix = "[pp]";
    exec.loggerLabel = "DStarAnalysisV2forpp";
    exec.dcaYieldHistFileOverride = dcaYieldHistFileOverride;
    exec.dcaYieldHistName = dcaYieldHistName.empty() ? std::string("dataYieldHist") : dcaYieldHistName;

    ApplyFitMode(mode, exec);
    return exec;
}

inline ExecutionOptions MakePbPbExecutionOptions(bool doReFit,
                                                 bool doDCA,
                                                 bool plotFit,
                                                 bool usePrecomputedDcaYield,
                                                 bool isMC,
                                                 const std::string& dcaYieldHistFileOverride,
                                                 const std::string& dcaYieldHistName,
                                                 FitMode mode) {
    ExecutionOptions exec;
    exec.doReFit = doReFit;
    exec.doDCA = doDCA;
    exec.plotFit = plotFit;
    exec.usePrecomputedDcaYield = usePrecomputedDcaYield;
    exec.isMC = isMC;
    exec.dropCentralityFromCuts = false;
    exec.isPP = false;
    exec.systemLabel = "PbPb";
    exec.logPrefix = "[PbPb]";
    exec.loggerLabel = "DStarAnalysisPbPb";
    exec.dcaYieldHistFileOverride = dcaYieldHistFileOverride;
    exec.dcaYieldHistName = dcaYieldHistName.empty() ? std::string("dataYieldHist") : dcaYieldHistName;

    ApplyFitMode(mode, exec);
    return exec;
}

} // namespace DStarAnalysis

#endif // DSTAR_EXECUTION_PROFILES_H
