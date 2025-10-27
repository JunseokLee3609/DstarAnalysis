// Create mass histogram after reducing RooDataSet by kinematic ranges and MVA
// Usage (ROOT6.24):
//   root -l -b -q 'MakeHistKinematicBin.cpp(ptMin, ptMax, yAbsMin, yAbsMax, centMin, centMax, mvaThr, "PbPb_Data_EPtransformation")'
// Notes:
//   - mass variable name: massPion
//   - centrality variable name in RooDataSet: Centrality
//   - y selection is on abs(y)

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

#include "TFile.h"
#include "TH1D.h"
#include "TSystem.h"
#include "TROOT.h"

#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooArgSet.h"
#include "RooFit.h"

#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"

// Mass histogram config (match existing analysis)
static const int    NBINS = 80;
static const double MASS_MIN = 0.14;
static const double MASS_MAX = 0.155;

// Combined kinematic selection: pT, |y|, Centrality
void MakeHistKinematicBin(double ptMin, double ptMax,
                          double yAbsMin, double yAbsMax,
                          double centMin, double centMax,
                          double mvaThr,
                          const char* datasetKey = "PbPb_Data_EPtransformation",
                          const char* outDir = "results/reduced_hist")
{
  using namespace RooFit;

  // Open dataset via SimpleDatasetManager
  SimpleDatasetManager dm("/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/datasets.json");
  auto info = dm.GetDataset(datasetKey);
  if (info.file.empty()) {
    std::cerr << "[ERROR] Dataset key not found: " << datasetKey << std::endl;
    return;
  }

  TFile* fin = TFile::Open(info.file.c_str());
  if (!fin || fin->IsZombie()) {
    std::cerr << "[ERROR] Cannot open file: " << info.file << std::endl;
    return;
  }

  RooDataSet* ds = (RooDataSet*)fin->Get(info.dataset_name.c_str());
  if (!ds) {
    std::cerr << "[ERROR] RooDataSet not found: " << info.dataset_name << std::endl;
    fin->Close();
    return;
  }

  // Build selection cuts
  std::vector<std::string> cuts;
  {
    std::ostringstream oss; oss << std::fixed << std::setprecision(3) << mvaThr;
    cuts.push_back(std::string("mva > ") + oss.str());
  }
  if (ptMax > ptMin) {
    std::ostringstream oss; oss << std::fixed << std::setprecision(6)
      << "pT > " << ptMin << " && pT <= " << ptMax;
    cuts.push_back(oss.str());
  }
  if (yAbsMax > yAbsMin) {
    std::ostringstream oss; oss << std::fixed << std::setprecision(6)
      << "abs(y) > " << yAbsMin << " && abs(y) <= " << yAbsMax;
    cuts.push_back(oss.str());
  }
  if (centMax > centMin) {
    std::ostringstream oss; oss << std::fixed << std::setprecision(6)
      << "Centrality > " << centMin << " && Centrality <= " << centMax;
    cuts.push_back(oss.str());
  }
  {
    std::ostringstream oss; oss << std::fixed << std::setprecision(6)
      << "massPion >= " << MASS_MIN << " && massPion < " << MASS_MAX;
    cuts.push_back(oss.str());
  }

  std::ostringstream sel;
  for (size_t i=0;i<cuts.size();++i) {
    if (i) sel << " && ";
    sel << "(" << cuts[i] << ")";
  }

  // Reduce dataset
  RooDataSet* ds_red = (RooDataSet*)ds->reduce(Cut(sel.str().c_str()));
  if (!ds_red || ds_red->numEntries() == 0) {
    std::cerr << "[WARN] Empty dataset after cuts. Cut: " << sel.str() << std::endl;
    if (ds_red) delete ds_red;
    fin->Close();
    return;
  }

  // Create mass variable and map only massPion into a lightweight dataset
  RooRealVar massPion("massPion", "#Delta M(D*-D^{0})", MASS_MIN, MASS_MAX, "GeV/c^{2}");
  massPion.setBins(NBINS);

  RooDataSet mapped("mapped", "mapped", RooArgSet(massPion));
  for (int i=0;i<ds_red->numEntries();++i) {
    const RooArgSet* row = ds_red->get(i);
    if (!row) continue;
    auto* mp = (RooRealVar*)row->find("massPion");
    if (!mp) continue;
    double v = mp->getVal();
    if (v >= MASS_MIN && v < MASS_MAX) {
      massPion.setVal(v);
      mapped.add(RooArgSet(massPion));
    }
  }

  if (mapped.numEntries() == 0) {
    std::cerr << "[WARN] No entries in mass window after mapping." << std::endl;
    delete ds_red;
    fin->Close();
    return;
  }

  // Create histogram
  RooDataHist binned("binned", "binned", RooArgSet(massPion), mapped);
  TH1D* h = (TH1D*)binned.createHistogram("h_massPion", massPion);
  if (!h) {
    std::cerr << "[ERROR] Failed to create histogram." << std::endl;
    delete ds_red;
    fin->Close();
    return;
  }

  // Prepare output
  gSystem->mkdir(outDir, true);
  std::ostringstream on;
  on << outDir << "/hist_" << datasetKey
     << std::fixed
     << "_pt" << std::setprecision(1) << ptMin << "_" << ptMax
     << "_y"  << std::setprecision(1) << yAbsMin << "_" << yAbsMax
     << "_cent" << std::setprecision(0) << centMin << "_" << centMax
     << "_mva" << std::setprecision(3) << mvaThr;
  std::string outPath = on.str();
  for (char& c : outPath) if (c=='.') c='p';  // Convert dots to 'p' in the filename (but not in .root extension)
  outPath += ".root";  // Append .root extension after the conversion

  TFile* fout = TFile::Open(outPath.c_str(), "RECREATE");
  h->SetDirectory(fout);
  h->Write("hist_massPion");
  fout->Write();
  fout->Close();

  // Summary
  std::cout << "Saved histogram: " << outPath
            << " | entries: " << h->GetEntries()
            << " | cut: " << sel.str() << std::endl;

  delete h;
  delete ds_red;
  fin->Close();
}
