// #include "common/headers.h"
// #include "common/VertexCompositeTree.h"
// #include "common/funUtil.h"
#include "../../interface/simpleDMC.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TChain.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TSystemFile.h"
#include "TMath.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace DataFormat;

void GetFiles(char const* input, std::vector<std::string>& files);

class EventPlaneAnalyzer {
public:
    // Constructor - 원본과 동일한 매개변수 구조
    EventPlaneAnalyzer(const TString& outputFileNamePrefix, Int_t triggerIdx, Int_t minCen, Int_t maxCen);

    // Destructor
    ~EventPlaneAnalyzer() = default;

    // Main analysis function
    int Analyze(const char* inputFileList);

private:
    // Configuration parameters
    TString fOutputFileNamePrefix;
    Int_t fTriggerIdx;
    Int_t fMinCentrality;
    Int_t fMaxCentrality;

    // Histograms managed by unique_ptr
    std::map<TString, std::unique_ptr<TH1D>> fHist1D;
    std::map<TString, std::unique_ptr<TH2D>> fHist2D;

    // Private member functions
    void BookHistos();
    void WriteHistos();
    void FillChain(TChain& chain, const std::vector<std::string>& files);
};

// Constructor implementation - centering file 제거하고 단순화
EventPlaneAnalyzer::EventPlaneAnalyzer(const TString& outputFileNamePrefix, Int_t triggerIdx, Int_t minCen, Int_t maxCen)
    : fOutputFileNamePrefix(outputFileNamePrefix),
      fTriggerIdx(triggerIdx),
      fMinCentrality(minCen),
      fMaxCentrality(maxCen)
{
    TH1::SetDefaultSumw2(kTRUE);
    BookHistos();
}

void EventPlaneAnalyzer::FillChain(TChain& chain, const std::vector<std::string>& files) {
    for (const auto& file : files) {
        chain.Add(file.c_str());
    }
}

