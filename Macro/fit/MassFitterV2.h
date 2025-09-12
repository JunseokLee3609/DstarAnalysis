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
#include "PDFFactory.h"
#include "FitStrategy.h"
#include "ResultManager.h"
#include "ErrorHandler.h"
#include "TestFramework.h"
#include "Params.h"
#include "Opt.h"
#include "Helper.h"  // for createDir/ensureOutputDir

// Constants
constexpr double PION_MASS_V2 = 0.13957039;
constexpr double DELTAMASS_V2 = 0.1454257;

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
    void SetConfiguration(const FitConfig& config) { fitConfig_ = config; }
    const FitConfig& GetConfiguration() const { return fitConfig_; }
    
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
    bool FitWithConfig(const FitConfig& config, const std::string& resultName = "configured");
    
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
                                                      double sigmaScale = 2.0,
                                                      const std::string& resultName = "");

    // Combined: MC-driven Gaussian constraints on signal + SB-prefit constraints on background
    template<typename SignalParams, typename BackgroundParams>
    bool PerformGaussianConstraintFitWithMCAndBkgSB(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                                    const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                    const std::vector<std::string>& signalParamsToConstrain,
                                                    double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
                                                    double sigmaScaleSignal = 5.0, double sigmaScaleBkg = 2.0,
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
        if (strategy) {
            std::cout << "[MassFitterV2] Setting fit strategy to: " << strategy->GetName() << std::endl;
            LogOperation("SetFitStrategy", "Changed to " + strategy->GetName());
        }
        fitStrategy_ = std::move(strategy);
    }
    void SetFitStrategy(const std::string& strategyName);
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
    
    // Statistical utilities
    double CalculateSignificance(const std::string& resultName = "default") const;
    double CalculatePurity(const std::string& resultName = "default") const;
    std::pair<double, double> GetSignalToBackgroundRatio(const std::string& resultName = "default") const;

    // Helper to copy MC-fit parameter values to data signal parameters and fix them
    void FixSignalParamsFromFitResult(const RooFitResult* rf, const std::vector<std::string>& prefixesToFix);
    
