// Build per-MVA 1D correction factors vs kinematic variable from MC (matchGEN==1)
// Apply to data fit yields and plot corrected vs uncorrected distributions (normalized and raw)
// Usage:
//   root -l -b -q 'CorrectYieldsWithMVA.cpp(0, "results/data_fits", 1)'
// Args:
//   varTypeInt: 0=PT, 1=RAPIDITY, 2=CENTRALITY, 3=COS (COS not supported for dataset vars here)
//   resultsDir: directory containing data fitresult_*.root
//   doNormPlots: 1 to also produce normalized plots

#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <regex>
#include <string>

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
#include "TPad.h" // Added for pad creation

#include "RooFitResult.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooArgSet.h"

// Access MC dataset through SimpleDatasetManager (as used in FitSingleBin.cpp)
#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"

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
  std::string replaceAll(std::string s, char from, char to){ for(char& c: s){ if(c==from) c=to; } return s; }

  bool getParamValErr(RooFitResult* res, const char* name, double& val, double& err){
    if(!res) return false;
    RooArgList fl = res->floatParsFinal();
    if (auto* rr = dynamic_cast<RooRealVar*>(fl.find(name))) { val = rr->getVal(); err = rr->getError(); return true; }
    RooArgList cp = res->constPars();
    if (auto* rc = dynamic_cast<RooRealVar*>(cp.find(name))) { val = rc->getVal(); err = 0.0; return true; }
    return false;
  }

  struct Bin { double min, max; double center() const { return 0.5*(min+max); } };
  
  // Helper: build gen-pT weight histogram w(pt) = H_target(pt) / H_mc,gen(pt)
  // - target from data fit yields vs pT at baseline MVA (default 0.990)
  // - mc,gen from MC RooDataSet with matchGEN==1 using reco pT as proxy for gen pT
  // - same variable binning, Sumw2, under/overflow fixed to edge bins, optional smoothing, clipping
  TH1D* BuildGenPTWeightHist(const char* resultsDir, RooDataSet* mcRds, double baseMVA = 0.990,
                             double wmin = 0.2, double wmax = 5.0, bool smooth = true){
    // pT binning
    const double ptEdges[] = {5,7,10,20,30,40,50,60,70,80,90,100,110,120};
    const int nPt = sizeof(ptEdges)/sizeof(double) - 1;
    auto* Hmc = new TH1D("H_mc_gen",";p_{T} [GeV/c];MC gen-matched", nPt, ptEdges);
    auto* Htg = new TH1D("H_target",";p_{T} [GeV/c];Target (Data)", nPt, ptEdges);
    Hmc->Sumw2(); Htg->Sumw2();
    
    // Fill MC gen-matched pT
    std::unique_ptr<RooDataSet> genAll((RooDataSet*)mcRds->reduce("matchGEN==1"));
    if (genAll){
      for (int i=0;i<genAll->numEntries();++i){ const RooArgSet* row = genAll->get(i); if(!row) continue; auto* vpt = (RooRealVar*)row->find("pT"); if(!vpt) continue; Hmc->Fill(vpt->getVal()); }
    }
    
    // Fill target from fit results: fitresult_pT_<min>_<max>_mva<val>.root
    TSystemDirectory dir("resdir_pt", resultsDir);
    TList* files = dir.GetListOfFiles();
    double chosenMVA = baseMVA;
    // First pass: detect available MVAs and choose closest to base
    std::vector<double> mvaAvail;
    if(files){ TIter it(files); while (TObject* obj = it()){ auto* f = dynamic_cast<TSystemFile*>(obj); if(!f || f->IsDirectory()) continue; std::string fn = f->GetName(); std::cmatch m; std::regex rePT("^fitresult_pT_([0-9]+p[0-9]+)_([0-9]+p[0-9]+)_mva([0-9]+p[0-9]+)\\.root$"); if(std::regex_match(fn.c_str(), m, rePT)){ std::string sM = m[3]; std::replace(sM.begin(), sM.end(), 'p', '.'); mvaAvail.push_back(atof(sM.c_str())); } } }
    if(!mvaAvail.empty()){ std::sort(mvaAvail.begin(), mvaAvail.end()); chosenMVA = *std::min_element(mvaAvail.begin(), mvaAvail.end(), [&](double a,double b){ return fabs(a-baseMVA) < fabs(b-baseMVA); }); }
    
    // Second pass: collect yields at chosenMVA
    if(files){ TIter it2(files); while (TObject* obj = it2()){ auto* f = dynamic_cast<TSystemFile*>(obj); if(!f || f->IsDirectory()) continue; std::string fn = f->GetName(); std::cmatch m; std::regex rePT(Form("^fitresult_pT_([0-9]+p[0-9]+)_([0-9]+p[0-9]+)_mva%sp%sp%sp$","[0-9]+","[0-9]+","[0-9]+")); // not used; fallback to manual parse
        std::regex re("^fitresult_pT_([0-9]+p[0-9]+)_([0-9]+p[0-9]+)_mva([0-9]+p[0-9]+)\\.root$");
        if(!std::regex_match(fn.c_str(), m, re)) continue; std::string sMin=m[1], sMax=m[2], sMva=m[3]; std::replace(sMin.begin(),sMin.end(),'p','.'); std::replace(sMax.begin(),sMax.end(),'p','.'); std::replace(sMva.begin(),sMva.end(),'p','.'); double vmin=atof(sMin.c_str()); double vmax=atof(sMax.c_str()); double mva=atof(sMva.c_str()); if (fabs(mva - chosenMVA) > 1e-6) continue; std::string full = std::string(resultsDir) + "/" + fn; TFile* tf = TFile::Open(full.c_str(), "READ"); if(!tf||tf->IsZombie()){ if(tf) tf->Close(); continue; } auto* fr = dynamic_cast<RooFitResult*>(tf->Get("fitResult")); if(!fr){ tf->Close(); continue; } double ns=0, nse=0; if(!getParamValErr(fr,"nsig",ns,nse)){ tf->Close(); continue; } tf->Close(); int ib = Htg->FindBin(0.5*(vmin+vmax)); Htg->SetBinContent(ib, ns); Htg->SetBinError(ib, nse); }
    }
    
    // Ratio
    auto* Hw = (TH1D*)Htg->Clone("H_weight_pt");
    Hw->SetDirectory(0);
    Hw->SetTitle(";p_{T} [GeV/c];Weight");
    Hw->Divide(Hmc);
    // Optional smoothing: 3-bin moving average (keep edges)
    if (smooth){ std::vector<double> tmp(Hw->GetNbinsX()+2); for (int i=1;i<=Hw->GetNbinsX();++i){ double a=Hw->GetBinContent(std::max(1,i-1)); double b=Hw->GetBinContent(i); double c=Hw->GetBinContent(std::min(Hw->GetNbinsX(),i+1)); tmp[i] = (a+b+c)/3.0; } for (int i=1;i<=Hw->GetNbinsX();++i) Hw->SetBinContent(i, tmp[i]); }
    // Clip and fix under/overflow
    int nb = Hw->GetNbinsX(); for (int i=1;i<=nb;++i){ double w = Hw->GetBinContent(i); if (!std::isfinite(w) || w<=0) w = 1.0; if (w < wmin) w = wmin; if (w > wmax) w = wmax; Hw->SetBinContent(i, w); Hw->SetBinError(i, 0.0); }
    Hw->SetBinContent(0, Hw->GetBinContent(1)); Hw->SetBinContent(nb+1, Hw->GetBinContent(nb));
    delete Hmc; delete Htg;
    return Hw;
  }
  
  // Save correction map to ROOT file
  bool SaveCorrectionMap(const std::string& filename, const std::map<double, std::vector<std::pair<Bin,double>>>& corrMap, const std::vector<double>& mvaList) {
    TFile* file = TFile::Open(filename.c_str(), "RECREATE");
    if (!file || file->IsZombie()) {
      std::cerr << "ERROR: Cannot create correction map file: " << filename << std::endl;
      if (file) file->Close();
      return false;
    }
    
    for (double mvaThr : mvaList) {
      if (corrMap.find(mvaThr) == corrMap.end()) continue;
      const auto& vecCF = corrMap.at(mvaThr);
      int n = vecCF.size();
      if (n == 0) continue;
      
      auto* gr = new TGraphErrors(n);
      for (int i = 0; i < n; ++i) {
        double x = vecCF[i].first.center();
        double ex = 0.5 * (vecCF[i].first.max - vecCF[i].first.min);
        double y = vecCF[i].second;
        gr->SetPoint(i, x, y);
        gr->SetPointError(i, ex, 0.0);
      }
      
      std::string graphName = Form("corrFactor_mva_%.3f", mvaThr);
      graphName = replaceAll(graphName, '.', '_');
      gr->Write(graphName.c_str());
    }
    
    file->Close();
    std::cout << "✅ Saved correction map to: " << filename << std::endl;
    return true;
  }
  
  // Load correction map from ROOT file
  bool LoadCorrectionMap(const std::string& filename, std::map<double, std::vector<std::pair<Bin,double>>>& corrMap, std::vector<double>& mvaList) {
    TFile* file = TFile::Open(filename.c_str(), "READ");
    if (!file || file->IsZombie()) {
      std::cout << "⚠️  Correction map file not found: " << filename << std::endl;
      if (file) file->Close();
      return false;
    }
    
    std::cout << "📂 Loading correction map from: " << filename << std::endl;
    
    corrMap.clear();
    mvaList.clear();
    
    TIter keyIter(file->GetListOfKeys());
    TKey* key = nullptr;
    std::map<double, std::vector<std::pair<Bin,double>>> tempMap;
    
    while ((key = (TKey*) keyIter())) {
      std::string keyName = key->GetName();
      if (keyName.find("corrFactor_mva_") != 0) continue;
      
      TGraphErrors* gr = (TGraphErrors*) file->Get(keyName.c_str());
      if (!gr) continue;
      
      // Parse MVA threshold from name
      std::string mvaStr = keyName.substr(15); // Remove "corrFactor_mva_"
      mvaStr = replaceAll(mvaStr, '_', '.');
      double mvaThr = atof(mvaStr.c_str());
      
      std::vector<std::pair<Bin,double>> vecCF;
      int n = gr->GetN();
      for (int i = 0; i < n; ++i) {
        double x, y;
        gr->GetPoint(i, x, y);
        double ex = gr->GetErrorX(i);
        vecCF.push_back({{x - ex, x + ex}, y});
      }
      
      tempMap[mvaThr] = vecCF;
      mvaList.push_back(mvaThr);
    }
    
    if (mvaList.empty()) {
      std::cerr << "ERROR: No correction factors found in file" << std::endl;
      file->Close();
      return false;
    }
    
    std::sort(mvaList.begin(), mvaList.end());
    for (double mva : mvaList) {
      corrMap[mva] = tempMap[mva];
    }
    
    file->Close();
    std::cout << "✅ Loaded " << mvaList.size() << " MVA thresholds from correction map" << std::endl;
    return true;
  }
}

