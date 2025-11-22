// Stage 1 (Reweighting) - Independent macro
// Build reweighting function: w(x) = N_data(x) / N_MC_gen(x)
//
// Usage:
//   root -l -b -q 'CorrectYieldsWithMVA_Stage1_Independent.cpp(0, "results/data_fits", "incl")'
//
// Args:
//   varTypeInt: 0=PT, 1=RAPIDITY, 2=CENTRALITY
//   resultsDir: directory containing fitresult_*.root files
//   suffix: "incl", "prompt", or "nonprompt"

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <regex>
#include <string>
#include <cmath>

#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TH1D.h"
#include "TLine.h"

#include "RooFitResult.h"
#include "RooArgList.h"
#include "RooRealVar.h"

#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"
#include "/home/jun502s/DstarAna/DStarAnalysis/Interface/simpleDMC.h"
#include "BDTKinematicConfig.h"

namespace {
  using namespace BDTKinematicConfig;
  const std::string kBDTBaseDir = "/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Test/BDT_Evaluation";
  
  std::string AggregatedDir(const std::string& suffix = ""){
    std::string base = kBDTBaseDir + "/results/aggregated";
    if(!suffix.empty()){
      if(suffix.front() != '/') base += "/";
      base += suffix;
    }
    return base;
  }
  
  bool getParamValErr(RooFitResult* res, const char* name, double& val, double& err){
    if(!res) return false;
    RooArgList fl = res->floatParsFinal();
    if (auto* rr = dynamic_cast<RooRealVar*>(fl.find(name))) { val = rr->getVal(); err = rr->getError(); return true; }
    RooArgList cp = res->constPars();
    if (auto* rc = dynamic_cast<RooRealVar*>(cp.find(name))) { val = rc->getVal(); err = 0.0; return true; }
    return false;
  }

  // Build MC histogram once (independent of MVA)
  TH1D* BuildMCRecoHistogram(VarType vtype, const char* suffix = "incl"){
    std::cout << "\n=== Building MC Reco Histogram ===" << std::endl;
    
    SimpleDatasetManager mgr("/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json");
    auto mcInfo = mgr.GetDataset("PbPb_MC_PromptRich_Nov9");
    if(mcInfo.file.empty()){ 
      std::cerr << "ERROR: PbPb_MC_Oct22 dataset not found" << std::endl; 
      return nullptr; 
    }
    
    TFile* mcFile = TFile::Open(mcInfo.file.c_str(), "READ");
    if(!mcFile || mcFile->IsZombie()){ 
      std::cerr << "ERROR: Cannot open MC file: " << mcInfo.file << std::endl; 
      return nullptr; 
    }
    
    TTree* recoTree = (TTree*)mcFile->Get(mcInfo.dataset_name.c_str());
    if(!recoTree){ 
      std::cerr << "ERROR: Cannot find tree '" << mcInfo.dataset_name << "' in MC file" << std::endl; 
      mcFile->Close(); 
      return nullptr; 
    }
    
    std::cout << "Loaded MC tree: " << mcInfo.dataset_name << " (" << recoTree->GetEntries() << " entries)" << std::endl;
    
    const auto& edges = GetFineBinsEdges(vtype);
    TH1D* hMCReco = new TH1D("hMCReco", "MC Reco (matchGEN==1)", GetFineBinsNBins(vtype), edges.data());
    hMCReco->SetDirectory(0);  // Remove from gDirectory
    hMCReco->Sumw2();
    
    // Fill MC Reco histogram from matched events
    DataFormat::simpleDStarMCTreeflat evtReco;
    evtReco.setTree(recoTree);
    const Long64_t nReco = recoTree->GetEntries();
    
    Long64_t nTotal = 0, nMatchGEN = 0, nModePass = 0, nDCAPass = 0, nFilled = 0;
    const double dcaCut = 0.01;
    const bool isPrompt = (std::string(suffix) == "prompt");
    const bool isNonprompt = (std::string(suffix) == "nonprompt");
    
    for(Long64_t ie=0; ie<nReco; ++ie){
      recoTree->GetEntry(ie);
      nTotal++;
      
      if(!evtReco.matchGEN) continue;
      nMatchGEN++;
      
      // Mode filter
      int flav = evtReco.matchGen_D1ancestorFlavor_;
      if(isPrompt && flav==5) continue;
      if(isNonprompt && flav!=5) continue;
      nModePass++;
      
      // DCA filter (consistent with data selection)
      if(isPrompt && evtReco.dca3D >= dcaCut) continue;
      if(isNonprompt && evtReco.dca3D < dcaCut) continue;
      nDCAPass++;
      
      double val = 0;
      if(vtype == VarType::PT){ val = evtReco.pT; }
      else if(vtype == VarType::RAPIDITY){ val = fabs(evtReco.y); }
      else if(vtype == VarType::CENTRALITY){ val = evtReco.centrality; }
      else { continue; }
      
      hMCReco->Fill(val);
      nFilled++;
    }
    
    std::cout << "\n=== MC Reco Event Selection ===" << std::endl;
    std::cout << "Total events:          " << nTotal << std::endl;
    std::cout << "After matchGEN==1:     " << nMatchGEN << " (" << 100.0*nMatchGEN/nTotal << "%)" << std::endl;
    std::cout << "After mode filter:     " << nModePass << " (" << (nMatchGEN>0 ? 100.0*nModePass/nMatchGEN : 0) << "%)" << std::endl;
    std::cout << "After DCA filter:      " << nDCAPass << " (" << (nModePass>0 ? 100.0*nDCAPass/nModePass : 0) << "%)" << std::endl;
    std::cout << "Filled in histogram:   " << nFilled << std::endl;
    std::cout << "Histogram integral:    " << hMCReco->Integral() << std::endl;
    
    mcFile->Close();
    return hMCReco;
  }
  