private:
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
    FitConfig fitConfig_;
    
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
        FitConfig config = options.ToFitConfig();
        // Ensure mass variable uses the same named range and bounds as the config
        if (activeMassVar_) {
            if (!config.rangeName.empty()) {
                activeMassVar_->setRange(config.rangeName.c_str(), config.rangeMin, config.rangeMax);
            }
            activeMassVar_->setMin(config.rangeMin);
            activeMassVar_->setMax(config.rangeMax);
        }
        
        // Select appropriate strategy based on FitMethod
        FitStrategyFactory::StrategyType strategyType = FitStrategyFactory::StrategyType::Basic; // default
        
        switch (config.fitMethod) {
            case FitMethod::NLL:
                strategyType = FitStrategyFactory::StrategyType::Basic;
                break;
            case FitMethod::BinnedNLL:
                strategyType = FitStrategyFactory::StrategyType::Binned;
                break;
            case FitMethod::Extended:
                strategyType = FitStrategyFactory::StrategyType::Basic;   // Extended-only (no robust expansion)
                break;
            case FitMethod::Robust:
                strategyType = FitStrategyFactory::StrategyType::Robust;  // Robust iterative expansion
                break;
            case FitMethod::GaussianConstraint:
                strategyType = FitStrategyFactory::StrategyType::Basic;   // handled at higher level
                break;
            case FitMethod::FixedFromMC:
                strategyType = FitStrategyFactory::StrategyType::Basic;   // handled at higher level
                break;
        }
        
        // Override or set strategy based on fit method
        fitStrategy_ = FitStrategyFactory::CreateStrategy(strategyType);
        
        LogOperation("FitStrategy", "Auto-selected " + fitStrategy_->GetName() + " for " + 
                    (config.fitMethod == FitMethod::NLL ? "NLL" : 
                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : 
                     config.fitMethod == FitMethod::Extended ? "Extended" : "Robust") + " method");
        std::cout << "[MassFitterV2] Using fit strategy: " << fitStrategy_->GetName() 
                  << " (from " << (config.fitMethod == FitMethod::NLL ? "NLL" : 
                                  config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : 
                                  config.fitMethod == FitMethod::Extended ? "Extended" : "Robust") << " method)" << std::endl;
        
        auto fitResult = fitStrategy_->Execute(totalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("Fit execution returned null result", "PerformFit");
            throw FitException("PerformFit", -1, "Fit execution returned null result");
        }
        fitResult->Print("v");
        
        // Create and populate workspace
        auto workspace = std::make_unique<RooWorkspace>(("workspace_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        if (signalPdf_) workspace->import(*signalPdf_);
        if (backgroundPdf_) workspace->import(*backgroundPdf_);
        
        // Store results
        std::string finalResultName = resultName.empty() ? GenerateResultName() : resultName;
        resultManager_->StoreResult(finalResultName, std::move(fitResult), 
                                   std::move(workspace), "StandardFit");
        
        // Store yields (supports RooFormulaVar via propagated errors)
        std::map<std::string, RooAbsReal*> yieldExprs;
        if (nsig_) yieldExprs["nsig"] = nsig_.get();
        if (nbkg_) yieldExprs["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalResultName, yieldExprs);
        
        // Calculate quality metrics
        resultManager_->CalculateChiSquare(finalResultName, totalPdf_.get(), activeDataset_, activeMassVar_);
        
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
        
        // Set data and apply MC cuts
        SetData(mcDataset);
        if (!options.cutMCExpr.empty()) {
            LogOperation("ApplyCut", "Applying MC cut: " + options.cutMCExpr);
            ApplyCut(options.cutMCExpr);
        }
        
        // Create signal PDF only for MC fit
        SetSignalPDF(signalParams, "mcSignal");
        
        if (!signalPdf_) {
            LOG_AND_THROW(PDFCreationException, "MC signal PDF creation failed", "PerformMCFit");
        }
        
        // Create MC fit configuration (force unbinned NLL for MC)
        FitConfig config = options.ToFitConfig();
        config.fitMethod = FitMethod::NLL;   // Always use NLL for MC fits
        config.useMinos = false;             // MC fits typically don't need Minos
        // Align mass variable range with configuration
        if (activeMassVar_) {
            if (!config.rangeName.empty()) {
                activeMassVar_->setRange(config.rangeName.c_str(), config.rangeMin, config.rangeMax);
            }
            activeMassVar_->setMin(config.rangeMin);
            activeMassVar_->setMax(config.rangeMax);
        }
        
        // Select appropriate strategy based on FitMethod for MC
        FitStrategyFactory::StrategyType mcStrategyType = FitStrategyFactory::StrategyType::MC; // default for MC
        
        switch (config.fitMethod) {
            case FitMethod::NLL:
                mcStrategyType = FitStrategyFactory::StrategyType::Basic;
                break;
            case FitMethod::BinnedNLL:
                mcStrategyType = FitStrategyFactory::StrategyType::Binned;
                break;
            case FitMethod::Extended:
                mcStrategyType = FitStrategyFactory::StrategyType::MC;     // MC strategy for extended
                break;
            case FitMethod::Robust:
                mcStrategyType = FitStrategyFactory::StrategyType::MC;     // Keep MC path same for robust
                break;
            case FitMethod::GaussianConstraint:
                mcStrategyType = FitStrategyFactory::StrategyType::MC;
                break;
            case FitMethod::FixedFromMC:
                mcStrategyType = FitStrategyFactory::StrategyType::MC;
                break;
        }
        
        auto mcStrategy = FitStrategyFactory::CreateStrategy(mcStrategyType);
        
        LogOperation("FitStrategy", "Auto-selected " + mcStrategy->GetName() + " for MC " + 
                    (config.fitMethod == FitMethod::NLL ? "NLL" : 
                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : 
                     config.fitMethod == FitMethod::Extended ? "Extended" : "Robust") + " method");
        std::cout << "[MassFitterV2] Using MC fit strategy: " << mcStrategy->GetName() 
                  << " (from " << (config.fitMethod == FitMethod::NLL ? "NLL" : 
                                  config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : 
                                  config.fitMethod == FitMethod::Extended ? "Extended" : "Robust") << " method)" << std::endl;
        
        // Perform MC fit
        auto fitResult = mcStrategy->Execute(signalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("MC fit execution returned null result", "PerformMCFit");
            throw FitException("PerformMCFit", -1, "MC fit execution returned null result");
        }
        fitResult->Print("v");
        
        // Create MC workspace
        auto workspace = std::make_unique<RooWorkspace>(("mcWorkspace_" + name_).c_str());
        workspace->import(*signalPdf_);
        workspace->import(*activeDataset_);
        
        // Store MC results
        std::string finalResultName = resultName.empty() ? GenerateResultName() + "_MC" : resultName;
        resultManager_->StoreResult(finalResultName, std::move(fitResult),
                                   std::move(workspace), "MCFit");
        
        // Store signal yield for MC (supports both formula and real var)
        std::map<std::string, RooAbsReal*> yieldExprs;
        if (nsig_) yieldExprs["nsig"] = nsig_.get();
        resultManager_->StoreYieldsFromAbsReal(finalResultName, yieldExprs);
        
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
        {
            FitConfig baseCfg = options.ToFitConfig();
            if (activeMassVar_) {
                if (!baseCfg.rangeName.empty()) {
                    activeMassVar_->setRange(baseCfg.rangeName.c_str(), baseCfg.rangeMin, baseCfg.rangeMax);
                }
                activeMassVar_->setMin(baseCfg.rangeMin);
                activeMassVar_->setMax(baseCfg.rangeMax);
            }
        }

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
        FitConfig config = options.ToFitConfig();
        
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
        
        // Store constraint fit results
        std::string finalResultName = resultName.empty() ? GenerateResultName() + "_constraint" : resultName;
        resultManager_->StoreResult(finalResultName, std::move(fitResult),
                                   std::move(workspace), "ConstraintFit");
        
        // Store yields with error propagation
        std::map<std::string, RooAbsReal*> yieldExprs;
        if (nsig_) yieldExprs["nsig"] = nsig_.get();
        if (nbkg_) yieldExprs["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalResultName, yieldExprs);
        
        // Calculate quality metrics
        resultManager_->CalculateChiSquare(finalResultName, totalPdf_.get(), activeDataset_, activeMassVar_);
        
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
    int dataSize = dataset->sumEntries();
    std::cout << "total Entries : " << dataSize << std::endl;
    if (yieldMode_ == YieldMode::Fraction) {
        // Update total yield based on dataset size
        if (ntot_) {
            ntot_->setVal(dataSize);
            ntot_->setRange(std::max(0.0, dataSize * 0.2), std::max(1.0, dataSize * 5.0));
            ntot_->setConstant(true);
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
        activeDataset_ = (RooDataSet*)fullDataset_->reduce(cutExpr.c_str());
        
        if (!activeDataset_) return;
        int cutDataSize = activeDataset_->sumEntries();
        std::cout << "Cut applied. New data size: " << cutDataSize << std::endl;
        if (yieldMode_ == YieldMode::Fraction) {
            // Update Ntot based on cut dataset size
            if (ntot_) {
                ntot_->setVal(cutDataSize);
                ntot_->setRange(std::max(0.0, cutDataSize * 0.2), std::max(1.0, cutDataSize * 5.0));
                ntot_->setConstant(true);
                double fval = fsig_ ? fsig_->getVal() : -1.0;
                LogOperation("ApplyCut", "Updated Ntot to " + std::to_string(cutDataSize) +
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
    try {
        LogOperation("PerformGaussianConstraintFitWithMC", "Delegating to GaussianConstraintStrategy");
        if (!paramsToConstrain.empty()) {
            std::ostringstream oss;
            for (size_t i=0;i<paramsToConstrain.size();++i) { if (i) oss << ", "; oss << paramsToConstrain[i]; }
            std::cout << "[GaussConstr] Constraining parameters: [" << oss.str() << "]" << std::endl;
        } else {
            std::cout << "[GaussConstr] No explicit constraint parameters provided" << std::endl;
        }

        // Validate inputs
        Validator::ValidateNotNull(dataset, "dataset");
        Validator::ValidateNotNull(mcDataset, "mcDataset");

        // Prepare reduced MC dataset if cut is requested
        std::unique_ptr<RooDataSet> mcReduced;
        RooDataSet* mcForConstraints = mcDataset;
        if (!options.cutMCExpr.empty()) {
            mcReduced.reset(dynamic_cast<RooDataSet*>(mcDataset->reduce(options.cutMCExpr.c_str())));
            if (mcReduced) mcForConstraints = mcReduced.get();
        }

        // Set data and cuts
        SetData(dataset);
        if (!options.cutExpr.empty()) {
            ApplyCut(options.cutExpr);
        }

        // Build PDFs and model
        SetSignalPDF(signalParams, "signal");
        SetBackgroundPDF(backgroundParams, "background");
        CreateTotalPDF();
        if (!ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", "PerformGaussianConstraintFitWithMC");
        }

        // Create strategy and execute constrained fit
        auto strategy = FitStrategyFactory::CreateGaussianConstraintStrategy(mcForConstraints, signalPdf_.get(), paramsToConstrain, 5.0);
        FitConfig cfg = options.ToFitConfig();
        if (activeMassVar_) {
            if (!cfg.rangeName.empty()) activeMassVar_->setRange(cfg.rangeName.c_str(), cfg.rangeMin, cfg.rangeMax);
            activeMassVar_->setMin(cfg.rangeMin); activeMassVar_->setMax(cfg.rangeMax);
        }
        auto fitResult = strategy->Execute(totalPdf_.get(), activeDataset_, cfg, activeMassVar_);
        if (!fitResult) {
            LOG_AND_THROW(MassFitterException, "Gaussian constrained fit returned null result", "PerformGaussianConstraintFitWithMC");
        }

        // Store results
        auto workspace = std::make_unique<RooWorkspace>(("constraintWorkspace_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        if (signalPdf_) workspace->import(*signalPdf_);
        if (backgroundPdf_) workspace->import(*backgroundPdf_);

        std::string finalResultName = resultName.empty() ? (GenerateResultName() + std::string("_gauss_constr")) : resultName;
        resultManager_->StoreResult(finalResultName, std::move(fitResult), std::move(workspace), "GaussianConstraintFit");
        std::map<std::string, RooAbsReal*> yieldExprs;
        if (nsig_) yieldExprs["nsig"] = nsig_.get();
        if (nbkg_) yieldExprs["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalResultName, yieldExprs);
        resultManager_->CalculateChiSquare(finalResultName, totalPdf_.get(), activeDataset_, activeMassVar_);

        LogOperation("PerformGaussianConstraintFitWithMC", "Completed successfully: " + finalResultName);
        return true;

    } catch (const std::exception& e) {
        HandleFitException(e, "PerformGaussianConstraintFitWithMC");
        return false;
    }
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
        FitConfig cfg = options.ToFitConfig();
        if (activeMassVar_) {
            if (!cfg.rangeName.empty()) activeMassVar_->setRange(cfg.rangeName.c_str(), cfg.rangeMin, cfg.rangeMax);
            activeMassVar_->setMin(cfg.rangeMin); activeMassVar_->setMax(cfg.rangeMax);
        }

        // Choose strategy based on method (FixedFromMC maps to Basic)
        auto stType = FitStrategyFactory::GetStrategyTypeFromFitMethod(cfg.fitMethod);
        fitStrategy_ = FitStrategyFactory::CreateStrategy(stType);
        auto fitResult = fitStrategy_->Execute(totalPdf_.get(), activeDataset_, cfg, activeMassVar_);
        if (!fitResult) {
            throw FitException("PerformFixedParameterFitWithMC", -1, "FixedFromMC data fit returned null result");
        }

        auto workspace = std::make_unique<RooWorkspace>(("fixedFromMC_ws_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        if (signalPdf_) workspace->import(*signalPdf_);
        if (backgroundPdf_) workspace->import(*backgroundPdf_);

        std::string finalName = resultName.empty() ? (GenerateResultName() + std::string("_fixed_from_mc")) : resultName;
        resultManager_->StoreResult(finalName, std::move(fitResult), std::move(workspace), "FixedFromMC");
        std::map<std::string, RooAbsReal*> yexprs;
        if (nsig_) yexprs["nsig"] = nsig_.get();
        if (nbkg_) yexprs["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalName, yexprs);
        resultManager_->CalculateChiSquare(finalName, totalPdf_.get(), activeDataset_, activeMassVar_);

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
        FitConfig cfg = options.ToFitConfig();
        if (activeMassVar_) {
            if (!cfg.rangeName.empty()) activeMassVar_->setRange(cfg.rangeName.c_str(), cfg.rangeMin, cfg.rangeMax);
            activeMassVar_->setMin(cfg.rangeMin); activeMassVar_->setMax(cfg.rangeMax);
        }
        auto stType = FitStrategyFactory::GetStrategyTypeFromFitMethod(cfg.fitMethod);
        fitStrategy_ = FitStrategyFactory::CreateStrategy(stType);
        auto fitResult = fitStrategy_->Execute(totalPdf_.get(), activeDataset_, cfg, activeMassVar_);
        if (!fitResult) {
            throw FitException("PerformFixedParameterFitWithMCFile", -1, "FixedFromMC (file) data fit returned null result");
        }

        auto workspace = std::make_unique<RooWorkspace>(("fixedFromMCFile_ws_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        if (signalPdf_) workspace->import(*signalPdf_);
        if (backgroundPdf_) workspace->import(*backgroundPdf_);

        std::string finalName = resultName.empty() ? (GenerateResultName() + std::string("_fixed_from_mcfile")) : resultName;
        resultManager_->StoreResult(finalName, std::move(fitResult), std::move(workspace), "FixedFromMCFile");
        std::map<std::string, RooAbsReal*> yexprs;
        if (nsig_) yexprs["nsig"] = nsig_.get();
        if (nbkg_) yexprs["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalName, yexprs);
        resultManager_->CalculateChiSquare(finalName, totalPdf_.get(), activeDataset_, activeMassVar_);

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
    try {
        LogOperation("PerformGaussianConstraintFitWithMCFile", "Using MC result file: " + mcResultFile);
        Validator::ValidateNotNull(dataset, "dataset");

        // Set data and apply cuts
        SetData(dataset);
        if (!options.cutExpr.empty()) {
            ApplyCut(options.cutExpr);
        }

        // Build PDFs and model
        SetSignalPDF(signalParams, "signal");
        SetBackgroundPDF(backgroundParams, "background");
        CreateTotalPDF();
        if (!ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", "PerformGaussianConstraintFitWithMCFile");
        }

        // Build Gaussian constraints from saved RooFitResult
        std::unique_ptr<TFile> fin(TFile::Open(mcResultFile.c_str(), "READ"));
        if (!fin || fin->IsZombie()) {
            LOG_AND_THROW(MassFitterException, "Cannot open MC result file: " + mcResultFile, "PerformGaussianConstraintFitWithMCFile");
        }
        RooFitResult* rf = dynamic_cast<RooFitResult*>(fin->Get("fitResult"));
        if (!rf) {
            LOG_AND_THROW(MassFitterException, "fitResult object not found in MC file", "PerformGaussianConstraintFitWithMCFile");
        }

        std::map<std::string, std::pair<double,double>> constr; // name -> {mean, sigma}
        auto finals = rf->floatParsFinal();
        for (const auto& pname : paramsToConstrain) {
            RooAbsArg* arg = nullptr;
            for (int i=0; i<finals.getSize(); ++i) {
                if (std::string(finals.at(i)->GetName()) == pname) { arg = finals.at(i); break; }
            }
            if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                double mu = v->getVal();
                double err = v->getError();
                if (err <= 0) {
                    double eh = std::abs(v->getErrorHi());
                    double el = std::abs(v->getErrorLo());
                    err = 0.5 * (eh + el);
                    if (err <= 0) err = 1e-6;
                }
                constr[pname] = {mu, std::max(err, 1e-6)};
            }
        }

        std::unique_ptr<RooArgSet> dataVars(totalPdf_->getVariables());
        RooArgSet constraintPdfs;
        std::cout << "[GC-File][Debug] Prepared " << constr.size() << " constraints from file '" 
                  << mcResultFile << "'" << std::endl;
        std::vector<std::unique_ptr<RooRealVar>> meansKeeper;
        std::vector<std::unique_ptr<RooRealVar>> sigmasKeeper;
        std::vector<std::unique_ptr<RooGaussian>> gaussKeeper;
        int applied = 0, missing = 0;
        for (const auto& [name, muSig] : constr) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(name.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("mc_mean_" + name).c_str(), ("mc_mean_" + name).c_str(), muSig.first);
                auto sigma = std::make_unique<RooRealVar>(("mc_sigma_" + name).c_str(), ("mc_sigma_" + name).c_str(), muSig.second);
                sigma->setMin(1e-12);
                sigma->setConstant(true);
                mean->setConstant(true);
                auto gauss = std::make_unique<RooGaussian>(("constr_" + name).c_str(), ("constr_" + name).c_str(), *target, *mean, *sigma);
                constraintPdfs.add(*gauss);
                meansKeeper.push_back(std::move(mean));
                sigmasKeeper.push_back(std::move(sigma));
                gaussKeeper.push_back(std::move(gauss));
                std::cout << "[GC-File][Debug] Apply constraint to '" << name << "': mu=" << muSig.first
                          << ", sigma=" << muSig.second << ", data init=" << target->getVal() << std::endl;
                applied++;
            } else {
                std::cout << "[GC-File][Debug][WARN] Data var not found for constraint '" << name << "'" << std::endl;
                missing++;
            }
        }
        std::cout << "[GC-File][Debug] Constraints applied: " << applied << ", missing: " << missing << std::endl;

        // Fit with external constraints
        FitConfig cfg = options.ToFitConfig();
        if (activeMassVar_) {
            if (!cfg.rangeName.empty()) activeMassVar_->setRange(cfg.rangeName.c_str(), cfg.rangeMin, cfg.rangeMax);
            activeMassVar_->setMin(cfg.rangeMin); activeMassVar_->setMax(cfg.rangeMax);
        }
        RooLinkedList fitOpts;
        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(cfg.numCPU)));
        fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(cfg.verbose ? 1 : -1)));
        fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer(cfg.strategy.c_str(), cfg.minimizer.c_str())));
        fitOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
        fitOpts.Add(new RooCmdArg((cfg.fitMethod == FitMethod::Extended || cfg.fitMethod == FitMethod::Robust) ? RooFit::Extended(true) : RooFit::Extended(false)));
        if (cfg.useHesse) fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
        if (cfg.useMinos) fitOpts.Add(new RooCmdArg(RooFit::Minos(true)));
        if (!cfg.rangeName.empty()) fitOpts.Add(new RooCmdArg(RooFit::Range(cfg.rangeName.c_str())));
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
        if (cfg.useCUDA) fitOpts.Add(new RooCmdArg(RooFit::EvalBackend("cuda")));
#endif
        if (constraintPdfs.getSize() > 0) {
            fitOpts.Add(new RooCmdArg(RooFit::ExternalConstraints(constraintPdfs)));
            std::cout << "[GC-File][Debug] ExternalConstraints attached: count=" << constraintPdfs.getSize() << std::endl;
        } else {
            std::cout << "[GC-File][Debug] No ExternalConstraints attached" << std::endl;
        }

        auto fitResult = std::unique_ptr<RooFitResult>(totalPdf_->fitTo(*activeDataset_, fitOpts));
        if (!fitResult) {
            LOG_AND_THROW(MassFitterException, "Gaussian constrained (file) fit returned null result", "PerformGaussianConstraintFitWithMCFile");
        }
        // Post-fit debug: compare constrained parameters to MC means
        if (fitResult) {
            auto finals = fitResult->floatParsFinal();
            for (const auto& [name, muSig] : constr) {
                RooAbsArg* arg = nullptr;
                for (int i=0; i<finals.getSize(); ++i) {
                    if (std::string(finals.at(i)->GetName()) == name) { arg = finals.at(i); break; }
                }
                if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                    double val = v->getVal();
                    double err = v->getError();
                    std::cout << "[GC-File][Debug][PostFit] " << name << " = " << val
                              << " ± " << err << " (MC mu=" << muSig.first
                              << ", pull=" << ((err>0)?(val - muSig.first)/err:0.0) << ")" << std::endl;
                }
            }
        }

        // Store results as usual
        auto workspace = std::make_unique<RooWorkspace>(("constraintWorkspace_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        if (signalPdf_) workspace->import(*signalPdf_);
        if (backgroundPdf_) workspace->import(*backgroundPdf_);

        std::string finalResultName = resultName.empty() ? (GenerateResultName() + std::string("_gauss_constr")) : resultName;
        resultManager_->StoreResult(finalResultName, std::move(fitResult), std::move(workspace), "GaussianConstraintFit");
        std::map<std::string, RooAbsReal*> yieldExprs2;
        if (nsig_) yieldExprs2["nsig"] = nsig_.get();
        if (nbkg_) yieldExprs2["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalResultName, yieldExprs2);
        resultManager_->CalculateChiSquare(finalResultName, totalPdf_.get(), activeDataset_, activeMassVar_);

        LogOperation("PerformGaussianConstraintFitWithMCFile", "Completed successfully: " + finalResultName);
        return true;

    } catch (const std::exception& e) {
        HandleFitException(e, "PerformGaussianConstraintFitWithMCFile");
        return false;
    }
}

// Background sideband prefit → weak Gaussian constraints on background parameters
template<typename SignalParams, typename BackgroundParams>
bool MassFitterV2::PerformSidebandPrefitBackgroundConstraintFit(
    const FitOpt& options, RooDataSet* dataset,
    const SignalParams& signalParams, const BackgroundParams& backgroundParams,
    double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
    double sigmaScale, const std::string& resultName) {
    try {
        LogOperation("SBPrefitBkg", "Starting sideband prefit-constrained fit");
        Validator::ValidateNotNull(dataset, "dataset");

        // 1) Set data and cuts
        SetData(dataset);
        if (!options.cutExpr.empty()) ApplyCut(options.cutExpr);

        // 2) Build background PDF and perform background-only prefit in sidebands
        SetBackgroundPDF(backgroundParams, "background");
        if (!backgroundPdf_) {
            LOG_AND_THROW(PDFCreationException, "Background PDF not configured", "SBPrefitBkg");
        }

        FitConfig cfg = options.ToFitConfig();
        if (activeMassVar_) {
            activeMassVar_->setRange("SBlo", sbLoMin, sbLoMax);
            activeMassVar_->setRange("SBhi", sbHiMin, sbHiMax);
            activeMassVar_->setMin(cfg.rangeMin);
            activeMassVar_->setMax(cfg.rangeMax);
        }

        RooLinkedList preOpts;
        preOpts.Add(new RooCmdArg(RooFit::NumCPU(cfg.numCPU)));
        preOpts.Add(new RooCmdArg(RooFit::PrintLevel(cfg.verbose ? 1 : -1)));
        preOpts.Add(new RooCmdArg(RooFit::Save(true)));
        preOpts.Add(new RooCmdArg(RooFit::Minimizer(cfg.strategy.c_str(), cfg.minimizer.c_str())));
        preOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
        preOpts.Add(new RooCmdArg(RooFit::Range("SBlo,SBhi")));

        std::cout << "[SB][Debug] Prefitting background in SB ranges: [" << sbLoMin << ", " << sbLoMax
                  << "] U [" << sbHiMin << ", " << sbHiMax << "]" << std::endl;
        auto prefit = std::unique_ptr<RooFitResult>(backgroundPdf_->fitTo(*activeDataset_, preOpts));
        if (!prefit) {
            LOG_AND_THROW(MassFitterException, "Sideband prefit returned null result", "SBPrefitBkg");
        }

        // 3) Collect background parameters and build constraints
        std::map<std::string, std::pair<double,double>> constr; // name -> (mu, sigma)
        std::unique_ptr<RooArgSet> bkgVars(backgroundPdf_->getVariables());
        int totalVars = 0;
        for (auto it = bkgVars->fwdIterator(); auto* var = it.next();) {
            if (auto* v = dynamic_cast<RooRealVar*>(var)) {
                std::string pname = v->GetName();
                // Only constrain background-labeled params
                bool isBkg = pname.size() >= 11 && pname.rfind("_background") == pname.size() - 11;
                if (!isBkg) continue;
                if (v->isConstant()) continue;
                double mu = v->getVal();
                double err = v->getError();
                if (err <= 0) {
                    double eh = std::abs(v->getErrorHi());
                    double el = std::abs(v->getErrorLo());
                    err = 0.5 * (eh + el);
                }
                if (err <= 0) err = 1e-6;
                err = std::max(err * sigmaScale, 1e-6);
                constr[pname] = {mu, err};
                totalVars++;
                std::cout << "[SB][Debug] PreFit param '" << pname << "': mu=" << mu << ", err=" << err/sigmaScale
                          << " → cons sigma=" << err << std::endl;
            }
        }
        std::cout << "[SB][Debug] Collected " << constr.size() << " background parameters for constraints (from "
                  << totalVars << " candidates)" << std::endl;

        // 4) Build full model (signal + background) and apply constraints on background params
        SetSignalPDF(signalParams, "signal");
        CreateTotalPDF();
        if (!ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", "SBPrefitBkg");
        }

        std::unique_ptr<RooArgSet> dataVars(totalPdf_->getVariables());
        RooArgSet constraintPdfs;
        std::vector<std::unique_ptr<RooRealVar>> meanKeep;
        std::vector<std::unique_ptr<RooRealVar>> sigmaKeep;
        std::vector<std::unique_ptr<RooGaussian>> gaussKeep;
        int applied = 0, missing = 0;
        for (const auto& [name, ms] : constr) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(name.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("sb_mean_" + name).c_str(), ("sb_mean_" + name).c_str(), ms.first);
                auto sigm = std::make_unique<RooRealVar>(("sb_sigma_" + name).c_str(), ("sb_sigma_" + name).c_str(), ms.second);
                sigm->setMin(1e-12); sigm->setConstant(true);
                mean->setConstant(true);
                auto gauss = std::make_unique<RooGaussian>(("sb_constr_" + name).c_str(), ("sb_constr_" + name).c_str(), *target, *mean, *sigm);
                constraintPdfs.add(*gauss);
                meanKeep.push_back(std::move(mean));
                sigmaKeep.push_back(std::move(sigm));
                gaussKeep.push_back(std::move(gauss));
                std::cout << "[SB][Debug] Apply constraint to '" << name << "': mu=" << ms.first
                          << ", sigma=" << ms.second << ", data init=" << target->getVal() << std::endl;
                applied++;
            } else {
                std::cout << "[SB][Debug][WARN] Data var not found for constraint '" << name << "'" << std::endl;
                missing++;
            }
        }
        std::cout << "[SB][Debug] Constraints applied: " << applied << ", missing: " << missing << std::endl;

        // 5) Perform constrained full fit
        FitConfig fullCfg = options.ToFitConfig();
        if (activeMassVar_) {
            if (!fullCfg.rangeName.empty()) activeMassVar_->setRange(fullCfg.rangeName.c_str(), fullCfg.rangeMin, fullCfg.rangeMax);
            activeMassVar_->setMin(fullCfg.rangeMin); activeMassVar_->setMax(fullCfg.rangeMax);
        }
        RooLinkedList fitOpts;
        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(fullCfg.numCPU)));
        fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(fullCfg.verbose ? 1 : -1)));
        fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer(fullCfg.strategy.c_str(), fullCfg.minimizer.c_str())));
        fitOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
        fitOpts.Add(new RooCmdArg((fullCfg.fitMethod == FitMethod::Extended || fullCfg.fitMethod == FitMethod::Robust)
                                  ? RooFit::Extended(true) : RooFit::Extended(false)));
        if (fullCfg.useHesse) fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
        if (fullCfg.useMinos) fitOpts.Add(new RooCmdArg(RooFit::Minos(true)));
        if (!fullCfg.rangeName.empty()) fitOpts.Add(new RooCmdArg(RooFit::Range(fullCfg.rangeName.c_str())));
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
        if (fullCfg.useCUDA) fitOpts.Add(new RooCmdArg(RooFit::EvalBackend("cuda")));
#endif
        if (constraintPdfs.getSize() > 0) {
            fitOpts.Add(new RooCmdArg(RooFit::ExternalConstraints(constraintPdfs)));
            std::cout << "[SB][Debug] ExternalConstraints attached: count=" << constraintPdfs.getSize() << std::endl;
        }

        std::unique_ptr<RooFitResult> fitResult;
        if (fullCfg.fitMethod == FitMethod::BinnedNLL) {
            RooRealVar* m = activeMassVar_;
            if (!m) {
                auto argSet = activeDataset_->get();
                for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
                    if (auto* rv = dynamic_cast<RooRealVar*>(var)) { m = rv; break; }
                }
            }
            if (!m) LOG_AND_THROW(MassFitterException, "Mass variable not found for binned fit", "SBPrefitBkg");
            auto binnedVar = std::unique_ptr<RooRealVar>(dynamic_cast<RooRealVar*>(m->clone((std::string(m->GetName()) + "_binned").c_str())));
            binnedVar->setBins(fullCfg.histogramBins);
            auto binnedData = std::make_unique<RooDataHist>((std::string(activeDataset_->GetName()) + "_binned").c_str(),
                                                            (std::string(activeDataset_->GetTitle()) + " (binned)").c_str(),
                                                            RooArgSet(*binnedVar), *activeDataset_);
            fitResult = std::unique_ptr<RooFitResult>(totalPdf_->fitTo(*binnedData, fitOpts));
        } else {
            fitResult = std::unique_ptr<RooFitResult>(totalPdf_->fitTo(*activeDataset_, fitOpts));
        }
        if (!fitResult) {
            LOG_AND_THROW(MassFitterException, "Sideband-constrained fit returned null result", "SBPrefitBkg");
        }

        // Post-fit debug
        auto finals = fitResult->floatParsFinal();
        for (const auto& [name, ms] : constr) {
            RooAbsArg* arg = nullptr;
            for (int i=0; i<finals.getSize(); ++i) {
                if (std::string(finals.at(i)->GetName()) == name) { arg = finals.at(i); break; }
            }
            if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                double val = v->getVal();
                double err = v->getError();
                std::cout << "[SB][Debug][PostFit] " << name << " = " << val
                          << " ± " << err << " (SB mu=" << ms.first
                          << ", pull=" << ((err>0)?(val - ms.first)/err:0.0) << ")" << std::endl;
            }
        }

        // 6) Store results
        auto workspace = std::make_unique<RooWorkspace>(("sbConstraintWorkspace_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        if (signalPdf_) workspace->import(*signalPdf_);
        if (backgroundPdf_) workspace->import(*backgroundPdf_);

        std::string finalResultName = resultName.empty() ? (GenerateResultName() + std::string("_sb_constr")) : resultName;
        resultManager_->StoreResult(finalResultName, std::move(fitResult), std::move(workspace), "SidebandConstraintFit");
        std::map<std::string, RooAbsReal*> yieldExprs;
        if (nsig_) yieldExprs["nsig"] = nsig_.get();
        if (nbkg_) yieldExprs["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalResultName, yieldExprs);
        resultManager_->CalculateChiSquare(finalResultName, totalPdf_.get(), activeDataset_, activeMassVar_);

        LogOperation("SBPrefitBkg", "Completed successfully: " + finalResultName);
        return true;

    } catch (const std::exception& e) {
        HandleFitException(e, "SBPrefitBkg");
        return false;
    }
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

    try {
        LogOperation("GC+SB", "Starting combined GaussianConstraint (signal) + SB constraints (background)");
        Validator::ValidateNotNull(dataset, "dataset");
        Validator::ValidateNotNull(mcDataset, "mcDataset");

        // Reduce MC dataset if requested
        std::unique_ptr<RooDataSet> mcReduced;
        RooDataSet* mcForConstraints = mcDataset;
        if (!options.cutMCExpr.empty()) {
            mcReduced.reset(dynamic_cast<RooDataSet*>(mcDataset->reduce(options.cutMCExpr.c_str())));
            if (mcReduced) mcForConstraints = mcReduced.get();
        }

        // Set data and optional cut
        SetData(dataset);
        if (!options.cutExpr.empty()) ApplyCut(options.cutExpr);

        // Build PDFs and model
        SetSignalPDF(signalParams, "signal");
        SetBackgroundPDF(backgroundParams, "background");
        CreateTotalPDF();
        if (!ValidatePDFSetup()) {
            LOG_AND_THROW(PDFCreationException, "Total PDF is not configured", "GC+SB");
        }

        FitConfig baseCfg = options.ToFitConfig();
        if (activeMassVar_) {
            if (!baseCfg.rangeName.empty()) activeMassVar_->setRange(baseCfg.rangeName.c_str(), baseCfg.rangeMin, baseCfg.rangeMax);
            activeMassVar_->setMin(baseCfg.rangeMin); activeMassVar_->setMax(baseCfg.rangeMax);
        }

        // 1) Build signal constraints from MC (fit signalPdf_ to MC dataset)
        std::map<std::string, std::pair<double,double>> sigConstr;
        auto makeFitOpts = [](const FitConfig& cfg) {
            RooLinkedList opts;
            opts.Add(new RooCmdArg(RooFit::NumCPU(cfg.numCPU)));
            opts.Add(new RooCmdArg(RooFit::PrintLevel(cfg.verbose ? 1 : -1)));
            opts.Add(new RooCmdArg(RooFit::Save(true)));
            opts.Add(new RooCmdArg(RooFit::Minimizer(cfg.strategy.c_str(), cfg.minimizer.c_str())));
            opts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
            return opts;
        };
        {
            FitConfig mcCfg = baseCfg; mcCfg.useMinos = false;
            auto mcOpts = makeFitOpts(mcCfg);
            auto mcRes = std::unique_ptr<RooFitResult>(signalPdf_->fitTo(*mcForConstraints, mcOpts));
            if (!mcRes) {
                LOG_AND_THROW(MassFitterException, "MC signal prefit failed", "GC+SB");
            }
            std::unique_ptr<RooArgSet> sigVars(signalPdf_->getVariables());
            for (const auto& name : signalParamsToConstrain) {
                if (auto* v = dynamic_cast<RooRealVar*>(sigVars->find(name.c_str()))) {
                    double mu = v->getVal();
                    double err = v->getError();
                    if (err <= 0) {
                        double eh = std::abs(v->getErrorHi());
                        double el = std::abs(v->getErrorLo());
                        err = 0.5 * (eh + el);
                    }
                    err = std::max(err * sigmaScaleSignal, 1e-6);
                    sigConstr[name] = {mu, err};
                    std::cout << "[GC+SB][Sig] '" << name << "': mu=" << mu << ", err=" << err/sigmaScaleSignal
                              << " → cons sigma=" << err << std::endl;
                } else {
                    std::cout << "[GC+SB][Sig][WARN] MC variable not found: '" << name << "'" << std::endl;
                }
            }
        }

        // 2) Background-only sideband prefit and constraints
        std::map<std::string, std::pair<double,double>> bkgConstr;
        if (activeMassVar_) {
            activeMassVar_->setRange("SBlo", sbLoMin, sbLoMax);
            activeMassVar_->setRange("SBhi", sbHiMin, sbHiMax);
        }
        RooLinkedList preOpts;
        preOpts.Add(new RooCmdArg(RooFit::NumCPU(baseCfg.numCPU)));
        preOpts.Add(new RooCmdArg(RooFit::PrintLevel(baseCfg.verbose ? 1 : -1)));
        preOpts.Add(new RooCmdArg(RooFit::Save(true)));
        preOpts.Add(new RooCmdArg(RooFit::Minimizer(baseCfg.strategy.c_str(), baseCfg.minimizer.c_str())));
        preOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
        preOpts.Add(new RooCmdArg(RooFit::Range("SBlo,SBhi")));
        auto prefitB = std::unique_ptr<RooFitResult>(backgroundPdf_->fitTo(*activeDataset_, preOpts));
        if (!prefitB) {
            LOG_AND_THROW(MassFitterException, "Sideband background prefit failed", "GC+SB");
        }
        std::unique_ptr<RooArgSet> bkgVars(backgroundPdf_->getVariables());
        for (auto it = bkgVars->fwdIterator(); auto* var = it.next();) {
            if (auto* v = dynamic_cast<RooRealVar*>(var)) {
                std::string pname = v->GetName();
                bool isBkg = pname.size() >= 11 && pname.rfind("_background") == pname.size() - 11;
                if (!isBkg || v->isConstant()) continue;
                double mu = v->getVal();
                double err = v->getError();
                if (err <= 0) {
                    double eh = std::abs(v->getErrorHi());
                    double el = std::abs(v->getErrorLo());
                    err = 0.5 * (eh + el);
                }
                err = std::max(err * sigmaScaleBkg, 1e-6);
                bkgConstr[pname] = {mu, err};
                std::cout << "[GC+SB][Bkg] '" << pname << "': mu=" << mu << ", err=" << err/sigmaScaleBkg
                          << " → cons sigma=" << err << std::endl;
            }
        }

        // 3) Build single ExternalConstraints set (signal + background)
        std::unique_ptr<RooArgSet> dataVars(totalPdf_->getVariables());
        RooArgSet consPdfs;
        std::vector<std::unique_ptr<RooRealVar>> keepMeans, keepSigmas;
        std::vector<std::unique_ptr<RooGaussian>> keepGauss;
        auto addConstr = [&](const std::string& name, const std::pair<double,double>& ms) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(name.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("gc_mean_" + name).c_str(), ("gc_mean_" + name).c_str(), ms.first);
                auto sigm = std::make_unique<RooRealVar>(("gc_sigma_" + name).c_str(), ("gc_sigma_" + name).c_str(), ms.second);
                sigm->setMin(1e-12); sigm->setConstant(true);
                mean->setConstant(true);
                auto gauss = std::make_unique<RooGaussian>(("gc_constr_" + name).c_str(), ("gc_constr_" + name).c_str(), *target, *mean, *sigm);
                consPdfs.add(*gauss);
                keepMeans.push_back(std::move(mean));
                keepSigmas.push_back(std::move(sigm));
                keepGauss.push_back(std::move(gauss));
                std::cout << "[GC+SB][Apply] '" << name << "': mu=" << ms.first << ", sigma=" << ms.second
                          << ", init=" << target->getVal() << std::endl;
            } else {
                std::cout << "[GC+SB][WARN] Data var not found for constraint '" << name << "'" << std::endl;
            }
        };
        for (const auto& kv : sigConstr) addConstr(kv.first, kv.second);
        for (const auto& kv : bkgConstr) addConstr(kv.first, kv.second);
        std::cout << "[GC+SB] Total ExternalConstraints: " << consPdfs.getSize() << std::endl;

        // 4) Fit with combined external constraints
        FitConfig cfg = baseCfg;
        RooLinkedList fitOpts;
        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(cfg.numCPU)));
        fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(cfg.verbose ? 1 : -1)));
        fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer(cfg.strategy.c_str(), cfg.minimizer.c_str())));
        fitOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
        fitOpts.Add(new RooCmdArg((cfg.fitMethod == FitMethod::Extended || cfg.fitMethod == FitMethod::Robust)
                                  ? RooFit::Extended(true) : RooFit::Extended(false)));
        if (cfg.useHesse) fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
        if (cfg.useMinos) fitOpts.Add(new RooCmdArg(RooFit::Minos(true)));
        if (!cfg.rangeName.empty()) fitOpts.Add(new RooCmdArg(RooFit::Range(cfg.rangeName.c_str())));
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
        if (cfg.useCUDA) fitOpts.Add(new RooCmdArg(RooFit::EvalBackend("cuda")));
