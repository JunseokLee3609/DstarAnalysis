// DCA fitting with kinematic-inclusive templates and kinematic-binned data
// Templates: Built from MC with all kinematic ranges combined (inclusive)
// Data: Fitted in individual kinematic bins
//
// Usage:
//   root -l -b -q 'FitDCAKinematicBins.cpp("PbPb_Data_EPtransformation", "PbPb_MC", 0)'
// 
// Arguments:
//   datasetKey: Dataset key for data (e.g., "PbPb_Data_EPtransformation")
//   mcDatasetKey: Dataset key for MC (e.g., "PbPb_MC")
//   varTypeInt: 0=pT (default), 1=rapidity, 2=centrality

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>

#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TLatex.h"

#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooArgSet.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooMsgService.h"

#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"
#include "KinematicBinConfig.h"

using namespace RooFit;
using namespace KinematicBinConfig;

static const double DCA_MIN = 0.0;
static const double DCA_MAX = 0.05;
static const int DCA_BINS = 50;
static const double DCA_THRESHOLD = 0.035;  // Prompt/nonprompt separation threshold (cm)

std::string replaceAll(std::string s, char from, char to) {
  for(char& c: s) { if(c==from) c=to; }
  return s;
}

std::string formatVal(double v, int prec) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(prec) << v;
  return oss.str();
}

std::string formatValP(double v, int prec) {
  std::string s = formatVal(v, prec);
  std::replace(s.begin(), s.end(), '.', 'p');
  return s;
}

// Build inclusive DCA templates from MC (all kinematic ranges combined)
bool BuildInclusiveDCATemplates(
    RooDataSet* mcDataset,
    RooRealVar* dcaVar,
    RooHistPdf*& promptPdf,
    RooHistPdf*& nonpromptPdf) {
  
  std::cout << "\n=== Building Kinematic-Inclusive DCA Templates from MC ===" << std::endl;
  
  if (!mcDataset || !dcaVar) {
    std::cerr << "ERROR: MC dataset or DCA variable is null" << std::endl;
    return false;
  }
  
  // Reduce MC to matched GEN (inclusive of all kinematics)
  std::string matchCut = "matchGEN==1";
  RooDataSet* mcMatched = (RooDataSet*)mcDataset->reduce(Cut(matchCut.c_str()));
  if (!mcMatched || mcMatched->numEntries() == 0) {
    std::cerr << "ERROR: No matched MC entries found" << std::endl;
    if (mcMatched) delete mcMatched;
    return false;
  }
  
  std::cout << "MC matched entries (all kinematics): " << mcMatched->numEntries() << std::endl;
  
  // Prompt template: dca3D <= threshold
  std::string promptCut = Form("dca3D <= %f", DCA_THRESHOLD);
  RooDataSet* mcPrompt = (RooDataSet*)mcMatched->reduce(Cut(promptCut.c_str()));
  
  // Nonprompt template: dca3D > threshold
  std::string nonpromptCut = Form("dca3D > %f", DCA_THRESHOLD);
  RooDataSet* mcNonprompt = (RooDataSet*)mcMatched->reduce(Cut(nonpromptCut.c_str()));
  
  std::cout << "  Prompt (dca3D <= " << DCA_THRESHOLD << "): " << (mcPrompt ? mcPrompt->numEntries() : 0) << " entries" << std::endl;
  std::cout << "  Nonprompt (dca3D > " << DCA_THRESHOLD << "): " << (mcNonprompt ? mcNonprompt->numEntries() : 0) << " entries" << std::endl;
  
  if (!mcPrompt || mcPrompt->numEntries() == 0 || !mcNonprompt || mcNonprompt->numEntries() == 0) {
    std::cerr << "ERROR: Prompt or nonprompt template is empty" << std::endl;
    if (mcPrompt) delete mcPrompt;
    if (mcNonprompt) delete mcNonprompt;
    if (mcMatched) delete mcMatched;
    return false;
  }
  
  // Create histograms and PDFs
  RooDataHist promptHist("promptHist", "Prompt DCA template", *dcaVar, *mcPrompt);
  RooDataHist nonpromptHist("nonpromptHist", "Nonprompt DCA template", *dcaVar, *mcNonprompt);
  
  promptPdf = new RooHistPdf("promptPdf", "Prompt PDF from MC (Blue)", *dcaVar, promptHist, 0);
  nonpromptPdf = new RooHistPdf("nonpromptPdf", "Nonprompt PDF from MC (Red)", *dcaVar, nonpromptHist, 0);
  
  std::cout << "✓ Kinematic-inclusive DCA templates created successfully" << std::endl;
  
  if (mcPrompt) delete mcPrompt;
  if (mcNonprompt) delete mcNonprompt;
  if (mcMatched) delete mcMatched;
  
  return true;
}

