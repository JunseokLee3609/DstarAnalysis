#ifndef DCA_FITTER_H
#define DCA_FITTER_H

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iostream> // Added for output
#include <limits>
#include <memory>   // Added for smart pointers
#include <utility>

// ROOT includes
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h" // Added for string manipulation
#include "TStyle.h" // For gStyle
#include "TPad.h"   // For gPad
#include "TH1.h"    // For TH1
#include "TLatex.h"  // For CMS label
#include "Opt.h"
#include "Helper.h"
#include "EnhancedPlotManager.h"

// RooFit includes
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooAddPdf.h"
#include "RooArgList.h"
#include "RooArgSet.h" // Added for RooArgSet
#include "RooWorkspace.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooCategory.h" // Added for splitting MC
#include "RooAbsData.h" // Added for DataError options
#include "RooCmdArg.h"
#include "RooLinkedList.h" // Add this include
#include "TSystem.h" // Added for gSystem
#include "RooMsgService.h"

#include "MassFitterV2.h"  // Updated to use V2
#include "FitStrategy.h"    // For configurable fitting strategies
#include "JSONParameterUtils.h"    // For loading PDF params from JSON
#include "JSONParameterLoader.h"   // Loader interface
// #include "../Tools/ConfigManager.h"  // Optional external configuration manager
#include "ErrorHandler.h"   // For enhanced error handling (ConsoleErrorHandler, etc.)
#include "ParameterDebugUtils.h"
#include <sstream>
#include <stdexcept>
#include "Params.h"
using namespace RooFit;

// -----------------------------------------------------------------------------
// Shim classes to ensure this header builds even when a full ConfigManager or
// ErrorHandler class is not available in the project.
// - ConfigManager: provides GetConfiguration returning default FitOpt
// - DCAErrorHandler: provides HandleError(context, message) using ConsoleErrorHandler
// -----------------------------------------------------------------------------
class ConfigManager {
public:
    FitOpt GetConfiguration(const std::string&) const { return FitOpt(); }
};

class DCAErrorHandler {
public:
    void HandleError(const std::string& context, const std::string& message) {
        ConsoleErrorHandler ch;
        ch.HandleException(std::runtime_error(message), context);
    }
};

// Type aliases for cleaner code
using RooRealVarPtr = std::unique_ptr<RooRealVar>;
using RooWorkspacePtr = std::unique_ptr<RooWorkspace>;
using RooDataSetPtr = std::unique_ptr<RooDataSet>;
using TH1DPtr = std::unique_ptr<TH1D>;
using TFilePtr = std::unique_ptr<TFile>;

class DCAFitter {
private:
    std::string LogContext() const;

    template<typename... Args>
    std::string BuildLogMessage(Args&&... args) const;

    template<typename... Args>
    void LogInfoMsg(Args&&... args) const;

    template<typename... Args>
    void LogWarningMsg(Args&&... args) const;

    template<typename... Args>
    void LogErrorMsg(Args&&... args) const;

    bool EnsureDirectoryReady(const std::string& path) const;
    bool EnsureModelPreconditions(const char* context, RooRealVar*& dcaVar);
    bool BuildMCModelFromTemplates(RooRealVar* dcaVar, double totalEvents);

public:
    // Enhanced constructor with dependency injection and MassFitterV2 integration
    DCAFitter(const FitOpt& opt, 
              const std::string& name, 
              const std::string& massVarName, 
              double dcaMin, 
              double dcaMax, 
              int nBins,
              std::unique_ptr<ConfigManager> configManager = nullptr,
              std::unique_ptr<class DCAErrorHandler> errorHandler = nullptr) :
        name_(name),
        opt_(opt),
        latestDetailedPlotDir_(),
        dcaMin_(dcaMin),
        dcaMax_(dcaMax),
        nBins_(nBins),
        massVarName_(massVarName),
        dataSet_(nullptr),
        promptTemplate_(nullptr),
        nonPromptTemplate_(nullptr),
        promptHist_(nullptr),
        nonPromptHist_(nullptr),
        promptPdf_(nullptr),
        nonPromptPdf_(nullptr),
        fracPrompt_(nullptr),
        model_(nullptr),
        outFile_(nullptr),
        dcaBins_(opt.dcaBins),
        configManager_(std::move(configManager)),
        errorHandler_(std::move(errorHandler))
    {
        RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
        try {
            // Initialize default dependencies if not provided
            if (!configManager_) {
                // Provide a minimal ConfigManager shim if project has none
                configManager_ = std::make_unique<ConfigManager>();
            }
            if (!errorHandler_) {
                // Avoid ROOT global ErrorHandler name conflict
                errorHandler_ = std::unique_ptr<DCAErrorHandler>(new DCAErrorHandler());
            }
            
            // Enhanced parameter validation with error handler
            validateConstructorParameters(name, massVarName, dcaMin, dcaMax, nBins);
            
            // Create workspace with enhanced error handling
            initializeWorkspace();
            
            // Initialize variables
            initializeVariables();
            
            LogInfoMsg("Create: Enhanced DCAFitter instance created: ", name_);
            
        } catch (const std::exception& e) {
            if (errorHandler_) {
                errorHandler_->HandleError("DCAFitter Constructor", e.what());
            }
            throw;
        }
    }
    
    // MassFitterV2-style DCA mass fits over all DCA slices
    template<typename SignalParams, typename BackgroundParams>
    bool FitSlicesMassV2(
        RooDataSet* fullDataset,
        const SignalParams& signalParams,
        const BackgroundParams& backgroundParams,
        const std::string& sliceResultPrefix = "DCA_Mass_Slice",
        bool savePlots = false);
    
    // Destructor
    virtual ~DCAFitter() {
        LogInfoMsg("Destroy: DCAFitter instance destroyed");
    }

    // --- Configuration ---
    void setMCFile(const std::string& fileName, const std::string& datasetName = "") {
        mcFileName_ = fileName;
        mcRooDatsetName_ = datasetName;
    }
    // Use MC mass-fit result ROOT (with RooWorkspace) as input for templates
    void setMCResultFile(const std::string& fileName,
                         const std::string& workspaceName = "workspace",
                         const std::string& datasetName = "") {
        mcResultFileName_ = fileName;
        mcWorkspaceName_ = workspaceName;
        mcResultDatasetName_ = datasetName;
    }
    void setDataFile(const std::string& fileName, const std::string& datasetName = "") {
        dataFileName_ = fileName;
        // dataTreeName_ = treeName;
        dataRooDatsetName_ = datasetName; // Default name for data set
    }
    // Use mass-fit result ROOT (with RooWorkspace) as input instead of raw RDS
    void setDataResultFile(const std::string& fileName,
                           const std::string& workspaceName = "workspace",
                           const std::string& datasetName = "") {
        dataResultFileName_ = fileName;
        dataWorkspaceName_ = workspaceName;
        dataResultDatasetName_ = datasetName; // if empty, will try common defaults
    }
    void setDCABranchName(const std::string& branchName) {
        dcaBranchName_ = branchName;
        // Update RooRealVar name if needed, though constructor sets it
        if (dcaVar_) dcaVar_->SetName(branchName.c_str());
    }
    void setMotherPdgIdBranchName(const std::string& branchName) {
        motherPdgIdBranchName_ = branchName;
    }
    void setPromptPdgIds(const std::vector<int>& pdgIds) {
        promptPdgIds_ = pdgIds;
    }
    void setNonPromptPdgIds(const std::vector<int>& pdgIds) {
        nonpromptPdgIds_ = pdgIds;
    }
    // Enable/disable third "test" component and configure transformation
    void EnableTestComponent(bool enable = true) { enableTestComponent_ = enable; }
    void SetTestComponentPower(double power) { testComponentPower_ = power; }
    void UseTestSlope(double slope) { testUseSlope_ = true; testSlope_ = slope; enableTestComponent_ = true; }
    void DisableTestSlope() { testUseSlope_ = false; testSlope_ = 0.0; }
    void UseTestExponential(double lambda) { UseTestSlope(lambda); }
    void DisableTestExponential() { DisableTestSlope(); }
    void UseTestBasePrompt() { testBasePrompt_ = true; }
    void UseTestBaseNonPrompt() { testBasePrompt_ = false; }
    void plotSignalAndSidebandDCAFromHist(const std::string& plotName = "dca_sig_vs_sb_hist_plot");
    bool GenerateTemplatesWithScale(double scale,
                                    std::unique_ptr<TH1D>& promptOut,
                                    std::unique_ptr<TH1D>& nonPromptOut) const;
    void setWeightBranchName(const std::string& branchName = "") {
        weightBranchName_ = branchName;
        if (!weightBranchName_.empty()) {
            weights_ = std::make_unique<RooRealVar>(weightBranchName_.c_str(), "Event Weight", 1.0);
        } else {
            weights_.reset(); 
        }
    }

    // Optional: load mass-fit PDF parameters (signal/background) from external JSON
    void setMassParamJSON(const std::string& jsonFile) {
        try {
            jsonLoader_ = std::make_unique<JSONParameterLoader>();
            jsonLoader_->setIgnoreCentralityInMatching(true);
            jsonLoader_->loadFromFile(jsonFile);
            useJSONParams_ = true;
            LogInfoMsg("JSONParams: Loaded mass-fit parameters from ", jsonFile);
        } catch (const std::exception& e) {
            LogErrorMsg(std::string("Failed to load mass-fit JSON: ") + e.what());
            useJSONParams_ = false;
            jsonLoader_.reset();
        }
    }

    void ConfigureGaussianConstraint(double massCenter, double innerWidth, double outerWidth,
                                     double sigmaScaleSignal = 2.0, double sigmaScaleBackground = 2.0) {
        gaussianConstraintMassCenter_ = massCenter;
        // Ensure inner/outer widths are positive and ordered (inner < outer)
        double inner = std::abs(innerWidth);
        double outer = std::abs(outerWidth);
        if (inner > outer) std::swap(inner, outer);
        gaussianConstraintInnerWidth_ = inner;
        gaussianConstraintOuterWidth_ = outer;
        gaussianConstraintSignalScale_ = sigmaScaleSignal;
        gaussianConstraintBackgroundScale_ = sigmaScaleBackground;
    }

    void SetGaussianConstraintSigmaScales(double sigmaScaleSignal, double sigmaScaleBackground) {
        gaussianConstraintSignalScale_ = sigmaScaleSignal;
        gaussianConstraintBackgroundScale_ = sigmaScaleBackground;
    }

    void SetDefaultGaussianConstraintParameters(const std::vector<std::string>& params) {
        defaultConstraintParams_ = params;
    }

    void setMCCuts(const std::string& cuts) {
        mcCuts_ = cuts;
    }
    void setDataCuts(const std::string& cuts) {
        dataCuts_ = cuts;
    }
    void setOutputFile(const std::string& fileName) {
        outputFileName_ = fileName;
        if (outFile_) {
            outFile_.reset();
        }
    }

    void EnableMassFits(bool enable = true) { runMassFits_ = enable; }

    void setDataYieldHistInput(const std::string& fileName,
                               const std::string& histName = "dataYieldHist") {
        dataYieldHistInputFile_ = fileName;
        dataYieldHistInputHist_ = histName;
    }

    const std::string& GetLatestDetailedPlotDir() const { return latestDetailedPlotDir_; }

    void setDataYieldHistOutput(const std::string& fileName,
                                const std::string& histName = "dataYieldHist") {
        dataYieldHistOutputFile_ = fileName;
        dataYieldHistOutputHist_ = histName;
    }
    
    // Helper function for safe file opening
    TFilePtr openFile(const std::string& fileName, const std::string& mode = "READ") {
        auto file = std::unique_ptr<TFile>(TFile::Open(fileName.c_str(), mode.c_str()));
        if (!file || file->IsZombie()) {
            LogErrorMsg("Could not open file " + fileName);
            return nullptr;
        }
        return file;
    }

    // --- Enhanced MassFitterV2 Integration Methods ---
    
    // Template-based DCA slice fitting with MassFitterV2
    template<typename SignalParams, typename BackgroundParams>
    bool FitDCASlice(RooDataSet* dataSlice, RooDataSet* mcSlice,
                     const SignalParams& signalParams, 
                     const BackgroundParams& backgroundParams,
                     const std::string& sliceName,
                     double dcaLow, double dcaHigh,
                     double& yield, double& yieldError) {
        try {
            if (!dataSlice) {
                throw std::invalid_argument("Data slice is null");
            }
            
            // Create MassFitterV2 instance for this slice (kept alive in sliceFitters_)
            auto* massFitter = createMassFitterV2ForSlice(sliceName);
            
            // Set data
            massFitter->SetData(dataSlice);
            
            // Setup configuration for mass fitting
            FitOpt massFitOpt = createMassFitOpt(sliceName, dcaLow, dcaHigh);
            massFitter->SetConfiguration(massFitOpt);
            
            // Perform template-based fit
            bool fitSuccess = false;
            if (mcSlice) {
                fitSuccess = massFitter->PerformConstraintFit(
                    massFitOpt, dataSlice, mcSlice, signalParams, backgroundParams,
                    {}, sliceName
                );
            } else {
                fitSuccess = massFitter->PerformFit(
                    massFitOpt, dataSlice, signalParams, backgroundParams, sliceName
                );
            }
            
            if (fitSuccess) {
                yield = massFitter->GetSignalYield(sliceName);
                yieldError = massFitter->GetSignalYieldError(sliceName);
                
                // Store result for later access
                sliceFitResults_[sliceName] = massFitter->GetFitResults(sliceName);
                
                LogInfoMsg("FitDCASlice: Success for slice ", sliceName,
                           ": yield=", std::to_string(yield),
                           " +/- ", std::to_string(yieldError));
                return true;
            } else {
                LogWarningMsg("Mass fitting failed for slice: " + sliceName);
                errorHandler_->HandleError("FitDCASlice", "Mass fitting failed for slice: " + sliceName);
                return false;
            }
            
        } catch (const std::exception& e) {
            LogErrorMsg(std::string("Exception in FitDCASlice: ") + e.what());
            errorHandler_->HandleError("FitDCASlice", e.what());
            yield = 0.0;
            yieldError = 0.0;
            return false;
        }
    }
    
    // Enhanced DCA template fitting with configuration management
    template<typename SignalParams, typename BackgroundParams>
    bool FitDCATemplates(const SignalParams& signalParams,
                        const BackgroundParams& backgroundParams,
                        const std::string& strategyName = "Robust") {
        try {
            if (!validateSetup()) {
                throw std::runtime_error("DCAFitter setup validation failed");
            }
            
            // Create global configuration
            FitOpt globalConfig = configManager_->GetConfiguration("DCAFitting");
            
            bool allFitsSuccessful = true;
            sliceFitResults_.clear();
            
            // Iterate through DCA bins and fit each slice
            for (size_t i = 0; i < dcaBins_.size() - 1; ++i) {
                std::string sliceName = Form("dca_%.3f_%.3f", dcaBins_[i], dcaBins_[i+1]);
                
                // Create data slice
                auto dataSlice = createDCASlice(dataSet_, dcaBins_[i], dcaBins_[i+1]);
                auto mcSlice = mcDataSet_ ? createDCASlice(mcDataSet_, dcaBins_[i], dcaBins_[i+1]) : nullptr;
                
                double yield, yieldError;
                bool sliceSuccess = FitDCASlice(dataSlice.get(), mcSlice.get(),
                                              signalParams, backgroundParams,
                                              sliceName,
                                              dcaBins_[i], dcaBins_[i+1],
                                              yield, yieldError);
                
                if (sliceSuccess) {
                    // Store results in histogram
                    updateYieldHistogram(i, yield, yieldError);
                } else {
                    allFitsSuccessful = false;
                }
            }
            
            return allFitsSuccessful;
            
        } catch (const std::exception& e) {
            errorHandler_->HandleError("FitDCATemplates", e.what());
            return false;
        }
    }
    
    // Configuration and dependency management
    void SetConfigManager(std::unique_ptr<ConfigManager> configManager) {
        configManager_ = std::move(configManager);
    }

    void SetErrorHandler(std::unique_ptr<class DCAErrorHandler> errorHandler) {
        errorHandler_ = std::move(errorHandler);
    }
    
    // Result access methods
    FitResults* GetSliceFitResult(const std::string& sliceName) const {
        auto it = sliceFitResults_.find(sliceName);
        return (it != sliceFitResults_.end()) ? it->second : nullptr;
    }
    