#endif
        if (consPdfs.getSize() > 0) fitOpts.Add(new RooCmdArg(RooFit::ExternalConstraints(consPdfs)));

        std::unique_ptr<RooFitResult> fitResult;
        if (cfg.fitMethod == FitMethod::BinnedNLL) {
            RooRealVar* m = activeMassVar_;
            if (!m) {
                auto argSet = activeDataset_->get();
                for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
                    if (auto* rv = dynamic_cast<RooRealVar*>(var)) { m = rv; break; }
                }
            }
            if (!m) LOG_AND_THROW(MassFitterException, "Mass var not found for binned fit", "GC+SB");
            auto binnedVar = std::unique_ptr<RooRealVar>(dynamic_cast<RooRealVar*>(m->clone((std::string(m->GetName()) + "_binned").c_str())));
            binnedVar->setBins(cfg.histogramBins);
            auto binnedData = std::make_unique<RooDataHist>((std::string(activeDataset_->GetName()) + "_binned").c_str(),
                                                            (std::string(activeDataset_->GetTitle()) + " (binned)").c_str(),
                                                            RooArgSet(*binnedVar), *activeDataset_);
            fitResult = std::unique_ptr<RooFitResult>(totalPdf_->fitTo(*binnedData, fitOpts));
        } else {
            fitResult = std::unique_ptr<RooFitResult>(totalPdf_->fitTo(*activeDataset_, fitOpts));
        }
        if (!fitResult) {
            LOG_AND_THROW(MassFitterException, "Combined constraint fit returned null result", "GC+SB");
        }

        // Post-fit debug
        auto finals = fitResult->floatParsFinal();
        auto postPrint = [&](const std::string& name, double mu) {
            RooAbsArg* arg = nullptr;
            for (int i=0; i<finals.getSize(); ++i) { if (std::string(finals.at(i)->GetName()) == name) { arg = finals.at(i); break; } }
            if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                double val = v->getVal(); double err = v->getError();
                std::cout << "[GC+SB][PostFit] " << name << " = " << val << " ± " << err
                          << " (mu=" << mu << ", pull=" << ((err>0)?(val - mu)/err:0.0) << ")" << std::endl;
            }
        };
        for (const auto& kv : sigConstr) postPrint(kv.first, kv.second.first);
        for (const auto& kv : bkgConstr) postPrint(kv.first, kv.second.first);

        // Store results
        auto workspace = std::make_unique<RooWorkspace>(("gcSbWorkspace_" + name_).c_str());
        workspace->import(*totalPdf_);
        workspace->import(*activeDataset_);
        if (signalPdf_) workspace->import(*signalPdf_);
        if (backgroundPdf_) workspace->import(*backgroundPdf_);
        std::string finalName = resultName.empty() ? (GenerateResultName() + std::string("_gc_sb")) : resultName;
        resultManager_->StoreResult(finalName, std::move(fitResult), std::move(workspace), "GaussianConstraint+Sideband");
        std::map<std::string, RooAbsReal*> yexprs; if (nsig_) yexprs["nsig"] = nsig_.get(); if (nbkg_) yexprs["nbkg"] = nbkg_.get();
        resultManager_->StoreYieldsFromAbsReal(finalName, yexprs);
        resultManager_->CalculateChiSquare(finalName, totalPdf_.get(), activeDataset_, activeMassVar_);

        LogOperation("GC+SB", "Completed successfully: " + finalName);
        return true;

    } catch (const std::exception& e) {
        HandleFitException(e, "GC+SB");
        return false;
    }
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
    std::cout << "[" << name_ << "] " << operation;
    if (!details.empty()) {
        std::cout << ": " << details;
    }
    std::cout << std::endl;
}

