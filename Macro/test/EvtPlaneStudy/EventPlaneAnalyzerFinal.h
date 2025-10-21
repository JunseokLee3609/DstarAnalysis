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

class EventPlaneAnalyzerFinal {
public:
    EventPlaneAnalyzerFinal(const TString& centeringFile, const TString& flatteningFile, 
                           const TString& outputFileNamePrefix, Int_t triggerIdx, 
                           Int_t minCen, Int_t maxCen);

    ~EventPlaneAnalyzerFinal() = default;

    int Analyze(const char* inputFileList);

private:
    TString fCenteringFile;
    TString fFlatteningFile;
    TString fOutputFileNamePrefix;
    Int_t fTriggerIdx;
    Int_t fMinCentrality;
    Int_t fMaxCentrality;

    std::map<TString, std::unique_ptr<TH1D>> fHist1D;
    std::map<TString, std::unique_ptr<TH2D>> fHist2D;

    Double_t fQxMean, fQyMean;
    Double_t fQxMeanPlus, fQyMeanPlus;
    Double_t fQxMeanMinus, fQyMeanMinus;
    Double_t fQxMeanTrk, fQyMeanTrk;

    Double_t fSin2iPsiMean[10];
    Double_t fCos2iPsiMean[10];
    Double_t fSin2iPsiMeanPlus[10];
    Double_t fCos2iPsiMeanPlus[10];
    Double_t fSin2iPsiMeanMinus[10];
    Double_t fCos2iPsiMeanMinus[10];
    Double_t fSin2iPsiMeanTrk[10];
    Double_t fCos2iPsiMeanTrk[10];

    void BookHistos();
    void WriteHistos();
    bool LoadCenteringParameters();
    bool LoadFlatteningParameters();
    void FillChain(TChain& chain, const std::vector<std::string>& files);
    
    Double_t Bounds(Double_t ang);
    Double_t Bounds2(Double_t ang);
};

EventPlaneAnalyzerFinal::EventPlaneAnalyzerFinal(const TString& centeringFile, const TString& flatteningFile,
                                                 const TString& outputFileNamePrefix, Int_t triggerIdx, 
                                                 Int_t minCen, Int_t maxCen)
    : fCenteringFile(centeringFile),
      fFlatteningFile(flatteningFile),
      fOutputFileNamePrefix(outputFileNamePrefix),
      fTriggerIdx(triggerIdx),
      fMinCentrality(minCen),
      fMaxCentrality(maxCen),
      fQxMean(0.0), fQyMean(0.0),
      fQxMeanPlus(0.0), fQyMeanPlus(0.0),
      fQxMeanMinus(0.0), fQyMeanMinus(0.0),
      fQxMeanTrk(0.0), fQyMeanTrk(0.0)
{
    for (int i = 0; i < 10; i++) {
        fSin2iPsiMean[i] = 0.0;
        fCos2iPsiMean[i] = 0.0;
        fSin2iPsiMeanPlus[i] = 0.0;
        fCos2iPsiMeanPlus[i] = 0.0;
        fSin2iPsiMeanMinus[i] = 0.0;
        fCos2iPsiMeanMinus[i] = 0.0;
        fSin2iPsiMeanTrk[i] = 0.0;
        fCos2iPsiMeanTrk[i] = 0.0;
    }

    TH1::SetDefaultSumw2(kTRUE);
    BookHistos();
    
    if (!LoadCenteringParameters()) {
        std::cerr << "Error: Failed to load centering parameters from " << fCenteringFile << std::endl;
    }
    
    if (!LoadFlatteningParameters()) {
        std::cerr << "Error: Failed to load flattening parameters from " << fFlatteningFile << std::endl;
    }
}

