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
#include "PlotManager.h"

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

#include "MassFitter.h"
#include "Params.h"

using namespace RooFit;

class DCAFitter {
public:
    // Constructor
    DCAFitter(FitOpt &opt, const std::string& name, const std::string& massVarName, double dcaMin, double dcaMax, int nBins) :
        name_(name),
        opt_(opt),
        ws_(new RooWorkspace(Form("ws_%s",opt.name.c_str()), (name + " Workspace").c_str())),
        dcaMin_(dcaMin),
        dcaMax_(dcaMax),
        nBins_(nBins),
        massVarName_( massVarName ),
        dcaVar_(nullptr),
        massVar_(nullptr),
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
        dcaVar_ = new RooRealVar("dca3D", "Distance of Closest Approach", dcaMin_, dcaMax_, "cm"); // Add units if known
        massVar_= new RooRealVar(opt.massVar.c_str(), "Mass Variable", opt.massMin, opt.massMax, "GeV/c^{2}"); // Assuming GeV/c^2 for mass
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
             dcaVar_->SetName("dca3D");
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
    template <typename SigFitParams, typename BkgFitParams>
    bool fitMassInSliceAndGetYield(RooDataSet* sliceData, RooRealVar* massVar,
                                   double& yield, double& yieldError,
                                   const std::string& sliceName, // 플롯 저장 등을 위한 이름
                                   const SigFitParams& sigParams, // 시그널 PDF 파라미터
                                   const BkgFitParams& bkgParams  // 배경 PDF 파라미터
                                   ) {
        if (!sliceData || sliceData->numEntries() == 0) {
            std::cout << "No data in slice " << sliceName << " to fit." << std::endl;
            yield = 0;
            yieldError = 0;
            return false;
        }
        std::cout << "Fitting mass for DCA slice: " << sliceName << " with " << sliceData->sumEntries() << " weighted entries." << std::endl;

        // MassFitter 인스턴스 생성
        // MassFitter 생성자는 (const std::string& name, std::string& massvar, double massMin, double massMax) 형태
        // std::string fitterName = "massFit_" + name_ + "_" + sliceName;
        std::string massVarNameStr = massVar->GetName(); // RooRealVar 이름에서 std::string 얻기

        MassFitter massFitter(opt_.name, massVarNameStr, massVar->getMin(), massVar->getMax());
        // MassFitter.loadDataSet(sliceData); // 슬라이스된 데이터셋을 MassFitter에 전달

        // MassFitter에 필요한 FitOpt 객체 설정 (DCAFitter의 opt_를 사용하거나 새로 생성)
        FitOpt massFitOpt = opt_; // DCAFitter의 FitOpt를 복사하여 사용
        massFitOpt.outputDir = opt_.outputDir + "/mass_fits_" + name_ + "/"; // 슬라이스별 하위 디렉토리
        massFitOpt.outputFile = "massfit_" + sliceName + ".root";
        massFitOpt.plotName = "massfit_plot_" + sliceName; // MassFitter 내부 플롯 이름
        // massFitOpt.cutExpr = ""; // MassFitter에는 이미 슬라이스된 데이터가 전달되므로 추가 컷은 필요 없을 수 있음
        // MassFitter.SetData(sliceData);
        // std::string filePath = "plots/Data_DStar_ppRef/" + "mass_fits_" + name_ + "/";
        

        // MassFitter::PerformFit 호출
        // MassFitter.h의 PerformFit 템플릿 시그니처:
        // template <typename SigPar, typename BkgPar>
        // void PerformFit(FitOpt opt, RooDataSet* dataset, bool inclusive, const std::string pTbin,const std::string etabin, SigPar sigParams, BkgPar bkgParams);
        // 여기서는 inclusive=true, pTbin/etabin은 빈 문자열로 전달 (이미 슬라이스됨)
        massFitter.PerformFit<SigFitParams, BkgFitParams>(massFitOpt, sliceData, true, "", "", sigParams, bkgParams);
        

        if (massFitter.GetSignalYield() >= 0) { // 유효한 수율 값인지 확인 (음수가 아님)
            yield = massFitter.GetSignalYield();
            yieldError = massFitter.GetSignalYieldError();
            std::cout << "Slice " << sliceName << ": Yield = " << yield << " +/- " << yieldError << std::endl;
            return true;
        } else {
            std::cerr << "Warning: Mass fit for slice " << sliceName << " did not produce a valid yield." << std::endl;
            yield = 0;
            yieldError = 0; // 또는 매우 큰 값으로 설정하여 문제 표시
            return false;
        }
    }