// Book histograms - 원본과 정확히 일치하도록 수정
void EventPlaneAnalyzer::BookHistos() {
    // Event plane angle histograms
    fHist1D["hephfAngleRaw"] = std::make_unique<TH1D>("hephfAngleRaw", "hephfAngleRaw; ephfAngleRaw", 180, -2, 2);
    fHist1D["hephfAngle"] = std::make_unique<TH1D>("hephfAngle", "hephfAngle; ephfAngle", 180, -2, 2);
    fHist1D["hephfAngleoff"] = std::make_unique<TH1D>("hephfAngleoff", "hephfAngleoff; ephfAngleoff", 180, -2, 2);

    // Full HF histograms
    fHist1D["hsumPtOrEt"] = std::make_unique<TH1D>("hsumPtOrEt", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw"] = std::make_unique<TH2D>("hQxvsQyRaw", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck"] = std::make_unique<TH2D>("hQxvsQyRawCheck", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw"] = std::make_unique<TH1D>("hPsiRaw", "", 180, -2, 2);
    fHist1D["hPsiRawCheck"] = std::make_unique<TH1D>("hPsiRawCheck", "", 180, -2, 2);

    // HF Plus histograms
    fHist1D["hephfpAngleRaw"] = std::make_unique<TH1D>("hephfpAngleRaw", "hephfpAngleRaw; ephfpAngleRaw", 180, -2, 2);
    fHist1D["hsumPtOrEt_Plus"] = std::make_unique<TH1D>("hsumPtOrEt_Plus", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw_Plus"] = std::make_unique<TH2D>("hQxvsQyRaw_Plus", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck_Plus"] = std::make_unique<TH2D>("hQxvsQyRawCheck_Plus", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw_Plus"] = std::make_unique<TH1D>("hPsiRaw_Plus", "", 180, -2, 2);
    fHist1D["hPsiRawCheck_Plus"] = std::make_unique<TH1D>("hPsiRawCheck_Plus", "", 180, -2, 2);

    // HF Minus histograms
    fHist1D["hephfmAngleRaw"] = std::make_unique<TH1D>("hephfmAngleRaw", "hephfmAngleRaw; ephfmAngleRaw", 180, -2, 2);
    fHist1D["hsumPtOrEt_Minus"] = std::make_unique<TH1D>("hsumPtOrEt_Minus", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw_Minus"] = std::make_unique<TH2D>("hQxvsQyRaw_Minus", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck_Minus"] = std::make_unique<TH2D>("hQxvsQyRawCheck_Minus", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw_Minus"] = std::make_unique<TH1D>("hPsiRaw_Minus", "", 180, -2, 2);
    fHist1D["hPsiRawCheck_Minus"] = std::make_unique<TH1D>("hPsiRawCheck_Minus", "", 180, -2, 2);

    // Track multiplicity histograms
    fHist1D["hNtrkoffline"] = std::make_unique<TH1D>("hNtrkoffline", "", 200, 0, 1000);
    fHist1D["hNtrkoffline_JpsiSel"] = std::make_unique<TH1D>("hNtrkoffline_JpsiSel", "", 200, 0, 1000);
    fHist1D["hNtrkHP"] = std::make_unique<TH1D>("hNtrkHP", "", 200, 0, 1000);
    fHist1D["hNtrkHP_JpsiSel"] = std::make_unique<TH1D>("hNtrkHP_JpsiSel", "", 200, 0, 1000);

    // Track Event Plane histograms
    fHist2D["hQxvsQyRaw_Trk"] = std::make_unique<TH2D>("hQxvsQyRaw_Trk", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRaw_Trk"] = std::make_unique<TH1D>("hPsiRaw_Trk", "", 180, -2, 2);
}

void EventPlaneAnalyzer::WriteHistos() {
    TFile* fOut = new TFile(Form("%s.root", fOutputFileNamePrefix.Data()), "recreate");
    fOut->cd();

    // 원본 writeHistos와 동일한 순서로 작성
    fHist1D["hephfAngleRaw"]->Write();
    fHist1D["hephfAngle"]->Write();
    fHist1D["hephfAngleoff"]->Write();

    fHist1D["hsumPtOrEt"]->Write();
    fHist2D["hQxvsQyRaw"]->Write();
    fHist2D["hQxvsQyRawCheck"]->Write();
    fHist1D["hPsiRaw"]->Write();
    fHist1D["hPsiRawCheck"]->Write();

    fHist1D["hephfpAngleRaw"]->Write();
    fHist1D["hsumPtOrEt_Plus"]->Write();
    fHist2D["hQxvsQyRaw_Plus"]->Write();
    fHist2D["hQxvsQyRawCheck_Plus"]->Write();
    fHist1D["hPsiRaw_Plus"]->Write();
    fHist1D["hPsiRawCheck_Plus"]->Write();

    fHist1D["hephfmAngleRaw"]->Write();
    fHist1D["hsumPtOrEt_Minus"]->Write();
    fHist2D["hQxvsQyRaw_Minus"]->Write();
    fHist2D["hQxvsQyRawCheck_Minus"]->Write();
    fHist1D["hPsiRaw_Minus"]->Write();
    fHist1D["hPsiRawCheck_Minus"]->Write();

    fHist1D["hNtrkoffline"]->Write();
    fHist1D["hNtrkoffline_JpsiSel"]->Write();
    fHist1D["hNtrkHP"]->Write();
    fHist1D["hNtrkHP_JpsiSel"]->Write();

    fHist1D["hPsiRaw_Trk"]->Write();
    fHist2D["hQxvsQyRaw_Trk"]->Write();

    fOut->Map();
    fOut->Close();
    delete fOut;
    std::cout << "Histograms written to " << fOutputFileNamePrefix << ".root" << std::endl;
}

int EventPlaneAnalyzer::Analyze(const char* inputFileList) {
  // std::ifstream inputFile(inputFileList);
    // if (!inputFile.is_open()) {
    //     std::cerr << "Error opening the input file list: " << inputFileList << std::endl;
    //     return 1;
    // }

    // std::string line;
    // std::vector<std::string> files;
    // std::cout << "File Content: " << std::endl;
    // while (getline(inputFile, line)) {
    //     std::cout << line << std::endl;
    //     files.push_back(line);
    // }
    // inputFile.close();

    // Setup trees using simpleDMC structures
    simpleDStarDataTreeevt *csTree = new simpleDStarDataTreeevt;
    simpleDStarDataTreeevt *wsTree = new simpleDStarDataTreeevt;
    
    TChain csChain("dStarana/PATCompositeNtuple");
    // TChain wsChain("dimucontana_wrongsign/PATCompositeNtuple");
    // FillChain(csChain, inputFileList);
    // FillChain(wsChain, files);
    csChain.Add(inputFileList);
    
    csTree->setTree(&csChain);
    // wsTree->setTree(&wsChain);

    // Track Event Plane setup
    TChain TrkEvtChain("eventplane/EventPlane");
    TrkEvtChain.Add(inputFileList);  // Add the input file list directly
    // FillChain(TrkEvtChain, inputFileList);
    Double_t trkQx, trkQy;
    TrkEvtChain.SetBranchAddress("trkQx", &trkQx);
    TrkEvtChain.SetBranchAddress("trkQy", &trkQy);

    Long64_t totalEvents = TrkEvtChain.GetEntries();
    std::cout << "cstree entries: " << csChain.GetEntries() << std::endl;
    // std::cout << "wstree entries: " << wsChain.GetEntries() << std::endl;
    std::cout << "TrkEvtChain entries: " << totalEvents << std::endl;

    // 원본과 동일한 루프 구조: jentry = 1부터 시작
    for (Long64_t jentry = 1; jentry < csChain.GetEntries(); jentry++) {
        if (jentry % (csChain.GetEntries() / 10) == 0)
            std::cout << "begin " << jentry << "th entry...." << std::endl;

        // Get track event plane data
        TrkEvtChain.GetEntry(jentry);
        
        // Get CS tree data
        if (csChain.GetEntry(jentry) < 0) {
            std::cout << "Invalid correct-sign entry!" << std::endl;
            return 0;
        }

        // Event selection cuts - 원본과 동일한 조건들
        // if (!csTree->trigHLT()[fTriggerIdx]) continue;  // 필요시 활성화
        // if (!csTree->evtSel()[2]) continue;             // 필요시 활성화
        // if (!csTree->evtSel()[3]) continue;             // 필요시 활성화
        
        // Centrality cut - 원본: if(cen<100 || cen>=160) continue;
        // Int_t cen = csTree->centrality();
        // if (cen < fMinCentrality || cen >= fMaxCentrality) continue;

        // J/psi selection - 원본과 동일한 로직
        bool isJpsi = false;
        for (UInt_t icand = 0; icand < csTree->candSize; icand++) {
            // if (!csTree->softCand(icand)) continue;      // 필요시 활성화
            if (std::abs(csTree->y[icand]) >= 1) continue;  // 원본: abs(csTree.y()[icand])>=2.4
            Float_t pt = csTree->pT[icand];
            Float_t mass = csTree->mass[icand];
            if (mass > 1.7 && mass < 2.1 && pt >= 0.2 && pt < 20) isJpsi = true;
        }
        if (!isJpsi) continue;  // 원본: if (isJpsi!=true) continue;

        // Event plane variables - 현재 simpleDStarDataTreeevt에 없으므로 더미값 사용
        // 실제로는 이 변수들을 simpleDStarDataTreeevt에 추가해야 함
        Float_t ephfAngle=csTree->ephfAngle[0];
        Float_t ephfAngleoff=csTree->ephfAngleoff[0];
        Float_t ephfAngleRaw=csTree->ephfAngleRaw[0];
        Float_t ephfsumCos=csTree->ephfsumCos[0];
        Float_t ephfsumSin=csTree->ephfsumSin[0];
        Float_t ephfsumCosRaw=csTree->ephfsumCosRaw[0]; //- 더미값으로 0이 아닌 값 사용
        Float_t ephfsumSinRaw=csTree->ephfsumSinRaw[0]; //- 더미값으로 0이 아닌 값 사용
        Float_t ephfsumPtOrEt=csTree->ephfsumPtOrEt[0]; //- 더미값으로 0이 아닌 값 사용

        // Fill histograms - 원본과 동일한 방식으로 히스토그램 채우기
        Double_t QxRaw = ephfsumPtOrEt > 0 ? ephfsumCosRaw / ephfsumPtOrEt : 0.0;
        Double_t QyRaw = ephfsumPtOrEt > 0 ? ephfsumSinRaw / ephfsumPtOrEt : 0.0;
        Double_t Psi2Raw = 0.5 * TMath::ATan2(QyRaw, QxRaw);
        Double_t Psi2RawCheck = 0.5 * TMath::ATan2(ephfsumSinRaw, ephfsumCosRaw);

        fHist1D["hephfAngleRaw"]->Fill(ephfAngleRaw);
        fHist1D["hephfAngle"]->Fill(ephfAngle);
        fHist1D["hephfAngleoff"]->Fill(ephfAngleoff);
        fHist1D["hsumPtOrEt"]->Fill(ephfsumPtOrEt);
        fHist2D["hQxvsQyRaw"]->Fill(QxRaw, QyRaw);
        fHist2D["hQxvsQyRawCheck"]->Fill(ephfsumCosRaw, ephfsumSinRaw);
        fHist1D["hPsiRaw"]->Fill(Psi2Raw);
        fHist1D["hPsiRawCheck"]->Fill(Psi2RawCheck);

        // Recentering - 더미값을 사용하므로 실제 재센터링은 의미 없음
        Double_t Qx_rec = QxRaw - 0.0;
        Double_t Qy_rec = QyRaw - 0.0;
        // fHist2D["hQxvsQyRec"]->Fill(Qx_rec, Qy_rec);
        Double_t Psi2Rec = 0.5 * TMath::ATan2(Qy_rec, Qx_rec);
        // fHist1D["hPsiRec"]->Fill(Psi2Rec);

        // for (int i = 1; i < 11; i++) {
        //     fHist1D[Form("hsin2iPsi_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec));
        //     fHist1D[Form("hcos2iPsi_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec));
        // }

        // HF Plus and Minus - 더미값 사용
        Float_t ephfpAngleRaw = csTree->ephfpAngleRaw[0];
        Float_t ephfpsumCosRaw = csTree->ephfpsumCosRaw[0];
        Float_t ephfpsumSinRaw = csTree->ephfpsumSinRaw[0];
        Float_t ephfpsumPtOrEt = csTree->ephfpsumPtOrEt[0];
        Float_t ephfmAngleRaw = csTree->ephfmAngleRaw[0];
        Float_t ephfmsumCosRaw= csTree->ephfmsumCosRaw[0];
        Float_t ephfmsumSinRaw= csTree->ephfmsumSinRaw[0];
        Float_t ephfmsumPtOrEt= csTree->ephfmsumPtOrEt[0];

        Double_t QxRaw_Plus = ephfpsumPtOrEt > 0 ? ephfpsumCosRaw / ephfpsumPtOrEt : 0.0;
        Double_t QyRaw_Plus = ephfpsumPtOrEt > 0 ? ephfpsumSinRaw / ephfpsumPtOrEt : 0.0;
        Double_t Psi2Raw_Plus = 0.5 * TMath::ATan2(QyRaw_Plus, QxRaw_Plus);
        Double_t Psi2RawCheck_Plus = 0.5 * TMath::ATan2(ephfpsumSinRaw, ephfpsumCosRaw);

        Double_t QxRaw_Minus = ephfmsumPtOrEt > 0 ? ephfmsumCosRaw / ephfmsumPtOrEt : 0.0;
        Double_t QyRaw_Minus = ephfmsumPtOrEt > 0 ? ephfmsumSinRaw / ephfmsumPtOrEt : 0.0;
        Double_t Psi2Raw_Minus = 0.5 * TMath::ATan2(QyRaw_Minus, QxRaw_Minus);
        Double_t Psi2RawCheck_Minus = 0.5 * TMath::ATan2(ephfmsumSinRaw, ephfmsumCosRaw);

        fHist1D["hephfpAngleRaw"]->Fill(ephfpAngleRaw);
        fHist1D["hephfmAngleRaw"]->Fill(ephfmAngleRaw);
        fHist1D["hsumPtOrEt_Plus"]->Fill(ephfpsumPtOrEt);
        fHist1D["hsumPtOrEt_Minus"]->Fill(ephfmsumPtOrEt);
        fHist2D["hQxvsQyRaw_Plus"]->Fill(QxRaw_Plus, QyRaw_Plus);
        fHist2D["hQxvsQyRaw_Minus"]->Fill(QxRaw_Minus, QyRaw_Minus);
        fHist2D["hQxvsQyRawCheck_Plus"]->Fill(ephfpsumCosRaw, ephfpsumSinRaw);
        fHist2D["hQxvsQyRawCheck_Minus"]->Fill(ephfmsumCosRaw, ephfmsumSinRaw);
        fHist1D["hPsiRaw_Plus"]->Fill(Psi2Raw_Plus);
        fHist1D["hPsiRaw_Minus"]->Fill(Psi2Raw_Minus);
        fHist1D["hPsiRawCheck_Plus"]->Fill(Psi2RawCheck_Plus);
        fHist1D["hPsiRawCheck_Minus"]->Fill(Psi2RawCheck_Minus);

        // Recentering for HF Plus and Minus - 더미값 사용
        Double_t Qx_rec_Plus = QxRaw_Plus - 0.0;
        Double_t Qy_rec_Plus = QyRaw_Plus - 0.0;
        Double_t Qx_rec_Minus = QxRaw_Minus - 0.0;
        Double_t Qy_rec_Minus = QyRaw_Minus - 0.0;

        // fHist2D["hQxvsQyRec_Plus"]->Fill(Qx_rec_Plus, Qy_rec_Plus);
        Double_t Psi2Rec_Plus = 0.5 * TMath::ATan2(Qy_rec_Plus, Qx_rec_Plus);
        // fHist1D["hPsiRec_Plus"]->Fill(Psi2Rec_Plus);
        // fHist2D["hQxvsQyRec_Minus"]->Fill(Qx_rec_Minus, Qy_rec_Minus);
        Double_t Psi2Rec_Minus = 0.5 * TMath::ATan2(Qy_rec_Minus, Qx_rec_Minus);
        // fHist1D["hPsiRec_Minus"]->Fill(Psi2Rec_Minus);

        // for (int i = 1; i < 11; i++) {
        //     fHist1D[Form("hsin2iPsi_Plus_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec_Plus));
        //     fHist1D[Form("hcos2iPsi_Plus_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec_Plus));
        //     fHist1D[Form("hsin2iPsi_Minus_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec_Minus));
        //     fHist1D[Form("hcos2iPsi_Minus_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec_Minus));
        // }

        // Track Event Plane
        Double_t Psi2Raw_Trk = 0.5 * TMath::ATan2(trkQy, trkQx);
        fHist2D["hQxvsQyRaw_Trk"]->Fill(trkQx, trkQy);
        fHist1D["hPsiRaw_Trk"]->Fill(Psi2Raw_Trk);

        // Recentering for Trk - 더미값 사용
        Double_t Qx_rec_Trk = trkQx - 0.0;
        Double_t Qy_rec_Trk = trkQy - 0.0;
        // fHist2D["hQxvsQyRec_Trk"]->Fill(Qx_rec_Trk, Qy_rec_Trk);
        Double_t Psi2Rec_Trk = 0.5 * TMath::ATan2(Qy_rec_Trk, Qx_rec_Trk);
        // fHist1D["hPsiRec_Trk"]->Fill(Psi2Rec_Trk);

        // for (int i = 1; i < 11; i++) {
        //     fHist1D[Form("hsin2iPsi_Trk_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec_Trk));
        //     fHist1D[Form("hcos2iPsi_Trk_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec_Trk));
        // }
    }

    // Clean up
    delete csTree;
    // delete wsTree;
    
    WriteHistos();
    return 0;
}

// Global function GetFiles (can be moved into the class if it uses class members)
void GetFiles(char const* input, std::vector<std::string>& files) {
    TSystemDirectory dir(input, input);
    TList *list = dir.GetListOfFiles();

    if (list) {
        TSystemFile *file;
        std::string fname;
        TIter next(list);
        while ((file = (TSystemFile*) next())) {
            fname = file->GetName();

            if (file->IsDirectory() && (fname.find(".") == std::string::npos)) {
                std::string newDir = std::string(input) + fname + "/";
                GetFiles(newDir.c_str(), files);
            }
            else if ((fname.find(".root") != std::string::npos)) {
                files.push_back(std::string(input) + fname);
                // std::cout << files.back() << std::endl; // uncomment for verbose file listing
            }
        }
    }
}

// How to use the class in your main function or ROOT macro:
// int eventplaneana(int argc, char* argv[]) {
//     // if (argc < 2) {
//     //     std::cerr << "Usage: " << argv[0] << " <input_file_list>" << std::endl;
//     //     return 1;
//     // }

//     // Example usage:
//     // Centering file: "/eos/user/x/xueli/jpsiHistos/EvtPlane.MB.ForRec.Correct.pt0p2to20.root"
//     // Output prefix: "EvtPlane.MB.ForFlat"
//     // Trigger index: 10 (corresponding to trigIdx in original code)
//     // Centrality range: 100 to 160 (original code: cen<100 || cen>=160)
//     EventPlaneAnalyzer analyzer(
//         "/eos/user/x/xueli/jpsiHistos/EvtPlane.MB.ForRec.Correct.pt0p2to20.root",
//         "EvtPlane.MB.ForFlat",
//         10, // trigIdx
//         100, 160 // centrality range
//     );

//     // return analyzer.Analyze(argv[1]);
// }

// Or for ROOT macro:
/*
void runAnalysis() {
    EventPlaneAnalyzer analyzer(
        "/eos/user/x/xueli/jpsiHistos/EvtPlane.MB.ForRec.Correct.pt0p2to20.root",
        "EvtPlane.MB.ForFlat",
        10, // trigIdx
        100, 160 // centrality range
    );

    // Replace "file_list.txt" with your actual file list
    analyzer.Analyze("file_list.txt");
}
*/