    std::vector<std::string> GetSliceNames() const {
        std::vector<std::string> names;
        for (const auto& pair : sliceFitResults_) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    // --- Legacy Workflow Methods (Maintained for compatibility) ---
    bool createTemplatesFromMC() {
        LogInfoMsg("CreateTemplatesFromMC: Starting template creation (mcFile=", mcFileName_,
                   ", mcResult=", mcResultFileName_, ")");
        if ((mcFileName_.empty() && mcResultFileName_.empty()) || dcaBranchName_.empty() || motherPdgIdBranchName_.empty()) {
            LogErrorMsg("MC file/result, DCA branch, or mother PDG branch not configured");
            return false;
        }

        RooDataSet* mcDataset = nullptr;
        std::unique_ptr<TFile> mcFileHandle;
        if (!TryResolveMCDataset(mcDataset, mcFileHandle)) {
            LogErrorMsg("No MC dataset available (workspace and raw file resolution failed)");
            return false;
        }
        return BuildTemplatesFromMCDataset(mcDataset);
    }



    bool loadData() {
        LogInfoMsg("LoadData: Loading data from file: ", dataFileName_);
        if (dataFileName_.empty() || dataRooDatsetName_.empty() || dcaBranchName_.empty()) {
            LogErrorMsg("Data file, dataset name, or DCA branch not configured");
            return false;
        }

        TFile* dataFile = TFile::Open(dataFileName_.c_str());
        if (!dataFile || dataFile->IsZombie()) {
            LogErrorMsg("Could not open data file: " + dataFileName_);
            return false;
        }
        RooDataSet* dataDataset = dynamic_cast<RooDataSet*>(dataFile->Get(dataRooDatsetName_.c_str()));

        // Ensure dcaVar_ uses the correct branch name for loading
        dcaVar_->SetName(dcaBranchName_.c_str());
        RooArgSet dataVars(*dcaVar_);
        std::unique_ptr<RooRealVar> tempWeights; // Use unique_ptr for temporary weight var if needed
        if (!weightBranchName_.empty()) {
             // If using a global weights_ variable:
             if (weights_) {
                 dataVars.add(*weights_);
             } else {
                 // Create a temporary variable just for loading if global one isn't set
                 tempWeights.reset(new RooRealVar(weightBranchName_.c_str(), "Event Weight", 1.0));
                 dataVars.add(*tempWeights);
             }
        }


        // Delete previous dataset if it exists
        if (dataSet_) {
            delete dataSet_; // Delete the old object before creating a new one
            dataSet_ = nullptr;
        }

        const char* weightVarName = !weightBranchName_.empty() ? weightBranchName_.c_str() : nullptr;
        if(dataDataset) {
            dataSet_ = dynamic_cast<RooDataSet*>(dataDataset);
        } else {
            LogErrorMsg("Could not find data dataset '" + dataRooDatsetName_ + "' in file: " + dataFileName_);
            dataFile->Close();
            delete dataFile;
            dcaVar_->SetName(opt_.dcaVar.c_str());
            return false;
        }


        if (!dataSet_ || dataSet_->numEntries() == 0) {
            LogErrorMsg("Failed to load data or no entries passed cuts: " + dataCuts_);
            dataFile->Close();
            delete dataFile;
            // Restore dcaVar_ name
            dcaVar_->SetName(opt_.dcaVar.c_str());
            return false;
        }

        LogInfoMsg("LoadData: Loaded ", std::to_string(dataSet_->sumEntries()),
                   " data entries (after cuts, weighted)");
        ws_->import(*dataSet_); // Import dataset into workspace

        dataFile->Close();
        delete dataFile;
        // Restore dcaVar_ name
        dcaVar_->SetName(opt_.dcaVar.c_str());
        return true;
    }

    // Load data from a MassFitter result file that contains a RooWorkspace
    bool loadDataFromResult() {
        if (dataResultFileName_.empty()) return false;
        LogInfoMsg("LoadDataFromResult: Loading data from result file: ", dataResultFileName_);
        std::unique_ptr<TFile> fin(TFile::Open(dataResultFileName_.c_str(), "READ"));
        if (!fin || fin->IsZombie()) {
            LogErrorMsg("Could not open result file: " + dataResultFileName_);
            return false;
        }
        RooWorkspace* srcWs = dynamic_cast<RooWorkspace*>(fin->Get(dataWorkspaceName_.c_str()));
        if (!srcWs) srcWs = dynamic_cast<RooWorkspace*>(fin->Get("workspace"));
        if (!srcWs) {
            LogErrorMsg("RooWorkspace not found in result file (tried '" + dataWorkspaceName_ + "' and 'workspace')");
            return false;
        }
        // Determine dataset name
        const char* candNames[] = { dataResultDatasetName_.empty() ? nullptr : dataResultDatasetName_.c_str(),
                                    "dataSet", "datasetHX", "reducedData", nullptr };
        RooDataSet* ds = nullptr;
        for (int i = 0; i < 4; ++i) {
            const char* nm = candNames[i];
            if (!nm) continue;
            ds = dynamic_cast<RooDataSet*>(srcWs->data(nm));
            if (ds) { LogInfoMsg("LoadDataFromResult: Found dataset in workspace: ", nm); break; }
        }
        if (!ds) {
            LogErrorMsg("No suitable RooDataSet found in workspace");
            return false;
        }
        dcaVar_->SetName(dcaBranchName_.c_str());
        int rc = ws_->import(*ds);
        if (rc != 0) {
            LogErrorMsg("Failed to import dataset into local workspace (rc=" + std::to_string(rc) + ")");
            return false;
        }
        // Point dataSet_ to the imported dataset inside our workspace
        dataSet_ = dynamic_cast<RooDataSet*>(ws_->data(ds->GetName()));
        if (!dataSet_) {
            LogErrorMsg("Imported dataset not found in local workspace");
            return false;
        }
        LogInfoMsg("LoadDataFromResult: Loaded ", std::to_string(dataSet_->sumEntries()),
                   " entries from result workspace");
        return true;
    }

    bool loadDataYieldHistFromFile() {
        if (dataYieldHistInputFile_.empty()) {
            LogErrorMsg("No data-yield histogram input file specified");
            return false;
        }
        std::unique_ptr<TFile> fIn(TFile::Open(dataYieldHistInputFile_.c_str(), "READ"));
        if (!fIn || fIn->IsZombie()) {
            LogErrorMsg("Failed to open data-yield histogram file: " + dataYieldHistInputFile_);
            return false;
        }

        TH1* h = dynamic_cast<TH1*>(fIn->Get(dataYieldHistInputHist_.c_str()));
        if (!h) {
            LogErrorMsg("Histogram '" + dataYieldHistInputHist_ + "' not found in file " + dataYieldHistInputFile_);
            return false;
        }

        TH1D* hClone = dynamic_cast<TH1D*>(h->Clone("dataYieldHist_loaded"));
        if (!hClone) {
            TH1D* hAsD = dynamic_cast<TH1D*>(h);
            if (!hAsD) {
                LogErrorMsg("Histogram '" + dataYieldHistInputHist_ + "' is not a TH1D");
                return false;
            }
            hClone = static_cast<TH1D*>(hAsD->Clone("dataYieldHist_loaded"));
        }
        hClone->SetDirectory(nullptr);

        if (static_cast<int>(dcaBins_.size()) - 1 != hClone->GetNbinsX()) {
            LogWarningMsg("Data-yield histogram binning mismatch: " + std::to_string(hClone->GetNbinsX()) +
                        " vs expected " + std::to_string(dcaBins_.size() - 1));
        }

        dataYieldHist_.reset(hClone);
        LogInfoMsg("LoadDataYieldHist: Loaded histogram '", dataYieldHistInputHist_, "' from ",
                   dataYieldHistInputFile_, " (integral=", std::to_string(dataYieldHist_->Integral()), ")");
        return true;
    }

    void saveDataYieldHistToFile() const {
        if (!dataYieldHist_ || dataYieldHistOutputFile_.empty()) {
            return;
        }
        std::unique_ptr<TFile> fOut(TFile::Open(dataYieldHistOutputFile_.c_str(), "UPDATE"));
        if (!fOut || fOut->IsZombie()) {
            fOut.reset(TFile::Open(dataYieldHistOutputFile_.c_str(), "RECREATE"));
        }
        if (!fOut || fOut->IsZombie()) {
            ErrorHandlerManager::Instance().LogError("Failed to open output file for data-yield histogram: " + dataYieldHistOutputFile_,
                                                     std::string("DCAFitter:") + name_);
            return;
        }
        fOut->cd();
        TH1D tmp(*dataYieldHist_);
        tmp.SetName(dataYieldHistOutputHist_.c_str());
        tmp.SetDirectory(fOut.get());
        tmp.Write(dataYieldHistOutputHist_.c_str(), TObject::kOverwrite);
        ErrorHandlerManager::Instance().LogInfo("Saved data-yield histogram to " + dataYieldHistOutputFile_ +
                                               " as '" + dataYieldHistOutputHist_ + "'",
                                               std::string("DCAFitter:") + name_);
    }

        // Legacy buildModel disabled due to unstable brace structure; use buildModelwSideband instead
// #if 0
        bool buildModel() { 
        LogInfoMsg("Building RooFit model...");

        RooRealVar* dca = nullptr;
        if (!EnsureModelPreconditions("buildModel", dca)) {
            return false;
        }


        bool histogramReady = false;
        bool shouldSaveYieldHistogram = false;

        if (runMassFits_) {
            LogInfoMsg("Building model using data-driven DCA templates from mass fits.");

            dataYieldHist_.reset();
            dataYieldHist_ = std::make_unique<TH1D>("dataYieldHist", "DCA Yield from Mass Fits", dcaBins_.size() - 1, dcaBins_.data());
            dataYieldHist_->Sumw2();
            for (size_t i = 0; i < dcaBins_.size() - 1; ++i) {
                LogInfoMsg("Processing DCA bin ", i, ": [", dcaBins_[i], ", ", dcaBins_[i + 1], "]");
                double dcaLow = dcaBins_[i];
                double dcaHigh = dcaBins_[i + 1];
                DCASliceInfo sliceInfo(dcaLow, dcaHigh, static_cast<int>(i),
                                       Form("DCA_%.4f_%.4f", dcaLow, dcaHigh));
                BinInfo currentBin = createBinInfoFromFitOpt(opt_, dcaLow, dcaHigh);
                std::string sliceName = Form("dca_%.3f_%.3f", dcaLow, dcaHigh);
                std::replace(sliceName.begin(), sliceName.end(), '.', 'p');

                TString dcaCut = TString::Format("%s >= %f && %s < %f", dca->GetName(), dcaLow, dca->GetName(), dcaHigh);
                std::unique_ptr<RooDataSet> dcaSliceDataHolder;
                RooDataSet* dcaSliceData = nullptr;
                if (dataSet_) {
                    dcaSliceDataHolder.reset(dynamic_cast<RooDataSet*>(dataSet_->reduce(dcaCut.Data())));
                    dcaSliceData = dcaSliceDataHolder.get();
                    if (dcaSliceData) {
                        RooRealVar* dataMassVar = dynamic_cast<RooRealVar*>(dcaSliceData->get()->find(massVarName_.c_str()));
                        if (dataMassVar) {
                            dataMassVar->setRange(opt_.massMin, opt_.massMax);
                            dataMassVar->setMin(opt_.massMin);
                            dataMassVar->setMax(opt_.massMax);
                        }
                    }
                }

                std::unique_ptr<RooDataSet> dcaSliceMCHolder;
                RooDataSet* dcaSliceMC = nullptr;
                if (fullMCDataSet) {
                    dcaSliceMCHolder.reset(dynamic_cast<RooDataSet*>(fullMCDataSet->reduce(dcaCut.Data())));
                    dcaSliceMC = dcaSliceMCHolder.get();
                    if (dcaSliceMC) {
                        RooRealVar* mcMassVar = dynamic_cast<RooRealVar*>(dcaSliceMC->get()->find(massVarName_.c_str()));
                        if (mcMassVar) {
                            mcMassVar->setRange(opt_.massMin, opt_.massMax);
                            mcMassVar->setMin(opt_.massMin);
                            mcMassVar->setMax(opt_.massMax);
                        }
                    }
                }

                double yield = 0;
                double yieldError = 0;
                bool fitSuccess = false;

                if (dcaSliceData && dcaSliceData->numEntries() > 0) {
                    LogInfoMsg("Attempting to fit mass for slice: ", sliceName);

                    try {
                        double nsigRatioInit = 0.5;
                        double nsigRatioMin = 0.0;
                        double nsigRatioMax = 1.0;
                        double nbkgRatioInit = 0.2;
                        double nbkgRatioMin = 0.0;
                        double nbkgRatioMax = 1.0;

                        auto* massFitter = createMassFitterV2ForSlice(sliceName);

                        FitOpt massFitOpt = createMassFitOpt(sliceName, dcaLow, dcaHigh);
                        massFitter->SetConfiguration(massFitOpt);

                        std::string sliceBaseOutputDir = joinPath(massFitOpt.outputDir, massFitOpt.subDir);
                        if (!EnsureDirectoryReady(massFitOpt.outputDir) ||
                            !EnsureDirectoryReady(sliceBaseOutputDir)) {
                            continue;
                        }
                        std::string detailedPlotOutputDir = joinPath(
                            sliceBaseOutputDir,
                            Form("pt%.0f_%.0f_cos%02.0f_%02.0f", opt_.pTMin, opt_.pTMax,
                                 opt_.cosMin * 100.0, opt_.cosMax * 100.0));
                        if (!EnsureDirectoryReady(detailedPlotOutputDir)) {
                            continue;
                        }
                        latestDetailedPlotDir_ = detailedPlotOutputDir;

                        const std::string particleTypeLabel = "D^{*+}";
                        const std::string energyLabel = "ppRef #sqrt{s_{NN}} = 5.36 TeV";

                        std::string ptLabel = massFitOpt.pTLegend;
                        if (ptLabel.empty()) {
                            ptLabel = Form("%.1f < p_{T} < %.1f GeV/c", massFitOpt.pTMin, massFitOpt.pTMax);
                        }

                        std::string yLabel = massFitOpt.yLegend;
                        if (yLabel.empty()) {
                            yLabel = Form("|y| < %.1f", std::abs(massFitOpt.etaMax));
                        }

                        std::string analysisCutLabel = massFitOpt.cosLegend;
                        if (analysisCutLabel.empty()) {
                            analysisCutLabel = Form("%.2f < cos#theta_{HX} < %.2f",
                                                     massFitOpt.cosMin, massFitOpt.cosMax);
                        }

                        std::string dcaLabel = massFitOpt.dcaLegend;
                        if (dcaLabel.empty()) {
                            dcaLabel = Form("%.3f < DCA < %.3f cm", dcaLow, dcaHigh);
                        }

                        std::string slicePlotDir = joinPath(detailedPlotOutputDir, sliceName);
                        if (!EnsureDirectoryReady(slicePlotDir)) {
                            continue;
                        }

                        auto runMCFitAndPlot = [&](const auto& sigParamsForMC,
                                                   const auto& /*bkgParamsForMC*/,
                                                   const std::vector<std::string>& paramsToConstrainForMC) {
                            if (!dcaSliceMC || dcaSliceMC->numEntries() == 0) {
                                return;
                            }

                            std::string mcSliceName = sliceName + "_MC";
                            auto* mcMassFitter = createMassFitterV2ForSlice(mcSliceName);
                            FitOpt mcOpt = createMassFitOpt(mcSliceName, dcaLow, dcaHigh, true);
                            mcOpt.fitMethod = FitMethod::NLL;
                            mcMassFitter->SetConfiguration(mcOpt);
                            mcMassFitter->ConfigureYieldRatios(nsigRatioInit, nsigRatioMin, nsigRatioMax,
                                                               nbkgRatioInit, nbkgRatioMin, nbkgRatioMax);
                            mcMassFitter->SetData(dcaSliceMC);
                            // dcaSliceMC->Print("v");
                            

                            std::vector<std::string> constr = paramsToConstrainForMC;
                            if (constr.empty()) constr = defaultConstraintParams_;

                            bool mcFitOk = false;
                            try {
                                mcFitOk = mcMassFitter->PerformMCFit(
                                    mcOpt, dcaSliceMC,
                                    sigParamsForMC,
                                    mcSliceName);
                            } catch (const std::exception& e) {
                                LogErrorMsg("[DCAFitter][MC] Exception during MC fit for slice ",
                                             mcSliceName, ": ", e.what());
                                mcFitOk = false;
                            }

                            if (!mcFitOk) {
                                LogWarningMsg("[DCAFitter][MC] MC fit failed for slice ", mcSliceName);
                                return;
                            }

                            RooWorkspace* mcWs = mcMassFitter->GetWorkspace(mcSliceName);
                            RooFitResult* mcFitResult = mcMassFitter->GetRooFitResult(mcSliceName);
                            if (!mcWs || !mcFitResult) {
                                LogErrorMsg("[DCAFitter][MC] Missing workspace or fit result for slice ",
                                            mcSliceName);
                                return;
                            }

                            std::string mcPlotDir = joinPath(slicePlotDir, "mc/");
                            if (!EnsureDirectoryReady(mcPlotDir)) {
                                return;
                            }

                            double mcYield = mcMassFitter->GetSignalYield(mcSliceName);
                            double mcYieldErr = mcMassFitter->GetSignalYieldError(mcSliceName);

                            plotSliceFitDetails(dcaSliceMC, mcWs, mcFitResult, massVar_.get(),
                                                mcSliceName, mcOpt, mcPlotDir, true,
                                                particleTypeLabel, energyLabel + " (MC)", ptLabel, yLabel,
                                                analysisCutLabel, dcaLabel,
                                                mcYield, mcYieldErr);
                        };

                        fitSuccess = false;
                        massParamFixedInfo_ = ParameterFixedInfo();
                        bool usedJSONParameters = false;
                        DStarBinParameters jsonParams;
                        KinematicBin debugBin(opt_.pTMin, opt_.pTMax,
                                              opt_.cosMin, opt_.cosMax,
                                              opt_.centMin, opt_.centMax);

                        if (useJSONParams_ && jsonLoader_) {
                            BinIdentifier binId;
                            binId.ptMin = opt_.pTMin; binId.ptMax = opt_.pTMax;
                            binId.cosMin = opt_.cosMin; binId.cosMax = opt_.cosMax;
                            binId.centralityMin = opt_.centMin; binId.centralityMax = opt_.centMax;
                            binId.dcaMin = dcaBins_[i];
                            binId.dcaMax = dcaBins_[i + 1];

                            auto loaded = LoadBinParametersFromJSONWithFixedInfo(*jsonLoader_, binId);
                            jsonParams = loaded.first;
                            massParamFixedInfo_ = loaded.second;

                            if (massParamFixedInfo_.HasJSONConfig()) {
                                usedJSONParameters = true;
                                nsigRatioInit = jsonParams.nsig_ratio;
                                nsigRatioMin = jsonParams.nsig_min_ratio;
                                nsigRatioMax = jsonParams.nsig_max_ratio;
                                nbkgRatioInit = jsonParams.nbkg_ratio;
                                nbkgRatioMin = jsonParams.nbkg_min_ratio;
                                nbkgRatioMax = jsonParams.nbkg_max_ratio;
                            } else {
                                LogWarningMsg("[DCAFitter][JSON] No JSON configuration for slice ", sliceName,
                                              "; falling back to built-in defaults.");
                            }
                        }

                        massFitter->ConfigureYieldRatios(nsigRatioInit, nsigRatioMin, nsigRatioMax,
                                                         nbkgRatioInit, nbkgRatioMin, nbkgRatioMax);
                        massFitter->SetData(dcaSliceData);

                        if (usedJSONParameters) {
                            ParameterDebug::PrintBinParameters(debugBin, jsonParams,
                                "[DCAFitter][JSON] Loaded parameters",
                                dcaBins_[i], dcaBins_[i + 1]);

                            auto logParams = [&](const char* tag, const auto& sigParams, const auto& bkgParams) {
                                double meanVal = safeGetMean(sigParams);
                                double sigmaVal = safeGetSigma(sigParams);
                                if (std::isnan(sigmaVal)) sigmaVal = safeGetSigmaL(sigParams);
                                double lambdaVal = safeGetLambda(bkgParams);
                                LogInfoMsg("[DCAFitter][JSON] ", tag,
                                           " slice=", sliceName,
                                           " mean=", meanVal,
                                           " sigma=", sigmaVal,
                                           " bkg_lambda=", lambdaVal);
                            };

                            auto paramsToConstrain = buildGaussianConstraintParameterList();
                            if (paramsToConstrain.empty()) {
                                paramsToConstrain = defaultConstraintParams_;
                            }
                            const double sbLoMin = 0.140;
                            const double sbLoMax = 0.143;
                            const double sbHiMin = 0.149;
                            const double sbHiMax = 0.155;
                            {
                                std::ostringstream oss;
                                oss << "[GC] Sideband windows: LO[" << sbLoMin << ", " << sbLoMax
                                    << "], HI[" << sbHiMin << ", " << sbHiMax << "]\n"
                                    << "[GC] Sigma scales (signal, background) = ("
                                    << gaussianConstraintSignalScale_ << ", "
                                    << gaussianConstraintBackgroundScale_ << ")";
                                ErrorHandlerManager::Instance().LogInfo(oss.str(), "DCAFitter");
                            }

                            if (!paramsToConstrain.empty()) {
                                std::ostringstream oss;
                                oss << "[GC] Constraining parameters: ";
                                for (size_t idx = 0; idx < paramsToConstrain.size(); ++idx) {
                                    if (idx) oss << ", ";
                                    oss << paramsToConstrain[idx];
                                }
                                ErrorHandlerManager::Instance().LogInfo(oss.str(), "DCAFitter");
                            }

                            jsonParams.ApplyToSignalParams([&](const auto& sigParams) {
                                jsonParams.ApplyToBackgroundParams([&](const auto& bkgParams) {
                                    logParams("Using JSON-loaded parameters", sigParams, bkgParams);
                                    if (dcaSliceMC) {
                                        runMCFitAndPlot(sigParams, bkgParams, paramsToConstrain);
                                        fitSuccess = massFitter->PerformGaussianConstraintFitWithMCAndBkgSB(
                                            massFitOpt, dcaSliceData, dcaSliceMC,
                                            sigParams, bkgParams,
                                            paramsToConstrain,
                                            sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                            gaussianConstraintSignalScale_, gaussianConstraintBackgroundScale_,
                                            sliceName);
                                        // fitSuccess = massFitter->PerformSidebandPrefitBackgroundConstraintFit(
                                        //     massFitOpt, dcaSliceData,
                                        //     sigParams, bkgParams,
                                        //     sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                        //     gaussianConstraintBackgroundScale_,
                                        //     sliceName);
                                        if (fitSuccess) {
                                            runMCFitAndPlot(sigParams, bkgParams, paramsToConstrain);
                                        }
                                    } else {
                                        fitSuccess = massFitter->PerformFit(
                                            massFitOpt, dcaSliceData,
                                            sigParams, bkgParams, sliceName);
                                    }
                                });
                            });
                        }

                        if (!usedJSONParameters) {
                            auto paramsToConstrain = buildGaussianConstraintParameterList();
                            if (paramsToConstrain.empty()) {
                                paramsToConstrain = defaultConstraintParams_;
                            }
                            const double sbLoMin = 0.140;
                            const double sbLoMax = 0.143;
                            const double sbHiMin = 0.149;
                            const double sbHiMax = 0.155;
                            {
                                std::ostringstream oss;
                                oss << "[GC] Sideband windows (fallback): LO[" << sbLoMin << ", "
                                    << sbLoMax << "], HI[" << sbHiMin << ", " << sbHiMax << "]\n"
                                    << "[GC] Sigma scales (signal, background) fallback = ("
                                    << gaussianConstraintSignalScale_ << ", "
                                    << gaussianConstraintBackgroundScale_ << ")";
                                ErrorHandlerManager::Instance().LogInfo(oss.str(), "DCAFitter");
                            }
                            if (!paramsToConstrain.empty()) {
                                std::ostringstream oss;
                                oss << "[GC] Constraining parameters (fallback): ";
                                for (size_t idx = 0; idx < paramsToConstrain.size(); ++idx) {
                                    if (idx) oss << ", ";
                                    oss << paramsToConstrain[idx];
                                }
                                ErrorHandlerManager::Instance().LogInfo(oss.str(), "DCAFitter");
                            }

                            PDFParams::DBCrystalBallParams sigDBCBParams;
                            sigDBCBParams.mean = 0.1455;
                            sigDBCBParams.mean_min = 0.145;
                            sigDBCBParams.mean_max = 0.146;
                            sigDBCBParams.sigma = 0.0005;
                            sigDBCBParams.sigma_min = 0.0001;
                            sigDBCBParams.sigma_max = 0.01;
                            sigDBCBParams.sigmaR = 0.0005;
                            sigDBCBParams.sigmaR_min = 0.0001;
                            sigDBCBParams.sigmaR_max = 0.01;
                            sigDBCBParams.sigmaL = 0.0005;
                            sigDBCBParams.sigmaL_min = 0.0001;
                            sigDBCBParams.sigmaL_max = 0.01;
                            sigDBCBParams.alphaL = 1.1;
                            sigDBCBParams.alphaL_min = 0.1;
                            sigDBCBParams.alphaL_max = 3;
                            sigDBCBParams.nL = 2.5;
                            sigDBCBParams.nL_min = 1;
                            sigDBCBParams.nL_max = 50;
                            sigDBCBParams.alphaR = 1.1;
                            sigDBCBParams.alphaR_min = 0.1;
                            sigDBCBParams.alphaR_max = 3.;
                            sigDBCBParams.nR = 2.5;
                            sigDBCBParams.nR_min = 1;
                            sigDBCBParams.nR_max = 50;

                            PDFParams::Phenomenological2Params bkgD0dstParams;
                            bkgD0dstParams.m = 0.5;   bkgD0dstParams.m_min = 0.0;    bkgD0dstParams.m_max = 5.0;
                            bkgD0dstParams.lambda = -1; bkgD0dstParams.lambda_min = -500; bkgD0dstParams.lambda_max = 2;

                            DStarBinParameters fallbackParams;
                            fallbackParams.signalPdfType = PDFType::DBCrystalBall;
                            fallbackParams.dbCrystalBallParams = sigDBCBParams;
                            fallbackParams.backgroundPdfType = PDFType::Phenomenological2;
                            fallbackParams.phenomenological2Params = bkgD0dstParams;
                            ParameterDebug::PrintBinParameters(debugBin, fallbackParams,
                                "[DCAFitter] Fallback parameters",
                                dcaBins_[i], dcaBins_[i + 1]);

                            if (dcaSliceMC) {
                                fitSuccess = massFitter->PerformGaussianConstraintFitWithMCAndBkgSB(
                                    massFitOpt, dcaSliceData, dcaSliceMC,
                                    sigDBCBParams, bkgD0dstParams,
                                    paramsToConstrain,
                                    sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                    gaussianConstraintSignalScale_, gaussianConstraintBackgroundScale_,
                                    sliceName);
                                // fitSuccess = massFitter->PerformSidebandPrefitBackgroundConstraintFit(
                                //     massFitOpt, dcaSliceData,
                                //     sigDBCBParams, bkgD0dstParams,
                                //     sbLoMin, sbLoMax, sbHiMin, sbHiMax,
                                //     gaussianConstraintBackgroundScale_,
                                //     sliceName);
                                if (fitSuccess) {
                                    runMCFitAndPlot(sigDBCBParams, bkgD0dstParams, paramsToConstrain);
                                }
                            } else {
                                fitSuccess = massFitter->PerformFit(
                                    massFitOpt, dcaSliceData,
                                    sigDBCBParams, bkgD0dstParams, sliceName);
                            }
                        }

                        if (fitSuccess && massFitter->GetSignalYield(sliceName) >= 0) {
                            yield = massFitter->GetSignalYield(sliceName);
                            yieldError = massFitter->GetSignalYieldError(sliceName);
                            LogInfoMsg("Mass fit SUCCESS for DCA slice ", sliceName,
                                        ": Yield = ", yield, " +/- ", yieldError);

                            sliceFitResults_[sliceName] = massFitter->GetFitResults(sliceName);

                            RooWorkspace* mfWs = massFitter->GetWorkspace(sliceName);
                            RooFitResult* mfFitResult = massFitter->GetRooFitResult(sliceName);

                            if (mfWs && mfFitResult) {
                                notifyDCASliceFit(currentBin, sliceInfo, mfFitResult, "DCA_Mass_Slice");
                                plotSliceFitDetails(dcaSliceData, mfWs, mfFitResult, massVar_.get(),
                                                    sliceName, massFitOpt, slicePlotDir, false,
                                                    particleTypeLabel, energyLabel, ptLabel, yLabel,
                                                    analysisCutLabel, dcaLabel, yield, yieldError);
                            } else {
                                LogErrorMsg("Error: Could not retrieve workspace or fit result from MassFitter for slice ",
                                            sliceName);
                            }
                        } else {
                            FitStatus failedStatus(-998, "DCA_Mass_Slice", "Slice fit failed", -1, -1, sliceInfo.sliceName);
                            addFitStatus(currentBin, failedStatus);
                            LogWarningMsg("Mass fit FAILED for DCA slice ", sliceName, ". Yield set to 0.");
                            yield = 0;
                            yieldError = 0;
                            fitSuccess = false;
                        }
                    } catch (const std::exception& e) {
                        std::string errorMsg = "Exception in mass fitting for slice " + sliceName + ": " + std::string(e.what());
                        errorHandler_->HandleError("FitDCASlice", errorMsg);
                        yield = 0.0;
                        yieldError = 0.0;
                        fitSuccess = false;
                    }

                    dataYieldHist_->SetBinContent(i + 1, yield);
                    dataYieldHist_->SetBinError(i + 1, yieldError);
                }
            }

            histogramReady = true;
            shouldSaveYieldHistogram = true;
        } else {
            LogInfoMsg("Using pre-computed data-yield histogram for template building.");
            if (!dataYieldHistInputFile_.empty()) {
                if (!loadDataYieldHistFromFile()) {
                    LogErrorMsg("[DCAFitter] Failed to load data-yield histogram from file. Enable mass fits or provide a valid histogram.");
                    return false;
                }
            } else if (!dataYieldHist_) {
                LogErrorMsg("[DCAFitter] No data-yield histogram in memory and no input file specified.");
                LogErrorMsg("           Call setDataYieldHistInput() or enable mass fits.");
                return false;
            } else {
                LogInfoMsg("[DCAFitter] Reusing in-memory data-yield histogram with integral ",
                           dataYieldHist_->Integral());
            }
            histogramReady = static_cast<bool>(dataYieldHist_);
        }

        if (!histogramReady || !dataYieldHist_) {
            LogErrorMsg("[DCAFitter] Data-yield histogram is not available. Cannot build data-driven template.");
            return false;
        }

        if (dataYieldHist_->GetNbinsX() != static_cast<int>(dcaBins_.size() - 1)) {
            LogWarningMsg("[DCAFitter] Warning: Histogram binning (", dataYieldHist_->GetNbinsX(),
                          ") does not match configured DCA bins (", (dcaBins_.size() - 1), ").");
        }

        delete dataDrivenTemplate_;
        dataDrivenTemplate_ = new RooDataHist("dataDrivenTemplate", "Data-driven DCA Template", RooArgList(*dca), dataYieldHist_.get());
        ws_->import(*dataDrivenTemplate_, RooFit::RecycleConflictNodes());
        LogInfoMsg("[DCAFitter] Data-driven DCA template (RooDataHist) created from ",
                   (runMassFits_ ? "mass fits." : "loaded histogram."));

        if (shouldSaveYieldHistogram) {
            saveDataYieldHistToFile();
        }

        if (dataYieldHist_ && dataDrivenTemplate_) {
            LogInfoMsg("[DCAFitter][DBG] Comparing dataYieldHist vs dataDrivenTemplate.");
            for (int ib = 1; ib <= dataYieldHist_->GetNbinsX(); ++ib) {
                double histYield = dataYieldHist_->GetBinContent(ib);
                double templYield = 0.0;
                if (dataDrivenTemplate_->numEntries() >= ib) {
                    dataDrivenTemplate_->get(ib - 1);
                    templYield = dataDrivenTemplate_->weight();
                }
                LogInfoMsg("  bin ", ib,
                            ": hist=", histYield,
                            ", template=", templYield);
            }
        }


        double nevt = dataYieldHist_->Integral();
        if (!BuildMCModelFromTemplates(dca, nevt)) {
            return false;
        }

        return true;
    }

    RooFitResult* performFit(bool useMinos = false, bool useDataTemplates = true) {
        LogInfoMsg("Performing fit...");

        if (!model_) {
            LogErrorMsg("Model not available for fitting.");
            return nullptr;
        }

        RooAbsData* fitData = nullptr;
        if (useDataTemplates) {
            if (!dataDrivenTemplate_) {
                LogErrorMsg("Data-driven template not available for fitting.");
                return nullptr;
            }
            fitData = dataDrivenTemplate_;
        } else {
            if (!dataSet_) {
                LogErrorMsg("Dataset not available for fitting.");
                return nullptr;
            }
            fitData = dataSet_;
        }

        dcaVar_->SetName(opt_.dcaVar.c_str());

        RooLinkedList fitOptionsList;
        fitOptionsList.Add(RooFit::Save(true).Clone());
        fitOptionsList.Add(RooFit::PrintLevel(-1).Clone());
        fitOptionsList.Add(RooFit::Extended(true).Clone());
        fitOptionsList.Add(RooFit::Minimizer("Minuit", "Minimuzer").Clone());
        fitOptionsList.Add(RooFit::Hesse(true).Clone());
        fitOptionsList.Add(RooFit::Optimize(true).Clone());
        fitOptionsList.Add(RooFit::SumW2Error(true).Clone());
        if (useMinos) {
            fitOptionsList.Add(RooFit::Minos(true).Clone());
        }

        RooFitResult* fitResult = model_->fitTo(*fitData, fitOptionsList);
        if (fitResult) {
            fitResult->Print("v");
        }

        int ntry = 0;
        while (fitResult && (fitResult->statusCodeHistory(0) != 0 || fitResult->statusCodeHistory(1) != 0) && ntry < 3) {
            int status0 = fitResult->statusCodeHistory(0);
            int status1 = fitResult->statusCodeHistory(1);
            delete fitResult;
            RooLinkedList retryOptions = fitOptionsList;
            retryOptions.Add(new RooCmdArg(RooFit::Strategy(2 - ntry)));
            LogWarningMsg("Initial fit failed with status ", status0,
                          " and ", status1, ". Retrying...");
            fitResult = model_->fitTo(*fitData, retryOptions);
            if (fitResult) {
                LogInfoMsg("#### Fit attempt ", ntry + 1,
                           " completed with status: ", fitResult->status(), " ####");
                fitResult->Print("v");
            }
            ++ntry;
        }

        if (!fitResult) {
            LogErrorMsg("Fit failed to produce a result object.");
            return nullptr;
        }

        LogInfoMsg("Fit completed with status: ", fitResult->status());
        LogInfoMsg("Fit result covQual: ", fitResult->covQual());
        if (fitResult->covQual() < 3) {
            LogWarningMsg("Covariance matrix quality is low (", fitResult->covQual(), "). Fit might be unreliable.");
        }

        if (fracPrompt_) {
            delete fracPrompt_;
            fracPrompt_ = nullptr;
        }

        fracPrompt_ = new RooFormulaVar("fracPrompt", "Prompt Fraction from MC fit",
                                        "@0 / (@0 + @1)", RooArgList(*n_prompt_, *n_nonprompt_));

        ws_->import(*fitResult);
        ws_->import(*fracPrompt_);

        return fitResult;
    }

    void plotResults(RooFitResult* fitResult = nullptr,
                     const std::string& plotName = "dca_fit_plot",
                     bool useDataTemplates = true,
                     const std::string& extraOutputDir = "") {
    LogInfoMsg("Plotting results with pull distribution (CMS style)...");
    RooRealVar* dca = ws_->var(opt_.dcaVar.c_str()); 
    if (!dca) {
        dca = dcaVar_.get(); 
        if (!dca) {
             LogErrorMsg("Standard 'dca' variable not found in workspace or dcaVar_ for plotting.");
             return;
        }
    }

    if (!model_) {
        LogErrorMsg("Model not available for plotting.");
        return;
    }

    TCanvas* c = new TCanvas(plotName.c_str(), "DCA Fit Results with Pull", 800, 800);
    // Debug: print presence of testPdf and current n_test
    {
        RooAbsPdf* dbgTestPdf = ws_->pdf("testPdf");
        RooRealVar* dbgNtest = ws_->var("n_test");
        LogInfoMsg("[DCAFitter][Plot] testPdf=", (dbgTestPdf?"present":"absent"),
                  ", n_test=", (dbgNtest?std::to_string(dbgNtest->getVal()):std::string("NA")));
    }
    
    TPad* mainPad = new TPad("mainPad", "Main Plot", 0.0, 0.25, 1.0, 1.0);
    mainPad->SetBottomMargin(0.02);
    mainPad->SetLeftMargin(0.15);
    mainPad->SetRightMargin(0.05);
    mainPad->SetTopMargin(0.08);
    mainPad->SetLogy();
    
    TPad* pullPad = new TPad("pullPad", "Pull Distribution", 0.0, 0.0, 1.0, 0.25);
    pullPad->SetTopMargin(0.02);
    pullPad->SetBottomMargin(0.4);
    pullPad->SetLeftMargin(0.15);
    pullPad->SetRightMargin(0.05);
    
    TPad* ratioPad = new TPad("ratioPad", "Data/Fit Ratio", 0.0, 0.0, 1.0, 0.25);
    ratioPad->SetTopMargin(0.04);
    ratioPad->SetBottomMargin(0.4);
    ratioPad->SetLeftMargin(0.15);
    ratioPad->SetRightMargin(0.05);
    
    mainPad->Draw();
    pullPad->Draw();
    ratioPad->Draw();

    mainPad->cd();
    // dca->setBinning(RooBinning(dcaBins_.size() - 1, dcaBins_.data()));
    RooBinning customBinning(dcaBins_.size() - 1, dcaBins_.data());
    RooPlot* frame = dca->frame(Bins(dcaBins_.size()-1),RooFit::Title(" "));
    // frame->setBinning(customBinning);

    if (useDataTemplates) {
        // if (!dataDrivenTemplate_) {
        //     std::cerr << "Error: dataDrivenTemplate_ (data yield histogram) not available for plotting." << std::endl;
        //     delete c;
        //     return;
        // }
        // TH1* h_density = dataDrivenTemplate_->createHistogram("h_density", *dca);
        // h_density->Scale(1.0, "width");
        // // h_density->Scale(1.0 / h_density->Integral() * dataSet_->sumEntries()); // Normalize to unit area
        // RooHist* rh_density=new RooHist(*h_density, 0.0, 1, RooAbsData::SumW2, 1.0, "P");
        // frame->addPlotable(rh_density, "P");
        
        dataDrivenTemplate_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_yield_hist"), RooFit::DataError(RooAbsData::SumW2));
        // dataDrivenTemplate_->plotOn(frame, RooFit::Name("data_yield_hist"), RooFit::DataError(RooAbsData::SumW2));

        RooAbsPdf* mcPromptPdf = ws_->pdf("promptPdf"); // MC prompt PDF
        RooAbsPdf* mcNonPromptPdf = ws_->pdf("nonPromptPdf"); // MC non-prompt PDF
        RooAbsPdf* mcTestPdf   = ws_->pdf("testPdf");   // MC-derived test PDF (optional)
        
        // dataSet_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_original"), RooFit::DataError(RooAbsData::SumW2));
        // dataSet_->plotOn(frame, RooFit::Name("data_original"), RooFit::DataError(RooAbsData::SumW2));
        
        model_->plotOn(frame,RooFit::Name("model_fit_to_mc"), RooFit::LineColor(kRed + 1));

        if (mcNonPromptPdf) {
            model_->plotOn(frame, RooFit::Components(*mcNonPromptPdf), RooFit::Name("mc_nonprompt_comp"), 
                          RooFit::FillStyle(3354), RooFit::FillColor(kBlue - 9), RooFit::LineColor(kBlue + 1), RooFit::DrawOption("F"));
        }
        if (mcPromptPdf) {
            model_->plotOn(frame, RooFit::Components(*mcPromptPdf), RooFit::Name("mc_prompt_comp"), 
                          RooFit::FillStyle(3345), RooFit::FillColor(kRed - 9), RooFit::LineColor(kRed + 1), RooFit::DrawOption("F"));
        }
        if (mcTestPdf) {
            // Filled area for visibility
            model_->plotOn(frame, RooFit::Components(*mcTestPdf), RooFit::Name("mc_test_comp"),
                           RooFit::FillStyle(3351), RooFit::FillColor(kGreen - 9), RooFit::LineColor(kGreen + 2), RooFit::DrawOption("F"));
            // Outline to ensure it's visible even with small yield
            model_->plotOn(frame, RooFit::Components(*mcTestPdf), RooFit::Name("mc_test_line"),
                           RooFit::LineColor(kGreen + 2), RooFit::LineStyle(kSolid), RooFit::LineWidth(2));
        }

        // Overlay the test template shape (RooDataHist) if available
        if (testTemplate_) {
            testTemplate_->plotOn(frame,
                                  RooFit::Binning(customBinning),
                                  RooFit::Name("test_template_hist"),
                                  RooFit::MarkerStyle(kOpenTriangleUp),
                                  RooFit::MarkerColor(kGreen + 2),
                                  RooFit::LineColor(kGreen + 2));
        } else if (auto* tt = dynamic_cast<RooDataHist*>(ws_->data("testTemplate"))) {
            tt->plotOn(frame,
                       RooFit::Binning(customBinning),
                       RooFit::Name("test_template_hist"),
                       RooFit::MarkerStyle(kOpenTriangleUp),
                       RooFit::MarkerColor(kGreen + 2),
                       RooFit::LineColor(kGreen + 2));
        }
        // sumFunc_->plotOn(frame, RooFit::Name("model_fit_to_mc"),Normalization(dataDrivenTemplate_->sum(false)), RooFit::LineColor(kRed + 1));
        // if (nonPromptFunc_) {
        //     nonPromptFunc_->plotOn(frame, RooFit::Components(*nonPromptFunc_), RooFit::Name("mc_nonprompt_comp"),Normalization(coef_nonprompt_->getVal()*dataDrivenTemplate_->sum(false)), 
        //                   RooFit::FillStyle(3354), RooFit::FillColor(kBlue - 9), RooFit::LineColor(kBlue + 1), RooFit::DrawOption("F"));
        // }
        // if (promptFunc_) {
        //     promptFunc_->plotOn(frame, RooFit::Components(*promptFunc_), RooFit::Name("mc_prompt_comp"),Normalization(coef_prompt_->getVal()*dataDrivenTemplate_->sum(false)), 
        //                   RooFit::FillStyle(3345), RooFit::FillColor(kRed - 9), RooFit::LineColor(kRed + 1), RooFit::DrawOption("F"));
        // }
        
        // dataDrivenTemplate_->plotOn(frame, RooFit::Name("data_yield_hist"), RooFit::DataError(RooAbsData::SumW2));

    } else { 
        if (!dataSet_) {
            dataSet_ = static_cast<RooDataSet*>(ws_->data("dataSet"));
            if (!dataSet_) {
                LogErrorMsg("Original dataset not available for plotting.");
                delete c;
                return;
            }
        }
        
        dataSet_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_original"), RooFit::DataError(RooAbsData::SumW2));

        model_->plotOn(frame, RooFit::Name("model_fit_to_data"), RooFit::LineColor(kRed + 1));

        RooAbsPdf* promptPdf = ws_->pdf("promptPdf");
        RooAbsPdf* nonPromptPdf = ws_->pdf("nonPromptPdf");
        RooAbsPdf* testPdf   = ws_->pdf("testPdf");

        if (nonPromptPdf) {
            model_->plotOn(frame, RooFit::Components(*nonPromptPdf), RooFit::Name("nonprompt_comp"), 
                          RooFit::FillStyle(3354), RooFit::FillColor(kBlue - 9), RooFit::LineColor(kBlue + 1), RooFit::DrawOption("F"));
        }
        if (promptPdf) {
            model_->plotOn(frame, RooFit::Components(*promptPdf), RooFit::Name("prompt_comp"), 
                          RooFit::FillStyle(3345), RooFit::FillColor(kRed - 9), RooFit::LineColor(kRed + 1), RooFit::DrawOption("F"));
        }
        if (testPdf) {
            model_->plotOn(frame, RooFit::Components(*testPdf), RooFit::Name("test_comp"),
                           RooFit::FillStyle(3351), RooFit::FillColor(kGreen - 9), RooFit::LineColor(kGreen + 2), RooFit::DrawOption("F"));
            model_->plotOn(frame, RooFit::Components(*testPdf), RooFit::Name("test_line"),
                           RooFit::LineColor(kGreen + 2), RooFit::LineStyle(kSolid), RooFit::LineWidth(2));
        }

        // Overlay the test template shape (raw shape points)
        if (testTemplate_) {
            testTemplate_->plotOn(frame,
                                  RooFit::Binning(customBinning),
                                  RooFit::Name("test_template_hist"),
                                  RooFit::MarkerStyle(kOpenTriangleUp),
                                  RooFit::MarkerColor(kGreen + 2),
                                  RooFit::LineColor(kGreen + 2));
        } else if (auto* tt = dynamic_cast<RooDataHist*>(ws_->data("testTemplate"))) {
            tt->plotOn(frame,
                       RooFit::Binning(customBinning),
                       RooFit::Name("test_template_hist"),
                       RooFit::MarkerStyle(kOpenTriangleUp),
                       RooFit::MarkerColor(kGreen + 2),
                       RooFit::LineColor(kGreen + 2));
        }
        
        dataSet_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_original"), RooFit::DataError(RooAbsData::SumW2));

    }

    frame->Draw();
    
    if (dataYieldHist_ && useDataTemplates) {
         frame->GetYaxis()->SetTitle(Form("Yield / (%.3f cm)", dataYieldHist_->GetBinWidth(1)));
    } else if (dataSet_ && !useDataTemplates) {
        if (!dcaBins_.empty() && dcaBins_.size() > 1) {
             frame->GetYaxis()->SetTitle(Form("Candidates / (%.3f cm)", dcaBins_[1] - dcaBins_[0]));
        } else {
             frame->GetYaxis()->SetTitle("Candidates");
        }
    } else {
        frame->GetYaxis()->SetTitle("Entries");
    }

    frame->GetXaxis()->SetLabelSize(0);
    frame->GetXaxis()->SetTitleSize(0); 
    frame->GetYaxis()->SetTitleOffset(1.4);
    frame->GetYaxis()->SetTitleSize(0.05);
    frame->GetYaxis()->SetLabelSize(0.04);
    frame->SetMinimum(1);
    frame->SetMaximum(frame->GetMaximum() * 1.8);

    // 범례 추가
    TLegend* leg = new TLegend(0.55, 0.65, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.04);

    auto getFinalYield = [&](const char* name) -> double {
        if (fitResult) {
            const RooArgList& floats = fitResult->floatParsFinal();
            if (auto* arg = floats.find(name)) {
                if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                    return v->getVal();
                }
            }
        }
        if (auto* var = ws_->var(name)) {
            return var->getVal();
        }
        return 0.0;
    };

    auto hasTestComponent = [&]() -> bool {
        if (fitResult) {
            const RooArgList& floats = fitResult->floatParsFinal();
            if (floats.find("n_test")) {
                return true;
            }
        }
        return ws_->var("n_test") != nullptr;
    };

    if (useDataTemplates) {
        TObject* dataYieldObj = frame->findObject("data_yield_hist");
        TObject* modelFitObj = frame->findObject("model_fit_to_mc");
        TObject* mcPromptObj = frame->findObject("mc_prompt_comp");
        TObject* mcNonPromptObj = frame->findObject("mc_nonprompt_comp");
        TObject* mcTestObj = frame->findObject("mc_test_comp");
        TObject* mcTestLineObj = frame->findObject("mc_test_line");
        TObject* myObject = nullptr;

        if (dataYieldObj) leg->AddEntry(dataYieldObj, "Data Yield (from Mass Fit)", "pe");
        if (modelFitObj) leg->AddEntry(modelFitObj, "Total MC Fit", "l");
        if (mcPromptObj) leg->AddEntry(mcPromptObj, "Prompt D^{*} (MC)", "f");
        if (mcNonPromptObj) leg->AddEntry(mcNonPromptObj, "Non-Prompt D^{*} (MC)", "f");
        if (mcTestObj) leg->AddEntry(mcTestObj, "Test Component (MC-derived)", "f");
        if (!mcTestObj && mcTestLineObj) leg->AddEntry(mcTestLineObj, "Test Component (MC-derived)", "l");

        // Add fractions summary using final fit values when available
        double nP = getFinalYield("n_prompt");
        double nNP = getFinalYield("n_nonprompt");
        double nT = getFinalYield("n_test");
        double sumN = nP + nNP + nT;
        double fP = (sumN > 0) ? (nP / sumN) : 0.0;
        double fNP = (sumN > 0) ? (nNP / sumN) : 0.0;
        double fT = (sumN > 0) ? (nT / sumN) : 0.0;
        leg->AddEntry(myObject, Form("Prompt Fraction = %.2f", fP));
        leg->AddEntry(myObject, Form("NonPrompt Fraction = %.2f", fNP));
        if (hasTestComponent()) {
            leg->AddEntry(myObject, Form("Test Fraction = %.2f", fT));
        }
    } else {
        TObject* dataOrigObj = frame->findObject("data_original");
        TObject* modelFitToDataObj = frame->findObject("model_fit_to_data");
        TObject* promptCompObj = frame->findObject("prompt_comp");
        TObject* nonPromptCompObj = frame->findObject("nonprompt_comp");
        TObject* testCompObj = frame->findObject("test_comp");
        TObject* testLineObj = frame->findObject("test_line");
        TObject* myObject = nullptr;

        if (dataOrigObj) leg->AddEntry(dataOrigObj, "Data", "pe");
        if (modelFitToDataObj) leg->AddEntry(modelFitToDataObj, "Total Fit", "l");
        if (promptCompObj) leg->AddEntry(promptCompObj, "Prompt D^{*} (MC)", "f");
        if (nonPromptCompObj) leg->AddEntry(nonPromptCompObj, "Non-Prompt D^{*} (MC)", "f");
        if (testCompObj) leg->AddEntry(testCompObj, "Test Component (MC-derived)", "f");
        if (!testCompObj && testLineObj) leg->AddEntry(testLineObj, "Test Component (MC-derived)", "l");

        double nP = getFinalYield("n_prompt");
        double nNP = getFinalYield("n_nonprompt");
        double nT = getFinalYield("n_test");
        double sumN = nP + nNP + nT;
        double fP = (sumN > 0) ? (nP / sumN) : 0.0;
        double fNP = (sumN > 0) ? (nNP / sumN) : 0.0;
        double fT = (sumN > 0) ? (nT / sumN) : 0.0;
        leg->AddEntry(myObject, Form("Prompt Fraction = %.2f", fP));
        leg->AddEntry(myObject, Form("NonPrompt Fraction = %.2f", fNP));
        if (hasTestComponent()) leg->AddEntry(myObject, Form("Test Fraction = %.2f", fT));
    }
    leg->Draw();

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.045);
    latex.DrawLatex(0.18, 0.93, "#bf{CMS} #it{Internal}");
    latex.SetTextSize(0.04);
    latex.SetTextAlign(31); // Align right
    latex.DrawLatex(0.94, 0.93, "PP #sqrt{s_{NN}} = 5.32 TeV"); // 예제 그림의 텍스트

