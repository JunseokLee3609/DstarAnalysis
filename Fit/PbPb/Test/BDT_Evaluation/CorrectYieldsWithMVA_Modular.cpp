// Modular yield correction workflow with automatic dependency resolution
// 
// Three independent stages with automatic input generation:
// Stage 1: Build reweighting function from MC + plot
// Stage 2: Build efficiency map from reweighting + plot  
// Stage 3: Apply corrections to data yields + plot corrected/raw yields
//
// Usage:
//   root -l -b -q 'CorrectYieldsWithMVA_Modular.cpp(0, "results/data_fits", 1, 1, 1, 1)'
//
// Args:
//   varTypeInt: 0=PT, 1=RAPIDITY, 2=CENTRALITY
//   resultsDir: directory containing data fitresult_*.root  
//   stage1: 1=build reweighting, 0=skip
//   stage2: 1=build efficiency map, 0=skip
//   stage3: 1=apply corrections & plot, 0=skip
//   doNormPlots: 1=also produce normalized plots

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <algorithm>
#include <regex>
#include <string>
#include <cmath>
#include <numeric>

#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLine.h"
#include "TPad.h"

#include "RooFitResult.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooArgSet.h"

#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"

namespace {
  enum class VarType { PT=0, RAPIDITY=1, CENTRALITY=2, COS=3 };
  enum class Stage { REWEIGHT=1, EFFICIENCY=2, CORRECT=3 };
  
  const char* VarName(VarType v){
    switch(v){ 
      case VarType::PT: return "pT"; 
      case VarType::RAPIDITY: return "y"; 
      case VarType::CENTRALITY: return "centrality"; 
      case VarType::COS: return "cos"; 
    }
    return "pT";
  }
  
  const char* VarLabel(VarType v){
    switch(v){ 
      case VarType::PT: return "p_{T} [GeV/c]"; 
      case VarType::RAPIDITY: return "|y|"; 
      case VarType::CENTRALITY: return "Centrality [%]"; 
      case VarType::COS: return "cos(#theta)"; 
    }
    return "p_{T} [GeV/c]";
  }
  
  std::string replaceAll(std::string s, char from, char to){ 
    for(char& c: s){ if(c==from) c=to; } 
    return s; 
  }

  bool getParamValErr(RooFitResult* res, const char* name, double& val, double& err){
    if(!res) return false;
    RooArgList fl = res->floatParsFinal();
    if (auto* rr = dynamic_cast<RooRealVar*>(fl.find(name))) { val = rr->getVal(); err = rr->getError(); return true; }
    RooArgList cp = res->constPars();
    if (auto* rc = dynamic_cast<RooRealVar*>(cp.find(name))) { val = rc->getVal(); err = 0.0; return true; }
    return false;
  }

  struct Bin { 
    double min, max; 
    double center() const { return 0.5*(min+max); } 
  };

  // ============================================
  // Stage 1: Build Reweighting Function
  // ============================================

  class ReweightingFunctionFromMC {
  private:
    TH1D* fWeightHist;
    VarType fVarType;
    
  public:
    ReweightingFunctionFromMC() : fWeightHist(nullptr), fVarType(VarType::PT) {}
    ~ReweightingFunctionFromMC() { if(fWeightHist) delete fWeightHist; }
    
    double GetWeight(double varValue) const {
      if(!fWeightHist) return 1.0;
      int bin = fWeightHist->FindBin(varValue);
      return fWeightHist->GetBinContent(bin);
    }
    
    double GetWeightError(double varValue) const {
      if(!fWeightHist) return 0.0;
      int bin = fWeightHist->FindBin(varValue);
      return fWeightHist->GetBinError(bin);
    }
    
    TH1D* GetHistogram() const { return fWeightHist; }
    void SetHistogram(TH1D* h) { fWeightHist = h; }
    void SetVarType(VarType v) { fVarType = v; }
  };
  
