// Aggregate fit results and plot yield vs variable for each MVA
// Usage example:
//   root -l -b -q 'PlotYieldVsVar.cpp(0, "results/data_fits", 1)'
// Args:
//   varTypeInt: 0=PT, 1=RAPIDITY, 2=CENTRALITY, 3=COS
//   resultsDir: directory containing fitresult_*.root
//   normMode: 0=none, 1=normalize per-MVA to unit area across bins

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <regex>
#include <string>
#include <utility>

#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TFile.h"
#include "TKey.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TGraphErrors.h"
#include "TAxis.h"
#include "TColor.h"
#include "TH1D.h"

#include "RooFitResult.h"
#include "RooArgList.h"
#include "RooRealVar.h"

namespace {
  enum class VarType { PT=0, RAPIDITY=1, CENTRALITY=2, COS=3 };
  const char* VarName(VarType v){
    switch(v){ case VarType::PT: return "pT"; case VarType::RAPIDITY: return "y"; case VarType::CENTRALITY: return "centrality"; case VarType::COS: return "cos"; }
    return "pT";
  }
  const char* VarLabel(VarType v){
    switch(v){ case VarType::PT: return "p_{T} [GeV/c]"; case VarType::RAPIDITY: return "|y|"; case VarType::CENTRALITY: return "Centrality [%]"; case VarType::COS: return "cos(\u03B8)"; }
    return "p_{T} [GeV/c]";
  }

  bool getParamValErr(RooFitResult* res, const char* name, double& val, double& err){
    if(!res) return false;
    // Check floatParsFinal
    RooArgList fl = res->floatParsFinal();
    if (auto* rr = dynamic_cast<RooRealVar*>(fl.find(name))) { val = rr->getVal(); err = rr->getError(); return true; }
    // Check constPars
    RooArgList cp = res->constPars();
    if (auto* rc = dynamic_cast<RooRealVar*>(cp.find(name))) { val = rc->getVal(); err = 0.0; return true; }
    return false;
  }

  // Replace all occurrences of a char in string
  std::string replaceAll(std::string s, char from, char to){ for(char& c: s){ if(c==from) c=to; } return s; }

  struct BinInfo { double min; double max; double center() const { return 0.5*(min+max); } };
}

