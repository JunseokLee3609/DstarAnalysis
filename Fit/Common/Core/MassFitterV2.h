#ifndef MASS_FITTER_V2_H
#define MASS_FITTER_V2_H

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <ctime>
#include <cmath>
#include <iostream>
#include <regex>
#include <cctype>
#include <sstream>

// Core includes
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooAddPdf.h"
#include "RooFitResult.h"
#include "RooWorkspace.h"
#include "RooFormulaVar.h"
#include "RooGaussian.h"
#include "RooArgSet.h"
#include "RooLinkedList.h"

// New modular components
#include "../Core/PDFFactory.h"
#include "../Utils/FitStrategy.h"
#include "../Managers/ResultManager.h"
#include "../Managers/ErrorHandler.h"
// #include "../Managers/TestFramework.h"
#include "../Config/Params.h"
#include "../Config/Opt.h"
#include "../Utils/Helper.h"  // for createDir/ensureOutputDir

// Constants
constexpr double PION_MASS_V2 = 0.13957039;
constexpr double DELTAMASS_V2 = 0.1454257;

namespace {
inline void ReplaceAll(std::string& src, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = src.find(from, pos)) != std::string::npos) {
        src.replace(pos, from.length(), to);
        pos += to.length();
    }
}

inline void TrimWhitespace(std::string& s) {
    auto isSpace = [](unsigned char ch) { return std::isspace(ch); };
    auto beginIt = s.begin();
    while (beginIt != s.end() && isSpace(*beginIt)) {
        ++beginIt;
    }
    auto endIt = s.end();
    if (beginIt == endIt) {
        s.clear();
        return;
    }
    do {
        --endIt;
    } while (endIt >= beginIt && isSpace(*endIt));
    s.assign(beginIt, endIt + 1);
}

inline std::string RemoveCosCuts(const std::string& expr) {
    std::string result = expr;
    const std::vector<std::string> cosTokens = {"cosThetaEP", "cosThetaHX", "cosThetaCS"};
    for (const auto& token : cosTokens) {
        std::regex absPattern("\\s*&&\\s*abs\\(\\s*" + token + "\\s*\\)\\s*[<>]=?\\s*[^&|]+");
        result = std::regex_replace(result, absPattern, "");
        std::regex directPattern("\\s*&&\\s*" + token + "\\s*[<>]=?\\s*[^&|]+");
        result = std::regex_replace(result, directPattern, "");
    }
    std::regex multiSpace("\\s{2,}");
    result = std::regex_replace(result, multiSpace, " ");
    TrimWhitespace(result);
    return result;
}

inline std::string NormalizeCutExpression(const std::string& expr, bool removeCosCuts = false) {
    std::string normalized = expr;
    ReplaceAll(normalized, "Centrality", "(centrality/2.0)");
    if (removeCosCuts) {
        normalized = RemoveCosCuts(normalized);
    }
    TrimWhitespace(normalized);
    return normalized;
}
} // namespace

/**
 * @brief Modern, modular MassFitter implementation
 * 
 * Key improvements over the original:
 * - Modular design with dependency injection
 * - Smart pointer-based memory management
 * - Exception-safe operations
 * - Configurable fitting strategies
 * - Comprehensive error handling
 * - Full unit test support
 */
class MassFitterV2 {
public:
    // Yield mode selection (public so callers can reference the enum)
    enum class YieldMode { Fraction, Independent };
    // Constructor with dependency injection
    MassFitterV2(const std::string& name, 
                 const std::string& massVar,
                 double massMin, 
                 double massMax,
                 std::unique_ptr<PDFFactory> pdfFactory = nullptr,
                 std::unique_ptr<FitStrategy> fitStrategy = nullptr,
                 std::unique_ptr<ResultManager> resultManager = nullptr,
                 YieldMode yieldMode = YieldMode::Fraction);
    
    // Traditional constructor (creates default implementations)
    MassFitterV2(const std::string& name,
                 const std::string& massVar, 
                 double massMin, 
                 double massMax,
                 double nsigRatio = 0.1,
                 double nsigMinRatio = 0.0, 
                 double nsigMaxRatio = 1.0,
                 double nbkgRatio = 0.0,
                 double nbkgMinRatio = 0.0,
                 double nbkgMaxRatio = 1.0,
                 YieldMode yieldMode = YieldMode::Fraction);
    
    ~MassFitterV2() = default;
    
    // Configuration management
    void LoadConfiguration(const std::string& configFile = "");
    void SetConfiguration(const FitOpt& config) { fitConfig_ = config; }
    const FitOpt& GetConfiguration() const { return fitConfig_; }
    
    // Data management with validation
    void SetData(RooDataSet* dataset);
    void ApplyCut(const std::string& cutExpr);
    RooDataSet* GetData() const { return activeDataset_; }
    int GetDataSize() const;
    
    // PDF setup - Template-based with type safety
    template<typename SignalParams>
    void SetSignalPDF(const SignalParams& params, const std::string& name = "signal");
    
    template<typename BackgroundParams>
    void SetBackgroundPDF(const BackgroundParams& params, const std::string& name = "background");
    
    // Mass variable configuration
    // void UseDeltaMass(bool use = true, double daughterMassMin = 1.8, double daughterMassMax = 1.9);
    RooRealVar* GetActiveMassVariable() const { return activeMassVar_; }
    double GetMassMin() const;
    double GetMassMax() const;
    
    // Simplified fitting interface
    bool Fit(const std::string& strategyName = "Robust", const std::string& resultName = "default");
    bool FitWithConfig(const FitOpt& config, const std::string& resultName = "configured");
    
    // Template-based fitting interface with enhanced error handling
    template<typename SignalParams, typename BackgroundParams>
    bool PerformFit(const FitOpt& options, RooDataSet* dataset, 
                   const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                   const std::string& resultName = "");
    
    template<typename SignalParams>
    bool PerformMCFit(const FitOpt& options, RooDataSet* mcDataset,
                     const SignalParams& signalParams,
                     const std::string& resultName = "");
    
    template<typename SignalParams, typename BackgroundParams>
    bool PerformConstraintFit(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                             const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                             const std::vector<std::string>& constraintParameters = {},
                             const std::string& resultName = "");

    // MC-driven Gaussian-constraint fit on selected parameters
    template<typename SignalParams, typename BackgroundParams>
    bool PerformGaussianConstraintFitWithMC(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                            const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                            const std::vector<std::string>& paramsToConstrain,
                                            const std::string& resultName = "");

    // Constrained fit using saved MC fit result file (contains RooFitResult "fitResult")
    template<typename SignalParams, typename BackgroundParams>
    bool PerformGaussianConstraintFitWithMCFile(const FitOpt& options, RooDataSet* dataset,
                                                const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                const std::string& mcResultFile,
                                                const std::vector<std::string>& paramsToConstrain,
                                                const std::string& resultName = "");

    // Background sideband prefit → weak Gaussian constraints (regularization on background params)
    template<typename SignalParams, typename BackgroundParams>
    bool PerformSidebandPrefitBackgroundConstraintFit(const FitOpt& options, RooDataSet* dataset,
                                                      const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                      double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
                                                      double sigmaScale = 1.0,
                                                      const std::string& resultName = "");

    // Combined: MC-driven Gaussian constraints on signal + SB-prefit constraints on background
    template<typename SignalParams, typename BackgroundParams>
    bool PerformGaussianConstraintFitWithMCAndBkgSB(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                                    const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                    const std::vector<std::string>& signalParamsToConstrain,
                                                    double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
                                                    double sigmaScaleSignal = 1.0, double sigmaScaleBkg = 2.0,
                                                    const std::string& resultName = "");

    // Hybrid: fix signal parameters to MC fit values, background constrained via sideband prefit
    template<typename SignalParams, typename BackgroundParams>
    bool PerformFixedSignalWithMCAndBkgSB(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                          const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                          const std::vector<std::string>& paramPrefixesToFix,
                                          double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
                                          double sigmaScaleBkg = 2.0,
                                          const std::string& resultName = "");

    // Fixed-from-MC: fit MC to get shape, fix selected params in data to MC values, then fit data
    template<typename SignalParams, typename BackgroundParams>
    bool PerformFixedParameterFitWithMC(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                        const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                        const std::vector<std::string>& paramPrefixesToFix,
                                        const std::string& resultName = "");

    // Fixed-from-MC using a saved RooFitResult file
    template<typename SignalParams, typename BackgroundParams>
    bool PerformFixedParameterFitWithMCFile(const FitOpt& options, RooDataSet* dataset,
                                            const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                            const std::string& mcResultFile,
                                            const std::vector<std::string>& paramPrefixesToFix,
                                            const std::string& resultName = "");
    
    // Results access with error checking
    double GetSignalYield(const std::string& resultName = "default") const;
    double GetSignalYieldError(const std::string& resultName = "default") const;
    double GetBackgroundYield(const std::string& resultName = "default") const;
    double GetBackgroundYieldError(const std::string& resultName = "default") const;
    double GetTotalYield(const std::string& resultName = "default") const;
    double GetChiSquare(const std::string& resultName = "default") const;
    double GetReducedChiSquare(const std::string& resultName = "default") const;
    double GetNDF(const std::string& resultName = "default") const;
    bool IsGoodFit(const std::string& resultName = "default") const;
    
    // Advanced results access
    FitResults* GetFitResults(const std::string& resultName = "default");
    RooFitResult* GetRooFitResult(const std::string& resultName = "default");
    RooWorkspace* GetWorkspace(const std::string& resultName = "default");
    std::vector<std::string> GetResultNames() const;
    
    // Plotting and visualization
    std::unique_ptr<RooPlot> CreatePlot(const std::string& resultName = "default",
                                       const PlotOptions& options = PlotOptions{});
    std::unique_ptr<TCanvas> CreateCanvas(const std::string& resultName = "default",
                                         const PlotOptions& options = PlotOptions{});
    
    // File I/O with error handling
    void SaveResults(const std::string& filePath, const std::string& fileName,
                    bool saveWorkspaces = true);
    void SaveResult(const std::string& resultName, const std::string& filePath, 
                   const std::string& fileName, bool saveWorkspace = true);
    void LoadResults(const std::string& filePath, const std::string& fileName);
    
    // Analysis and comparison
    void CompareResults(const std::vector<std::string>& resultNames, const std::string& outputFile = "");
    void PrintSummary(const std::string& resultName = "") const;
    void ExportResults(const std::string& format = "json", const std::string& fileName = "");
    
    // Strategy management
    void SetFitStrategy(std::unique_ptr<FitStrategy> strategy) {
        if (strategy) { LogOperation("SetFitStrategy", "Changed to " + strategy->GetName()); }
        fitStrategy_ = std::move(strategy);
    }
    // Automation-first: FitStrategy is determined by FitMethod via FitStrategyFactory
    FitStrategy* GetFitStrategy() const { return fitStrategy_.get(); }
    std::vector<std::string> GetAvailableStrategies() const;
    