  TH1D* BuildGenPTWeightHist(const char* resultsDir, TTree* mcGenTree, VarType vtype,
                             double baseMVA = 0.990, double wmin = 0.2, double wmax = 5.0){
    std::cout << "\n=== STAGE 1: Build Reweighting Function ===" << std::endl;
    std::cout << "Variable: " << VarName(vtype) << std::endl;
    std::cout << "Base MVA: " << baseMVA << std::endl;
    std::cout << "MC gen distribution (matchGEN==1 from skimGENTreeFlat)" << std::endl;
    
    const double ptEdges[] = {5,7,10,20,30,40,50,60,70,80,90,100,110,120};
    const int nPt = sizeof(ptEdges)/sizeof(double) - 1;
    const double yEdges[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5};
    const int nY = sizeof(yEdges)/sizeof(double) - 1;
    const double centEdges[] = {0, 10, 30, 50, 90};
    const int nCent = sizeof(centEdges)/sizeof(double) - 1;

    TH1D *hMCGen=nullptr, *hTarget=nullptr;
    
    if(vtype == VarType::PT){
      hMCGen = new TH1D("hMCGenPt", "D* MC gen pT (fiducial)", nPt, ptEdges);
      hTarget = new TH1D("hTargetPt", "Data fit yields at baseline MVA", nPt, ptEdges);
    } else if(vtype == VarType::RAPIDITY){
      hMCGen = new TH1D("hMCGenY", "D* MC gen |y| (fiducial)", nY, yEdges);
      hTarget = new TH1D("hTargetY", "Data fit yields at baseline MVA", nY, yEdges);
    } else if(vtype == VarType::CENTRALITY){
      hMCGen = new TH1D("hMCGenCent", "D* MC gen centrality (fiducial)", nCent, centEdges);
      hTarget = new TH1D("hTargetCent", "Data fit yields at baseline MVA", nCent, centEdges);
    }
    
    if(!hMCGen || !hTarget) return nullptr;
    
    hMCGen->Sumw2();
    hTarget->Sumw2();

    // Fill MC gen histogram from gen tree (skimGENTreeFlat)
    std::cout << "Filling gen D* distribution from skimGENTreeFlat..." << std::endl;
    if(!mcGenTree){
      std::cerr << "ERROR: MC gen tree not provided!" << std::endl;
      return nullptr;
    }
    
    if(vtype == VarType::PT){
      mcGenTree->Draw("gen_pT>>hMCGenPt", "", "goff");
    } else if(vtype == VarType::RAPIDITY){
      mcGenTree->Draw("abs(gen_y)>>hMCGenY", "", "goff");
    } else if(vtype == VarType::CENTRALITY){
      mcGenTree->Draw("centrality>>hMCGenCent", "", "goff");
    }

    std::cout << "Gen D* entries (fiducial): " << hMCGen->Integral() << std::endl;

    // Fill target histogram from data fit results at baseline MVA
    {
      std::string varStr = VarName(vtype);
      char mvaBuf[64];
      snprintf(mvaBuf, sizeof(mvaBuf), "mva%.3f", baseMVA);
      std::string mvaTag = mvaBuf;
      for(char &c : mvaTag) if(c=='.') c='p';
      std::string prefix = std::string("fitresult_") + varStr + "_";

      TSystemDirectory dir(resultsDir, resultsDir);
      TList* fl = dir.GetListOfFiles();
      std::cout << "Reading fit results from: " << resultsDir << " for MVA " << baseMVA << std::endl;
      
      if (fl) {
        TIter next(fl);
        TSystemFile* f;
        int count = 0;
        while((f=(TSystemFile*)next())) {
          std::string fname = f->GetName();
          if (fname.find(prefix) == 0 && fname.find(mvaTag) != std::string::npos && 
              fname.size() > 5 && fname.substr(fname.size()-5) == ".root") {
            count++;
            size_t start = prefix.size();
            size_t pos_mva = fname.find("_mva");
            if (pos_mva == std::string::npos || pos_mva <= start) continue;
            std::string range = fname.substr(start, pos_mva - start);
            size_t us = range.find('_');
            if (us == std::string::npos) continue;
            std::string smin = range.substr(0, us);
            std::string smax = range.substr(us+1);
            for(char &c : smin) if(c=='p') c='.';
            for(char &c : smax) if(c=='p') c='.';
            double vmin = atof(smin.c_str());
            double vmax = atof(smax.c_str());
            double x = 0.5*(vmin+vmax);

            std::string fpath = std::string(resultsDir) + "/" + fname;
            TFile* tf = TFile::Open(fpath.c_str(), "READ");
            if (!tf || tf->IsZombie()) { if(tf) tf->Close(); continue; }
            RooFitResult* r = (RooFitResult*)tf->Get("fitResult");
            double y=0, ey=0;
            if (r) {
              getParamValErr(r, "nsig", y, ey);
              std::cout << "  Bin [" << vmin << ", " << vmax << "]: yield = " << y << " +/- " << ey << std::endl;
            }
            tf->Close();
            int bin = hTarget->FindBin(x);
            hTarget->SetBinContent(bin, y);
            hTarget->SetBinError(bin, ey);
          }
        }
        std::cout << "Found " << count << " fit results at MVA " << baseMVA << std::endl;
      }
      std::cout << "Target total yield: " << hTarget->Integral() << std::endl;
    }
    
    // Build weight histogram: w(x) = hTarget(x) / hMCGen(x)
    TH1D* hWeight = (TH1D*)hTarget->Clone("hWeight");
    hWeight->SetTitle("Reweighting factor: data_yield / MC_gen");
    for(int i=1; i<=hWeight->GetNbinsX(); ++i){
      if(hMCGen->GetBinContent(i) > 0){
        double ratio = hTarget->GetBinContent(i) / hMCGen->GetBinContent(i);
        double err = 0;
        if(hMCGen->GetBinContent(i) > 0 && hTarget->GetBinContent(i) > 0){
          err = ratio * sqrt(pow(hTarget->GetBinError(i)/hTarget->GetBinContent(i), 2) + 
                             pow(hMCGen->GetBinError(i)/hMCGen->GetBinContent(i), 2));
        }
        hWeight->SetBinContent(i, ratio);
        hWeight->SetBinError(i, err);
      } else {
        hWeight->SetBinContent(i, 1.0);
        hWeight->SetBinError(i, 0);
      }
    }
    
    // Clip weights to [wmin, wmax]
    for(int i=1; i<=hWeight->GetNbinsX(); ++i){
      double w = hWeight->GetBinContent(i);
      if(w < wmin || w > wmax){
        std::cout << "  Bin " << i << ": weight " << w << " clipped to [" << wmin << ", " << wmax << "]" << std::endl;
        if(w < wmin) hWeight->SetBinContent(i, wmin);
        if(w > wmax) hWeight->SetBinContent(i, wmax);
        hWeight->SetBinError(i, 0);
      }
    }
    
    // Plot reweighting factor
    gSystem->mkdir("results/aggregated", true);
    TCanvas* cWeight = new TCanvas("cWeight", "Reweighting Factor", 900, 700);
    cWeight->SetLeftMargin(0.12);
    cWeight->SetBottomMargin(0.12);
    cWeight->SetRightMargin(0.05);
    hWeight->SetMarkerStyle(20);
    hWeight->SetMarkerSize(1.2);
    hWeight->SetMarkerColor(kBlack);
    hWeight->SetLineWidth(2);
    hWeight->SetLineColor(kBlack);
    hWeight->GetYaxis()->SetTitle("w(x) = N_{data} / N_{MC,gen}");
    hWeight->GetXaxis()->SetTitle(VarLabel(vtype));
    hWeight->GetYaxis()->SetTitleOffset(1.3);
    hWeight->SetMinimum(0);
    hWeight->Draw("E");
    TLine lineOne(hWeight->GetXaxis()->GetXmin(), 1.0, hWeight->GetXaxis()->GetXmax(), 1.0);
    lineOne.SetLineStyle(2);
    lineOne.SetLineColor(kRed);
    lineOne.SetLineWidth(2);
    lineOne.Draw();
    
    std::string baseName = std::string("results/aggregated/reweight_") + VarName(vtype);
    cWeight->SaveAs((baseName + ".png").c_str());
    cWeight->SaveAs((baseName + ".pdf").c_str());
    
    // Save to ROOT file
    TFile* fOut = TFile::Open((baseName + ".root").c_str(), "RECREATE");
    hWeight->Write();
    hMCGen->Write();
    hTarget->Write();
    fOut->Close();
    delete fOut;
    
    std::cout << "Reweighting function saved: " << baseName << ".root" << std::endl;
    delete cWeight;
    
    return hWeight;
  }

