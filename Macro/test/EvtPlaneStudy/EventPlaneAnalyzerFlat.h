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

class EventPlaneAnalyzerFlat {
public:
    // Constructor
    EventPlaneAnalyzerFlat(const TString& centeringFile, const TString& outputFileNamePrefix, 
                          Int_t triggerIdx, Int_t minCen, Int_t maxCen);

    // Destructor
    ~EventPlaneAnalyzerFlat() = default;

    // Main analysis function
    int Analyze(const char* inputFileList);

private:
    // Configuration parameters
    TString fCenteringFile;
    TString fOutputFileNamePrefix;
    Int_t fTriggerIdx;
    Int_t fMinCentrality;
    Int_t fMaxCentrality;

    // Histograms managed by unique_ptr
    std::map<TString, std::unique_ptr<TH1D>> fHist1D;
    std::map<TString, std::unique_ptr<TH2D>> fHist2D;

    // Mean values for recentering (loaded from file)
    Double_t fQxMean, fQyMean;
    Double_t fQxMeanPlus, fQyMeanPlus;
    Double_t fQxMeanMinus, fQyMeanMinus;
    Double_t fQxMeanTrk, fQyMeanTrk;

    // Private member functions
    void BookHistos();
    void WriteHistos();
    bool LoadCenteringParameters();
    void FillChain(TChain& chain, const std::vector<std::string>& files);
};

// Constructor implementation
EventPlaneAnalyzerFlat::EventPlaneAnalyzerFlat(const TString& centeringFile, const TString& outputFileNamePrefix,
                                               Int_t triggerIdx, Int_t minCen, Int_t maxCen)
    : fCenteringFile(centeringFile),
      fOutputFileNamePrefix(outputFileNamePrefix),
      fTriggerIdx(triggerIdx),
      fMinCentrality(minCen),
      fMaxCentrality(maxCen),
      fQxMean(0.0), fQyMean(0.0),
      fQxMeanPlus(0.0), fQyMeanPlus(0.0),
      fQxMeanMinus(0.0), fQyMeanMinus(0.0),
      fQxMeanTrk(0.0), fQyMeanTrk(0.0)
{
    TH1::SetDefaultSumw2(kTRUE);
    BookHistos();
    
    if (!LoadCenteringParameters()) {
        std::cerr << "Error: Failed to load centering parameters from " << fCenteringFile << std::endl;
    }
}

bool EventPlaneAnalyzerFlat::LoadCenteringParameters() {
    TFile* infile = TFile::Open(fCenteringFile);
    if (!infile || infile->IsZombie()) {
        std::cerr << "Error: Could not open centering file: " << fCenteringFile << std::endl;
        return false;
    }

    TH2D* hQxvsQyRaw_ForRec = dynamic_cast<TH2D*>(infile->Get("hQxvsQyRaw"));
    TH2D* hQxvsQyRaw_Plus_ForRec = dynamic_cast<TH2D*>(infile->Get("hQxvsQyRaw_Plus"));
    TH2D* hQxvsQyRaw_Minus_ForRec = dynamic_cast<TH2D*>(infile->Get("hQxvsQyRaw_Minus"));
    TH2D* hQxvsQyRaw_Trk_ForRec = dynamic_cast<TH2D*>(infile->Get("hQxvsQyRaw_Trk"));

    if (!hQxvsQyRaw_ForRec || !hQxvsQyRaw_Plus_ForRec || !hQxvsQyRaw_Minus_ForRec || !hQxvsQyRaw_Trk_ForRec) {
        std::cerr << "Error: One or more centering histograms not found in " << fCenteringFile << std::endl;
        infile->Close();
        return false;
    }

    // Rename histograms to avoid conflicts
    hQxvsQyRaw_ForRec->SetName("hQxvsQyRaw_ForRec");
    hQxvsQyRaw_Plus_ForRec->SetName("hQxvsQyRaw_Plus_ForRec");
    hQxvsQyRaw_Minus_ForRec->SetName("hQxvsQyRaw_Minus_ForRec");
    hQxvsQyRaw_Trk_ForRec->SetName("hQxvsQyRaw_Trk_ForRec");

    // Extract mean values
    fQxMean = hQxvsQyRaw_ForRec->GetMean(1);
    fQyMean = hQxvsQyRaw_ForRec->GetMean(2);
    fQxMeanPlus = hQxvsQyRaw_Plus_ForRec->GetMean(1);
    fQyMeanPlus = hQxvsQyRaw_Plus_ForRec->GetMean(2);
    fQxMeanMinus = hQxvsQyRaw_Minus_ForRec->GetMean(1);
    fQyMeanMinus = hQxvsQyRaw_Minus_ForRec->GetMean(2);
    fQxMeanTrk = hQxvsQyRaw_Trk_ForRec->GetMean(1);
    fQyMeanTrk = hQxvsQyRaw_Trk_ForRec->GetMean(2);
 
    infile->Close();
    std::cout << "Centering parameters loaded successfully from " << fCenteringFile << std::endl;
    return true;
}