inline void MassFitterV2::HandleFitException(const std::exception& e, const std::string& context) {
    std::cerr << "Exception in " << context << ": " << e.what() << std::endl;
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
    std::cout << "Saving results to: " << filePath << "/" << fileName << std::endl;
    
    if (!resultManager_) {
        std::cout << "[MassFitterV2] ResultManager not configured; cannot save results." << std::endl;
        return;
    }
    
    try {
        // Ensure the output directory exists (create recursively if needed)
        createDir(filePath);
        // Use ResultManager to save the result
        std::string fullPath = filePath + "/" + fileName;
        resultManager_->SaveResult(resultName, fullPath, saveWorkspace);
        std::cout << "[MassFitterV2] Results saved successfully: " << fullPath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[MassFitterV2] Failed to save results: " << e.what() << std::endl;
    }
}

inline void MassFitterV2::SaveResults(const std::string& filePath, const std::string& fileName, bool saveWorkspaces) {
    std::cout << "Saving all results to: " << filePath << "/" << fileName << std::endl;
    
    if (!resultManager_) {
        std::cout << "[MassFitterV2] ResultManager not configured; cannot save results." << std::endl;
        return;
    }
    
    try {
        // Ensure the output directory exists (create recursively if needed)
        createDir(filePath);
        // Save all stored results
        auto resultNames = GetResultNames();
        if (resultNames.empty()) {
            std::cout << "[MassFitterV2] No results to save." << std::endl;
            return;
        }
        
        std::string fullPath = filePath + "/" + fileName;
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

// inline void MassFitterV2::UseDeltaMass(bool use, double daughterMassMin, double daughterMassMax) {
//     useDeltaMass_ = use;
//     if (use && !massDaughter1_) {
//         massDaughter1_ = std::make_unique<RooRealVar>("massDaughter1", "Daughter1 Mass", daughterMassMin, daughterMassMax);
//         massPion_ = std::make_unique<RooRealVar>("massPion", "Pion Mass", PION_MASS_V2);
//         massPion_->setConstant(true);
//     }
// }

#endif // MASS_FITTER_V2_H
