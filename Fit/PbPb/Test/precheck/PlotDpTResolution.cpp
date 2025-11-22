#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TLegend.h>

#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"
#include "/home/jun502s/DstarAna/DStarAnalysis/Interface/simpleDMC.h"

void PlotDpTResolution() {
    gStyle->SetOptStat(0);
    
    SimpleDatasetManager mgr("/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json");
    auto mcInfo = mgr.GetDataset("PbPb_MC_Oct22");
    
    if(mcInfo.file.empty()) {
        std::cerr << "ERROR: Dataset not found" << std::endl;
        return;
    }
    
    TFile* mcFile = TFile::Open(mcInfo.file.c_str(), "READ");
    if(!mcFile || mcFile->IsZombie()) {
        std::cerr << "ERROR: Cannot open MC file: " << mcInfo.file << std::endl;
        return;
    }
    
    TTree* recoTree = (TTree*)mcFile->Get(mcInfo.dataset_name.c_str());
    if(!recoTree) {
        std::cerr << "ERROR: Cannot find reco tree" << std::endl;
        mcFile->Close();
        return;
    }
    
    DataFormat::simpleDStarMCTreeflat evt;
    evt.setTree(recoTree);
    
    TH1D* hDpT = new TH1D("hDpT", "pT Resolution;(reco p_{T} - gen p_{T})/gen p_{T};Events", 100, -0.5, 0.5);
    hDpT->Sumw2();
    
    TH2D* hDpTvsPt = new TH2D("hDpTvsPt", "pT Resolution vs pT;gen p_{T} (GeV/c);(reco p_{T} - gen p_{T})/gen p_{T}", 50, 0, 20, 100, -0.5, 0.5);
    hDpTvsPt->Sumw2();
    
    TH2D* hDpTvsY = new TH2D("hDpTvsY", "pT Resolution vs Rapidity;|gen y|;(reco p_{T} - gen p_{T})/gen p_{T}", 50, 0, 2, 100, -0.5, 0.5);
    hDpTvsY->Sumw2();
    
    Long64_t nEntries = recoTree->GetEntries();
    std::cout << "Processing " << nEntries << " entries..." << std::endl;
    
    for(Long64_t ie = 0; ie < nEntries; ++ie) {
        recoTree->GetEntry(ie);
        
        if(ie % 100000 == 0) {
            std::cout << "\rProcessed: " << ie << " / " << nEntries << std::flush;
        }
        
        if(!evt.matchGEN) continue;
        
        float recoPt = evt.pT;
        float genPt = evt.matchGen_DStarpT;
        float genY = evt.matchGen_DStary;
        
        if(genPt <= 0) continue;
        
        float dpT = (recoPt - genPt) / genPt;
        
        hDpT->Fill(dpT);
        hDpTvsPt->Fill(genPt, dpT);
        hDpTvsY->Fill(fabs(genY), dpT);
    }
    std::cout << "\rProcessed: " << nEntries << " / " << nEntries << std::endl;
    
    TCanvas* c1 = new TCanvas("c1", "dpT Distribution", 1200, 800);
    c1->Divide(2, 2);
    
    c1->cd(1);
    gPad->SetLogy();
    hDpT->SetLineColor(kBlue+1);
    hDpT->SetLineWidth(2);
    hDpT->Draw("HIST");
    
    TLatex tx;
    tx.SetNDC();
    tx.SetTextSize(0.04);
    tx.DrawLatex(0.15, 0.85, "D*^{#pm} #rightarrow D^{0}#pi^{#pm}");
    tx.DrawLatex(0.15, 0.80, Form("Entries: %.0f", hDpT->GetEntries()));
    tx.DrawLatex(0.15, 0.75, Form("Mean: %.4f", hDpT->GetMean()));
    tx.DrawLatex(0.15, 0.70, Form("RMS: %.4f", hDpT->GetRMS()));
    
    c1->cd(2);
    hDpT->SetLineColor(kRed+1);
    hDpT->Draw("HIST");
    tx.DrawLatex(0.15, 0.85, "D*^{#pm} #rightarrow D^{0}#pi^{#pm}");
    tx.DrawLatex(0.15, 0.80, Form("Entries: %.0f", hDpT->GetEntries()));
    tx.DrawLatex(0.15, 0.75, Form("Mean: %.4f", hDpT->GetMean()));
    tx.DrawLatex(0.15, 0.70, Form("RMS: %.4f", hDpT->GetRMS()));
    
    c1->cd(3);
    gPad->SetLogz();
    hDpTvsPt->Draw("COLZ");
    
    c1->cd(4);
    gPad->SetLogz();
    hDpTvsY->Draw("COLZ");
    
    c1->SaveAs("results/dpT_resolution.png");
    c1->SaveAs("results/dpT_resolution.pdf");
    
    TCanvas* c2 = new TCanvas("c2", "dpT vs pT Profile", 1000, 700);
    TProfile* profPt = hDpTvsPt->ProfileX("profPt");
    profPt->SetTitle("pT Resolution Profile;gen p_{T} (GeV/c);<(reco p_{T} - gen p_{T})/gen p_{T}>");
    profPt->SetMarkerStyle(20);
    profPt->SetMarkerColor(kBlue+1);
    profPt->SetLineColor(kBlue+1);
    profPt->SetLineWidth(2);
    profPt->Draw("E");
    
    TLatex tx2;
    tx2.SetNDC();
    tx2.SetTextSize(0.04);
    tx2.DrawLatex(0.15, 0.85, "D*^{#pm} #rightarrow D^{0}#pi^{#pm}");
    
    c2->SaveAs("results/dpT_resolution_vs_pt.png");
    c2->SaveAs("results/dpT_resolution_vs_pt.pdf");
    
    TCanvas* c3 = new TCanvas("c3", "dpT vs Y Profile", 1000, 700);
    TProfile* profY = hDpTvsY->ProfileX("profY");
    profY->SetTitle("pT Resolution Profile vs Rapidity;|gen y|;<(reco p_{T} - gen p_{T})/gen p_{T}>");
    profY->SetMarkerStyle(20);
    profY->SetMarkerColor(kRed+1);
    profY->SetLineColor(kRed+1);
    profY->SetLineWidth(2);
    profY->Draw("E");
    
    tx2.DrawLatex(0.15, 0.85, "D*^{#pm} #rightarrow D^{0}#pi^{#pm}");
    
    c3->SaveAs("results/dpT_resolution_vs_y.png");
    c3->SaveAs("results/dpT_resolution_vs_y.pdf");
    
    TFile* outFile = new TFile("results/dpT_resolution.root", "RECREATE");
    hDpT->Write();
    hDpTvsPt->Write();
    hDpTvsY->Write();
    profPt->Write();
    profY->Write();
    outFile->Close();
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "Total matched entries: " << hDpT->GetEntries() << std::endl;
    std::cout << "Mean dpT: " << hDpT->GetMean() << std::endl;
    std::cout << "RMS dpT: " << hDpT->GetRMS() << std::endl;
    std::cout << "Plots saved to results/" << std::endl;
    
    mcFile->Close();
    delete c1;
    delete c2;
    delete c3;
}