  // ============================================
  // Stage 2: Build Efficiency Map
  // ============================================

  class EfficiencyMap {
  private:
    std::map<double, TH1D*> fEffHistos;
    VarType fVarType;
    
  public:
    EfficiencyMap() : fVarType(VarType::PT) {}
    ~EfficiencyMap() {
      for(auto &kv : fEffHistos) if(kv.second) delete kv.second;
      fEffHistos.clear();
    }
    
    double GetEfficiency(double mva, double varValue) const {
      auto it = fEffHistos.find(mva);
      if(it == fEffHistos.end() || !it->second) return 1.0;
      int bin = it->second->FindBin(varValue);
      return it->second->GetBinContent(bin);
    }
    
    double GetEfficiencyError(double mva, double varValue) const {
      auto it = fEffHistos.find(mva);
      if(it == fEffHistos.end() || !it->second) return 0.0;
      int bin = it->second->FindBin(varValue);
      return it->second->GetBinError(bin);
    }
    
    void AddHistogram(double mva, TH1D* h) { fEffHistos[mva] = h; }
    TH1D* GetHistogram(double mva) const {
      auto it = fEffHistos.find(mva);
      return (it != fEffHistos.end()) ? it->second : nullptr;
    }
    std::map<double, TH1D*>& GetMap() { return fEffHistos; }
  };
  