void PlotYieldVsVar(int varTypeInt=0, const char* resultsDir="results/data_fits", int normMode=1){
  VarType vtype = static_cast<VarType>(varTypeInt);
  gStyle->SetOptStat(0);

  // Prepare
  std::string varName = VarName(vtype);
  std::string dirPath = resultsDir;

  // List files in resultsDir
  TSystemDirectory dir("resdir", dirPath.c_str());
  TList* files = dir.GetListOfFiles();
  if(!files){ std::cerr << "No files in " << dirPath << std::endl; return; }

  // Data structure: mva -> vector of (bin info, yield, err)
  std::map<double, std::vector<std::tuple<BinInfo,double,double>>> mvaMap;

  // Regex to match filenames: fitresult_<var>_<min>_<max>_mva<score>.root
  // Example: fitresult_pT_5p00_7p00_mva0p990.root
  std::regex re(Form("^fitresult_%s_([0-9]+p[0-9]+)_([0-9]+p[0-9]+)_mva([0-9]+p[0-9]+)\\.root$", varName.c_str()));

  // Also accept files with extra directory path
  TIter next(files);
  while (TObject* obj = next()){
    auto* f = dynamic_cast<TSystemFile*>(obj);
    if(!f) continue;
    const char* fname = f->GetName();
    if(f->IsDirectory()) continue;
    std::cmatch m;
    if(!std::regex_match(fname, m, re)) continue;

    std::string sMin = m[1];
    std::string sMax = m[2];
    std::string sMva = m[3];

    sMin = replaceAll(sMin,'p','.');
    sMax = replaceAll(sMax,'p','.');
    sMva = replaceAll(sMva,'p','.');

    double vmin = atof(sMin.c_str());
    double vmax = atof(sMax.c_str());
    double mva  = atof(sMva.c_str());

    std::string fullPath = dirPath + std::string("/") + fname;
    TFile* tf = TFile::Open(fullPath.c_str(), "READ");
    if(!tf || tf->IsZombie()){ if(tf) tf->Close(); continue; }

    auto* fitRes = dynamic_cast<RooFitResult*>(tf->Get("fitResult"));
    if(!fitRes){ tf->Close(); continue; }

    double nsig=0, nsigErr=0; bool ok = getParamValErr(fitRes, "nsig", nsig, nsigErr);
    tf->Close();
    if(!ok) continue;

    mvaMap[mva].push_back({ BinInfo{vmin, vmax}, nsig, nsigErr });
  }

  if(mvaMap.empty()){
    std::cerr << "No fitresult files matched for var=" << varName << " under " << dirPath << std::endl;
    return;
  }

  // Sort bins for each mva by lower edge
  for(auto& kv : mvaMap){
    auto& vec = kv.second;
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b){ return std::get<0>(a).min < std::get<0>(b).min; });
  }

  // Keep raw copy before optional normalization
  auto mvaMapRaw = mvaMap;
  // Normalize per-MVA if requested
  if(normMode==1){
    for(auto& kv : mvaMap){
      double sum=0.0; for(const auto& t : kv.second) sum += std::get<1>(t);
      if(sum>0){ for(auto& t : kv.second){ std::get<1>(t) /= sum; std::get<2>(t) /= sum; } }
    }
  }

  // Prepare canvas and legend
  gSystem->mkdir("results/aggregated", true);
  TCanvas* c = new TCanvas("cYieldVsVar","Yield vs variable per MVA", 1000, 700);
  c->SetLeftMargin(0.12); c->SetBottomMargin(0.12); c->SetRightMargin(0.04);

  // Determine x-range from all bins
  double xMin=1e9, xMax=-1e9; int maxNBins=0;
  for(const auto& kv : mvaMap){
    for(const auto& t : kv.second){ xMin = std::min(xMin, std::get<0>(t).min); xMax = std::max(xMax, std::get<0>(t).max); }
    if((int)kv.second.size()>maxNBins) maxNBins = (int)kv.second.size();
  }

  // Create an empty frame histogram for axes
  TH1D* hframe = new TH1D("hframe","", 100, xMin, xMax);
  hframe->GetXaxis()->SetTitle(VarLabel(vtype));
  hframe->GetYaxis()->SetTitle(normMode?"Normalized yield":"Yield");
  hframe->SetMaximum(0.0);
  for(const auto& kv : mvaMap){ for(const auto& t : kv.second){ hframe->SetMaximum(std::max(hframe->GetMaximum(), std::get<1>(t)*1.3)); } }
  if(hframe->GetMaximum()<=0) hframe->SetMaximum(1.0);
  hframe->SetMinimum(0);
  hframe->Draw("AXIS");

  TLegend* leg = new TLegend(0.60, 0.60, 0.88, 0.88);
  leg->SetBorderSize(0); leg->SetFillStyle(0); leg->SetTextSize(0.032);

  // Assign colors across MVA range
  std::vector<int> colors = {kBlack, kRed+1, kBlue+1, kGreen+2, kMagenta+1, kCyan+2, kOrange+7, kViolet+1, kTeal+1, kPink+1};
  int colorIdx=0;

  // Draw one graph per MVA
  std::vector<TGraphErrors*> graphs;
  for(auto& kv : mvaMap){
    double mva = kv.first;
    auto& vec = kv.second;
    int n = (int)vec.size();
    if(n==0) continue;

    auto* gr = new TGraphErrors(n);
    for(int i=0;i<n;++i){
      const auto& bi = std::get<0>(vec[i]);
      double x = bi.center();
      double ex = 0.5*(bi.max - bi.min);
      double y = std::get<1>(vec[i]);
      double ey = std::get<2>(vec[i]);
      gr->SetPoint(i, x, y);
      gr->SetPointError(i, ex, ey);
    }
    int col = colors[colorIdx % colors.size()]; colorIdx++;
    gr->SetMarkerStyle(20); gr->SetMarkerSize(0.9); gr->SetLineWidth(2); gr->SetMarkerColor(col); gr->SetLineColor(col);
    if(graphs.empty()) hframe->Draw("AXIS");
    gr->Draw("P L SAME");
    graphs.push_back(gr);
    leg->AddEntry(gr, Form("MVA %.3f", mva), "pl");
  }

  leg->Draw();
  TLatex tx; tx.SetNDC(); tx.SetTextSize(0.035);
  tx.DrawLatex(0.15, 0.92, Form("Yield vs %s per MVA%s", VarName(vtype), normMode?" (normalized)":""));

  // Save outputs
  std::string outBase = std::string("results/aggregated/yield_vs_") + varName + (normMode?"_norm":"");
  c->SaveAs((outBase + ".png").c_str());
  c->SaveAs((outBase + ".pdf").c_str());
  TFile* fout = TFile::Open((outBase + ".root").c_str(), "RECREATE");
  c->Write("canvas");
  for(size_t i=0;i<graphs.size();++i){ graphs[i]->Write(Form("graph_mva_%zu", i)); }
  fout->Close();

  // Additionally, draw unnormalized (raw) yields with y-max = 1.2 * max
  TCanvas* c2 = new TCanvas("cYieldVsVar_raw","Yield vs variable per MVA (raw)", 1000, 700);
  c2->SetLeftMargin(0.12); c2->SetBottomMargin(0.12); c2->SetRightMargin(0.04);

  double maxYRaw = 0.0; double xMinRaw = 1e9, xMaxRaw = -1e9;
  for(const auto& kv : mvaMapRaw){
    for(const auto& t : kv.second){
      xMinRaw = std::min(xMinRaw, std::get<0>(t).min);
      xMaxRaw = std::max(xMaxRaw, std::get<0>(t).max);
      maxYRaw = std::max(maxYRaw, std::get<1>(t));
    }
  }
  TH1D* hframeRaw = new TH1D("hframeRaw","", 100, xMinRaw, xMaxRaw);
  hframeRaw->GetXaxis()->SetTitle(VarLabel(vtype));
  hframeRaw->GetYaxis()->SetTitle("Yield");
  hframeRaw->SetMinimum(0);
  hframeRaw->SetMaximum(maxYRaw>0 ? maxYRaw*1.2 : 1.0);
  hframeRaw->Draw("AXIS");

  TLegend* leg2 = new TLegend(0.60, 0.60, 0.88, 0.88);
  leg2->SetBorderSize(0); leg2->SetFillStyle(0); leg2->SetTextSize(0.032);

  std::vector<TGraphErrors*> graphsRaw;
  int colorIdx2 = 0; std::vector<int> colors2 = {kBlack, kRed+1, kBlue+1, kGreen+2, kMagenta+1, kCyan+2, kOrange+7, kViolet+1, kTeal+1, kPink+1};
  for(auto& kv : mvaMapRaw){
    double mva = kv.first; auto& vec = kv.second; int n = (int)vec.size(); if(n==0) continue;
    auto* gr = new TGraphErrors(n);
    for(int i=0;i<n;++i){
      const auto& bi = std::get<0>(vec[i]); double x=bi.center(); double ex=0.5*(bi.max-bi.min);
      double y=std::get<1>(vec[i]); double ey=std::get<2>(vec[i]);
      gr->SetPoint(i, x, y); gr->SetPointError(i, ex, ey);
    }
    int col = colors2[colorIdx2 % colors2.size()]; colorIdx2++;
    gr->SetMarkerStyle(20); gr->SetMarkerSize(0.9); gr->SetLineWidth(2); gr->SetMarkerColor(col); gr->SetLineColor(col);
    gr->Draw("P L SAME");
    graphsRaw.push_back(gr);
    leg2->AddEntry(gr, Form("MVA %.3f", mva), "pl");
  }
  leg2->Draw();
  TLatex tx2; tx2.SetNDC(); tx2.SetTextSize(0.035);
  tx2.DrawLatex(0.15, 0.92, Form("Yield vs %s per MVA (raw)", VarName(vtype)));

  std::string outBaseRaw = std::string("results/aggregated/yield_vs_") + varName + std::string("_raw");
  c2->SaveAs((outBaseRaw + ".png").c_str());
  c2->SaveAs((outBaseRaw + ".pdf").c_str());
  TFile* foutRaw = TFile::Open((outBaseRaw + ".root").c_str(), "RECREATE");
  c2->Write("canvas");
  for(size_t i=0;i<graphsRaw.size();++i){ graphsRaw[i]->Write(Form("graph_mva_%zu", i)); }
  foutRaw->Close();
}