    latex.SetTextAlign(11); // Align left
    latex.SetTextSize(0.035);
    // latex.DrawLatex(0.55,0.50, Form("Prompt Fraction = %.2f", fracPrompt_->getVal()));
    // latex.DrawLatex(0.55, 0.67, opt_.pTLegend.empty() ? Form("%.1f < p_{T} < %.1f GeV/c", opt_.pTMin, opt_.pTMax) : opt_.pTLegend.c_str());
    // latex.DrawLatex(0.55, 0.62, opt_.centLegend.empty() ? Form("%.1f < cos#Theta_{HX} < %.1f", opt_.cosMin, opt_.cosMax) : opt_.centLegend.c_str());
    // latex.DrawLatex(0.55, 0.57, opt_.etaLegend.empty() ? Form("|y| < 1", opt_.etaMax) : opt_.etaLegend.c_str());

    pullPad->cd();
    RooPlot* pullFrame = dca->frame(RooFit::Title(" "));
    
    std::string dataName = useDataTemplates ? "data_yield_hist" : "data_original";
    std::string modelName = useDataTemplates ? "model_fit_to_mc" : "model_fit_to_data";
    
    RooHist* hpull = frame->pullHist(dataName.c_str(), modelName.c_str(), true); // normalized residuals (pulls)
    