  std::map<double, TH1D*> BuildEfficiencyMaps(const char* resultsDir, VarType vtype, 
                                              const std::vector<double>& mvaList, TTree* mcGenTree, TTree* mcRecoTree){
    std::cout << "\n=== STAGE 2: Build Efficiency Map ===" << std::endl;
    std::cout << "Variable: " << VarName(vtype) << std::endl;
    std::cout << "Efficiency = N_reco(matchGEN==1) / N_gen for each kinematic bin" << std::endl;
    
    std::map<double, TH1D*> effMaps;
    
    if(mvaList.empty()){
      std::cout << "Warning: Empty MVA list. Skipping efficiency map calculation." << std::endl;
      return effMaps;
    }
    
    if(!mcGenTree || !mcRecoTree){
      std::cout << "Warning: Missing MC trees. Skipping efficiency map calculation." << std::endl;
      return effMaps;
    }

    // Define binning per variable
    const double ptEdges[] = {5,7,10,20,30,40,50,60,70,80,90,100,110,120};
    const int nPt = sizeof(ptEdges)/sizeof(double) - 1;
    const double yEdges[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5};
    const int nY = sizeof(yEdges)/sizeof(double) - 1;
    const double centEdges[] = {0, 10, 30, 50, 90};
    const int nCent = sizeof(centEdges)/sizeof(double) - 1;

    auto makeHist = [&](const char* name){
      if (vtype == VarType::PT) return (TH1D*)new TH1D(name, name, nPt, ptEdges);
      if (vtype == VarType::RAPIDITY) return (TH1D*)new TH1D(name, name, nY, yEdges);
      return (TH1D*)new TH1D(name, name, nCent, centEdges);
    };

    // Get gen distribution from skimGENTreeFlat
    TH1D* hGenAll = makeHist("hGenAll");
    hGenAll->Sumw2();
    if(vtype == VarType::PT){
      mcGenTree->Draw("gen_pT>>hGenAll", "", "goff");
    } else if(vtype == VarType::RAPIDITY){
      mcGenTree->Draw("abs(gen_y)>>hGenAll", "", "goff");
    } else if(vtype == VarType::CENTRALITY){
      mcGenTree->Draw("centrality>>hGenAll", "", "goff");
    }
    std::cout << "Gen total: " << hGenAll->Integral() << " entries" << std::endl;

    // Build efficiency per MVA: eff = N_reco(matchGEN==1) / N_gen
    std::cout << "Building efficiency for " << mvaList.size() << " MVA thresholds..." << std::endl;
    for(size_t imva = 0; imva < mvaList.size(); ++imva){
      double mva = mvaList[imva];
      TH1D* hEff = makeHist(Form("eff_mva_%0.3f", mva));
      hEff->Sumw2();
      
      std::string varStr = VarName(vtype);
      char mvaBuf[64]; 
      snprintf(mvaBuf, sizeof(mvaBuf), "mva%.3f", mva);
      std::string mvaTag = mvaBuf; 
      for(char &c : mvaTag) if(c=='.') c='p';
      std::string prefix = std::string("fitresult_") + varStr + "_";
      
      std::map<int, double> recoCount;
      
      // Count reco entries (matchGEN==1) per bin for this MVA
      std::cout << "  MVA " << mva << ": Scanning fit results..." << std::endl;
      TSystemDirectory dir(resultsDir, resultsDir);
      TList* fl = dir.GetListOfFiles();
      if (fl){ 
        TIter next(fl); 
        TSystemFile* f; 
        while((f=(TSystemFile*)next())){
          std::string fname = f->GetName();
          if (fname.find(prefix)==0 && fname.find(mvaTag)!=std::string::npos && 
              fname.size()>5 && fname.substr(fname.size()-5)==".root"){
            size_t start = prefix.size(); 
            size_t pos_mva = fname.find("_mva"); 
            if(pos_mva==std::string::npos||pos_mva<=start) continue;
            std::string range = fname.substr(start, pos_mva-start); 
            size_t us = range.find('_'); 
            if(us==std::string::npos) continue;
            std::string smin=range.substr(0,us), smax=range.substr(us+1); 
            for(char &c:smin) if(c=='p') c='.'; 
            for(char &c:smax) if(c=='p') c='.';
            double vmin=atof(smin.c_str()), vmax=atof(smax.c_str()); 
            double x=0.5*(vmin+vmax);
            TH1D* hTmp = makeHist("hTmp"); 
            int bin = hTmp->FindBin(x); 
            delete hTmp;
            
            std::string fpath = std::string(resultsDir)+"/"+fname; 
            TFile* tf=TFile::Open(fpath.c_str(),"READ"); 
            if(!tf||tf->IsZombie()){ if(tf) tf->Close(); continue; }
            RooFitResult* r=(RooFitResult*)tf->Get("fitResult"); 
            double nsig=0, ey=0; 
            if(r){ 
              getParamValErr(r, "nsig", nsig, ey);
              recoCount[bin] = nsig;
            } 
            tf->Close();
          }
        }
      }
      
      // Calculate efficiency as reco_yield / gen_count
      for(int ibin=1; ibin<=hGenAll->GetNbinsX(); ++ibin){
        double ngen = hGenAll->GetBinContent(ibin);
        double nreco = recoCount[ibin];
        if(ngen > 0){
          double eff = nreco / ngen;
          hEff->SetBinContent(ibin, eff);
          double err = 0;
          if(eff > 0 && eff < 1){
            err = sqrt(eff*(1-eff)/ngen);
          }
          hEff->SetBinError(ibin, err);
        } else {
          hEff->SetBinContent(ibin, 0);
          hEff->SetBinError(ibin, 0);
        }
      }
      
      // Plot efficiency
      gSystem->mkdir("results/aggregated", true);
      TCanvas* c = new TCanvas(Form("cEff_%0.3f", mva), "Efficiency vs var", 900, 700);
      c->SetLeftMargin(0.12); 
      c->SetBottomMargin(0.12);
      c->SetRightMargin(0.05);
      hEff->GetXaxis()->SetTitle(VarLabel(vtype)); 
      hEff->GetYaxis()->SetTitle("Efficiency (N_{reco,matched} / N_{gen})");
      hEff->SetMinimum(0.0); 
      hEff->SetMaximum(1.2); 
      hEff->SetLineWidth(2); 
      hEff->SetMarkerStyle(20);
      hEff->SetMarkerColor(kBlue);
      hEff->SetLineColor(kBlue);
      hEff->Draw("E");
      std::string base = std::string("results/aggregated/efficiency_") + VarName(vtype) + Form("_mva%0.3f", mva);
      for(char &cch : base) if(cch=='.') cch='p';
      c->SaveAs((base+".png").c_str()); 
      c->SaveAs((base+".pdf").c_str());
      delete c;
      
      effMaps[mva] = (TH1D*)hEff->Clone();
    }

    delete hGenAll;

    // Save all to ROOT file
    gSystem->mkdir("results/aggregated", true);
    std::string effAll = std::string("results/aggregated/efficiency_") + VarName(vtype) + ".root";
    TFile* fOut = TFile::Open(effAll.c_str(), "RECREATE");
    for(auto &kv : effMaps){ 
      if(kv.second) kv.second->Write(); 
    }
    fOut->Close(); 
    delete fOut;
    
    std::cout << "Efficiency maps saved: " << effAll << std::endl;
    return effMaps;
  }