bool EventPlaneAnalyzerFinal::LoadCenteringParameters() {
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

    hQxvsQyRaw_ForRec->SetName("hQxvsQyRaw_ForRec");
    hQxvsQyRaw_Plus_ForRec->SetName("hQxvsQyRaw_Plus_ForRec");
    hQxvsQyRaw_Minus_ForRec->SetName("hQxvsQyRaw_Minus_ForRec");
    hQxvsQyRaw_Trk_ForRec->SetName("hQxvsQyRaw_Trk_ForRec");

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

bool EventPlaneAnalyzerFinal::LoadFlatteningParameters() {
    TFile* infileFlat = TFile::Open(fFlatteningFile);
    if (!infileFlat || infileFlat->IsZombie()) {
        std::cerr << "Error: Could not open flattening file: " << fFlatteningFile << std::endl;
        return false;
    }

    for (int i = 1; i < 11; i++) {
        TH1D* hsin2iPsi = dynamic_cast<TH1D*>(infileFlat->Get(Form("hsin2iPsi_%d", i)));
        TH1D* hcos2iPsi = dynamic_cast<TH1D*>(infileFlat->Get(Form("hcos2iPsi_%d", i)));
        TH1D* hsin2iPsi_Plus = dynamic_cast<TH1D*>(infileFlat->Get(Form("hsin2iPsi_Plus_%d", i)));
        TH1D* hcos2iPsi_Plus = dynamic_cast<TH1D*>(infileFlat->Get(Form("hcos2iPsi_Plus_%d", i)));
        TH1D* hsin2iPsi_Minus = dynamic_cast<TH1D*>(infileFlat->Get(Form("hsin2iPsi_Minus_%d", i)));
        TH1D* hcos2iPsi_Minus = dynamic_cast<TH1D*>(infileFlat->Get(Form("hcos2iPsi_Minus_%d", i)));
        TH1D* hsin2iPsi_Trk = dynamic_cast<TH1D*>(infileFlat->Get(Form("hsin2iPsi_Trk_%d", i)));
        TH1D* hcos2iPsi_Trk = dynamic_cast<TH1D*>(infileFlat->Get(Form("hcos2iPsi_Trk_%d", i)));

        if (hsin2iPsi && hcos2iPsi) {
            fSin2iPsiMean[i-1] = hsin2iPsi->GetMean(1);
            fCos2iPsiMean[i-1] = hcos2iPsi->GetMean(1);
        }
        if (hsin2iPsi_Plus && hcos2iPsi_Plus) {
            fSin2iPsiMeanPlus[i-1] = hsin2iPsi_Plus->GetMean(1);
            fCos2iPsiMeanPlus[i-1] = hcos2iPsi_Plus->GetMean(1);
        }
        if (hsin2iPsi_Minus && hcos2iPsi_Minus) {
            fSin2iPsiMeanMinus[i-1] = hsin2iPsi_Minus->GetMean(1);
            fCos2iPsiMeanMinus[i-1] = hcos2iPsi_Minus->GetMean(1);
        }
        if (hsin2iPsi_Trk && hcos2iPsi_Trk) {
            fSin2iPsiMeanTrk[i-1] = hsin2iPsi_Trk->GetMean(1);
            fCos2iPsiMeanTrk[i-1] = hcos2iPsi_Trk->GetMean(1);
        }
    }

    infileFlat->Close();
    std::cout << "Flattening parameters loaded successfully from " << fFlatteningFile << std::endl;
    return true;
}

void EventPlaneAnalyzerFinal::FillChain(TChain& chain, const std::vector<std::string>& files) {
    for (const auto& file : files) {
        chain.Add(file.c_str());
    }
}

void EventPlaneAnalyzerFinal::BookHistos() {
    fHist1D["hephfAngleRaw"] = std::make_unique<TH1D>("hephfAngleRaw", "hephfAngleRaw; ephfAngleRaw", 180, -2, 2);
    fHist1D["hephfAngle"] = std::make_unique<TH1D>("hephfAngle", "hephfAngle; ephfAngle", 180, -2, 2);
    fHist1D["hephfAngleoff"] = std::make_unique<TH1D>("hephfAngleoff", "hephfAngleoff; ephfAngleoff", 180, -2, 2);

    fHist1D["hsumPtOrEt"] = std::make_unique<TH1D>("hsumPtOrEt", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw"] = std::make_unique<TH2D>("hQxvsQyRaw", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck"] = std::make_unique<TH2D>("hQxvsQyRawCheck", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw"] = std::make_unique<TH1D>("hPsiRaw", "", 180, -2, 2);
    fHist1D["hPsiRawCheck"] = std::make_unique<TH1D>("hPsiRawCheck", "", 180, -2, 2);
    fHist2D["hQxvsQyRec"] = std::make_unique<TH2D>("hQxvsQyRec", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec"] = std::make_unique<TH1D>("hPsiRec", "", 180, -2, 2);
    fHist1D["hPsiFlat"] = std::make_unique<TH1D>("hPsiFlat", "", 180, -2, 2);

    fHist1D["hephfpAngleRaw"] = std::make_unique<TH1D>("hephfpAngleRaw", "hephfpAngleRaw; ephfpAngleRaw", 180, -2, 2);
    fHist1D["hsumPtOrEt_Plus"] = std::make_unique<TH1D>("hsumPtOrEt_Plus", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw_Plus"] = std::make_unique<TH2D>("hQxvsQyRaw_Plus", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck_Plus"] = std::make_unique<TH2D>("hQxvsQyRawCheck_Plus", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw_Plus"] = std::make_unique<TH1D>("hPsiRaw_Plus", "", 180, -2, 2);
    fHist1D["hPsiRawCheck_Plus"] = std::make_unique<TH1D>("hPsiRawCheck_Plus", "", 180, -2, 2);
    fHist2D["hQxvsQyRec_Plus"] = std::make_unique<TH2D>("hQxvsQyRec_Plus", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec_Plus"] = std::make_unique<TH1D>("hPsiRec_Plus", "", 180, -2, 2);
    fHist1D["hPsiFlat_Plus"] = std::make_unique<TH1D>("hPsiFlat_Plus", "", 180, -2, 2);

    fHist1D["hephfmAngleRaw"] = std::make_unique<TH1D>("hephfmAngleRaw", "hephfmAngleRaw; ephfmAngleRaw", 180, -2, 2);
    fHist1D["hsumPtOrEt_Minus"] = std::make_unique<TH1D>("hsumPtOrEt_Minus", "", 2000, 0, 2000);
    fHist2D["hQxvsQyRaw_Minus"] = std::make_unique<TH2D>("hQxvsQyRaw_Minus", "", 160, -2, 2, 160, -2, 2);
    fHist2D["hQxvsQyRawCheck_Minus"] = std::make_unique<TH2D>("hQxvsQyRawCheck_Minus", "", 600, -300, 300, 600, -300, 300);
    fHist1D["hPsiRaw_Minus"] = std::make_unique<TH1D>("hPsiRaw_Minus", "", 180, -2, 2);
    fHist1D["hPsiRawCheck_Minus"] = std::make_unique<TH1D>("hPsiRawCheck_Minus", "", 180, -2, 2);
    fHist2D["hQxvsQyRec_Minus"] = std::make_unique<TH2D>("hQxvsQyRec_Minus", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec_Minus"] = std::make_unique<TH1D>("hPsiRec_Minus", "", 180, -2, 2);
    fHist1D["hPsiFlat_Minus"] = std::make_unique<TH1D>("hPsiFlat_Minus", "", 180, -2, 2);

    fHist1D["hNtrkoffline"] = std::make_unique<TH1D>("hNtrkoffline", "", 200, 0, 1000);
    fHist1D["hNtrkoffline_JpsiSel"] = std::make_unique<TH1D>("hNtrkoffline_JpsiSel", "", 200, 0, 1000);
    fHist1D["hNtrkHP"] = std::make_unique<TH1D>("hNtrkHP", "", 200, 0, 1000);
    fHist1D["hNtrkHP_JpsiSel"] = std::make_unique<TH1D>("hNtrkHP_JpsiSel", "", 200, 0, 1000);

    fHist2D["hQxvsQyRaw_Trk"] = std::make_unique<TH2D>("hQxvsQyRaw_Trk", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRaw_Trk"] = std::make_unique<TH1D>("hPsiRaw_Trk", "", 180, -2, 2);
    fHist2D["hQxvsQyRec_Trk"] = std::make_unique<TH2D>("hQxvsQyRec_Trk", "", 160, -2, 2, 160, -2, 2);
    fHist1D["hPsiRec_Trk"] = std::make_unique<TH1D>("hPsiRec_Trk", "", 180, -2, 2);
    fHist1D["hPsiFlat_Trk"] = std::make_unique<TH1D>("hPsiFlat_Trk", "", 180, -2, 2);

    fHist2D["hPsiFlat_TrkvsHF"] = std::make_unique<TH2D>("hPsiFlat_TrkvsHF", "", 180, -2, 2, 180, -2, 2);
    fHist1D["hCos2TrkHFp"] = std::make_unique<TH1D>("hCos2TrkHFp", "", 100, -1, 1);
    fHist1D["hCos2TrkHFm"] = std::make_unique<TH1D>("hCos2TrkHFm", "", 100, -1, 1);
    fHist1D["hCos2HFpHFm"] = std::make_unique<TH1D>("hCos2HFpHFm", "", 100, -1, 1);
}

void EventPlaneAnalyzerFinal::WriteHistos() {
    TFile* fOut = new TFile(Form("%s.root", fOutputFileNamePrefix.Data()), "recreate");
    fOut->cd();

    for (auto const& [key, val] : fHist1D) {
        if (val) val->Write();
    }
    for (auto const& [key, val] : fHist2D) {
        if (val) val->Write();
    }

    fOut->Map();
    fOut->Close();
    delete fOut;
    std::cout << "Histograms written to " << fOutputFileNamePrefix << ".root" << std::endl;
}

int EventPlaneAnalyzerFinal::Analyze(const char* inputFileList) {
    // std::ifstream inputFile(inputFileList);
    // if (!inputFile.is_open()) {
    //     std::cerr << "Error opening the input file list: " << inputFileList << std::endl;
    //     return 1;
    // }

    // std::string line;
    std::vector<std::string> files;
    std::cout << "File Content: " << std::endl;

    simpleDStarDataTreeevt *csTree = new simpleDStarDataTreeevt;
    
    TChain csChain("dStarana/PATCompositeNtuple");
    csChain.Add(inputFileList);
    
    csTree->setTree(&csChain);

    TChain TrkEvtChain("eventplane/EventPlane");
    Double_t trkQx, trkQy;
    TrkEvtChain.SetBranchAddress("trkQx", &trkQx);
    TrkEvtChain.SetBranchAddress("trkQy", &trkQy);

    Long64_t totalEvents = TrkEvtChain.GetEntries();
    std::cout << "cstree entries: " << csChain.GetEntries() << std::endl;
    std::cout << "TrkEvtChain entries: " << totalEvents << std::endl;

    for (Long64_t jentry = 1; jentry < csChain.GetEntries(); jentry++) {
        if (jentry % (csChain.GetEntries() / 10) == 0)
            std::cout << "begin " << jentry << "th entry...." << std::endl;

        TrkEvtChain.GetEntry(jentry);
        if (csChain.GetEntry(jentry) < 0) {
            std::cout << "Invalid correct-sign entry!" << std::endl;
            return 0;
        }

        bool isJpsi = false;
        for (UInt_t icand = 0; icand < csTree->candSize; icand++) {
            if (std::abs(csTree->y[icand]) >= 1) continue;
            Float_t pt = csTree->pT[icand];
            Float_t mass = csTree->mass[icand];
            if (mass > 1.7 && mass < 2.1 && pt >= 0.2 && pt < 20) isJpsi = true;
        }
        if (!isJpsi) continue;
        cout << "trkQx: " << trkQx << ", trkQy: " << trkQy << endl;

        Float_t ephfAngle = csTree->ephfAngle[0];
        Float_t ephfAngleoff = csTree->ephfAngleoff[0];
        Float_t ephfAngleRaw = csTree->ephfAngleRaw[0];
        Float_t ephfsumCos = csTree->ephfsumCos[0];
        Float_t ephfsumSin = csTree->ephfsumSin[0];
        Float_t ephfsumCosRaw = csTree->ephfsumCosRaw[0];
        Float_t ephfsumSinRaw = csTree->ephfsumSinRaw[0];
        Float_t ephfsumPtOrEt = csTree->ephfsumPtOrEt[0];

        Double_t QxRaw = ephfsumCosRaw / ephfsumPtOrEt;
        Double_t QyRaw = ephfsumSinRaw / ephfsumPtOrEt;
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

        Double_t Qx_rec = QxRaw - fQxMean;
        Double_t Qy_rec = QyRaw - fQyMean;
        fHist2D["hQxvsQyRec"]->Fill(Qx_rec, Qy_rec);
        Double_t Psi2Rec = 0.5 * TMath::ATan2(Qy_rec, Qx_rec);
        fHist1D["hPsiRec"]->Fill(Psi2Rec);

        Double_t deltaPsi2 = 0;
        for (int i = 1; i < 11; i++) {
            deltaPsi2 += (2.0 / i) * ((-1) * fSin2iPsiMean[i-1] * TMath::Cos(2. * i * Psi2Rec) + 
                                      fCos2iPsiMean[i-1] * TMath::Sin(2. * i * Psi2Rec));
        }
        Double_t PsiFlat = Psi2Rec + 0.5 * deltaPsi2;
        fHist1D["hPsiFlat"]->Fill(PsiFlat);

        Float_t ephfpAngleRaw = csTree->ephfpAngleRaw[0];
        Float_t ephfpsumCosRaw = csTree->ephfpsumCosRaw[0];
        Float_t ephfpsumSinRaw = csTree->ephfpsumSinRaw[0];
        Float_t ephfpsumPtOrEt = csTree->ephfpsumPtOrEt[0];
        Float_t ephfmAngleRaw = csTree->ephfmAngleRaw[0];
        Float_t ephfmsumCosRaw = csTree->ephfmsumCosRaw[0];
        Float_t ephfmsumSinRaw = csTree->ephfmsumSinRaw[0];
        Float_t ephfmsumPtOrEt = csTree->ephfmsumPtOrEt[0];

        if (ephfpsumPtOrEt > 0 && ephfmsumPtOrEt > 0) {
            Double_t QxRaw_Plus = ephfpsumCosRaw / ephfpsumPtOrEt;
            Double_t QyRaw_Plus = ephfpsumSinRaw / ephfpsumPtOrEt;
            Double_t Psi2Raw_Plus = 0.5 * TMath::ATan2(QyRaw_Plus, QxRaw_Plus);
            Double_t Psi2RawCheck_Plus = 0.5 * TMath::ATan2(ephfpsumSinRaw, ephfpsumCosRaw);
            
            Double_t QxRaw_Minus = ephfmsumCosRaw / ephfmsumPtOrEt;
            Double_t QyRaw_Minus = ephfmsumSinRaw / ephfmsumPtOrEt;
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

            Double_t Qx_rec_Plus = QxRaw_Plus - fQxMeanPlus;
            Double_t Qy_rec_Plus = QyRaw_Plus - fQyMeanPlus;
            Double_t Qx_rec_Minus = QxRaw_Minus - fQxMeanMinus;
            Double_t Qy_rec_Minus = QyRaw_Minus - fQyMeanMinus;

            fHist2D["hQxvsQyRec_Plus"]->Fill(Qx_rec_Plus, Qy_rec_Plus);
            fHist2D["hQxvsQyRec_Minus"]->Fill(Qx_rec_Minus, Qy_rec_Minus);

            Double_t Psi2Rec_Plus = 0.5 * TMath::ATan2(Qy_rec_Plus, Qx_rec_Plus);
            Double_t Psi2Rec_Minus = 0.5 * TMath::ATan2(Qy_rec_Minus, Qx_rec_Minus);

            fHist1D["hPsiRec_Plus"]->Fill(Psi2Rec_Plus);
            fHist1D["hPsiRec_Minus"]->Fill(Psi2Rec_Minus);

            Double_t deltaPsi2_Plus = 0, deltaPsi2_Minus = 0;
            for (int i = 1; i < 11; i++) {
                deltaPsi2_Plus += (2.0 / i) * ((-1) * fSin2iPsiMeanPlus[i-1] * TMath::Cos(2. * i * Psi2Rec_Plus) + 
                                               fCos2iPsiMeanPlus[i-1] * TMath::Sin(2. * i * Psi2Rec_Plus));
                deltaPsi2_Minus += (2.0 / i) * ((-1) * fSin2iPsiMeanMinus[i-1] * TMath::Cos(2. * i * Psi2Rec_Minus) + 
                                                fCos2iPsiMeanMinus[i-1] * TMath::Sin(2. * i * Psi2Rec_Minus));
            }
            Double_t PsiFlat_Plus = Psi2Rec_Plus + 0.5 * deltaPsi2_Plus;
            Double_t PsiFlat_Minus = Psi2Rec_Minus + 0.5 * deltaPsi2_Minus;

            fHist1D["hPsiFlat_Plus"]->Fill(PsiFlat_Plus);
            fHist1D["hPsiFlat_Minus"]->Fill(PsiFlat_Minus);

            Double_t Psi2Raw_Trk = 0.5 * TMath::ATan2(trkQy, trkQx);
            fHist2D["hQxvsQyRaw_Trk"]->Fill(trkQx, trkQy);
            fHist1D["hPsiRaw_Trk"]->Fill(Psi2Raw_Trk);

            Double_t Qx_rec_Trk = trkQx - fQxMeanTrk;
            Double_t Qy_rec_Trk = trkQy - fQyMeanTrk;
            fHist2D["hQxvsQyRec_Trk"]->Fill(Qx_rec_Trk, Qy_rec_Trk);
            Double_t Psi2Rec_Trk = 0.5 * TMath::ATan2(Qy_rec_Trk, Qx_rec_Trk);
            fHist1D["hPsiRec_Trk"]->Fill(Psi2Rec_Trk);

            Double_t deltaPsi2_Trk = 0;
            for (int i = 1; i < 11; i++) {
                deltaPsi2_Trk += (2.0 / i) * ((-1) * fSin2iPsiMeanTrk[i-1] * TMath::Cos(2. * i * Psi2Rec_Trk) + 
                                              fCos2iPsiMeanTrk[i-1] * TMath::Sin(2. * i * Psi2Rec_Trk));
            }
            Double_t PsiFlat_Trk = Psi2Rec_Trk + 0.5 * deltaPsi2_Trk;
            fHist1D["hPsiFlat_Trk"]->Fill(PsiFlat_Trk);

            fHist2D["hPsiFlat_TrkvsHF"]->Fill(PsiFlat_Trk, PsiFlat);
            fHist1D["hCos2TrkHFp"]->Fill(TMath::Cos(2. * (PsiFlat_Trk - PsiFlat_Plus)));
            fHist1D["hCos2TrkHFm"]->Fill(TMath::Cos(2. * (PsiFlat_Trk - PsiFlat_Minus)));
            fHist1D["hCos2HFpHFm"]->Fill(TMath::Cos(2. * (PsiFlat_Plus - PsiFlat_Minus)));
        }
    }

    Double_t Cos2TrkHFp_Mean = fHist1D["hCos2TrkHFp"]->GetMean(1);
    Double_t Cos2TrkHFm_Mean = fHist1D["hCos2TrkHFm"]->GetMean(1);
    Double_t Cos2HFpHFm_Mean = fHist1D["hCos2HFpHFm"]->GetMean(1);
    Double_t Cos2TrkR_Mean = TMath::Sqrt(Cos2TrkHFp_Mean * Cos2TrkHFm_Mean / Cos2HFpHFm_Mean);
    
    std::cout << "Cos2TrkR_Mean = sqrt(" << Cos2TrkHFp_Mean << " * " << Cos2TrkHFm_Mean 
              << " / " << Cos2HFpHFm_Mean << ") = " << Cos2TrkR_Mean << std::endl;

    delete csTree;
    WriteHistos();
    return 0;
}

Double_t EventPlaneAnalyzerFinal::Bounds(Double_t ang) {
    if (ang < -M_PI) ang += 2. * M_PI;
    if (ang > M_PI) ang -= 2. * M_PI;
    return ang;
}

Double_t EventPlaneAnalyzerFinal::Bounds2(Double_t ang) {
    Double_t range = M_PI / 2;
    while (ang < -range) ang += 2 * range;
    while (ang > range) ang -= 2 * range;
    return ang;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        EventPlaneAnalyzerFinal analyzer(
            "/eos/user/x/xueli/jpsiHistos/EvtPlane.MB.ForRec.Correct.pt0p2to20.root",
            "/eos/user/x/xueli/jpsiHistos/EvtPlane.MB.ForFlat.Correct.pt0p2to20.root",
            "EvtPlane.MB.Final",
            10, 100, 160
        );
        return analyzer.Analyze(argv[1]);
    } else {
        std::cout << "ERROR" << std::endl;
        return -1;
    }
}