  // Build reweighting function for specific MVA threshold
  TH1D* BuildReweightingFunction(const char* resultsDir, VarType vtype, TH1D* hMCReco, double mvaThreshold, const char* suffix = "incl"){
    std::cout << "\n=== Building Reweighting for MVA " << mvaThreshold << " ===" << std::endl;
    
    const auto& edges = GetFineBinsEdges(vtype);
    TH1D* hTarget = new TH1D(Form("hTarget_mva%s", FormatMVA(mvaThreshold).c_str()), "Data Yields", GetFineBinsNBins(vtype), edges.data());
    hTarget->SetDirectory(0);  // Remove from gDirectory
    hTarget->Sumw2();
    
    // Fill target histogram from data fit results
    const std::string varStr = VarName(vtype);
    char mvaBuf[64];
    snprintf(mvaBuf, sizeof(mvaBuf), "mva%.3f", mvaThreshold);
    std::string mvaTag = mvaBuf;
    for(char &c : mvaTag) if(c=='.') c='p';
    
    // Try both file naming patterns: fitresult_ (inclusive) and mass_fits_ (prompt/nonprompt)
    const std::string prefix1 = std::string("fitresult_") + varStr + "_";
    const std::string prefix2 = std::string("mass_fits_") + varStr + "_";
    int nFilesProcessed = 0;
    
    // Build set of valid fine bin ranges for filtering
    std::set<std::pair<double,double>> validFineBins;
    for(size_t i=0; i+1<edges.size(); ++i){
      validFineBins.insert({edges[i], edges[i+1]});
    }
    
    TSystemDirectory dir(resultsDir, resultsDir);
    TList* fl = dir.GetListOfFiles();
    
    if(!fl){
      std::cerr << "ERROR: Cannot read directory: " << resultsDir << std::endl;
      // delete hTarget;
      return nullptr;
    }
    
    {
      TIter next(fl);
      TSystemFile* f;
      while((f=(TSystemFile*)next())){
        std::string fname = f->GetName();
        
        // Check for mode suffix if not inclusive
        bool modeMatch = true;
        if(std::string(suffix) != "incl"){
          modeMatch = (fname.find(std::string("_") + suffix) != std::string::npos);
        }
        
        // Match either prefix pattern
        bool matchPrefix1 = (fname.find(prefix1) == 0);
        bool matchPrefix2 = (fname.find(prefix2) == 0);
        
        if((matchPrefix1 || matchPrefix2) && fname.find(mvaTag) != std::string::npos && modeMatch &&
           fname.size() > 5 && fname.substr(fname.size()-5) == ".root"){
          
          size_t start = matchPrefix1 ? prefix1.size() : prefix2.size();
          size_t pos_mva = fname.find("_mva");
          if(pos_mva == std::string::npos || pos_mva <= start) continue;
          
          std::string range = fname.substr(start, pos_mva - start);
          
          // Remove mode suffix if present (e.g., "_prompt" or "_nonprompt")
          if(std::string(suffix) != "incl"){
            std::string suffixStr = std::string("_") + suffix;
            size_t pos_suffix = range.find(suffixStr);
            if(pos_suffix != std::string::npos){
              range = range.substr(0, pos_suffix);
            }
          }
          
          size_t us = range.find('_');
          if(us == std::string::npos) continue;
          
          std::string smin = range.substr(0, us);
          std::string smax = range.substr(us+1);
          for(char &c : smin) if(c=='p') c='.';
          for(char &c : smax) if(c=='p') c='.';
          
          double vmin = atof(smin.c_str());
          double vmax = atof(smax.c_str());
          
          // Only process bins that match fine bin configuration
          bool isFineBin = (validFineBins.find({vmin, vmax}) != validFineBins.end());
          if(!isFineBin){
            continue; // Skip coarse bins
          }
          
          std::string fpath = std::string(resultsDir) + "/" + fname;
          TFile* tf = TFile::Open(fpath.c_str(), "READ");
          if(!tf || tf->IsZombie()){ 
            if(tf) tf->Close(); 
            std::cerr << "WARNING: Cannot open fit result: " << fname << std::endl;
            continue; 
          }
          
          RooFitResult* r = (RooFitResult*)tf->Get("fitResult");
          double y=0, ey=0;
          if(r && getParamValErr(r, "nsig", y, ey)){
            // For fine bins, assign full yield to the corresponding bin
            int bin = hTarget->FindBin((vmin + vmax) / 2.0);
            
            if(bin >= 1 && bin <= hTarget->GetNbinsX()){
              hTarget->SetBinContent(bin, y);
              hTarget->SetBinError(bin, ey);
              nFilesProcessed++;
            }
          } else {
            std::cerr << "WARNING: Cannot extract nsig from: " << fname << std::endl;
          }
          tf->Close();
        }
      }
    }
    
    std::cout << "\n=== Data Yield Loading ===" << std::endl;
    std::cout << "Files processed: " << nFilesProcessed << std::endl;
    
    std::cout << "Data yields total:   " << hTarget->Integral() << std::endl;
    
    if(hTarget->Integral() <= 0){
      std::cerr << "ERROR: No data yields found" << std::endl;
      // delete hTarget;
      return nullptr;
    }
    
    if(hMCReco->Integral() <= 0){
      std::cerr << "ERROR: No MC reco events found" << std::endl;
      // delete hTarget;
      return nullptr;
    }
    
    // Build weight histogram: w(x) = N_Data(x) / N_MC_Reco(x)
    const std::string mvaStr = FormatMVA(mvaThreshold);
    TH1D* hWeight = (TH1D*)hTarget->Clone(Form("hWeight_mva%s", mvaStr.c_str()));
    hWeight->SetDirectory(0);  // Remove from gDirectory to avoid conflicts
    hWeight->SetTitle("Reweighting factor");
    hWeight->Divide(hTarget, hMCReco, 1.0, 1.0, "");
    
    // Check for problematic bins
    int nZeroBins = 0, nLargeBins = 0;
    for(int b=1; b<=hWeight->GetNbinsX(); ++b){
      double w = hWeight->GetBinContent(b);
      if(w <= 0) nZeroBins++;
      if(w > 10) nLargeBins++;
    }
    if(nZeroBins > 0) std::cout << "WARNING: " << nZeroBins << " bins have zero or negative weight" << std::endl;
    if(nLargeBins > 0) std::cout << "WARNING: " << nLargeBins << " bins have weight > 10" << std::endl;
    
    // Plot reweighting function
    TCanvas* c = new TCanvas("cWeight", "Reweighting", 900, 700);
    c->SetLeftMargin(0.13);
    c->SetBottomMargin(0.12);
    c->SetRightMargin(0.05);
    
    hWeight->SetMarkerStyle(20);
    hWeight->SetMarkerSize(1.2);
    hWeight->SetMarkerColor(kBlack);
    hWeight->SetLineColor(kBlack);
    hWeight->SetLineWidth(2);
    hWeight->GetXaxis()->SetTitle(VarLabel(vtype));
    hWeight->GetXaxis()->SetTitleSize(0.045);
    hWeight->GetXaxis()->SetLabelSize(0.040);
    hWeight->GetYaxis()->SetTitle("w(x) = N_{data} / N_{MC,reco,matched}");
    hWeight->GetYaxis()->SetTitleSize(0.045);
    hWeight->GetYaxis()->SetLabelSize(0.040);
    hWeight->GetYaxis()->SetTitleOffset(1.4);
    hWeight->SetMinimum(0);
    hWeight->Draw("E");
    
    TLine lineOne(hWeight->GetXaxis()->GetXmin(), 1.0, hWeight->GetXaxis()->GetXmax(), 1.0);
    lineOne.SetLineStyle(2);
    lineOne.SetLineColor(kRed);
    lineOne.SetLineWidth(2);
    lineOne.Draw();
    
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.035);
    latex.DrawLatex(0.15, 0.85, Form("Mode: %s", suffix));
    latex.DrawLatex(0.15, 0.80, Form("MVA > %.3f", mvaThreshold));
    
