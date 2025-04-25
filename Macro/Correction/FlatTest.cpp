#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.h"
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"


TH2* DrawCosThetaPhiDistribution(TH2D *histo, TCanvas *canvas,  const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
    Double_t massMin = 1.9, massMax = 2.1;

    Int_t nCosThetaBins = 20;
    Float_t cosThetaMin = -1, cosThetaMax = 1;

    Int_t nPhiBins = 25;
    Float_t phiMin = -200, phiMax = 300;
    /// Draw and save the number of events(signal+background) plots in the 2D (costheta, phi) space
    const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

    histo->SetTitle(Form(";cos #theta_{%s}; #varphi_{%s} (#circ)", frameAcronym, frameAcronym));
    histo->Draw("COLZ");

    TLatex legend;
    legend.SetTextAlign(22);
    legend.SetTextSize(0.05);
    legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

    histo->GetXaxis()->CenterTitle();
    histo->GetYaxis()->SetRangeUser(-200, 300);
    histo->GetYaxis()->CenterTitle();
    histo->GetZaxis()->SetMaxDigits(3);

    gPad->Update();

    return histo;
}

TH1* DrawCosThetaDistribution(TH1D *histo, TCanvas *canvas, TString drawoption="", const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
    Double_t massMin = 1.9, massMax = 2.1;

    Int_t nCosThetaBins = 20;
    Float_t cosThetaMin = -1, cosThetaMax = 1;

    const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

    histo->SetTitle(Form(";cos (#theta_{%s})", frameAcronym));
    histo->Draw(drawoption.Data());
    histo->GetYaxis()->SetRangeUser(0,histo->GetMaximum()*1.2);

    TLatex legend;
    legend.SetTextAlign(22);
    legend.SetTextSize(0.05);
    legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

    histo->GetXaxis()->CenterTitle();
    histo->GetYaxis()->SetRangeUser(0, histo->GetMaximum()*1.2);

    gPad->Update();

    return histo;
}

double findWeight(Float_t pt, Float_t y, Float_t phi, TH3D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(pt, y, phi);
        double eff = effMap->GetBinContent(bin);
        weight = 1.0/eff;
    }
    return weight;
}

double findWeight(Float_t x, Float_t y, TH2D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(x, y);
        double eff = effMap->GetBinContent(bin);
        if (eff > 0){
            weight = 1.0/eff;
        }
    }
    return weight;
}

double findWeight(Float_t x, TH1D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(x);
        double eff = effMap->GetBinContent(bin);
        weight = 1.0/eff;
    }
    return weight;
}