  // ============================================
  // Stage 3: Apply Corrections & Plot
  // ============================================
  
  void ApplyCorrectionsAndPlot(const char* resultsDir, VarType vtype, 
                               TH1D* hWeight, int doNormPlots, const std::vector<double>& mvaList){
    std::cout << "\n=== STAGE 3: Apply Corrections & Plot ===" << std::endl;
    std::cout << "Variable: " << VarName(vtype) << std::endl;
    
    if(!hWeight){
      std::cerr << "ERROR: No reweighting histogram provided!" << std::endl;
      return;
    }
    
    if(mvaList.empty()){
      std::cout << "Warning: Empty MVA list. Generating placeholder plots." << std::endl;
      gSystem->mkdir("results/aggregated", true);
      
      // Create empty canvases
      TCanvas* cRaw = new TCanvas("cRawCorr", "Raw vs Corrected (empty)", 1000, 700);
      cRaw->SetLeftMargin(0.12); cRaw->SetBottomMargin(0.12);
      TH1D* hFrame = new TH1D("hFrame","",100, 0, 100);
      hFrame->GetXaxis()->SetTitle(VarLabel(vtype));
      hFrame->GetYaxis()->SetTitle("Yield (no data)");
      hFrame->Draw("AXIS");
      TLatex tx; tx.SetNDC(); tx.SetTextSize(0.04);
      tx.DrawLatex(0.3, 0.5, "No MVA thresholds detected");
      
      std::string baseOut = std::string("results/aggregated/corrected_yield_vs_") + VarName(vtype) + "_raw";
      cRaw->SaveAs((baseOut+".png").c_str());
      cRaw->SaveAs((baseOut+".pdf").c_str());
      delete cRaw;
      
      std::cout << "Corrections applied and plots generated (empty)." << std::endl;
      return;
    }
    
    // mvaList is already provided as parameter, use it directly
    // Collect raw and corrected yields per MVA
    std::vector<TGraphErrors*> grRaw, grCorr, grNorm;
    std::vector<int> colors = {kRed, kBlue, kGreen+2, kMagenta, kCyan+2, kOrange+1, kViolet, kTeal, kPink+2, kSpring};
    double xMin=1e9, xMax=-1e9;

    for(size_t im=0; im<mvaList.size(); ++im){ double mva = mvaList[im]; int color = colors[im % colors.size()];
      std::string varStr = VarName(vtype);
      char mvaBuf[64]; snprintf(mvaBuf,sizeof(mvaBuf),"mva%.3f", mva); std::string mvaTag=mvaBuf; for(char &c:mvaTag) if(c=='.') c='p';
      std::string prefix = std::string("fitresult_") + varStr + "_";
      std::vector<double> xs, exs, yraw, eyraw, ycor, eycor;
      TSystemDirectory dir(resultsDir, resultsDir); TList* fl = dir.GetListOfFiles();
      if(fl){ TIter next(fl); TSystemFile* f; while((f=(TSystemFile*)next())){
        std::string fname=f->GetName(); if(fname.find(prefix)==0 && fname.find(mvaTag)!=std::string::npos && fname.size()>5 && fname.substr(fname.size()-5)==".root"){
          size_t start=prefix.size(); size_t pos_mva=fname.find("_mva"); if(pos_mva==std::string::npos||pos_mva<=start) continue;
          std::string range=fname.substr(start,pos_mva-start); size_t us=range.find('_'); if(us==std::string::npos) continue;
          std::string smin=range.substr(0,us), smax=range.substr(us+1); for(char &c:smin) if(c=='p') c='.'; for(char &c:smax) if(c=='p') c='.';
          double vmin=atof(smin.c_str()), vmax=atof(smax.c_str()); double x=0.5*(vmin+vmax); double ex=0.5*(vmax-vmin);
          std::string fpath=std::string(resultsDir)+"/"+fname; TFile* tf=TFile::Open(fpath.c_str(),"READ"); if(!tf||tf->IsZombie()){ if(tf) tf->Close(); continue; }
          RooFitResult* r=(RooFitResult*)tf->Get("fitResult"); double y=0,ey=0; if(r){ getParamValErr(r, "nsig", y, ey);} tf->Close();
          double w = hWeight->GetBinContent(hWeight->FindBin(x));
          xs.push_back(x); exs.push_back(ex); yraw.push_back(y); eyraw.push_back(ey); ycor.push_back(y*w); eycor.push_back(ey*w);
          if(x<xMin) xMin=x; if(x>xMax) xMax=x;
        }
      }}
      // sort by x
      std::vector<size_t> idx(xs.size()); std::iota(idx.begin(), idx.end(), 0);
      std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b){ return xs[a] < xs[b]; });
      auto reorder = [&](std::vector<double>& v){ std::vector<double> t(v.size()); for(size_t i=0;i<v.size();++i) t[i]=v[idx[i]]; v.swap(t); };
      reorder(xs); reorder(exs); reorder(yraw); reorder(eyraw); reorder(ycor); reorder(eycor);
      TGraphErrors* grR = new TGraphErrors((int)xs.size(), xs.data(), yraw.data(), exs.data(), eyraw.data()); grR->SetMarkerStyle(20); grR->SetMarkerColor(color); grR->SetLineColor(color); grR->SetLineStyle(2);
      TGraphErrors* grC = new TGraphErrors((int)xs.size(), xs.data(), ycor.data(), exs.data(), eycor.data()); grC->SetMarkerStyle(21); grC->SetMarkerColor(color); grC->SetLineColor(color);
      grRaw.push_back(grR); grCorr.push_back(grC);