void EventPlaneAnalyzerFlat::FillChain(TChain& chain, const std::vector<std::string>& files) {
    for (const auto& file : files) {
        chain.Add(file.c_str());
    }
}

void EventPlaneAnalyzerFlat::BookHistos() {
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
    fHist2D["hQxvsQyRec"] = std::make_unique<TH2D>("hQxvsQyRec", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec"] = std::make_unique<TH1D>("hPsiRec", "", 180, -2, 2);

    // Sin/Cos histograms for flattening - Full HF
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_%d", i)] = std::make_unique<TH1D>(Form("hsin2iPsi_%d", i), Form("hsin2iPsi_%d", i), 100, -1, 1);
        fHist1D[Form("hcos2iPsi_%d", i)] = std::make_unique<TH1D>(Form("hcos2iPsi_%d", i), Form("hcos2iPsi_%d", i), 100, -1, 1);
    }

    // HF Plus histograms
    fHist1D["hephfpAngleRaw"] = std::make_unique<TH1D>("hephfpAngleRaw", "hephfpAngleRaw; ephfpAngleRaw", 180, -2, 2);
    fHist1D["hsumPtOrEt_Plus"] = std::make_unique<TH1D>("hsumPtOrEt_Plus", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw_Plus"] = std::make_unique<TH2D>("hQxvsQyRaw_Plus", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck_Plus"] = std::make_unique<TH2D>("hQxvsQyRawCheck_Plus", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw_Plus"] = std::make_unique<TH1D>("hPsiRaw_Plus", "", 180, -2, 2);
    fHist1D["hPsiRawCheck_Plus"] = std::make_unique<TH1D>("hPsiRawCheck_Plus", "", 180, -2, 2);
    fHist2D["hQxvsQyRec_Plus"] = std::make_unique<TH2D>("hQxvsQyRec_Plus", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec_Plus"] = std::make_unique<TH1D>("hPsiRec_Plus", "", 180, -2, 2);

    // Sin/Cos histograms for flattening - HF Plus
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_Plus_%d", i)] = std::make_unique<TH1D>(Form("hsin2iPsi_Plus_%d", i), "", 100, -1, 1);
        fHist1D[Form("hcos2iPsi_Plus_%d", i)] = std::make_unique<TH1D>(Form("hcos2iPsi_Plus_%d", i), "", 100, -1, 1);
    }

    // HF Minus histograms
    fHist1D["hephfmAngleRaw"] = std::make_unique<TH1D>("hephfmAngleRaw", "hephfmAngleRaw; ephfmAngleRaw", 180, -2, 2);
    fHist1D["hsumPtOrEt_Minus"] = std::make_unique<TH1D>("hsumPtOrEt_Minus", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw_Minus"] = std::make_unique<TH2D>("hQxvsQyRaw_Minus", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck_Minus"] = std::make_unique<TH2D>("hQxvsQyRawCheck_Minus", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw_Minus"] = std::make_unique<TH1D>("hPsiRaw_Minus", "", 180, -2, 2);
    fHist1D["hPsiRawCheck_Minus"] = std::make_unique<TH1D>("hPsiRawCheck_Minus", "", 180, -2, 2);
    fHist2D["hQxvsQyRec_Minus"] = std::make_unique<TH2D>("hQxvsQyRec_Minus", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec_Minus"] = std::make_unique<TH1D>("hPsiRec_Minus", "", 180, -2, 2);

    // Sin/Cos histograms for flattening - HF Minus
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_Minus_%d", i)] = std::make_unique<TH1D>(Form("hsin2iPsi_Minus_%d", i), "", 100, -1, 1);
        fHist1D[Form("hcos2iPsi_Minus_%d", i)] = std::make_unique<TH1D>(Form("hcos2iPsi_Minus_%d", i), "", 100, -1, 1);
    }

    // Track multiplicity histograms
    fHist1D["hNtrkoffline"] = std::make_unique<TH1D>("hNtrkoffline", "", 200, 0, 1000);
    fHist1D["hNtrkoffline_JpsiSel"] = std::make_unique<TH1D>("hNtrkoffline_JpsiSel", "", 200, 0, 1000);
    fHist1D["hNtrkHP"] = std::make_unique<TH1D>("hNtrkHP", "", 200, 0, 1000);
    fHist1D["hNtrkHP_JpsiSel"] = std::make_unique<TH1D>("hNtrkHP_JpsiSel", "", 200, 0, 1000);

    // Track Event Plane histograms
    fHist2D["hQxvsQyRaw_Trk"] = std::make_unique<TH2D>("hQxvsQyRaw_Trk", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRaw_Trk"] = std::make_unique<TH1D>("hPsiRaw_Trk", "", 180, -2, 2);
    fHist2D["hQxvsQyRec_Trk"] = std::make_unique<TH2D>("hQxvsQyRec_Trk", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec_Trk"] = std::make_unique<TH1D>("hPsiRec_Trk", "", 180, -2, 2);

    // Sin/Cos histograms for flattening - Track
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_Trk_%d", i)] = std::make_unique<TH1D>(Form("hsin2iPsi_Trk_%d", i), Form("hsin2iPsi_Trk_%d", i), 100, -1, 1);
        fHist1D[Form("hcos2iPsi_Trk_%d", i)] = std::make_unique<TH1D>(Form("hcos2iPsi_Trk_%d", i), Form("hcos2iPsi_Trk_%d", i), 100, -1, 1);
    }
}

void EventPlaneAnalyzerFlat::WriteHistos() {
    TFile* fOut = new TFile(Form("%s.root", fOutputFileNamePrefix.Data()), "recreate");
    fOut->cd();

    // 원본과 동일한 순서로 저장
    fHist1D["hephfAngleRaw"]->Write();
    fHist1D["hephfAngle"]->Write();
    fHist1D["hephfAngleoff"]->Write();

    fHist1D["hsumPtOrEt"]->Write();
    fHist2D["hQxvsQyRaw"]->Write();
    fHist2D["hQxvsQyRawCheck"]->Write();
    fHist1D["hPsiRaw"]->Write();
    fHist1D["hPsiRawCheck"]->Write();
    fHist2D["hQxvsQyRec"]->Write();
    fHist1D["hPsiRec"]->Write();

    // Full HF sin/cos histograms
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_%d", i)]->Write();
        fHist1D[Form("hcos2iPsi_%d", i)]->Write();
    }

    fHist1D["hephfpAngleRaw"]->Write();
    fHist1D["hsumPtOrEt_Plus"]->Write();
    fHist2D["hQxvsQyRaw_Plus"]->Write();
    fHist2D["hQxvsQyRawCheck_Plus"]->Write();
    fHist1D["hPsiRaw_Plus"]->Write();
    fHist1D["hPsiRawCheck_Plus"]->Write();
    fHist2D["hQxvsQyRec_Plus"]->Write();
    fHist1D["hPsiRec_Plus"]->Write();

    // HF Plus sin/cos histograms
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_Plus_%d", i)]->Write();
        fHist1D[Form("hcos2iPsi_Plus_%d", i)]->Write();
    }

    fHist1D["hephfmAngleRaw"]->Write();  // 원본에 hephfpAngleRaw->Write() 중복이 있었으나 수정
    fHist1D["hsumPtOrEt_Minus"]->Write();
    fHist2D["hQxvsQyRaw_Minus"]->Write();
    fHist2D["hQxvsQyRawCheck_Minus"]->Write();
    fHist1D["hPsiRaw_Minus"]->Write();
    fHist1D["hPsiRawCheck_Minus"]->Write();
    fHist2D["hQxvsQyRec_Minus"]->Write();
    fHist1D["hPsiRec_Minus"]->Write();

    // HF Minus sin/cos histograms
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_Minus_%d", i)]->Write();
        fHist1D[Form("hcos2iPsi_Minus_%d", i)]->Write();
    }

    fHist1D["hNtrkoffline"]->Write();
    fHist1D["hNtrkoffline_JpsiSel"]->Write();
    fHist1D["hNtrkHP"]->Write();
    fHist1D["hNtrkHP_JpsiSel"]->Write();

    fHist1D["hPsiRaw_Trk"]->Write();
    fHist1D["hPsiRec_Trk"]->Write();
    fHist2D["hQxvsQyRaw_Trk"]->Write();
    fHist2D["hQxvsQyRec_Trk"]->Write();

    // Track sin/cos histograms
    for (int i = 1; i < 11; i++) {
        fHist1D[Form("hsin2iPsi_Trk_%d", i)]->Write();
        fHist1D[Form("hcos2iPsi_Trk_%d", i)]->Write();
    }

    fOut->Map();
    fOut->Close();
    delete fOut;
    std::cout << "Histograms written to " << fOutputFileNamePrefix << ".root" << std::endl;
}

