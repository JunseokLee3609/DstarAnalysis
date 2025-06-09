#ifndef DCA_FITTER_H
#define DCA_FITTER_H

#include <string>
#include <vector>
#include <map>
#include <iostream> // Added for output
#include <memory>   // Added for smart pointers (optional but good practice)

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

using namespace RooFit;

class DCAFitter {
public:
    // Constructor
    DCAFitter(const std::string& name, double dcaMin, double dcaMax, int nBins) :
        name_(name),
        ws_(new RooWorkspace(name.c_str(), (name + " Workspace").c_str())),
        dcaMin_(dcaMin),
        dcaMax_(dcaMax),
        nBins_(nBins),
        dcaVar_(nullptr),
        weights_(nullptr),
        dataSet_(nullptr),
        promptTemplate_(nullptr),
        nonPromptTemplate_(nullptr),
        promptPdf_(nullptr),
        nonPromptPdf_(nullptr),
        fracPrompt_(nullptr),
        model_(nullptr),
        outFile_(nullptr)
    {
        // Define the primary variable
        dcaVar_ = new RooRealVar("dca", "Distance of Closest Approach", dcaMin_, dcaMax_, "cm"); // Add units if known
        ws_->import(*dcaVar_);
        std::cout << "DCAFitter instance created: " << name_ << std::endl;
    }

    // Destructor
    virtual ~DCAFitter() {
        // RooWorkspace owns the objects imported into it, so it handles their deletion
        delete ws_; // Deleting workspace deletes imported objects like dcaVar_, pdfs, datasets if imported

        // Delete objects not owned by the workspace (or manage with smart pointers)
        // Note: RooFitResult is usually owned by the caller or managed separately
        if (outFile_ && outFile_->IsOpen()) {
            outFile_->Close();
            delete outFile_;
        }
        // Pointers like dcaVar_, promptPdf_, etc., are managed by ws_ if imported.
        // If not imported or managed differently, delete them here.
        // Example: delete dcaVar_; (Only if NOT imported or managed by ws_)

        std::cout << "DCAFitter instance destroyed: " << name_ << std::endl;
    }