      if(doNormPlots){ double sum=0; for(double v: ycor) sum+=v; std::vector<double> ycn(ycor.size()), eycn(ycor.size()); if(sum>0){ for(size_t i=0;i<ycor.size();++i){ ycn[i]=ycor[i]/sum; eycn[i]=eycor[i]/sum; }}
        TGraphErrors* grN = new TGraphErrors((int)xs.size(), xs.data(), ycn.data(), exs.data(), eycn.data()); grN->SetMarkerStyle(21); grN->SetMarkerColor(color); grN->SetLineColor(color);
        grNorm.push_back(grN);
      }
    }

    // Plot raw and corrected overlays
    gSystem->mkdir("results/aggregated", true);
    TCanvas* cRaw = new TCanvas("cRawCorr", "Raw vs Corrected (overlay)", 1000, 700); cRaw->SetLeftMargin(0.12); cRaw->SetBottomMargin(0.12);
    TH1D* hAxis = new TH1D("hAxis","",100, xMin, xMax); hAxis->GetXaxis()->SetTitle(VarLabel(vtype)); hAxis->GetYaxis()->SetTitle("Yield"); hAxis->SetMinimum(0);
    hAxis->Draw("AXIS"); TLegend* leg = new TLegend(0.60,0.60,0.88,0.88); leg->SetBorderSize(0); leg->SetFillStyle(0); leg->SetTextSize(0.032);
    for(size_t i=0;i<grCorr.size();++i){ grCorr[i]->Draw("P L SAME"); leg->AddEntry(grCorr[i], Form("Corrected MVA %.3f", mvaList[i]), "pl"); }
    for(size_t i=0;i<grRaw.size();++i){ grRaw[i]->Draw("P L SAME"); }
    leg->Draw();
    std::string baseOut = std::string("results/aggregated/corrected_yield_vs_") + VarName(vtype) + "_raw";
    cRaw->SaveAs((baseOut+".png").c_str()); cRaw->SaveAs((baseOut+".pdf").c_str()); delete cRaw;

    if(doNormPlots && !grNorm.empty()){
      TCanvas* cNorm = new TCanvas("cCorrNorm", "Corrected (normalized)", 1000, 700); cNorm->SetLeftMargin(0.12); cNorm->SetBottomMargin(0.12);
      TH1D* hAxisN = new TH1D("hAxisN","",100, xMin, xMax); hAxisN->GetXaxis()->SetTitle(VarLabel(vtype)); hAxisN->GetYaxis()->SetTitle("Yield (normalized)"); hAxisN->SetMinimum(0);
      hAxisN->Draw("AXIS"); TLegend* legN = new TLegend(0.60,0.60,0.88,0.88); legN->SetBorderSize(0); legN->SetFillStyle(0); legN->SetTextSize(0.032);
      for(size_t i=0;i<grNorm.size();++i){ grNorm[i]->Draw("P L SAME"); legN->AddEntry(grNorm[i], Form("MVA %.3f", mvaList[i]), "pl"); }
      legN->Draw(); std::string baseOutN = std::string("results/aggregated/corrected_yield_vs_") + VarName(vtype) + "_norm";
      cNorm->SaveAs((baseOutN+".png").c_str()); cNorm->SaveAs((baseOutN+".pdf").c_str()); delete cNorm;
    }

    std::cout << "Corrections applied and plots generated." << std::endl;
  }

  // ============================================
  // Dependency Checker & Auto-execution
  // ============================================
  
  bool CheckInput(const char* path, const char* description){
    TFile* f = TFile::Open(path, "READ");
    if(f && !f->IsZombie()){
      f->Close();
      std::cout << "[INPUT OK] " << description << ": " << path << std::endl;
      return true;
    }
    std::cout << "[INPUT MISSING] " << description << ": " << path << std::endl;
    return false;
  }

  void RunStageWithDependencies(Stage s, bool force, VarType vtype, const char* resultsDir,
                                RooDataSet* mcRds, std::vector<double>& mvaList){
    std::string varStr = VarName(vtype);
    std::string reweightFile = std::string("results/aggregated/reweight_") + varStr + ".root";
    std::string effFile = std::string("results/aggregated/efficiency_") + varStr + ".root";
    
    if(s == Stage::REWEIGHT){
      std::cout << "\n[STAGE 1] Reweighting" << std::endl;
      if(!force && CheckInput(reweightFile.c_str(), "Reweight file")) return;
      BuildGenPTWeightHist(resultsDir, mcRds, vtype);
      
    } else if(s == Stage::EFFICIENCY){
      std::cout << "\n[STAGE 2] Efficiency Map" << std::endl;
      if(!CheckInput(reweightFile.c_str(), "Reweight dependency")){
        std::cout << "Auto-running Stage 1 first..." << std::endl;
        RunStageWithDependencies(Stage::REWEIGHT, true, vtype, resultsDir, mcRds, mvaList);
      }
      if(!force && CheckInput(effFile.c_str(), "Efficiency file")) return;
      BuildEfficiencyMaps(resultsDir, vtype, mvaList, mcRds);
      
    } else if(s == Stage::CORRECT){
      std::cout << "\n[STAGE 3] Correction & Plot" << std::endl;
      if(!CheckInput(reweightFile.c_str(), "Reweight dependency")){
        std::cout << "Auto-running Stage 1 first..." << std::endl;
        RunStageWithDependencies(Stage::REWEIGHT, true, vtype, resultsDir, mcRds, mvaList);
      }
      
      TFile* fWeight = TFile::Open(reweightFile.c_str(), "READ");
      TH1D* hWeight = nullptr;
      if(fWeight && !fWeight->IsZombie()){
        hWeight = (TH1D*)fWeight->Get("hWeight");
      }
      
      ApplyCorrectionsAndPlot(resultsDir, vtype, hWeight, 1, mvaList);
      
      if(fWeight) fWeight->Close();
    }
  }
}

