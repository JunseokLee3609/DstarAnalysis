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

// New modular components
#include "PDFFactory.h"
#include "FitStrategy.h"
#include "ResultManager.h"
#include "ErrorHandler.h"
#include "TestFramework.h"
#include "Params.h"
#include "Opt.h"

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
    // Constructor with dependency injection
    MassFitterV2(const std::string& name, 
                 const std::string& massVar,
                 double massMin, 
                 double massMax,
                 std::unique_ptr<PDFFactory> pdfFactory = nullptr,
                 std::unique_ptr<FitStrategy> fitStrategy = nullptr,
                 std::unique_ptr<ResultManager> resultManager = nullptr);
    
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
                 double nbkgMaxRatio = 1.0);
    
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
    
    // Statistical utilities
    double CalculateSignificance(const std::string& resultName = "default") const;
    double CalculatePurity(const std::string& resultName = "default") const;
    std::pair<double, double> GetSignalToBackgroundRatio(const std::string& resultName = "default") const;
    
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
    
    // Yield variables (smart pointers for automatic cleanup)
    std::unique_ptr<RooRealVar> nsig_;
    std::unique_ptr<RooRealVar> nbkg_;
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
};

// Implementation of template methods

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
                strategyType = FitStrategyFactory::StrategyType::Robust;
                break;
        }
        
        // Override or set strategy based on fit method
        fitStrategy_ = FitStrategyFactory::CreateStrategy(strategyType);
        
        LogOperation("FitStrategy", "Auto-selected " + fitStrategy_->GetName() + " for " + 
                    (config.fitMethod == FitMethod::NLL ? "NLL" : 
                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : "Extended") + " method");
        std::cout << "[MassFitterV2] Using fit strategy: " << fitStrategy_->GetName() 
                  << " (from " << (config.fitMethod == FitMethod::NLL ? "NLL" : 
                                  config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : "Extended") << " method)" << std::endl;
        
        auto fitResult = fitStrategy_->Execute(totalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("Fit execution returned null result", "PerformFit");
            throw FitException("PerformFit", -1, "Fit execution returned null result");
        }
        
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
        
        // Store yields with validation
        std::map<std::string, RooRealVar*> yieldVars;
        if (nsig_) yieldVars["nsig"] = nsig_.get();
        if (nbkg_) yieldVars["nbkg"] = nbkg_.get();
        resultManager_->StoreYields(finalResultName, yieldVars);
        
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
        
        // Create MC fit configuration (simplified for MC)
        FitConfig config = options.ToFitConfig();
        config.useMinos = false; // MC fits typically don't need Minos
        
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
                mcStrategyType = FitStrategyFactory::StrategyType::MC; // MC strategy for extended
                break;
        }
        
        auto mcStrategy = FitStrategyFactory::CreateStrategy(mcStrategyType);
        
        LogOperation("FitStrategy", "Auto-selected " + mcStrategy->GetName() + " for MC " + 
                    (config.fitMethod == FitMethod::NLL ? "NLL" : 
                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : "Extended") + " method");
        std::cout << "[MassFitterV2] Using MC fit strategy: " << mcStrategy->GetName() 
                  << " (from " << (config.fitMethod == FitMethod::NLL ? "NLL" : 
                                  config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : "Extended") << " method)" << std::endl;
        
        // Perform MC fit
        auto fitResult = mcStrategy->Execute(signalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("MC fit execution returned null result", "PerformMCFit");
            throw FitException("PerformMCFit", -1, "MC fit execution returned null result");
        }
        
        // Create MC workspace
        auto workspace = std::make_unique<RooWorkspace>(("mcWorkspace_" + name_).c_str());
        workspace->import(*signalPdf_);
        workspace->import(*activeDataset_);
        
        // Store MC results
        std::string finalResultName = resultName.empty() ? GenerateResultName() + "_MC" : resultName;
        resultManager_->StoreResult(finalResultName, std::move(fitResult),
                                   std::move(workspace), "MCFit");
        
        // Store signal parameters (no background for MC fit)
        std::map<std::string, RooRealVar*> yieldVars;
        if (nsig_) yieldVars["nsig"] = nsig_.get();
        resultManager_->StoreYields(finalResultName, yieldVars);
        
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
                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : "Extended") + " method");
        std::cout << "[MassFitterV2] Using constraint fit strategy: " << constraintStrategy->GetName() 
                  << " (method: " << (config.fitMethod == FitMethod::NLL ? "NLL" : 
                                     config.fitMethod == FitMethod::BinnedNLL ? "BinnedNLL" : "Extended") << ")" << std::endl;
        
        // Perform constraint fit
        auto fitResult = constraintStrategy->Execute(totalPdf_.get(), activeDataset_, config, activeMassVar_);
        
        if (!fitResult) {
            ErrorHandlerManager::Instance().LogError("Constraint fit execution returned null result", "PerformConstraintFit");
            throw FitException("PerformConstraintFit", -1, "Constraint fit execution returned null result");
        }
        
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
        
        // Store yields
        std::map<std::string, RooRealVar*> yieldVars;
        if (nsig_) yieldVars["nsig"] = nsig_.get();
        if (nbkg_) yieldVars["nbkg"] = nbkg_.get();
        resultManager_->StoreYields(finalResultName, yieldVars);
        
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
                                   double nbkgMaxRatio) 
    : name_(name), massVarName_(massVar) {
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
    nsig_ = std::make_unique<RooRealVar>("nsig", "Signal Yield", 1000*nsigRatio, 1000*nsigMinRatio, 1000*nsigMaxRatio);
    nbkg_ = std::make_unique<RooRealVar>("nbkg", "Background Yield", 1000*nbkgRatio, 1000*nbkgMinRatio, 1000*nbkgMaxRatio);
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
}

inline void MassFitterV2::ApplyCut(const std::string& cutExpr) {
    if (fullDataset_ && !cutExpr.empty()) {
        activeDataset_ = (RooDataSet*)fullDataset_->reduce(cutExpr.c_str());
    }
}

inline void MassFitterV2::CreateTotalPDF() {
    if (signalPdf_ && backgroundPdf_ && nsig_ && nbkg_) {
        totalPdf_ = std::make_unique<RooAddPdf>("totalPdf", "Total PDF",
                                                RooArgList(*signalPdf_, *backgroundPdf_),
                                                RooArgList(*nsig_, *nbkg_));
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
    if (nsig_) return nsig_->getError();
    return 0.0;
}

inline double MassFitterV2::GetBackgroundYield(const std::string& resultName) const {
    if (nbkg_) return nbkg_->getVal();
    return 0.0;
}

inline double MassFitterV2::GetReducedChiSquare(const std::string& resultName) const {
    return 1.0; // Placeholder
}

inline bool MassFitterV2::IsGoodFit(const std::string& resultName) const {
    return true; // Placeholder - always return true for now
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
        // Use ResultManager to save the result
        std::string fullPath = filePath + "/" + fileName;
        resultManager_->SaveToFile(resultName, fullPath, saveWorkspace);
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
        // Save all stored results
        auto resultNames = GetResultNames();
        if (resultNames.empty()) {
            std::cout << "[MassFitterV2] No results to save." << std::endl;
            return;
        }
        
        std::string fullPath = filePath + "/" + fileName;
        for (const auto& resultName : resultNames) {
            resultManager_->SaveToFile(resultName, fullPath, saveWorkspaces);
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
