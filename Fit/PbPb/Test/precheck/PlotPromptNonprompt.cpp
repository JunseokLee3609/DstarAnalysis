// Plot prompt vs nonprompt D* DCA distributions from MC
// Usage (ROOT6.24):
//   root -l -b -q 'PlotPromptNonprompt.cpp()'
// Notes:
//   - Use RECO tree with matchGEN==1 (matched to gen)
//   - Prompt: matchGen_D1ancestorFlavor_ != 5
//   - Nonprompt: matchGen_D1ancestorFlavor_ == 5
//   - DCA range: 0 ~ 0.1 cm

#include <iostream>
#include <vector>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSystem.h"

#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"
#include "/home/jun502s/DstarAna/DStarAnalysis/Interface/simpleDMC.h"

void PlotPromptNonprompt(const char* mcDataset = "PbPb_MC_Oct22", const char* outDir = "results/prompt_nonprompt"){
  using namespace DataFormat;
  
  gStyle->SetOptStat(0);  // Disable statistics box
  
  // Open MC dataset
  SimpleDatasetManager mgr("/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json");
  auto mcInfo = mgr.GetDataset(mcDataset);
  if(mcInfo.file.empty()){
    std::cerr << "ERROR: dataset not found: " << mcDataset << std::endl;
    return;
  }
  
  TFile* mcFile = TFile::Open(mcInfo.file.c_str(), "READ");
  if(!mcFile || mcFile->IsZombie()){
    std::cerr << "ERROR: Cannot open: " << mcInfo.file << std::endl;
    return;
  }
  
  // Get RECO tree (with matching info)
  TTree* recoTree = (TTree*)mcFile->Get("skimTreeFlat");
  if(!recoTree){
    std::cerr << "ERROR: skimTreeFlat not found" << std::endl;
    mcFile->Close();
    return;
  }
  
  // Setup event structure
  simpleDStarMCTreeflat evt;
  evt.setTree(recoTree);
  
  // Custom DCA bin edges (matching DCAFitter.h)
  const double dcaBinEdges[] = {0.0, 0.0012, 0.0023, 0.0039, 0.0059, 0.0085, 0.0160, 0.0281, 0.0476, 0.07};
  const int nBins = sizeof(dcaBinEdges)/sizeof(double) - 1;
  
  TH1D* hPrompt = new TH1D("hPrompt", "Prompt D*; D^{0} DCA (cm); Normalized counts per cm", nBins, dcaBinEdges);
  TH1D* hNonprompt = new TH1D("hNonprompt", "Nonprompt D*; D^{0} DCA (cm); Normalized counts per cm", nBins, dcaBinEdges);
  
  hPrompt->SetLineColor(kBlue);
  hPrompt->SetLineWidth(2);
  hPrompt->SetMarkerColor(kBlue);
  hPrompt->SetMarkerStyle(20);
  hPrompt->SetMarkerSize(0.8);
  
  hNonprompt->SetLineColor(kRed);
  hNonprompt->SetLineWidth(2);
  hNonprompt->SetMarkerColor(kRed);
  hNonprompt->SetMarkerStyle(20);
  hNonprompt->SetMarkerSize(0.8);
  
  hPrompt->Sumw2();
  hNonprompt->Sumw2();
  
  // Fill histograms
  Long64_t nTotal = recoTree->GetEntries();
  std::cout << "Processing " << nTotal << " RECO entries..." << std::endl;
  
  for(Long64_t ie = 0; ie < nTotal; ++ie){
    recoTree->GetEntry(ie);
    
    if(ie % 100000 == 0){
      std::cout << "\rProgress: " << ie << "/" << nTotal;
      std::cout.flush();
    }
    
    // Only use matched candidates
    if(!evt.matchGEN) continue;
    
    // Reco D* kinematic info
    double dca = evt.dca3D;
    
    // Skip if DCA outside range
    if(dca < dcaBinEdges[0] || dca >= dcaBinEdges[nBins]) continue;
    
    // Classify prompt vs nonprompt based on matched Gen info
    if(evt.matchGen_D1ancestorFlavor_ == 2 || evt.matchGen_D1ancestorFlavor_ == 4){
      // Prompt: ancestor is not b
      hPrompt->Fill(dca);
    } else if(evt.matchGen_D1ancestorFlavor_ == 5){
      // Nonprompt: ancestor is b (flavor=5)
      hNonprompt->Fill(dca);
    } else {
      continue;
    }
  }
  std::cout << "\n";
  
  // Print statistics before normalization
  std::cout << "\n=== Before Normalization ===" << std::endl;
  std::cout << "Prompt entries: " << hPrompt->GetEntries() << std::endl;
  std::cout << "Nonprompt entries: " << hNonprompt->GetEntries() << std::endl;
  std::cout << "Prompt integral: " << hPrompt->Integral() << std::endl;
  std::cout << "Nonprompt integral: " << hNonprompt->Integral() << std::endl;
  
  // Normalize histograms (order matters!)
  // Step 1: Normalize by integral
  double promptIntegral = hPrompt->Integral();
  double nonpromptIntegral = hNonprompt->Integral();
  
  if(promptIntegral > 0) hPrompt->Scale(1.0 / promptIntegral);
  if(nonpromptIntegral > 0) hNonprompt->Scale(1.0 / nonpromptIntegral);
  
  // Step 2: Normalize by bin width (to get "normalized counts per cm")
  hPrompt->Scale(1.0, "width");
  hNonprompt->Scale(1.0, "width");
  
  std::cout << "\n=== After Normalization ===" << std::endl;
  std::cout << "Prompt integral: " << hPrompt->Integral() << std::endl;
  std::cout << "Nonprompt integral: " << hNonprompt->Integral() << std::endl;
  std::cout << "Prompt max: " << hPrompt->GetMaximum() << std::endl;
  std::cout << "Nonprompt max: " << hNonprompt->GetMaximum() << std::endl;
  
  // Create canvas and plot
  gSystem->mkdir(outDir, true);
  TCanvas* c = new TCanvas("cPromptNonprompt", "Prompt vs Nonprompt DCA (Matched RECO)", 1000, 700);
  c->SetLogy(1);  // Log scale for y-axis
  c->SetLeftMargin(0.15);
  c->SetBottomMargin(0.13);
  c->SetTopMargin(0.10);
  c->SetRightMargin(0.05);
  c->cd();
  
  // Calculate y-axis range for log scale
  double ymin = 0.01;
  double ymax = std::max(hPrompt->GetMaximum(), hNonprompt->GetMaximum());
  hPrompt->SetMinimum(ymin);
  hPrompt->SetMaximum(ymax * 100);
  
  // Draw both histograms
  hPrompt->Draw("E1 P");
  hNonprompt->Draw("SAME E1 P");
  
  // Add legend
  TLegend* leg = new TLegend(0.60, 0.75, 0.95, 0.88);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->SetTextSize(0.035);
  leg->AddEntry(hPrompt, "Prompt D^{*} (Blue)", "LP");
  leg->AddEntry(hNonprompt, "Nonprompt D^{*} (Red)", "LP");
  leg->Draw();
  
  // Save
  std::string outPath = std::string(outDir) + "/dca_prompt_nonprompt";
  c->SaveAs((outPath + ".png").c_str());
  c->SaveAs((outPath + ".pdf").c_str());
  
  std::cout << "\nPlots saved: " << outPath << ".[png|pdf]" << std::endl;
  
  // Cleanup
  //   delete c;
  //   delete leg;
  //   delete hPrompt;
  //   delete hNonprompt;
  
  mcFile->Close();
  //   delete mcFile;
}
