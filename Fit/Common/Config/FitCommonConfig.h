#ifndef FIT_COMMON_CONFIG_H
#define FIT_COMMON_CONFIG_H

#include <string>

// Shared fit method enum used across configuration structures
enum class FitMethod {
    NLL,                 // Negative Log Likelihood (unbinned)
    BinnedNLL,           // Binned Negative Log Likelihood
    Extended,            // Extended Maximum Likelihood (no robust range expansion)
    Robust,              // Robust Extended fit (iterative with range expansion)
    GaussianConstraint,  // Apply Gaussian constraints from MC to selected params
    GaussianConstraintWithSB, // Combined: signal Gaussian constraints + background SB constraints
    FixedFromMC,         // Fix selected parameters to MC-fit values
    FixedFromMCWithSideband // Fix signal params to MC, constrain background with sidebands
};

inline const char* FitMethodName(FitMethod method) {
    switch (method) {
        case FitMethod::NLL:                     return "NLL";
        case FitMethod::BinnedNLL:               return "BinnedNLL";
        case FitMethod::Extended:                return "Extended";
        case FitMethod::Robust:                  return "Robust";
        case FitMethod::GaussianConstraint:      return "GaussianConstraint";
        case FitMethod::GaussianConstraintWithSB:return "GaussianConstraintWithSB";
        case FitMethod::FixedFromMC:             return "FixedFromMC";
        case FitMethod::FixedFromMCWithSideband: return "FixedFromMCWithSideband";
    }
    return "Unknown";
}

// Settings common to both FitConfig (strategy layer) and FitOpt (macro layer)
struct FitCommonSettings {
    FitMethod fitMethod = FitMethod::NLL;
    bool useMinos = false;
    bool useHesse = true;
    bool useCUDA = false;
    bool useIntegratedMC = false;
    bool verbose = false;
    int numCPU = 24;
    int histogramBins = 100;
    int maxRetries = 3;
    std::string strategy = "Minuit2";   // Minimizer type
    std::string minimizer = "migrad";   // Minimizer algorithm

    void CopyCommonSettingsTo(FitCommonSettings& target) const {
        target.fitMethod = fitMethod;
        target.useMinos = useMinos;
        target.useHesse = useHesse;
        target.useCUDA = useCUDA;
        target.useIntegratedMC = useIntegratedMC;
        target.verbose = verbose;
        target.numCPU = numCPU;
        target.histogramBins = histogramBins;
        target.maxRetries = maxRetries;
        target.strategy = strategy;
        target.minimizer = minimizer;
    }

    void AssignFrom(const FitCommonSettings& source) {
        source.CopyCommonSettingsTo(*this);
    }
};

#endif // FIT_COMMON_CONFIG_H
