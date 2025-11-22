// Stage 2 (Efficiency) - Independent macro  
// Build efficiency map: eff(x, MVA) = N_reco(matchGEN==1) / N_gen
// with reweighting applied
//
// Usage:
//   root -l -b -q 'CorrectYieldsWithMVA_Stage2_Independent.cpp(0, "incl", 1)'
//
// Args:
//   varTypeInt: 0=PT, 1=RAPIDITY, 2=CENTRALITY
//   suffix: "incl", "prompt", or "nonprompt"
//   generate: 1=build efficiency, 0=plot only from existing

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
#include "TPad.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TLine.h"
#include "TH1D.h"
#include "TKey.h"

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
  
  void PrintProgress(Long64_t current, Long64_t total, const char* label){
    static std::map<std::string,int> lastPrint;
    if(total <= 0) return;
    int pct = static_cast<int>(100.0 * (current + 1) / total);
    if(pct > 100) pct = 100;
    int bucket = pct / 5;
    int& lastBucket = lastPrint[label];
    if(bucket > lastBucket || pct == 100){
      std::cout << "\r[" << label << "] " << pct << "%";
      std::cout.flush();
      lastBucket = bucket;
      if(pct == 100) std::cout << std::endl;
    }
  }

  std::map<double, TH1D*> BuildEfficiencyMaps(VarType vtype, const std::vector<double>& mvaList,
                                               TTree* mcRecoTree, TTree* mcGenTree, const char* suffix){
    std::cout << "\n=== STAGE 2: Build Efficiency Map ===" << std::endl;
    std::cout << "Variable: " << VarName(vtype) << std::endl;
    std::cout << "Mode: " << suffix << std::endl;
    
    if(mvaList.empty() || !mcRecoTree){ std::cout << "Missing inputs." << std::endl; return {}; }
    
    std::map<double, TH1D*> effMaps;
    gSystem->mkdir(AggregatedDir().c_str(), true);
    
    // Load reweighting histograms from Stage1 (one per MVA)
    std::map<double, TH1D*> weightHists;
    for(double mva : mvaList){
      std::string mvaStr = FormatMVA(mva);
      std::string reweightPath = AggregatedDir(std::string("reweight_") + VarName(vtype) + "_" + suffix + "_mva" + mvaStr + ".root");
      TFile* fWeight = TFile::Open(reweightPath.c_str(), "READ");
      if(!fWeight || fWeight->IsZombie()){
        std::cout << "[Stage2] WARNING: Cannot open reweight file: " << reweightPath << std::endl;
        std::cout << "Using unit weights for MVA " << mva << std::endl;
        if(fWeight) fWeight->Close();
      } else {
        TH1D* hW = (TH1D*)fWeight->Get("hWeight");
        if(hW){
          weightHists[mva] = (TH1D*)hW->Clone(Form("hWeight_mva%.3f", mva));
          weightHists[mva]->SetDirectory(nullptr);
          std::cout << "Loaded reweighting for MVA " << mva << std::endl;
        }
        fWeight->Close();
      }
    }
    
    DataFormat::simpleDStarMCTreeflat evtReco;
    DataFormat::simpleDStarMCTreeflat evtGen;
    evtReco.setTree(mcRecoTree);
    evtGen.setGENTree(mcGenTree);
    
    const Long64_t nReco = mcRecoTree->GetEntries();
    const Long64_t nGen = mcGenTree->GetEntries();
    
    // Build gen histograms per MVA (with reweighting applied)
    std::map<double, TH1D*> genHists;
    for(double mva : mvaList){
      TH1D* hG = (TH1D*)CreateHistogram(Form("hGen_mva_%0.3f", mva), "MC Gen", vtype);
      hG->Sumw2();
      genHists[mva] = hG;
    }
    
    for(Long64_t ie=0; ie<nGen; ++ie){
      mcGenTree->GetEntry(ie);
      
      // Mode filter
      int flav = evtGen.gen_D0ancestorFlavor_;
      if(std::string(suffix)=="prompt" && flav==5) continue;
      if(std::string(suffix)=="nonprompt" && flav!=5) continue;
      // cout << "flav: " << flav << endl;
      
      double val = 0;
      if(vtype == VarType::PT){ val = evtGen.gen_pT; }
      else if(vtype == VarType::RAPIDITY){ val = fabs(evtGen.gen_y); }
      else { val = evtGen.centrality; }
      
      for(auto &kv : genHists){
        double mva = kv.first;
        TH1D* hGen = kv.second;
        
        TH1D* hWeight = nullptr;
        auto itW = weightHists.find(mva);
        if(itW != weightHists.end()) hWeight = itW->second;
        double w = hWeight ? hWeight->GetBinContent(hWeight->FindBin(val)) : 1.0;
        
        hGen->Fill(val, w);
      }
      PrintProgress(ie, nGen, "Stage2 Gen");
    }
    
    // Build reco histograms per MVA
    std::map<double, TH1D*> recoHists;
    for(double mva : mvaList){
      TH1D* hR = (TH1D*)CreateHistogram(Form("hReco_mva_%0.3f", mva), "MC Reco", vtype);
      hR->Sumw2();
      recoHists[mva] = hR;
    }
    
    for(Long64_t ie=0; ie<nReco; ++ie){
      mcRecoTree->GetEntry(ie);
      if(!evtReco.matchGEN) continue;
      
      // Mode filter  
      int flav = evtReco.matchGen_D1ancestorFlavor_;
      if(std::string(suffix)=="prompt" && flav==5) continue;
      if(std::string(suffix)=="nonprompt" && flav!=5) continue;
      
      // DCA filter
      const double dcaCut = 0.01;
      if(std::string(suffix)=="prompt" && evtReco.dca3D >= dcaCut) continue;
      if(std::string(suffix)=="nonprompt" && evtReco.dca3D < dcaCut) continue;
      
      double val = 0;
      if(vtype == VarType::PT){ val = evtReco.matchGen_DStarpT; }
      else if(vtype == VarType::RAPIDITY){ val = fabs(evtReco.matchGen_DStary); }
      else { val = evtReco.centrality; }
      
      for(auto &kv : recoHists){
        double mva = kv.first;
        if(evtReco.mva < mva) continue;
        
        TH1D* hReco = kv.second;
        int bin = hReco->FindBin(val);
        if(bin<1 || bin>hReco->GetNbinsX()) continue;
        
        TH1D* hWeight = nullptr;
        auto itW = weightHists.find(mva);
        if(itW != weightHists.end()) hWeight = itW->second;
        double w = hWeight ? hWeight->GetBinContent(hWeight->FindBin(val)) : 1.0;
        
        hReco->Fill(val, w);
      }
      PrintProgress(ie, nReco, "Stage2 Reco");
    }
    
    // Calculate efficiency maps
    for(double mva : mvaList){
      TH1D* hGen = genHists[mva];
      TH1D* hReco = recoHists[mva];
      if(!hGen || !hReco) continue;
      
      TH1D* hEff = (TH1D*)hReco->Clone(Form("eff_mva_%0.3f", mva));
      hEff->SetTitle("Efficiency");
      hEff->Divide(hReco, hGen, 1.0, 1.0, "B");
      hEff->GetXaxis()->SetTitle(VarLabel(vtype));
      hEff->GetYaxis()->SetTitle("Efficiency");
      hEff->SetMinimum(0);
      hEff->SetMaximum(1.2);
      
      effMaps[mva] = (TH1D*)hEff->Clone(Form("efficiency_mva%.3f", mva));
      effMaps[mva]->SetDirectory(nullptr);
      delete hEff;
    }
    
    for(auto &kv : recoHists) delete kv.second;
    for(auto &kv : genHists) delete kv.second;
    
    // Save to file
    std::string effAll = AggregatedDir(std::string("efficiency_") + VarName(vtype) + "_" + suffix + ".root");
    TFile* fOut = TFile::Open(effAll.c_str(), "RECREATE");
    for(auto &kv : effMaps){ 
      if(kv.second) kv.second->Write(Form("efficiency_mva%.3f", kv.first)); 
    }
    fOut->Close();
    delete fOut;
    
    std::cout << "Efficiency maps saved: " << effAll << std::endl;
    
    for(auto &kv : weightHists) if(kv.second) delete kv.second;
    
    return effMaps;
  }
  
  std::map<double, TH1D*> LoadEfficiencyMapsFromDisk(VarType vtype, const char* suffix){
    std::map<double, TH1D*> maps;
    std::string effFile = AggregatedDir(std::string("efficiency_") + VarName(vtype) + "_" + suffix + ".root");
    TFile* f = TFile::Open(effFile.c_str(), "READ");
    if(!f || f->IsZombie()){
      if(f) f->Close();
      delete f;
      std::cout << "WARNING: Cannot open efficiency file: " << effFile << std::endl;
      return maps;
    }
    TIter next(f->GetListOfKeys());
    while(auto key = (TKey*)next()){
      TObject* obj = key->ReadObj();
      if(!obj) continue;
      if(auto h = dynamic_cast<TH1D*>(obj)){
        std::string name = h->GetName();
        double mva = 0.0;
        if(sscanf(name.c_str(), "efficiency_mva%lf", &mva) == 1){
          TH1D* clone = (TH1D*)h->Clone(Form("%s_clone_plot", name.c_str()));
          clone->SetDirectory(nullptr);
          maps[mva] = clone;
        }
      }
    }
    f->Close();
    delete f;
    return maps;
  }
  
  void PlotEfficiencyOverview(VarType vtype, const std::map<double, TH1D*>& effMaps, const char* suffix){
    if(effMaps.empty()) return;
    
    // Plot all MVA thresholds (5 MVAs: 0.9, 0.95, 0.99, 0.995, 0.999)
    const auto& mvaThresholds = GetMVAThresholds();
    std::vector<double> mvaPlotList(mvaThresholds.begin(), mvaThresholds.end());
    
    gStyle->SetOptStat(0);
    gSystem->mkdir(AggregatedDir().c_str(), true);
    
    // Create canvas with 3x2 layout (3 in first row, 2 in second row)
    TCanvas* cAll = new TCanvas("cEfficiencyAll", "Efficiency overview", 1800, 1200);
    cAll->Divide(3, 2, 0.001, 0.001);
    
    double baseMVA = mvaPlotList.front();
    TH1D* hBase = nullptr;
    auto itBase = effMaps.find(baseMVA);
    if(itBase != effMaps.end()) hBase = itBase->second;
    
    std::vector<int> colors = {kBlue+1, kGreen+2, kRed+1, kMagenta+1, kOrange+1};
    int padIndex=1;
    
    for(double mva : mvaPlotList){
      auto it = effMaps.find(mva);
      if(it==effMaps.end()){ padIndex++; continue; }
      
      cAll->cd(padIndex);
      TPad* parent = (TPad*)gPad;
      parent->cd();
      
      TPad* padTop = new TPad(Form("padEffTop_%d", padIndex),"",0,0.3,1,1);
      padTop->SetLeftMargin(0.15);
      padTop->SetRightMargin(0.05);
      padTop->SetTopMargin(0.08);
      padTop->SetBottomMargin(0.01);
      padTop->Draw();
      padTop->cd();
      
      TH1D* hEffDraw = (TH1D*)it->second->Clone(Form("hEffDraw_%.3f", mva));
      int color = colors[(padIndex-1)%colors.size()];
      hEffDraw->SetMarkerStyle(20);
      hEffDraw->SetMarkerSize(1.0);
      hEffDraw->SetMarkerColor(color);
      hEffDraw->SetLineColor(color);
      hEffDraw->SetLineWidth(2);
      hEffDraw->SetMinimum(0);
      hEffDraw->SetMaximum(std::max(1.0, hEffDraw->GetMaximum()*1.2));
      hEffDraw->GetXaxis()->SetLabelSize(0);
      hEffDraw->Draw("E");
      
      TLatex tx;
      tx.SetNDC();
      tx.SetTextFont(62);
      tx.SetTextSize(0.07);
      tx.DrawLatex(0.25, 0.92, Form("MVA = %.3f", mva));
      
      TLegend* leg = new TLegend(0.55,0.70,0.92,0.88);
      leg->SetBorderSize(0);
      leg->SetFillStyle(0);
      leg->SetTextSize(0.05);
      leg->AddEntry(hEffDraw, "Efficiency", "pl");
      leg->Draw();
      
      parent->cd();
      TPad* padBot = new TPad(Form("padEffBot_%d", padIndex),"",0,0,1,0.3);
      padBot->SetLeftMargin(0.15);
      padBot->SetRightMargin(0.05);
      padBot->SetTopMargin(0.02);
      padBot->SetBottomMargin(0.28);
      padBot->Draw();
      padBot->cd();
      
      TH1D* hRatio = (TH1D*)it->second->Clone(Form("hEffRatio_%.3f", mva));
      hRatio->SetMarkerStyle(20);
      hRatio->SetMarkerSize(0.9);
      hRatio->SetLineWidth(2);
      hRatio->SetLineColor(kBlack);
      hRatio->SetMarkerColor(kBlack);
      hRatio->GetYaxis()->SetTitle("Eff / Eff_{0.99}");
      hRatio->GetXaxis()->SetTitle(VarLabel(vtype));
      hRatio->GetYaxis()->SetTitleSize(0.09);
      hRatio->GetXaxis()->SetTitleSize(0.09);
      hRatio->GetYaxis()->SetLabelSize(0.08);
      hRatio->GetXaxis()->SetLabelSize(0.08);
      hRatio->GetYaxis()->SetTitleOffset(0.6);
      hRatio->GetXaxis()->SetTitleOffset(1.0);
      hRatio->SetMinimum(0.5);
      hRatio->SetMaximum(1.5);
      
      if(hBase && mva != baseMVA){
        hRatio->Divide(it->second, hBase, 1.0, 1.0, "B");
      } else {
        for(int ib=1; ib<=hRatio->GetNbinsX(); ++ib){
          hRatio->SetBinContent(ib,1.0);
          hRatio->SetBinError(ib,0.0);
        }
      }
      hRatio->Draw("E");
      
      TLine refLine(hRatio->GetXaxis()->GetXmin(),1.0,hRatio->GetXaxis()->GetXmax(),1.0);
      refLine.SetLineStyle(2);
      refLine.SetLineColor(kRed+1);
      refLine.Draw();
      
      padIndex++;
    }
    
    std::string baseOut = AggregatedDir(std::string("efficiency_") + VarName(vtype) + "_" + suffix + "_overview");
    cAll->SaveAs((baseOut + ".png").c_str());
    cAll->SaveAs((baseOut + ".pdf").c_str());
    delete cAll;
  }
}

