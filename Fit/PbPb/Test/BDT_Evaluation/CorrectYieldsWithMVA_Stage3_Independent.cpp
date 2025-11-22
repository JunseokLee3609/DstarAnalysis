// Stage 3 (Correction & Plotting) - Independent macro
// Apply efficiency corrections to data yields and plot results
//
// Usage:
//   root -l -b -q 'CorrectYieldsWithMVA_Stage3_Independent.cpp(0, "results/data_fits", "incl", 1)'
//
// Args:
//   varTypeInt: 0=PT, 1=RAPIDITY, 2=CENTRALITY
//   resultsDir: directory containing fitresult_*.root files
//   suffix: "incl", "prompt", or "nonprompt"
//   doNormPlots: 1=also produce normalized plots

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
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLine.h"

#include "RooFitResult.h"
#include "RooArgList.h"
#include "RooRealVar.h"

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

  std::map<double, TGraphErrors*> ApplyCorrectionsAndPlot(VarType vtype, const char* resultsDir, 
                                                           const std::map<double, TH1D*>& effMaps,
                                                           const char* suffix, bool doNorm){
    std::cout << "\n=== STAGE 3: Apply Corrections & Plot ===" << std::endl;
    std::cout << "Variable: " << VarName(vtype) << std::endl;
    std::cout << "Mode: " << suffix << std::endl;
    
    if(effMaps.empty()){ std::cerr << "ERROR: No efficiency maps!" << std::endl; return {}; }
    
    std::string varStr = VarName(vtype);
    std::string prefix = std::string("fitresult_") + varStr + "_";
    
    // Get analytic bins from config
    const auto& analyticBins = GetBins(vtype);
    const int nBins = GetNBins(vtype);
    
    std::cout << "Using " << nBins << " analytic bins from config" << std::endl;
    
    // Get valid MVA thresholds from config
    const auto& mvaThresholds = GetMVAThresholds();
    
    // Structure: [mva][binIndex] -> (yield, error)
    std::map<double, std::vector<std::pair<double,double>>> dataYields;
    
    // Initialize arrays for each MVA
    for(double mva : mvaThresholds){
      dataYields[mva].resize(nBins, {0, 0});
    }
    
    // Load yields for each analytic bin
    for(int iBin=0; iBin<nBins; ++iBin){
      double vmin = analyticBins[iBin].first;
      double vmax = analyticBins[iBin].second;
      
      for(double mva : mvaThresholds){
        // Construct filename
        char mvaBuf[64];
        snprintf(mvaBuf, sizeof(mvaBuf), "mva%.3f", mva);
        std::string mvaTag = mvaBuf;
        for(char &c : mvaTag) if(c=='.') c='p';
        
        std::string vminStr = Form("%.2f", vmin);
        std::string vmaxStr = Form("%.2f", vmax);
        for(char &c : vminStr) if(c=='.') c='p';
        for(char &c : vmaxStr) if(c=='.') c='p';
        
        std::string fname = prefix + vminStr + "_" + vmaxStr;
        if(std::string(suffix) != "incl"){
          fname += std::string("_") + suffix;
        }
        fname += "_" + mvaTag + ".root";
        
        std::string fpath = std::string(resultsDir) + "/" + fname;
        TFile* tf = TFile::Open(fpath.c_str(), "READ");
        if(!tf || tf->IsZombie()){ 
          if(tf) tf->Close();
          std::cout << "WARNING: Cannot open " << fname << std::endl;
          continue; 
        }
        
        RooFitResult* r = (RooFitResult*)tf->Get("fitResult");
        double y=0, ey=0;
        if(r && getParamValErr(r, "nsig", y, ey)){
          dataYields[mva][iBin] = {y, ey};
        } else {
          std::cout << "WARNING: Cannot extract nsig from " << fname << std::endl;
        }
        tf->Close();
      }
    }
    
    std::cout << "Loaded data yields for " << mvaThresholds.size() << " MVA thresholds" << std::endl;
    
    std::map<double, TGraphErrors*> correctedGraphs;
    std::map<double, TGraphErrors*> rawGraphs;
    
    for(double mva : mvaThresholds){
      auto &yieldVec = dataYields[mva];
      
      auto itEff = effMaps.find(mva);
      if(itEff == effMaps.end()){
        std::cout << "WARNING: No efficiency map for MVA " << mva << std::endl;
        continue;
      }
      TH1D* hEff = itEff->second;
      
      std::vector<double> xs, ys, exs, eys;
      std::vector<double> ysRaw, eysRaw;
      
      for(int iBin=0; iBin<nBins; ++iBin){
        double vmin = analyticBins[iBin].first;
        double vmax = analyticBins[iBin].second;
        double x = 0.5*(vmin + vmax);
        
        double rawY = yieldVec[iBin].first;
        double rawEY = yieldVec[iBin].second;
        
        if(rawY <= 0) continue;  // Skip empty bins
        
        int bin = hEff->FindBin(x);
        double eff = hEff->GetBinContent(bin);
        double effErr = hEff->GetBinError(bin);
        
        if(eff <= 0 || eff > 1.5){ 
          std::cout << "WARNING: Bad efficiency at x=" << x << ": " << eff << std::endl;
          continue;
        }
        
        double corrY = rawY / eff;
        double corrEY = corrY * sqrt(pow(rawEY/rawY, 2) + pow(effErr/eff, 2));
        
        double exVal = 0.5 * (vmax - vmin);  // Half bin width for horizontal error bar
        
        xs.push_back(x);
        ys.push_back(corrY);
        exs.push_back(exVal);
        eys.push_back(corrEY);
        
        ysRaw.push_back(rawY);
        eysRaw.push_back(rawEY);
      }
      
      if(!xs.empty()){
        correctedGraphs[mva] = new TGraphErrors(xs.size(), xs.data(), ys.data(), exs.data(), eys.data());
        correctedGraphs[mva]->SetName(Form("corrected_mva%.3f", mva));
        
        rawGraphs[mva] = new TGraphErrors(xs.size(), xs.data(), ysRaw.data(), exs.data(), eysRaw.data());
        rawGraphs[mva]->SetName(Form("raw_mva%.3f", mva));
      }
    }
    
    std::cout << "Created corrected graphs for " << correctedGraphs.size() << " MVA thresholds" << std::endl;
    
    // Plot: Raw vs Corrected - use all MVA thresholds from config
    std::vector<double> mvaPlotList(mvaThresholds.begin(), mvaThresholds.end());
    std::vector<int> colors = {kBlue+1, kGreen+2, kRed+1, kMagenta+1, kOrange+1};
    
    // Create canvas with 3x2 layout (3 in first row, 2 in second row)
    TCanvas* c1 = new TCanvas("cRawVsCorrected", "Raw vs Corrected", 1800, 1200);
    c1->Divide(3, 2, 0.001, 0.001);
    
    for(size_t i=0; i<mvaPlotList.size(); ++i){
      double mva = mvaPlotList[i];
      auto itCorr = correctedGraphs.find(mva);
      auto itRaw = rawGraphs.find(mva);
      if(itCorr == correctedGraphs.end() || itRaw == rawGraphs.end()) continue;
      
      c1->cd(i+1);
      gPad->SetLeftMargin(0.13);
      gPad->SetRightMargin(0.05);
      gPad->SetBottomMargin(0.12);
      
      TGraphErrors* gCorr = itCorr->second;
      TGraphErrors* gRaw = itRaw->second;
      
      int color = colors[i % colors.size()];
      gCorr->SetMarkerStyle(20);
      gCorr->SetMarkerColor(color);
      gCorr->SetLineColor(color);
      gCorr->SetLineWidth(2);
      gCorr->SetMarkerSize(1.2);
      gCorr->GetXaxis()->SetTitle(VarLabel(vtype));
      gCorr->GetYaxis()->SetTitle("N_{signal}");
      gCorr->SetTitle("");
      
      gRaw->SetMarkerStyle(24);
      gRaw->SetMarkerColor(kGray+2);
      gRaw->SetLineColor(kGray+2);
      gRaw->SetLineWidth(2);
      gRaw->SetMarkerSize(1.0);
      
      double ymax = std::max(TMath::MaxElement(gCorr->GetN(), gCorr->GetY()),
                             TMath::MaxElement(gRaw->GetN(), gRaw->GetY())) * 1.3;
      gCorr->SetMinimum(0);
      gCorr->SetMaximum(ymax);
      
      gCorr->Draw("APE");
      gRaw->Draw("PE SAME");
      
      TLatex tx;
      tx.SetNDC();
      tx.SetTextFont(62);
      tx.SetTextSize(0.05);
      tx.DrawLatex(0.20, 0.85, Form("MVA > %.3f", mva));
      
      TLegend* leg = new TLegend(0.50, 0.70, 0.90, 0.88);
      leg->SetBorderSize(0);
      leg->SetFillStyle(0);
      leg->SetTextSize(0.04);
      leg->AddEntry(gCorr, "Corrected", "lp");
      leg->AddEntry(gRaw, "Raw", "lp");
      leg->Draw();
    }
    
    std::string outBase = AggregatedDir(std::string("yields_rawVsCorrected_") + varStr + "_" + suffix);
    c1->SaveAs((outBase + ".png").c_str());
    c1->SaveAs((outBase + ".pdf").c_str());
    delete c1;
    
    // Save to ROOT file
    TFile* fOut = TFile::Open((outBase + ".root").c_str(), "RECREATE");
    for(auto &kv : correctedGraphs) if(kv.second) kv.second->Write();
    for(auto &kv : rawGraphs) if(kv.second) kv.second->Write();
    fOut->Close();
    delete fOut;
    
    std::cout << "Saved plots: " << outBase << std::endl;
    
    // Normalized plot if requested
    if(doNorm){
      TCanvas* c2 = new TCanvas("cNormalized", "Normalized Yields", 1200, 800);
      c2->SetLeftMargin(0.12);
      c2->SetRightMargin(0.05);
      c2->SetBottomMargin(0.12);
      
      TLegend* legNorm = new TLegend(0.60, 0.65, 0.88, 0.88);
      legNorm->SetBorderSize(0);
      legNorm->SetFillStyle(0);
      legNorm->SetTextSize(0.035);
      
      bool first = true;
      for(size_t i=0; i<mvaPlotList.size(); ++i){
        double mva = mvaPlotList[i];
        auto it = correctedGraphs.find(mva);
        if(it == correctedGraphs.end()) continue;
        
        TGraphErrors* g = (TGraphErrors*)it->second->Clone();
        int color = colors[i % colors.size()];
        g->SetMarkerStyle(20+i);
        g->SetMarkerColor(color);
        g->SetLineColor(color);
        g->SetLineWidth(2);
        g->SetMarkerSize(1.2);
        
        double norm = (g->GetN() > 0) ? g->GetY()[0] : 1.0;
        if(norm > 0){
          for(int ip=0; ip<g->GetN(); ++ip){
            g->GetY()[ip] /= norm;
            g->GetEY()[ip] /= norm;
          }
        }
        
        g->GetXaxis()->SetTitle(VarLabel(vtype));
        g->GetYaxis()->SetTitle("N_{signal} / N_{signal}(first bin)");
        g->SetTitle("");
        g->SetMinimum(0);
        g->SetMaximum(2.0);
        
        if(first){
          g->Draw("APE");
          first = false;
        } else {
          g->Draw("PE SAME");
        }
        
        legNorm->AddEntry(g, Form("MVA > %.3f", mva), "lp");
      }
      
      legNorm->Draw();
      
      std::string outNorm = AggregatedDir(std::string("yields_normalized_") + varStr + "_" + suffix);
      c2->SaveAs((outNorm + ".png").c_str());
      c2->SaveAs((outNorm + ".pdf").c_str());
      delete c2;
      
      // Ratio plot: Each MVA / First MVA
      if(!mvaPlotList.empty() && correctedGraphs.size() > 1){
        double baseMVA = mvaPlotList[0];
        auto itBase = correctedGraphs.find(baseMVA);
        if(itBase != correctedGraphs.end()){
          TGraphErrors* gBase = itBase->second;
          
          TCanvas* c3 = new TCanvas("cRatio", "Ratio to First MVA", 1200, 800);
          c3->SetLeftMargin(0.12);
          c3->SetRightMargin(0.05);
          c3->SetBottomMargin(0.12);
          
          TLegend* legRatio = new TLegend(0.60, 0.65, 0.88, 0.88);
          legRatio->SetBorderSize(0);
          legRatio->SetFillStyle(0);
          legRatio->SetTextSize(0.035);
          
          bool firstRatio = true;
          for(size_t i=1; i<mvaPlotList.size(); ++i){  // Start from index 1 (skip base MVA)
            double mva = mvaPlotList[i];
            auto it = correctedGraphs.find(mva);
            if(it == correctedGraphs.end()) continue;
            
            TGraphErrors* gCurrent = it->second;
            
            // Create ratio graph
            std::vector<double> xRatio, yRatio, exRatio, eyRatio;
            for(int ip=0; ip<gCurrent->GetN(); ++ip){
              double x = gCurrent->GetX()[ip];
              double y = gCurrent->GetY()[ip];
              double ey = gCurrent->GetEY()[ip];
              
              // Find corresponding point in base graph
              int iBase = -1;
              for(int ib=0; ib<gBase->GetN(); ++ib){
                if(fabs(gBase->GetX()[ib] - x) < 0.01){
                  iBase = ib;
                  break;
                }
              }
              
              if(iBase >= 0){
                double yBase = gBase->GetY()[iBase];
                double eyBase = gBase->GetEY()[iBase];
                
                if(yBase > 0){
                  double ratio = y / yBase;
                  double ratioErr = ratio * sqrt(pow(ey/y, 2) + pow(eyBase/yBase, 2));
                  
                  xRatio.push_back(x);
                  yRatio.push_back(ratio);
                  exRatio.push_back(0);
                  eyRatio.push_back(ratioErr);
                }
              }
            }
            
            if(!xRatio.empty()){
              TGraphErrors* gRatio = new TGraphErrors(xRatio.size(), xRatio.data(), yRatio.data(), 
                                                      exRatio.data(), eyRatio.data());
              int color = colors[i % colors.size()];
              gRatio->SetMarkerStyle(20+i-1);
              gRatio->SetMarkerColor(color);
              gRatio->SetLineColor(color);
              gRatio->SetLineWidth(2);
              gRatio->SetMarkerSize(1.2);
              gRatio->GetXaxis()->SetTitle(VarLabel(vtype));
              gRatio->GetYaxis()->SetTitle(Form("N_{signal} / N_{signal}(MVA > %.3f)", baseMVA));
              gRatio->SetTitle("");
              gRatio->SetMinimum(0.5);
              gRatio->SetMaximum(1.5);
              
              if(firstRatio){
                gRatio->Draw("APE");
                firstRatio = false;
              } else {
                gRatio->Draw("PE SAME");
              }
              
              legRatio->AddEntry(gRatio, Form("MVA > %.3f / MVA > %.3f", mva, baseMVA), "lp");
            }
          }
          
          // Draw horizontal line at ratio = 1
          if(!firstRatio){
            TLine* line = new TLine(gBase->GetX()[0], 1.0, 
                                   gBase->GetX()[gBase->GetN()-1], 1.0);
            line->SetLineStyle(2);
            line->SetLineColor(kBlack);
            line->SetLineWidth(2);
            line->Draw();
          }
          
          legRatio->Draw();
          
          std::string outRatio = AggregatedDir(std::string("yields_ratio_") + varStr + "_" + suffix);
          c3->SaveAs((outRatio + ".png").c_str());
          c3->SaveAs((outRatio + ".pdf").c_str());
          delete c3;
        }
      }
    }
    
    for(auto &kv : rawGraphs) delete kv.second;
    
    return correctedGraphs;
  }
}