    // --- Configuration ---
    void setMCFile(const std::string& fileName, const std::string& treeName) {
        mcFileName_ = fileName;
        mcTreeName_ = treeName;
    }
    void setDataFile(const std::string& fileName, const std::string& treeName) {
        dataFileName_ = fileName;
        dataTreeName_ = treeName;
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
    void setWeightBranchName(const std::string& branchName = "") {
        weightBranchName_ = branchName;
        if (!weightBranchName_.empty() && !weights_) {
            weights_ = new RooRealVar(weightBranchName_.c_str(), "Event Weight", 1.0); // Default weight is 1
            // Don't import weights_ into workspace directly unless needed globally
        } else if (weightBranchName_.empty() && weights_) {
            delete weights_;
            weights_ = nullptr;
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
    }

    // --- Workflow ---
    bool createTemplatesFromMC() {
        std::cout << "Creating templates from MC file: " << mcFileName_ << std::endl;
        if (mcFileName_.empty() || mcTreeName_.empty() || dcaBranchName_.empty() || motherPdgIdBranchName_.empty()) {
            std::cerr << "Error: MC file, tree name, DCA branch, or Mother PDG ID branch not set." << std::endl;
            return false;
        }

        TFile* mcFile = TFile::Open(mcFileName_.c_str());
        if (!mcFile || mcFile->IsZombie()) {
            std::cerr << "Error: Could not open MC file: " << mcFileName_ << std::endl;
            return false;
        }

        TTree* mcTree = dynamic_cast<TTree*>(mcFile->Get(mcTreeName_.c_str()));
        if (!mcTree) {
            std::cerr << "Error: Could not find MC tree: " << mcTreeName_ << " in file: " << mcFileName_ << std::endl;
            mcFile->Close();
            delete mcFile;
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
             tempWeights.reset(new RooRealVar(weightBranchName_.c_str(), "Event Weight", 1.0));
             mcVars.add(*tempWeights);
             // If using a global weights_ variable:
             // if (weights_) mcVars.add(*weights_);
        }


        // --- Load full MC dataset ---
        std::unique_ptr<RooDataSet> fullMCDataSet;
        const char* weightVarName = !weightBranchName_.empty() ? weightBranchName_.c_str() : nullptr;

        fullMCDataSet.reset(new RooDataSet("fullMCDataSet", "Full MC DataSet", mcTree, mcVars, mcCuts_.c_str(), weightVarName));


        if (!fullMCDataSet || fullMCDataSet->numEntries() == 0) {
            std::cerr << "Error: Failed to load MC data or no entries passed cuts: " << mcCuts_ << std::endl;
            mcFile->Close();
            delete mcFile;
             // Restore dcaVar_ name before returning
             dcaVar_->SetName("dca3D");
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
             delete mcFile;
             // Restore dcaVar_ name before returning
             dcaVar_->SetName("dca");
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
             delete mcFile;
             // Restore dcaVar_ name before returning
             dcaVar_->SetName("dca3D");
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


        if (!promptDataSet || promptDataSet->numEntries() == 0) {
             std::cerr << "Warning: No prompt MC events found after selection." << std::endl;
        } else {
             std::cout << "Prompt MC entries: " << promptDataSet->sumEntries() << " (weighted)" << std::endl;
             // Create binned template (RooDataHist)
             // Ensure the correct ArgSet (just dcaVar_) is used for binning
             promptTemplate_ = new RooDataHist("promptTemplate", "Prompt MC Template", RooArgSet(*dcaVar_), *promptDataSet);
             ws_->import(*promptTemplate_); // Import into workspace
             std::cout << "Prompt template created." << std::endl;
        }

        if (!nonPromptDataSet || nonPromptDataSet->numEntries() == 0) {
             std::cerr << "Warning: No non-prompt MC events found after selection." << std::endl;
        } else {
             std::cout << "Non-prompt MC entries: " << nonPromptDataSet->sumEntries() << " (weighted)" << std::endl;
             // Create binned template (RooDataHist)
             nonPromptTemplate_ = new RooDataHist("nonPromptTemplate", "Non-prompt MC Template", RooArgSet(*dcaVar_), *nonPromptDataSet);
             ws_->import(*nonPromptTemplate_); // Import into workspace
             std::cout << "Non-prompt template created." << std::endl;
        }

        mcFile->Close();
        delete mcFile;

        // Restore dcaVar_ name if it was temporarily changed
        dcaVar_->SetName("dca3D");

        return (promptTemplate_ != nullptr || nonPromptTemplate_ != nullptr); // Success if at least one template was created
    }


    bool loadData() {
        std::cout << "Loading data from file: " << dataFileName_ << std::endl;
        if (dataFileName_.empty() || dataTreeName_.empty() || dcaBranchName_.empty()) {
            std::cerr << "Error: Data file, tree name, or DCA branch not set." << std::endl;
            return false;
        }

        TFile* dataFile = TFile::Open(dataFileName_.c_str());
        if (!dataFile || dataFile->IsZombie()) {
            std::cerr << "Error: Could not open data file: " << dataFileName_ << std::endl;
            return false;
        }

        TTree* dataTree = dynamic_cast<TTree*>(dataFile->Get(dataTreeName_.c_str()));
        if (!dataTree) {
            std::cerr << "Error: Could not find data tree: " << dataTreeName_ << " in file: " << dataFileName_ << std::endl;
            dataFile->Close();
            delete dataFile;
            return false;
        }

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
        dataSet_ = new RooDataSet("dataSet", "Data Set", dataTree, dataVars, dataCuts_.c_str(), weightVarName);


        if (!dataSet_ || dataSet_->numEntries() == 0) {
            std::cerr << "Error: Failed to load data or no entries passed cuts: " << dataCuts_ << std::endl;
            dataFile->Close();
            delete dataFile;
            // Restore dcaVar_ name
            dcaVar_->SetName("dca3D");
            return false;
        }

        std::cout << "Loaded " << dataSet_->sumEntries() << " data entries (after cuts, weighted)." << std::endl;
        ws_->import(*dataSet_); // Import dataset into workspace

        dataFile->Close();
        delete dataFile;
        // Restore dcaVar_ name
        dcaVar_->SetName("dca3D");
        return true;
    }

    bool buildModel() {
        std::cout << "Building RooFit model..." << std::endl;
        if (!promptTemplate_ && !nonPromptTemplate_) {
            std::cerr << "Error: No MC templates available to build the model." << std::endl;
            return false;
        }

        RooArgList pdfList;
        RooArgList fracList;

        // Create PDFs from templates
        if (promptTemplate_) {
            // Ensure PDF uses the base 'dca' variable name if templates were built correctly
            dcaVar_->SetName("dca3D"); // Ensure consistency
            promptPdf_ = new RooHistPdf("promptPdf", "Prompt PDF", RooArgSet(*dcaVar_), *promptTemplate_);
            ws_->import(*promptPdf_);
            pdfList.add(*promptPdf_);
            std::cout << "Prompt PDF created." << std::endl;
        }

        if (nonPromptTemplate_) {
            dcaVar_->SetName("dca3D"); // Ensure consistency
            nonPromptPdf_ = new RooHistPdf("nonPromptPdf", "Non-prompt PDF", RooArgSet(*dcaVar_), *nonPromptTemplate_);
            ws_->import(*nonPromptPdf_);
            pdfList.add(*nonPromptPdf_);
            std::cout << "Non-prompt PDF created." << std::endl;
        }

        // Build combined model
        if (pdfList.getSize() == 2) {
            // Both prompt and non-prompt exist
            fracPrompt_ = new RooRealVar("fracPrompt", "Fraction of prompt", 0.8, 0.0, 1.0); // Initial guess 0.8
            ws_->import(*fracPrompt_);
            fracList.add(*fracPrompt_);
            model_ = new RooAddPdf("model", "Prompt + Non-prompt Model", pdfList, fracList);
            std::cout << "Created combined model (Prompt + Non-prompt)." << std::endl;
        } else if (pdfList.getSize() == 1) {
            // Only one component exists (treat as the full model)
            // Clone the single PDF as the model. Ensure the name is "model".
            RooAbsPdf* singlePdf = dynamic_cast<RooAbsPdf*>(pdfList.at(0));
            if (singlePdf) {
                 model_ = dynamic_cast<RooAddPdf*>(singlePdf->Clone("model")); // Clone might not yield RooAddPdf, adjust if needed
                 if (!model_) { // If clone doesn't return RooAddPdf, handle appropriately
                     // Maybe just assign the single PDF directly if model_ is RooAbsPdf*?
                     // For now, assuming model_ should be RooAddPdf or compatible
                     std::cerr << "Warning: Cloning single PDF did not result in expected type. Model might be incomplete." << std::endl;
                     // Fallback or error handling needed here.
                     // Let's assume model_ is RooAbsPdf* for flexibility:
                     // model_ = singlePdf; // If model_ is RooAbsPdf*
                     // Or create a RooAddPdf with one component and fraction 1.0?
                     RooRealVar* fracOne = new RooRealVar("fracOne","Fraction=1", 1.0);
                     fracOne->setConstant(true);
                     ws_->import(*fracOne); // Import if needed elsewhere
                     model_ = new RooAddPdf("model", "Single Component Model", RooArgList(*singlePdf), RooArgList(*fracOne));

                 }
                 std::cout << "Created model with single component: " << singlePdf->GetName() << std::endl;
            } else {
                 std::cerr << "Error: Could not cast single component to RooAbsPdf." << std::endl;
                 return false;
            }
        } else {
            std::cerr << "Error: Could not create any PDFs from templates." << std::endl;
            return false;
        }

        ws_->import(*model_);
        std::cout << "Model built successfully." << std::endl;
        return true;
    }


    RooFitResult* performFit(bool useMinos = false) {
        std::cout << "Performing fit..." << std::endl;
        if (!model_ || !dataSet_) {
            std::cerr << "Error: Model or dataset not available for fitting." << std::endl;
            return nullptr;
        }
        // Ensure dcaVar_ has the standard name 'dca' before fitting
        dcaVar_->SetName("dca3D");


        // Fit options using RooLinkedList
        RooLinkedList fitOptionsList;
        fitOptionsList.Add(RooFit::Save(true).Clone()); // Save detailed fit result
        fitOptionsList.Add(RooFit::PrintLevel(-1).Clone()); // Reduce verbosity (-1), default is 1

        if (useMinos) {
            fitOptionsList.Add(RooFit::Minos(true).Clone());
            std::cout << "Using MINOS for error estimation." << std::endl;
        }
        if (dataSet_->isWeighted()) {
             fitOptionsList.Add(RooFit::SumW2Error(true).Clone()); // Use weighted likelihood / correct errors
             std::cout << "Using SumW2Error(true) for weighted dataset." << std::endl;
        }

        // Perform the fit using the RooLinkedList
        // Ensure the model is fitted to the dataset using the correct variable ('dca')
        RooFitResult* fitResult = model_->fitTo(*dataSet_, fitOptionsList);


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

        return fitResult; // Caller is responsible for deleting this object
    }

    void plotResults(RooFitResult* fitResult = nullptr, const std::string& plotName = "dca_fit_plot") {
        std::cout << "Plotting results..." << std::endl;
        if (!dcaVar_ || !dataSet_ || !model_) {
            std::cerr << "Error: Cannot plot results. Variables, data, or model missing." << std::endl;
            return;
        }
        // Ensure dcaVar_ has the standard name 'dca' for plotting
        dcaVar_->SetName("dca3D");


        TCanvas* canvas = new TCanvas(plotName.c_str(), "DCA Fit Results", 800, 600);
        RooPlot* frame = dcaVar_->frame(Title("DCA Distribution Fit"));
        frame->GetYaxis()->SetTitle(TString::Format("Events / (%.3f units)", (dcaMax_ - dcaMin_) / nBins_));
        frame->GetXaxis()->SetTitle(dcaVar_->GetTitle());


        // Plot data with appropriate error bars for weighted data
        RooCmdArg dataPlotArgs = Binning(nBins_);
        if (dataSet_->isWeighted()) {
            dataPlotArgs = RooFit::DataError(RooAbsData::SumW2);
        }
        dataSet_->plotOn(frame, dataPlotArgs, Name("data"));


        // Plot full model
        model_->plotOn(frame, Name("full_model"), LineColor(kRed));

        // Plot components if they exist
        if (promptPdf_ && nonPromptPdf_ && fracPrompt_) {
            model_->plotOn(frame, Components(*promptPdf_), LineStyle(kDashed), LineColor(kBlue), Name("prompt_comp"));
            model_->plotOn(frame, Components(*nonPromptPdf_), LineStyle(kDashed), LineColor(kGreen + 2), Name("nonprompt_comp"));
        } else if (promptPdf_) {
             // Check if model is composite or single PDF
             if (model_->dependsOn(*promptPdf_)) { // Check dependency
                 model_->plotOn(frame, Components(*promptPdf_), LineStyle(kDashed), LineColor(kBlue), Name("prompt_comp"));
             }
        } else if (nonPromptPdf_) {
             if (model_->dependsOn(*nonPromptPdf_)) { // Check dependency
                 model_->plotOn(frame, Components(*nonPromptPdf_), LineStyle(kDashed), LineColor(kGreen + 2), Name("nonprompt_comp"));
             }
        }


        frame->Draw();

        // Add Legend
        TLegend* legend = new TLegend(0.65, 0.65, 0.88, 0.88);
        legend->AddEntry(frame->findObject("data"), "Data", "LEP");
        legend->AddEntry(frame->findObject("full_model"), "Full Fit", "L");
        if (frame->findObject("prompt_comp")) legend->AddEntry(frame->findObject("prompt_comp"), "Prompt Component", "L");
        if (frame->findObject("nonprompt_comp")) legend->AddEntry(frame->findObject("nonprompt_comp"), "Non-prompt Component", "L");
        legend->SetBorderSize(0);
        legend->SetFillStyle(0);
        legend->Draw();

        // Optionally display fit parameters on plot (requires fitResult)
        if (fitResult) {
             // Display parameters associated with the model
             // Use RooArgSet containing parameters you want to show, e.g., fracPrompt_
             RooArgSet paramsToShow;
             if (fracPrompt_) paramsToShow.add(*fracPrompt_);
             // Add other parameters if needed, e.g., shape parameters if they were floated

             if (paramsToShow.getSize() > 0) {
                 model_->paramOn(frame, Layout(0.6, 0.9, 0.6), Parameters(paramsToShow), Format("NEU", AutoPrecision(2)));
                 frame->getAttText()->SetTextSize(0.03); // Adjust text size if needed
                 frame->Draw(); // Redraw frame to show parameters
             }
        }


        // Save plot if output file is set
        if (!outputFileName_.empty()) {
            if (!outFile_ || !outFile_->IsOpen()) {
                 outFile_ = TFile::Open(outputFileName_.c_str(), "UPDATE"); // Try UPDATE first
                 if (!outFile_ || outFile_->IsZombie()) {
                     delete outFile_;
                     outFile_ = TFile::Open(outputFileName_.c_str(), "RECREATE");
                 }
            } else {
                 if (!outFile_->IsWritable()) { // Check if writable
                     std::string currentFileName = outFile_->GetName();
                     delete outFile_;
                     outFile_ = TFile::Open(currentFileName.c_str(), "UPDATE");
                     if (!outFile_ || outFile_->IsZombie()) {
                         delete outFile_;
                         outFile_ = TFile::Open(currentFileName.c_str(), "RECREATE");
                     }
                 } else {
                    outFile_->cd(); // Ensure correct context
                 }
            }

            if(outFile_ && outFile_->IsOpen() && outFile_->IsWritable()){
                 canvas->Write(plotName.c_str());
                 std::cout << "Fit plot saved to " << outputFileName_ << " as " << plotName << std::endl;
                 // Don't close file here; let saveResults or destructor handle it.
            } else {
                 std::cerr << "Error: Could not open/write to output file " << outputFileName_ << " to save fit plot." << std::endl;
                 delete canvas; // Clean up canvas if saving failed
                 return;
            }
        }

        // Manage canvas lifetime appropriately (e.g., delete if in batch mode and saved)
        // delete canvas;
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
                 delete outFile_; // Close existing handle
                 outFile_ = TFile::Open(currentFileName.c_str(), "RECREATE"); // Reopen fresh
             } else {
                 // If already open and writable (e.g., from plotting), ensure we are in the root directory
                 outFile_->cd();
             }
         } else {
             // If not open, open in RECREATE mode
             delete outFile_; // Delete potential null or closed pointer
             outFile_ = TFile::Open(outputFileName_.c_str(), "RECREATE");
         }


         if (!outFile_ || !outFile_->IsOpen() || !outFile_->IsWritable()) {
             std::cerr << "Error: Could not open or write to output file: " << outputFileName_ << std::endl;
             // Clean up potential zombie file object
             delete outFile_;
             outFile_ = nullptr;
             return;
         }

         // Write workspace
         if (ws_) {
             ws_->Write(("ws_" + name_).c_str());
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
         delete outFile_;
         outFile_ = nullptr; // Reset pointer after closing and deleting
         std::cout << "Results saved and file closed." << std::endl;
    }

    // --- New method to plot raw data distribution ---
    void plotRawDataDistribution(const std::string& plotName = "dca_raw_data_plot");


private:
    // --- Internal Helper Methods ---
    // loadDataSet helper is integrated into createTemplatesFromMC and loadData for clarity

    // --- Member Variables ---
    std::string name_;
    RooWorkspace* ws_; // RooFit workspace

    // Variables
    RooRealVar* dcaVar_;    // The DCA variable
    RooRealVar* weights_;   // Optional global weights variable (use with care)

    // Configuration
    std::string mcFileName_;
    std::string mcTreeName_;
    std::string dataFileName_;
    std::string dataTreeName_;
    std::string dcaBranchName_;
    std::string motherPdgIdBranchName_;
    std::string weightBranchName_; // Name of the weight branch in Trees
    std::vector<int> promptPdgIds_;
    std::vector<int> nonpromptPdgIds_;
    std::string mcCuts_;
    std::string dataCuts_;
    std::string outputFileName_;

    // Datasets and Templates (Owned by Workspace after import)
    RooDataSet* dataSet_;           // Data to be fitted (pointer to object in workspace)
    RooDataHist* promptTemplate_;   // Prompt MC template (pointer to object in workspace)
    RooDataHist* nonPromptTemplate_; // Non-prompt MC template (pointer to object in workspace)

    // PDFs and Model (Owned by Workspace after import)
    RooHistPdf* promptPdf_;         // PDF from prompt template (pointer to object in workspace)
    RooHistPdf* nonPromptPdf_;      // PDF from non-prompt template (pointer to object in workspace)
    RooRealVar* fracPrompt_;        // Fraction of prompt component (pointer to object in workspace)
    RooAddPdf* model_;              // Combined model (pointer to object in workspace)

    // Parameters
    double dcaMin_;
    double dcaMax_;
    int nBins_;

    // ROOT Objects
    TFile* outFile_; // File handle for output
};


// --- Implementation of plotRawDataDistribution ---
// This function now plots normalized prompt and non-prompt MC distributions
inline void DCAFitter::plotRawDataDistribution(const std::string& plotName) {
    std::cout << "Plotting normalized MC template distributions..." << std::endl;

    if (!ws_) {
        std::cerr << "Error: Workspace is not available." << std::endl;
        return;
    }

    // Fetch datasets from workspace
    RooDataSet* promptDataSet = dynamic_cast<RooDataSet*>(ws_->data("promptDataSet"));
    RooDataSet* nonPromptDataSet = dynamic_cast<RooDataSet*>(ws_->data("nonPromptDataSet"));

    if (!promptDataSet && !nonPromptDataSet) {
        std::cerr << "Error: Neither prompt nor non-prompt dataset found in workspace." << std::endl;
        return;
    }

    // Fetch the variable from the workspace if necessary
    if (!dcaVar_) {
        dcaVar_ = ws_->var("dca3D"); // Assuming the standard name is 'dca' in the workspace
        if (!dcaVar_) {
             // Try the branch name if 'dca' isn't found
             if (!dcaBranchName_.empty()) dcaVar_ = ws_->var(dcaBranchName_.c_str());
        }
    }
     if (!dcaVar_) {
        std::cerr << "Error: DCA variable not found in workspace or class." << std::endl;
        return;
     }
     // Ensure variable has standard name for consistency if needed, but use the fetched one
     // dcaVar_->SetName("dca"); // Optional: Reset name if subsequent operations expect it


    // Create histograms
    TH1* hPrompt = nullptr;
    TH1* hNonPrompt = nullptr;

    // Define binning based on the RooRealVar
    // Note: createHistogram uses the variable's current binning if set, or defaults.
    // Ensure dcaVar_ has the desired binning for the plot.
    dcaVar_->setBins(nBins_); // Explicitly set bins for histogram creation

    if (promptDataSet) {
        hPrompt = promptDataSet->createHistogram("hPrompt", *dcaVar_);
        if (hPrompt && hPrompt->Integral() > 0) {
            hPrompt->Scale(1.0 / hPrompt->Integral());
            hPrompt->SetLineColor(kBlue);
            hPrompt->SetLineWidth(2);
            std::cout << "Created and normalized prompt histogram." << std::endl;
        } else {
            std::cerr << "Warning: Could not create or normalize prompt histogram." << std::endl;
            delete hPrompt;
            hPrompt = nullptr;
        }
    }

    if (nonPromptDataSet) {
        hNonPrompt = nonPromptDataSet->createHistogram("hNonPrompt", *dcaVar_);
        if (hNonPrompt && hNonPrompt->Integral() > 0) {
            hNonPrompt->Scale(1.0 / hNonPrompt->Integral());
            hNonPrompt->SetLineColor(kGreen + 2);
            hNonPrompt->SetLineWidth(2);
            hNonPrompt->SetLineStyle(kDashed); // Make it visually distinct
            std::cout << "Created and normalized non-prompt histogram." << std::endl;
        } else {
            std::cerr << "Warning: Could not create or normalize non-prompt histogram." << std::endl;
            delete hNonPrompt;
            hNonPrompt = nullptr;
        }
    }

    if (!hPrompt && !hNonPrompt) {
        std::cerr << "Error: Failed to create any histograms." << std::endl;
        return;
    }

    // Create canvas and draw
    TCanvas* canvas = new TCanvas(plotName.c_str(), "Normalized MC Templates", 800, 600);
    canvas->SetLogy(); // Set log scale for Y-axis

    // Determine plot range (find max Y value for setting range)
    double maxY = 0;
    if (hPrompt) maxY = std::max(maxY, hPrompt->GetMaximum());
    if (hNonPrompt) maxY = std::max(maxY, hNonPrompt->GetMaximum());

    bool firstDrawn = false;
    if (hPrompt) {
        hPrompt->SetTitle("Normalized MC Templates");
        hPrompt->GetXaxis()->SetTitle(dcaVar_->GetTitle()); // Use title from RooRealVar
        hPrompt->GetYaxis()->SetTitle("Normalized Events / Bin");
        hPrompt->SetMaximum(maxY * 1.5); // Add some headroom
        hPrompt->Draw("HIST");
        firstDrawn = true;
    }
    if (hNonPrompt) {
        if (!firstDrawn) { // If prompt wasn't drawn, draw non-prompt first
             hNonPrompt->SetTitle("Normalized MC Templates");
             hNonPrompt->GetXaxis()->SetTitle(dcaVar_->GetTitle());
             hNonPrompt->GetYaxis()->SetTitle("Normalized Events / Bin");
             hNonPrompt->SetMaximum(maxY * 1.5);
             hNonPrompt->Draw("HIST");
        } else {
             hNonPrompt->Draw("HIST SAME"); // Draw on top
        }
    }

    // Add Legend
    TLegend* legend = new TLegend(0.60, 0.75, 0.88, 0.88);
    if (hPrompt) legend->AddEntry(hPrompt, "Prompt MC", "L");
    if (hNonPrompt) legend->AddEntry(hNonPrompt, "Non-prompt MC", "L");
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->Draw();

    // Save plot as PNG
    std::string pngFileName = plotName + ".png";
    canvas->SaveAs(pngFileName.c_str());
    std::cout << "Normalized MC plot saved as " << pngFileName << std::endl;

    // Also save to ROOT file if open
    if (!outputFileName_.empty()) {
        // Re-use or open the output file (similar logic as before)
        TFile* tempOutFile = nullptr; // Use a temporary pointer for file handling here
         if (!outFile_ || !outFile_->IsOpen()) {
             tempOutFile = TFile::Open(outputFileName_.c_str(), "UPDATE");
              if (!tempOutFile || tempOutFile->IsZombie()) {
                  delete tempOutFile;
                  tempOutFile = TFile::Open(outputFileName_.c_str(), "RECREATE");
              }
              // If we opened the file here, assign it to outFile_ if it was null
              if (!outFile_) outFile_ = tempOutFile;
              else delete tempOutFile; // Otherwise, delete the temporary handle
         } else {
              // Ensure file is writable if already open
              if (!outFile_->IsWritable()) {
                  std::string currentFileName = outFile_->GetName();
                  delete outFile_; // Close and delete
                  outFile_ = TFile::Open(currentFileName.c_str(), "UPDATE"); // Try opening in UPDATE mode
                  if (!outFile_ || outFile_->IsZombie()) { // If UPDATE failed, try RECREATE
                     delete outFile_;
                     outFile_ = TFile::Open(currentFileName.c_str(), "RECREATE");
                  }
              } else {
                 outFile_->cd(); // Switch context if already open and writable
              }
         }


        if(outFile_ && outFile_->IsOpen() && outFile_->IsWritable()){
             canvas->Write(plotName.c_str()); // Save canvas to ROOT file as well
             std::cout << "Normalized MC plot also saved to " << outputFileName_ << " as " << plotName << std::endl;
             // Do not close the file here
        } else {
             std::cerr << "Warning: Could not open/write to output ROOT file " << outputFileName_ << " to save plot." << std::endl;
        }
    }


    // Clean up
    delete canvas;
    delete hPrompt; // Safe even if null
    delete hNonPrompt; // Safe even if null
    // Legend is owned by canvas, deleted with it
}

#endif // DCA_FITTER_H