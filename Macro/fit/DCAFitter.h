// DCAFitter.h - Enhanced version compatible with MassFitterV2
// Key improvements:
// 1. MassFitterV2 integration with modular architecture
// 2. Dependency injection pattern for enhanced testability
// 3. Template-based fitting interface for type safety
// 4. Enhanced error handling with exception safety
// 5. Configuration management with ConfigManager
// 6. Smart pointer-based memory management (RAII)
// 7. Result management with ResultManager integration

#ifndef DCA_FITTER_H
#define DCA_FITTER_H

#include <string>
#include <vector>
#include <map>
#include <iostream> // Added for output
#include <memory>   // Added for smart pointers

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h" // Added for string manipulation
#include "Helper.h" // Assuming Helper.h provides necessary utilities
#include "TStyle.h" // For gStyle
#include "TPad.h"   // For gPad
#include "TH1.h"    // For TH1
#include "THStack.h" // For THStack
#include "TLatex.h"  // For CMS label
#include "Opt.h"
#include "PlotManager.h"
#include "Helper.h" 

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
#include "RooLinkedList.h" // Add this include
#include "TSystem.h" // Added for gSystem

#include "MassFitterV2.h"  // Updated to use V2
#include "PDFFactory.h"     // For modular PDF creation
#include "FitStrategy.h"    // For configurable fitting strategies
#include "ResultManager.h"  // For enhanced result management
#include "ConfigManager.h"  // For configuration management
#include "ErrorHandler.h"   // For enhanced error handling
#include "Params.h"
#include "Helper.h"
using namespace RooFit;

// Type aliases for cleaner code
using RooRealVarPtr = std::unique_ptr<RooRealVar>;
using RooWorkspacePtr = std::unique_ptr<RooWorkspace>;
using RooDataSetPtr = std::unique_ptr<RooDataSet>;
using TH1DPtr = std::unique_ptr<TH1D>;
using TFilePtr = std::unique_ptr<TFile>;