void CorrectYieldsWithMVA_Modular(int varTypeInt = 0, const char* resultsDir = "results/data_fits",
                                  int stage1 = 1, int stage2 = 1, int stage3 = 1, int doNormPlots = 1){
  using namespace RooFit;
  
  VarType vtype = static_cast<VarType>(varTypeInt);
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "=== Modular Yield Correction Workflow ===" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Variable: " << VarName(vtype) << std::endl;
  std::cout << "Stage 1 (Reweight): " << (stage1 ? "ON" : "OFF") << std::endl;
  std::cout << "Stage 2 (Efficiency): " << (stage2 ? "ON" : "OFF") << std::endl;
  std::cout << "Stage 3 (Correct): " << (stage3 ? "ON" : "OFF") << std::endl;
  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  
  gSystem->mkdir("results/aggregated", true);
  
  // Load MC flattened dataset for gen D* distribution
  std::cout << "\nLoading gen D* from skimGenTreeFlat (PbPb_MC_Oct22)..." << std::endl;
  
  SimpleDatasetManager mgr("/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json");
  auto mcInfo = mgr.GetDataset("PbPb_MC_Oct22");
  if(mcInfo.file.empty()){
    std::cerr << "ERROR: PbPb_MC_Oct22 not found!" << std::endl;
    return;
  }
  
  TFile* mcFile = TFile::Open(mcInfo.file.c_str(), "READ");
  if(!mcFile || mcFile->IsZombie()){
    std::cerr << "ERROR: Cannot open: " << mcInfo.file << std::endl;
    return;
  }
  
  TTree* mcTree = (TTree*)mcFile->Get(mcInfo.dataset_name.c_str());
  if(!mcTree){
    std::cerr << "ERROR: Cannot load " << mcInfo.dataset_name << std::endl;
    mcFile->Close();
    return;
  }
  
  std::cout << "MC tree entries: " << mcTree->GetEntries() << std::endl;
  
  // Build gen histograms directly from tree
  const double ptEdges[] = {5,7,10,20,30,40,50,60,70,80,90,100,110,120};
  const int nPt = sizeof(ptEdges)/sizeof(double) - 1;
  const double yEdges[] = {0, 0.5, 1.0, 1.5, 2.0, 2.5};
  const int nY = sizeof(yEdges)/sizeof(double) - 1;
  const double centEdges[] = {0, 10, 30, 50, 90};
  const int nCent = sizeof(centEdges)/sizeof(double) - 1;
  
  TH1D hGenPt_tmp("hGenPt_tmp", "", nPt, ptEdges);
  TH1D hGenY_tmp("hGenY_tmp", "", nY, yEdges);
  TH1D hGenCent_tmp("hGenCent_tmp", "", nCent, centEdges);
  
  // Draw from tree with matchGEN==1
  mcTree->Draw("pT>>hGenPt_tmp", "matchGEN==1", "goff");
  mcTree->Draw("abs(y)>>hGenY_tmp", "matchGEN==1", "goff");
  mcTree->Draw("centrality>>hGenCent_tmp", "matchGEN==1", "goff");
  
  // Create minimal RooDataSet from drawn histograms
  RooRealVar gen_pT_var("gen_pT", "gen pT", 0, 500);
  RooRealVar gen_y_var("gen_y", "gen y", 0, 3);
  RooRealVar centrality_var("centrality", "centrality", 0, 100);
  
  RooArgSet genVarSet;
  genVarSet.add(gen_pT_var);
  genVarSet.add(gen_y_var);
  genVarSet.add(centrality_var);
  
  RooDataSet* mcRds = new RooDataSet("mcGen", "MC gen", genVarSet);
  
  // Populate from drawn histograms for each bin
  for(int i=1; i<=nPt; ++i){
    double x = hGenPt_tmp.GetBinCenter(i);
    double cnt = hGenPt_tmp.GetBinContent(i);
    for(int j=0; j<cnt; ++j){
      gen_pT_var.setVal(x);
      mcRds->add(genVarSet);
    }
  }
  
  std::cout << "MC gen entries (fiducial matchGEN==1): " << mcRds->numEntries() << std::endl;
  
  // Extract MVA thresholds from fit results directory
  std::vector<double> mvaList;
  TSystemDirectory dir(resultsDir, resultsDir);
  TList* files = dir.GetListOfFiles();
  if(files){
    TIter next(files);
    TSystemFile* file;
    std::regex pattern1("fitresult_.*_mva([0-9]+p[0-9]+)\\.root");  // mva0p990 format
    std::regex pattern2("fitresult_.*_mva([0-9.]+)\\.root");         // mva0.990 format
    std::set<double> mvaSet;  // Use set to avoid duplicates
    while((file = (TSystemFile*)next())){
      std::string fname = file->GetName();
      std::smatch match;
      if(std::regex_search(fname, match, pattern1)){
        std::string s = match[1];
        for(char &c : s) if(c=='p') c='.';
        double mva = std::stod(s);
        mvaSet.insert(mva);
      } else if(std::regex_search(fname, match, pattern2)){
        double mva = std::stod(match[1]);
        mvaSet.insert(mva);
      }
    }
    // Convert set to vector and sort
    mvaList.assign(mvaSet.begin(), mvaSet.end());
  }
  std::sort(mvaList.begin(), mvaList.end());
  
  std::cout << "\nDetected " << mvaList.size() << " MVA thresholds:" << std::endl;
  for(double m : mvaList) std::cout << "  " << m << std::endl;
  
  // If no MVA thresholds detected, use default (0.990)
  if(mvaList.empty()){
    std::cout << "Warning: No MVA thresholds detected. Using default baseline (0.990)" << std::endl;
    mvaList.push_back(0.990);
  }
  
  // Run stages with dependency tracking
  if(stage1) RunStageWithDependencies(Stage::REWEIGHT, false, vtype, resultsDir, mcRds, mvaList);
  if(stage2) RunStageWithDependencies(Stage::EFFICIENCY, false, vtype, resultsDir, mcRds, mvaList);
  if(stage3) RunStageWithDependencies(Stage::CORRECT, false, vtype, resultsDir, mcRds, mvaList);
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "=== Workflow Complete ===" << std::endl;
  std::cout << "========================================" << std::endl;
}