    if (hpull) {
        pullFrame->addPlotable(hpull, "P");
        pullFrame->SetMinimum(-3.9);
        pullFrame->SetMaximum(3.9);
        
        pullFrame->GetYaxis()->SetTitle("Pull");
        pullFrame->GetYaxis()->SetTitleSize(0.15);
        pullFrame->GetYaxis()->SetLabelSize(0.12);
        pullFrame->GetYaxis()->SetTitleOffset(0.5);
        pullFrame->GetYaxis()->SetNdivisions(505);
        
        pullFrame->GetXaxis()->SetTitle(Form("%s (cm)", dca->GetTitle()));
        pullFrame->GetXaxis()->SetTitleSize(0.15);
        pullFrame->GetXaxis()->SetLabelSize(0.12);
        pullFrame->GetXaxis()->SetTitleOffset(1.0);
        
        pullFrame->Draw();
        
        TLine* lineAtZero = new TLine(dca->getMin(), 0, dca->getMax(), 0);
        lineAtZero->SetLineColor(kRed);
        lineAtZero->SetLineStyle(kDashed);
        lineAtZero->SetLineWidth(1);
        lineAtZero->Draw("same");
        
        TLine* lineAtPlus3 = new TLine(dca->getMin(), 3, dca->getMax(), 3);
        lineAtPlus3->SetLineColor(kGray);
        lineAtPlus3->SetLineStyle(kDotted);
        lineAtPlus3->Draw("same");
        
        TLine* lineAtMinus3 = new TLine(dca->getMin(), -3, dca->getMax(), -3);
        lineAtMinus3->SetLineColor(kGray);
        lineAtMinus3->SetLineStyle(kDotted);
        lineAtMinus3->Draw("same");
    } else {
        LogWarningMsg("Could not create pull histogram");
        pullFrame->SetMinimum(-3.9);
        pullFrame->SetMaximum(3.9);
        pullFrame->GetYaxis()->SetTitle("Pull");
        pullFrame->GetYaxis()->SetTitleSize(0.15);
        pullFrame->GetYaxis()->SetLabelSize(0.12);
        pullFrame->GetYaxis()->SetTitleOffset(0.5);
        pullFrame->GetXaxis()->SetTitle(Form("%s (cm)", dca->GetTitle()));
        pullFrame->GetXaxis()->SetTitleSize(0.15);
        pullFrame->GetXaxis()->SetLabelSize(0.12);
        pullFrame->Draw();
    }

    // Ratio 그리기
    ratioPad->cd();
    RooPlot* ratioFrame = dca->frame(RooFit::Title(" "));