class DCAFitter {
public:
    // Enhanced constructor with dependency injection and MassFitterV2 integration
    DCAFitter(const FitOpt& opt, 
              const std::string& name, 
              const std::string& massVarName, 
              double dcaMin, 
              double dcaMax, 
              int nBins,
              std::unique_ptr<ConfigManager> configManager = nullptr,
              std::unique_ptr<ErrorHandler> errorHandler = nullptr) :
        name_(name),
        opt_(opt),
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
        try {
            // Initialize default dependencies if not provided
            if (!configManager_) {
                configManager_ = std::make_unique<ConfigManager>();
            }
            if (!errorHandler_) {
                errorHandler_ = std::make_unique<ErrorHandler>();
            }
            
            // Enhanced parameter validation with error handler
            validateConstructorParameters(name, massVarName, dcaMin, dcaMax, nBins);
            
            // Initialize configuration
            initializeConfiguration();
            
            // Create workspace with enhanced error handling
            initializeWorkspace();
            
            // Initialize variables
            initializeVariables();
            
            std::cout << "Enhanced DCAFitter instance created: " << name_ << std::endl;
            
        } catch (const std::exception& e) {
            if (errorHandler_) {
                errorHandler_->HandleError("DCAFitter Constructor", e.what());
            }
            throw;
        }
    }
    
    // Alternative constructor for backward compatibility
    DCAFitter(FitOpt &opt, const std::string& name, const std::string& massVarName, 
              double dcaMin, double dcaMax, int nBins) :
        DCAFitter(static_cast<const FitOpt&>(opt), name, massVarName, dcaMin, dcaMax, nBins)
    {}

    // Destructor
    virtual ~DCAFitter() {
        
        // delete promptHist_;
        // delete nonPromptHist_;

        std::cout << "DCAFitter instance destroyed: " << name_ << std::endl;
    }

    // --- Configuration ---
    void setMCFile(const std::string& fileName, const std::string& datasetName = "") {
        mcFileName_ = fileName;
        // mcTreeName_ = treeName;
        mcRooDatsetName_ = datasetName;
    }
    void setDataFile(const std::string& fileName, const std::string& datasetName = "") {
        dataFileName_ = fileName;
        // dataTreeName_ = treeName;
        dataRooDatsetName_ = datasetName; // Default name for data set
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
    void plotSignalAndSidebandDCAFromHist(const std::string& plotName = "dca_sig_vs_sb_hist_plot");
    void setWeightBranchName(const std::string& branchName = "") {
        weightBranchName_ = branchName;
        if (!weightBranchName_.empty()) {
            weights_ = std::make_unique<RooRealVar>(weightBranchName_.c_str(), "Event Weight", 1.0);
        } else {
            weights_.reset(); 
        }
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
    
    // Helper function for safe file opening
    TFilePtr openFile(const std::string& fileName, const std::string& mode = "READ") {
        auto file = std::unique_ptr<TFile>(TFile::Open(fileName.c_str(), mode.c_str()));
        if (!file || file->IsZombie()) {
            std::cerr << "Error: Could not open file " << fileName << std::endl;
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
                     double& yield, double& yieldError) {
        try {
            if (!dataSlice) {
                throw std::invalid_argument("Data slice is null");
            }
            
            // Create MassFitterV2 instance for this slice
            auto massFitter = createMassFitterV2ForSlice(sliceName);
            
            // Set data
            massFitter->SetData(dataSlice);
            
            // Setup configuration for mass fitting
            FitConfig massFitConfig = createMassFitConfig(sliceName);
            massFitter->SetConfiguration(massFitConfig);
            
            // Perform template-based fit
            bool fitSuccess = false;
            if (mcSlice) {
                fitSuccess = massFitter->PerformConstraintFit(
                    opt_, dataSlice, mcSlice, signalParams, backgroundParams,
                    {}, sliceName
                );
            } else {
                fitSuccess = massFitter->PerformFit(
                    opt_, dataSlice, signalParams, backgroundParams, sliceName
                );
            }
            
            if (fitSuccess) {
                yield = massFitter->GetSignalYield(sliceName);
                yieldError = massFitter->GetSignalYieldError(sliceName);
                
                // Store result for later access
                sliceFitResults_[sliceName] = massFitter->GetFitResults(sliceName);
                
                std::cout << "Mass fit SUCCESS for DCA slice " << sliceName 
                         << ": Yield = " << yield << " +/- " << yieldError << std::endl;
                return true;
            } else {
                errorHandler_->HandleError("FitDCASlice", 
                    "Mass fitting failed for slice: " + sliceName);
                return false;
            }
            
        } catch (const std::exception& e) {
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
            FitConfig globalConfig = configManager_->GetConfiguration("DCAFitting");
            
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
                                              sliceName, yield, yieldError);
                
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
    
    void SetErrorHandler(std::unique_ptr<ErrorHandler> errorHandler) {
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
        std::cout << "Creating templates from MC file: " << mcFileName_ << std::endl;
        if (mcFileName_.empty() || mcRooDatsetName_.empty() || dcaBranchName_.empty() || motherPdgIdBranchName_.empty()) {
            std::cerr << "Error: MC file, tree name, DCA branch, or Mother PDG ID branch not set." << std::endl;
            return false;
        }

        auto mcFile = openFile(mcFileName_, "READ");
        if (!mcFile) {
            std::cerr << "Error: Could not open MC file: " << mcFileName_ << std::endl;
            return false;
        }
        RooDataSet* mcDataset = dynamic_cast<RooDataSet*>(mcFile->Get(mcRooDatsetName_.c_str()));
        if (!mcDataset) {
            std::cerr << "Error: Could not load dataset " << mcRooDatsetName_ << " from file" << std::endl;
            return false;
        }

        // --- Define variables needed from the TTree ---
        // Ensure dcaVar_ uses the correct branch name
        dcaVar_->SetName(dcaBranchName_.c_str());
        RooRealVar motherPdgIdVar(motherPdgIdBranchName_.c_str(), "Mother PDG ID", 0); // Temporary variable for loading

        RooArgSet mcVars(*dcaVar_, motherPdgIdVar);
        std::unique_ptr<RooRealVar> tempWeights; // Use unique_ptr for temporary weight var if needed
        if (!weightBranchName_.empty()) {
            // Create a temporary RooRealVar for loading weights if weights_ isn't already set up
            // or if its range/properties might differ from the global weights_
             tempWeights = std::make_unique<RooRealVar>(weightBranchName_.c_str(), "Event Weight", 1.0);
             mcVars.add(*tempWeights);
             // If using a global weights_ variable:
             // if (weights_) mcVars.add(*weights_);
        }


        // --- Load full MC dataset ---
        // std::unique_ptr<RooDataSet> fullMCDataSet;
        const char* weightVarName = !weightBranchName_.empty() ? weightBranchName_.c_str() : nullptr;

        if(mcDataset) fullMCDataSet.reset(mcDataset);
        else return cout << "Error: Could not find MC dataset: " << mcRooDatsetName_ << " in file: " << mcFileName_ << std::endl, false;


        if (!fullMCDataSet || fullMCDataSet->numEntries() == 0) {
            std::cerr << "Error: Failed to load MC data or no entries passed cuts: " << mcCuts_ << std::endl;
            mcFile->Close();
            mcFile.reset();
             // Restore dcaVar_ name before returning
             dcaVar_->SetName(opt_.dcaVar.c_str());
            return false;
        }
        std::cout << "Loaded " << fullMCDataSet->sumEntries() << " MC entries (after cuts, weighted)." << std::endl;

        // --- Split MC dataset into prompt and non-prompt ---
        TString promptCut = "";
        if (!promptPdgIds_.empty()) {
            promptCut += "(";
            for (size_t i = 0; i < promptPdgIds_.size(); ++i) {
                promptCut += TString::Format("abs(%s)==%d", motherPdgIdBranchName_.c_str(), promptPdgIds_[i]);
                if (i < promptPdgIds_.size() - 1) {
                    promptCut += " || ";
                }
            }
            promptCut += ")";
        } else {
             std::cerr << "Warning: No prompt PDG IDs specified. Cannot separate prompt/non-prompt." << std::endl;
             mcFile->Close();
             mcFile.reset();
             // Restore dcaVar_ name before returning
             dcaVar_->SetName((opt_.dcaVar).c_str());
             return false;
        }
        cout << "Prompt cut: " << promptCut << endl;

        TString nonPromptCut = "!" + promptCut + " && ";

        if (!nonpromptPdgIds_.empty()) {
            nonPromptCut += "(";
            for (size_t i = 0; i < nonpromptPdgIds_.size(); ++i) {
                nonPromptCut += TString::Format("abs(%s)==%d", motherPdgIdBranchName_.c_str(), nonpromptPdgIds_[i]);
                if (i < nonpromptPdgIds_.size() - 1) {
                    nonPromptCut += " || ";
                }
            }
            nonPromptCut += ")";
        } else {
             std::cerr << "Warning: No prompt PDG IDs specified. Cannot separate prompt/non-prompt." << std::endl;
             mcFile->Close();
             mcFile.reset();
             // Restore dcaVar_ name before returning
             dcaVar_->SetName(opt_.dcaVar.c_str());
             return false;
        }
        cout << "Non-prompt cut: " << nonPromptCut << endl;

        // Use Reduce to create the subsets
        // Need to use the ArgSet without motherPdgIdVar for the final datasets/templates
        RooArgSet finalVars(*dcaVar_);
        // Add the global weights_ variable if it exists and is intended for the templates
        if (weights_) finalVars.add(*weights_);
        // If loading used a temporary weight variable, ensure the final dataset retains weights
        // This might require adding the weight variable back if Reduce removes it,
        // or ensuring the weight variable used by Reduce is the one we want.
        // If fullMCDataSet was loaded with weights, Reduce should preserve them if the weight var is in finalVars.


        std::unique_ptr<RooDataSet> promptDataSet(dynamic_cast<RooDataSet*>(fullMCDataSet->reduce(finalVars, promptCut.Data())));
        promptDataSet->SetName("promptDataSet");
        std::unique_ptr<RooDataSet> nonPromptDataSet(dynamic_cast<RooDataSet*>(fullMCDataSet->reduce(finalVars, nonPromptCut.Data())));
        nonPromptDataSet->SetName("nonPromptDataSet");
        ws_->import(*promptDataSet);
        ws_->import(*nonPromptDataSet);

        promptHist_ = new TH1D("promptHist", "Prompt Histogram", dcaBins_.size() - 1, dcaBins_.data());
        promptHist_->Sumw2();
        for (int i = 0; i < promptDataSet->numEntries(); ++i) {
            const RooArgSet* row = promptDataSet->get(i);
            // cout << row->getRealValue(dcaVar_->GetName()) << endl;
            // cout << dcaVar_->GetName() << endl;
            promptHist_->Fill(row->getRealValue(dcaVar_->GetName()));
        }
        nonPromptHist_ = new TH1D("nonPromptHist", "Non-prompt Histogram", dcaBins_.size() - 1,dcaBins_.data());
        nonPromptHist_->Sumw2();
        for (int i = 0; i < nonPromptDataSet->numEntries(); ++i) {
            const RooArgSet* row = nonPromptDataSet->get(i);
            nonPromptHist_->Fill(row->getRealValue(dcaVar_->GetName()));
            // cout << row->getRealValue(dcaVar_->GetName()) << endl;
            // cout << dcaVar_->GetName() << endl;
        }
        // promptHist_->Scale(1.0, "width"); // Scale by bin width
        // nonPromptHist_->Scale(1.0, "width"); // Scale by bin width


        if (!promptDataSet || promptDataSet->numEntries() == 0) {
             std::cerr << "Warning: No prompt MC events found after selection." << std::endl;
        } else {
             std::cout << "Prompt MC entries: " << promptDataSet->sumEntries() << " (weighted)" << std::endl;
             // Create binned template (RooDataHist)
             // Ensure the correct ArgSet (just dcaVar_) is used for binning
             promptTemplate_ = new RooDataHist("promptTemplate", "Prompt MC Template", RooArgSet(*dcaVar_), promptHist_);
             ws_->import(*promptTemplate_); // Import into workspace
             std::cout << "Prompt template created." << std::endl;
        }

        if (!nonPromptDataSet || nonPromptDataSet->numEntries() == 0) {
            std::cerr << "Warning: No non-prompt MC events found after selection." << std::endl;
            std::cerr << "Will proceed with prompt-only fitting." << std::endl;
            nonPromptTemplate_ = nullptr;
            nonPromptHist_ = nullptr;
        } else {
             std::cout << "Non-prompt MC entries: " << nonPromptDataSet->sumEntries() << " (weighted)" << std::endl;
             // Create binned template (RooDataHist)
             nonPromptTemplate_ = new RooDataHist("nonPromptTemplate", "Non-prompt MC Template", RooArgSet(*dcaVar_), nonPromptHist_);
             ws_->import(*nonPromptTemplate_); // Import into workspace
             std::cout << "Non-prompt template created." << std::endl;
        }

        mcFile->Close();
        mcFile.reset();

        // Restore dcaVar_ name if it was temporarily changed
        // dcaVar_->SetName("dca3D");

        return (promptTemplate_ != nullptr || nonPromptTemplate_ != nullptr); // Success if at least one template was created
    }


    bool loadData() {
        std::cout << "Loading data from file: " << dataFileName_ << std::endl;
        if (dataFileName_.empty() || dataRooDatsetName_.empty() || dcaBranchName_.empty()) {
            std::cerr << "Error: Data file, tree name, or DCA branch not set." << std::endl;
            return false;
        }

        TFile* dataFile = TFile::Open(dataFileName_.c_str());
        if (!dataFile || dataFile->IsZombie()) {
            std::cerr << "Error: Could not open data file: " << dataFileName_ << std::endl;
            return false;
        }
        RooDataSet* dataDataset = dynamic_cast<RooDataSet*>(dataFile->Get(dataRooDatsetName_.c_str()));

        // TTree* dataTree = dynamic_cast<TTree*>(dataFile->Get(dataTreeName_.c_str()));
        // if (!dataTree) {
        //     std::cerr << "Error: Could not find data tree: " << dataTreeName_ << " in file: " << dataFileName_ << std::endl;
        //     dataFile->Close();
        //     delete dataFile;
        //     return false;
        // }

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
            // If imported into workspace, ws_->Delete("dataSet_") might be needed first
            // However, RooWorkspace::import with the same name should replace it.
            delete dataSet_; // Delete the old object before creating a new one
            dataSet_ = nullptr;
        }

        const char* weightVarName = !weightBranchName_.empty() ? weightBranchName_.c_str() : nullptr;
        if(dataDataset) 
            dataSet_ = dynamic_cast<RooDataSet*>(dataDataset);
        else cout << "Error: Could not find data dataset: " << dataRooDatsetName_ << " in file: " << dataFileName_ << std::endl, false;
        // else dataSet_ = new RooDataSet("dataSet", "Data Set", dataTree, dataVars, dataCuts_.c_str(), weightVarName);


        if (!dataSet_ || dataSet_->numEntries() == 0) {
            std::cerr << "Error: Failed to load data or no entries passed cuts: " << dataCuts_ << std::endl;
            dataFile->Close();
            delete dataFile;
            // Restore dcaVar_ name
            dcaVar_->SetName(opt_.dcaVar.c_str());
            return false;
        }

        std::cout << "Loaded " << dataSet_->sumEntries() << " data entries (after cuts, weighted)." << std::endl;
        ws_->import(*dataSet_); // Import dataset into workspace

        dataFile->Close();
        // delete dataFile;
        // Restore dcaVar_ name
        dcaVar_->SetName(opt_.dcaVar.c_str());
        return true;
    }

        bool buildModel() { 
        std::cout << "Building RooFit model..." << std::endl;

        if (!dataSet_) { 
             dataSet_ = (RooDataSet*)ws_->data("dataSet");
             if (!dataSet_){
                std::cerr << "Error: Data set is not loaded. Cannot build model." << std::endl;
                return false;
             }
        }
         if (!dataSet_ && !ws_->data("promptDataSetMC") && !ws_->data("nonPromptDataSetMC")){
            std::cerr << "Error: No MC templates and no data loaded. Cannot build model." << std::endl;
            return false;
        }


        if (massVarName_.empty()) {
            std::cerr << "Error: Mass variable name not set. Cannot build data-driven templates." << std::endl;
            return false;
        }
        if (dcaBins_.empty() || dcaBins_.size() < 2) {
            std::cerr << "Error: dcaBins_ not properly set." << std::endl;
            return false;
        }

        RooRealVar* dca = ws_->var(dcaVar_->GetName());
        if (!dca) {
            dca = dcaVar_.get();
            if (!dca) {
                 std::cerr << "Error: Standard 'dca' variable not found in workspace or dcaVar_." << std::endl;
                 return false;
            }
            ws_->import(*dca); 
        }


            std::cout << "Building model using data-driven DCA templates from mass fits." << std::endl;
            // RooRealVar* mass = ws_->var(massVarName_.c_str()); 
            // RooRealVar* mass = new RooRealVar(mass)
            // if (!mass) {
            //     std::cerr << "Error: Mass variable '" << massVarName_ << "' not found in workspace." << std::endl;
            //     return false;
            // }

            dataYieldHist_.reset(); 
            dataYieldHist_ = std::make_unique<TH1D>("dataYieldHist", "DCA Yield from Mass Fits", dcaBins_.size() - 1, dcaBins_.data());
            dataYieldHist_->Sumw2();
            for (size_t i = 0; i < dcaBins_.size() - 1; ++i) {
                cout << "Processing DCA bin " << i << ": [" << dcaBins_[i] << ", " << dcaBins_[i+1] << "]" << endl;
                double dcaLow = dcaBins_[i];
                double dcaHigh = dcaBins_[i+1];
                    DCASliceInfo sliceInfo(dcaLow, dcaHigh, static_cast<int>(i), 
                          Form("DCA_%.4f_%.4f", dcaLow, dcaHigh));
                     BinInfo currentBin = createBinInfoFromFitOpt(opt_, dcaLow, dcaHigh);
                std::string sliceName = Form("dca_%.3f_%.3f", dcaLow, dcaHigh);
                std::replace(sliceName.begin(), sliceName.end(), '.', 'p'); 

                TString dcaCut = TString::Format("%s >= %f && %s < %f", dca->GetName(), dcaLow, dca->GetName(), dcaHigh);
                RooDataSet* dcaSliceData = dynamic_cast<RooDataSet*>(dataSet_->reduce(dcaCut.Data()));
                RooDataSet* dcaSliceMC = dynamic_cast<RooDataSet*>(fullMCDataSet->reduce(dcaCut.Data()));

                double yield = 0;
                double yieldError = 0;
                bool fitSuccess = false;

                if (dcaSliceData && dcaSliceData->numEntries() > 0) {
                    
                    // RooPlot* massFrameBeforeFit = massVar_->frame(RooFit::Title(Form("Mass Distribution for %s (Before Fit)", sliceName.c_str())));
                    // dcaSliceData->plotOn(massFrameBeforeFit); 

                    // TCanvas* cSliceMassBeforeFit = new TCanvas(Form("cSliceMassBeforeFit_%s", sliceName.c_str()), Form("Mass Distribution for %s (Before Fit)", sliceName.c_str()), 800, 600);
                    // massFrameBeforeFit->Draw();

                    std::string plotDir = opt_.outputPlotDir + "/mass_fits_" + name_ + "/slice_mass_distributions/";
                    gSystem->mkdir(plotDir.c_str(), kTRUE); 
                    // std::string plotFileNameBeforeFit = plotDir + "mass_dist_before_fit_" + sliceName + ".png"ㅈ 
                    // cSliceMassBeforeFit->SaveAs(plotFileNameBeforeFit.c_str());
                    // delete cSliceMassBeforeFit;
                    // delete massFrameBeforeFit;
                    // End of plotting before fit

                    // auto params = DStarParamMaker1({0},{0});
                    auto params = D0ParamMaker({0},{0},3);
                    
                    PDFParams::DBCrystalBallParams sigDBCBParams; 
                    sigDBCBParams.mean = 0.1455;        // D* mass in GeV
                    sigDBCBParams.mean_min = 0.1452;
                    sigDBCBParams.mean_max = 0.1458;
                    sigDBCBParams.sigma = 0.0005; // D* sigma in GeV
                    sigDBCBParams.sigma_min = 0.0001;
                    sigDBCBParams.sigma_max = 0.01;
                    sigDBCBParams.sigmaR = 0.0005;
                    sigDBCBParams.sigmaR_min = 0.0001;
                    sigDBCBParams.sigmaR_max = 0.01;
                    sigDBCBParams.sigmaL = 0.0005;
                    sigDBCBParams.sigmaL_min = 0.0001;
                    sigDBCBParams.sigmaL_max = 0.01;
                    
                    sigDBCBParams.alphaL = 1.1;
                    sigDBCBParams.alphaL_min = 0.01;
                    sigDBCBParams.alphaL_max = 5;
                    sigDBCBParams.nL = 1.2;
                    sigDBCBParams.nL_min = 1.1;
                    sigDBCBParams.nL_max = 20;
                    
                    sigDBCBParams.alphaR = 1.1;
                    sigDBCBParams.alphaR_min = 0.01;
                    sigDBCBParams.alphaR_max = 5.;
                    sigDBCBParams.nR = 1.85;
                    sigDBCBParams.nR_min = 1.1;
                    sigDBCBParams.nR_max = 20;
                    
                    
                    PDFParams::PhenomenologicalParams bkgD0dstParams; 

                    bkgD0dstParams.p0 = 0.01;
                    bkgD0dstParams.p0_min = 0.001;
                    bkgD0dstParams.p0_max = 0.1;
                    bkgD0dstParams.p1 = 1;
                    bkgD0dstParams.p1_min = -10;
                    bkgD0dstParams.p1_max = 10.0;
                    bkgD0dstParams.p2 = 1;
                    bkgD0dstParams.p2_min = 10.0;
                    bkgD0dstParams.p2_max = 10;

                    std::cout << "Attempting to fit mass for slice: " << sliceName << std::endl;
                    
                    // Enhanced MassFitterV2 integration with proper error handling
                    try {
                        // Create MassFitterV2 instance with dependency injection
                        auto massFitter = createMassFitterV2ForSlice(sliceName);
                        
                        // Setup configuration
                        FitConfig massFitConfig = createMassFitConfig(sliceName);
                        gSystem->mkdir(massFitConfig.outputDir.c_str(), kTRUE);
                        
                        massFitter->SetConfiguration(massFitConfig); 
                        // Set data for fitting
                        massFitter->SetData(dcaSliceData);
                        
                        // Use template-based fitting approach
                        bool fitSuccess = false;
                        if (dcaSliceMC) {
                            // Perform constraint fitting with MC
                            fitSuccess = massFitter->PerformConstraintFit(
                                massFitConfig, dcaSliceData, dcaSliceMC,
                                sigDBCBParams, bkgD0dstParams,
                                {}, sliceName
                            );
                        } else {
                            // Perform regular fitting without MC constraint
                            fitSuccess = massFitter->PerformFit(
                                massFitConfig, dcaSliceData,
                                sigDBCBParams, bkgD0dstParams, sliceName
                            );
                        }


                        // Enhanced result handling with MassFitterV2
                        if (fitSuccess && massFitter->GetSignalYield(sliceName) >= 0) {
                            yield = massFitter->GetSignalYield(sliceName);
                            yieldError = massFitter->GetSignalYieldError(sliceName);
                            std::cout << "Mass fit SUCCESS for DCA slice " << sliceName 
                                     << ": Yield = " << yield << " +/- " << yieldError << std::endl;

                            // Store results
                            sliceFitResults_[sliceName] = massFitter->GetFitResults(sliceName);
                            
                            // Update yield histogram
                            updateYieldHistogram(currentBin, yield, yieldError);

                            // Get enhanced result objects
                            RooWorkspace* mfWs = massFitter->GetWorkspace(sliceName);
                            RooFitResult* mfFitResult = massFitter->GetRooFitResult(sliceName);

                            if (mfWs && mfFitResult) {
                                // Process successful fit results 
                            // RooRealVar* sliceMassVar = mfWs->var(massVar_->GetName());

                            notifyDCASliceFit(currentBin, sliceInfo, mfFitResult, "DCA_Mass_Slice");

                            plotDir += Form("pt%.0f_%.0f_cos%2f_%2f/", opt_.pTMin, opt_.pTMax,opt_.cosMin*100,opt_.cosMax*100);
                            std::string detailedPlotOutputDir = plotDir; 
                            
                            // --- Prepare strings for the plot labels ---
                            std::string particleTypeLabel = "D^{*+}";
                            std::string energyLabel = "ppRef #sqrt{s_{NN}} = 5.36 TeV"; // Adjust as needed
                            // Use legend strings from opt_ struct instead of hardcoded strings
                            std::string ptLabel = opt_.pTLegend.empty() ? Form("%.1f < p_{T} < %.1f GeV/c", opt_.pTMin, opt_.pTMax) : opt_.pTLegend;
                            std::string yLabel = opt_.yLegend.empty() ? Form("|y| < %.1f", opt_.etaMax) : opt_.yLegend;
                            std::string analysisCutLabel = opt_.centLegend.empty() ? Form("%.2f < cos#theta_{HX} < %.2f", opt_.cosMin, opt_.cosMax) : opt_.centLegend;
                            std::string dcaLabel = opt_.dcaLegend.empty() ? Form("%.3f < DCA < %.3f cm", dcaLow, dcaHigh) : opt_.dcaLegend;

                            // --- Call the new plotting function ---
                                         plotSliceFitDetails(dcaSliceData, mfWs, mfFitResult, massVar_.get(),
                                                sliceName, massFitOpt, detailedPlotOutputDir,false,
                                                particleTypeLabel, energyLabel, ptLabel, yLabel, analysisCutLabel, dcaLabel);
                                         plotSliceFitDetails(dcaSliceMC, mfWsMC, mfFitResultMC, massVar_.get(),
                                                sliceName, massFitOpt, detailedPlotOutputDir,true,
                                                particleTypeLabel, energyLabel, ptLabel, yLabel, analysisCutLabel, dcaLabel);

                            // Store path for the final summary plot
                            // std::string detailedPlotFileName = detailedPlotOutputDir + "mass_fit_detailed_" + sliceName + ".png"; 
                            // detailedPlotPaths_.push_back(detailedPlotFileName);
                        } else {
                            std::cerr << "Error: Could not retrieve workspace or fit result from MassFitter for slice " << sliceName << std::endl;
                        }
                    }
                     else {
                                FitStatus failedStatus(-998, "DCA_Mass_Slice", "Slice fit failed", -1, -1, sliceInfo.sliceName);
        addFitStatus(currentBin, failedStatus);
                        std::cout << "Mass fit FAILED for DCA slice " << sliceName << ". Yield set to 0." << std::endl;
                        yield = 0; 
                        yieldError = 0; 
                        fitSuccess = false;
                    }
                }
                
                     else {
                             std::cerr << "Could not retrieve MassFitter's workspace for slice " << sliceName << std::endl;
                        }
                    

                // if (fitSuccess) {
                    dataYieldHist_->SetBinContent(i + 1, yield); 
                    dataYieldHist_->SetBinError(i + 1, yieldError);
                // } else {
                    // dataYieldHist_->SetBinContent(i + 1, 0);
                    // dataYieldHist_->SetBinError(i + 1, 1e9); 
                            } else {
                                // Handle failed fit
                                std::string errorMsg = "Failed to get valid workspace or fit result for slice: " + sliceName;
                                errorHandler_->HandleError("FitDCASlice", errorMsg);
                                fitSuccess = false;
                            }
                        } else {
                            // Handle failed fit
                            std::string errorMsg = "Mass fitting failed for slice: " + sliceName;
                            errorHandler_->HandleError("FitDCASlice", errorMsg);
                            yield = 0.0;
                            yieldError = 0.0;
                        }
                        
                    } catch (const std::exception& e) {
                        // Enhanced exception handling
                        std::string errorMsg = "Exception in mass fitting for slice " + sliceName + ": " + std::string(e.what());
                        errorHandler_->HandleError("FitDCASlice", errorMsg);
                        yield = 0.0;
                        yieldError = 0.0;
                        fitSuccess = false;
                    }
                    
                    // Cleanup
                    delete dcaSliceData;
                    if (dcaSliceMC) delete dcaSliceMC;
             // End of DCA bin loop

            delete dataDrivenTemplate_;
            dataDrivenTemplate_ = new RooDataHist("dataDrivenTemplate", "Data-driven DCA Template", RooArgList(*dca), dataYieldHist_.get());
            ws_->import(*dataDrivenTemplate_, RooFit::RecycleConflictNodes());
            std::cout << "Data-driven DCA template (RooDataHist) created from mass fits." << std::endl;

            
            std::cout << "Building model using MC templates." << std::endl;
            promptTemplate_ = static_cast<RooDataHist*>(ws_->data("promptTemplate"));
            nonPromptTemplate_ = static_cast<RooDataHist*>(ws_->data("nonPromptTemplate"));

            if (!promptTemplate_ && !nonPromptTemplate_) {
                std::cerr << "Error: No MC templates (RooDataHist) are available to build the model." << std::endl;
                std::cerr << "       Please run createTemplatesFromMC() first if you intend to use MC templates." << std::endl;
                return false;
            }
            int a =0;
            
            // RooDataSet* dataForYieldEstimate = (RooDataSet*)ws_->data("dataSet");
            // double nevt = dataForYieldEstimate ? dataForYieldEstimate->sumEntries() : 1000; 
            double nevt = dataYieldHist_->Integral();
            if (nevt <=0) nevt = 1000;
            cout << "Estimated number of events for yield: " << nevt << endl;


            // delete n_prompt_;
            n_prompt_ = std::make_unique<RooRealVar>("n_prompt", "Number of prompt events", nevt * 0.8, 0, nevt * 1);
            ws_->import(*n_prompt_);
            // delete n_nonprompt_;
            n_nonprompt_ = std::make_unique<RooRealVar>("n_nonprompt", "Number of non-prompt events", nevt * 0.2, 0.1, nevt * 1);
            ws_->import(*n_nonprompt_);
            cout << a++ << endl;

            RooArgList pdfList;
            RooArgList yieldList;
            cout << a++ << endl;


            if (promptTemplate_) {
                delete promptPdf_;
                promptPdf_ = new RooHistPdf("promptPdf", "Prompt PDF from MC", RooArgSet(*dca), *promptTemplate_);
        promptFunc_ = new RooHistFunc("promptFunc", "Prompt Function from MC", RooArgSet(*dca), *promptTemplate_);
                ws_->import(*promptPdf_, RooFit::RecycleConflictNodes());
                pdfList.add(*promptPdf_);
                yieldList.add(*n_prompt_);
                std::cout << "Prompt PDF created from MC template." << std::endl;
            }

            if (nonPromptTemplate_) {
                delete nonPromptPdf_;
                nonPromptPdf_ = new RooHistPdf("nonPromptPdf", "Non-prompt PDF from MC", RooArgSet(*dca), *nonPromptTemplate_);
        nonPromptFunc_ = new RooHistFunc("nonPromptFunc", "Non-prompt Function from MC", RooArgSet(*dca), *nonPromptTemplate_);
                ws_->import(*nonPromptPdf_, RooFit::RecycleConflictNodes());
                pdfList.add(*nonPromptPdf_);
                yieldList.add(*n_nonprompt_);
                std::cout << "Non-prompt PDF created from MC template." << std::endl;
            }

            if (pdfList.getSize() > 0) {
                // delete model_;
                model_ = new RooAddPdf("model", "Extended P+NP Model from MC", pdfList, yieldList);
                ws_->import(*model_);
                std::cout << "Extended model built successfully with " << pdfList.getSize() << " MC component(s)." << std::endl;
            } else {
                std::cerr << "Error: No MC PDFs were created to build the model." << std::endl;
                return false;
            }


        
        return true;
    }
        bool buildModelwSideband() { 
        std::cout << "Building RooFit sideband and signal model..." << std::endl;

        if (!dataSet_) { 
             dataSet_ = (RooDataSet*)ws_->data("dataSet");
             if (!dataSet_){
                std::cerr << "Error: Data set is not loaded. Cannot build model." << std::endl;
                return false;
             }
        }
         if (!dataSet_ && !ws_->data("promptDataSetMC") && !ws_->data("nonPromptDataSetMC")){
            std::cerr << "Error: No MC templates and no data loaded. Cannot build model." << std::endl;
            return false;
        }


        if (massVarName_.empty()) {
            std::cerr << "Error: Mass variable name not set. Cannot build data-driven templates." << std::endl;
            return false;
        }
        if (dcaBins_.empty() || dcaBins_.size() < 2) {
            std::cerr << "Error: dcaBins_ not properly set." << std::endl;
            return false;
        }

        RooRealVar* dca = ws_->var(dcaVar_->GetName()); 
        if (!dca) {
            dca = dcaVar_.get();
            if (!dca) {
                 std::cerr << "Error: Standard 'dca' variable not found in workspace or dcaVar_." << std::endl;
                 return false;
            }
            ws_->import(*dca); 
        }


	std::cout << "Building model using data-driven DCA templates from mass fits." << std::endl;

	// delete sigYieldHist_;
	sigYieldHist_ = std::make_unique<TH1D>("sigYieldHist", "Signal DCA Yield from Mass Fits", dcaBins_.size() - 1, dcaBins_.data());
	sigYieldHist_->Sumw2();
	// delete sbYieldHist_;
	sbYieldHist_ = std::make_unique<TH1D>("sbYieldHist", "Sideband DCA Yield from Mass Fits", dcaBins_.size() - 1, dcaBins_.data());
	sbYieldHist_->Sumw2();


	// double sbLow = 0.002;
	// double sbHigh = 0.004;
	double sbLow = 0.02;
    double sbmid = 0.04;
	double sbHigh = 0.08;
	// std::string sbCut = Form("abs(%f-mass) > %f && abs(%f-mass) < %f ", deltaM_PDG, sbLow, deltaM_PDG, sbHigh);
	// std::string sigCut = Form("abs(%f-mass) < %f ", deltaM_PDG, sbLow);
    // TString sbCut = TString::Format("abs(%s-%f) < %f && abs(%s-%f) > %f", massVar_->GetName(),deltaM_PDG, sbHigh, massVar_->GetName(),deltaM_PDG, sbLow);
    // TString sigCut = TString::Format("abs(%f-%s) < %f", deltaM_PDG,massVar_->GetName(), sbLow);
    for(int i =0; i < dcaBins_.size() - 1; ++i) {
        cout << "Processing DCA bin " << i << ": [" << dcaBins_[i] << ", " << dcaBins_[i+1] << "]" << endl;
        double dcaLow = dcaBins_[i];
        double dcaHigh = dcaBins_[i+1];
    TString dcaCut = TString::Format("%s >= %f && %s < %f", dca->GetName(), dcaLow, dca->GetName(), dcaHigh);
    TString sbCut = TString::Format("abs(%s-%f) < %f && abs(%s-%f) > %f", massVar_->GetName(),D0_PDG, sbHigh, massVar_->GetName(),D0_PDG, sbmid);
    TString sigCut = TString::Format("abs(%f-%s) < %f", D0_PDG,massVar_->GetName(), sbLow);
    // TString sigCut = TString::Format("%s < %f && %s > %f", massVar_->GetName(), sbLow, massVar_->GetName(), sbHigh);
    // TString sbCut = TString::Format("%s < 0.148 && %s > 0.144",massVar_->GetName(), massVar_->GetName());
    // TString sigCut = TString::Format("%s < 0.148 && %s > 0.144",massVar_->GetName(), massVar_->GetName());

	RooDataSet* SliceSig = dynamic_cast<RooDataSet*>(dataSet_->reduce((dcaCut+"&&" +sigCut).Data()));
	RooDataSet* SliceSB = dynamic_cast<RooDataSet*>(dataSet_->reduce((dcaCut+"&&"+sbCut).Data()));
    cout << "sliceSig Cut " << dcaCut << " && " << sigCut << endl;
    cout << "sliceSB Cut " << dcaCut << " && " << sbCut << endl;
    cout << " DCA bin: " << i << " has " << SliceSig->numEntries() << " signal entries and " << SliceSB->numEntries() << " sideband entries." << endl;
    double yieldsig = SliceSig ? SliceSig->sumEntries() : 0;
    double yieldsigError = SliceSig ? sqrt(SliceSig->sumEntries()) : 0;
                    sigYieldHist_->SetBinContent(i + 1, yieldsig);
                    sigYieldHist_->SetBinError(i + 1, yieldsigError);
    double yieldsb = SliceSB ? SliceSB->sumEntries() : 0;
    double yieldsbError = SliceSB ? sqrt(SliceSB->sumEntries()) : 0;
                    sbYieldHist_->SetBinContent(i + 1, yieldsb); 
                    sbYieldHist_->SetBinError(i + 1, yieldsbError);

        // for (int i = 0; i < SliceSig->numEntries(); ++i) {
        //     const RooArgSet* row = SliceSig->get(i);
        //     sigYieldHist_->Fill(row->getRealValue(dcaVar_->GetName()));
        // }
    //     for (int i = 0; i < SliceSB->numEntries(); ++i) {
	// const RooArgSet* row = SliceSB->get(i);
	// sbYieldHist_->Fill(row->getRealValue(dcaVar_->GetName()));
	// }
}
    sbYieldHist_->Scale(1/2.0); // Scale sideband yield to match signal yield
    // sbYieldHist_->Scale(1.0, "width"); 
    // sigYieldHist_->Scale(1.0, "width"); // DCA bin 폭
    dataYieldHist_.reset(static_cast<TH1D*>(sigYieldHist_->Clone()));
    dataYieldHist_->Add(sbYieldHist_.get(), -1);
    // dataYieldHist_->Scale(1.0, "width"); 
    sbDrivenTemplate_ = new RooDataHist("sbDrivenTemplate", "Sideband Driven DCA Template", RooArgSet(*dca), sbYieldHist_.get());
    ws_->import(*sbDrivenTemplate_);
    sigDrivenTemplate_ = new RooDataHist("sigDrivenTemplate", "Signal Driven DCA Template", RooArgSet(*dca), sigYieldHist_.get());
    ws_->import(*sigDrivenTemplate_);
    plotSignalAndSidebandDCAFromHist(Form("dca_sig_vs_sb_hist_plot_pT%.1f_%.1f_cos_%3f_%3f", (opt_.pTMin), (opt_.pTMax), opt_.cosMin*100, opt_.cosMax*100));
    dataDrivenTemplate_= new RooDataHist("dataDrivenTemplate", "Data Driven DCA Template", RooArgSet(*dca), dataYieldHist_.get()); 
    cout << "Data-driven DCA template created with " << dataYieldHist_->GetEntries() << " entries." << endl;
    cout << "Signal DCA template created with " << sigYieldHist_->GetEntries() << " entries." << endl;
    cout << "Sideband DCA template created with " << sbYieldHist_->GetEntries() << " entries." << endl;
    


	promptTemplate_ = static_cast<RooDataHist*>(ws_->data("promptTemplate"));
	nonPromptTemplate_ = static_cast<RooDataHist*>(ws_->data("nonPromptTemplate"));

	if (!promptTemplate_ && !nonPromptTemplate_) {
		std::cerr << "Error: No MC templates (RooDataHist) are available to build the model." << std::endl;
		std::cerr << "       Please run createTemplatesFromMC() first if you intend to use MC templates." << std::endl;
		return false;
	}
	int a =0;
	double nevt = dataYieldHist_->Integral();
	if (nevt <=0) nevt = 1000;


	// delete n_prompt_;
	n_prompt_ = std::make_unique<RooRealVar>("n_prompt", "Number of prompt events", nevt * 0.5, 0, nevt * 1);
	ws_->import(*n_prompt_, RooFit::RecycleConflictNodes());
	// delete n_nonprompt_;
	n_nonprompt_ = std::make_unique<RooRealVar>("n_nonprompt", "Number of non-prompt events", nevt * 0.5, 0, nevt * 1);
	ws_->import(*n_nonprompt_, RooFit::RecycleConflictNodes());

	RooArgList pdfList;
	RooArgList yieldList;


	if (promptTemplate_) {
		delete promptPdf_;
		promptPdf_ = new RooHistPdf("promptPdf", "Prompt PDF from MC", RooArgSet(*dca), *promptTemplate_);
        promptFunc_ = new RooHistFunc("promptFunc", "Prompt Function from MC", RooArgSet(*dca), *promptTemplate_);
		ws_->import(*promptPdf_, RooFit::RecycleConflictNodes());
		pdfList.add(*promptPdf_);
		yieldList.add(*n_prompt_);
		std::cout << "Prompt PDF created from MC template." << std::endl;
	}

	if (nonPromptTemplate_) {
		delete nonPromptPdf_;
		nonPromptPdf_ = new RooHistPdf("nonPromptPdf", "Non-prompt PDF from MC", RooArgSet(*dca), *nonPromptTemplate_);
        nonPromptFunc_ = new RooHistFunc("nonPromptFunc", "Non-prompt Function from MC", RooArgSet(*dca), *nonPromptTemplate_);
		ws_->import(*nonPromptPdf_, RooFit::RecycleConflictNodes());
		pdfList.add(*nonPromptPdf_);
		yieldList.add(*n_nonprompt_);
		std::cout << "Non-prompt PDF created from MC template." << std::endl;
	}

	if (pdfList.getSize() > 0) {
		// delete model_;
		model_ = new RooAddPdf("model", "Extended P+NP Model from MC", pdfList, yieldList);
		ws_->import(*model_);
		std::cout << "Extended model built successfully with " << pdfList.getSize() << " MC component(s)." << std::endl;
	} else {
		std::cerr << "Error: No MC PDFs were created to build the model." << std::endl;
		return false;
	}

    if(nonPromptFunc_ && promptFunc_) {
        ws_->import(*promptFunc_);
        ws_->import(*nonPromptFunc_);
    }

	return true;
	}
    RooFitResult* performFit(bool useMinos = false) {
        std::cout << "Performing fit..." << std::endl;
        if (!model_ || !dataSet_) {
            std::cerr << "Error: Model or dataset not available for fitting." << std::endl;
            return nullptr;
        }
        // Ensure dcaVar_ has the standard name 'dca' before fitting
        dcaVar_->SetName((opt_.dcaVar).c_str());
        //TH1D* tempDataHist = new TH1D("tempDataHistForBinning", "Temporary Data Hist for Binning",dcaBins_.size() - 1, dcaBins_.data());
        //dataSet_->fillHistogram(tempDataHist, RooArgList(*dcaVar_));

        //RooDataHist dataHist("dataHist", "Binned Data", RooArgList(*dcaVar_), tempDataHist);
        //delete tempDataHist; // Clean up temporary TH1


        // Fit options using RooLinkedList
        RooLinkedList fitOptionsList;
        fitOptionsList.Add(RooFit::Save(true).Clone()); // Save detailed fit result
        fitOptionsList.Add(RooFit::PrintLevel(-1).Clone()); // Reduce verbosity (-1), default is 1
        fitOptionsList.Add(RooFit::Extended(true).Clone());
        fitOptionsList.Add(RooFit::Minimizer("Minuit", "Minimuzer").Clone()); // Use Minuit2 with Migrad algorithm
        fitOptionsList.Add(RooFit::Hesse(true).Clone()); // Use Hesse to compute errors
        fitOptionsList.Add(RooFit::Optimize(true).Clone()); // Use Minuit2 with Migrad algorithm
        fitOptionsList.Add(RooFit::SumW2Error(true).Clone()); // Use weighted likelihood / correct errors
        RooFitResult* fitResult = model_->fitTo(*dataDrivenTemplate_, fitOptionsList);
        fitResult->Print("v");


            int ntry = 0;
    while(((fitResult->statusCodeHistory(0) != 0) || (fitResult->statusCodeHistory(1) != 0)) && (ntry)<3){
    delete fitResult;
    RooLinkedList fitOptstemp = fitOptionsList;;
    // fitOptstemp.Add(new RooCmdArg(RooFit::NumCPU(24)));
    // fitOptstemp.Add(new RooCmdArg(RooFit::Minimizer("Minuit","Minimizer")));
    // #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    // fitOptstemp.Add(new RooCmdArg(RooFit::EvalBackend(useCUDA ? "cuda" : "CPU")));
    // #endif
    // fitOptstemp.Add(new RooCmdArg(RooFit::Extended(true)));
    // fitOptstemp.Add(new RooCmdArg(RooFit::Save(true)));
    // fitOptstemp.Add(new RooCmdArg(RooFit::Minos(useMinos)));
    // fitOptstemp.Add(new RooCmdArg(RooFit::Optimize(true)));
    // fitOptstemp.Add(new RooCmdArg(RooFit::Hesse(useHesse)));
    // fitOptstemp.Add(new RooCmdArg(RooFit::PrintLevel(verbose ? 1 : -1)));
    // fitOptstemp.Add(new RooCmdArg(RooFit::Range("analysis")));
    fitOptstemp.Add(new RooCmdArg(RooFit::Strategy(2-ntry))); 
        std::cout << "Initial fit failed with status " << fitResult->statusCodeHistory(0) << " and " << fitResult->statusCodeHistory(1)
                  << ". Retrying..." << std::endl;
        RooFitResult* fitResult = model_->fitTo(*dataDrivenTemplate_, fitOptstemp);
    cout << " #### Fit attempt " << ntry + 1 << " completed with status: " << fitResult->status() << "####" << std::endl;
        fitResult->Print("v");
        ntry++;
    }
        // coef_prompt_ = new RooRealVar("coef_prompt", "Prompt Coefficient", fracPrompt_->getVal()/promptTemplate_->sum(false));
        // coef_nonprompt_ = new RooRealVar("coef_nonprompt", "Non-Prompt Coefficient", (1 - fracPrompt_->getVal())/nonPromptTemplate_->sum(false));

        // sumFunc_ = new RooRealSumFunc("sumFunc", "Sum of Prompt and Non-Prompt Functions", RooArgList(*promptFunc_, *nonPromptFunc_), RooArgList(*coef_prompt_, *coef_nonprompt_));



        // if (useMinos) {
        //     fitOptionsList.Add(RooFit::Minos(true).Clone());
        //     std::cout << "Using MINOS for error estimation." << std::endl;
        // }
        // if (dataSet_->isWeighted()) {
        //      std::cout << "Using SumW2Error(true) for weighted dataset." << std::endl;
        // }

        // Perform the fit using the RooLinkedList
        // Ensure the model is fitted to the dataset using the correct variable ('dca')

        fitOptionsList.Add(RooFit::Strategy(2).Clone()); // Set strategy to 2 for better convergence

        if (fitResult) {
            fitResult->Print("v"); // Print verbose fit results
            std::cout << "Fit completed with status: " << fitResult->status() << std::endl;
            // Check covariance matrix quality
             std::cout << "Fit result covQual: " << fitResult->covQual() << std::endl;
             if(fitResult->covQual() < 3) {
                 std::cerr << "Warning: Covariance matrix quality is low (" << fitResult->covQual() << "). Fit might be unreliable." << std::endl;
             }
        } else {
            std::cerr << "Error: Fit failed to produce a result object." << std::endl;
        }
    //     if (promptHist_) { // Check if it was created
    //         promptHist->Scale(1.0, "width"); // Scale prompt histogram by bin width
    //         // delete promptHist_;
    //         // promptHist_ = nullptr;
    //    }
    //    if (nonPromptHist_) { // Check if it was created
    //         nonPromptHist->Scale(1.0, "width"); // Scale non-prompt histogram by bin width
    //         // delete nonPromptHist_;
    //         // nonPromptHist_ = nullptr;
    //    }
      
    //  n_nonprompt_->setVal(1e-7);
     fracPrompt_ = new RooFormulaVar("fracPrompt", "Prompt Fraction from MC fit", "@0 / (@0 + @1)", RooArgList(*n_prompt_, *n_nonprompt_));
     
        ws_->import(*fitResult); // Import fit result into workspace
        ws_->import(*fracPrompt_); // Import prompt fraction into workspace


        return fitResult; // Caller is responsible for deleting this object
    }
   void plotResults(RooFitResult* fitResult = nullptr, const std::string& plotName = "dca_fit_plot", bool useDataTemplates = true) {
    std::cout << "Plotting results with pull distribution (CMS style)..." << std::endl;
    RooRealVar* dca = ws_->var(opt_.dcaVar.c_str()); 
    if (!dca) {
        dca = dcaVar_.get(); 
        if (!dca) {
             std::cerr << "Error: Standard 'dca' variable not found in workspace or dcaVar_ for plotting." << std::endl;
             return;
        }
    }

    if (!model_) {
        std::cerr << "Error: Model not available for plotting." << std::endl;
        return;
    }

    TCanvas* c = new TCanvas(plotName.c_str(), "DCA Fit Results with Pull", 800, 800);
    
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
                std::cerr << "Error: Original dataset not available for plotting." << std::endl;
                delete c;
                return;
            }
        }
        
        dataSet_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_original"), RooFit::DataError(RooAbsData::SumW2));

        model_->plotOn(frame, RooFit::Name("model_fit_to_data"), RooFit::LineColor(kRed + 1));

        RooAbsPdf* promptPdf = ws_->pdf("promptPdf");
        RooAbsPdf* nonPromptPdf = ws_->pdf("nonPromptPdf");

        if (nonPromptPdf) {
            model_->plotOn(frame, RooFit::Components(*nonPromptPdf), RooFit::Name("nonprompt_comp"), 
                          RooFit::FillStyle(3354), RooFit::FillColor(kBlue - 9), RooFit::LineColor(kBlue + 1), RooFit::DrawOption("F"));
        }
        if (promptPdf) {
            model_->plotOn(frame, RooFit::Components(*promptPdf), RooFit::Name("prompt_comp"), 
                          RooFit::FillStyle(3345), RooFit::FillColor(kRed - 9), RooFit::LineColor(kRed + 1), RooFit::DrawOption("F"));
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

    if (useDataTemplates) {
        TObject* dataYieldObj = frame->findObject("data_yield_hist");
        TObject* modelFitObj = frame->findObject("model_fit_to_mc");
        TObject* mcPromptObj = frame->findObject("mc_prompt_comp");
        TObject* mcNonPromptObj = frame->findObject("mc_nonprompt_comp");
        TObject* myObject = nullptr;

        if (dataYieldObj) leg->AddEntry(dataYieldObj, "Data Yield (from Mass Fit)", "pe");
        if (modelFitObj) leg->AddEntry(modelFitObj, "Total MC Fit", "l");
        if (mcPromptObj) leg->AddEntry(mcPromptObj, "Prompt D^{*} (MC)", "f");
        if (mcNonPromptObj) leg->AddEntry(mcNonPromptObj, "Non-Prompt D^{*} (MC)", "f");
        leg->AddEntry(myObject, Form("Prompt Fraction = %.2f", fracPrompt_->getVal()));
    } else {
        TObject* dataOrigObj = frame->findObject("data_original");
        TObject* modelFitToDataObj = frame->findObject("model_fit_to_data");
        TObject* promptCompObj = frame->findObject("prompt_comp");
        TObject* nonPromptCompObj = frame->findObject("nonprompt_comp");
        TObject* myObject = nullptr;

        if (dataOrigObj) leg->AddEntry(dataOrigObj, "Data", "pe");
        if (modelFitToDataObj) leg->AddEntry(modelFitToDataObj, "Total Fit", "l");
        if (promptCompObj) leg->AddEntry(promptCompObj, "Prompt D^{*} (MC)", "f");
        if (nonPromptCompObj) leg->AddEntry(nonPromptCompObj, "Non-Prompt D^{*} (MC)", "f");
        leg->AddEntry(myObject, Form("Prompt Fraction = %.2f", fracPrompt_->getVal()));
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
        std::cerr << "Warning: Could not create pull histogram" << std::endl;
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
        std::cerr << "Warning: Could not create ratio plot. Data or fit curve not found." << std::endl;
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
         std::cout << "Saving results to " << outputFileName_ << std::endl;
         if (outputFileName_.empty()) {
             std::cerr << "Error: Output file name not set. Cannot save results." << std::endl;
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
             std::cerr << "Error: Could not open or write to output file: " << outputFileName_ << std::endl;
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
             std::cout << "Workspace saved." << std::endl;
         }

         // Write fit result
         if (fitResult) {
             fitResult->Write("fitResult");
             std::cout << "Fit result saved." << std::endl;
         }

         // Note: Plots are saved in their respective functions.

         // Close the file now that everything is saved
         outFile_->Close();
        //  delete outFile_;
         outFile_ = nullptr; // Reset pointer after closing and deleting
         std::cout << "Results saved and file closed." << std::endl;
    }

    // --- New method to plot raw data distribution ---
    void plotRawDataDistribution(const std::string& plotName = "dca_raw_data_plot");


private:
    // --- Internal Helper Methods ---
    // loadDataSet helper is integrated into createTemplatesFromMC and loadData for clarity
    RooAbsPdf* ExtractComponent(RooAbsPdf* pdf_, const std::string& namePattern) {
    if (std::string(pdf_->GetName()).find(namePattern) != std::string::npos) {
    }

    const RooArgSet* components = pdf_->getComponents();
    RooAbsPdf* foundPdf = nullptr;

    if (components) {
        for (const auto& obj : *components) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(obj);
                std::cout << "Found component '" << namePattern << "': " << pdfComponent->GetName() << std::endl;
            if (pdfComponent && std::string(pdfComponent->GetName()).find(namePattern) != std::string::npos) {
                foundPdf = pdfComponent;
                // std::cout << "Found component '" << namePattern << "': " << foundPdf->GetName() << std::endl;
                break;
            }
        }
    }

    if (!foundPdf) {
         std::cout << "Warning: Component containing '" << namePattern << "' not found." << std::endl;
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
        RooDataSet* sliceMCData = nullptr,  // Optional: MC data for the same slice
        RooWorkspace* mcFitWs = nullptr,    // Optional: MC fit workspace
        RooFitResult* mcFitResult = nullptr // Optional: MC fit result
    ) {
        // --- Initial Checks ---
        RooAbsPdf* sliceModel = fitWs->pdf("total_pdf");
        RooRealVar* sliceMassVar = fitWs->var(massVar->GetName());
        if(!sliceFitResult)
        {
            std::cerr << "Error: Fit result not provided for slice " << sliceName << std::endl; 
            return;
        }
        if (!sliceData) {
            std::cerr << "Error: Slice data not provided for slice " << sliceName << std::endl;
            return;
        }
        if (!sliceModel) {
            std::cerr << "Error: Model not found in workspace for slice " << sliceName << std::endl;
            return;
        }

        // if (!sliceModel || !sliceMassVar || !sliceFitResult) {
        //     std::cerr << "Error in plotSliceFitDetails: Model, Mass Variable, or Fit Result not found for slice " << sliceName << std::endl;
        //     return;
        // }

        // --- Canvas and Pad Setup ---
        TCanvas* canvas = new TCanvas(Form("c_slice_detail_%s", sliceName.c_str()), Form("Detailed Mass Fit for %s", sliceName.c_str()), 1000, 800);
        
        // [ Main Plot Pad | Parameter Pad ]
        // [   Pull Pad    | Parameter Pad ]
        TPad* mainPad = new TPad(Form("mainPad_%s", sliceName.c_str()), "", 0.0, 0.25, 0.75, 1.0);
        TPad* pullPad = new TPad(Form("pullPad_%s", sliceName.c_str()), "", 0.0, 0.0, 0.75, 0.25);
        TPad* paramPad = new TPad(Form("paramPad_%s", sliceName.c_str()), "", 0.75, 0.0, 1.0, 1.0);

        mainPad->SetMargin(0.15, 0.02, 0.03, 0.1); // L, R, B, T
        pullPad->SetMargin(0.15, 0.02, 0.35, 0.02);
        paramPad->SetMargin(0.1, 0.1, 0.05, 0.05);

        mainPad->Draw();
        pullPad->Draw();
        paramPad->Draw();

        // --- Main Plot ---
        mainPad->cd();
        RooPlot* frame = sliceMassVar->frame(RooFit::Title(""));
        
        // Plot data points
        sliceData->plotOn(frame, RooFit::Name("datapoints"), RooFit::MarkerStyle(kFullCircle), RooFit::MarkerSize(0.8), RooFit::DataError(RooAbsData::SumW2));

        // Get model components from the MassFitter's workspace

        RooAbsPdf* sigComponent = ExtractComponent(sliceModel, "sig");
        RooAbsPdf* bkgComponent = ExtractComponent(sliceModel, "bkg");

        // RooAbsPdf* bkgComponent = fitWs->pdf("bkg_pdf");
        // RooAbsPdf* sigComponent = fitWs->pdf("sig_pdf");
        
        // Plot components first (background, then signal)
        if (bkgComponent) {
            sliceModel->plotOn(frame, RooFit::Components(*bkgComponent), RooFit::Name("background"),
                               RooFit::LineStyle(kDashed), RooFit::LineColor(kBlue + 2), RooFit::LineWidth(2));
        }
        if (sigComponent) {
            sliceModel->plotOn(frame, RooFit::Components(*sigComponent), RooFit::Name("signal"),
                               RooFit::FillColor(kAzure - 9), RooFit::FillStyle(3354), RooFit::DrawOption("F"),
                               RooFit::LineColor(kAzure - 9), RooFit::VLines()); // VLines removes border
        }

        // Plot total fit line over the components
        sliceModel->plotOn(frame, RooFit::Name("model"), RooFit::LineColor(kRed), RooFit::LineWidth(2));

        // Re-plot data points on top of the filled areas
        sliceData->plotOn(frame, RooFit::Name("datapoints_top"), RooFit::MarkerStyle(kFullCircle), RooFit::MarkerSize(0.8), RooFit::DataError(RooAbsData::SumW2));

        // Formatting the frame
        double binWidth = sliceData->get()->getRealValue(sliceMassVar->GetName(), 0, 1) - sliceData->get()->getRealValue(sliceMassVar->GetName(), 0, 0);
        if(frame->GetXaxis()->GetBinWidth(1) > 0) binWidth = frame->GetXaxis()->GetBinWidth(1);

        frame->GetYaxis()->SetTitle(Form("Entries / (%.3f GeV/c^{2})", binWidth));
        frame->GetYaxis()->SetTitleOffset(1.4);
        frame->GetYaxis()->SetTitleSize(0.055);
        frame->GetYaxis()->SetLabelSize(0.045);
        frame->GetXaxis()->SetLabelSize(0); // Hide X-axis labels on main plot
        frame->GetXaxis()->SetTitleSize(0); // Hide X-axis title on main plot
        frame->SetTitle("");
        frame->Draw();

        // --- Legend ---
        TLegend* legend = new TLegend(0.58, 0.60, 0.95, 0.88);
        legend->SetBorderSize(0); legend->SetFillStyle(0); legend->SetTextSize(0.045);
        legend->AddEntry(frame->findObject("datapoints_top"), "Data", "PE");
        legend->AddEntry(frame->findObject("model"), "Fit", "L");
        if (sigComponent) legend->AddEntry(frame->findObject("signal"), Form("%s Signal", particleType.c_str()), "F");
        if (bkgComponent) legend->AddEntry(frame->findObject("background"), "Combinatorial", "L");
        legend->Draw();

        // --- CMS and Kinematic Labels ---
        TLatex latex; latex.SetNDC(); latex.SetTextFont(42);
        latex.SetTextSize(0.05);
        latex.DrawLatex(0.18, 0.93, "#bf{CMS} #it{Preliminary}");

        latex.SetTextSize(0.045);
        latex.SetTextAlign(31); // Align right
        latex.DrawLatex(0.97, 0.92, energyStr.c_str());
        
        latex.SetTextAlign(11); // Align left
        latex.SetTextSize(0.04);
        float currentY = 0.83;
        latex.DrawLatex(0.18, currentY, ptRangeStr.c_str());    currentY -= 0.05;
        latex.DrawLatex(0.18, currentY, yRangeStr.c_str());     currentY -= 0.05;
        latex.DrawLatex(0.18, currentY, analysisCutStr.c_str()); currentY -= 0.05;
        latex.DrawLatex(0.18, currentY, dcaRangeStr.c_str());

        // --- Pull Plot ---
        pullPad->cd();
        RooPlot* pullFrame = sliceMassVar->frame(RooFit::Title(""));
        RooHist* hpull = frame->pullHist("datapoints_top", "model", true); // Use normalized residuals
        if (hpull) {
            pullFrame->addPlotable(hpull, "P");
            pullFrame->SetMinimum(-3.9); pullFrame->SetMaximum(3.9);
            pullFrame->GetYaxis()->SetTitle("Pull");
            pullFrame->GetYaxis()->SetTitleSize(0.12);
            pullFrame->GetYaxis()->SetLabelSize(0.1);
            pullFrame->GetYaxis()->SetTitleOffset(0.5);
            pullFrame->GetYaxis()->SetNdivisions(505);
            pullFrame->GetXaxis()->SetTitle(Form("%s (GeV/c^{2})", sliceMassVar->GetTitle()));
            pullFrame->GetXaxis()->SetTitleSize(0.12);
            pullFrame->GetXaxis()->SetLabelSize(0.1);
            pullFrame->GetXaxis()->SetTitleOffset(1.1);
            pullFrame->SetTitle("");
            pullFrame->Draw();

            TLine *lineAtZero = new TLine(sliceMassVar->getMin(), 0, sliceMassVar->getMax(), 0);
            lineAtZero->SetLineColor(kRed); lineAtZero->SetLineStyle(kDashed);
            lineAtZero->Draw("same");
        }
        
        // --- Parameter Pad ---
        paramPad->cd();
        TLatex paramLatex; paramLatex.SetNDC(); paramLatex.SetTextFont(42);
        paramLatex.SetTextSize(0.06);
        paramLatex.SetTextAlign(12); // Align left, center Y
        
        float y_pos = 0.90f;
        
        // Display chi-squared / NDF
        double chi2ndf = frame->chiSquare("model", "datapoints_top", sliceFitResult->floatParsFinal().getSize());
        paramLatex.DrawLatex(0.1, y_pos, Form("#chi^{2}/NDF = %.2f", chi2ndf));
        y_pos -= 0.08;
        paramLatex.DrawLatex(0.1, y_pos, Form("%s : %d, %s : %d", sliceFitResult->statusLabelHistory(0), 
                                                sliceFitResult->statusCodeHistory(0),
                                                sliceFitResult->statusLabelHistory(1), 
                                                sliceFitResult->statusCodeHistory(1))); 
                                                // sliceFitResult->statusLabelHistory(2),
                                                // sliceFitResult->statusCodeHistory(2)));

                                                // sliceFitResult->statusCodeHistory(2)));
        y_pos -= 0.08;

        // Display Signal and Background Yields
        const RooArgList& finalParams = sliceFitResult->floatParsFinal();
        RooRealVar* nsig = dynamic_cast<RooRealVar*>(finalParams.find("nsig"));
        if (nsig) {
             paramLatex.DrawLatex(0.1, y_pos, Form("N_{sig} = %.0f #pm %.0f", nsig->getVal(), nsig->getError()));
             y_pos -= 0.08;
        }
        RooRealVar* nbkg = dynamic_cast<RooRealVar*>(finalParams.find("nbkg"));
         if (nbkg) {
             paramLatex.DrawLatex(0.1, y_pos, Form("N_{bkg} = %.0f #pm %.0f", nbkg->getVal(), nbkg->getError()));
             y_pos -= 0.08;
        }

        // Display other floating parameters
        paramLatex.SetTextSize(0.05);
        for (int i = 0; i < finalParams.getSize(); ++i) {
            RooRealVar* var = dynamic_cast<RooRealVar*>(finalParams.at(i));
            if (var) {
                std::string varName = var->GetName();
                // Don't re-print yields
                if (varName == "nsig" || varName == "nbkg") continue;
                if (y_pos < 0.1) break; // Stop if we run out of space
                paramLatex.DrawLatex(0.1, y_pos, Form("%s = %.3g #pm %.1e", var->GetTitle(), var->getVal(), var->getError()));
                y_pos -= 0.06;
            }
        }

        // --- Save Canvas ---
        gSystem->mkdir(outputDir.c_str(), kTRUE); // Ensure output directory exists
        std::string canvasName = outputDir + "mass_fit_detailed_" + sliceName + "_" +std::to_string(isMC) + ".png";
        canvas->SaveAs(canvasName.c_str());
        std::cout << "Saved detailed mass fit for slice " << sliceName << " to " << canvasName << std::endl;

        // --- Clean up memory ---
        delete canvas;
        delete frame;
        delete pullFrame;
        delete legend;
        // hpull is owned by pullFrame, no need to delete
    }

    // --- Member Variables ---
    std::string name_;
    RooWorkspacePtr ws_; // RooFit workspace
    std::vector<double> dcaBins_ = {0,0.001,0.0023,0.0039,0.0059,0.0085,0.0160,0.0281,0.0476,0.1};
    // vector<double> dcaBins_ = {0,0.1};
    FitOpt opt_;

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
    std::string dataFileName_;
    std::string dataTreeName_;
    std::string dataRooDatsetName_;
    std::string dcaBranchName_;
    std::string motherPdgIdBranchName_;
    std::string weightBranchName_; // Name of the weight branch in Trees
    std::vector<int> promptPdgIds_;
    std::vector<int> nonpromptPdgIds_;
    std::string mcCuts_;
    std::string dataCuts_;
    std::string outputFileName_;
    static constexpr double deltaM_PDG = 0.1454258; 
    static constexpr double D0_PDG = 1.86483; 

    // Datasets and Templates (managed by smart pointers and workspace)
    RooDataSet* dataSet_;           // Data to be fitted (external reference)
    RooDataSetPtr fullMCDataSet; // Full MC dataset
    RooDataHist* promptTemplate_;   // Prompt MC template (workspace managed)
    TH1* promptHist_; // Histogram for prompt template (manually managed)
    TH1* nonPromptHist_; // Histogram for non-prompt template (manually managed)
    RooDataHist* nonPromptTemplate_; // Non-prompt MC template (workspace managed)
    std::string massVarName_;
    double massMin_;
    double massMax_;
    std::string massUnit_;


    // PDFs and Model (managed by smart pointers and workspace)
    RooHistPdf* promptPdf_;         // PDF from prompt template (workspace managed)
    RooHistPdf* nonPromptPdf_;      // PDF from non-prompt template (workspace managed)  
    RooRealVar* coef_prompt_;
    RooRealVar* coef_nonprompt_;
    RooHistFunc* promptFunc_ = nullptr;         // PDF from prompt template (workspace managed)
    RooHistFunc* nonPromptFunc_= nullptr;      // PDF from non-prompt template (workspace managed)  
    RooRealSumFunc *sumFunc_;
    RooFormulaVar* fracPrompt_;        // Fraction of prompt component (workspace managed)
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
    std::unique_ptr<ErrorHandler> errorHandler_;
    
    // Result storage
    std::map<std::string, FitResults*> sliceFitResults_;
    std::map<std::string, std::unique_ptr<MassFitterV2>> sliceFitters_;
    
    // MC dataset for constraint fitting
    RooDataSet* mcDataSet_ = nullptr;

    void Clear() {
        std::cout << "Clearing DCAFitter state..." << std::endl;
        
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
    
    void initializeConfiguration() {
        // Set up default configuration if not provided
        if (configManager_) {
            FitConfig defaultConfig;
            defaultConfig.strategyName = "Robust";
            defaultConfig.useMinos = opt_.useMinos;
            defaultConfig.useHesse = opt_.useHesse;
            defaultConfig.verbose = true;
            
            configManager_->SetConfiguration("DCAFitting", defaultConfig);
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
    std::unique_ptr<MassFitterV2> createMassFitterV2ForSlice(const std::string& sliceName) {
        std::string fitterName = opt_.name + "_" + sliceName;
        
        auto massFitter = std::make_unique<MassFitterV2>(
            fitterName, 
            massVar_->GetName(),
            massVar_->getMin(),
            massVar_->getMax()
        );
        
        // Store for later access
        sliceFitters_[sliceName] = std::move(massFitter);
        return std::move(sliceFitters_[sliceName]);
    }
    
    FitConfig createMassFitConfig(const std::string& sliceName) const {
        FitConfig config;
        if (configManager_) {
            config = configManager_->GetConfiguration("DCAFitting");
        }
        
        // Customize for mass fitting
        config.outputDir = opt_.outputDir + "/mass_fits_" + name_ + "/" + sliceName + "/";
        config.outputFile = Form("massfit_details_%s.root", sliceName.c_str());
        config.plotName = "massfit_detail_plot";
        
        return config;
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
    
    // Result management helpers
    void updateYieldHistogram(size_t binIndex, double yield, double yieldError) {
        if (dataYieldHist_ && binIndex < dcaBins_.size() - 1) {
            double binCenter = (dcaBins_[binIndex] + dcaBins_[binIndex + 1]) / 2.0;
            int histBin = dataYieldHist_->FindBin(binCenter);
            dataYieldHist_->SetBinContent(histBin, yield);
            dataYieldHist_->SetBinError(histBin, yieldError);
        }
    }
};
inline void DCAFitter::plotSignalAndSidebandDCAFromHist(const std::string& plotName) {
    std::cout << "Using existing histograms to plot DCA distribution for signal and sideband regions..." << std::endl;
    RooRealVar* dca = ws_->var(opt_.dcaVar.c_str()); 
    if (!dca) {
        std::cerr << "Error: dca3D variable not found in workspace." << std::endl;
        return;
    }
    gStyle->SetOptStat(0); 

    if (!sigDrivenTemplate_) {
        std::cerr << "Error: Signal yield histogram 'sigDrivenTemplate_' is not available." << std::endl;
        return;
    }
    if (!sbYieldHist_) {
        std::cerr << "Error: Sideband yield histogram 'sbYieldHist_' is not available." << std::endl;
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
    gSystem->mkdir(outputPath.c_str(), kTRUE);
    c->SaveAs((outputPath + plotName + ".png").c_str());
    // c->SaveAs((outputPath + plotName + ".pdf").c_str());
}



// --- Implementation of plotRawDataDistribution ---
// This function now plots normalized prompt and non-prompt MC distributions
inline void DCAFitter::plotRawDataDistribution(const std::string& plotName) {
    gStyle->SetOptStat(0); // Disable statistics box

    if (!ws_) {
        std::cerr << "Error: Workspace is not available." << std::endl;
        return;
    }

    RooDataSet* promptDataSet = dynamic_cast<RooDataSet*>(ws_->data("promptDataSet"));
    RooDataSet* nonPromptDataSet = dynamic_cast<RooDataSet*>(ws_->data("nonPromptDataSet"));

    if (!promptDataSet && !nonPromptDataSet) {
        std::cerr << "Error: Neither prompt nor non-prompt dataset found in workspace." << std::endl;
        return;
    }

    if (!dcaVar_) {
        RooRealVar* dcaFromWs = ws_->var(opt_.dcaVar.c_str()); 
        if (!dcaFromWs && !dcaBranchName_.empty()) dcaFromWs = ws_->var(dcaBranchName_.c_str());
        
        if (dcaFromWs) {
            std::cerr << "Warning: DCA variable found in workspace but dcaVar_ is not set. Using workspace variable." << std::endl;
        }
    }
    if (!dcaVar_) {
        std::cerr << "Error: DCA variable not found." << std::endl;
        return;
    }

    int nCustomBins = dcaBins_.size() - 1;
    if (nCustomBins <= 0) {
        std::cerr << "Error: Invalid binning in dcaBins_." << std::endl;
        return;
    }
    const double* binArray = dcaBins_.data();

    TH1* hPrompt = nullptr;
    TH1* hNonPrompt = nullptr;

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
            std::cout << "Created and normalized prompt histogram (per cm)." << std::endl;
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
            std::cout << "Created and normalized non-prompt histogram (per cm)." << std::endl;
        } else {
            delete hNonPrompt; hNonPrompt = nullptr;
        }
    }

    if (!hPrompt && !hNonPrompt) {
        std::cerr << "Error: Failed to create any histograms for raw data plot." << std::endl;
        return;
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
     if (hAxis) hAxis->Draw("AXIS SAME"); 


    TLegend* legend = new TLegend(0.55, 0.75, 0.93, 0.88); 
    if (hPrompt) legend->AddEntry(hPrompt, "MC Prompt D^{*}", "pe");
    if (hNonPrompt) legend->AddEntry(hNonPrompt, "MC Non-prompt D^{*}", "pe");
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
    latex.DrawLatex(0.30, canvas->GetBottomMargin() + 0.11, opt_.centLegend.empty() ? Form("%0.2f < cos#theta_{HX} < %0.2f", opt_.cosMin, opt_.cosMax) : opt_.centLegend.c_str());



    std::string pngFileName = plotName + ".png";
    canvas->SaveAs(pngFileName.c_str());
    std::cout << "Normalized MC plot saved as " << pngFileName << std::endl;




    delete canvas;
    delete hPrompt;
    delete hNonPrompt;
}

#endif // DCA_FITTER_H