    std::string baseName = AggregatedDir(std::string("reweight_") + VarName(vtype) + "_" + suffix + "_mva" + mvaStr);
    c->SaveAs((baseName + ".png").c_str());
    c->SaveAs((baseName + ".pdf").c_str());
    // delete c;
    
    // Save ROOT file
    TFile* fOut = TFile::Open((baseName + ".root").c_str(), "RECREATE");
    if(fOut && !fOut->IsZombie()){
      hWeight->Write("hWeight");
      hMCReco->Write("hMCReco");
      hTarget->Write("hTarget");
      fOut->Close();
      std::cout << "Reweighting function saved: " << baseName << ".root" << std::endl;
    } else {
      std::cerr << "ERROR: Cannot create output file: " << baseName << ".root" << std::endl;
      if(fOut) delete fOut;
    }
    
    // delete hTarget;
    
    return hWeight;
  }
}

void CorrectYieldsWithMVA_Stage1_Independent(int varTypeInt = 0, const char* resultsDir = "results/data_fits", const char* suffix = "incl"){
  if(varTypeInt < 0 || varTypeInt > 3){
    std::cerr << "ERROR: Invalid varTypeInt " << varTypeInt << " (must be 0-3)" << std::endl;
    return;
  }
  
  VarType vtype = static_cast<VarType>(varTypeInt);
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "=== Stage 1: Reweighting Function ===" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Variable: " << VarName(vtype) << std::endl;
  std::cout << "Results directory: " << resultsDir << std::endl;
  std::cout << "Mode: " << suffix << std::endl;
  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  
  gSystem->mkdir(AggregatedDir().c_str(), true);
  
  // Build MC histogram once (independent of MVA)
  TH1D* hMCReco = BuildMCRecoHistogram(vtype, suffix);
  if(!hMCReco){
    std::cerr << "ERROR: Failed to build MC reco histogram" << std::endl;
    return;
  }
  
  // Loop over all MVA thresholds
  const auto& mvaThresholds = GetMVAThresholds();
  int nSuccess = 0;
  
  for(double mva : mvaThresholds){
    TH1D* hWeight = BuildReweightingFunction(resultsDir, vtype, hMCReco, mva, suffix);
    
    if(hWeight){
      nSuccess++;
      // delete hWeight;
    }
  }
  
  // delete hMCReco;
  
  std::cout << "\n========================================" << std::endl;
  if(nSuccess == mvaThresholds.size()){
    std::cout << "✓ Stage 1 completed successfully!" << std::endl;
    std::cout << "Generated " << nSuccess << "/" << mvaThresholds.size() << " reweighting functions" << std::endl;
  } else {
    std::cout << "⚠ Stage 1 completed with warnings" << std::endl;
    std::cout << "Generated " << nSuccess << "/" << mvaThresholds.size() << " reweighting functions" << std::endl;
  }
  std::cout << "========================================" << std::endl;
}