    // Get the histogram for data and the curve for the fit
    RooHist* h_data = (RooHist*)frame->findObject(dataName.c_str());
    RooCurve* c_fit = (RooCurve*)frame->findObject(modelName.c_str());

    if (h_data && c_fit) {
        TGraphAsymmErrors* g_ratio = new TGraphAsymmErrors(h_data->GetN());
        g_ratio->SetName("ratio");

        for (int i = 0; i < h_data->GetN(); ++i) {
            double x, y;
            h_data->GetPoint(i, x, y);
            double ey_low = h_data->GetErrorYlow(i);
            double ey_high = h_data->GetErrorYhigh(i);
            double ex = h_data->GetErrorX(i);

            double fit_val = c_fit->Eval(x);

            if (fit_val > 1e-9) { // Avoid division by zero or very small numbers
                g_ratio->SetPoint(i, x, y / fit_val);
                g_ratio->SetPointError(i, ex, ex, ey_low / fit_val, ey_high / fit_val);
            } else {
                g_ratio->SetPoint(i, x, 0);
                g_ratio->SetPointError(i, ex, ex, 0, 0);
            }
        }
        
        g_ratio->SetMarkerStyle(20);
        g_ratio->SetMarkerSize(0.8);
        g_ratio->SetMarkerColor(kBlack);
        g_ratio->SetLineColor(kBlack);

        ratioFrame->addObject(g_ratio, "P");
        ratioFrame->SetMinimum(0);
        ratioFrame->SetMaximum(2);

        // Style the ratio plot
        ratioFrame->GetYaxis()->SetTitle("Data / Fit");
        ratioFrame->GetYaxis()->SetTitleSize(0.15);
        ratioFrame->GetYaxis()->SetLabelSize(0.12);
        ratioFrame->GetYaxis()->SetTitleOffset(0.5);
        ratioFrame->GetYaxis()->SetNdivisions(505);

        ratioFrame->GetXaxis()->SetTitle(Form("%s (cm)", dca->GetTitle()));
        ratioFrame->GetXaxis()->SetTitleSize(0.15);
        ratioFrame->GetXaxis()->SetLabelSize(0.12);
        ratioFrame->GetXaxis()->SetTitleOffset(1.0);

        ratioFrame->Draw();

        // Draw a line at 1
        TLine* lineAtOne = new TLine(dca->getMin(), 1.0, dca->getMax(), 1.0);
        lineAtOne->SetLineColor(kRed);
        lineAtOne->SetLineStyle(kDashed);
        lineAtOne->SetLineWidth(2);
        lineAtOne->Draw("same");
    } else {
        LogWarningMsg("Could not create ratio plot. Data or fit curve not found.");
        // Draw an empty frame if ratio cannot be calculated
        ratioFrame->SetMinimum(0.5);
        ratioFrame->SetMaximum(1.5);
        ratioFrame->GetYaxis()->SetTitle("Data / Fit");
        ratioFrame->GetYaxis()->SetTitleSize(0.15);
        ratioFrame->GetYaxis()->SetLabelSize(0.12);
        ratioFrame->GetYaxis()->SetTitleOffset(0.5);
        ratioFrame->GetXaxis()->SetTitle(Form("%s (cm)", dca->GetTitle()));
        ratioFrame->GetXaxis()->SetTitleSize(0.15);
        ratioFrame->GetXaxis()->SetLabelSize(0.12);
        ratioFrame->Draw();
    }

    // 파일 저장
    c->SaveAs((plotName + ".png").c_str());
    c->SaveAs((plotName + ".pdf").c_str());

    if (!extraOutputDir.empty()) {
        if (EnsureDirectoryReady(extraOutputDir)) {
            std::string baseName = plotName;
            auto pos = baseName.find_last_of("/\\");
            if (pos != std::string::npos) {
                baseName = baseName.substr(pos + 1);
            }
            std::string extraBase = joinPath(extraOutputDir, baseName);
            c->SaveAs((extraBase + ".png").c_str());
            c->SaveAs((extraBase + ".pdf").c_str());
        }
    }
    
    if (!outputFileName_.empty()) {
        TFile* fout = outFile_.get();
        bool closeFileAfterWrite = false;
        if (!fout || !fout->IsOpen()) {
            fout = TFile::Open(outputFileName_.c_str(), "UPDATE");
            closeFileAfterWrite = true;
        }
        if (fout && fout->IsOpen()) {
            fout->cd();
            c->Write((name_ + "_" + plotName).c_str(), TObject::kOverwrite);
            if (closeFileAfterWrite) {
                fout->Close();
                if (outFile_.get() == fout) outFile_.reset();
            }
        }
    }

    delete c;
    delete frame;
    delete pullFrame;
    delete leg;
}

    void saveResults(RooFitResult* fitResult) {
        LogInfoMsg("Saving results to ", outputFileName_);
         if (outputFileName_.empty()) {
            LogErrorMsg("Output file name not set. Cannot save results.");
             return;
         }

         // Ensure the file is open and writable, prefer RECREATE for final save
         if (outFile_ && outFile_->IsOpen()) {
             if (!outFile_->IsWritable()) {
                 std::string currentFileName = outFile_->GetName();
                 outFile_.reset(); 
                 outFile_.reset(TFile::Open(currentFileName.c_str(), "RECREATE"));
             } else {
                 // If already open and writable (e.g., from plotting), ensure we are in the root directory
                 outFile_->cd();
             }
         } else {
             // If not open, open in RECREATE mode
             outFile_.reset(); 
             outFile_.reset(TFile::Open(outputFileName_.c_str(), "RECREATE"));
         }


         if (!outFile_ || !outFile_->IsOpen() || !outFile_->IsWritable()) {
            LogErrorMsg("Could not open or write to output file: ", outputFileName_);
             // Clean up potential zombie file object
             outFile_.reset(); 
             return;
         }
            TParameter<double>* PromptFracValParam = new TParameter<double>("PromptFraction", fracPrompt_->getVal());
            TParameter<double>* PromptFracErrParam = new TParameter<double>("PromptFractionError", fracPrompt_->getPropagatedError(*fitResult));
            PromptFracValParam->Write();
            PromptFracErrParam->Write();
            delete PromptFracValParam;
            delete PromptFracErrParam;

         // Write workspace
         if (ws_) {
             ws_->Write();
            LogInfoMsg("Workspace saved.");
         }

         // Write fit result
         if (fitResult) {
             fitResult->Write("fitResult");
            LogInfoMsg("Fit result saved.");
         }

         // Note: Plots are saved in their respective functions.

         // Close the file now that everything is saved
         outFile_->Close();
        //  delete outFile_;
         outFile_ = nullptr; // Reset pointer after closing and deleting
        LogInfoMsg("Results saved and file closed.");
    }

    // --- New method to plot raw data distribution ---
    void plotRawDataDistribution(const std::string& plotName = "dca_raw_data_plot");