void CorrectYieldsWithMVA(int varTypeInt=0, const char* resultsDir="results/data_fits", int doNormPlots=1){
  gStyle->SetOptStat(0);
  VarType vtype = static_cast<VarType>(varTypeInt);
  std::string varName = VarName(vtype);

  // 1) Load data fit yields per MVA and var-bin
  TSystemDirectory dir("resdir", resultsDir);
  TList* files = dir.GetListOfFiles();
  if(!files){ std::cerr << "No files in resultsDir: " << resultsDir << std::endl; return; }

  std::regex re(Form("^fitresult_%s_([0-9]+p[0-9]+)_([0-9]+p[0-9]+)_mva([0-9]+p[0-9]+)\\.root$", varName.c_str()));
  std::map<double, std::vector<std::tuple<Bin,double,double>>> yieldsMap; // mva -> (bin, yield, err)
  std::vector<Bin> allBins;

  TIter next(files);
  while (TObject* obj = next()){
    auto* f = dynamic_cast<TSystemFile*>(obj); if(!f) continue; if(f->IsDirectory()) continue;
    const char* fname = f->GetName(); std::cmatch m;
    if(!std::regex_match(fname, m, re)) continue;
    std::string sMin = m[1], sMax = m[2], sMva = m[3];
    sMin = replaceAll(sMin,'p','.'); sMax = replaceAll(sMax,'p','.'); sMva = replaceAll(sMva,'p','.') ;
    double vmin = atof(sMin.c_str()); double vmax = atof(sMax.c_str()); double mva = atof(sMva.c_str());

    std::string fullPath = std::string(resultsDir) + "/" + fname;
    TFile* tf = TFile::Open(fullPath.c_str(), "READ"); if(!tf || tf->IsZombie()){ if(tf) tf->Close(); continue; }
    auto* fitRes = dynamic_cast<RooFitResult*>(tf->Get("fitResult")); if(!fitRes){ tf->Close(); continue; }
    double nsig=0, nsigErr=0; if(!getParamValErr(fitRes, "nsig", nsig, nsigErr)){ tf->Close(); continue; }
    tf->Close();

    Bin bin{vmin, vmax};
    yieldsMap[mva].push_back({bin, nsig, nsigErr});
    allBins.push_back(bin);
  }
  if(yieldsMap.empty()){ std::cerr << "No fitresult files matched for var=" << varName << std::endl; return; }

  // Unique and sort bins
  std::sort(allBins.begin(), allBins.end(), [](auto a, auto b){ return a.min<b.min || (a.min==b.min && a.max<b.max); });
  allBins.erase(std::unique(allBins.begin(), allBins.end(), [](auto a, auto b){ return a.min==b.min && a.max==b.max; }), allBins.end());

  for(auto& kv : yieldsMap){ auto& vec=kv.second; std::sort(vec.begin(), vec.end(), [](const auto& A, const auto& B){ return std::get<0>(A).min < std::get<0>(B).min; }); }

  // 2) Build correction factors from MC reco-level with matchGEN==1
  SimpleDatasetManager dsm("/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/datasets.json");
  auto mcInfo = dsm.GetDataset("PbPb_MC");
  TFile* fmc = TFile::Open(mcInfo.file.c_str(), "READ"); if(!fmc||fmc->IsZombie()){ std::cerr<<"Cannot open MC file"<<std::endl; return; }
  auto* rds = dynamic_cast<RooDataSet*>(fmc->Get(mcInfo.dataset_name.c_str())); if(!rds){ std::cerr<<"Cannot get MC RooDataSet"<<std::endl; fmc->Close(); return; }

  // === BUILD GEN-pT WEIGHT HISTOGRAM (w = H_target/H_mc_gen) ===
  std::vector<double> mvaList; for(const auto& kv: yieldsMap) mvaList.push_back(kv.first); std::sort(mvaList.begin(), mvaList.end());
  TH1D* H_w = BuildGenPTWeightHist(resultsDir, rds, 0.990, 0.0, 1e9, false);
  if (!H_w) { std::cerr << "Failed to build gen-pT weight histogram. Falling back to w=1." << std::endl; }
  else {
    gSystem->mkdir("results/aggregated", true);
    TCanvas* cW = new TCanvas("cGenPTW","Gen-pT reweighting function", 800, 600);
    H_w->SetLineWidth(2);
    H_w->SetMarkerStyle(20);
    H_w->Draw("E1");
    cW->SaveAs("results/aggregated/genpt_weight.png");
    cW->SaveAs("results/aggregated/genpt_weight.pdf");
    cW->SaveAs("results/aggregated/genpt_weight.root");
    delete cW;
  }
  
  // === COMMENTED OUT: PLOT REWEIGHTING DISTRIBUTION FOR EACH MVA ===
  /*
  int nBins = allBins.size();
  std::vector<double> binCenters(nBins), binErrors(nBins);
  for (int i = 0; i < nBins; ++i) {
    binCenters[i] = allBins[i].center();
    binErrors[i] = 0.5 * (allBins[i].max - allBins[i].min);
  }
  
  for (double mvaThr : mvaList) {
    const auto& vecRW = reweightMap[mvaThr];
    
    TCanvas* cRW = new TCanvas("cRW", Form("Reweighting Factors (MVA %.3f)", mvaThr), 900, 700);
    cRW->SetLeftMargin(0.12); cRW->SetBottomMargin(0.05);
    
    // Create pads with 7:3 ratio (top:bottom)
    TPad* padTop = new TPad("padTop", "Top pad", 0, 0.30, 1, 1);
    TPad* padBot = new TPad("padBot", "Bottom pad", 0, 0, 1, 0.30);
    padTop->SetLeftMargin(0.12); padTop->SetRightMargin(0.05); padTop->SetBottomMargin(0.02); padTop->SetTopMargin(0.08);
    padBot->SetLeftMargin(0.12); padBot->SetRightMargin(0.05); padBot->SetBottomMargin(0.25); padBot->SetTopMargin(0.02);
    padTop->Draw(); padBot->Draw();
    
    std::vector<double> mcCounts(nBins), dataCounts(nBins), ratios(nBins);
    
    for (int i = 0; i < nBins; ++i) {
      const auto& bin = allBins[i];
      
      // Get MC count
      std::string varCut;
      if (vtype == VarType::PT) varCut = Form("pT > %f && pT <= %f", bin.min, bin.max);
      else if (vtype == VarType::RAPIDITY) varCut = Form("abs(y) > %f && abs(y) <= %f", bin.min, bin.max);
      else if (vtype == VarType::CENTRALITY) varCut = Form("centrality > %f && centrality <= %f", bin.min, bin.max);
      else varCut = "";
      
      std::string denCut = Form("matchGEN==1 && (%s)", varCut.c_str());
      std::unique_ptr<RooDataSet> mcBinData((RooDataSet*)rds->reduce(denCut.c_str()));
      mcCounts[i] = mcBinData ? mcBinData->numEntries() : 0.0;
      
      // Get Data yield
      dataCounts[i] = 0.0;
      if (yieldsMap.count(mvaThr) > 0) {
        for (const auto& tp : yieldsMap[mvaThr]) {
          if (std::get<0>(tp).min == bin.min && std::get<0>(tp).max == bin.max) {
            dataCounts[i] = std::get<1>(tp);
            break;
          }
        }
      }
      
      ratios[i] = (mcCounts[i] > 0) ? (dataCounts[i] / mcCounts[i]) : 1.0;
    }
    
    // TOP PAD: Data and MC yields
    padTop->cd();
    auto* grMC = new TGraphErrors(nBins, binCenters.data(), mcCounts.data(), binErrors.data(), nullptr);
    auto* grData = new TGraphErrors(nBins, binCenters.data(), dataCounts.data(), binErrors.data(), nullptr);
    
    grMC->SetMarkerStyle(20); grMC->SetMarkerSize(1.0); grMC->SetLineWidth(2);
    grMC->SetMarkerColor(kRed+1); grMC->SetLineColor(kRed+1);
    
    grData->SetMarkerStyle(21); grData->SetMarkerSize(1.0); grData->SetLineWidth(2);
    grData->SetMarkerColor(kBlue+1); grData->SetLineColor(kBlue+1);
    
    double yMinTop = std::min(*std::min_element(mcCounts.begin(), mcCounts.end()),
                                *std::min_element(dataCounts.begin(), dataCounts.end())) * 0.7;
    double yMaxTop = std::max(*std::max_element(mcCounts.begin(), mcCounts.end()),
                                *std::max_element(dataCounts.begin(), dataCounts.end())) * 1.3;
    
    TH1D* hframeTop = new TH1D("hframeTop", "", 100, allBins.front().min, allBins.back().max);
    hframeTop->GetYaxis()->SetTitle("Yield (Events)");
    hframeTop->SetMinimum(yMinTop); hframeTop->SetMaximum(yMaxTop);
    hframeTop->GetXaxis()->SetLabelSize(0);
    
    hframeTop->Draw("AXIS");
    grMC->Draw("P L SAME");
    grData->Draw("P L SAME");
    
    TLegend* legTop = new TLegend(0.60, 0.65, 0.88, 0.88);
    legTop->SetBorderSize(0); legTop->SetFillStyle(0); legTop->SetTextSize(0.045);
    legTop->AddEntry(grData, "Data (fitting)", "pl");
    legTop->AddEntry(grMC, "MC (genmatched)", "pl");
    legTop->Draw();
    
    TLatex txTop; txTop.SetNDC(); txTop.SetTextSize(0.05);
    txTop.DrawLatex(0.15, 0.92, Form("Reweighting Factors (MVA > %.3f)", mvaThr));
    
    // BOTTOM PAD: Ratio
    padBot->cd();
    auto* grRatio = new TGraphErrors(nBins, binCenters.data(), ratios.data(), binErrors.data(), nullptr);
    grRatio->SetMarkerStyle(20); grRatio->SetMarkerSize(1.0); grRatio->SetLineWidth(2);
    grRatio->SetMarkerColor(kGreen+2); grRatio->SetLineColor(kGreen+2);
    
    TH1D* hframeBot = new TH1D("hframeBot", "", 100, allBins.front().min, allBins.back().max);
    hframeBot->GetXaxis()->SetTitle(VarLabel(vtype));
    hframeBot->GetYaxis()->SetTitle("Data/MC");
    hframeBot->SetMinimum(*std::min_element(ratios.begin(), ratios.end()) * 0.8);
    hframeBot->SetMaximum(*std::max_element(ratios.begin(), ratios.end()) * 1.2);
    hframeBot->GetYaxis()->SetLabelSize(0.1); hframeBot->GetXaxis()->SetLabelSize(0.1);
    hframeBot->GetYaxis()->SetTitleSize(0.12); hframeBot->GetXaxis()->SetTitleSize(0.12);
    
    hframeBot->Draw("AXIS");
    grRatio->Draw("P L SAME");
    TLine lineRatio(allBins.front().min, 1.0, allBins.back().max, 1.0);
    lineRatio.SetLineStyle(2); lineRatio.SetLineColor(kGray+2); lineRatio.SetLineWidth(2);
    lineRatio.Draw();
    
    std::string rwPlotBase = Form("results/aggregated/reweighting_factor_vs_%s_mva_%.3f", VarName(vtype), mvaThr);
    rwPlotBase = replaceAll(rwPlotBase, '.', '_');
    cRW->SaveAs((rwPlotBase + ".png").c_str());
    cRW->SaveAs((rwPlotBase + ".pdf").c_str());
    
    delete cRW;
  }
  */

  // Compute eff(var|mva) = N(matchGEN && mva>thr && var within bin) / N(matchGEN && var within bin)
  std::map<double, std::vector<std::pair<Bin,double>>> corrMap; // mva -> [(bin, CF)]

  // Try to load correction map from file
  std::string corrMapFile = Form("results/aggregated/correctionMap_%s.root", VarName(vtype));
  gSystem->mkdir("results/aggregated", true);
  
  bool corrMapLoaded = LoadCorrectionMap(corrMapFile, corrMap, mvaList);
  
  if (!corrMapLoaded) {
    std::cout << "🔄 Computing correction factors from MC..." << std::endl;
    
    // Recompute mvaList from corrMap since we might have modified it
    mvaList.clear();
    for(const auto& kv: yieldsMap) mvaList.push_back(kv.first);
    std::sort(mvaList.begin(), mvaList.end());

    for(double mvaThr : mvaList){
      std::vector<std::pair<Bin,double>> vecCF;
      for(const auto& bin : allBins){
        std::string varCut;
        if(vtype==VarType::PT) varCut = Form("pT > %f && pT <= %f", bin.min, bin.max);
        else if(vtype==VarType::RAPIDITY) varCut = Form("abs(y) > %f && abs(y) <= %f", bin.min, bin.max);
        else if(vtype==VarType::CENTRALITY) varCut = Form("centrality > %f && centrality <= %f", bin.min, bin.max);
        else varCut = "";

        std::string denCut = Form("matchGEN==1 && (%s)", varCut.c_str());
        std::string numCut = Form("matchGEN==1 && mva > %.3f && (%s)", mvaThr, varCut.c_str());

        std::unique_ptr<RooDataSet> den((RooDataSet*)rds->reduce(denCut.c_str()));
        std::unique_ptr<RooDataSet> num((RooDataSet*)rds->reduce(numCut.c_str()));
        
        // Calculate weighted sums using gen-pT weights from H_w (evaluate on reco pT of gen-matched)
        auto weightFromPt = [&](double pt){ if (!H_w) return 1.0; int ib = H_w->FindBin(pt); if (ib<1) ib=1; if (ib>H_w->GetNbinsX()) ib=H_w->GetNbinsX(); double w = H_w->GetBinContent(ib); if (!std::isfinite(w) || w<=0) w=1.0; return w; };
        double nDenWeighted = 0.0;
        if (den && den->numEntries() > 0) {
          for (int i = 0; i < den->numEntries(); ++i) {
            const RooArgSet* row = den->get(i); if (!row) continue; auto* vpt = (RooRealVar*)row->find("pT"); double pt = vpt? vpt->getVal() : 0.0; nDenWeighted += weightFromPt(pt);
          }
        }
        
        double nNumWeighted = 0.0;
        if (num && num->numEntries() > 0) {
          for (int i = 0; i < num->numEntries(); ++i) {
            const RooArgSet* row = num->get(i); if (!row) continue; auto* vpt = (RooRealVar*)row->find("pT"); double pt = vpt? vpt->getVal() : 0.0; nNumWeighted += weightFromPt(pt);
          }
        }
        
        // Weighted efficiency
        double effWeighted = (nDenWeighted > 0) ? (nNumWeighted / nDenWeighted) : 0.0;
        
        // Use weighted efficiency for correction factor
        double cf = (effWeighted > 0) ? (1.0 / effWeighted) : 0.0;
        vecCF.push_back({bin, cf});
      }
      corrMap[mvaThr] = vecCF;
    }
    
    // Save computed correction map
    SaveCorrectionMap(corrMapFile, corrMap, mvaList);
  }
  fmc->Close();

  // === PLOT ALL CORRECTION FACTORS IN 2x5 GRID ===
  gSystem->mkdir("results/aggregated", true);
  int nMVA = mvaList.size();
  int nRows = 2, nCols = (nMVA + nRows - 1) / nRows;  // 2 rows, auto cols
  if (nCols < 5) nCols = 5;  // Ensure at least 5 cols for 2x5 layout
  
  TCanvas* cCFGrid = new TCanvas("cCFGrid", "Correction Factors per MVA", 2400, 900);
  cCFGrid->Divide(nCols, nRows, 0.01, 0.01);
  
  double xMin=1e9, xMax=-1e9, yMinCF=1e9, yMaxCF=-1e9;
  for (double mvaThr : mvaList) {
    const auto& vecCF = corrMap[mvaThr];
    for (const auto& bp : vecCF) {
      xMin = std::min(xMin, bp.first.min);
      xMax = std::max(xMax, bp.first.max);
      yMinCF = std::min(yMinCF, bp.second);
      yMaxCF = std::max(yMaxCF, bp.second);
    }
  }
  
  int padIdx = 1;
  for (size_t i=0; i<mvaList.size(); ++i) {
    double mvaThr = mvaList[i];
    cCFGrid->cd(padIdx++);
    gPad->SetLeftMargin(0.10); gPad->SetRightMargin(0.02); gPad->SetBottomMargin(0.10); gPad->SetTopMargin(0.05);
    
    const auto& vecCF = corrMap[mvaThr];
    int n = vecCF.size();
    auto* gr = new TGraphErrors(n);
    for (int j=0; j<n; ++j) {
      double x = vecCF[j].first.center();
      double ex = 0.5 * (vecCF[j].first.max - vecCF[j].first.min);
      double y = vecCF[j].second;
      gr->SetPoint(j, x, y);
      gr->SetPointError(j, ex, 0.0);
    }
    gr->SetMarkerStyle(20); gr->SetMarkerSize(0.7); gr->SetLineWidth(1.5);
    gr->SetMarkerColor(kBlack); gr->SetLineColor(kBlack);
    gr->GetXaxis()->SetLimits(xMin, xMax);
    gr->SetMinimum(std::max(0.0, yMinCF*0.8)); gr->SetMaximum(yMaxCF*1.2);
    gr->GetXaxis()->SetTitle(VarLabel(vtype)); gr->GetYaxis()->SetTitle("CF");
    gr->GetXaxis()->SetTitleSize(0.08); gr->GetYaxis()->SetTitleSize(0.08);
    gr->GetXaxis()->SetLabelSize(0.07); gr->GetYaxis()->SetLabelSize(0.07);
    gr->Draw("ALP");
    
    TLatex txMVA; txMVA.SetNDC(); txMVA.SetTextSize(0.12); txMVA.SetTextFont(62);
    txMVA.DrawLatex(0.15, 0.85, Form("MVA > %.3f", mvaThr));
  }
  
  cCFGrid->SaveAs("results/aggregated/corrfactor_grid_all_mva.png");
  cCFGrid->SaveAs("results/aggregated/corrfactor_grid_all_mva.pdf");
  cCFGrid->SaveAs("results/aggregated/corrfactor_grid_all_mva.root");
  std::cout << "✅ Saved MVA grid plot: results/aggregated/corrfactor_grid_all_mva.*" << std::endl;
  delete cCFGrid;

  // 4) Apply corrections to data yields
  // Build maps for quick CF lookup by bin
  auto cfAt = [&](double mvaThr, double v){ const auto& vec = corrMap[mvaThr]; for(const auto& bp: vec){ if(v>=bp.first.min && v<=bp.first.max) return bp.second; } return 0.0; };

  // For plotting corrected yields
  TCanvas* cRaw = new TCanvas("cCorrRaw","Corrected yield vs var (raw)", 1000, 700);
  cRaw->SetLeftMargin(0.12); cRaw->SetBottomMargin(0.12);
  TCanvas* cNorm=nullptr; if(doNormPlots) { cNorm = new TCanvas("cCorrNorm","Corrected yield vs var (normalized)", 1000, 700); cNorm->SetLeftMargin(0.12); cNorm->SetBottomMargin(0.12);} 

  std::vector<TGraphErrors*> grRaw, grNorm;
  double yMaxRaw = 0.0; double xMinY=1e9, xMaxY=-1e9; int colorIdxY=0;
  for(double mvaThr : mvaList){
    auto& yvec = yieldsMap[mvaThr]; int n=yvec.size(); if(n==0) continue;
    std::vector<double> xs(n), exs(n), ys(n), eys(n), ysCorr(n), eysCorr(n);
    for(int i=0;i<n;++i){ const auto& tp=yvec[i]; Bin bin=std::get<0>(tp); double x=bin.center(); double ex=0.5*(bin.max-bin.min); double y=std::get<1>(tp); double ey=std::get<2>(tp); double cf=cfAt(mvaThr, x); double yc=y*cf; double eyc=ey*cf; xs[i]=x; exs[i]=ex; ys[i]=y; eys[i]=ey; ysCorr[i]=yc; eysCorr[i]=eyc; xMinY=std::min(xMinY, bin.min); xMaxY=std::max(xMaxY, bin.max); yMaxRaw = std::max(yMaxRaw, yc); }
    int col = colors[colorIdxY % colors.size()]; colorIdxY++;
    auto* grC = new TGraphErrors(n, xs.data(), ysCorr.data(), exs.data(), eysCorr.data()); grC->SetMarkerStyle(20); grC->SetMarkerSize(0.9); grC->SetLineWidth(2); grC->SetMarkerColor(col); grC->SetLineColor(col); grRaw.push_back(grC);

    if(doNormPlots){ double sum=0; for(double v : ysCorr) sum+=v; std::vector<double> ysn(n), eyn(n); for(int i=0;i<n;++i){ if(sum>0){ ysn[i]=ysCorr[i]/sum; eyn[i]=eysCorr[i]/sum; } else { ysn[i]=0; eyn[i]=0; } }
      auto* grN = new TGraphErrors(n, xs.data(), ysn.data(), exs.data(), eyn.data()); grN->SetMarkerStyle(20); grN->SetMarkerSize(0.9); grN->SetLineWidth(2); grN->SetMarkerColor(col); grN->SetLineColor(col); grNorm.push_back(grN);
    }
  }

  TH1D* hframeRaw = new TH1D("hframeCorrRaw","",100, xMinY, xMaxY); hframeRaw->GetXaxis()->SetTitle(VarLabel(vtype)); hframeRaw->GetYaxis()->SetTitle("Corrected yield"); hframeRaw->SetMinimum(0.0); hframeRaw->SetMaximum(yMaxRaw>0? yMaxRaw*1.2:1.0);
  cRaw->cd(); hframeRaw->Draw("AXIS"); TLegend* legR = new TLegend(0.60,0.60,0.88,0.88); legR->SetBorderSize(0); legR->SetFillStyle(0); legR->SetTextSize(0.032);
  for(size_t i=0;i<grRaw.size();++i){ grRaw[i]->Draw("P L SAME"); legR->AddEntry(grRaw[i], Form("MVA %.3f", mvaList[i]), "pl"); } legR->Draw(); TLatex txR; txR.SetNDC(); txR.SetTextSize(0.035); txR.DrawLatex(0.15,0.92, Form("Corrected yield vs %s (raw)", VarName(vtype)));
  std::string baseCorrRaw = std::string("results/aggregated/corrected_yield_vs_") + VarName(vtype) + "_raw"; cRaw->SaveAs((baseCorrRaw+".png").c_str()); cRaw->SaveAs((baseCorrRaw+".pdf").c_str());

  if(doNormPlots){ cNorm->cd(); TH1D* hframeNorm = new TH1D("hframeCorrNorm","",100,xMinY,xMaxY); hframeNorm->GetXaxis()->SetTitle(VarLabel(vtype)); hframeNorm->GetYaxis()->SetTitle("Corrected yield (normalized)"); hframeNorm->SetMinimum(0.0); hframeNorm->SetMaximum(1.2);
    hframeNorm->Draw("AXIS"); TLegend* legN = new TLegend(0.60,0.60,0.88,0.88); legN->SetBorderSize(0); legN->SetFillStyle(0); legN->SetTextSize(0.032);
    for(size_t i=0;i<grNorm.size();++i){ grNorm[i]->Draw("P L SAME"); legN->AddEntry(grNorm[i], Form("MVA %.3f", mvaList[i]), "pl"); } legN->Draw(); TLatex txN; txN.SetNDC(); txN.SetTextSize(0.035); txN.DrawLatex(0.15,0.92, Form("Corrected yield vs %s (normalized)", VarName(vtype)));
    std::string baseCorrNorm = std::string("results/aggregated/corrected_yield_vs_") + VarName(vtype) + "_norm"; cNorm->SaveAs((baseCorrNorm+".png").c_str()); cNorm->SaveAs((baseCorrNorm+".pdf").c_str());
  }

  // 5) Ratio plots to baseline MVA=0.990 for corrected yields
  double baselineTarget = 0.990;
  double baselineMVA = baselineTarget;
  bool foundBase = false;
  for(double m : mvaList){ if (fabs(m - baselineTarget) < 1e-6) { baselineMVA = m; foundBase = true; break; } }
  if(!foundBase && !mvaList.empty()) baselineMVA = mvaList.front();

  // Build baseline arrays (raw and normalized)
  std::vector<double> bX, bEX, bYC, bEYC, bYCN, bEYCN;
  {
    // Compute baseline corrected arrays
    auto& yvecB = yieldsMap[baselineMVA];
    int nB = (int)yvecB.size();
    bX.resize(nB); bEX.resize(nB); bYC.resize(nB); bEYC.resize(nB); bYCN.resize(nB); bEYCN.resize(nB);
    double sumB=0.0;
    for(int i=0;i<nB;++i){
      const auto& tp=yvecB[i]; Bin bin=std::get<0>(tp); double x=bin.center(); double ex=0.5*(bin.max-bin.min);
      double y=std::get<1>(tp); double ey=std::get<2>(tp); double cf=cfAt(baselineMVA, x);
      double yc=y*cf; double eyc=ey*cf; bX[i]=x; bEX[i]=ex; bYC[i]=yc; bEYC[i]=eyc; sumB += yc; }
    for(size_t i=0;i<bYC.size();++i){ if(sumB>0){ bYCN[i]=bYC[i]/sumB; bEYCN[i]=bEYC[i]/sumB; } else { bYCN[i]=0; bEYCN[i]=0; } }
  }

  // Raw ratio plot
  TCanvas* cRR = new TCanvas("cCorrRatioRaw","Corrected yield ratio to MVA 0.990 (raw)", 1000, 700);
  cRR->SetLeftMargin(0.12); cRR->SetBottomMargin(0.12);
  TH1D* hframeRR = new TH1D("hframeCorrRatioRaw","",100,xMinY,xMaxY);
  hframeRR->GetXaxis()->SetTitle(VarLabel(vtype)); hframeRR->GetYaxis()->SetTitle("Corrected yield ratio to MVA 0.990");
  hframeRR->SetMinimum(0.8);
  hframeRR->SetMaximum(1.2);
  std::vector<TGraphErrors*> grRRs; int ci=0;
  for(double mvaThr : mvaList){
    auto& yvec = yieldsMap[mvaThr]; int n=(int)yvec.size(); if(n==0 || n!=(int)bYC.size()) continue;
    std::vector<double> xs(n), exs(n), rat(n), erat(n);
    for(int i=0;i<n;++i){ double x=bX[i]; double ex=bEX[i]; double y=std::get<1>(yvec[i]); double ey=std::get<2>(yvec[i]); double cf=cfAt(mvaThr, x); double yc=y*cf; double eyc=ey*cf; double rb = (bYC[i]>0? yc/bYC[i] : 0.0); double erb = (bYC[i]>0? rb*sqrt((eyc>0? (eyc/yc)*(eyc/yc):0.0) + (bEYC[i]>0? (bEYC[i]/bYC[i])*(bEYC[i]/bYC[i]):0.0)) : 0.0); xs[i]=x; exs[i]=ex; rat[i]=rb; erat[i]=erb; }
    auto* gr = new TGraphErrors(n, xs.data(), rat.data(), exs.data(), erat.data()); int col = colors[ci % colors.size()]; ci++; gr->SetMarkerStyle(20); gr->SetMarkerSize(0.9); gr->SetLineWidth(2); gr->SetMarkerColor(col); gr->SetLineColor(col); grRRs.push_back(gr);
  }
  hframeRR->Draw("AXIS"); for(auto* gr: grRRs) gr->Draw("P L SAME");
  TLegend* legRR = new TLegend(0.60,0.60,0.88,0.88); legRR->SetBorderSize(0); legRR->SetFillStyle(0); legRR->SetTextSize(0.032);
  for(size_t i=0;i<grRRs.size();++i) legRR->AddEntry(grRRs[i], Form("MVA %.3f", mvaList[i]), "pl"); legRR->Draw();
  TLine line1(xMinY,1.0,xMaxY,1.0); line1.SetLineStyle(2); line1.SetLineColor(kGray+2); line1.Draw();
  TLatex txRR; txRR.SetNDC(); txRR.SetTextSize(0.035); txRR.DrawLatex(0.15,0.92, Form("Ratio to MVA 0.990 (raw)"));
  std::string baseRR = std::string("results/aggregated/corrected_yield_ratio_vs_") + VarName(vtype) + "_raw"; cRR->SaveAs((baseRR+".png").c_str()); cRR->SaveAs((baseRR+".pdf").c_str());

  // Normalized ratio plot
  if(doNormPlots){
    TCanvas* cRN = new TCanvas("cCorrRatioNorm","Corrected yield ratio to MVA 0.990 (normalized)", 1000, 700);
    cRN->SetLeftMargin(0.12); cRN->SetBottomMargin(0.12);
    TH1D* hframeRN = new TH1D("hframeCorrRatioNorm","",100,xMinY,xMaxY);
    hframeRN->GetXaxis()->SetTitle(VarLabel(vtype)); hframeRN->GetYaxis()->SetTitle("Corrected yield ratio to MVA 0.990 (norm)");
    hframeRN->SetMinimum(0.8);
    hframeRN->SetMaximum(1.2);
    std::vector<TGraphErrors*> grRNs; int cj=0;
    for(double mvaThr: mvaList){ auto& yvec = yieldsMap[mvaThr]; int n=(int)yvec.size(); if(n==0 || n!=(int)bYC.size()) continue; std::vector<double> xs(n), exs(n), ycn(n), eycn(n), rat(n), erat(n); double sum=0; std::vector<double> ysC(n), eysC(n); for(int i=0;i<n;++i){ double x=bX[i]; double y=std::get<1>(yvec[i]); double ey=std::get<2>(yvec[i]); double cf=cfAt(mvaThr, x); ysC[i]=y*cf; eysC[i]=ey*cf; sum+=ysC[i]; xs[i]=bX[i]; exs[i]=bEX[i]; } for(int i=0;i<n;++i){ if(sum>0){ ycn[i]=ysC[i]/sum; eycn[i]=eysC[i]/sum; } else { ycn[i]=0; eycn[i]=0; } double rb = (bYCN[i]>0? ycn[i]/bYCN[i] : 0.0); double erb = 0.0; rat[i]=rb; erat[i]=erb; }
      auto* gr = new TGraphErrors(n, xs.data(), rat.data(), exs.data(), erat.data()); int col=colors[cj % colors.size()]; cj++; gr->SetMarkerStyle(20); gr->SetMarkerSize(0.9); gr->SetLineWidth(2); gr->SetMarkerColor(col); gr->SetLineColor(col); grRNs.push_back(gr);
    }
    hframeRN->SetMaximum(1.2); hframeRN->Draw("AXIS"); for(auto* gr: grRNs) gr->Draw("P L SAME");
    TLegend* legRN = new TLegend(0.60,0.60,0.88,0.88); legRN->SetBorderSize(0); legRN->SetFillStyle(0); legRN->SetTextSize(0.032);
    for(size_t i=0;i<grRNs.size();++i) legRN->AddEntry(grRNs[i], Form("MVA %.3f", mvaList[i]), "pl"); legRN->Draw();
    TLine lineN(xMinY,1.0,xMaxY,1.0); lineN.SetLineStyle(2); lineN.SetLineColor(kGray+2); lineN.Draw();
    TLatex txRN; txRN.SetNDC(); txRN.SetTextSize(0.035); txRN.DrawLatex(0.15,0.92, Form("Ratio to MVA 0.990 (normalized)"));
    std::string baseRN = std::string("results/aggregated/corrected_yield_ratio_vs_") + VarName(vtype) + "_norm"; cRN->SaveAs((baseRN+".png").c_str()); cRN->SaveAs((baseRN+".pdf").c_str());
  }
}