    // Factory management
    PDFFactory* GetPDFFactory() const { return pdfFactory_.get(); }
    void SetPDFFactory(std::unique_ptr<PDFFactory> factory);
    
    // Result management
    ResultManager* GetResultManager() const { return resultManager_.get(); }
    void SetResultManager(std::unique_ptr<ResultManager> manager);
    
    // Validation and diagnostics
    bool ValidateSetup() const;
    std::vector<std::string> GetValidationErrors() const;
    void RunDiagnostics(const std::string& outputFile = "") const;
    
    // Utility methods
    void Clear();
    void Reset();
    std::string GetInfo() const;
    std::string GetName() const { return name_; }
    
    // Yield mode control
    void UseIndependentYields(bool enable = true) { SetYieldMode(enable ? YieldMode::Independent : YieldMode::Fraction); }
    // Enable coefficient-based fraction model: RooAddPdf(sig,bkg; fsig)
    // Useful for non-extended fits where only the mixture fraction is floated.
    void UseFractionCoefficientModel(bool enable = true) {
        useCoeffFractionModel_ = enable;
        LogOperation("UseFractionCoefficientModel", std::string("set to ") + (enable ? "ON" : "OFF"));
    }
    void ConfigureYieldRatios(double nsigRatio, double nsigMinRatio, double nsigMaxRatio,
                              double nbkgRatio, double nbkgMinRatio, double nbkgMaxRatio) {
        InitializeYieldVariables(nsigRatio, nsigMinRatio, nsigMaxRatio,
                                 nbkgRatio, nbkgMinRatio, nbkgMaxRatio);
    }
    
    // Statistical utilities
    double CalculateSignificance(const std::string& resultName = "default") const;
    double CalculatePurity(const std::string& resultName = "default") const;
    std::pair<double, double> GetSignalToBackgroundRatio(const std::string& resultName = "default") const;

    // Helper to copy MC-fit parameter values to data signal parameters and fix them
    void FixSignalParamsFromFitResult(const RooFitResult* rf, const std::vector<std::string>& prefixesToFix);
    
private:
    struct ConstraintApplier {
        template<typename SignalParams, typename BackgroundParams>
        static bool GaussianConstraint(MassFitterV2& self,
                                       const FitOpt& options,
                                       RooDataSet* dataset,
                                       RooDataSet* mcDataset,
                                       const SignalParams& signalParams,
                                       const BackgroundParams& backgroundParams,
                                       const std::vector<std::string>& paramsToConstrain,
                                       const std::string& resultName);

        template<typename SignalParams, typename BackgroundParams>
        static bool GaussianConstraintFromFile(MassFitterV2& self,
                                               const FitOpt& options,
                                               RooDataSet* dataset,
                                               const SignalParams& signalParams,
                                               const BackgroundParams& backgroundParams,
                                               const std::string& mcResultFile,
                                               const std::vector<std::string>& paramsToConstrain,
                                               const std::string& resultName);

        template<typename SignalParams, typename BackgroundParams>
        static bool SidebandPrefitBackground(MassFitterV2& self,
                                             const FitOpt& options,
                                             RooDataSet* dataset,
                                             const SignalParams& signalParams,
                                             const BackgroundParams& backgroundParams,
                                             double sbLoMin, double sbLoMax,
                                             double sbHiMin, double sbHiMax,
                                             double sigmaScale,
                                             const std::string& resultName);

        template<typename SignalParams, typename BackgroundParams>
        static bool GaussianConstraintWithSidebands(MassFitterV2& self,
                                                    const FitOpt& options,
                                                    RooDataSet* dataset,
                                                    RooDataSet* mcDataset,
                                                    const SignalParams& signalParams,
                                                    const BackgroundParams& backgroundParams,
                                                    const std::vector<std::string>& paramsToConstrain,
                                                    double sbLoMin, double sbLoMax,
                                                    double sbHiMin, double sbHiMax,
                                                    double sigmaScaleSignal,
                                                    double sigmaScaleBkg,
                                                    const std::string& resultName);

        template<typename SignalParams, typename BackgroundParams>
        static bool FixedSignalWithSidebands(MassFitterV2& self,
                                             const FitOpt& options,
                                             RooDataSet* dataset,
                                             RooDataSet* mcDataset,
                                             const SignalParams& signalParams,
                                             const BackgroundParams& backgroundParams,
                                             const std::vector<std::string>& prefixesToFix,
                                             double sbLoMin, double sbLoMax,
                                             double sbHiMin, double sbHiMax,
                                             double sigmaScaleBkg,
                                             const std::string& resultName);
    };

    struct MassModelBuilder {
        template<typename SignalParams, typename BackgroundParams>
        static void PrepareFullModel(MassFitterV2& self,
                                     const FitOpt& options,
                                     RooDataSet* dataset,
                                     const SignalParams& signalParams,
                                     const BackgroundParams& backgroundParams,
                                     const char* contextTag);

        template<typename BackgroundParams>
        static void PrepareBackgroundModel(MassFitterV2& self,
                                           const FitOpt& options,
                                           RooDataSet* dataset,
                                           const BackgroundParams& backgroundParams,
                                           const char* contextTag);
    };

    struct MassFitExecutor {
        static std::unique_ptr<RooFitResult> Execute(MassFitterV2& self,
                                                     std::unique_ptr<FitStrategy> strategy,
                                                     FitOpt cfg,
                                                     const char* contextTag);
    };

    // Core member variables
    std::string name_;
    std::string massVarName_;
    // bool useDeltaMass_ = false;
    
    // Mass variables (smart pointers for automatic cleanup)
    std::unique_ptr<RooRealVar> mass_;
    std::unique_ptr<RooRealVar> massDaughter1_;
    std::unique_ptr<RooRealVar> massPion_;
    RooRealVar* activeMassVar_ = nullptr;
    
    // Data (non-owning pointers - data ownership is external)
    RooDataSet* fullDataset_ = nullptr;
    RooDataSet* activeDataset_ = nullptr;
    
    // PDF components (smart pointers for automatic cleanup)
    std::unique_ptr<RooAbsPdf> signalPdf_;
    std::unique_ptr<RooAbsPdf> backgroundPdf_;
    std::unique_ptr<RooAddPdf> totalPdf_;
    
    // Yield parameterization (two modes)
    YieldMode yieldMode_ = YieldMode::Fraction;
    // Fraction mode: Ntot and fraction fsig with derived yields nsig, nbkg
    std::unique_ptr<RooRealVar> fsig_;     // signal fraction in [0,1]
    std::unique_ptr<RooRealVar> ntot_;     // total expected yield
    std::unique_ptr<RooAbsReal> nsig_;     // nsig = fsig * ntot
    std::unique_ptr<RooAbsReal> nbkg_;     // nbkg = (1-fsig) * ntot
    // Independent mode: direct yields (non-owning convenience pointers)
    RooRealVar* nsigVar_ = nullptr;
    RooRealVar* nbkgVar_ = nullptr;
    // Cache last ratios for initialization on data-dependent updates
    double lastNsigRatio_ = 0.1;
    double lastNsigMinRatio_ = 0.0;
    double lastNsigMaxRatio_ = 1.0;
    double lastNbkgRatio_ = 0.0;
    double lastNbkgMinRatio_ = 0.0;
    double lastNbkgMaxRatio_ = 1.0;
    std::vector<std::unique_ptr<RooRealVar>> additionalYields_;
    
    // Configuration
    FitOpt fitConfig_;
    
    // Dependency injection components (smart pointers for automatic cleanup)
    std::unique_ptr<PDFFactory> pdfFactory_;
    std::unique_ptr<FitStrategy> fitStrategy_;  // Use concrete FitStrategy instead of interface
    std::unique_ptr<ResultManager> resultManager_;  // Use concrete ResultManager instead of interface
    
    // Parameter storage for cleanup (smart pointers for automatic cleanup)
    std::vector<std::unique_ptr<RooRealVar>> parameters_;
    std::vector<std::unique_ptr<RooAbsPdf>> componentPdfs_;
    
    // Initialization and setup helpers
    void InitializeDefaults();
    void InitializeMassVariables(double massMin, double massMax);
    void InitializeYieldVariables(double nsigRatio, double nsigMinRatio, double nsigMaxRatio,
                                 double nbkgRatio, double nbkgMinRatio, double nbkgMaxRatio);
    void CreateTotalPDF();
    void SetupDefaultDependencies();
    // Yield mode management
    void SetYieldMode(YieldMode mode) {
        if (yieldMode_ == mode) return;
        yieldMode_ = mode;
        // Rebuild with cached ratios
        InitializeYieldVariables(lastNsigRatio_, lastNsigMinRatio_, lastNsigMaxRatio_,
                                 lastNbkgRatio_, lastNbkgMinRatio_, lastNbkgMaxRatio_);
        CreateTotalPDF();
    }
    
    // Validation helpers
    bool ValidateInputs(RooDataSet* dataset) const;
    bool ValidatePDFSetup() const;
    bool ValidateConfiguration() const;
    
    // Utility helpers
    std::string GenerateResultName() const;
    std::string GetCurrentTimestamp() const;
    void HandleFitException(const std::exception& e, const std::string& context);
    void LogOperation(const std::string& operation, const std::string& details = "") const;
    
    // Template implementation helpers
    template<typename T>
    std::string GetPDFType() const;
    
    template<typename T>
    std::unique_ptr<RooAbsPdf> CreatePDFFromParams(const T& params, const std::string& name, bool isSignal);
    
    // Statistics calculation helpers
    double CalculateIntegral(RooAbsPdf* pdf, double min, double max) const;
    std::pair<double, double> GetYieldValues(const std::string& resultName) const;

    // Flag to switch to coefficient-based fraction model in Fraction yield mode
    bool useCoeffFractionModel_ = false;

    // Helper to de-duplicate result storing and bookkeeping
    void StoreFitOutcome(const std::string& resultName,
                         std::unique_ptr<RooFitResult> fitResult,
                         const std::string& label,
                         const std::string& workspacePrefix = "workspace_");

    // Apply config range consistently to active mass variable
    void ApplyConfigRangeToMassVar(const FitOpt& cfg) {
        if (!activeMassVar_) return;
        if (!cfg.rangeName.empty()) {
            activeMassVar_->setRange(cfg.rangeName.c_str(), cfg.rangeMin, cfg.rangeMax);
        }
        activeMassVar_->setMin(cfg.rangeMin);
        activeMassVar_->setMax(cfg.rangeMax);
    }
};

    // Implementation of template methods