        bool buildModel() { // 데이터 기반 템플릿 사용 여부 플래그 추가
        std::cout << "Building RooFit model..." << std::endl;

        if (!dataSet_) { // dataSet_은 data를 로드할 때 ws_에 "dataSet"으로 저장됨
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

        RooRealVar* dca = ws_->var(dcaVar_->GetName()); // 표준 DCA 변수
        if (!dca) {
            // 생성자에서 dcaVar_가 "dca"로 생성되었으므로 이를 사용
            dca = dcaVar_;
            if (!dca) {
                 std::cerr << "Error: Standard 'dca' variable not found in workspace or dcaVar_." << std::endl;
                 return false;
            }
            ws_->import(*dca, RooFit::RecycleConflictNodes()); // 워크스페이스에 dcaVar_를 "dca"로 import
        }


            std::cout << "Building model using data-driven DCA templates from mass fits." << std::endl;
            // RooRealVar* mass = ws_->var(massVarName_.c_str()); // 설정된 질량 변수 이름으로 가져오기
            // RooRealVar* mass = new RooRealVar(mass)
            // if (!mass) {
            //     std::cerr << "Error: Mass variable '" << massVarName_ << "' not found in workspace." << std::endl;
            //     return false;
            // }

            // 1. DCA 수율 히스토그램 생성 (TH1D)
            delete dataYieldHist_;
            dataYieldHist_ = new TH1D("dataYieldHist", "DCA Yield from Mass Fits", dcaBins_.size() - 1, dcaBins_.data());
            dataYieldHist_->Sumw2();

            // --- MassFitter에 전달할 PDF 파라미터 정의 (예시) --
            // bkgExpParams.lambda = -1.0; bkgExpParams.lambda_min = -10.0; bkgExpParams.lambda_max = -0.01;
            // ... 다른 배경 PDF 파라미터 타입 사용 가능 ...
            // 예: PDFParams::ChebychevBkgParams bkgChebParams;
            //     bkgChebParams.coefficients = {0.1, -0.05}; ...

            // --- 실제 사용할 파라미터 타입으로 아래 템플릿 인자 수정 필요 ---
            // 예를 들어 CrystalBall과 Chebychev를 사용한다면:
            // PDFParams::CrystalBallParams sigFitParams; /* 값 설정 */
            // PDFParams::ChebychevBkgParams bkgFitParams; /* 값 설정 */
            // 그리고 fitMassInSliceAndGetYield<PDFParams::CrystalBallParams, PDFParams::ChebychevBkgParams>(...) 호출

            // 여기서는 Gaussian과 Exponential을 예시로 사용합니다.
            // 실제 분석에 맞는 파라미터 값으로 채워야 합니다.
            // DCAFitter의 opt_ 멤버를 통해 pT, eta 등의 정보를 얻어와서
            // 파라미터 초기값을 설정하는 로직을 추가할 수 있습니다.
            // 예: if (opt_.particle == "D0") { sigGaussParams.mean = 1.8648; ... }

            // 2. 각 DCA 빈에 대해 질량 피팅 수행
            for (size_t i = 0; i < dcaBins_.size() - 1; ++i) {
                double dcaLow = dcaBins_[i];
                double dcaHigh = dcaBins_[i+1];
                std::string sliceName = Form("dca_%.3f_%.3f", dcaLow, dcaHigh);
                std::replace(sliceName.begin(), sliceName.end(), '.', 'p'); // 파일 이름용으로 '.'을 'p'로 변경

                TString dcaCut = TString::Format("%s >= %f && %s < %f", dca->GetName(), dcaLow, dca->GetName(), dcaHigh);
                RooDataSet* dcaSliceData = dynamic_cast<RooDataSet*>(dataSet_->reduce(RooArgSet(*massVar_, *dca), dcaCut.Data()));

                double yield = 0;
                double yieldError = 0;
                bool fitSuccess = false;

                if (dcaSliceData && dcaSliceData->numEntries() > 0) {
                    
                    // Plotting the mass distribution for the current slice (기존 코드)
                    // RooPlot* massFrameBeforeFit = massVar_->frame(RooFit::Title(Form("Mass Distribution for %s (Before Fit)", sliceName.c_str())));
                    // dcaSliceData->plotOn(massFrameBeforeFit); 

                    // TCanvas* cSliceMassBeforeFit = new TCanvas(Form("cSliceMassBeforeFit_%s", sliceName.c_str()), Form("Mass Distribution for %s (Before Fit)", sliceName.c_str()), 800, 600);
                    // massFrameBeforeFit->Draw();

                    std::string plotDir = opt_.outputDir + "/mass_fits_" + name_ + "/slice_mass_distributions/";
                    gSystem->mkdir(plotDir.c_str(), kTRUE); 
                    // std::string plotFileNameBeforeFit = plotDir + "mass_dist_before_fit_" + sliceName + ".png"; 
                    // cSliceMassBeforeFit->SaveAs(plotFileNameBeforeFit.c_str());
                    // delete cSliceMassBeforeFit;
                    // delete massFrameBeforeFit;
                    // End of plotting before fit
                    
                    PDFParams::DBCrystalBallParams sigDBCBParams; // 예시: DBCrystalBall 시그널
                    sigDBCBParams.mean = 0.1455;        // D* mass in GeV
                    sigDBCBParams.mean_min = 0.1452;
                    sigDBCBParams.mean_max = 0.1458;
                    sigDBCBParams.sigmaR = 0.0005;
                    sigDBCBParams.sigmaR_min = 0.0001;
                    sigDBCBParams.sigmaR_max = 0.01;
                    sigDBCBParams.sigmaL = 0.0005;
                    sigDBCBParams.sigmaL_min = 0.0001;
                    sigDBCBParams.sigmaL_max = 0.01;
                    
                    sigDBCBParams.alphaL = 2.;
                    sigDBCBParams.alphaL_min = 0.0001;
                    sigDBCBParams.alphaL_max = 10;
                    sigDBCBParams.nL = 1.5;
                    sigDBCBParams.nL_min = 1;
                    sigDBCBParams.nL_max = 10;
                    
                    sigDBCBParams.alphaR = 2.;
                    sigDBCBParams.alphaR_min = 0.0001;
                    sigDBCBParams.alphaR_max = 10.;
                    sigDBCBParams.nR = 1.5;
                    sigDBCBParams.nR_min = 1;
                    sigDBCBParams.nR_max = 10;
                    
                    
                    PDFParams::PhenomenologicalParams bkgD0dstParams; // 예시: 지수 배경

                    bkgD0dstParams.p0 = 0.1;
                    bkgD0dstParams.p0_min = 0.0;
                    bkgD0dstParams.p0_max = 5.0;
                    bkgD0dstParams.p1 = 2;
                    bkgD0dstParams.p1_min = -10.0;
                    bkgD0dstParams.p1_max = 10.0;
                    bkgD0dstParams.p2 = -2;
                    bkgD0dstParams.p2_min = -10.0;
                    bkgD0dstParams.p2_max = 10.0;

                    std::cout << "Attempting to fit mass for slice: " << sliceName << std::endl;
                    
                    // --- MassFitter 인스턴스 생성 (fitMassInSliceAndGetYield 내부 로직을 일부 가져옴) ---
                    std::string massVarNameStr = massVar_->GetName();
                    MassFitter massFitter(opt_.name + "_" + sliceName, massVarNameStr, massVar_->getMin(), massVar_->getMax());
                    FitOpt massFitOpt = opt_;
                    massFitOpt.outputDir = opt_.outputDir + "/mass_fits_" + name_ + "/" + sliceName + "/"; // MassFitter용 상세 출력 경로
                    gSystem->mkdir(massFitOpt.outputDir.c_str(), kTRUE);
                    massFitOpt.outputFile = "massfit_details.root"; // MassFitter가 상세 정보를 저장할 파일
                    massFitOpt.plotName = "massfit_detail_plot";


                    // --- fitMassInSliceAndGetYield 호출 ---
                    // fitSuccess = fitMassInSliceAndGetYield<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>(
                    //     dcaSliceData, massVar_, yield, yieldError, sliceName, sigDBCBParams, bkgD0dstParams
                    // );
                    // --- 직접 MassFitter의 PerformFit 호출 (fitMassInSliceAndGetYield 함수 대신) ---
                    massFitter.PerformFit<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>(
                        massFitOpt, dcaSliceData, true, "", "", sigDBCBParams, bkgD0dstParams
                    );

                    if (massFitter.GetSignalYield() >= 0) {
                        yield = massFitter.GetSignalYield();
                        yieldError = massFitter.GetSignalYieldError();
                        fitSuccess = true;
                        std::cout << "Mass fit SUCCESS for DCA slice " << sliceName << ": Yield = " << yield << " +/- " << yieldError << std::endl;

                        // --- 피팅 결과 간략 플로팅 ---
                        // MassFitter가 내부 Workspace에 "model"과 "data"를 저장했다고 가정
                        // 또는 MassFitter에 GetModel(), GetData(), GetFitResult() 같은 getter가 필요
                        RooWorkspace* mfWs = massFitter.GetWorkspace(); // MassFitter에 GetWorkspace() 메서드가 있다고 가정!
                        if (mfWs) {
                            RooAbsPdf* sliceModel = mfWs->pdf("total_pdf"); // MassFitter 내부 모델 이름 가정
                            // RooAbsData* sliceDataForPlot = dcaSliceData; // MassFitter 내부 데이터 이름 가정
                            RooRealVar* sliceMassVar = mfWs->var(massVar_->GetName());


                            if (sliceModel &&  sliceMassVar) {
                                RooPlot* massFrameAfterFit = sliceMassVar->frame(RooFit::Title(Form("Mass Fit for %s", sliceName.c_str())));
                                dcaSliceData->plotOn(massFrameAfterFit, RooFit::Name("data_slice"));
                                sliceModel->plotOn(massFrameAfterFit, RooFit::Name("model_slice"), RooFit::LineColor(kRed));
                                // 필요시 컴포넌트 플로팅
                                // if (mfWs->pdf("signal")) mfWs->pdf("signal")->plotOn(massFrameAfterFit, RooFit::LineStyle(kDashed), RooFit::LineColor(kGreen+2));
                                // if (mfWs->pdf("background")) mfWs->pdf("background")->plotOn(massFrameAfterFit, RooFit::LineStyle(kDashed), RooFit::LineColor(kBlue+2));


                                TCanvas* cSliceFit = new TCanvas(Form("cSliceFit_%s", sliceName.c_str()), Form("Mass Fit for %s", sliceName.c_str()), 700, 500);
                                massFrameAfterFit->Draw();
                                
                                // 간단한 범례 추가
                                TLegend* legSlice = new TLegend(0.65, 0.75, 0.88, 0.88);
                                legSlice->AddEntry(massFrameAfterFit->findObject("data_slice"), "Data", "pe");
                                legSlice->AddEntry(massFrameAfterFit->findObject("model_slice"), "Total Fit", "l");
                                legSlice->SetBorderSize(0);
                                legSlice->SetFillStyle(0);
                                legSlice->Draw();

                                std::string fitPlotFileName = plotDir + "mass_fit_overview_" + sliceName + ".png";
                                cSliceFit->SaveAs(fitPlotFileName.c_str());
                                std::cout << "Saved mass fit overview for slice " << sliceName << " to " << fitPlotFileName << std::endl;
                                delete legSlice;
                                delete cSliceFit;
                                delete massFrameAfterFit;
                            } else {
                                std::cerr << "Could not retrieve model/data/var from MassFitter's workspace for slice " << sliceName << std::endl;
                            }
                        } else {
                             std::cerr << "Could not retrieve MassFitter's workspace for slice " << sliceName << std::endl;
                        }
                        // --- 간략 플로팅 끝 ---

                    } else {
                        std::cout << "Mass fit FAILED for DCA slice " << sliceName << ". Yield set to 0." << std::endl;
                        yield = 0; 
                        yieldError = 0; 
                        fitSuccess = false;
                    }
                    // --- MassFitter 호출 및 결과 처리 끝 ---


                } else {
                     std::cout << "No data in DCA slice: " << dcaLow << " - " << dcaHigh << ". Yield set to 0." << std::endl;
                     yield = 0; yieldError = 0;
                     fitSuccess = false; 
                }

                if (fitSuccess) {
                    dataYieldHist_->SetBinContent(i + 1, yield);
                    dataYieldHist_->SetBinError(i + 1, yieldError);
                } else {
                    dataYieldHist_->SetBinContent(i + 1, 0);
                    dataYieldHist_->SetBinError(i + 1, 1e9); // 피팅 실패 시 큰 오차
                }
                delete dcaSliceData;
            } // End of DCA bin loop

            // 3. 생성된 수율 히스토그램으로 RooDataHist (템플릿) 생성
            delete dataDrivenTemplate_;
            dataDrivenTemplate_ = new RooDataHist("dataDrivenTemplate", "Data-driven DCA Template", RooArgList(*dca), dataYieldHist_);
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
            // double nevt = dataForYieldEstimate ? dataForYieldEstimate->sumEntries() : 1000; // 데이터셋이 있으면 그것으로, 없으면 임의의 값
            double nevt = dataYieldHist_->Integral(); // 데이터셋이 있으면 그것으로, 없으면 임의의 값
            if (nevt <=0) nevt = 1000;
            cout << a++ << endl;


            // delete n_prompt_;
            n_prompt_ = new RooRealVar("n_prompt", "Number of prompt events", nevt * 0.8, 0, nevt * 1.5);
            ws_->import(*n_prompt_, RooFit::RecycleConflictNodes());
            // delete n_nonprompt_;
            n_nonprompt_ = new RooRealVar("n_nonprompt", "Number of non-prompt events", nevt * 0.2, 0, nevt * 1.5);
            ws_->import(*n_nonprompt_, RooFit::RecycleConflictNodes());
            cout << a++ << endl;

            RooArgList pdfList;
            RooArgList yieldList;
            cout << a++ << endl;


            if (promptTemplate_) {
                delete promptPdf_;
                promptPdf_ = new RooHistPdf("promptPdf", "Prompt PDF from MC", RooArgSet(*dca), *promptTemplate_);
                ws_->import(*promptPdf_, RooFit::RecycleConflictNodes());
                pdfList.add(*promptPdf_);
                yieldList.add(*n_prompt_);
                std::cout << "Prompt PDF created from MC template." << std::endl;
            }

            if (nonPromptTemplate_) {
                delete nonPromptPdf_;
                nonPromptPdf_ = new RooHistPdf("nonPromptPdf", "Non-prompt PDF from MC", RooArgSet(*dca), *nonPromptTemplate_);
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

            if (pdfList.getSize() == 2 && n_prompt_ && n_nonprompt_) {
                // delete fracPrompt_;
                fracPrompt_ = new RooFormulaVar("fracPrompt", "Prompt Fraction from MC fit", "@0 / (@0 + @1)", RooArgList(*n_prompt_, *n_nonprompt_));
                ws_->import(*fracPrompt_);
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

        // if (useMinos) {
        //     fitOptionsList.Add(RooFit::Minos(true).Clone());
        //     std::cout << "Using MINOS for error estimation." << std::endl;
        // }
        // if (dataSet_->isWeighted()) {
             fitOptionsList.Add(RooFit::SumW2Error(true).Clone()); // Use weighted likelihood / correct errors
        //      std::cout << "Using SumW2Error(true) for weighted dataset." << std::endl;
        // }

        // Perform the fit using the RooLinkedList
        // Ensure the model is fitted to the dataset using the correct variable ('dca')
        RooFitResult* fitResult = model_->fitTo(*dataDrivenTemplate_, fitOptionsList);


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
    void plotResults(RooFitResult* fitResult = nullptr, const std::string& plotName = "dca_fit_plot", bool useDataTemplates = true) {
        std::cout << "Plotting results (CMS style)..." << std::endl;
        RooRealVar* dca = ws_->var("dca3D"); // 표준 dca 변수
        if (!dca) {
            dca = dcaVar_; // 생성자에서 만들어진 dcaVar_ 사용
            if (!dca) {
                 std::cerr << "Error: Standard 'dca' variable not found in workspace or dcaVar_ for plotting." << std::endl;
                 return;
            }
        }

        if (!model_) {
            std::cerr << "Error: Model not available for plotting." << std::endl;
            return;
        }

        RooPlot* frame = dca->frame(RooFit::Title(" "));
        RooBinning customBinning(dcaBins_.size() - 1, dcaBins_.data());

        if (useDataTemplates) {
            // 데이터 기반 템플릿 (dataDrivenTemplate_)과 MC 템플릿으로 피팅한 결과를 플로팅
            if (!dataDrivenTemplate_) {
                std::cerr << "Error: dataDrivenTemplate_ (data yield histogram) not available for plotting." << std::endl;
                return;
            }
            // 1. dataDrivenTemplate_ (질량 피팅으로 얻은 수율 히스토그램) 플로팅
            dataDrivenTemplate_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_yield_hist"), RooFit::DataError(RooAbsData::SumW2));

            // 2. MC 템플릿 기반 모델 (model_)과 그 컴포넌트 플로팅
            model_->plotOn(frame, RooFit::Name("model_mc_fit"), RooFit::LineColor(kRed + 1));

            RooAbsPdf* mcPromptPdf = ws_->pdf("promptPdf"); // MC prompt PDF
            RooAbsPdf* mcNonPromptPdf = ws_->pdf("nonPromptPdf"); // MC non-prompt PDF

            if (mcNonPromptPdf) {
                model_->plotOn(frame, RooFit::Components(*mcNonPromptPdf), RooFit::Name("mc_nonprompt_comp"), RooFit::FillStyle(3354), RooFit::FillColor(kBlue - 9), RooFit::LineColor(kBlue + 1), RooFit::DrawOption("F"));
            }
            if (mcPromptPdf) {
                 // AddTo를 사용하여 스택 형태로 그리기
                model_->plotOn(frame, RooFit::Components(*mcPromptPdf), RooFit::Name("mc_prompt_comp"), RooFit::FillStyle(3345), RooFit::FillColor(kRed - 9), RooFit::LineColor(kRed + 1), RooFit::DrawOption("F"));
            }
            // 데이터를 다시 그려서 맨 위에 오도록 함
            dataDrivenTemplate_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_yield_hist"), RooFit::DataError(RooAbsData::SumW2));


        } else { // MC 템플릿을 사용하여 원본 데이터를 피팅한 경우 (기존 로직과 유사)
            if (!dataSet_) {
                dataSet_ = static_cast<RooDataSet*>(ws_->data("dataSet"));
                if (!dataSet_) {
                    std::cerr << "Error: Original dataset not available for plotting." << std::endl;
                    return;
                }
            }
            // 1. 원본 데이터 플로팅
            dataSet_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_original"), RooFit::DataError(RooAbsData::SumW2));

            // 2. 모델과 컴포넌트 플로팅
            model_->plotOn(frame, RooFit::Name("model_fit_to_data"), RooFit::LineColor(kRed + 1));

            RooAbsPdf* promptPdf = ws_->pdf("promptPdf"); // 또는 멤버 변수 promptPdf_
            RooAbsPdf* nonPromptPdf = ws_->pdf("nonPromptPdf"); // 또는 멤버 변수 nonPromptPdf_

            if (nonPromptPdf) {
                model_->plotOn(frame, RooFit::Components(*nonPromptPdf), RooFit::Name("nonprompt_comp"), RooFit::FillStyle(3354), RooFit::FillColor(kBlue - 9), RooFit::LineColor(kBlue + 1), RooFit::DrawOption("F"));
            }
            if (promptPdf) {
                // model_->plotOn(frame, RooFit::Components(*promptPdf), RooFit::AddTo("model_fit_to_data", nonPromptPdf ? "nonprompt_comp" : "model_fit_to_data"), RooFit::Name("prompt_comp"), RooFit::FillStyle(3345), RooFit::FillColor(kRed - 9), RooFit::LineColor(kRed + 1), RooFit::DrawOption("F"));
            }
            // 데이터를 다시 그려서 맨 위에 오도록 함
            dataSet_->plotOn(frame, RooFit::Binning(customBinning), RooFit::Name("data_original"), RooFit::DataError(RooAbsData::SumW2));
        }

        TCanvas* c = new TCanvas(plotName.c_str(), "DCA Fit Results", 800, 700);
        c->SetLogy();
        c->SetLeftMargin(0.15);

        frame->Draw();
        // Y축 제목: dataYieldHist_의 Y축 제목을 사용하거나, 빈 너비로 정규화된 형태로 설정
        if (dataYieldHist_ && useDataTemplates) {
             frame->GetYaxis()->SetTitle(Form("Yield / (%.3f cm)", dataYieldHist_->GetBinWidth(1)));
        } else if (dataSet_ && !useDataTemplates) {
            // 원본 데이터를 플로팅할 때의 Y축 제목 (예: Candidates / (bin width))
            // dcaBins_를 사용하므로 첫 번째 빈의 너비를 가져올 수 있음
            if (!dcaBins_.empty() && dcaBins_.size() > 1) {
                 frame->GetYaxis()->SetTitle(Form("Candidates / (%.3f cm)", dcaBins_[1] - dcaBins_[0]));
            } else {
                 frame->GetYaxis()->SetTitle("Candidates");
            }
        } else {
            frame->GetYaxis()->SetTitle("Entries");
        }

        frame->GetXaxis()->SetTitle(Form("%s (%s)", dca->GetTitle(), dca->getUnit()));
        frame->GetXaxis()->SetTitleOffset(1.1);
        frame->GetYaxis()->SetTitleOffset(1.4);
        frame->SetMinimum(0.1); // 최소값은 데이터에 맞게 조정
        frame->SetMaximum(frame->GetMaximum() * 1.8);

        TLegend* leg = new TLegend(0.55, 0.65, 0.88, 0.88);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->SetTextSize(0.04);

        if (useDataTemplates) {
            TObject* dataYieldObj = frame->findObject("data_yield_hist");
            TObject* modelFitObj = frame->findObject("model_mc_fit");
            TObject* mcPromptObj = frame->findObject("mc_prompt_comp");
            TObject* mcNonPromptObj = frame->findObject("mc_nonprompt_comp");

            if (dataYieldObj) leg->AddEntry(dataYieldObj, "Data Yield (from Mass Fit)", "pe");
            if (modelFitObj) leg->AddEntry(modelFitObj, "Total MC Fit", "l");
            if (mcPromptObj) leg->AddEntry(mcPromptObj, "Prompt D^{0} (MC)", "f");
            if (mcNonPromptObj) leg->AddEntry(mcNonPromptObj, "Non-Prompt D^{0} (MC)", "f");
        } else {
            TObject* dataOrigObj = frame->findObject("data_original");
            TObject* modelFitToDataObj = frame->findObject("model_fit_to_data");
            TObject* promptCompObj = frame->findObject("prompt_comp");
            TObject* nonPromptCompObj = frame->findObject("nonprompt_comp");

            if (dataOrigObj) leg->AddEntry(dataOrigObj, "Data", "pe");
            if (modelFitToDataObj) leg->AddEntry(modelFitToDataObj, "Total Fit", "l");
            if (promptCompObj) leg->AddEntry(promptCompObj, "Prompt D^{0} (MC)", "f");
            if (nonPromptCompObj) leg->AddEntry(nonPromptCompObj, "Non-Prompt D^{0} (MC)", "f");
        }
        leg->Draw();

        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.18, 0.93, "#bf{CMS} #it{Preliminary}");
        latex.SetTextSize(0.04);
        latex.DrawLatex(0.60, 0.93, "pp #sqrt{s_{NN}} = 5.36 TeV"); // 예시 텍스트

        latex.SetTextSize(0.035);
        // Fit fraction (fracPrompt_는 useDataTemplates 여부에 따라 다른 이름으로 저장될 수 있음)
        // RooFormulaVar* currentFracPrompt = nullptr;
        // if (useDataTemplates) {
        //     currentFracPrompt = static_cast<RooFormulaVar*>(ws_->obj("fracPrompt_mc_fit_to_data_yields"));
        // } else {
            // currentFracPrompt = static_cast<RooFormulaVar*>(ws_->obj("fracPrompt"));
        // }

        if (fitResult) {
            double frac_val = fracPrompt_->getVal();
            // 에러 전파는 n_prompt와 n_nonprompt가 fitResult에 의해 업데이트 되었을 때 의미가 있음
            double frac_err = 0;
            RooRealVar* np = ws_->var("n_prompt");
            RooRealVar* nnp = ws_->var("n_nonprompt");
            if (np && nnp && np->isConstant() == kFALSE && nnp->isConstant() == kFALSE) { // 피팅 파라미터일 경우
                 frac_err = fracPrompt_->getPropagatedError(*fitResult);
            } else if (np && nnp) { // 파라미터가 고정되어 있다면, 해당 값으로 계산된 에러 (주로 0)
                 frac_err = fracPrompt_->getPropagatedError(*fitResult); // 시도해볼 수 있음
            }


            latex.SetTextSize(0.038);
            latex.DrawLatex(0.55, 0.60, Form("Prompt frac. = %.1f #pm %.1f %%", frac_val*100, frac_err*100));
        }
        latex.DrawLatex(0.55, 0.55, Form("%0.2f < p_{T} < %0.2f GeV/c",opt_.pTMin, opt_.pTMax));
        latex.DrawLatex(0.55, 0.50, "|y| < 1");
        // latex.DrawLatex(0.55, 0.45, Form("%0.2f < cos#theta_{HX} < %0.2f", opt_.cosMin, opt_.cosMax));


        c->SaveAs((plotName + ".png").c_str());
        c->SaveAs((plotName + ".pdf").c_str());
        if (!outputFileName_.empty()) {
            TFile* fout = outFile_; // 멤버 변수 outFile_ 사용
            bool closeFileAfterWrite = false;
            if (!fout || !fout->IsOpen()) {
                fout = TFile::Open(outputFileName_.c_str(), "UPDATE");
                closeFileAfterWrite = true; // 이 함수 내에서 열었으면 닫아줘야 함
            }
            if (fout && fout->IsOpen()) {
                fout->cd(); // 파일의 루트 디렉토리로 이동
                c->Write((name_ + "_" + plotName).c_str(), TObject::kOverwrite); // 고유한 이름으로 저장
                if (closeFileAfterWrite) {
                    fout->Close();
                    delete fout; // 여기서 열었으면 여기서 닫고 삭제
                    if (outFile_ == fout) outFile_ = nullptr; // 멤버 변수도 업데이트
                }
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
    // vector<double> dcaBins_ = {0,0.1};
    FitOpt opt_;

    // Variables
    RooRealVar* dcaVar_;    // The DCA variable
    RooRealVar* massVar_; // The mass variable (pointer to object in workspace)
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
    std::string massVarName_;
    double massMin_;
    double massMax_;
    std::string massUnit_;


    // PDFs and Model (Owned by Workspace after import)
    RooHistPdf* promptPdf_;         // PDF from prompt template (pointer to object in workspace)
    RooHistPdf* nonPromptPdf_;      // PDF from non-prompt template (pointer to object in workspace)
    RooFormulaVar* fracPrompt_;        // Fraction of prompt component (pointer to object in workspace)
    RooAddPdf* model_;              // Combined model (pointer to object in workspace)

    // Parameters
    double dcaMin_;
    double dcaMax_;
    int nBins_;
    TH1D* dataYieldHist_ = nullptr;
    RooDataHist* dataDrivenTemplate_ = nullptr;
    RooHistPdf* dataDrivenPdf_ = nullptr;
    RooRealVar* n_totalData_ = nullptr;

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
    // delete massVar_;
}
#endif // DCA_FITTER_H