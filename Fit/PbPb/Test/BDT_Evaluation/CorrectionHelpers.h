// Helper functions for modular yield correction workflow
// Stage implementations and utility functions

#ifndef CORRECT_YIELDS_HELPERS_H
#define CORRECT_YIELDS_HELPERS_H

#include <vector>
#include <map>
#include <tuple>
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include "RooFitResult.h"

namespace YieldCorrectionHelpers {

  // ============================================
  // Data Structures
  // ============================================
  
  struct KinBin {
    double min, max;
    double center() const { return 0.5*(min+max); }
    double width() const { return max-min; }
  };
  
  struct YieldData {
    KinBin bin;
    double yield;
    double error;
    double efficiency;  // For efficiency corrections
    double effError;
  };

  // ============================================
  // File I/O
  // ============================================
  
  bool SaveHistogramToFile(TH1D* hist, const char* filename, const char* histName = nullptr){
    if(!hist) return false;
    TFile* f = TFile::Open(filename, "RECREATE");
    if(!f || f->IsZombie()) return false;
    if(histName) hist->SetName(histName);
    hist->Write();
    f->Close();
    delete f;
    return true;
  }
  
  TH1D* LoadHistogramFromFile(const char* filename, const char* histName){
    TFile* f = TFile::Open(filename, "READ");
    if(!f || f->IsZombie()) return nullptr;
    TH1D* h = (TH1D*)f->Get(histName);
    if(h) h->SetDirectory(0);
    f->Close();
    delete f;
    return h;
  }

  // ============================================
  // Stage 1: Reweighting Utilities
  // ============================================
  
  // Smooth histogram with moving average
  TH1D* SmoothHistogram(TH1D* h, int windowSize = 3){
    if(!h || windowSize < 1) return nullptr;
    TH1D* hSmooth = (TH1D*)h->Clone("hSmooth");
    
    for(int i = 1; i <= h->GetNbinsX(); ++i){
      double sum = 0, sumW = 0;
      for(int j = -windowSize; j <= windowSize; ++j){
        int bin = i + j;
        if(bin < 1 || bin > h->GetNbinsX()) continue;
        double w = (windowSize - fabs(j) + 1);
        sum += w * h->GetBinContent(bin);
        sumW += w;
      }
      hSmooth->SetBinContent(i, sumW > 0 ? sum/sumW : 0);
    }
    return hSmooth;
  }
  
  // Clip weight values to reasonable range
  void ClipWeights(TH1D* hWeight, double wmin = 0.2, double wmax = 5.0){
    if(!hWeight) return;
    for(int i = 1; i <= hWeight->GetNbinsX(); ++i){
      double w = hWeight->GetBinContent(i);
      if(w < wmin) hWeight->SetBinContent(i, wmin);
      if(w > wmax) hWeight->SetBinContent(i, wmax);
      hWeight->SetBinError(i, 0);
    }
  }

  // ============================================
  // Stage 2: Efficiency Utilities
  // ============================================
  
  struct EfficiencyResult {
    double eff;
    double effErr;
    bool valid;
  };
  
  EfficiencyResult ComputeEfficiency(double nGen, double nReco, double nGenErr = 0, double nRecoErr = 0){
    EfficiencyResult res;
    res.valid = false;
    
    if(nGen <= 0 || nReco < 0 || nReco > nGen) return res;
    
    res.eff = nReco / nGen;
    res.valid = true;
    
    if(nGenErr > 0 || nRecoErr > 0){
      double relErrGen = (nGenErr > 0 ? nGenErr/nGen : 0);
      double relErrReco = (nRecoErr > 0 ? nRecoErr/nReco : 0);
      res.effErr = res.eff * sqrt(relErrGen*relErrGen + relErrReco*relErrReco);
    } else {
      res.effErr = 0;
    }
    
    return res;
  }

  // ============================================
  // Stage 3: Correction Utilities
  // ============================================
  
  struct CorrectionResult {
    double rawYield;
    double rawError;
    double correctedYield;
    double correctedError;
    double correctionFactor;
  };
  
  CorrectionResult ApplyWeightCorrection(double yield, double yieldErr, 
                                         double weight, double weightErr = 0){
    CorrectionResult res;
    res.rawYield = yield;
    res.rawError = yieldErr;
    res.correctionFactor = weight;
    res.correctedYield = yield * weight;
    
    // Error propagation: dy_c = y_c * sqrt((dy/y)^2 + (dw/w)^2)
    if(yieldErr > 0 || weightErr > 0){
      double relErrY = (yieldErr > 0 && yield > 0 ? yieldErr/yield : 0);
      double relErrW = (weightErr > 0 && weight > 0 ? weightErr/weight : 0);
      res.correctedError = res.correctedYield * sqrt(relErrY*relErrY + relErrW*relErrW);
    } else {
      res.correctedError = 0;
    }
    
    return res;
  }

  // ============================================
  // Plotting Utilities
  // ============================================
  
  void PlotComparisonBefore_After(const std::vector<YieldData>& rawYields,
                                  const std::vector<YieldData>& correctedYields,
                                  const char* varLabel, const char* outputBase){
    if(rawYields.empty() || correctedYields.empty()) return;
    
    int n = rawYields.size();
    std::vector<double> x(n), ex(n), yRaw(n), eyRaw(n), yCor(n), eyCor(n);
    
    for(int i = 0; i < n; ++i){
      x[i] = rawYields[i].bin.center();
      ex[i] = rawYields[i].bin.width() / 2.0;
      yRaw[i] = rawYields[i].yield;
      eyRaw[i] = rawYields[i].error;
      yCor[i] = correctedYields[i].yield;
      eyCor[i] = correctedYields[i].error;
    }
    
    TGraphErrors* grRaw = new TGraphErrors(n, x.data(), yRaw.data(), ex.data(), eyRaw.data());
    TGraphErrors* grCor = new TGraphErrors(n, x.data(), yCor.data(), ex.data(), eyCor.data());
    
    grRaw->SetTitle("Raw vs Corrected Yield");
    grRaw->SetMarkerStyle(20);
    grRaw->SetMarkerColor(kBlue);
    grRaw->SetLineColor(kBlue);
    grRaw->SetMarkerSize(1.0);
    
    grCor->SetMarkerStyle(21);
    grCor->SetMarkerColor(kRed);
    grCor->SetLineColor(kRed);
    grCor->SetMarkerSize(1.0);
    
    TCanvas* c = new TCanvas("cComparison", "Raw vs Corrected", 900, 600);
    c->SetLeftMargin(0.12);
    c->SetBottomMargin(0.12);
    
    grRaw->GetXaxis()->SetTitle(varLabel);
    grRaw->GetYaxis()->SetTitle("Yield");
    grRaw->Draw("APL");
    grCor->Draw("PL SAME");
    
    TLegend* leg = new TLegend(0.70, 0.70, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(grRaw, "Raw", "pl");
    leg->AddEntry(grCor, "Corrected", "pl");
    leg->Draw();
    
    c->SaveAs((std::string(outputBase) + ".png").c_str());
    c->SaveAs((std::string(outputBase) + ".pdf").c_str());
    
    delete c;
    delete grRaw;
    delete grCor;
  }

  // ============================================
  // Normalization Utilities
  // ============================================
  
  std::vector<double> NormalizeArray(const std::vector<double>& values){
    double sum = 0;
    for(double v : values) sum += v;
    
    std::vector<double> normalized(values.size());
    if(sum > 0){
      for(size_t i = 0; i < values.size(); ++i){
        normalized[i] = values[i] / sum;
      }
    }
    return normalized;
  }

}

#endif // CORRECT_YIELDS_HELPERS_H