inline MassFitterV2::MassFitterV2(const std::string& name, 
                                   const std::string& massVar,
                                   double massMin, 
                                   double massMax,
                                   std::unique_ptr<PDFFactory> pdfFactory,
                                   std::unique_ptr<FitStrategy> fitStrategy,
                                   std::unique_ptr<ResultManager> resultManager,
                                   YieldMode yieldMode)
    : name_(name), massVarName_(massVar) {
    yieldMode_ = yieldMode;
    InitializeMassVariables(massMin, massMax);
    // Initialize yields with defaults (ratios will be updated when PerformFit is called)
    InitializeYieldVariables( lastNsigRatio_, lastNsigMinRatio_, lastNsigMaxRatio_,
                              lastNbkgRatio_, lastNbkgMinRatio_, lastNbkgMaxRatio_);
    // Setup DI objects if provided
    pdfFactory_ = pdfFactory ? std::move(pdfFactory) : std::make_unique<PDFFactory>(activeMassVar_);
    fitStrategy_ = fitStrategy ? std::move(fitStrategy) : std::make_unique<BinnedFitStrategy>();
    resultManager_ = resultManager ? std::move(resultManager) : std::make_unique<ResultManager>();
}

template<typename SignalParams>
void MassFitterV2::SetSignalPDF(const SignalParams& params, const std::string& name) {
    try {
        LogOperation("SetSignalPDF", "Creating " + name + " with type " + GetPDFType<SignalParams>());
        
        if (!pdfFactory_) {
            LOG_AND_THROW(PDFCreationException, "PDF factory not initialized", "SetSignalPDF");
        }
        
        Validator::ValidateNotEmpty(name, "PDF name");
        
        signalPdf_ = CreatePDFFromParams(params, name, true);
        
        if (!signalPdf_) {
            LOG_AND_THROW(PDFCreationException, "Failed to create signal PDF", "SetSignalPDF");
        }
        
        ErrorHandlerManager::Instance().LogInfo("Signal PDF created successfully: " + name, "MassFitterV2");
        
    } catch (const std::exception& e) {
        HandleFitException(e, "SetSignalPDF");
        throw;
    }
}

