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
#include "THStack.h" // For THStack
#include "TLatex.h"  // For CMS label
#include "Opt.h"

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
    DCAFitter(FitOpt &opt, const std::string& name, double dcaMin, double dcaMax, int nBins) :
        name_(name),
        opt_(opt),
        ws_(new RooWorkspace(name.c_str(), (name + " Workspace").c_str())),
        dcaMin_(dcaMin),
        dcaMax_(dcaMax),
        nBins_(nBins),
        dcaVar_(nullptr),
        weights_(nullptr),
        dataSet_(nullptr),
        promptTemplate_(nullptr),
        nonPromptTemplate_(nullptr),
        promptHist_(nullptr),
        nonPromptHist_(nullptr),
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
        if (mcFileName_.empty() || mcRooDatsetName_.empty() || dcaBranchName_.empty() || motherPdgIdBranchName_.empty()) {
            std::cerr << "Error: MC file, tree name, DCA branch, or Mother PDG ID branch not set." << std::endl;
            return false;
        }

        TFile* mcFile = TFile::Open(mcFileName_.c_str());
        if (!mcFile || mcFile->IsZombie()) {
            std::cerr << "Error: Could not open MC file: " << mcFileName_ << std::endl;
            return false;
        }
        RooDataSet* mcDataset = dynamic_cast<RooDataSet*>(mcFile->Get(mcRooDatsetName_.c_str()));

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

        if(mcDataset) fullMCDataSet.reset(mcDataset);
        else return cout << "Error: Could not find MC dataset: " << mcRooDatsetName_ << " in file: " << mcFileName_ << std::endl, false;


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

        promptHist_ = new TH1D("promptHist", "Prompt Histogram", dcaBins_.size() - 1, dcaBins_.data());
        promptHist_->Sumw2();
        for (int i = 0; i < promptDataSet->numEntries(); ++i) {
            const RooArgSet* row = promptDataSet->get(i);
            promptHist_->Fill(row->getRealValue(dcaVar_->GetName()));
        }
        nonPromptHist_ = new TH1D("nonPromptHist", "Non-prompt Histogram", dcaBins_.size() - 1,dcaBins_.data());
        nonPromptHist_->Sumw2();
        for (int i = 0; i < nonPromptDataSet->numEntries(); ++i) {
            const RooArgSet* row = nonPromptDataSet->get(i);
            nonPromptHist_->Fill(row->getRealValue(dcaVar_->GetName()));
        }


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
        } else {
             std::cout << "Non-prompt MC entries: " << nonPromptDataSet->sumEntries() << " (weighted)" << std::endl;
             // Create binned template (RooDataHist)
             nonPromptTemplate_ = new RooDataHist("nonPromptTemplate", "Non-prompt MC Template", RooArgSet(*dcaVar_), nonPromptHist_);
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
            dcaVar_->SetName("dca3D");
            return false;
        }

        std::cout << "Loaded " << dataSet_->sumEntries() << " data entries (after cuts, weighted)." << std::endl;
        ws_->import(*dataSet_); // Import dataset into workspace

        dataFile->Close();
        // delete dataFile;
        // Restore dcaVar_ name
        dcaVar_->SetName("dca3D");
        return true;
    }

    bool buildModel() {
        std::cout << "Building RooFit model..." << std::endl;
        
        // 1. createTemplatesFromMC()에서 생성된 템플릿이 있는지 먼저 확인합니다.
        if (!promptTemplate_ && !nonPromptTemplate_) {
            std::cerr << "Error: No MC templates (RooDataHist) are available to build the model." << std::endl;
            std::cerr << "       Please run createTemplatesFromMC() first." << std::endl;
            return false;
        }
        if (!dataSet_) {
            std::cerr << "Error: Data set is not loaded. Needed for initial yield guess." << std::endl;
            return false;
        }
    
        // *** 기존의 비어있는 히스토그램과 템플릿을 생성하던 코드를 모두 삭제했습니다. ***
        // *** 이것이 가장 중요한 수정 사항입니다. ***
    
        // --- 2. 확장된(Extended) 모델을 구성합니다 ---
        // 각 컴포넌트의 수율(yield)을 나타내는 변수들을 만듭니다. 이 값들이 피팅의 대상이 됩니다.
        double nevt = dataSet_->sumEntries(); // 데이터의 총 이벤트 수를 초기값 추정에 사용
        
        // 멤버 변수로 선언된 n_prompt_, n_nonprompt_를 사용합니다. (헤더 파일에 추가 필요)
        
        n_prompt_ = new RooRealVar("n_prompt", "Number of prompt events", nevt * 0.8, 0, nevt*0.9);
        n_nonprompt_ = new RooRealVar("n_nonprompt", "Number of non-prompt events", nevt * 0.01, 0, nevt*0.8);
    
        RooArgList pdfList;
        RooArgList yieldList;
    
        dcaVar_->SetName("dca3D"); // 일관된 변수 이름 사용
    
        // --- 3. 유효한 템플릿으로부터 PDF를 생성하고 리스트에 추가합니다 ---
        if (promptTemplate_) {
            // 보간(interpolation) 옵션(e.g., 2)을 추가하여 더 부드러운 PDF를 만듭니다.
            promptPdf_ = new RooHistPdf("promptPdf", "Prompt PDF", RooArgSet(*dcaVar_), *promptTemplate_);
            pdfList.add(*promptPdf_);
            yieldList.add(*n_prompt_);
            ws_->import(*promptPdf_);
            std::cout << "Prompt PDF created from existing template." << std::endl;
        }
    
        if (nonPromptTemplate_) {
            nonPromptPdf_ = new RooHistPdf("nonPromptPdf", "Non-prompt PDF", RooArgSet(*dcaVar_), *nonPromptTemplate_);
            pdfList.add(*nonPromptPdf_);
            yieldList.add(*n_nonprompt_);
            ws_->import(*nonPromptPdf_);
            std::cout << "Non-prompt PDF created from existing template." << std::endl;
        }
    
        // --- 4. 최종 모델을 생성합니다 (한 컴포넌트만 있어도 작동) ---
        if (pdfList.getSize() > 0) {
            model_ = new RooAddPdf("model", "Extended P+NP Model", pdfList, yieldList);
            ws_->import(*model_);
            std::cout << "Extended model built successfully with " << pdfList.getSize() << " component(s)." << std::endl;
        } else {
            std::cerr << "Error: No PDFs were created to build the model." << std::endl;
            return false;
        }
    
        // --- 5. (결과 분석용) 분율을 계산하는 RooFormulaVar를 만듭니다 ---
        // 이 변수는 피팅 파라미터가 아니라, 피팅된 수율로부터 계산되는 값입니다.
        if (pdfList.getSize() == 2) {
            // fracPrompt_ 멤버 변수 선언 필요 (RooFormulaVar*)
            fracPrompt_ = new RooFormulaVar("fracPrompt", "Prompt Fraction", "@0 / (@0 + @1)", RooArgList(*n_prompt_, *n_nonprompt_));
            ws_->import(*fracPrompt_);
            std::cout << "Prompt fraction variable (RooFormulaVar) created for post-fit analysis." << std::endl;
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
        dcaVar_->SetName("dca3D");
        TH1D* tempDataHist = new TH1D("tempDataHistForBinning", "Temporary Data Hist for Binning",dcaBins_.size() - 1, dcaBins_.data());
        // Fill the temporary TH1 from the RooDataSet
        // Make sure dcaVar_ in dataSet_ has the correct name (dcaBranchName_ or "dca3D")
        // If dataSet_ was created with dcaVar_ having dcaBranchName_, it should be fine.
        // If dcaVar_ name was changed after dataSet_ creation, ensure consistency.
        dataSet_->fillHistogram(tempDataHist, RooArgList(*dcaVar_));

        RooDataHist dataHist("dataHist", "Binned Data", RooArgList(*dcaVar_), tempDataHist);
        delete tempDataHist; // Clean up temporary TH1


        // Fit options using RooLinkedList
        RooLinkedList fitOptionsList;
        fitOptionsList.Add(RooFit::Save(true).Clone()); // Save detailed fit result
        fitOptionsList.Add(RooFit::PrintLevel(-1).Clone()); // Reduce verbosity (-1), default is 1
        fitOptionsList.Add(RooFit::Extended(true).Clone());

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
        RooFitResult* fitResult = model_->fitTo(dataHist, fitOptionsList);


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
        if (promptHist_) { // Check if it was created
            // delete promptHist_;
            // promptHist_ = nullptr;
       }
       if (nonPromptHist_) { // Check if it was created
            // delete nonPromptHist_;
            // nonPromptHist_ = nullptr;
       }


        return fitResult; // Caller is responsible for deleting this object
    }
    void plotResults(RooFitResult* fitResult = nullptr, const std::string& plotName = "dca_fit_plot") {
        std::cout << "Plotting results (CMS style)..." << std::endl;
        if (!dcaVar_ || !dataSet_ || !model_) {
            std::cerr << "Error: Cannot plot results. Variables, data, or model missing." << std::endl;
            return;
        }
    
        // --- 새로운 부분: RooPlot을 사용하여 모델 곡선을 가져옵니다 ---
        RooPlot* frame = dcaVar_->frame(RooFit::Title(" "));
        
        // 데이터 플로팅 (bin 너비 보정됨)
        // *** MODIFIED ***: dataSet_에 RooFit::Binning(nBins_)를 적용하여 binning을 명시적으로 설정할 수 있습니다.
        // 만약 dcaBins_를 사용하고 싶다면, RooBinning 객체를 만들어 전달합니다.
        RooBinning binning(dcaBins_.size() - 1, &dcaBins_[0]);
        dataSet_->plotOn(frame, RooFit::Binning(binning), RooFit::Name("data"));
    
        // 모델과 컴포넌트 플로팅
        model_->plotOn(frame, RooFit::Name("model"), RooFit::LineColor(kRed+1));
        model_->plotOn(frame, RooFit::Components(*nonPromptPdf_), RooFit::Name("nonprompt"), RooFit::FillStyle(3354), RooFit::FillColor(kBlue-9), RooFit::LineColor(kBlue+1), RooFit::DrawOption("F"));
        model_->plotOn(frame, RooFit::Components(*promptPdf_),RooFit::AddTo("nonprompt"), RooFit::Name("prompt"), RooFit::FillStyle(3354), RooFit::FillColor(kRed-9), RooFit::LineColor(kRed+1), RooFit::DrawOption("F"));
        model_->plotOn(frame, RooFit::Components(*nonPromptPdf_), RooFit::Name("nonprompt"), RooFit::FillStyle(3354), RooFit::FillColor(kBlue-9), RooFit::LineColor(kBlue+1), RooFit::DrawOption("F"));
        
        // 데이터를 한 번 더 그려서 스택 위에 오도록 합니다.
        // dataSet_->plotOn(frame, RooFit::Binning(binning), RooFit::Name("data")); 
    
        // --- 캔버스 및 스타일링 ---
        TCanvas* c = new TCanvas(plotName.c_str(), "CMS DCA Fit", 800, 700); // 캔버스 크기 조정
        c->SetLogy();
        c->SetLeftMargin(0.15); // Y축 제목 공간 확보
        
        frame->Draw();
        
        // *** MODIFIED ***: Y축 제목을 "Yield per cm"으로 변경
        frame->GetYaxis()->SetTitle("Yield per cm");
        frame->GetXaxis()->SetTitle("D^{0} DCA (cm)");
        frame->GetXaxis()->SetTitleOffset(1.1);
        frame->GetYaxis()->SetTitleOffset(1.4);
        frame->SetMinimum(100); // 최소값은 데이터에 맞게 조정
        frame->SetMaximum(frame->GetMaximum() * 1.5);
    
        // --- 범례 (Legend) 생성 ---
        TLegend* leg = new TLegend(0.55, 0.65, 0.88, 0.88);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->SetTextSize(0.04);
        
        // RooPlot에서 객체를 이름으로 찾아와서 범례에 추가합니다.
        TObject* dataObj = frame->findObject("data");
        TObject* promptObj = frame->findObject("prompt");
        TObject* nonPromptObj = frame->findObject("nonprompt");
        
        if (dataObj) leg->AddEntry(dataObj, "Data", "pe");
        if (promptObj) leg->AddEntry(promptObj, "Prompt D^{0}", "f");
        if (nonPromptObj) leg->AddEntry(nonPromptObj, "Non-Prompt D^{0}", "f");
        leg->Draw();
    
        // --- 텍스트 라벨 (CMS, Kinematics, Fit Result) ---
        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.18, 0.93, "#bf{CMS} #it{Preliminary}");
        latex.SetTextSize(0.04);
        latex.DrawLatex(0.60, 0.93, "pp #sqrt{s_{NN}} = 5.36 TeV"); // 예시 텍스트
        
        latex.SetTextSize(0.035);
        // latex.DrawLatex(0.55, 0.60, "10.0 < p_{T} < 12.5 GeV/c");
        // latex.DrawLatex(0.55, 0.60, "|y| < 1.0");
    
        // Fit fraction
        if (fracPrompt_ && fitResult) {
            double frac_val = fracPrompt_->getVal();
            double frac_err = fracPrompt_->getPropagatedError(*fitResult);
            std::cout << "Prompt Fraction = " << frac_val << " +/- " << frac_err << std::endl;
            
            latex.SetTextSize(0.038);
            latex.DrawLatex(0.55, 0.60, Form("Prompt frac. = %.1f #pm %.1f %%", frac_val*100, frac_err*100));

            latex.DrawLatex(0.55, 0.55, Form("%0.2f < p_{T} < %0.2f GeV/c",opt_.pTMin, opt_.pTMax));
            latex.DrawLatex(0.55, 0.50, "|y| < 1");
            latex.DrawLatex(0.55, 0.45, Form("%0.2f < cos#theta_{HX} < %0.2f", opt_.cosMin, opt_.cosMax));
        }
    
        // --- 파일 저장 ---
        c->SaveAs((plotName + ".png").c_str());
        c->SaveAs((plotName + ".pdf").c_str());
        if (!outputFileName_.empty()) {
            TFile* fout = outFile_;
            if (!fout || !fout->IsOpen()) fout = TFile::Open(outputFileName_.c_str(), "UPDATE");
            if (fout && fout->IsOpen()) {
                c->Write(plotName.c_str());
                if (!outFile_) fout->Close();
            }
        }
        
        delete c;
        delete frame;
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
        //  delete outFile_;
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
    vector<double> dcaBins_ = {0,0.002,0.004,0.006,0.008,0.01,0.012,0.014,0.016,0.022,0.03,0.038,0.05,0.065,0.08,0.1};
    FitOpt opt_;

    // Variables
    RooRealVar* dcaVar_;    // The DCA variable
    RooRealVar* weights_;   // Optional global weights variable (use with care)
    RooRealVar* n_prompt_; // Number of prompt events (pointer to object in workspace)
    RooRealVar* n_nonprompt_; // Number of non-prompt events (pointer to object in workspace)

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

    // Datasets and Templates (Owned by Workspace after import)
    RooDataSet* dataSet_;           // Data to be fitted (pointer to object in workspace)
    RooDataHist* promptTemplate_;   // Prompt MC template (pointer to object in workspace)
    TH1* promptHist_; // Histogram for prompt template (not owned by workspace)
    TH1* nonPromptHist_; // Histogram for non-prompt template (not owned by workspace)
    RooDataHist* nonPromptTemplate_; // Non-prompt MC template (pointer to object in workspace)

    // PDFs and Model (Owned by Workspace after import)
    RooHistPdf* promptPdf_;         // PDF from prompt template (pointer to object in workspace)
    RooHistPdf* nonPromptPdf_;      // PDF from non-prompt template (pointer to object in workspace)
    RooFormulaVar* fracPrompt_;        // Fraction of prompt component (pointer to object in workspace)
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
        dcaVar_ = ws_->var("dca3D"); // Workspace에서 변수 가져오기
        if (!dcaVar_ && !dcaBranchName_.empty()) dcaVar_ = ws_->var(dcaBranchName_.c_str());
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
        if (hPrompt->Integral() > 0) {
            hPrompt->Scale(1.0/hPrompt->Integral());
            for (int i = 1; i <= hPrompt->GetNbinsX(); ++i) {
                double content = hPrompt->GetBinContent(i);
                double error = hPrompt->GetBinError(i);
                double width = hPrompt->GetBinWidth(i);
                if (width > 0) {
                    hPrompt->SetBinContent(i, content / width);
                    hPrompt->SetBinError(i, error / width);
                } else {
                    hPrompt->SetBinContent(i, 0);
                    hPrompt->SetBinError(i, 0);
                }
            }
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
            for (int i = 1; i <= hNonPrompt->GetNbinsX(); ++i) {
                double content = hNonPrompt->GetBinContent(i);
                double error = hNonPrompt->GetBinError(i);
                double width = hNonPrompt->GetBinWidth(i);
                if (width > 0) {
                    hNonPrompt->SetBinContent(i, content / width);
                    hNonPrompt->SetBinError(i, error / width);
                } else {
                    hNonPrompt->SetBinContent(i, 0);
                    hNonPrompt->SetBinError(i, 0);
                }
            }
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

    // Y축 범위 결정
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
        hAxis->SetTitle(""); // 그림 제목 제거
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
    latex.DrawLatex(0.30, canvas->GetBottomMargin() + 0.03, Form("%0.2f < p_{T} < %0.2f GeV/c",opt_.pTMin, opt_.pTMax));
    latex.DrawLatex(0.30, canvas->GetBottomMargin() + 0.07, "|y| < 1");
    latex.DrawLatex(0.30, canvas->GetBottomMargin() + 0.11, Form("%0.2f < cos#theta_{HX} < %0.2f", opt_.cosMin, opt_.cosMax));



    std::string pngFileName = plotName + ".png";
    canvas->SaveAs(pngFileName.c_str());
    std::cout << "Normalized MC plot saved as " << pngFileName << std::endl;




    delete canvas;
    delete hPrompt;
    delete hNonPrompt;
}
void Clear(){
    // Clear the static variables or reset the state if needed
    // This is a placeholder for any cleanup logic you might need
    std::cout << "Clearing DCAFitter state..." << std::endl;
    // Reset member variables, close files, etc.
    // For example, if you have static members, reset them here
    // delete dataSet_
}
#endif // DCA_FITTER_H