// Fit data in a single kinematic bin
void FitDataInKinematicBin(
    RooDataSet* dataDataset,
    RooRealVar* dcaVar,
    RooHistPdf* promptPdf,
    RooHistPdf* nonpromptPdf,
    double varMin,
    double varMax,
    const std::string& varName,
    const std::string& outDir) {
  
  std::cout << "\n=== Fitting " << varName << " bin [" << varMin << ", " << varMax << "] ===" << std::endl;
  
  if (!dataDataset || !dcaVar || !promptPdf || !nonpromptPdf) {
    std::cerr << "ERROR: Invalid inputs for fitting" << std::endl;
    return;
  }
  
  // Build kinematic cut based on variable name
  std::string kinCut;
  if (varName == "pT") {
    kinCut = Form("pT > %f && pT <= %f", varMin, varMax);
  } else if (varName == "y") {
    kinCut = Form("abs(y) > %f && abs(y) <= %f", varMin, varMax);
  } else if (varName == "cent") {
    kinCut = Form("Centrality > %f && Centrality <= %f", varMin, varMax);
  }
  
  // Reduce data to kinematic bin
  RooDataSet* dataBinned = (RooDataSet*)dataDataset->reduce(Cut(kinCut.c_str()));
  if (!dataBinned || dataBinned->numEntries() == 0) {
    std::cout << "WARNING: No data entries in bin [" << varMin << ", " << varMax << "]" << std::endl;
    if (dataBinned) delete dataBinned;
    return;
  }
  
  std::cout << "Data entries in this bin: " << dataBinned->numEntries() << std::endl;
  
  // Create histogram with variable bin widths
  TH1D* hData = new TH1D("hData", "", nDCABins, dcaBinEdges);
  
  // Fill histogram from dataset
  for (int i = 0; i < dataBinned->numEntries(); ++i) {
    const RooArgSet* row = dataBinned->get(i);
    if (!row) continue;
    auto* dcaPtr = (RooRealVar*)row->find("dca3D");
    if (!dcaPtr) continue;
    double dca = dcaPtr->getVal();
    hData->Fill(dca);
  }
  
  // First normalize by integral
  double integral = hData->Integral();
  if (integral > 0) {
    hData->Scale(1.0 / integral);
  }
  
  // Then normalize by bin width using ROOT's built-in function
  hData->Scale(1.0, "width");
  
  hData->SetLineColor(kBlack);
  hData->SetMarkerStyle(20);
  hData->SetMarkerColor(kBlack);
  hData->SetMarkerSize(0.8);
  hData->SetTitle(Form("DCA distribution: %s [%.2f, %.2f]", varName.c_str(), varMin, varMax));
  hData->GetXaxis()->SetTitle("DCA [cm]");
  hData->GetYaxis()->SetTitle("Normalized counts per cm");
  hData->GetXaxis()->SetLabelSize(0.04);
  hData->GetYaxis()->SetLabelSize(0.04);
  hData->GetXaxis()->SetTitleSize(0.045);
  hData->GetYaxis()->SetTitleSize(0.045);
  
  // Set y-axis range for log scale (avoid 0)
  double yMin = 0.01;
  double yMax = hData->GetMaximum() * 100;
  hData->SetMinimum(yMin);
  hData->SetMaximum(yMax);
  
  // Draw histogram
  hData->Draw("E1 P");
  
  // Add legend
  TLegend* leg = new TLegend(0.60, 0.70, 0.88, 0.88);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->SetTextSize(0.035);
  leg->AddEntry(hData, "Data", "ep");
  leg->AddEntry((TObject*)nullptr, Form("Prompt frac: %.3f #pm %.3f", fracPrompt.getVal(), fracPrompt.getError()), "");
  leg->Draw();
  
  // Save results
  gSystem->mkdir(outDir.c_str(), true);
  std::string plotPath = outDir + "/dca_fit_" + varName + "_" + formatValP(varMin, 2) + "_" + formatValP(varMax, 2);
  c->SaveAs((plotPath + ".pdf").c_str());
  c->SaveAs((plotPath + ".png").c_str());
  
  std::cout << "✓ Fit completed. Prompt fraction: " << fracPrompt.getVal() << " +/- " << fracPrompt.getError() << std::endl;
  std::cout << "  Plot saved: " << plotPath << std::endl;
  
  delete c;
  delete hData;
  delete dataBinned;
  delete fitResult;
}