private:
    // --- Internal Helper Methods ---
    // loadDataSet helper is integrated into createTemplatesFromMC and loadData for clarity
    void addFractionSummaryToLegend(TLegend* leg) const;
    RooAbsPdf* ExtractComponent(RooAbsPdf* pdf_, const std::string& namePattern) {
    // if (std::string(pdf_->GetName()).find(namePattern) != std::string::npos) {
    // }

    const RooArgSet* components = pdf_->getComponents();
    RooAbsPdf* foundPdf = nullptr;

    if (components) {
        for (const auto& obj : *components) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(obj);
                LogInfoMsg("Found component '", namePattern, "': ", pdfComponent->GetName());
            if (pdfComponent && std::string(pdfComponent->GetName()).find(namePattern) != std::string::npos) {
                foundPdf = pdfComponent;
                // std::cout << "Found component '" << namePattern << "': " << foundPdf->GetName() << std::endl;
                break;
            }
        }
    }

    if (!foundPdf) {
         LogWarningMsg("Component containing '", namePattern, "' not found.");
    }

    return foundPdf;
}
        void plotSliceFitDetails(
        RooDataSet* sliceData,
        RooWorkspace* fitWs, // Workspace from MassFitter for this slice
        RooFitResult* sliceFitResult,
        RooRealVar* massVar, // The mass variable (e.g., DCAFitter::massVar_)
        const std::string& sliceName,
        const FitOpt& fitOptForSlice, // The FitOpt used by MassFitter for this slice
        const std::string& outputDir,
        bool isMC,
        const std::string& particleType, // e.g., "D^{*+}", "D^{0}"
        const std::string& energyStr,    // e.g., "pp #sqrt{s_{NN}} = 5.36 TeV"
        const std::string& ptRangeStr,   // e.g., "10.0 < p_{T} < 20.0 GeV/c"
        const std::string& yRangeStr,    // e.g., "|y| < 1.0"
        const std::string& analysisCutStr, // e.g., MVA cut or cos(theta) cut
        const std::string& dcaRangeStr,     // e.g., "0.002 < DCA < 0.004 cm"
         double signalYield = 0.0,
        double signalYieldError = 0.0,
        RooDataSet* sliceMCData = nullptr,  // Optional: MC data for the same slice
        RooWorkspace* mcFitWs = nullptr,    // Optional: MC fit workspace
        RooFitResult* mcFitResult = nullptr // Optional: MC fit result
        
    ) {
        (void)signalYield;
        (void)signalYieldError;
        (void)sliceMCData;
        (void)mcFitWs;
        (void)mcFitResult;

        if (!sliceData) {
            LogErrorMsg("Slice data not provided for slice ", sliceName);
            return;
        }
        if (!fitWs) {
            LogErrorMsg("Workspace not provided for slice ", sliceName);
            return;
        }
        if (!sliceFitResult) {
            LogErrorMsg("Fit result not provided for slice ", sliceName);
            return;
        }

        RooAbsPdf* sliceModel = fitWs->pdf("totalPdf");
        if (!sliceModel) sliceModel = fitWs->pdf("total_pdf");
        if (!sliceModel) sliceModel = fitWs->pdf("pdf");
        if (!sliceModel) sliceModel = fitWs->pdf("model");
        if (!sliceModel && isMC) {
            sliceModel = fitWs->pdf("mcSignal");
            if (!sliceModel) sliceModel = fitWs->pdf("signal");
        }
        if (!sliceModel) {
            LogWarningMsg("Could not locate PDF in workspace for slice ", sliceName);
        }

        FitOpt plotOpt = fitOptForSlice;
        plotOpt.outputDir = outputDir;
        plotOpt.ELabel = energyStr;
        plotOpt.pTLegend = ptRangeStr;
        plotOpt.yLegend = yRangeStr;
        plotOpt.cosLegend = analysisCutStr;
        plotOpt.dcaLegend = dcaRangeStr;
        plotOpt.drawMvaLegend = true;
        plotOpt.drawDcaLegend = true;
        plotOpt.isPP = opt_.isPP;

        bool treatAsDstar = (particleType.find("D^{0}") == std::string::npos);

        EnhancedPlotManager plotManager(plotOpt, fitWs, sliceData, sliceFitResult,
                                        outputDir, plotOpt.isPP, treatAsDstar, massVar, sliceModel, false);

        if (!plotManager.IsValid()) {
            LogErrorMsg("EnhancedPlotManager initialization failed for slice ", sliceName);
            return;
        }

        std::string outputName = "mass_fit_detailed_" + sliceName + "_" + std::to_string(isMC) + ".png";
        if (plotManager.DrawFittedModel(true, outputName, true)) {
            LogInfoMsg("Saved detailed mass fit for slice ", sliceName, " to ", outputDir + "/" + outputName);
        } else {
            LogErrorMsg("Failed to draw mass fit plot for slice ", sliceName);
        }

        std::string outputNameNoPad = "mass_fit_detailed_noParamPad_" + sliceName + "_" + std::to_string(isMC) + ".png";
        if (plotManager.DrawFittedModel(true, outputNameNoPad, false)) {
            LogInfoMsg("Saved compact mass fit (no parameter pad) for slice ", sliceName,
                       " to ", outputDir + "/" + outputNameNoPad);
        } else {
            LogWarningMsg("Failed to draw compact mass fit (no parameter pad) for slice ", sliceName);
        }
    }

    // --- Member Variables ---
    std::string name_;
    RooWorkspacePtr ws_; // RooFit workspace
    std::vector<double> dcaBins_ = {0,0.001,0.0023,0.0039,0.0059,0.0085,0.0160,0.0281,0.0476,0.1};
    // vector<double> dcaBins_ = {0,0.1};
    FitOpt opt_;
    std::string latestDetailedPlotDir_;

    // Variables (using smart pointers for better memory management)
    RooRealVarPtr dcaVar_;    // The DCA variable
    RooRealVarPtr massVar_; // The mass variable
    RooRealVarPtr weights_;   // Optional global weights variable
    RooRealVarPtr n_prompt_; // Number of prompt events
    RooRealVarPtr n_nonprompt_; // Number of non-prompt events


    // Configuration
    std::string mcFileName_;
    std::string mcTreeName_;
    std::string mcRooDatsetName_;
    // MC result (workspace) inputs
    std::string mcResultFileName_;
    std::string mcWorkspaceName_ = "workspace";
    std::string mcResultDatasetName_;
    std::string dataFileName_;
    std::string dataTreeName_;
    std::string dataRooDatsetName_;
    std::string dcaBranchName_;
    // Mass-fit result input (workspace-based)
    std::string dataResultFileName_;
    std::string dataWorkspaceName_ = "workspace";
    std::string dataResultDatasetName_;
    std::string motherPdgIdBranchName_;
    std::string weightBranchName_; // Name of the weight branch in Trees
    std::vector<int> promptPdgIds_;
    std::vector<int> nonpromptPdgIds_;
    std::string mcCuts_;
    std::string dataCuts_;
    std::string outputFileName_;
    bool runMassFits_ = true;
    std::string dataYieldHistInputFile_;
    std::string dataYieldHistInputHist_ = "dataYieldHist";
    std::string dataYieldHistOutputFile_;
    std::string dataYieldHistOutputHist_ = "dataYieldHist";
    static constexpr double deltaM_PDG = 0.1454258; 
    static constexpr double D0_PDG = 1.86483; 

    // Datasets and Templates (managed by smart pointers and workspace)
    RooDataSet* dataSet_;           // Data to be fitted (external reference)
    RooDataSetPtr fullMCDataSet; // Full MC dataset
    RooDataHist* promptTemplate_;   // Prompt MC template (workspace managed)
    TH1* promptHist_; // Histogram for prompt template (manually managed)
    TH1* nonPromptHist_; // Histogram for non-prompt template (manually managed)
    RooDataHist* nonPromptTemplate_; // Non-prompt MC template (workspace managed)
    // Test (intermediate-slope) template derived from non-prompt
    RooDataHist* testTemplate_ = nullptr; // Test template (workspace managed)
    std::string massVarName_;
    double massMin_;
    double massMax_;
    std::string massUnit_;


    // PDFs and Model (managed by smart pointers and workspace)
    RooHistPdf* promptPdf_;         // PDF from prompt template (workspace managed)
    RooHistPdf* nonPromptPdf_;      // PDF from non-prompt template (workspace managed)  
    RooHistPdf* testPdf_ = nullptr; // PDF from test template (workspace managed)
    RooRealVar* coef_prompt_;
    RooRealVar* coef_nonprompt_;
    RooHistFunc* promptFunc_ = nullptr;         // PDF from prompt template (workspace managed)
    RooHistFunc* nonPromptFunc_= nullptr;      // PDF from non-prompt template (workspace managed)  
    RooRealSumFunc *sumFunc_;
    RooFormulaVar* fracPrompt_;        // Fraction of prompt component (workspace managed)
    std::unique_ptr<RooRealVar> n_test_; // Number of test-component events
    RooAddPdf* model_;              // Combined model (workspace managed)

    // Parameters
    double dcaMin_;
    double dcaMax_;
    int nBins_;
    TH1DPtr dataYieldHist_;
    TH1DPtr sigYieldHist_;
    TH1DPtr sbYieldHist_;
    RooDataHist* dataDrivenTemplate_ = nullptr;
    RooDataHist* sigDrivenTemplate_ = nullptr;
    RooDataHist* sbDrivenTemplate_ = nullptr;
    RooRealVarPtr n_totalData_;

    // ROOT Objects (using smart pointers for better memory management)
    TFilePtr outFile_;
    
    // --- Enhanced MassFitterV2 Integration Members ---
    
    // Configuration and dependency injection
    std::unique_ptr<ConfigManager> configManager_;
    std::unique_ptr<class DCAErrorHandler> errorHandler_;

    // Optional third component control
    bool enableTestComponent_ = false;
    double testComponentPower_ = 0.7; // y_new = y_old^power (renormalized)
    bool   testUseSlope_ = false;       // if true, apply linear slope scaling to template
    double testSlope_ = 0.0;            // slope parameter for linear scaling
    bool   testBasePrompt_ = true;      // true: base from prompt, false: base from non-prompt
    
    // Result storage
    std::map<std::string, FitResults*> sliceFitResults_;
    std::map<std::string, std::unique_ptr<MassFitterV2>> sliceFitters_;
    
    // MC dataset for constraint fitting
    RooDataSet* mcDataSet_ = nullptr;

    void Clear() {
        LogInfoMsg("Clearing DCAFitter state...");
        
        dcaVar_.reset();
        massVar_.reset();
        weights_.reset();
        n_prompt_.reset();
        n_nonprompt_.reset();
        n_totalData_.reset();
        fullMCDataSet.reset();
        dataYieldHist_.reset();
        sigYieldHist_.reset();
        sbYieldHist_.reset();
        outFile_.reset();
        
        ws_ = std::make_unique<RooWorkspace>(Form("ws_%s", opt_.name.c_str()), (name_ + " Workspace").c_str());
        
        dataSet_ = nullptr;
        promptTemplate_ = nullptr;
        nonPromptTemplate_ = nullptr;
        promptPdf_ = nullptr;
        nonPromptPdf_ = nullptr;
        fracPrompt_ = nullptr;
        model_ = nullptr;
        dataDrivenTemplate_ = nullptr;
        sigDrivenTemplate_ = nullptr;
        sbDrivenTemplate_ = nullptr;
        
        delete promptHist_;
        delete nonPromptHist_;
        promptHist_ = nullptr;
        nonPromptHist_ = nullptr;
    }

    // Const getter methods for better encapsulation
    const std::string& getName() const { return name_; }
    double getDCAMin() const { return dcaMin_; }
    double getDCAMax() const { return dcaMax_; }
    int getNBins() const { return nBins_; }
    const std::vector<double>& getDCABins() const { return dcaBins_; }
    const std::string& getMCFileName() const { return mcFileName_; }
    const std::string& getDataFileName() const { return dataFileName_; }
    const std::string& getOutputFileName() const { return outputFileName_; }
    static constexpr double getDeltaMPDG() { return deltaM_PDG; }
    
    // Check if objects are properly initialized
    bool isInitialized() const {
        return dcaVar_ && massVar_ && ws_;
    }
    
    bool hasData() const {
        return dataSet_ != nullptr;
    }
    
    bool hasTemplates() const {
        return promptTemplate_ != nullptr && nonPromptTemplate_ != nullptr;
    }
    
    // --- Enhanced MassFitterV2 Helper Methods ---
    
    // Validation and initialization helpers
    void validateConstructorParameters(const std::string& name, const std::string& massVarName,
                                     double dcaMin, double dcaMax, int nBins) {
        if (name.empty()) {
            throw std::invalid_argument("DCAFitter name cannot be empty");
        }
        if (dcaMin >= dcaMax) {
            throw std::invalid_argument("Invalid DCA range: dcaMin >= dcaMax");
        }
        if (nBins <= 0) {
            throw std::invalid_argument("Number of bins must be positive");
        }
        if (massVarName.empty()) {
            throw std::invalid_argument("Mass variable name cannot be empty");
        }
    }
    
    void initializeWorkspace() {
        ws_ = std::make_unique<RooWorkspace>(Form("ws_%s", opt_.name.c_str()), 
                                           (name_ + " Enhanced Workspace").c_str());
    }
    
    void initializeVariables() {
        dcaVar_ = std::make_unique<RooRealVar>(opt_.dcaVar.c_str(), 
                                             "Distance of Closest Approach", 
                                             opt_.dcaMin, opt_.dcaMax, "cm");
        massVar_ = std::make_unique<RooRealVar>(opt_.massVar.c_str(), 
                                              "Mass Variable", 
                                              opt_.massMin, opt_.massMax, "GeV/c^{2}");
        
        // Enhanced error handling for workspace import
        if (ws_->import(*dcaVar_) != 0) {
            throw std::runtime_error("Failed to import DCA variable to workspace");
        }
        if (ws_->import(*massVar_) != 0) {
            throw std::runtime_error("Failed to import mass variable to workspace");
        }
    }
    
    bool validateSetup() const {
        return dcaVar_ && massVar_ && ws_ && configManager_ && errorHandler_;
    }
    
    // MassFitterV2 creation and management
    MassFitterV2* createMassFitterV2ForSlice(const std::string& sliceName) {
        std::string fitterName = opt_.name + "_" + sliceName;
        
        // Disambiguate ctor: use ratio-based constructor. Store in map to keep lifetime.
        sliceFitters_[sliceName] = std::make_unique<MassFitterV2>(
            fitterName,
            massVar_->GetName(),
            massVar_->getMin(),
            massVar_->getMax(),
            /*nsigRatio*/0.5, /*nsigMin*/0.0, /*nsigMax*/1.0,
            /*nbkgRatio*/0.2, /*nbkgMin*/0.0, /*nbkgMax*/1.0,
            MassFitterV2::YieldMode::Independent
        );

        // Ensure independent yields even if constructor defaults change
        sliceFitters_[sliceName]->UseIndependentYields(true);

        // Return non-owning pointer; ownership stays in sliceFitters_
        return sliceFitters_[sliceName].get();
    }
    
    static std::string joinPath(const std::string& base, const std::string& part) {
        if (base.empty()) return part;
        if (part.empty()) return base;
        const bool baseHasSlash = base.back() == '/';
        const bool partHasSlash = part.front() == '/';
        if (baseHasSlash && partHasSlash) {
            return base + part.substr(1);
        }
        if (baseHasSlash || partHasSlash) {
            return base + part;
        }
        return base + "/" + part;
    }

    FitOpt createMassFitOpt(const std::string& sliceName,
                            double dcaLow,
                            double dcaHigh,
                            bool isMC = false) const {
        FitOpt massOpt = opt_;

        massOpt.name = opt_.name + "_" + sliceName + (isMC ? "_MC" : "");
        massOpt.dcaMin = dcaLow;
        massOpt.dcaMax = dcaHigh;
        massOpt.dcaLegend = Form("%.3f < DCA < %.3f cm", dcaLow, dcaHigh);
        massOpt.GenerateLegends();

        std::string outputDir = joinPath(opt_.outputDir, opt_.subDir);
        outputDir = joinPath(outputDir, isMC ? "MC" : "Data");
        outputDir = joinPath(outputDir, "mass_fits_" + name_);
        massOpt.outputDir = outputDir;
        massOpt.subDir = "/slice_mass_distributions";

        std::string baseLabel = (isMC ? "MC_" : "") + sliceName;
        massOpt.outputFile = "mass_fit_" + baseLabel + ".root";
        massOpt.plotName = "fitted_" + baseLabel;
        massOpt.plotMCName = massOpt.plotName;
        massOpt.fitResultName = "massFit_" + baseLabel;
        massOpt.wsName = "workspace_" + massOpt.name;

        return massOpt;
    }

    std::vector<std::string> buildGaussianConstraintParameterList() const {
        std::vector<std::string> params;
        std::unordered_set<std::string> seen;

        static const std::unordered_set<std::string> allowedPrefixes = {
            "mean", "mean1", "mean2",
            "sigma", "sigma1", "sigma2",
            "sigmaL", "sigmaL1", "sigmaL2",
            "sigmaR", "sigmaR1", "sigmaR2",
            "alpha", "alphaL", "alphaR",
            "n", "nL", "nR",
            "fraction", "width",
            "m", "lambda"
        };

        for (const auto& entry : massParamFixedInfo_.fixedFlags) {
            if (!entry.second) continue;
            std::string key = entry.first;
            auto pos = key.find('_');
            if (pos != std::string::npos && pos > 0) {
                key = key.substr(0, pos);
            }
            if (key.empty()) continue;
            if (!allowedPrefixes.empty() && allowedPrefixes.find(key) == allowedPrefixes.end()) {
                continue; // Skip background-specific parameters
            }
            std::string fullName = key + "_signal";
            if (seen.insert(fullName).second) {
                params.push_back(fullName);
            }
        }

        // If nothing was marked as fixed, fall back to defaults (ensuring uniqueness)
        if (params.empty()) {
            for (const auto& def : defaultConstraintParams_) {
                if (def.empty()) continue;
                if (seen.insert(def).second) {
                    params.push_back(def);
                }
            }
        }

        return params;
    }

    std::tuple<double, double, double, double> computeGaussianConstraintSidebandEdges() const {
        double inner = std::min(gaussianConstraintInnerWidth_, gaussianConstraintOuterWidth_);
        double outer = std::max(gaussianConstraintInnerWidth_, gaussianConstraintOuterWidth_);
        double center = gaussianConstraintMassCenter_;
        return std::make_tuple(
            center - outer,  // Lower sideband minimum
            center - inner,  // Lower sideband maximum
            center + inner,  // Upper sideband minimum
            center + outer   // Upper sideband maximum
        );
    }
    
    // Data slicing helper
    std::unique_ptr<RooDataSet> createDCASlice(RooDataSet* fullDataset, 
                                               double dcaMin, double dcaMax) {
        if (!fullDataset || !dcaVar_) {
            return nullptr;
        }        
        std::string cutString = Form("%s >= %f && %s < %f", 
                                   dcaVar_->GetName(), dcaMin, 
                                   dcaVar_->GetName(), dcaMax);
        
        return std::unique_ptr<RooDataSet>(
            static_cast<RooDataSet*>(fullDataset->reduce(cutString.c_str()))
        );
    }

    // --- Extracted loader/builder helpers (Step 3) ---
    // Resolve MC dataset from result workspace or raw file; keep the owning TFile alive via mcFileHandle
    bool TryResolveMCDataset(RooDataSet*& outMcDataset, std::unique_ptr<TFile>& mcFileHandle) {
        outMcDataset = nullptr;
        // Try result workspace first
        if (!mcResultFileName_.empty()) {
            LogInfoMsg("MCResolve: Trying MC result workspace: ", mcResultFileName_);
            mcFileHandle.reset(TFile::Open(mcResultFileName_.c_str(), "READ"));
            if (mcFileHandle && !mcFileHandle->IsZombie()) {
                RooWorkspace* wsmc = dynamic_cast<RooWorkspace*>(mcFileHandle->Get(mcWorkspaceName_.c_str()));
                if (!wsmc) wsmc = dynamic_cast<RooWorkspace*>(mcFileHandle->Get("workspace"));
                if (wsmc) {
                    const char* candNames[] = { mcResultDatasetName_.empty() ? nullptr : mcResultDatasetName_.c_str(),
                                                "dataSet", "datasetHX", "reducedData", nullptr };
                    for (int i=0; i<4; ++i) {
                        const char* nm = candNames[i]; if (!nm) continue;
                        outMcDataset = dynamic_cast<RooDataSet*>(wsmc->data(nm));
                        if (outMcDataset) { LogInfoMsg("MCResolve: Found MC dataset in workspace: ", nm); break; }
                    }
                    if (!outMcDataset) {
                        LogWarningMsg("No suitable MC dataset found in MC workspace; fallback to raw MC file if available");
                        mcFileHandle.reset();
                    }
                } else {
                    LogWarningMsg("No workspace found in MC result file; fallback to raw MC file if available");
                    mcFileHandle.reset();
                }
            } else {
                LogWarningMsg("Could not open MC result file: " + mcResultFileName_);
                mcFileHandle.reset();
            }
        }

        // Fallback to raw MC file if needed
        if (!outMcDataset && !mcFileName_.empty() && !mcRooDatsetName_.empty()) {
            mcFileHandle = openFile(mcFileName_, "READ");
            if (!mcFileHandle) {
                LogErrorMsg("Could not open MC file: " + mcFileName_);
                return false;
            }
            outMcDataset = dynamic_cast<RooDataSet*>(mcFileHandle->Get(mcRooDatsetName_.c_str()));
            if (!outMcDataset) {
                LogErrorMsg("Could not load dataset " + mcRooDatsetName_ + " from MC file");
                return false;
            }
        }

        return outMcDataset != nullptr;
    }

    // Build prompt/non-prompt DCA templates from a resolved MC dataset
    bool BuildTemplatesFromMCDataset(RooDataSet* mcDataset) {
        if (!mcDataset) return false;

        // Ensure DCA branch name is active for loading
        dcaVar_->SetName(dcaBranchName_.c_str());
        RooRealVar motherPdgIdVar(motherPdgIdBranchName_.c_str(), "Mother PDG ID", 0);
        const char* kMatchGenBranch = "matchGEN";

        // Apply kinematic cuts if provided
        RooDataSet* baseDS = mcDataset;
        std::unique_ptr<RooDataSet> tmpReduced;
        if (!mcCuts_.empty()) {
            tmpReduced.reset(dynamic_cast<RooDataSet*>(mcDataset->reduce(mcCuts_.c_str())));
            if (tmpReduced && tmpReduced->numEntries() > 0) {
                baseDS = tmpReduced.get();
                LogInfoMsg("BuildTemplates: Applied MC cuts ('", mcCuts_, "') -> entries: ",
                           std::to_string(baseDS->numEntries()));
            } else {
                LogWarningMsg("MC cuts produced empty/invalid dataset; fallback to full MC dataset");
                baseDS = mcDataset;
            }
        }
        if (!baseDS || baseDS->numEntries() <= 0) {
            LogErrorMsg("Could not obtain base MC dataset after cuts");
            return false;
        }
        LogInfoMsg("BuildTemplates: Loaded ", std::to_string(baseDS->sumEntries()),
                   " MC entries (after cuts, weighted)");

        // Keep the full MC dataset (clone so lifetime is independent from source file)
        const std::string mcWsName = "fullMCDataSet";
        fullMCDataSet = RooDataSetPtr(dynamic_cast<RooDataSet*>(baseDS->Clone(mcWsName.c_str())));
        if (!fullMCDataSet) {
            LogErrorMsg("Failed to clone MC dataset for template building.");
            return false;
        }
        int importStatus = ws_->import(*fullMCDataSet, RooFit::Rename(mcWsName.c_str()), RooFit::RecycleConflictNodes());
        if (importStatus != 0) {
            LogErrorMsg("Failed to import cloned MC dataset into workspace (status=" + std::to_string(importStatus) + ")");
            return false;
        }
        fullMCDataSet->SetName(mcWsName.c_str());
        mcDataSet_ = fullMCDataSet.get();
        mcDataSet_ = fullMCDataSet.get();

        // Build selection expressions
        TString pdgPromptExpr;
        if (!promptPdgIds_.empty()) {
            pdgPromptExpr = "(";
            for (size_t i = 0; i < promptPdgIds_.size(); ++i) {
                pdgPromptExpr += TString::Format("%s==%d", motherPdgIdBranchName_.c_str(), promptPdgIds_[i]);
                if (i < promptPdgIds_.size() - 1) pdgPromptExpr += " || ";
            }
            pdgPromptExpr += ")";
        } else {
            pdgPromptExpr = TString::Format("%s==0", motherPdgIdBranchName_.c_str());
        }
        TString pdgNonPromptExpr;
        if (!nonpromptPdgIds_.empty()) {
            pdgNonPromptExpr = "(";
            for (size_t i = 0; i < nonpromptPdgIds_.size(); ++i) {
                pdgNonPromptExpr += TString::Format("%s==%d", motherPdgIdBranchName_.c_str(), nonpromptPdgIds_[i]);
                if (i < nonpromptPdgIds_.size() - 1) pdgNonPromptExpr += " || ";
            }
            pdgNonPromptExpr += ")";
        }
        TString promptCut = pdgPromptExpr;
        TString nonPromptCut;
        if (!nonpromptPdgIds_.empty()) {
            nonPromptCut = pdgNonPromptExpr + TString::Format(" && %s==1", kMatchGenBranch);
        } else {
            nonPromptCut = "!(" + pdgPromptExpr + ")" + TString::Format(" && %s==1", kMatchGenBranch);
        }
        LogInfoMsg("BuildTemplates: Prompt cut: ", promptCut);
        LogInfoMsg("BuildTemplates: Non-prompt cut: ", nonPromptCut);

        // Final vars for template datasets
        RooArgSet finalVars(*dcaVar_);
        if (weights_) finalVars.add(*weights_);

        std::unique_ptr<RooDataSet> promptDataSet(dynamic_cast<RooDataSet*>(fullMCDataSet->reduce(promptCut.Data())));
        std::unique_ptr<RooDataSet> nonPromptDataSet(dynamic_cast<RooDataSet*>(fullMCDataSet->reduce(nonPromptCut.Data())));
        if (promptDataSet) promptDataSet->SetName("promptDataSet");
        if (nonPromptDataSet) nonPromptDataSet->SetName("nonPromptDataSet");
        LogInfoMsg("BuildTemplates: prompt entries=", std::to_string(promptDataSet ? promptDataSet->numEntries() : -1),
                   ", nonPrompt entries=", std::to_string(nonPromptDataSet ? nonPromptDataSet->numEntries() : -1));
        if (promptDataSet) ws_->import(*promptDataSet);
        if (nonPromptDataSet) ws_->import(*nonPromptDataSet);

        // Fill histograms
        double scale = 1.0; // keep existing scaling behavior
        promptHist_ = new TH1D("promptHist", "Prompt Histogram", dcaBins_.size() - 1, dcaBins_.data());
        promptHist_->Sumw2();
        if (promptDataSet) {
            for (int i = 0; i < promptDataSet->numEntries(); ++i) {
                const RooArgSet* row = promptDataSet->get(i);
                double val = row->getRealValue(dcaVar_->GetName());
                double w = promptDataSet->isWeighted() ? promptDataSet->weight() : 1.0;
                promptHist_->Fill(scale*val, w);
            }
        }
        nonPromptHist_ = new TH1D("nonPromptHist", "Non-prompt Histogram", dcaBins_.size() - 1, dcaBins_.data());
        nonPromptHist_->Sumw2();
        if (nonPromptDataSet) {
            for (int i = 0; i < nonPromptDataSet->numEntries(); ++i) {
                const RooArgSet* row = nonPromptDataSet->get(i);
                double val = row->getRealValue(dcaVar_->GetName());
                double w = nonPromptDataSet->isWeighted() ? nonPromptDataSet->weight() : 1.0;
                nonPromptHist_->Fill(scale*val, w);
            }
        }

        // Create RooDataHist templates and import
        if (!promptDataSet || promptDataSet->numEntries() == 0) {
            LogWarningMsg("No prompt MC events found after selection");
        } else {
            LogInfoMsg("BuildTemplates: Prompt MC entries (weighted): ",
                       std::to_string(promptDataSet->sumEntries()));
            promptTemplate_ = new RooDataHist("promptTemplate", "Prompt MC Template", RooArgSet(*dcaVar_), promptHist_);
            ws_->import(*promptTemplate_);
            LogInfoMsg("BuildTemplates: Prompt template created");
        }

        if (!nonPromptDataSet || nonPromptDataSet->numEntries() == 0) {
            LogWarningMsg("No non-prompt MC events found after selection; proceeding with prompt-only fitting");
            nonPromptTemplate_ = nullptr;
            nonPromptHist_ = nullptr;
        } else {
            LogInfoMsg("BuildTemplates: Non-prompt MC entries (weighted): ",
                       std::to_string(nonPromptDataSet->sumEntries()));
            nonPromptTemplate_ = new RooDataHist("nonPromptTemplate", "Non-prompt MC Template", RooArgSet(*dcaVar_), nonPromptHist_);
            ws_->import(*nonPromptTemplate_);
            LogInfoMsg("BuildTemplates: Non-prompt template created");
        }

        return (promptTemplate_ != nullptr || nonPromptTemplate_ != nullptr);
    }
    
    // Result management helpers
    void updateYieldHistogram(size_t binIndex, double yield, double yieldError) {
        if (dataYieldHist_ && binIndex < dcaBins_.size() - 1) {
            double binCenter = (dcaBins_[binIndex] + dcaBins_[binIndex + 1]) / 2.0;
            int histBin = dataYieldHist_->FindBin(binCenter);
            dataYieldHist_->SetBinContent(histBin, yield);
            dataYieldHist_->SetBinError(histBin, yieldError);
        }
    }

    template<typename T>
    static auto safeGetMeanImpl(const T& params, int) -> decltype(params.mean, double()) { return params.mean; }
    template<typename T>
    static double safeGetMeanImpl(const T&, long) { return std::numeric_limits<double>::quiet_NaN(); }
    template<typename T>
    static double safeGetMean(const T& params) { return safeGetMeanImpl(params, 0); }

    template<typename T>
    static auto safeGetSigmaImpl(const T& params, int) -> decltype(params.sigma, double()) { return params.sigma; }
    template<typename T>
    static double safeGetSigmaImpl(const T&, long) { return std::numeric_limits<double>::quiet_NaN(); }
    template<typename T>
    static double safeGetSigma(const T& params) { return safeGetSigmaImpl(params, 0); }

    template<typename T>
    static auto safeGetSigmaLImpl(const T& params, int) -> decltype(params.sigmaL, double()) { return params.sigmaL; }
    template<typename T>
    static double safeGetSigmaLImpl(const T&, long) { return std::numeric_limits<double>::quiet_NaN(); }
    template<typename T>
    static double safeGetSigmaL(const T& params) { return safeGetSigmaLImpl(params, 0); }

    template<typename T>
    static auto safeGetLambdaImpl(const T& params, int) -> decltype(params.lambda, double()) { return params.lambda; }
    template<typename T>
    static double safeGetLambdaImpl(const T&, long) { return std::numeric_limits<double>::quiet_NaN(); }
    template<typename T>
    static double safeGetLambda(const T& params) { return safeGetLambdaImpl(params, 0); }