void CorrectYieldsWithMVA_Stage3_Independent(int varTypeInt = 0, const char* resultsDir = "results/data_fits",
                                              const char* suffix = "incl", int doNormPlots = 1){
  VarType vtype = static_cast<VarType>(varTypeInt);
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "=== Stage 3: Yield Corrections ===" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Variable: " << VarName(vtype) << std::endl;
  std::cout << "Mode: " << suffix << std::endl;
  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  
  // Load efficiency maps from Stage 2
  std::string effFile = AggregatedDir(std::string("efficiency_") + VarName(vtype) + "_" + suffix + ".root");
  TFile* f = TFile::Open(effFile.c_str(), "READ");
  if(!f || f->IsZombie()){
    std::cerr << "ERROR: Cannot open efficiency file: " << effFile << std::endl;
    std::cerr << "Run Stage 2 first!" << std::endl;
    if(f) f->Close();
    return;
  }
  
  std::map<double, TH1D*> effMaps;
  TIter next(f->GetListOfKeys());
  while(auto key = (TKey*)next()){
    TObject* obj = key->ReadObj();
    if(!obj) continue;
    if(auto h = dynamic_cast<TH1D*>(obj)){
      std::string name = h->GetName();
      double mva = 0.0;
      if(sscanf(name.c_str(), "efficiency_mva%lf", &mva) == 1){
        TH1D* clone = (TH1D*)h->Clone();
        clone->SetDirectory(nullptr);
        effMaps[mva] = clone;
      }
    }
  }
  f->Close();
  
  std::cout << "Loaded " << effMaps.size() << " efficiency maps" << std::endl;
  
  if(effMaps.empty()){
    std::cerr << "ERROR: No efficiency maps found!" << std::endl;
    return;
  }
  
  auto correctedGraphs = ApplyCorrectionsAndPlot(vtype, resultsDir, effMaps, suffix, doNormPlots);
  
  if(!correctedGraphs.empty()){
    std::cout << "\n✓ Stage 3 complete!" << std::endl;
  } else {
    std::cout << "\n✗ Stage 3 failed!" << std::endl;
  }
  
  for(auto &kv : effMaps) delete kv.second;
  for(auto &kv : correctedGraphs) delete kv.second;
}