void CorrectYieldsWithMVA_Stage2_Independent(int varTypeInt = 0, const char* suffix = "incl", int generate = 1){
  VarType vtype = static_cast<VarType>(varTypeInt);
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "=== Stage 2: Efficiency Maps ===" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Variable: " << VarName(vtype) << std::endl;
  std::cout << "Mode: " << suffix << std::endl;
  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  
  if(!generate){
    auto storedMaps = LoadEfficiencyMapsFromDisk(vtype, suffix);
    if(!storedMaps.empty()){
      PlotEfficiencyOverview(vtype, storedMaps, suffix);
      for(auto &kv : storedMaps) if(kv.second) delete kv.second;
      std::cout << "\n✓ Stage 2 plotting complete!" << std::endl;
    } else {
      std::cout << "\n✗ No stored efficiency maps found!" << std::endl;
    }
    return;
  }
  
  // Load MC dataset
  SimpleDatasetManager mgr("/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json");
  auto mcInfo = mgr.GetDataset("PbPb_MC_PromptRich_Nov9");
  if(mcInfo.file.empty()){ std::cerr << "ERROR: dataset not found" << std::endl; return; }
  
  TFile* mcFile = TFile::Open(mcInfo.file.c_str(), "READ");
  if(!mcFile || mcFile->IsZombie()){ std::cerr << "ERROR: Cannot open: " << mcInfo.file << std::endl; return; }
  
  TTree* recoTree = (TTree*)mcFile->Get(mcInfo.dataset_name.c_str());
  TTree* genTree = (TTree*)mcFile->Get("skimGENTreeFlat");
  if(!recoTree){ std::cerr << "ERROR: Missing reco tree" << std::endl; mcFile->Close(); return; }
  
  // Get MVA thresholds
  const auto& mvaThresholdsArray = GetMVAThresholds();
  std::vector<double> mvaList(mvaThresholdsArray.begin(), mvaThresholdsArray.end());
  
  auto effMaps = BuildEfficiencyMaps(vtype, mvaList, recoTree, genTree, suffix);
  
  if(!effMaps.empty()){
    PlotEfficiencyOverview(vtype, effMaps, suffix);
    std::cout << "\n✓ Stage 2 complete!" << std::endl;
  } else {
    std::cout << "\n✗ Stage 2 failed!" << std::endl;
  }
  
  for(auto &kv : effMaps) if(kv.second) delete kv.second;
  mcFile->Close();
}