private:
    // Optional JSON-based parameter source for mass fits
    std::unique_ptr<JSONParameterLoader> jsonLoader_;
    bool useJSONParams_ = false;
    ParameterFixedInfo massParamFixedInfo_;
    double gaussianConstraintMassCenter_ = deltaM_PDG;
    double gaussianConstraintInnerWidth_ = 0.002;   // inner half-width around the pole mass (GeV)
    double gaussianConstraintOuterWidth_ = 0.004;   // outer half-width around the pole mass (GeV)
    double gaussianConstraintSignalScale_ = 2.0;
    double gaussianConstraintBackgroundScale_ = 2.0;
    std::vector<std::string> defaultConstraintParams_ = {
        "mean_signal",
        // "nL_signal",
        // "nR_signal",
    };
};
inline bool DCAFitter::GenerateTemplatesWithScale(double scale,
                                                  std::unique_ptr<TH1D>& promptOut,
                                                  std::unique_ptr<TH1D>& nonPromptOut) const {
    if (!ws_) {
        LogErrorMsg("[DCAFitter] Workspace is not available. Cannot generate scaled templates.");
        return false;
    }
    if (dcaBins_.size() < 2) {
        LogErrorMsg("[DCAFitter] DCA binning is not configured.");
        return false;
    }

    const char* dcaName = dcaVar_ ? dcaVar_->GetName() : opt_.dcaVar.c_str();
    RooDataSet* promptDataSetWS = dynamic_cast<RooDataSet*>(ws_->data("promptDataSet"));
    RooDataSet* nonPromptDataSetWS = dynamic_cast<RooDataSet*>(ws_->data("nonPromptDataSet"));
    if (!promptDataSetWS || !nonPromptDataSetWS) {
        LogErrorMsg("[DCAFitter] Prompt/non-prompt datasets not found in workspace. Please run createTemplatesFromMC() beforehand.");
        return false;
    }

    auto makeHist = [&](RooDataSet* ds, const std::string& name) -> std::unique_ptr<TH1D> {
        auto hist = std::make_unique<TH1D>(name.c_str(), name.c_str(),
                                           static_cast<int>(dcaBins_.size()) - 1,
                                           dcaBins_.data());
        hist->Sumw2();
        const int nEntries = ds->numEntries();
        for (int i = 0; i < nEntries; ++i) {
            const RooArgSet* row = ds->get(i);
            if (!row) continue;
            double val = row->getRealValue(dcaName, 0.0);
            double scaled = scale * val;
            if (scaled < dcaBins_.front() || scaled >= dcaBins_.back()) {
                continue;
            }
            double weight = ds->isWeighted() ? ds->weight() : 1.0;
            hist->Fill(scaled, weight);
        }
        return hist;
    };

    promptOut = makeHist(promptDataSetWS, Form("promptScaled_%0.2f", scale));
    nonPromptOut = makeHist(nonPromptDataSetWS, Form("nonPromptScaled_%0.2f", scale));
    return true;
}

inline void DCAFitter::plotSignalAndSidebandDCAFromHist(const std::string& plotName) {
    LogInfoMsg("Using existing histograms to plot DCA distribution for signal and sideband regions...");
    RooRealVar* dca = ws_->var(opt_.dcaVar.c_str()); 
    if (!dca) {
        LogErrorMsg("dca3D variable not found in workspace.");
        return;
    }
    gStyle->SetOptStat(0); 

    if (!sigDrivenTemplate_) {
        LogErrorMsg("Signal yield histogram 'sigDrivenTemplate_' is not available.");
        return;
    }
    if (!sbYieldHist_) {
        LogErrorMsg("Sideband yield histogram 'sbYieldHist_' is not available.");
        return;
    }
    // RooPlot* frame = dca->frame(RooFit::Title("DCA Distribution for Signal and Sideband Regions"));
    // auto frame = std::make_unique<RooPlot>(dca->frame(RooFit::Title("DCA Distribution for Signal and Sideband Regions")));
    RooPlot* frame = dca->frame(RooFit::Title(" "));
    // auto c = std::make_unique<TCanvas>(plotName.c_str(), "DCA Signal vs Sideband (from Histograms)", 800, 700);
    RooBinning customBinning(dcaBins_.size() - 1, dcaBins_.data());
    RooLinkedList sbPlotOpts_line;
    sbPlotOpts_line.Add(RooFit::Binning(customBinning).Clone());
    sbPlotOpts_line.Add(RooFit::LineWidth(2).Clone());
    sbPlotOpts_line.Add(RooFit::LineColor(kBlue).Clone());
    sbPlotOpts_line.Add(RooFit::MarkerColor(kBlue).Clone());
    sbPlotOpts_line.Add(RooFit::Name("sb_yield_hist").Clone()); //  
    // sbPlotOpts_line.Add(RooFit::DrawOption("HIST").Clone());
    
    sigDrivenTemplate_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_yield_hist"), RooFit::DataError(RooAbsData::SumW2));
    sbDrivenTemplate_->plotOn(frame,sbPlotOpts_line);
    

    TCanvas * c = new TCanvas(" ", " ", 800, 700);
    c->SetLogy();
    c->SetLeftMargin(0.15);
        c->SetBottomMargin(0.13);

    frame->GetYaxis()->SetTitle("counts per cm");
    frame->GetYaxis()->SetTitleOffset(1.4);
    frame->GetXaxis()->SetTitle("D^{0} DCA (cm)");

    frame->Draw();

    // auto leg = std::make_unique<TLegend>(0.55, 0.72, 0.93, 0.88);
    auto leg = new TLegend(0.55, 0.72, 0.93, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.04);
    TObject* dataObj = frame->findObject("data_yield_hist");
    TObject* sbObj = frame->findObject("sb_yield_hist");
    if (dataObj) leg->AddEntry(dataObj, "D^{0} candidate", "pe");
    if (sbObj) leg->AddEntry(sbObj, "Side band", "l"); 
    leg->Draw();

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.045);
    latex.DrawLatex(0.18, 0.93, "#bf{CMS} #it{Preliminary}");
    latex.SetTextSize(0.04);
    latex.SetTextAlign(31); // Align right
    latex.DrawLatex(0.94, 0.93, "PP #sqrt{s_{NN}} = 5.32 TeV"); 

    latex.SetTextAlign(11); // Align left
    latex.SetTextSize(0.035);
    latex.DrawLatex(0.55, 0.67, opt_.pTLegend.empty() ? Form("%.1f < p_{T} < %.1f GeV/c", opt_.pTMin, opt_.pTMax) : opt_.pTLegend.c_str());
    latex.DrawLatex(0.55, 0.62, opt_.centLegend.empty() ? Form("%.1f < cos#Theta_{HX} < %.1f", opt_.cosMin, opt_.cosMax) : opt_.centLegend.c_str());
    latex.DrawLatex(0.55, 0.57, opt_.yLegend.empty() ? Form("|y| < %.1f", opt_.etaMax) : opt_.yLegend.c_str());

    // 5. 플롯 저장
    // std::string plotDir = opt_.outputPlotDir + "/mass_fits_" + name_ + "/slice_mass_distributions/";
    std::string outputPath = opt_.outputPlotDir.empty() ? "" : (opt_.outputPlotDir + "/");
    if (!outputPath.empty() && !ensureDir(outputPath)) {
        ErrorHandlerManager::Instance().LogError(std::string("Cannot create output directory: ") + outputPath,
                                                 std::string("DCAFitter:") + name_);
        delete c;
        return;
    }
    c->SaveAs((outputPath + plotName + ".png").c_str());
    // c->SaveAs((outputPath + plotName + ".pdf").c_str());
}



// --- Implementation of plotRawDataDistribution ---
// This function now plots normalized prompt and non-prompt MC distributions
inline void DCAFitter::plotRawDataDistribution(const std::string& plotName) {
    gStyle->SetOptStat(0); // Disable statistics box

    if (!ws_) {
        LogErrorMsg("Workspace is not available.");
        return;
    }

    RooDataSet* promptDataSet = dynamic_cast<RooDataSet*>(ws_->data("promptDataSet"));
    RooDataSet* nonPromptDataSet = dynamic_cast<RooDataSet*>(ws_->data("nonPromptDataSet"));

    if (!promptDataSet && !nonPromptDataSet) {
        LogErrorMsg("Neither prompt nor non-prompt dataset found in workspace.");
        return;
    }

    if (!dcaVar_) {
        RooRealVar* dcaFromWs = ws_->var(opt_.dcaVar.c_str()); 
        if (!dcaFromWs && !dcaBranchName_.empty()) dcaFromWs = ws_->var(dcaBranchName_.c_str());
        
        if (dcaFromWs) {
            LogWarningMsg("DCA variable found in workspace but dcaVar_ is not set. Using workspace variable.");
        }
    }
    if (!dcaVar_) {
        LogErrorMsg("DCA variable not found.");
        return;
    }

    int nCustomBins = dcaBins_.size() - 1;
    if (nCustomBins <= 0) {
        LogErrorMsg("Invalid binning in dcaBins_.");
        return;
    }
    const double* binArray = dcaBins_.data();

    TH1* hPrompt = nullptr;
    TH1* hNonPrompt = nullptr;
    TH1* hTestTemplate = nullptr;

    if (promptDataSet) {
        hPrompt = new TH1F("hPromptRaw", "MC Prompt D^{0}", nCustomBins, binArray);
        hPrompt->Sumw2();
        for (int i = 0; i < promptDataSet->numEntries(); ++i) {
            const RooArgSet* row = promptDataSet->get(i);
            hPrompt->Fill(row->getRealValue(dcaVar_->GetName()));
        }
        // hprompt->Scale(1.0, "width");
        if (hPrompt->Integral() > 0) {
            hPrompt->Scale(1.0/hPrompt->Integral());
            // hPrompt->Scale(1.0/hPrompt->Integral());
            hPrompt->Scale(1.0, "width");
            // for (int i = 1; i <= hPrompt->GetNbinsX(); ++i) {
            //     double content = hPrompt->GetBinContent(i);
            //     double error = hPrompt->GetBinError(i);
            //     double width = hPrompt->GetBinWidth(i);
            //     if (width > 0) {
            //         hPrompt->SetBinContent(i, content / width);
            //         hPrompt->SetBinError(i, error / width);
            //     } else {
            //         hPrompt->SetBinContent(i, 0);
            //         hPrompt->SetBinError(i, 0);
            //     }
            // }
            hPrompt->SetLineColor(kRed);
            hPrompt->SetMarkerColor(kRed);
            hPrompt->SetMarkerStyle(20); 
            hPrompt->SetMarkerSize(1.0);
            LogInfoMsg("Created and normalized prompt histogram (per cm).");
        } else {
            delete hPrompt; hPrompt = nullptr;
        }
    }

    if (nonPromptDataSet) {
        hNonPrompt = new TH1F("hNonPromptRaw", "MC Non-prompt D^{0}", nCustomBins, binArray);
        hNonPrompt->Sumw2();
        for (int i = 0; i < nonPromptDataSet->numEntries(); ++i) {
            const RooArgSet* row = nonPromptDataSet->get(i);
            hNonPrompt->Fill(row->getRealValue(dcaVar_->GetName()));
        }
        if (hNonPrompt->Integral() > 0) {
            hNonPrompt->Scale(1.0/hNonPrompt->Integral());
            hNonPrompt->Scale(1.0, "width");
            // hNonPrompt->Scale(1.0/hNonPrompt->Integral());
            // for (int i = 1; i <= hNonPrompt->GetNbinsX(); ++i) {
            //     double content = hNonPrompt->GetBinContent(i);
            //     double error = hNonPrompt->GetBinError(i);
            //     double width = hNonPrompt->GetBinWidth(i);
            //     if (width > 0) {
            //         hNonPrompt->SetBinContent(i, content / width);
            //         hNonPrompt->SetBinError(i, error / width);
            //     } else {
            //         hNonPrompt->SetBinContent(i, 0);
            //         hNonPrompt->SetBinError(i, 0);
            //     }
            // }
            hNonPrompt->SetLineColor(kBlue);
            hNonPrompt->SetMarkerColor(kBlue);
            hNonPrompt->SetMarkerStyle(20); 
            hNonPrompt->SetMarkerSize(1.0);
            LogInfoMsg("Created and normalized non-prompt histogram (per cm).");
        } else {
            delete hNonPrompt; hNonPrompt = nullptr;
        }
    }

    if (!hPrompt && !hNonPrompt) {
        LogErrorMsg("Failed to create any histograms for raw data plot.");
        return;
    }

    // Try to build test template histogram from RooDataHist if available
    if (testTemplate_) {
        hTestTemplate = testTemplate_->createHistogram("hTestTemplateRaw", *dcaVar_);
    } else if (auto* tt = dynamic_cast<RooDataHist*>(ws_->data("testTemplate"))) {
        hTestTemplate = tt->createHistogram("hTestTemplateRaw", *dcaVar_);
    } else if (enableTestComponent_) {
        auto* baseTemplate = testBasePrompt_ ? promptTemplate_ : nonPromptTemplate_;
        if (baseTemplate) {
            const char* histName = testBasePrompt_ ? "hPromptForTest_templates" : "hNonPromptForTest_templates";
            TH1* hTmp = baseTemplate->createHistogram(histName, *dcaVar_);
            if (hTmp) {
                double sum = 0.0;
                for (int ib = 1; ib <= hTmp->GetNbinsX(); ++ib) {
                    double y = std::max(0.0, hTmp->GetBinContent(ib));
                    double x = hTmp->GetXaxis()->GetBinCenter(ib);
                    double yn = y;
                    if (testUseSlope_) {
                        double factor = TMath::Exp(-1.0 * testSlope_ * x);
                        if (factor < 0.0) factor = 0.0;
                        yn *= factor;
                    } else {
                        yn = std::pow(y, std::max(0.0, testComponentPower_));
                    }
                    hTmp->SetBinContent(ib, yn);
                    hTmp->SetBinError(ib, 0.0);
                    sum += yn;
                }
                if (sum > 0) hTmp->Scale(1.0 / sum);
                hTmp->Scale(1.0, "width"); // per-cm view, consistent with others
                hTestTemplate = hTmp; // adopt for drawing + deletion below
            }
        }
    }
    if (hTestTemplate) {
        // Normalize to unit area and convert to per-cm like others
        if (hTestTemplate->Integral() > 0) {
            hTestTemplate->Scale(1.0 / hTestTemplate->Integral());
            hTestTemplate->Scale(1.0, "width");
        }
        hTestTemplate->SetLineColor(kGreen + 2);
        hTestTemplate->SetMarkerColor(kGreen + 2);
        hTestTemplate->SetMarkerStyle(kOpenTriangleUp);
        hTestTemplate->SetMarkerSize(1.0);
    }

    TCanvas* canvas = new TCanvas(plotName.c_str(), "Normalized MC Templates", 700, 600);
    canvas->SetLogy();
    canvas->SetLeftMargin(0.15);
    canvas->SetBottomMargin(0.13);
    canvas->SetTopMargin(0.10); 
    canvas->SetRightMargin(0.05);

    double minY = 1e9, maxY = -1e9;
    if (hPrompt) {
        for(int i=1; i<=hPrompt->GetNbinsX(); ++i) {
            if(hPrompt->GetBinContent(i) > 0) minY = std::min(minY, hPrompt->GetBinContent(i));
            maxY = std::max(maxY, hPrompt->GetBinContent(i) + hPrompt->GetBinError(i));
        }
    }
    if (hNonPrompt) {
         for(int i=1; i<=hNonPrompt->GetNbinsX(); ++i) {
            if(hNonPrompt->GetBinContent(i) > 0) minY = std::min(minY, hNonPrompt->GetBinContent(i));
            maxY = std::max(maxY, hNonPrompt->GetBinContent(i) + hNonPrompt->GetBinError(i));
        }
    }
    // Update axis range with test template if present
    if (hTestTemplate) {
        for (int i=1; i<=hTestTemplate->GetNbinsX(); ++i) {
            if (hTestTemplate->GetBinContent(i) > 0) minY = std::min(minY, hTestTemplate->GetBinContent(i));
            maxY = std::max(maxY, hTestTemplate->GetBinContent(i) + hTestTemplate->GetBinError(i));
        }
    }
    if (minY > maxY) { minY = 0.1; maxY = 100; } 

    TH1* hAxis = (hPrompt) ? hPrompt : hNonPrompt; 
    if (hAxis) {
        hAxis->SetTitle(""); 
        hAxis->GetXaxis()->SetTitle("D^{0} DCA (cm)");
        hAxis->GetYaxis()->SetTitle("normalized counts per cm");
        hAxis->GetXaxis()->SetTitleOffset(1.1);
        hAxis->GetYaxis()->SetTitleOffset(1.3);
        hAxis->GetXaxis()->SetLabelSize(0.04);
        hAxis->GetYaxis()->SetLabelSize(0.04);
        hAxis->GetXaxis()->SetTitleSize(0.045);
        hAxis->GetYaxis()->SetTitleSize(0.045);
        hAxis->SetMinimum(minY * 0.5); 
        hAxis->SetMaximum(maxY * 2.0); 
        hAxis->Draw("AXIS"); 
    }


    if (hPrompt) {
        hPrompt->Draw("E1 SAME"); 
    }
    if (hNonPrompt) {
        hNonPrompt->Draw("E1 SAME"); 
    }
    if (hTestTemplate) {
        hTestTemplate->Draw("E1 SAME");
    }
     if (hAxis) hAxis->Draw("AXIS SAME"); 


    TLegend* legend = new TLegend(0.55, 0.75, 0.93, 0.88); 
    if (hPrompt) legend->AddEntry(hPrompt, "MC Prompt D^{*}", "pe");
    if (hNonPrompt) legend->AddEntry(hNonPrompt, "MC Non-prompt D^{*}", "pe");
    if (hTestTemplate) legend->AddEntry(hTestTemplate, "Test Template (shape)", "pe");
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextFont(42);
    legend->SetTextSize(0.035);
    legend->Draw();

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.03);
    latex.SetTextAlign(22); 
    latex.DrawLatex(0.30, canvas->GetBottomMargin() + 0.03, opt_.pTLegend.empty() ? Form("%0.2f < p_{T} < %0.2f GeV/c",opt_.pTMin, opt_.pTMax) : opt_.pTLegend.c_str());
    latex.DrawLatex(0.30, canvas->GetBottomMargin() + 0.07, opt_.yLegend.empty() ? "|y| < 1" : opt_.yLegend.c_str());
    latex.DrawLatex(0.30, canvas->GetBottomMargin() + 0.11, opt_.cosLegend.empty() ? Form("%0.2f < cos#theta_{HX} < %0.2f", opt_.cosMin, opt_.cosMax) : opt_.cosLegend.c_str());



    std::string pngFileName = plotName + ".png";
    canvas->SaveAs(pngFileName.c_str());
    LogInfoMsg("Normalized MC plot saved as ", pngFileName);




    delete canvas;
    delete hPrompt;
    delete hNonPrompt;
    delete hTestTemplate;
}