int EventPlaneAnalyzerFlat::Analyze(const char* inputFileList) {
    std::ifstream inputFile(inputFileList);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening the input file list: " << inputFileList << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> files;
    std::cout << "File Content: " << std::endl;
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
    // FillChain(TrkEvtChain, files);
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

        // Get entries
        TrkEvtChain.GetEntry(jentry);
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
        Float_t ephfAngle = csTree->ephfAngle[0];
        Float_t ephfAngleoff = csTree->ephfAngleoff[0];
        Float_t ephfAngleRaw = csTree->ephfAngleRaw[0];
        Float_t ephfsumCos = csTree->ephfsumCos[0];
        Float_t ephfsumSin = csTree->ephfsumSin[0];
        Float_t ephfsumCosRaw= csTree->ephfsumCosRaw[0];// - 더미값
        Float_t ephfsumSinRaw= csTree->ephfsumSinRaw[0];// - 더미값
        Float_t ephfsumPtOrEt= csTree->ephfsumPtOrEt[0];// - 더미값

        // 원본과 동일한 계산
        Double_t QxRaw = ephfsumCosRaw / ephfsumPtOrEt;
        Double_t QyRaw = ephfsumSinRaw / ephfsumPtOrEt;
        Double_t Psi2Raw = 0.5 * TMath::ATan2(QyRaw, QxRaw);         // 원본: 1/2.*atan2(QyRaw,QxRaw)
        Double_t Psi2RawCheck = 0.5 * TMath::ATan2(ephfsumSinRaw, ephfsumCosRaw);

        // 원본 주석: these Q vector and Psi2 Raw plots are for checking with the last step
        fHist1D["hephfAngleRaw"]->Fill(ephfAngleRaw);
        fHist1D["hephfAngle"]->Fill(ephfAngle);
        fHist1D["hephfAngleoff"]->Fill(ephfAngleoff);

        fHist1D["hsumPtOrEt"]->Fill(ephfsumPtOrEt);
        fHist2D["hQxvsQyRaw"]->Fill(QxRaw, QyRaw);
        fHist2D["hQxvsQyRawCheck"]->Fill(ephfsumCosRaw, ephfsumSinRaw);
        fHist1D["hPsiRaw"]->Fill(Psi2Raw);
        fHist1D["hPsiRawCheck"]->Fill(Psi2RawCheck);

        // Recentering
        Double_t Qx_rec = QxRaw - fQxMean;
        Double_t Qy_rec = QyRaw - fQyMean;
        fHist2D["hQxvsQyRec"]->Fill(Qx_rec, Qy_rec);
        Double_t Psi2Rec = 0.5 * TMath::ATan2(Qy_rec, Qx_rec);  // 원본: 1/2.*atan2(Qy_rec,Qx_rec)
        fHist1D["hPsiRec"]->Fill(Psi2Rec);

        // Fill sin/cos histograms for flattening
        for (int i = 1; i < 11; i++) {
            fHist1D[Form("hsin2iPsi_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec));  // 원본: sin(2.*i*Psi2Rec)
            fHist1D[Form("hcos2iPsi_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec));  // 원본: cos(2.*i*Psi2Rec)
        }

        // HF Plus and Minus - 원본과 동일한 구조
        Float_t ephfpAngleRaw =  csTree->ephfpAngleRaw[0];
        Float_t ephfpsumCosRaw = csTree->ephfpsumCosRaw[0];// - 더미값
        Float_t ephfpsumSinRaw = csTree->ephfpsumSinRaw[0];// - 더미값
        Float_t ephfpsumPtOrEt = csTree->ephfpsumPtOrEt[0];// - 더미값
        Float_t ephfmAngleRaw =  csTree->ephfmAngleRaw[0];
        Float_t ephfmsumCosRaw = csTree->ephfmsumCosRaw[0];// - 더미값
        Float_t ephfmsumSinRaw = csTree->ephfmsumSinRaw[0];// - 더미값
        Float_t ephfmsumPtOrEt = csTree->ephfmsumPtOrEt[0];// - 더미값

        Double_t QxRaw_Plus = ephfpsumCosRaw / ephfpsumPtOrEt;
        Double_t QyRaw_Plus = ephfpsumSinRaw / ephfpsumPtOrEt;
        Double_t Psi2Raw_Plus = 0.5 * TMath::ATan2(QyRaw_Plus, QxRaw_Plus);
        Double_t Psi2RawCheck_Plus = 0.5 * TMath::ATan2(ephfpsumSinRaw, ephfpsumCosRaw);
        
        Double_t QxRaw_Minus = ephfmsumCosRaw / ephfmsumPtOrEt;
        Double_t QyRaw_Minus = ephfmsumSinRaw / ephfmsumPtOrEt;
        Double_t Psi2Raw_Minus = 0.5 * TMath::ATan2(QyRaw_Minus, QxRaw_Minus);
        Double_t Psi2RawCheck_Minus = 0.5 * TMath::ATan2(ephfmsumSinRaw, ephfmsumCosRaw);

        // 원본 주석: these Q vector and Psi2 Raw plots are for checking with the last step
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

        // Track Event Plane
        Double_t Psi2Raw_Trk = 0.5 * TMath::ATan2(trkQy, trkQx);  // 원본: 1/2.*TMath::ATan2(trkQy,trkQx)
        fHist2D["hQxvsQyRaw_Trk"]->Fill(trkQx, trkQy);
        fHist1D["hPsiRaw_Trk"]->Fill(Psi2Raw_Trk);

        // Recentering for Plus and Minus
        Double_t Qx_rec_Plus = QxRaw_Plus - fQxMeanPlus;
        Double_t Qy_rec_Plus = QyRaw_Plus - fQyMeanPlus;
        Double_t Qx_rec_Minus = QxRaw_Minus - fQxMeanMinus;
        Double_t Qy_rec_Minus = QyRaw_Minus - fQyMeanMinus;

        fHist2D["hQxvsQyRec_Plus"]->Fill(Qx_rec_Plus, Qy_rec_Plus);
        Double_t Psi2Rec_Plus = 0.5 * TMath::ATan2(Qy_rec_Plus, Qx_rec_Plus);  // 원본: 1/2.*atan2(Qy_rec_Plus,Qx_rec_Plus)
        fHist1D["hPsiRec_Plus"]->Fill(Psi2Rec_Plus);
        
        fHist2D["hQxvsQyRec_Minus"]->Fill(Qx_rec_Minus, Qy_rec_Minus);
        Double_t Psi2Rec_Minus = 0.5 * TMath::ATan2(Qy_rec_Minus, Qx_rec_Minus);  // 원본: 1/2.*atan2(Qy_rec_Minus,Qx_rec_Minus)
        fHist1D["hPsiRec_Minus"]->Fill(Psi2Rec_Minus);

        // Fill sin/cos histograms for Plus and Minus
        for (int i = 1; i < 11; i++) {
            fHist1D[Form("hsin2iPsi_Plus_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec_Plus));    // 원본: sin(2.*i*Psi2Rec_Plus)
            fHist1D[Form("hcos2iPsi_Plus_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec_Plus));    // 원본: cos(2.*i*Psi2Rec_Plus)
            fHist1D[Form("hsin2iPsi_Minus_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec_Minus));  // 원본: sin(2.*i*Psi2Rec_Minus)
            fHist1D[Form("hcos2iPsi_Minus_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec_Minus));  // 원본: cos(2.*i*Psi2Rec_Minus)
        }

        // Recentering for Track
        Double_t Qx_rec_Trk = trkQx - fQxMeanTrk;
        Double_t Qy_rec_Trk = trkQy - fQyMeanTrk;
        
        // Debug output - 원본과 동일 (but fixed variable names)
        if (jentry < 2) {
            std::cout << "QxRaw_Trk - QxMean_Trk = " << trkQx << " - " << fQxMeanTrk << " = " << Qx_rec_Trk << std::endl;
            std::cout << "QyRaw_Trk - QyMean_Trk = " << trkQy << " - " << fQyMeanTrk << " = " << Qy_rec_Trk << std::endl;
        }
        
        fHist2D["hQxvsQyRec_Trk"]->Fill(Qx_rec_Trk, Qy_rec_Trk);
        Double_t Psi2Rec_Trk = 0.5 * TMath::ATan2(Qy_rec_Trk, Qx_rec_Trk);  // 원본: 1/2.*TMath::ATan2(Qy_rec_Trk,Qx_rec_Trk)
        fHist1D["hPsiRec_Trk"]->Fill(Psi2Rec_Trk);

        // Fill sin/cos histograms for Track
        for (int i = 1; i < 11; i++) {
            fHist1D[Form("hsin2iPsi_Trk_%d", i)]->Fill(TMath::Sin(2. * i * Psi2Rec_Trk));  // 원본: sin(2.*i*Psi2Rec_Trk)
            fHist1D[Form("hcos2iPsi_Trk_%d", i)]->Fill(TMath::Cos(2. * i * Psi2Rec_Trk));  // 원본: cos(2.*i*Psi2Rec_Trk)
        }
    }

    // 원본과 동일한 출력 파일명
    std::cout << "fileName: " << fOutputFileNamePrefix << std::endl;

    delete csTree;
    // delete wsTree;
    WriteHistos();
    return 0;
}

// Global function GetFiles - 원본과 동일
// void GetFiles(char const* input, std::vector<std::string>& files) {
//     TSystemDirectory dir(input, input);
//     TList *list = dir.GetListOfFiles();

//     if (list) {
//         TSystemFile *file;
//         std::string fname;
//         TIter next(list);
//         while ((file = (TSystemFile*) next())) {
//             fname = file->GetName();

//             if (file->IsDirectory() && (fname.find(".") == std::string::npos)) {
//                 std::string newDir = std::string(input) + fname + "/";
//                 GetFiles(newDir.c_str(), files);
//             }
//             else if ((fname.find(".root") != std::string::npos)) {
//                 files.push_back(std::string(input) + fname);
//                 std::cout << files.back() << std::endl;  // 원본과 동일하게 파일명 출력
//             }
//         }
//     }
// }

// Main function - 원본과 동일한 구조
// int main(int argc, char* argv[]) {
//     if (argc == 2) {
//         // 원본과 동일한 파라미터: trigIdx=10, cen 조건 <100 || >=160
//         EventPlaneAnalyzerFlat analyzer(
//             "/eos/user/x/xueli/jpsiHistos/EvtPlane.MB.ForRec.Correct.pt0p2to20.root",  // centering file
//             "EvtPlane.MB.ForFlat",  // output prefix - 원본과 동일
//             10, 100, 160  // trigIdx, centrality range
//         );
//         return analyzer.Analyze(argv[1]);
//     } else {
//         std::cout << "ERROR" << std::endl;
//         return -1;
//     }
// }