void FlatTest(
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_0_06Apr25.root"
    string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_1_07Apr25.root"
    // string fileNameReco="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_0_08Apr25.root"
){
    TFile* feffMap = TFile::Open("output.root");

    // Open separate files for Gen and Reco flat trees
    TFile* MCPR = TFile::Open(fileMCPR.c_str());
    // TFile* fReco = TFile::Open(fileNameReco.c_str());
    
    // if (!fGen || !fReco) {
    //     std::cerr << "Error opening input files" << std::endl;
    //     return;
    // }
    
    // Get the flat trees
    TTree* tGen = (TTree*)MCPR->Get("skimGENTreeFlat");
    TTree* tReco = (TTree*)MCPR->Get("skimTreeFlat");
    
    if (!tGen || !tReco) {
        std::cerr << "Error getting trees from files" << std::endl;
        return;
    }
    
    // Set up data structures for flat trees
    DataFormat::simpleDStarMCTreeflat evtGenFlat;
    DataFormat::simpleDStarMCTreeflat evtRecoFlat;
    
    // Set branch addresses
    evtGenFlat.setGENTree(tGen);
    evtRecoFlat.setTree(tReco);
    
    int nGenEntries = tGen->GetEntries();
    int nRecoEntries = tReco->GetEntries();
    
    std::cout << "Gen entries: " << nGenEntries << ", Reco entries: " << nRecoEntries << std::endl;
    
    // Create histograms
    TH1D* h = new TH1D("HX", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hNW = new TH1D("HX NoWeight", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN = new TH1D("HX Gen", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN2 = new TH1D("HX Gen2", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH2D* h2DGEN = new TH2D("HX 2D Gen", "HX;cos(#theta_{HX});#phi", 30, -1, 1, 100, -300, 300);

    // Process GEN tree entries
    for (int iEntry = 0; iEntry < nGenEntries; iEntry++) {
        tGen->GetEntry(iEntry);
        
        TLorentzVector Dstar, D0, D1, D0Dau1, D0Dau2;
        Dstar.SetPtEtaPhiM(evtGenFlat.gen_pT, evtGenFlat.gen_eta, evtGenFlat.gen_phi, evtGenFlat.gen_mass);
        D0.SetPtEtaPhiM(evtGenFlat.gen_D0pT, evtGenFlat.gen_D0eta, evtGenFlat.gen_D0phi, evtGenFlat.gen_D0mass);
        D1.SetPtEtaPhiM(evtGenFlat.gen_D1pT, evtGenFlat.gen_D1eta, evtGenFlat.gen_D1phi, evtGenFlat.gen_D1mass);
        D0Dau1.SetPtEtaPhiM(evtGenFlat.gen_D0Dau1_pT, evtGenFlat.gen_D0Dau1_eta, evtGenFlat.gen_D0Dau1_phi, evtGenFlat.gen_D0Dau1_mass);
        D0Dau2.SetPtEtaPhiM(evtGenFlat.gen_D0Dau2_pT, evtGenFlat.gen_D0Dau2_eta, evtGenFlat.gen_D0Dau2_phi, evtGenFlat.gen_D0Dau2_mass);
        
        if(fabs(evtGenFlat.gen_pT - Dstar.Pt()) > 0.01) 
            std::cout << "Dstar pT: " << Dstar.Pt() << " D0 pT: " << evtGenFlat.gen_pT << std::endl;
            
        if(fabs(evtGenFlat.gen_y - Dstar.Rapidity()) > 0.01) 
            std::cout << "Dstar y: " << Dstar.Rapidity() << " D0 y: " << evtGenFlat.gen_y << std::endl;
        
        // Fill hGEN with acceptance cuts
        if (evtGenFlat.gen_pT > DSGLPTLO && 
            fabs(evtGenFlat.gen_y) < DSGLABSY && 
            DstarDauSimpleAcc(D0, D1) &&
            D0DauAcc(D0Dau1, D0Dau2) &&
            true) {
            
            TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);
            hGEN->Fill(vect.CosTheta());
            h2DGEN->Fill(vect.CosTheta(), vect.Phi()/TMath::Pi()*180);
        }
        
        // Fill hGEN2 with minimal cuts
        if (evtGenFlat.gen_pT > DSGLPTLO && 
            fabs(evtGenFlat.gen_y) < DSGLABSY) {
            
            TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);
            hGEN2->Fill(vect.CosTheta());
        }
    }
    
    // Process RECO tree entries
    TH2D* effMap = (TH2D*)feffMap->Get("pt_cos_pr_pass_ratio");
    
    for (int iEntry = 0; iEntry < nRecoEntries; iEntry++) {
        tReco->GetEntry(iEntry);
        
        auto D0y = algo::rapidity(evtRecoFlat.pTD1, evtRecoFlat.EtaD1, evtRecoFlat.PhiD1, evtRecoFlat.massDaugther1);
        
        if (evtRecoFlat.matchGEN && !evtRecoFlat.isSwap &&
            fabs(evtRecoFlat.y) < DSGLABSY &&
            evtRecoFlat.pT > DSGLPTLO &&
            fabs(D0y) < DSGLABSY_D0 &&
            fabs(evtRecoFlat.EtaD2) < DSGLABSETA_D1 && 
            evtRecoFlat.pTD2 > DSGLPT_D1 && 
            fabs(evtRecoFlat.EtaGrandD1) < DSGLABSETA_D0DAU1 &&
            fabs(evtRecoFlat.EtaGrandD2) < DSGLABSETA_D0DAU2 &&
            evtRecoFlat.pTGrandD1 > DSGLPT_D0DAU1 &&
            evtRecoFlat.pTGrandD2 > DSGLPT_D0DAU2) {
            
            TLorentzVector Dstar, D0;
            Dstar.SetPtEtaPhiM(evtRecoFlat.pT, evtRecoFlat.eta, evtRecoFlat.phi, evtRecoFlat.mass);
            D0.SetPtEtaPhiM(evtRecoFlat.pTD1, evtRecoFlat.EtaD1, evtRecoFlat.PhiD1, evtRecoFlat.massDaugther1);
            TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);

            double weight = findWeight(evtRecoFlat.pT, vect.CosTheta(), effMap);
            hNW->Fill(vect.CosTheta());
            h->Fill(vect.CosTheta(), weight);
        }
    }
    
    // Draw histograms
    TCanvas* c = new TCanvas("c", "c", 800, 600);
    c->Divide(3,1);
    
    c->cd(1);
    h->GetYaxis()->SetRangeUser(0, h->GetMaximum()*1.2);
    h->Draw();
    
    c->cd(2);
    hNW->GetYaxis()->SetRangeUser(0, hNW->GetMaximum()*1.2);
    hNW->Draw();
    
    c->cd(3);
    hGEN2->GetYaxis()->SetRangeUser(0, hGEN2->GetMaximum()*1.2);
    hGEN2->SetMarkerStyle(20);
    hGEN2->SetMarkerSize(0.5);
    hGEN2->SetMarkerColor(kRed);
    hGEN2->SetLineColor(kRed);
    
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.5);
    h->SetMarkerColor(kBlue);
    h->SetLineColor(kBlue);
    
    TLegend* leg = new TLegend(0.11, 0.11, 0.28, 0.25);
    leg->AddEntry(hGEN, "GEN", "l");
    leg->AddEntry(h, "reco with weight", "l");
    leg->SetBorderSize(0);
    
    hGEN2->Draw("E");
    h->Draw("same");
    leg->Draw("same");
    
    c->SaveAs("test_2.png");
    
    TCanvas* c2 = new TCanvas("c2", "c2", 800, 600);
    c2->cd();
    
    hGEN2->GetYaxis()->SetRangeUser(0, hGEN2->GetMaximum()*1.2);
    hGEN2->SetMarkerStyle(20);
    hGEN2->SetMarkerSize(0.5);
    hGEN2->SetMarkerColor(kRed);
    hGEN2->SetLineColor(kRed);
    
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.5);
    h->SetMarkerColor(kBlue);
    h->SetLineColor(kBlue);
    
    TLegend* leg2 = new TLegend(0.11, 0.11, 0.28, 0.25);
    leg2->AddEntry(hGEN2, "GEN", "l");
    leg2->AddEntry(h, "reco with weight", "l");
    leg2->SetBorderSize(0);
    
    hGEN2->Draw("E");
    h->Draw("same");
    leg2->Draw("same");
    gStyle->SetOptStat(0);
    
    c2->SaveAs("test_3.png");
    
    // Clean up
    MCPR->Close();
    feffMap->Close();
}