inline void DCAFitter::addFractionSummaryToLegend(TLegend* leg) const {
    if (!leg || !ws_) return;

    RooRealVar* nPromptVar = ws_->var("n_prompt");
    RooRealVar* nNonPromptVar = ws_->var("n_nonprompt");
    RooRealVar* nTestVar = ws_->var("n_test");

    const double nP = nPromptVar ? nPromptVar->getVal() : 0.0;
    const double nNP = nNonPromptVar ? nNonPromptVar->getVal() : 0.0;
    const double nT = nTestVar ? nTestVar->getVal() : 0.0;
    const double sumN = nP + nNP + nT;

    if (sumN <= 0.0) return;

    const double fP = nP / sumN;
    const double fNP = nNP / sumN;
    const double fT = nT / sumN;

    TObject* dummy = nullptr;
    leg->AddEntry(dummy, Form("Prompt Fraction = %.2f", fP), "");
    leg->AddEntry(dummy, Form("NonPrompt Fraction = %.2f", fNP), "");
    if (nTestVar) {
        leg->AddEntry(dummy, Form("Test Fraction = %.2f", fT), "");
    }
}

// --- Implementation: MassFitterV2-style DCA slice fitting loop ---
template<typename SignalParams, typename BackgroundParams>
inline bool DCAFitter::FitSlicesMassV2(
    RooDataSet* fullDataset,
    const SignalParams& /*signalParams*/,
    const BackgroundParams& /*backgroundParams*/,
    const std::string& sliceResultPrefix,
    bool savePlots) {
    try {
        if (!fullDataset) {
            if (errorHandler_) errorHandler_->HandleError("FitSlicesMassV2", "Null dataset");
            return false;
        }

        // Determine DCA binning
        const std::vector<double>& edges = !dcaBins_.empty() ? dcaBins_ : std::vector<double>{dcaMin_, dcaMax_};
        if (edges.size() < 2) {
            if (errorHandler_) errorHandler_->HandleError("FitSlicesMassV2", "Insufficient DCA bin edges");
            return false;
        }

        bool allOk = true;
        for (size_t i = 0; i + 1 < edges.size(); ++i) {
            const double dmin = edges[i];
            const double dmax = edges[i + 1];
            const std::string sliceName = Form("%s_%zu_%.6g_%.6g", sliceResultPrefix.c_str(), i, dmin, dmax);

            // Create a DCA slice of the dataset
            auto sliceData = createDCASlice(fullDataset, dmin, dmax);
            if (!sliceData || sliceData->numEntries() == 0) {
                // Record as failed (no data)
                BinInfo base = createBinInfoFromFitOpt(opt_, dmin, dmax);
                DCASliceInfo sInfo(dmin, dmax, static_cast<int>(i), sliceName);
                notifyDCASliceFit(base, sInfo, nullptr, "DCA_Mass_Slice");
                allOk = false; // mark but continue
                continue;
            }

            // Build a MassFitterV2 for this slice (lifetime kept in map)
            MassFitterV2* fitter = createMassFitterV2ForSlice(sliceName);
            fitter->SetData(sliceData.get());

            // Apply any analysis cut carried by FitOpt
            if (!opt_.cutExpr.empty()) fitter->ApplyCut(opt_.cutExpr);

            // Prefer simple Fit() with internal configuration
            bool ok = false;
            try {
                ok = fitter->Fit();
            } catch (const std::exception& e) {
                ok = false;
                if (errorHandler_) errorHandler_->HandleError("FitSlicesMassV2::Fit", e.what());
            }

            RooFitResult* r = fitter->GetRooFitResult();
            BinInfo base = createBinInfoFromFitOpt(opt_, dmin, dmax);
            DCASliceInfo sInfo(dmin, dmax, static_cast<int>(i), sliceName);
            notifyDCASliceFit(base, sInfo, r, "DCA_Mass_Slice");

            if (!ok) allOk = false;

            // Optional quick plot per-slice
            if (savePlots) {
                try {
                    auto c = fitter->CreateCanvas();
                    if (c) {
                        // Annotate and save in structured output directory
                        c->cd();
                        TLatex tx; tx.SetNDC(); tx.SetTextFont(42);
                        tx.SetTextSize(0.035);
                        // Compose legend lines from FitOpt
                        std::string l_pt   = opt_.pTLegend.empty() ? Form("%.1f < p_{T} < %.1f GeV/c", opt_.pTMin, opt_.pTMax) : opt_.pTLegend;
                        std::string l_cos  = opt_.centLegend.empty() ? Form("%.1f < cos#theta_{HX} < %.1f", opt_.cosMin, opt_.cosMax) : opt_.centLegend;
                        std::string l_y    = opt_.yLegend.empty() ? Form("|y| < %.1f", opt_.etaMax) : opt_.yLegend;
                        std::string l_dca  = Form("DCA: %.6g - %.6g cm", dmin, dmax);
                        tx.DrawLatex(0.18, 0.88, l_pt.c_str());
                        tx.DrawLatex(0.18, 0.84, l_cos.c_str());
                        tx.DrawLatex(0.18, 0.80, l_y.c_str());
                        tx.DrawLatex(0.18, 0.76, l_dca.c_str());

                        // Save under output directory
                        std::string baseDir = opt_.outputPlotDir.empty() ? std::string(".") : opt_.outputPlotDir;
                        std::string outDir = baseDir + "/mass_fits_" + name_ + "/slice_mass_distributions";
                        if (!ensureDir(outDir)) {
                            ErrorHandlerManager::Instance().LogError(std::string("Cannot create output directory: ") + outDir,
                                                                     std::string("DCAFitter:") + name_);
                        } else {
                            std::string outPng = outDir + "/" + sliceName + ".png";
                            c->SaveAs(outPng.c_str());
                        }
                    }
                } catch (...) {
                    // best-effort plotting, ignore errors
                }
            }
        }

        return allOk;
    } catch (const std::exception& e) {
        if (errorHandler_) errorHandler_->HandleError("FitSlicesMassV2", e.what());
        return false;
    }
}

inline std::string DCAFitter::LogContext() const {
    return std::string("DCAFitter:") + name_;
}

template<typename... Args>
inline std::string DCAFitter::BuildLogMessage(Args&&... args) const {
    std::ostringstream oss;
    using expander = int[];
    (void)expander{0, ((void)(oss << std::forward<Args>(args)), 0)...};
    return oss.str();
}

template<typename... Args>
inline void DCAFitter::LogInfoMsg(Args&&... args) const {
    ErrorHandlerManager::Instance().LogInfo(
        BuildLogMessage(std::forward<Args>(args)...),
        LogContext());
}

template<typename... Args>
inline void DCAFitter::LogWarningMsg(Args&&... args) const {
    ErrorHandlerManager::Instance().LogWarning(
        BuildLogMessage(std::forward<Args>(args)...),
        LogContext());
}

template<typename... Args>
inline void DCAFitter::LogErrorMsg(Args&&... args) const {
    ErrorHandlerManager::Instance().LogError(
        BuildLogMessage(std::forward<Args>(args)...),
        LogContext());
}

inline bool DCAFitter::EnsureDirectoryReady(const std::string& path) const {
    if (path.empty()) return true;
    if (ensureDir(path)) return true;
    LogErrorMsg("Failed to ensure directory: ", path);
    return false;
}

inline bool DCAFitter::EnsureModelPreconditions(const char* context, RooRealVar*& dcaVar) {
    if (!dataSet_) {
        dataSet_ = static_cast<RooDataSet*>(ws_->data("dataSet"));
    }
    if (!dataSet_) {
        LogErrorMsg(context, ": Data set is not loaded. Cannot build model.");
        return false;
    }
    if (!ws_->data("promptTemplate") && !ws_->data("nonPromptTemplate") && !runMassFits_) {
        LogErrorMsg(context, ": No MC templates and mass-fit mode disabled. Cannot build model.");
        return false;
    }
    if (massVarName_.empty()) {
        LogErrorMsg(context, ": Mass variable name not set. Cannot build templates.");
        return false;
    }
    if (dcaBins_.size() < 2) {
        LogErrorMsg(context, ": dcaBins_ not properly configured.");
        return false;
    }

    dcaVar_->SetName(dcaBranchName_.empty() ? opt_.dcaVar.c_str() : dcaBranchName_.c_str());
    dcaVar = ws_->var(dcaVar_->GetName());
    if (!dcaVar) {
        dcaVar = dcaVar_.get();
        if (!dcaVar) {
            LogErrorMsg(context, ": Active DCA variable not found in workspace.");
            return false;
        }
        ws_->import(*dcaVar);
    }
    return true;
}

inline bool DCAFitter::BuildMCModelFromTemplates(RooRealVar* dcaVar, double totalEvents) {
        LogInfoMsg("Building model using MC templates.");

        promptTemplate_ = static_cast<RooDataHist*>(ws_->data("promptTemplate"));
        nonPromptTemplate_ = static_cast<RooDataHist*>(ws_->data("nonPromptTemplate"));

        if (!promptTemplate_ && !nonPromptTemplate_) {
            LogErrorMsg("Error: No MC templates (RooDataHist) are available to build the model.");
            LogErrorMsg("       Please run createTemplatesFromMC() first if you intend to use MC templates.");
            return false;
        }

        double nevt = totalEvents;
        if (nevt <= 0.0) nevt = 1000.0;
        LogInfoMsg("Estimated number of events for yield: ", nevt);

        n_prompt_ = std::make_unique<RooRealVar>("n_prompt", "Number of prompt events", nevt * 0.8, 0.0, nevt);
        ws_->import(*n_prompt_, RooFit::RecycleConflictNodes());
        n_nonprompt_ = std::make_unique<RooRealVar>("n_nonprompt", "Number of non-prompt events", nevt * 0.2, 0.0, nevt);
        ws_->import(*n_nonprompt_, RooFit::RecycleConflictNodes());

        RooArgList pdfList;
        RooArgList yieldList;

        if (promptTemplate_) {
            delete promptPdf_;
            promptPdf_ = new RooHistPdf("promptPdf", "Prompt PDF from MC", RooArgSet(*dcaVar), *promptTemplate_);
            promptFunc_ = new RooHistFunc("promptFunc", "Prompt Function from MC", RooArgSet(*dcaVar), *promptTemplate_);
            ws_->import(*promptPdf_, RooFit::RecycleConflictNodes());
            pdfList.add(*promptPdf_);
            yieldList.add(*n_prompt_);
            LogInfoMsg("Prompt PDF created from MC template.");
        }

        if (nonPromptTemplate_) {
            delete nonPromptPdf_;
            nonPromptPdf_ = new RooHistPdf("nonPromptPdf", "Non-prompt PDF from MC", RooArgSet(*dcaVar), *nonPromptTemplate_);
            nonPromptFunc_ = new RooHistFunc("nonPromptFunc", "Non-prompt Function from MC", RooArgSet(*dcaVar), *nonPromptTemplate_);
            ws_->import(*nonPromptPdf_, RooFit::RecycleConflictNodes());
            pdfList.add(*nonPromptPdf_);
            yieldList.add(*n_nonprompt_);
            LogInfoMsg("Non-prompt PDF created from MC template.");
        }

        if (pdfList.getSize() > 0) {
            auto* baseTemplate = testBasePrompt_ ? promptTemplate_ : nonPromptTemplate_;
            LogInfoMsg("[DCAFitter][Test] enableTestComponent=", (enableTestComponent_?"true":"false"),
                       ", base=", (testBasePrompt_?"prompt":"nonprompt"),
                       ", basePresent=", (baseTemplate?"yes":"no"),
                       ", useSlope=", (testUseSlope_?"true":"false"),
                       ", param=", (testUseSlope_?testSlope_:testComponentPower_));

            if (enableTestComponent_ && baseTemplate) {
                TH1* baseHist = baseTemplate->createHistogram((std::string(testBasePrompt_?"hPromptForTest_":"hNonPromptForTest_") + name_).c_str(), *dcaVar);
                if (baseHist) {
                    double sum = 0.0;
                    for (int ib = 1; ib <= baseHist->GetNbinsX(); ++ib) {
                        double y = std::max(0.0, baseHist->GetBinContent(ib));
                        double yn = y;
                        if (testUseSlope_) {
                            double x = baseHist->GetXaxis()->GetBinCenter(ib);
                            double factor = TMath::Exp(-1.0 * testSlope_ * x);
                            yn = std::max(0.0, factor) * y;
                        } else {
                            yn = std::pow(y, std::max(0.0, testComponentPower_));
                        }
                        baseHist->SetBinContent(ib, yn);
                        baseHist->SetBinError(ib, 0.0);
                        sum += yn;
                    }
                    if (sum > 0.0) baseHist->Scale(1.0 / sum);

                    if (testTemplate_) { delete testTemplate_; testTemplate_ = nullptr; }
                    testTemplate_ = new RooDataHist("testTemplate", "Test (intermediate) Template", RooArgSet(*dcaVar), baseHist);
                    if (testPdf_) { delete testPdf_; testPdf_ = nullptr; }
                    testPdf_ = new RooHistPdf("testPdf", "Test PDF from transformed template", RooArgSet(*dcaVar), *testTemplate_);
                    ws_->import(*testTemplate_, RooFit::RecycleConflictNodes());
                    ws_->import(*testPdf_, RooFit::RecycleConflictNodes());

                    if (!n_test_) {
                        n_test_ = std::make_unique<RooRealVar>("n_test", "Number of test events", nevt * 0.1, 0.0, nevt);
                        ws_->import(*n_test_, RooFit::RecycleConflictNodes());
                    } else {
                        n_test_->setVal(nevt * 0.1);
                        n_test_->setMax(nevt);
                    }
                    pdfList.add(*testPdf_);
                    yieldList.add(*n_test_);
                    LogInfoMsg("[DCAFitter][Test] Test PDF added (", (testUseSlope_?"slope":"pow"),
                               ") base=", (testBasePrompt_?"prompt":"nonprompt"),
                               ", param=", (testUseSlope_?testSlope_:testComponentPower_),
                               ", n_test init=", n_test_->getVal(), "/", nevt);
                    delete baseHist;
                } else {
                    LogErrorMsg("[DCAFitter][Test] Failed to create TH1 from base template for test component");
                }
            } else if (enableTestComponent_ && !baseTemplate) {
                LogWarningMsg("[DCAFitter][Test] Skipped: base template not available (cannot build test component)");
            }
        }

        if (pdfList.getSize() == 0) {
            LogErrorMsg("Error: No MC PDFs were created to build the model.");
            return false;
        }

        delete model_;
        model_ = new RooAddPdf("model", "Extended P+NP(+Test) Model", pdfList, yieldList);
        ws_->import(*model_, RooFit::RecycleConflictNodes());
        LogInfoMsg("Extended model built successfully with ", pdfList.getSize(), " component(s).");

        if (nonPromptFunc_ && promptFunc_) {
            ws_->import(*promptFunc_, RooFit::RecycleConflictNodes());
            ws_->import(*nonPromptFunc_, RooFit::RecycleConflictNodes());
        }

        return true;
    }


#endif // DCA_FITTER_H