void FitDCAKinematicBins(
    const char* datasetKey = "PbPb_Data_Ocr29_mva0p9",
    const char* mcDatasetKey = "PbPb_MC",
    int varTypeInt = 0) {
  
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  gStyle->SetOptStat(0);
  
  std::string outDir = "results/dca_kinematic_fits";
  gSystem->mkdir(outDir.c_str(), true);
  
  std::cout << "\n========== DCA Fitting with Kinematic Bins ==========" << std::endl;
  std::cout << "Data dataset: " << datasetKey << std::endl;
  std::cout << "MC dataset: " << mcDatasetKey << std::endl;
  
  // Load datasets
  SimpleDatasetManager dm("/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/datasets.json");
  
  auto dataInfo = dm.GetDataset(datasetKey);
  if (dataInfo.file.empty()) {
    std::cerr << "ERROR: Data dataset not found: " << datasetKey << std::endl;
    return;
  }
  
  auto mcInfo = dm.GetDataset(mcDatasetKey);
  if (mcInfo.file.empty()) {
    std::cerr << "ERROR: MC dataset not found: " << mcDatasetKey << std::endl;
    return;
  }
  
  // Open files and load datasets
  TFile* dataFile = TFile::Open(dataInfo.file.c_str(), "READ");
  if (!dataFile || dataFile->IsZombie()) {
    std::cerr << "ERROR: Cannot open data file: " << dataInfo.file << std::endl;
    return;
  }
  
  TFile* mcFile = TFile::Open(mcInfo.file.c_str(), "READ");
  if (!mcFile || mcFile->IsZombie()) {
    std::cerr << "ERROR: Cannot open MC file: " << mcInfo.file << std::endl;
    dataFile->Close();
    return;
  }
  
  RooDataSet* dataDataset = (RooDataSet*)dataFile->Get(dataInfo.dataset_name.c_str());
  if (!dataDataset) {
    std::cerr << "ERROR: Data RooDataSet not found: " << dataInfo.dataset_name << std::endl;
    dataFile->Close();
    mcFile->Close();
    return;
  }
  
  RooDataSet* mcDataset = (RooDataSet*)mcFile->Get(mcInfo.dataset_name.c_str());
  if (!mcDataset) {
    std::cerr << "ERROR: MC RooDataSet not found: " << mcInfo.dataset_name << std::endl;
    dataFile->Close();
    mcFile->Close();
    return;
  }
  
  // Create DCA variable
  RooRealVar dcaVar("dca3D", "3D DCA [cm]", DCA_MIN, DCA_MAX);
  dcaVar.setBins(DCA_BINS);
  
  // Step 1: Build kinematic-inclusive DCA templates from MC
  RooHistPdf* promptPdf = nullptr;
  RooHistPdf* nonpromptPdf = nullptr;
  
  if (!BuildInclusiveDCATemplates(mcDataset, &dcaVar, promptPdf, nonpromptPdf)) {
    std::cerr << "ERROR: Failed to build DCA templates" << std::endl;
    dataFile->Close();
    mcFile->Close();
    return;
  }
  
  // Step 2: Fit data in kinematic bins
  std::string varName;
  std::vector<std::pair<double, double>> kinBins;

  if (varTypeInt == 0) {  // pT
    varName = "pT";
    kinBins = GetPtBins();
  } else if (varTypeInt == 1) {  // Rapidity
    varName = "y";
    kinBins = GetYBins();
  } else if (varTypeInt == 2) {  // Centrality
    varName = "cent";
    kinBins = GetCentBins();
  } else {
    std::cerr << "ERROR: Invalid varTypeInt: " << varTypeInt << std::endl;
    dataFile->Close();
    mcFile->Close();
    return;
  }
  
  std::cout << "\nFitting " << kinBins.size() << " " << varName << " bins..." << std::endl;
  
  for (const auto& bin : kinBins) {
    FitDataInKinematicBin(dataDataset, &dcaVar, promptPdf, nonpromptPdf, 
                          bin.first, bin.second, varName, outDir);
  }
  
  std::cout << "\n✓ All fits completed" << std::endl;
  std::cout << "Results saved in: " << outDir << std::endl;
  
  // Cleanup
  delete promptPdf;
  delete nonpromptPdf;
  dataFile->Close();
  mcFile->Close();
}