template<typename BackgroundParams>
void MassFitterV2::SetBackgroundPDF(const BackgroundParams& params, const std::string& name) {
    try {
        LogOperation("SetBackgroundPDF", "Creating " + name + " with type " + GetPDFType<BackgroundParams>());
        
        if (!pdfFactory_) {
            LOG_AND_THROW(PDFCreationException, "PDF factory not initialized", "SetBackgroundPDF");
        }
        
        Validator::ValidateNotEmpty(name, "PDF name");
        
        backgroundPdf_ = CreatePDFFromParams(params, name, false);
        
        if (!backgroundPdf_) {
            LOG_AND_THROW(PDFCreationException, "Failed to create background PDF", "SetBackgroundPDF");
        }
        
        ErrorHandlerManager::Instance().LogInfo("Background PDF created successfully: " + name, "MassFitterV2");
        
    } catch (const std::exception& e) {
        HandleFitException(e, "SetBackgroundPDF");
        throw;
    }
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformFit(const FitOpt& options, RooDataSet* dataset,
                              const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                              const std::string& resultName) {
    try {
        LogOperation("PerformFit", "Starting fit with result name: " + resultName);
        
        // Input validation
        Validator::ValidateNotNull(dataset, "dataset");
        if (!ValidateInputs(dataset)) {
            LOG_AND_THROW(DataException, "Invalid dataset provided", "PerformFit");
        }
        
        // Set data and apply cuts
        SetData(dataset);
        if (!options.cutExpr.empty()) {
            LogOperation("ApplyCut", "Applying cut: " + options.cutExpr);
            ApplyCut(options.cutExpr);
        }
        
        // Create PDFs
        SetSignalPDF(signalParams, "signal");
        SetBackgroundPDF(backgroundParams, "background");
        CreateTotalPDF();
        
        // Validate PDF setup
        if (!ValidatePDFSetup()) {
            LOG_AND_THROW(ValidationException, "PDF setup validation failed", {"Total PDF not created properly"});
        }
        
        // Create fit configuration
        FitOpt config = options.ToFitConfig();
        // Ensure mass variable uses the same named range and bounds as the config
        ApplyConfigRangeToMassVar(config);
        if (activeMassVar_) {
            std::cout << "[FitRange] rangeName='" << config.rangeName << "', ["
                      << config.rangeMin << ", " << config.rangeMax << "]" << std::endl;
            std::cout << "[FitRange] var '" << activeMassVar_->GetName() << "' bounds: ["
                      << activeMassVar_->getMin() << ", " << activeMassVar_->getMax() << "]" << std::endl;
        }
        
        // Select appropriate strategy and execute fit based on FitMethod
        auto created = FitStrategyFactory::CreateStrategy(config.fitMethod);
        fitStrategy_ = std::move(created);
        const char* methodName = FitMethodName(config.fitMethod);

        if (!fitStrategy_) {
            throw FitException("PerformFit", -1, "Failed to create fit strategy for method: " + std::string(methodName));
        }

        LogOperation("FitStrategy", std::string("Selected ") + fitStrategy_->GetName() + " for " + methodName + " method");
        
        auto fitResult = fitStrategy_->Execute(totalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("Fit execution returned null result", "PerformFit");
            throw FitException("PerformFit", -1, "Fit execution returned null result");
        }
        fitResult->Print("v");
        
        // Store results (centralized)
        std::string finalResultName = resultName.empty() ? GenerateResultName() : resultName;
        StoreFitOutcome(finalResultName, std::move(fitResult), "StandardFit", "workspace_");
        
        // Log successful completion
        LogOperation("PerformFit", "Fit completed successfully: " + finalResultName);
        ErrorHandlerManager::Instance().LogInfo("Fit completed successfully: " + finalResultName, "MassFitterV2");
        
        return true;
        
    } catch (const std::exception& e) {
        HandleFitException(e, "PerformFit");
        return false;
    }
}

template<typename SignalParams>
bool MassFitterV2::PerformMCFit(const FitOpt& options, RooDataSet* mcDataset,
                                const SignalParams& signalParams, const std::string& resultName) {
    try {
        LogOperation("PerformMCFit", "Starting MC fit with result name: " + resultName);
        
        // Input validation
        Validator::ValidateNotNull(mcDataset, "mcDataset");
        if (!ValidateInputs(mcDataset)) {
            LOG_AND_THROW(DataException, "Invalid MC dataset provided", "PerformMCFit");
        }
        
        // Clear any stale combined/ background PDFs from previous data fits
        // to avoid accidental usage in MC-specific post-processing (e.g., chi2).
        totalPdf_.reset();
        backgroundPdf_.reset();

        // Set data and apply MC cuts
        SetData(mcDataset);
        if (!options.cutMCExpr.empty()) {
            std::string mcCutExpr = NormalizeCutExpression(options.cutMCExpr, options.useIntegratedMC);
            LogOperation("ApplyCut", "Applying MC cut: " + mcCutExpr);
            ApplyCut(mcCutExpr);
        }
        
        // Create signal PDF only for MC fit
        SetSignalPDF(signalParams, "mcSignal");
        
        if (!signalPdf_) {
            LOG_AND_THROW(PDFCreationException, "MC signal PDF creation failed", "PerformMCFit");
        }
        
        // Create MC fit configuration (force unbinned NLL for MC)
        FitOpt config = options.ToFitConfig();
        config.fitMethod = FitMethod::NLL;   // Always use NLL for MC fits
        config.useMinos = false;             // MC fits typically don't need Minos
        // Align mass variable range with configuration
        ApplyConfigRangeToMassVar(config);
        
        // Select appropriate strategy based on FitMethod for MC and execute
        auto mcStrategy = FitStrategyFactory::CreateStrategy(config.fitMethod);
        const char* mcMethodName = FitMethodName(config.fitMethod);

        if (!mcStrategy) {
            throw FitException("PerformMCFit", -1, "Failed to create MC fit strategy for method: " + std::string(mcMethodName));
        }

        LogOperation("FitStrategy", std::string("Selected ") + mcStrategy->GetName() + " for MC " + mcMethodName + " method");
        
        // Perform MC fit
        auto fitResult = mcStrategy->Execute(signalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("MC fit execution returned null result", "PerformMCFit");
            throw FitException("PerformMCFit", -1, "MC fit execution returned null result");
        }
        fitResult->Print("v");
        
        // Store MC results (centralized)
        std::string finalResultName = resultName.empty() ? GenerateResultName() + "_MC" : resultName;
        cout << "123" << endl;
        StoreFitOutcome(finalResultName, std::move(fitResult), "MCFit", "mcWorkspace_");
        cout << "456" << endl;
        
        LogOperation("PerformMCFit", "MC fit completed successfully: " + finalResultName);
        ErrorHandlerManager::Instance().LogInfo("MC fit completed successfully: " + finalResultName, "MassFitterV2");
        
        return true;
        
    } catch (const std::exception& e) {
        HandleFitException(e, "PerformMCFit");
        return false;
    }
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformConstraintFit(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                        const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                        const std::vector<std::string>& constraintParameters,
                                        const std::string& resultName) {
    try {
        LogOperation("PerformConstraintFit", "Starting constraint fit with " + 
                    std::to_string(constraintParameters.size()) + " constraints");
        
        // Input validation
        Validator::ValidateNotNull(dataset, "dataset");
        Validator::ValidateNotNull(mcDataset, "mcDataset");
        
        // Align mass variable range with configuration
        { FitOpt baseCfg = options.ToFitConfig(); ApplyConfigRangeToMassVar(baseCfg); }

        // First perform MC fit to establish constraints
        std::string mcResultName = resultName + "_constraint_mc";
        bool mcSuccess = PerformMCFit(options, mcDataset, signalParams, mcResultName);
        
        if (!mcSuccess) {
            ErrorHandlerManager::Instance().LogError("MC fit for constraints failed", "PerformConstraintFit");
            throw FitException("PerformConstraintFit", -1, "MC fit for constraints failed");
        }
        
        // Set data for constraint fit
        SetData(dataset);
        if (!options.cutExpr.empty()) {
            LogOperation("ApplyCut", "Applying constraint fit cut: " + options.cutExpr);
            ApplyCut(options.cutExpr);
        }
        
        // Create PDFs
        SetSignalPDF(signalParams, "signal");
        SetBackgroundPDF(backgroundParams, "background");
        CreateTotalPDF();
        
        // Create fit configuration
        FitOpt config = options.ToFitConfig();
        
        // Create constraint fit strategy with MC results (always uses ConstraintStrategy)
        auto constraintStrategy = FitStrategyFactory::CreateConstraintStrategy(
            options.outputDir + "/" + options.outputFile, constraintParameters);
        
        LogOperation("FitStrategy", "Using " + constraintStrategy->GetName() + " for constraint fitting with " + 
                    (config.fitMethod == FitMethod::NLL ? "NLL" : 
                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : 
                     config.fitMethod == FitMethod::Extended ? "Extended" : "Robust") + " method");
        std::cout << "[MassFitterV2] Using constraint fit strategy: " << constraintStrategy->GetName() 
                  << " (method: " << (config.fitMethod == FitMethod::NLL ? "NLL" : 
                                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : 
                                     config.fitMethod == FitMethod::Extended ? "Extended" : "Robust") << ")" << std::endl;
        
        // Perform constraint fit
        auto fitResult = constraintStrategy->Execute(totalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("Constraint fit execution returned null result", "PerformConstraintFit");
            throw FitException("PerformConstraintFit", -1, "Constraint fit execution returned null result");
        }
        fitResult->Print("v");
        
        // Create constraint workspace
        auto workspace = std::make_unique<RooWorkspace>(("constraintWorkspace_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        workspace->import(*signalPdf_);
        workspace->import(*backgroundPdf_);
        
        // Store constraint fit results (centralized)
        std::string finalResultName = resultName.empty() ? GenerateResultName() + "_constraint" : resultName;
        StoreFitOutcome(finalResultName, std::move(fitResult), "ConstraintFit", "constraintWorkspace_");
        
        LogOperation("PerformConstraintFit", "Constraint fit completed successfully: " + finalResultName);
        ErrorHandlerManager::Instance().LogInfo("Constraint fit completed successfully: " + finalResultName, "MassFitterV2");
        
        return true;
        
    } catch (const std::exception& e) {
        HandleFitException(e, "PerformConstraintFit");
        return false;
    }
}

// Type trait specializations for PDF type identification
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::GaussianParams>() const { return "Gaussian"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::DoubleGaussianParams>() const { return "DoubleGaussian"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::CrystalBallParams>() const { return "CrystalBall"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::DBCrystalBallParams>() const { return "DBCrystalBall"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::DoubleDBCrystalBallParams>() const { return "DoubleDBCrystalBall"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::VoigtianParams>() const { return "Voigtian"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::ExponentialBkgParams>() const { return "Exponential"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::ChebychevBkgParams>() const { return "Chebychev"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::PolynomialBkgParams>() const { return "Polynomial"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::PhenomenologicalParams>() const { return "Phenomenological"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::Phenomenological2Params>() const { return "Phenomenological2"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::DstD0Params>() const { return "DstD0"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::ExpErfBkgParams>() const { return "ExpErf"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::DstBkgParams>() const { return "DstBg"; }
template<> inline std::string MassFitterV2::GetPDFType<PDFParams::ThresholdFuncParams>() const { return "ThresholdFunction"; }

template<typename T>
std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams(const T& params, const std::string& name, bool isSignal) {
    try {
        // Use template specialization to call the correct PDFFactory method
        // This should not be reached - template specializations handle all cases
        return nullptr;
    } catch (const std::exception& e) {
        ErrorHandlerManager::Instance().LogError(std::string("Failed to create PDF: ") + e.what(), "MassFitterV2");
        return nullptr;
    }
}

// Template specializations for specific PDF creation
template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::GaussianParams>(
    const PDFParams::GaussianParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateGaussian(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::DoubleGaussianParams>(
    const PDFParams::DoubleGaussianParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateDoubleGaussian(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::CrystalBallParams>(
    const PDFParams::CrystalBallParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateCrystalBall(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::DBCrystalBallParams>(
    const PDFParams::DBCrystalBallParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateDBCrystalBall(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::VoigtianParams>(
    const PDFParams::VoigtianParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateVoigtian(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::ExponentialBkgParams>(
    const PDFParams::ExponentialBkgParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateExponential(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::ChebychevBkgParams>(
    const PDFParams::ChebychevBkgParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateChebychev(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::PolynomialBkgParams>(
    const PDFParams::PolynomialBkgParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreatePolynomial(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::ThresholdFuncParams>(
    const PDFParams::ThresholdFuncParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateThresholdFunction(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::PhenomenologicalParams>(
    const PDFParams::PhenomenologicalParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreatePhenomenological(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::Phenomenological2Params>(
    const PDFParams::Phenomenological2Params& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreatePhenomenological2(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::DstD0Params>(
    const PDFParams::DstD0Params& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateDstD0Background(params, name);
}


// (duplicate specialization for Phenomenological removed)

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::ExpErfBkgParams>(
    const PDFParams::ExpErfBkgParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateExpErf(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::DstBkgParams>(
    const PDFParams::DstBkgParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateDstBg(params, name);
}

template<>
inline std::unique_ptr<RooAbsPdf> MassFitterV2::CreatePDFFromParams<PDFParams::DoubleDBCrystalBallParams>(
    const PDFParams::DoubleDBCrystalBallParams& params, const std::string& name, bool isSignal) {
    return pdfFactory_->CreateDoubleDBCrystalBall(params, name);
}

// Inline implementations of essential methods
inline MassFitterV2::MassFitterV2(const std::string& name,
                                   const std::string& massVar, 
                                   double massMin, 
                                   double massMax,
                                   double nsigRatio,
                                   double nsigMinRatio, 
                                   double nsigMaxRatio,
                                   double nbkgRatio,
                                   double nbkgMinRatio,
                                   double nbkgMaxRatio,
                                   YieldMode yieldMode) 
    : name_(name), massVarName_(massVar) {
    yieldMode_ = yieldMode;
    InitializeMassVariables(massMin, massMax);
    InitializeYieldVariables(nsigRatio, nsigMinRatio, nsigMaxRatio, nbkgRatio, nbkgMinRatio, nbkgMaxRatio);
    SetupDefaultDependencies();
}

inline void MassFitterV2::InitializeMassVariables(double massMin, double massMax) {
    mass_ = std::make_unique<RooRealVar>(massVarName_.c_str(), "Mass Variable", massMin, massMax);
    activeMassVar_ = mass_.get();
}

inline void MassFitterV2::InitializeYieldVariables(double nsigRatio, double nsigMinRatio, double nsigMaxRatio,
                                                   double nbkgRatio, double nbkgMinRatio, double nbkgMaxRatio) {
    // Cache the ratios for later updates
    lastNsigRatio_ = nsigRatio; lastNsigMinRatio_ = nsigMinRatio; lastNsigMaxRatio_ = nsigMaxRatio;
    lastNbkgRatio_ = nbkgRatio; lastNbkgMinRatio_ = nbkgMinRatio; lastNbkgMaxRatio_ = nbkgMaxRatio;

    // Reset existing variables
    fsig_.reset(); ntot_.reset();
    nsig_.reset(); nbkg_.reset();
    nsigVar_ = nullptr; nbkgVar_ = nullptr;

    if (yieldMode_ == YieldMode::Fraction) {
        // Fraction-based parameterization: nsig = fsig * Ntot, nbkg = (1-fsig) * Ntot
        double fInit = std::max(0.0, std::min(1.0, nsigRatio));
        double fMin  = std::max(0.0, nsigMinRatio);
        double fMax  = std::min(1.0, nsigMaxRatio);
        fsig_ = std::make_unique<RooRealVar>("fsig", "Signal Fraction", fInit, fMin, fMax);
        
        // Initialize Ntot with a placeholder; updated when data is set
        ntot_ = std::make_unique<RooRealVar>("Ntot", "Total Expected Yield", 1000.0, 0.0, 1e12);
        nsig_.reset(new RooFormulaVar("nsig", "@0*@1", RooArgList(*fsig_, *ntot_)));
        nbkg_.reset(new RooFormulaVar("nbkg", "(1-@0)*@1", RooArgList(*fsig_, *ntot_)));
        LogOperation("InitializeYieldVariables", "Fraction mode: fsig init=" + std::to_string(fInit) +
                     ", range=[" + std::to_string(fMin) + "," + std::to_string(fMax) + "]");
    } else {
        // Independent-yield parameterization: nsig, nbkg are free yields
        double ntotGuess = 1000.0;
        double nSigInit = std::max(0.0, nsigRatio) * ntotGuess;
        double nBkgInit = std::max(0.0, nbkgRatio) * ntotGuess;
        double nSigMax  = std::max(10.0, std::min(1.0, nsigMaxRatio) * 5.0 * ntotGuess);
        double nBkgMax  = std::max(10.0, std::min(1.0, nbkgMaxRatio) * 5.0 * ntotGuess);
        nsig_.reset(new RooRealVar("nsig", "Signal Yield", nSigInit, 0.0, nSigMax));
        nbkg_.reset(new RooRealVar("nbkg", "Background Yield", nBkgInit, 0.0, nBkgMax));
        nsigVar_ = dynamic_cast<RooRealVar*>(nsig_.get());
        nbkgVar_ = dynamic_cast<RooRealVar*>(nbkg_.get());
    }
}

inline void MassFitterV2::SetupDefaultDependencies() {
    if (!pdfFactory_) {
        pdfFactory_ = std::make_unique<PDFFactory>(activeMassVar_);
    }
    if (!fitStrategy_) {
        fitStrategy_ = std::make_unique<BinnedFitStrategy>();
    }
    if (!resultManager_) {
        resultManager_ = std::make_unique<ResultManager>();
    }
}

inline void MassFitterV2::SetData(RooDataSet* dataset) {
    fullDataset_ = dataset;
    activeDataset_ = dataset;
    
    if (!dataset) return;
    // Print both weighted and raw entry counts for clarity
    const bool isWeighted = dataset->isWeighted();
    const double sumEntriesWeighted = dataset->sumEntries(Form("%s > %f && %s < %f", activeMassVar_->GetName(), activeMassVar_->getMin(), activeMassVar_->GetName(), activeMassVar_->getMax()));
    const int numEntriesRaw = dataset->numEntries();
    std::cout << "[Data] isWeighted=" << (isWeighted ? "true" : "false")
              << ", sumEntries=" << sumEntriesWeighted
              << ", numEntries=" << numEntriesRaw << std::endl;
    int dataSize = sumEntriesWeighted; // keep legacy behavior: use weighted sum for Ntot
    if (yieldMode_ == YieldMode::Fraction) {
        // Update total yield based on dataset size
        if (ntot_) {
            // Set Ntot to the current (weighted) data size without tightening the range,
            // to avoid unintended clamping after subsequent cuts.
            ntot_->setVal(dataSize);
            // Keep the wide default range set at construction (0 .. 1e12).
            // ntot_->setConstant(true);
            double fval = fsig_ ? fsig_->getVal() : -1.0;
            LogOperation("SetData", "Updated Ntot to " + std::to_string(dataSize) +
                         ", fsig=" + std::to_string(fval));
        }
    } else {
        // Update independent yield guesses based on ratios
        if (nsigVar_ && nbkgVar_) {
            auto clamp01 = [](double x){ return std::max(0.0, std::min(1.0, x)); };
            double nEntries = static_cast<double>(dataSize);
            // Convert ratio ranges → absolute ranges and clamp to [0, entries]
            double nsMin = clamp01(lastNsigMinRatio_) * nEntries;
            double nsMax = clamp01(lastNsigMaxRatio_) * nEntries;
            if (nsMin > nsMax) std::swap(nsMin, nsMax);
            double nsVal = clamp01(lastNsigRatio_) * nEntries;
            nsVal = std::max(nsMin, std::min(nsVal, nsMax));

            double nbMin = clamp01(lastNbkgMinRatio_) * nEntries;
            double nbMax = clamp01(lastNbkgMaxRatio_) * nEntries;
            if (nbMin > nbMax) std::swap(nbMin, nbMax);
            double nbVal = clamp01(lastNbkgRatio_) * nEntries;
            nbVal = std::max(nbMin, std::min(nbVal, nbMax));

            nsigVar_->setRange(nsMin, nsMax);
            nbkgVar_->setRange(nbMin, nbMax);
            nsigVar_->setVal(nsVal);
            nbkgVar_->setVal(nbVal);
            nsigVar_->setConstant(false);
            nbkgVar_->setConstant(false);
        }
    }
}

inline void MassFitterV2::ApplyCut(const std::string& cutExpr) {
    if (fullDataset_ && !cutExpr.empty()) {
        std::string normalizedCut = NormalizeCutExpression(cutExpr);
        activeDataset_ = (RooDataSet*)fullDataset_->reduce(normalizedCut.c_str());
        
        if (!activeDataset_) return;
        int cutDataSize = activeDataset_->sumEntries();
        std::cout << "Cut applied. New data size: " << cutDataSize << std::endl;
        if (yieldMode_ == YieldMode::Fraction) {
            // Update Ntot based on cut dataset size
            if (ntot_) {
                ntot_->setVal(cutDataSize);
                // ntot_->setRange(std::max(0.0, cutDataSize * 0.2), std::max(1.0, cutDataSize * 5.0));
                // ntot_->setConstant(true);
                double fval = fsig_ ? fsig_->getVal() : -1.0;
                LogOperation("ApplyCut", "Updated Ntot to " + std::to_string(cutDataSize) + ", ntot_=" + std::to_string(ntot_->getVal()) +
                             ", fsig=" + std::to_string(fval));
            }
        } else {
            // Update nsig/nbkg based on ratios
            if (nsigVar_ && nbkgVar_) {
                auto clamp01 = [](double x){ return std::max(0.0, std::min(1.0, x)); };
                double nEntries = static_cast<double>(cutDataSize);
                // Convert ratio ranges → absolute ranges and clamp to [0, entries]
                double nsMin = clamp01(lastNsigMinRatio_) * nEntries;
                double nsMax = clamp01(lastNsigMaxRatio_) * nEntries;
                if (nsMin > nsMax) std::swap(nsMin, nsMax);
                double nsVal = clamp01(lastNsigRatio_) * nEntries;
                nsVal = std::max(nsMin, std::min(nsVal, nsMax));

                double nbMin = clamp01(lastNbkgMinRatio_) * nEntries;
                double nbMax = clamp01(lastNbkgMaxRatio_) * nEntries;
                if (nbMin > nbMax) std::swap(nbMin, nbMax);
                double nbVal = clamp01(lastNbkgRatio_) * nEntries;
                nbVal = std::max(nbMin, std::min(nbVal, nbMax));

                nsigVar_->setRange(nsMin, nsMax);
                nbkgVar_->setRange(nbMin, nbMax);
                nsigVar_->setVal(nsVal);
                nbkgVar_->setVal(nbVal);
                std::cout << "Updated yield parameters after cut:" << std::endl;
                std::cout << "  Signal: " << nsVal << " [" << nsMin << ", " << nsMax << "]" << std::endl;
                std::cout << "  Background: " << nbVal << " [" << nbMin << ", " << nbMax << "]" << std::endl;
                cout << "nSigVar : " << nsigVar_->getVal() << " [" << nsigVar_->getMin() << ", " << nsigVar_->getMax() << "]" << endl;
                cout << "nBkgVar : " << nbkgVar_->getVal() << " [" << nbkgVar_->getMin() << ", " << nbkgVar_->getMax() << "]" << endl;
            }
        }
    }
}

// Perform data fit with Gaussian constraints derived from MC on selected parameters
template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformGaussianConstraintFitWithMC(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                                      const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                      const std::vector<std::string>& paramsToConstrain,
                                                      const std::string& resultName) {
    return ConstraintApplier::GaussianConstraint(*this, options, dataset, mcDataset,
                                                 signalParams, backgroundParams,
                                                 paramsToConstrain, resultName);
}

// Fixed-from-MC helper: copy MC fit values into data signal parameters and fix them
inline void MassFitterV2::FixSignalParamsFromFitResult(const RooFitResult* rf, const std::vector<std::string>& prefixesToFix) {
    if (!rf || !totalPdf_) return;

    // Debug: print requested prefixes
    if (!prefixesToFix.empty()) {
        std::ostringstream oss;
        for (size_t i=0;i<prefixesToFix.size();++i) { if (i) oss << ", "; oss << prefixesToFix[i]; }
        std::cout << "[FixedFromMC] Prefixes requested to fix: [" << oss.str() << "]" << std::endl;
    } else {
        std::cout << "[FixedFromMC] No prefixes requested; nothing will be fixed." << std::endl;
    }

    // Build quick lookup of MC final parameter values (avoid copying RooArgList)
    std::map<std::string, double> mcVals;
    const RooArgList& finals = rf->floatParsFinal();
    int nFinals = finals.getSize();
    for (int i = 0; i < nFinals; ++i) {
        RooAbsArg* arg = finals.at(i);
        if (!arg) continue;
        if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
            mcVals[v->GetName()] = v->getVal();
        }
    }

    // Allowed prefixes to avoid fixing yields or unrelated vars
    auto isAllowed = [](const std::string& p){
        static const std::vector<std::string> allow = {
            "mean","sigma","sigma1","sigma2","alpha","alphaL","alphaR","n","nL","nR","width","fraction"
        };
        for (const auto& a : allow) if (p == a) return true;
        return false;
    };

    std::unique_ptr<RooArgSet> vars(totalPdf_->getVariables());
    int fixedCount = 0;
    for (const auto& prefix : prefixesToFix) {
        if (!isAllowed(prefix)) continue;
        const std::string dataName = prefix + "_signal";
        const std::string mcName1 = prefix + "_mcSignal"; // preferred
        const std::string mcName2 = prefix + "_signal";    // fallback

        double val = NAN;
        if (mcVals.count(mcName1)) val = mcVals[mcName1];
        else if (mcVals.count(mcName2)) val = mcVals[mcName2];
        if (std::isnan(val)) {
            std::cout << "[FixedFromMC] No MC value for prefix '" << prefix
                      << "' (tried '" << mcName1 << "', '" << mcName2 << "')" << std::endl;
            continue;
        }

        if (auto* target = dynamic_cast<RooRealVar*>(vars->find(dataName.c_str()))) {
            target->setVal(val);
            target->setConstant(true);
            const char* src = mcVals.count(mcName1) ? mcName1.c_str() : mcName2.c_str();
            std::cout << "[FixedFromMC] Fixed '" << dataName << "' from '" << src << "' = " << val << std::endl;
            fixedCount++;
        } else {
            std::cout << "[FixedFromMC] Data parameter not found: '" << dataName << "'" << std::endl;
        }
    }
    std::cout << "[FixedFromMC] Total fixed parameters: " << fixedCount << std::endl;
}

// Fixed-from-MC: fit MC, fix selected data signal params to MC values, then fit data
template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformFixedParameterFitWithMC(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                                  const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                  const std::vector<std::string>& paramPrefixesToFix,
                                                  const std::string& resultName) {
    try {
        LogOperation("PerformFixedParameterFitWithMC", "Start");
        if (!paramPrefixesToFix.empty()) {
            std::ostringstream oss;
            for (size_t i=0;i<paramPrefixesToFix.size();++i) { if (i) oss << ", "; oss << paramPrefixesToFix[i]; }
            std::cout << "[FixedFromMC] Prefix list: [" << oss.str() << "]" << std::endl;
        } else {
            std::cout << "[FixedFromMC] Prefix list is empty" << std::endl;
        }

        Validator::ValidateNotNull(dataset, "dataset");
        Validator::ValidateNotNull(mcDataset, "mcDataset");

        // 1) MC fit using dedicated label to avoid name clashes
        std::string mcResName = resultName.empty() ? (GenerateResultName() + std::string("_mcfix")) : (resultName + std::string("_mcfix"));
        bool mcOk = PerformMCFit(options, mcDataset, signalParams, mcResName);
        std::cout << "[FixedFromMC] MC result name: " << mcResName << ", success=" << (mcOk?"true":"false") << std::endl;
        if (!mcOk) {
            LOG_AND_THROW(MassFitterException, "MC fit failed for FixedFromMC", "PerformFixedParameterFitWithMC");
        }
        RooFitResult* mcRf = GetRooFitResult(mcResName);
        if (!mcRf) {
            LOG_AND_THROW(MassFitterException, "MC RooFitResult missing", "PerformFixedParameterFitWithMC");
        }

        // 2) Build data PDFs and model
        SetData(dataset);
        if (!options.cutExpr.empty()) ApplyCut(options.cutExpr);
        SetSignalPDF(signalParams, "signal");
        SetBackgroundPDF(backgroundParams, "background");
        CreateTotalPDF();
        if (!ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF not configured", "PerformFixedParameterFitWithMC");
        }

        // 3) Fix chosen parameters to MC values
        FixSignalParamsFromFitResult(mcRf, paramPrefixesToFix);

        // 4) Fit data with fixed parameters
        FitOpt cfg = options.ToFitConfig();
        ApplyConfigRangeToMassVar(cfg);

        // Choose strategy based on method (FixedFromMC maps to Basic)
        fitStrategy_ = FitStrategyFactory::CreateStrategy(cfg.fitMethod);
        if (!fitStrategy_) {
            throw FitException("PerformFixedParameterFitWithMC", -1, "Failed to create fit strategy for FixedFromMC");
        }
        auto fitResult = fitStrategy_->Execute(totalPdf_.get(), activeDataset_, cfg, activeMassVar_);
        if (!fitResult) {
            throw FitException("PerformFixedParameterFitWithMC", -1, "FixedFromMC data fit returned null result");
        }

        std::string finalName = resultName.empty() ? (GenerateResultName() + std::string("_fixed_from_mc")) : resultName;
        StoreFitOutcome(finalName, std::move(fitResult), "FixedFromMC", "fixedFromMC_ws_");

        LogOperation("PerformFixedParameterFitWithMC", "Completed: " + finalName);
        return true;

    } catch (const std::exception& e) {
        HandleFitException(e, "PerformFixedParameterFitWithMC");
        return false;
    }
}

// Fixed-from-MC using a saved RooFitResult file
template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformFixedParameterFitWithMCFile(const FitOpt& options, RooDataSet* dataset,
                                                      const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                      const std::string& mcResultFile,
                                                      const std::vector<std::string>& paramPrefixesToFix,
                                                      const std::string& resultName) {
    try {
        LogOperation("PerformFixedParameterFitWithMCFile", "Start");

        Validator::ValidateNotNull(dataset, "dataset");
        if (mcResultFile.empty()) {
            LOG_AND_THROW(MassFitterException, "Empty MC result file path", "PerformFixedParameterFitWithMCFile");
        }

        // Build data PDFs
        SetData(dataset);
        if (!options.cutExpr.empty()) ApplyCut(options.cutExpr);
        SetSignalPDF(signalParams, "signal");
        SetBackgroundPDF(backgroundParams, "background");
        CreateTotalPDF();
        if (!ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF not configured", "PerformFixedParameterFitWithMCFile");
        }

        // Load MC RooFitResult
        std::unique_ptr<TFile> fin(TFile::Open(mcResultFile.c_str(), "READ"));
        if (!fin || fin->IsZombie()) {
            LOG_AND_THROW(MassFitterException, std::string("Cannot open file ") + mcResultFile, "PerformFixedParameterFitWithMCFile");
        }
        RooFitResult* rf = dynamic_cast<RooFitResult*>(fin->Get("fitResult"));
        if (!rf) {
            LOG_AND_THROW(MassFitterException, "fitResult not found in MC file", "PerformFixedParameterFitWithMCFile");
        }

        // Fix parameters
        FixSignalParamsFromFitResult(rf, paramPrefixesToFix);

        // Fit data
        FitOpt cfg = options.ToFitConfig();
        ApplyConfigRangeToMassVar(cfg);
        fitStrategy_ = FitStrategyFactory::CreateStrategy(cfg.fitMethod);
        if (!fitStrategy_) {
            throw FitException("PerformFixedParameterFitWithMCFile", -1, "Failed to create fit strategy for FixedFromMC file mode");
        }
        auto fitResult = fitStrategy_->Execute(totalPdf_.get(), activeDataset_, cfg, activeMassVar_);
        if (!fitResult) {
            throw FitException("PerformFixedParameterFitWithMCFile", -1, "FixedFromMC (file) data fit returned null result");
        }

        std::string finalName = resultName.empty() ? (GenerateResultName() + std::string("_fixed_from_mcfile")) : resultName;
        StoreFitOutcome(finalName, std::move(fitResult), "FixedFromMCFile", "fixedFromMCFile_ws_");

        LogOperation("PerformFixedParameterFitWithMCFile", "Completed: " + finalName);
        return true;

    } catch (const std::exception& e) {
        HandleFitException(e, "PerformFixedParameterFitWithMCFile");
        return false;
    }
}

// File-based Gaussian constraint: use RooFitResult from an MC result file
template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformGaussianConstraintFitWithMCFile(const FitOpt& options, RooDataSet* dataset,
                                                          const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                          const std::string& mcResultFile,
                                                          const std::vector<std::string>& paramsToConstrain,
                                                          const std::string& resultName) {
    return ConstraintApplier::GaussianConstraintFromFile(*this, options, dataset,
                                                         signalParams, backgroundParams,
                                                         mcResultFile, paramsToConstrain, resultName);
}

// Background sideband prefit → weak Gaussian constraints on background parameters
template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformSidebandPrefitBackgroundConstraintFit(
    const FitOpt& options, RooDataSet* dataset,
    const SignalParams& signalParams, const BackgroundParams& backgroundParams,
    double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
    double sigmaScale, const std::string& resultName) {
    return ConstraintApplier::SidebandPrefitBackground(*this, options, dataset,
                                                       signalParams, backgroundParams,
                                                       sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                                       sigmaScale, resultName);
}


// Combined: MC-driven Gaussian constraints on signal + SB-prefit constraints on background
template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformGaussianConstraintFitWithMCAndBkgSB(
    const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
    const SignalParams& signalParams, const BackgroundParams& backgroundParams,
    const std::vector<std::string>& signalParamsToConstrain,
    double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
    double sigmaScaleSignal, double sigmaScaleBkg,
    const std::string& resultName) {
    return ConstraintApplier::GaussianConstraintWithSidebands(*this, options, dataset, mcDataset,
                                                              signalParams, backgroundParams,
                                                              signalParamsToConstrain,
                                                              sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                                              sigmaScaleSignal, sigmaScaleBkg,
                                                              resultName);
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformFixedSignalWithMCAndBkgSB(
    const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
    const SignalParams& signalParams, const BackgroundParams& backgroundParams,
    const std::vector<std::string>& paramPrefixesToFix,
    double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
    double sigmaScaleBkg, const std::string& resultName) {
    return ConstraintApplier::FixedSignalWithSidebands(*this, options, dataset, mcDataset,
                                                       signalParams, backgroundParams,
                                                       paramPrefixesToFix,
                                                       sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                                       sigmaScaleBkg,
                                                       resultName);
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::ConstraintApplier::GaussianConstraint(MassFitterV2& self,
                                                         const FitOpt& options,
                                                         RooDataSet* dataset,
                                                         RooDataSet* mcDataset,
                                                         const SignalParams& signalParams,
                                                         const BackgroundParams& backgroundParams,
                                                         const std::vector<std::string>& paramsToConstrain,
                                                         const std::string& resultName) {
    try {
        self.LogOperation("PerformGaussianConstraintFitWithMC", "Delegating to GaussianConstraintStrategy");
        if (!paramsToConstrain.empty()) {
            std::ostringstream oss;
            for (size_t i = 0; i < paramsToConstrain.size(); ++i) {
                if (i) oss << ", ";
                oss << paramsToConstrain[i];
            }
            std::cout << "[GaussConstr] Constraining parameters: [" << oss.str() << "]" << std::endl;
        } else {
            std::cout << "[GaussConstr] No explicit constraint parameters provided" << std::endl;
        }

        Validator::ValidateNotNull(dataset, "dataset");
        Validator::ValidateNotNull(mcDataset, "mcDataset");

        std::unique_ptr<RooDataSet> mcReduced;
        RooDataSet* mcForConstraints = mcDataset;
        if (!options.cutMCExpr.empty()) {
            auto normalizedCut = NormalizeCutExpression(options.cutMCExpr, options.useIntegratedMC);
            mcReduced.reset(dynamic_cast<RooDataSet*>(mcDataset->reduce(normalizedCut.c_str())));
            if (mcReduced) mcForConstraints = mcReduced.get();
        }

        MassModelBuilder::PrepareFullModel(self, options, dataset, signalParams, backgroundParams,
                                           "PerformGaussianConstraintFitWithMC");

        auto strategy = FitStrategyFactory::CreateGaussianConstraintStrategy(mcForConstraints, self.signalPdf_.get(), paramsToConstrain, 1.0);
        FitOpt cfg = options.ToFitConfig();
        self.ApplyConfigRangeToMassVar(cfg);
        auto fitResult = MassFitExecutor::Execute(self, std::move(strategy), cfg, "PerformGaussianConstraintFitWithMC");

        std::string finalResultName = resultName.empty() ? (self.GenerateResultName() + std::string("_gauss_constr")) : resultName;
        self.StoreFitOutcome(finalResultName, std::move(fitResult), "GaussianConstraintFit", "constraintWorkspace_");

        self.LogOperation("PerformGaussianConstraintFitWithMC", "Completed successfully: " + finalResultName);
        return true;

    } catch (const std::exception& e) {
        self.HandleFitException(e, "PerformGaussianConstraintFitWithMC");
        return false;
    }
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::ConstraintApplier::GaussianConstraintFromFile(MassFitterV2& self,
                                                                 const FitOpt& options,
                                                                 RooDataSet* dataset,
                                                                 const SignalParams& signalParams,
                                                                 const BackgroundParams& backgroundParams,
                                                                 const std::string& mcResultFile,
                                                                 const std::vector<std::string>& paramsToConstrain,
                                                                 const std::string& resultName) {
    try {
        self.LogOperation("PerformGaussianConstraintFitWithMCFile", "Using MC result file: " + mcResultFile);
        Validator::ValidateNotNull(dataset, "dataset");

        MassModelBuilder::PrepareFullModel(self, options, dataset, signalParams, backgroundParams,
                                           "PerformGaussianConstraintFitWithMCFile");

        FitOpt cfg = options.ToFitConfig();
        self.ApplyConfigRangeToMassVar(cfg);
        auto strategy = FitStrategyFactory::CreateGaussianConstraintFromFileStrategy(
            mcResultFile,
            paramsToConstrain,
            "fitResult",
            1.0);
        auto fitResult = MassFitExecutor::Execute(self, std::move(strategy), cfg, "PerformGaussianConstraintFitWithMCFile");

        std::string finalResultName = resultName.empty() ? (self.GenerateResultName() + std::string("_gauss_constr")) : resultName;
        self.StoreFitOutcome(finalResultName, std::move(fitResult), "GaussianConstraintFit", "constraintWorkspace_");

        self.LogOperation("PerformGaussianConstraintFitWithMCFile", "Completed successfully: " + finalResultName);
        return true;

    } catch (const std::exception& e) {
        self.HandleFitException(e, "PerformGaussianConstraintFitWithMCFile");
        return false;
    }
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::ConstraintApplier::SidebandPrefitBackground(MassFitterV2& self,
                                                               const FitOpt& options,
                                                               RooDataSet* dataset,
                                                               const SignalParams& signalParams,
                                                               const BackgroundParams& backgroundParams,
                                                               double sbLoMin, double sbLoMax,
                                                               double sbHiMin, double sbHiMax,
                                                               double sigmaScale,
                                                               const std::string& resultName) {
    try {
        self.LogOperation("SBPrefitBkg", "Starting sideband prefit-constrained fit");
        Validator::ValidateNotNull(dataset, "dataset");

        MassModelBuilder::PrepareBackgroundModel(self, options, dataset, backgroundParams, "SBPrefitBkg");
        self.SetSignalPDF(signalParams, "signal");
        self.CreateTotalPDF();
        if (!self.ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", "SBPrefitBkg");
        }

        FitOpt cfg = options.ToFitConfig();
        self.ApplyConfigRangeToMassVar(cfg);

        auto sbStrategy = FitStrategyFactory::CreateSidebandPrefitBackgroundConstraintStrategy(
            self.backgroundPdf_.get(), sbLoMin, sbLoMax, sbHiMin, sbHiMax, sigmaScale);
        auto fitResult = MassFitExecutor::Execute(self, std::move(sbStrategy), cfg, "SBPrefitBkg");

        std::string finalResultName = resultName.empty() ? (self.GenerateResultName() + std::string("_sb_constr")) : resultName;
        self.StoreFitOutcome(finalResultName, std::move(fitResult), "SidebandConstraintFit", "sbConstraintWorkspace_");
        self.LogOperation("SBPrefitBkg", "Completed successfully: " + finalResultName);
        return true;

    } catch (const std::exception& e) {
        self.HandleFitException(e, "SBPrefitBkg");
        return false;
    }
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::ConstraintApplier::GaussianConstraintWithSidebands(MassFitterV2& self,
                                                                      const FitOpt& options,
                                                                      RooDataSet* dataset,
                                                                      RooDataSet* mcDataset,
                                                                      const SignalParams& signalParams,
                                                                      const BackgroundParams& backgroundParams,
                                                                      const std::vector<std::string>& signalParamsToConstrain,
                                                                      double sbLoMin, double sbLoMax,
                                                                      double sbHiMin, double sbHiMax,
                                                                      double sigmaScaleSignal,
                                                                      double sigmaScaleBkg,
                                                                      const std::string& resultName) {
    try {
        self.LogOperation("GC+SB", "Starting combined GaussianConstraint (signal) + SB constraints (background)");
        Validator::ValidateNotNull(dataset, "dataset");
        Validator::ValidateNotNull(mcDataset, "mcDataset");

        self.SetData(dataset);
        if (!options.cutExpr.empty()) self.ApplyCut(options.cutExpr);

        self.SetSignalPDF(signalParams, "signal");
        self.SetBackgroundPDF(backgroundParams, "background");
        self.CreateTotalPDF();
        if (!self.ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", "GC+SB");
        }

        FitOpt cfg = options.ToFitConfig();
        self.ApplyConfigRangeToMassVar(cfg);

        RooDataSet* mcForConstraints = mcDataset;
        std::unique_ptr<RooDataSet> mcReduced;
        const double mcEntriesBefore = mcDataset ? mcDataset->numEntries() : 0.0;
        const double mcWeightsBefore = mcDataset ? mcDataset->sumEntries() : 0.0;

        if (!options.cutMCExpr.empty()) {
            auto mcCutExpr = NormalizeCutExpression(options.cutMCExpr, options.useIntegratedMC);
            std::cout << "[GC+SB] Reducing MC dataset with cut: " << mcCutExpr
                      << " (entries=" << mcEntriesBefore << ", weighted=" << mcWeightsBefore << ")"
                      << std::endl;
            mcReduced.reset(dynamic_cast<RooDataSet*>(mcDataset->reduce(mcCutExpr.c_str())));
            if (mcReduced) {
                mcForConstraints = mcReduced.get();
                std::cout << "[GC+SB] MC entries after reduction: "
                          << mcForConstraints->numEntries()
                          << " (weighted=" << mcForConstraints->sumEntries() << ")"
                          << std::endl;
            } else {
                std::cout << "[GC+SB] MC reduction failed, falling back to full dataset"
                          << std::endl;
            }
        }

        auto gcsbStrategy = FitStrategyFactory::CreateGaussianConstraintWithSBStrategy(
            mcForConstraints,
            self.signalPdf_.get(),
            self.backgroundPdf_.get(),
            signalParamsToConstrain,
            sbLoMin, sbLoMax,
            sbHiMin, sbHiMax,
            sigmaScaleSignal,
            sigmaScaleBkg);
        auto fitResult = MassFitExecutor::Execute(self, std::move(gcsbStrategy), cfg, "GC+SB");

        std::string finalResultName = resultName.empty() ? (self.GenerateResultName() + std::string("_gc_sb")) : resultName;
        self.StoreFitOutcome(finalResultName, std::move(fitResult), "GaussianConstraint+Sideband", "constraintWorkspace_");

        self.LogOperation("GC+SB", "Completed successfully: " + finalResultName);
        return true;

    } catch (const std::exception& e) {
        self.HandleFitException(e, "GC+SB");
        return false;
    }
}

template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::ConstraintApplier::FixedSignalWithSidebands(MassFitterV2& self,
                                                               const FitOpt& options,
                                                               RooDataSet* dataset,
                                                               RooDataSet* mcDataset,
                                                               const SignalParams& signalParams,
                                                               const BackgroundParams& backgroundParams,
                                                               const std::vector<std::string>& prefixesToFix,
                                                               double sbLoMin, double sbLoMax,
                                                               double sbHiMin, double sbHiMax,
                                                               double sigmaScaleBkg,
                                                               const std::string& resultName) {
    try {
        self.LogOperation("FixedSignal+SB", "Starting fixed-signal with sideband-constrained background fit");
        Validator::ValidateNotNull(dataset, "dataset");
        Validator::ValidateNotNull(mcDataset, "mcDataset");

        double effectiveSigma = (sigmaScaleBkg > 0.0) ? sigmaScaleBkg : 2.0;

        if (!prefixesToFix.empty()) {
            std::ostringstream oss;
            for (size_t i = 0; i < prefixesToFix.size(); ++i) {
                if (i) oss << ", ";
                oss << prefixesToFix[i];
            }
            std::cout << "[FixedSignal+SB] Prefix list: [" << oss.str() << "]" << std::endl;
        } else {
            std::cout << "[FixedSignal+SB] No prefixes requested; signal parameters remain floating." << std::endl;
        }

        std::string mcResultName = resultName.empty()
            ? (self.GenerateResultName() + std::string("_fixsig_sb_mc"))
            : (resultName + std::string("_fixsig_sb_mc"));

        bool mcSuccess = self.PerformMCFit(options, mcDataset, signalParams, mcResultName);
        std::cout << "[FixedSignal+SB] MC fit result name: " << mcResultName
                  << ", success=" << (mcSuccess ? "true" : "false") << std::endl;
        if (!mcSuccess) {
            LOG_AND_THROW(MassFitterException, "MC fit failed for FixedSignalWithSideband", "FixedSignal+SB");
        }

        RooFitResult* mcRf = self.GetRooFitResult(mcResultName);
        if (!mcRf) {
            LOG_AND_THROW(MassFitterException, "MC RooFitResult missing for FixedSignalWithSideband", "FixedSignal+SB");
        }

        MassModelBuilder::PrepareFullModel(self, options, dataset, signalParams, backgroundParams, "FixedSignal+SB");
        self.FixSignalParamsFromFitResult(mcRf, prefixesToFix);

        self.CreateTotalPDF();
        if (!self.ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", "FixedSignal+SB");
        }

        FitOpt cfg = options.ToFitConfig();
        self.ApplyConfigRangeToMassVar(cfg);

        auto sbStrategy = FitStrategyFactory::CreateSidebandPrefitBackgroundConstraintStrategy(
            self.backgroundPdf_.get(), sbLoMin, sbLoMax, sbHiMin, sbHiMax, effectiveSigma);
        auto fitResult = MassFitExecutor::Execute(self, std::move(sbStrategy), cfg, "FixedSignal+SB");

        std::string finalResultName = resultName.empty()
            ? (self.GenerateResultName() + std::string("_fixed_signal_sb"))
            : resultName;

        self.StoreFitOutcome(finalResultName, std::move(fitResult), "FixedSignalWithSideband", "fixedSignalSB_ws_");
        self.LogOperation("FixedSignal+SB", "Completed successfully: " + finalResultName);
        return true;
    } catch (const std::exception& e) {
        self.HandleFitException(e, "FixedSignal+SB");
        return false;
    }
}

template<typename SignalParams, typename BackgroundParams>
void MassFitterV2::MassModelBuilder::PrepareFullModel(MassFitterV2& self,
                                                      const FitOpt& options,
                                                      RooDataSet* dataset,
                                                      const SignalParams& signalParams,
                                                      const BackgroundParams& backgroundParams,
                                                      const char* contextTag) {
    Validator::ValidateNotNull(dataset, "dataset");
    self.SetData(dataset);
    if (!options.cutExpr.empty()) {
        self.ApplyCut(options.cutExpr);
    }

    self.SetSignalPDF(signalParams, "signal");
    self.SetBackgroundPDF(backgroundParams, "background");
    self.CreateTotalPDF();
    if (!self.ValidatePDFSetup()) {
        LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", contextTag);
    }
}

template<typename BackgroundParams>
void MassFitterV2::MassModelBuilder::PrepareBackgroundModel(MassFitterV2& self,
                                                           const FitOpt& options,
                                                           RooDataSet* dataset,
                                                           const BackgroundParams& backgroundParams,
                                                           const char* contextTag) {
    Validator::ValidateNotNull(dataset, "dataset");
    self.SetData(dataset);
    if (!options.cutExpr.empty()) {
        self.ApplyCut(options.cutExpr);
    }

    self.SetBackgroundPDF(backgroundParams, "background");
    if (!self.backgroundPdf_) {
        LOG_AND_THROW(PDFCreationException, "Background PDF not configured", contextTag);
    }
}

inline std::unique_ptr<RooFitResult> MassFitterV2::MassFitExecutor::Execute(MassFitterV2& self,
                                                                           std::unique_ptr<FitStrategy> strategy,
                                                                           FitOpt cfg,
                                                                           const char* contextTag) {
    self.fitStrategy_ = std::move(strategy);
    if (!self.fitStrategy_) {
        LOG_AND_THROW(MassFitterException, "Failed to create FitStrategy", contextTag);
    }
    self.LogOperation("FitStrategy", "Selected " + self.fitStrategy_->GetName() + " (" + std::string(contextTag) + ")");
    auto fitResult = self.fitStrategy_->Execute(self.totalPdf_.get(), self.activeDataset_, cfg, self.activeMassVar_);
    if (!fitResult) {
        LOG_AND_THROW(MassFitterException, "Fit returned null result", contextTag);
    }
    return fitResult;
}


inline void MassFitterV2::CreateTotalPDF() {
    if (!(signalPdf_ && backgroundPdf_)) return;

    // If using Fraction mode and coefficient model is requested, build RooAddPdf with fsig as coefficient
    if (yieldMode_ == YieldMode::Fraction && useCoeffFractionModel_ && fsig_) {
        totalPdf_ = std::make_unique<RooAddPdf>("totalPdf", "Total PDF",
                                                RooArgList(*signalPdf_, *backgroundPdf_),
                                                RooArgList(*fsig_));
        std::string mode = "Fraction-Coeff";
        LogOperation("CreateTotalPDF", "YieldMode=" + mode + ", coeffVar=fsig");
        return;
    }

    // Fraction mode: use derived expressions (nsig, nbkg) via RooAbsReal
    if (yieldMode_ == YieldMode::Fraction) {
        if (nsig_ && nbkg_) {
            totalPdf_ = std::make_unique<RooAddPdf>("totalPdf", "Total PDF",
                                                    RooArgList(*signalPdf_, *backgroundPdf_),
                                                    RooArgList(*nsig_, *nbkg_));
            const char* nsType = dynamic_cast<RooFormulaVar*>(nsig_.get()) ? "RooFormulaVar"
                                     : (dynamic_cast<RooRealVar*>(nsig_.get()) ? "RooRealVar" : "RooAbsReal");
            const char* nbType = dynamic_cast<RooFormulaVar*>(nbkg_.get()) ? "RooFormulaVar"
                                     : (dynamic_cast<RooRealVar*>(nbkg_.get()) ? "RooRealVar" : "RooAbsReal");
            LogOperation("CreateTotalPDF", std::string("YieldMode=Fraction, nsigType=") + nsType + ", nbkgType=" + nbType);
        }
        return;
    }

    // Independent mode: use direct RooRealVar yields (nsigVar_, nbkgVar_)
    if (yieldMode_ == YieldMode::Independent) {
        if (nsigVar_ && nbkgVar_) {
            totalPdf_ = std::make_unique<RooAddPdf>("totalPdf", "Total PDF",
                                                    RooArgList(*signalPdf_, *backgroundPdf_),
                                                    RooArgList(*nsigVar_, *nbkgVar_));
            LogOperation("CreateTotalPDF", "YieldMode=Independent, nsigType=RooRealVar, nbkgType=RooRealVar");
        }
        return;
    }
}
inline bool MassFitterV2::ValidateInputs(RooDataSet* dataset) const {
    return dataset && dataset->numEntries() > 0;
}

inline bool MassFitterV2::ValidatePDFSetup() const {
    return totalPdf_ != nullptr;
}

inline std::string MassFitterV2::GenerateResultName() const {
    return name_ + "_result_" + std::to_string(std::time(nullptr));
}

inline void MassFitterV2::LogOperation(const std::string& operation, const std::string& details) const {
    std::string message = details.empty() ? operation : (operation + ": " + details);
    ErrorHandlerManager::Instance().LogInfo(message, std::string("MassFitterV2:") + name_);
}

inline void MassFitterV2::HandleFitException(const std::exception& e, const std::string& context) {
    ErrorHandlerManager::Instance().HandleException(e, context.empty() ? std::string("MassFitterV2:") + name_ : context);
}

// Placeholder implementations for methods called by DStarAnalysisV2
inline double MassFitterV2::GetSignalYield(const std::string& resultName) const {
    if (nsig_) return nsig_->getVal();
    return 0.0;
}

inline double MassFitterV2::GetSignalYieldError(const std::string& resultName) const {
    if (!nsig_) return 0.0;
    // Independent mode: nsig is a RooRealVar, return its fit error directly
    if (yieldMode_ == YieldMode::Independent) {
        if (auto* v = dynamic_cast<RooRealVar*>(nsig_.get())) {
            return v->getError();
        }
        return 0.0;
    }
    // Fraction mode: nsig is a formula; use propagated error from fit result when available
    if (resultManager_) {
        const FitResults* res = resultManager_->GetResult(resultName);
        if (res && res->fitResult) {
            return nsig_->getPropagatedError(*res->fitResult);
        }
    }
    return 0.0;
}

inline double MassFitterV2::GetBackgroundYield(const std::string& resultName) const {
    if (nbkg_) return nbkg_->getVal();
    return 0.0;
}

inline double MassFitterV2::GetBackgroundYieldError(const std::string& resultName) const {
    if (!nbkg_) return 0.0;
    if (yieldMode_ == YieldMode::Independent) {
        if (auto* v = dynamic_cast<RooRealVar*>(nbkg_.get())) {
            return v->getError();
        }
        return 0.0;
    }
    if (resultManager_) {
        const FitResults* res = resultManager_->GetResult(resultName);
        if (res && res->fitResult) {
            return nbkg_->getPropagatedError(*res->fitResult);
        }
    }
    return 0.0;
}

inline double MassFitterV2::GetTotalYield(const std::string& resultName) const {
    // In Fraction mode prefer the explicit Ntot variable when available.
    if (yieldMode_ == YieldMode::Fraction) {
        if (ntot_) return ntot_->getVal();
    }
    // Fallback: sum signal and background components
    return GetSignalYield(resultName) + GetBackgroundYield(resultName);
}

inline double MassFitterV2::GetReducedChiSquare(const std::string& resultName) const {
    return 1.0; // Placeholder
}

inline bool MassFitterV2::IsGoodFit(const std::string& resultName) const {
    return true; // Placeholder - always return true for now
}

// --- Accessors to fetch stored results from ResultManager ---
inline FitResults* MassFitterV2::GetFitResults(const std::string& resultName) {
    if (!resultManager_) return nullptr;
    return resultManager_->GetResult(resultName);
}

inline RooFitResult* MassFitterV2::GetRooFitResult(const std::string& resultName) {
    if (!resultManager_) return nullptr;
    const FitResults* res = resultManager_->GetResult(resultName);
    return res ? res->fitResult.get() : nullptr;
}

inline RooWorkspace* MassFitterV2::GetWorkspace(const std::string& resultName) {
    if (!resultManager_) return nullptr;
    return resultManager_->GetWorkspace(resultName);
}

inline double MassFitterV2::CalculateSignificance(const std::string& resultName) const {
    double sig = GetSignalYield(resultName);
    double bkg = GetBackgroundYield(resultName);
    if (bkg > 0) {
        return sig / sqrt(sig + bkg);
    }
    return 0.0;
}

inline double MassFitterV2::CalculatePurity(const std::string& resultName) const {
    double sig = GetSignalYield(resultName);
    double total = sig + GetBackgroundYield(resultName);
    return total > 0 ? sig / total : 0.0;
}

inline void MassFitterV2::PrintSummary(const std::string& resultName) const {
    std::cout << "=== Fit Results Summary ===" << std::endl;
    std::cout << "Signal Yield: " << GetSignalYield(resultName) << " ± " << GetSignalYieldError(resultName) << std::endl;
    std::cout << "Background Yield: " << GetBackgroundYield(resultName) << std::endl;
}

inline std::unique_ptr<TCanvas> MassFitterV2::CreateCanvas(const std::string& resultName, const PlotOptions& options) {
    auto canvas = std::make_unique<TCanvas>(("canvas_" + resultName).c_str(), options.title.c_str(), 800, 600);
    return canvas;
}

inline void MassFitterV2::SaveResult(const std::string& resultName, const std::string& filePath, 
                                     const std::string& fileName, bool saveWorkspace) {
    if (!resultManager_) {
        std::cout << "[MassFitterV2] ResultManager not configured; cannot save results." << std::endl;
        return;
    }
    
    try {
        // Ensure the output directory exists (create recursively if needed)
        if (!ensureDir(filePath)) {
            std::cerr << "[MassFitterV2] Cannot create output directory: " << filePath << std::endl;
            return;
        }
        // Use ResultManager to save the result
        std::string fullPath = filePath + "/" + fileName;
        std::cout << "Saving results to: " << fullPath << std::endl;
        resultManager_->SaveResult(resultName, fullPath, saveWorkspace);
        std::cout << "[MassFitterV2] Results saved successfully: " << fullPath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[MassFitterV2] Failed to save results: " << e.what() << std::endl;
    }
}

inline void MassFitterV2::SaveResults(const std::string& filePath, const std::string& fileName, bool saveWorkspaces) {
    if (!resultManager_) {
        std::cout << "[MassFitterV2] ResultManager not configured; cannot save results." << std::endl;
        return;
    }
    
    try {
        // Ensure the output directory exists (create recursively if needed)
        if (!ensureDir(filePath)) {
            std::cerr << "[MassFitterV2] Cannot create output directory: " << filePath << std::endl;
            return;
        }
        // Save all stored results
        auto resultNames = GetResultNames();
        if (resultNames.empty()) {
            std::cout << "[MassFitterV2] No results to save." << std::endl;
            return;
        }
        
        std::string fullPath = filePath + "/" + fileName;
        std::cout << "Saving all results to: " << fullPath << std::endl;
        for (const auto& resultName : resultNames) {
            resultManager_->SaveResult(resultName, fullPath, saveWorkspaces);
        }
        std::cout << "[MassFitterV2] All results (" << resultNames.size() << ") saved successfully: " << fullPath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[MassFitterV2] Failed to save results: " << e.what() << std::endl;
    }
}

inline void MassFitterV2::LoadResults(const std::string& filePath, const std::string& fileName) {
    std::string full = filePath + "/" + fileName;
    std::cout << "Loading results from: " << full << std::endl;
    if (!resultManager_) {
        std::cout << "[MassFitterV2] ResultManager not configured; skipping load." << std::endl;
        return;
    }
    try {
        resultManager_->LoadResults(full);
    } catch (const std::exception& e) {
        std::cerr << "[MassFitterV2] LoadResults exception: " << e.what() << std::endl;
        throw;
    }
}

inline void MassFitterV2::ExportResults(const std::string& format, const std::string& fileName) {
    std::cout << "Exporting results in " << format << " format to: " << fileName << std::endl;
    // Placeholder implementation
}

inline void MassFitterV2::RunDiagnostics(const std::string& outputFile) const {
    std::cout << "Running diagnostics, output to: " << outputFile << std::endl;
    // Placeholder implementation
}

inline void MassFitterV2::StoreFitOutcome(const std::string& resultName,
                                          std::unique_ptr<RooFitResult> fitResult,
                                          const std::string& label,
                                          const std::string& workspacePrefix) {
    if (!resultManager_) return;
    auto workspace = std::make_unique<RooWorkspace>((workspacePrefix + name_).c_str());
    if (totalPdf_) workspace->import(*totalPdf_);
    if (activeDataset_) workspace->import(*activeDataset_);
    if (signalPdf_) workspace->import(*signalPdf_);
    if (backgroundPdf_) workspace->import(*backgroundPdf_);

    resultManager_->StoreResult(resultName, std::move(fitResult), std::move(workspace), label);

    std::map<std::string, RooAbsReal*> yieldExprs;
    if (nsig_) yieldExprs["nsig"] = nsig_.get();
    if (nbkg_) yieldExprs["nbkg"] = nbkg_.get();
    if (!yieldExprs.empty()) {
        resultManager_->StoreYieldsFromAbsReal(resultName, yieldExprs);
    }
    // Choose appropriate PDF for chi2: for MC fits, use the signal PDF; otherwise use combined total PDF.
    RooAbsPdf* chi2Pdf = nullptr;
    if (label == "MCFit") {
        chi2Pdf = signalPdf_.get();
    } else if (totalPdf_) {
        chi2Pdf = totalPdf_.get();
    }
    if (!chi2Pdf && signalPdf_) {
        chi2Pdf = signalPdf_.get();
    }
    if (chi2Pdf && activeDataset_ && activeMassVar_) {
        resultManager_->CalculateChiSquare(resultName, chi2Pdf, activeDataset_, activeMassVar_);
        cout << "[MassFitterV2] Calculated chi-square for result: " << resultName << std::endl;
    }
}

// inline void MassFitterV2::UseDeltaMass(bool use, double daughterMassMin, double daughterMassMax) {
//     useDeltaMass_ = use;
//     if (use && !massDaughter1_) {
//         massDaughter1_ = std::make_unique<RooRealVar>("massDaughter1", "Daughter1 Mass", daughterMassMin, daughterMassMax);
//         massPion_ = std::make_unique<RooRealVar>("massPion", "Pion Mass", PION_MASS_V2);
//         massPion_->setConstant(true);
//     }
// }

#endif // MASS_FITTER_V2_H
