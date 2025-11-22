// Create mass histogram and perform fitting for kinematic bins with cosThetaEP binning
// Usage (ROOT6.24):
//   root -l -b -q 'MakeHistKinematicBin.cpp(ptMin, ptMax, yAbsMin, yAbsMax, centMin, centMax, mvaThr, "PbPb_Data_Oct29_mva0p9", 0, -1, 1, 0)'  // histogram only
//   root -l -b -q 'MakeHistKinematicBin.cpp(ptMin, ptMax, yAbsMin, yAbsMax, centMin, centMax, mvaThr, "PbPb_Data_Oct29_mva0p9", 0, -0.8, -0.6, 1)'  // histogram + MC fit + Data fit
// Notes:
//   - mass variable name: massPion
//   - centrality variable name in RooDataSet: Centrality
//   - y selection is on abs(y)
//   - dca3D threshold: 0.01 cm (prompt: dca3D < 0.01, nonprompt: dca3D >= 0.01)
//   - cosThetaEP bins: [-1, -0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4, 0.6, 0.8, 1] (10 bins for data)
//   - MC always uses cosThetaHX inclusive [-1, 1] for better statistics
//   - doFit: 0=histogram only, 1=histogram + MC fit + Data fit (tail params fixed from MC)

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

#include "TFile.h"
#include "TH1D.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TLatex.h"

#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooArgSet.h"
#include "RooFit.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooAddPdf.h"
#include "RooCrystalBall.h"
#include "RooDstD0BG.h"
#include "RooGenericPdf.h"

#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"
#include "/home/jun502s/DstarAna/DStarAnalysis/Interface/ConfigPbPb.h"
#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Test/BDT_Evaluation/BDTParameterLoader.h"
#include "KinematicBinConfig.h"

using namespace KinematicBinConfig;

// Mass histogram config (match existing analysis)
static const int    NBINS = 80;
static const double MASS_MIN = 0.14;
static const double MASS_MAX = 0.155;
static const double DCA_THRESHOLD = 0.01;  // DCA threshold for prompt/nonprompt separation (cm)

// Use DCAMode from KinematicBinConfig
// Note: KinematicBinConfig provides enum with: INCLUSIVE=0, PROMPT_RICH=1, NONPROMPT_RICH=2

// Combined kinematic selection: pT, |y|, Centrality, cos(EP), optional DCA-based prompt/nonprompt, with optional fitting
void MakeHistKinematicBin(double ptMin, double ptMax,
                          double yAbsMin, double yAbsMax,
                          double centMin, double centMax,
                          double mvaThr,
                          const char* datasetKey = "PbPb_Data_EPtransformation",
                          int dcaModeInt = 0,
                          double cosEPMin = -1.0,
                          double cosEPMax = 1.0,
                          int doFit = 1,
                          const char* outDir = "results/reduced_hist")
{
  using namespace RooFit;

  // Convert int to enum
  int mode = dcaModeInt;
  try {
    if (dcaModeInt < 0 || dcaModeInt > 2) {
      throw std::out_of_range("dcaModeInt");
    }
  } catch (...) {
    std::cerr << "[ERROR] Invalid dcaModeInt: " << dcaModeInt 
              << " (must be 0=inclusive, 1=prompt_rich, 2=nonprompt_rich)" << std::endl;
    return;
  }

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
  
  // Add cos(EP) selection
  {
    std::ostringstream oss; oss << std::fixed << std::setprecision(6)
      << "cosThetaEP > " << cosEPMin << " && cosThetaEP <= " << cosEPMax;
    cuts.push_back(oss.str());
  }
  
  // Add DCA-based selection based on mode
  if (mode == PROMPT_RICH) {  // PROMPT_RICH = 1
    std::ostringstream oss; oss << std::fixed << std::setprecision(6) << DCA_THRESHOLD;
    cuts.push_back(std::string("dca3D < ") + oss.str());
  } else if (mode == NONPROMPT_RICH) {  // NONPROMPT_RICH = 2
    std::ostringstream oss; oss << std::fixed << std::setprecision(6) << DCA_THRESHOLD;
    cuts.push_back(std::string("dca3D >= ") + oss.str());
  }
  // INCLUSIVE (mode == 0) has no DCA cut
  
  // Add ConfigPbPb selection cuts
  cuts.push_back(SelectionCuts::getDaughterCuts());
  cuts.push_back(SelectionCuts::getGrandDaughterCuts());

  std::ostringstream sel;
  for (size_t i=0;i<cuts.size();++i) {
    if (i) sel << " && ";
    sel << "(" << cuts[i] << ")";
  }

  // Reduce dataset
  RooDataSet* ds_red = (RooDataSet*)ds->reduce(Cut(sel.str().c_str()));
  if (!ds_red || ds_red->numEntries() == 0) {
    std::cerr << "[WARN] Empty dataset after cuts. Cut: " << sel.str() << std::endl;
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
    fin->Close();
    return;
  }

  // Create histogram
  RooDataHist binned("binned", "binned", RooArgSet(massPion), mapped);
  TH1D* h = (TH1D*)binned.createHistogram("h_massPion", massPion);
  if (!h) {
    std::cerr << "[ERROR] Failed to create histogram." << std::endl;
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
     << "_cosEP" << std::setprecision(2) << cosEPMin << "_" << cosEPMax
     << "_mva" << std::setprecision(3) << mvaThr
     << "_" << DCAModeToString(mode);  // Add mode suffix
  std::string outPath = on.str();
  for (char& c : outPath) if (c=='.') c='p';  // Convert dots to 'p' in the filename
  outPath += ".root";  // Append .root extension after the conversion

  TFile* fout = TFile::Open(outPath.c_str(), "RECREATE");
  h->SetDirectory(fout);
  h->Write("hist_massPion");
  fout->Write();

  // Store histogram info before cleanup
  int nEntries = h->Integral();
  
  // Summary (before deleting objects)
  std::cout << "\n=== Histogram Creation Summary ===" << std::endl;
  std::cout << "DCA Mode: " << DCAModeToString(mode) << " (" << dcaModeInt << ")" << std::endl;
  std::cout << "cos(EP) range: [" << cosEPMin << ", " << cosEPMax << "]" << std::endl;
  if (mode != INCLUSIVE) { // Changed from DCAMode::INCLUSIVE to INCLUSIVE
    std::cout << "DCA Threshold: " << DCA_THRESHOLD << " cm" << std::endl;
    if (mode == PROMPT_RICH) {
      std::cout << "  Condition: dca3D < " << DCA_THRESHOLD << std::endl;
    } else if (mode == NONPROMPT_RICH) {
      std::cout << "  Condition: dca3D >= " << DCA_THRESHOLD << std::endl;
    }
  }
  std::cout << "Saved histogram: " << outPath << std::endl
            << "Entries: " << nEntries << std::endl
            << "Cut: " << sel.str() << std::endl;

  // === FITTING (if doFit=1) ===
  if (doFit && nEntries > 50) {
    std::cout << "\n=== Starting Fit (MC -> Data) ===" << std::endl;
    
    // Load fit parameters from JSON
    BDTParameterLoader paramLoader;
    paramLoader.loadFromFile("/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/Test/BDT_Evaluation/bdt_fit_parameters_phenom2.json");
    
    // Create bin identifier (use pT as kinematic variable for now)
    // Note: You may need to adjust based on actual kinematic variable
    BDTBinIdentifier binId(0, ptMin, ptMax, mvaThr);  // 0=PT, 1=RAPIDITY, 2=CENTRALITY
    
    std::cout << "Loading initial parameters for MVA = " << mvaThr << std::endl;
    
    // Get initial parameters from JSON
    auto meanP = paramLoader.getParameter(binId, "signal_mean");
    auto sigmaP = paramLoader.getParameter(binId, "signal_sigma");
    auto alphaLP = paramLoader.getParameter(binId, "signal_alphaL");
    auto alphaRP = paramLoader.getParameter(binId, "signal_alphaR");
    auto nLP = paramLoader.getParameter(binId, "signal_nL");
    auto nRP = paramLoader.getParameter(binId, "signal_nR");
    
    auto mP = paramLoader.getParameter(binId, "background_m");
    auto lambdaP = paramLoader.getParameter(binId, "background_lambda");
    
    // Load matched MC dataset for comparison
    SimpleDatasetManager mcDM("/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/datasets.json");
    auto mcInfo = mcDM.GetDataset("PbPb_MC");
    if (mcInfo.file.empty()) {
      std::cerr << "[ERROR] MC dataset not found!" << std::endl;
      fin->Close();
      fout->Close();
      return;
    }
    
    TFile* mcDataFile = TFile::Open(mcInfo.file.c_str());
    if (!mcDataFile || mcDataFile->IsZombie()) {
      std::cerr << "[ERROR] Cannot open MC file!" << std::endl;
      fin->Close();
      fout->Close();
      return;
    }
    
    RooDataSet* mcDataset = (RooDataSet*)mcDataFile->Get(mcInfo.dataset_name.c_str());
    if (!mcDataset) {
      std::cerr << "[ERROR] Cannot load MC dataset!" << std::endl;
      mcDataFile->Close();
      fin->Close();
      fout->Close();
      return;
    }
    
    // Apply matchGEN==1 filter for MC signal only
    std::cout << "MC dataset entries: " << mcDataset->numEntries() << std::endl;
    RooDataSet* mcSignalOnly = (RooDataSet*)mcDataset->reduce(RooFit::Cut("matchGEN==1"));
    if (mcSignalOnly && mcSignalOnly->numEntries() > 0) {
      mcDataset = mcSignalOnly;
      std::cout << "MC dataset after matchGEN==1: " << mcDataset->numEntries() << " (signal only)" << std::endl;
    }
    
    // Build same kinematic cuts for MC (IMPORTANT: use cosThetaHX INCLUSIVE for MC)
    std::vector<std::string> mcCuts;
    {
      std::ostringstream oss; oss << std::fixed << std::setprecision(3) << mvaThr;
      mcCuts.push_back(std::string("mva > ") + oss.str());
    }
    if (ptMax > ptMin) {
      std::ostringstream oss; oss << std::fixed << std::setprecision(6)
        << "pT > " << ptMin << " && pT <= " << ptMax;
      mcCuts.push_back(oss.str());
    }
    if (yAbsMax > yAbsMin) {
      std::ostringstream oss; oss << std::fixed << std::setprecision(6)
        << "abs(y) > " << yAbsMin << " && abs(y) <= " << yAbsMax;
      mcCuts.push_back(oss.str());
    }
    // NOTE: For MC, make centrality inclusive to get more statistics for shape fitting
    // if (centMax > centMin) {
    //   std::ostringstream oss; oss << std::fixed << std::setprecision(6)
    //     << "Centrality > " << centMin << " && Centrality <= " << centMax;
    //   mcCuts.push_back(oss.str());
    // }
    {
      std::ostringstream oss; oss << std::fixed << std::setprecision(6)
        << "massPion >= " << MASS_MIN << " && massPion < " << MASS_MAX;
      mcCuts.push_back(oss.str());
    }
    // IMPORTANT: Use cosThetaHX INCLUSIVE [-1, 1] for MC to get better statistics for shape fitting
    // The cosThetaEP binning is only applied to data
    {
      std::ostringstream oss; oss << std::fixed << std::setprecision(6)
        << "cosThetaHX > -1.0 && cosThetaHX <= 1.0";
      mcCuts.push_back(oss.str());
    }
    std::cout << "MC uses cosThetaHX inclusive [-1, 1] for better statistics (Data uses cosThetaEP [" 
              << cosEPMin << ", " << cosEPMax << "])" << std::endl;
    
    switch(mode) {
      case PROMPT_RICH: { // Changed from DCAMode::PROMPT_RICH to PROMPT_RICH
        std::ostringstream oss; oss << std::fixed << std::setprecision(6) << DCA_THRESHOLD;
        mcCuts.push_back(std::string("dca3D < ") + oss.str());
        break;
      }
      case NONPROMPT_RICH: { // Changed from DCAMode::NONPROMPT_RICH to NONPROMPT_RICH
        std::ostringstream oss; oss << std::fixed << std::setprecision(6) << DCA_THRESHOLD;
        mcCuts.push_back(std::string("dca3D >= ") + oss.str());
        break;
      }
      case INCLUSIVE:
      default:
        break;
    }
    
    mcCuts.push_back(SelectionCuts::getDaughterCuts());
    mcCuts.push_back(SelectionCuts::getGrandDaughterCuts());
    
    std::ostringstream mcSelOss;
    for (size_t i=0; i<mcCuts.size(); ++i) {
      if (i) mcSelOss << " && ";
      mcSelOss << "(" << mcCuts[i] << ")";
    }
    
    std::cout << "Applying MC cuts with cosThetaHX..." << std::endl;
    RooDataSet* mcReduced = (RooDataSet*)mcDataset->reduce(RooFit::Cut(mcSelOss.str().c_str()));
    if (!mcReduced || mcReduced->numEntries() < 50) {
      std::cerr << "[WARN] Not enough MC events after cuts: " << (mcReduced ? mcReduced->numEntries() : 0) << std::endl;
      mcDataFile->Close();
      fin->Close();
      fout->Close();
      return;
    }
    
    std::cout << "MC entries after kinematic cuts: " << mcReduced->numEntries() << std::endl;
    
    // Debug: Check MC dataset structure
    std::cout << "\n[DEBUG] MC Dataset variables:" << std::endl;
    RooArgSet varsMC = mcReduced->get(0) ? *mcReduced->get(0) : RooArgSet();
    varsMC.Print();
    
    // Create histogram from MC reduced dataset
    RooRealVar mcMassPion("mcMassPion", "#Delta M(D*-D^{0})", MASS_MIN, MASS_MAX, "GeV/c^{2}");
    mcMassPion.setBins(NBINS);
    
    RooDataSet mcMapped("mcMapped", "mcMapped", RooArgSet(mcMassPion));
    int mcMappedCount = 0;
    int mcNotFound = 0;
    int mcOutOfRange = 0;
    
    for (int i=0; i<mcReduced->numEntries(); ++i) {
      const RooArgSet* row = mcReduced->get(i);
      if (!row) {
        std::cerr << "[DEBUG] MC row " << i << " is null" << std::endl;
        continue;
      }
      auto* mp = (RooRealVar*)row->find("massPion");
      if (!mp) {
        std::cerr << "[DEBUG] MC row " << i << " - massPion not found" << std::endl;
        mcNotFound++;
        continue;
      }
      double v = mp->getVal();
      if (v >= MASS_MIN && v < MASS_MAX) {
        mcMassPion.setVal(v);
        mcMapped.add(RooArgSet(mcMassPion));
        mcMappedCount++;
        if (i < 5) std::cout << "[DEBUG] MC row " << i << " - massPion = " << v << " ✓" << std::endl;
      } else {
        mcOutOfRange++;
        if (i < 5) std::cout << "[DEBUG] MC row " << i << " - massPion = " << v << " (out of range)" << std::endl;
      }
    }
    
    std::cout << "\nMC Mapping Summary:" << std::endl;
    std::cout << "  Total entries: " << mcReduced->numEntries() << std::endl;
    std::cout << "  Mapped: " << mcMappedCount << std::endl;
    std::cout << "  Not found: " << mcNotFound << std::endl;
    std::cout << "  Out of range: " << mcOutOfRange << std::endl;
    std::cout << "  MC mcMapped entries: " << mcMapped.numEntries() << std::endl;
    
    RooArgSet mcObs(mcMassPion);
    RooDataHist mcRooHist("mcRooHist", "mcRooHist", mcObs, mcMapped);
    std::cout << "  MC mcRooHist numEntries: " << mcRooHist.numEntries() << std::endl;
    
    // Create RooDataHist from data histogram
    RooDataHist rooHist("rooHist", "rooHist", massPion, RooFit::Import(*h));
    
    // === STEP 1: MC FIT (signal-only, all parameters float) ===
    std::cout << "\n--- Step 1: MC Signal-only Fit ---" << std::endl;
    
    // Initialize MC signal parameters with JSON defaults
    RooRealVar mcMeanCB("mcMeanCB", "mean", 0.1455, 0.145, 0.146);
    RooRealVar mcSigmaCB("mcSigmaCB", "sigma", 0.0005, 0.0001, 0.01);
    RooRealVar mcAlphaL("mcAlphaL", "alphaL", 1.1, 0.1, 5.0);
    RooRealVar mcNL("mcNL", "nL", 2.0, 1.0, 100.0);
    RooRealVar mcAlphaR("mcAlphaR", "alphaR", 1.1, 0.1, 5.0);
    RooRealVar mcNR("mcNR", "nR", 2.5, 1.0, 100.0);
    
    // NOTE: For MC signal-only fit, ALL parameters float to extract shape
    // They will be fixed in the Data fit
    
    RooCrystalBall mcSignalPdf("mcSignalPdf", "mcSignalPdf", mcMassPion, mcMeanCB, mcSigmaCB, 
                               mcAlphaL, mcNL, mcAlphaR, mcNR);
    
    std::cout << "Initial parameters from JSON defaults (all floating for MC):" << std::endl;
    std::cout << "  mean: 0.1455 [0.145, 0.146] (float)" << std::endl;
    std::cout << "  sigma: 0.0005 [0.0001, 0.01] (float)" << std::endl;
    std::cout << "  alphaL: 1.1 [0.1, 5.0] (float)" << std::endl;
    std::cout << "  alphaR: 1.1 [0.1, 5.0] (float)" << std::endl;
    std::cout << "  nL: 2.0 [1.0, 100.0] (float)" << std::endl;
    std::cout << "  nR: 2.5 [1.0, 100.0] (float)" << std::endl;
    
    std::cout << "Performing MC signal-only fit..." << std::endl;
    RooFitResult* mcFitRes = mcSignalPdf.fitTo(mcRooHist, Save(), PrintLevel(-1), 
                                               Warnings(kFALSE), Strategy(2));
    
    if (mcFitRes) {
      std::cout << "MC Fit status: " << mcFitRes->status() << std::endl;
      std::cout << "  Extracted tail parameters:" << std::endl;
      std::cout << "    mcMean = " << mcMeanCB.getVal() << " +/- " << mcMeanCB.getError() << std::endl;
      std::cout << "    mcSigma = " << mcSigmaCB.getVal() << " +/- " << mcSigmaCB.getError() << std::endl;
      std::cout << "    mcAlphaL = " << mcAlphaL.getVal() << " +/- " << mcAlphaL.getError() << std::endl;
      std::cout << "    mcAlphaR = " << mcAlphaR.getVal() << " +/- " << mcAlphaR.getError() << std::endl;
      std::cout << "    mcNL = " << mcNL.getVal() << " +/- " << mcNL.getError() << std::endl;
      std::cout << "    mcNR = " << mcNR.getVal() << " +/- " << mcNR.getError() << std::endl;
    } else {
      std::cerr << "MC fit failed!" << std::endl;
      delete mcFitRes;
      std::cout << "================================\n" << std::endl;
      fin->Close();
  fout->Close();
      mcDataFile->Close();
      return;
    }
    
    // === SAVE MC FIT RESULT ===
    if (mcFitRes) {
      std::cout << "\nSaving MC fit results..." << std::endl;
      
      // Create MC fit plot using MC's massPion variable and histogram
      RooPlot* mcFrame = mcMassPion.frame();
      mcRooHist.plotOn(mcFrame, MarkerSize(0.8), MarkerColor(kBlack), LineColor(kBlack), 
                       MarkerStyle(20), Name("mc_signal"));
      mcSignalPdf.plotOn(mcFrame, LineColor(kRed), LineWidth(3), Name("mc_fit"));
      mcSignalPdf.plotOn(mcFrame, LineStyle(kDashed), LineColor(kBlue), LineWidth(2), Name("mc_sig_comp"));
      
      mcFrame->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
      mcFrame->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
      
      // Save MC fit result to file (for reference)
      std::string mcFitResName = outPath;
      mcFitResName.replace(mcFitResName.end()-5, mcFitResName.end(), "_mc_fitResult.root");
      TFile* mcFitFile = TFile::Open(mcFitResName.c_str(), "RECREATE");
      mcFitRes->Write("fitResult");
      mcFrame->Write("massFrame");
      mcFitFile->Close();
      delete mcFitFile;
      
      std::cout << "MC fit result saved: " << mcFitResName << std::endl;
      
      delete mcFrame;
    }
    
    // Cleanup MC file
    mcDataFile->Close();
    delete mcDataFile;
    
    // === STEP 2: DATA FIT (with tail params fixed from MC) ===
    std::cout << "\n--- Step 2: Data Fit (tail params fixed from MC) ---" << std::endl;
    
    // Data signal parameters - initialized with JSON default values
    RooRealVar dataMeanCB("dataMeanCB", "mean", 0.1455, 0.145, 0.146);
    RooRealVar dataSigmaCB("dataSigmaCB", "sigma", 0.0005, 0.0001, 0.01);
    RooRealVar dataAlphaL("dataAlphaL", "alphaL", mcAlphaL.getVal(), 0.1, 5.0);
    RooRealVar dataNL("dataNL", "nL", mcNL.getVal(), 1.0, 100.0);
    RooRealVar dataAlphaR("dataAlphaR", "alphaR", mcAlphaR.getVal(), 0.1, 5.0);
    RooRealVar dataNR("dataNR", "nR", mcNR.getVal(), 1.0, 100.0);
    
    // Fix tail parameters from MC fit
    dataAlphaL.setVal(mcAlphaL.getVal());
    dataAlphaL.setConstant(true);
    dataNL.setVal(mcNL.getVal());
    dataNL.setConstant(true);
    dataAlphaR.setVal(mcAlphaR.getVal());
    dataAlphaR.setConstant(true);
    dataNR.setVal(mcNR.getVal());
    dataNR.setConstant(true);
    
    std::cout << "Fixing tail parameters from MC:" << std::endl;
    std::cout << "  alphaL = " << dataAlphaL.getVal() << " (fixed)" << std::endl;
    std::cout << "  nL = " << dataNL.getVal() << " (fixed)" << std::endl;
    std::cout << "  alphaR = " << dataAlphaR.getVal() << " (fixed)" << std::endl;
    std::cout << "  nR = " << dataNR.getVal() << " (fixed)" << std::endl;
    
    RooCrystalBall dataSignalPdf("dataSignalPdf", "dataSignalPdf", massPion, dataMeanCB, dataSigmaCB, 
                                 dataAlphaL, dataNL, dataAlphaR, dataNR);
    
    // Background: Phenomenological2 with JSON default values
    RooRealVar m_param("m_param", "m", 1.0, 0.0, 10.0);
    RooRealVar c_param("c_param", "c_lambda", -2.0, -50.0, 50.0);
    
    // Initialize and validate background parameters from JSON if available
    {
      // Try to load from JSON parameter loader
      BDTParameterLoader loader;
      loader.loadFromFile("bdt_fit_parameters_phenom2.json");
      
      // Validate and set m_param
      m_param.setRange(0.0, 10.0);
      if (m_param.getVal() < 0.0 || m_param.getVal() > 10.0) {
        m_param.setVal(1.0);
        std::cout << "m_param out of range, reset to default: 1.0" << std::endl;
      }
      
      // Validate and set c_param
      c_param.setRange(-50.0, 50.0);
      if (fabs(c_param.getVal()) > 50.0 || fabs(c_param.getVal()) < 1e-6) {
        c_param.setVal(-2.0);
        std::cout << "c_param out of range, reset to default: -2.0" << std::endl;
      }
    }
    
    std::string formula = "(@0>@1) * TMath::Power(TMath::Max(@0-@1, 1e-9), @2) * TMath::Exp(@3 * TMath::Max(@0-@1, 0.0))";
    RooArgList phenom2Args(massPion, RooFit::RooConst(0.13957), m_param, c_param);
    RooGenericPdf bkgPdf("bkgPdf", "Phenomenological2", formula.c_str(), phenom2Args);
    
    // Yields
    double nTotal = h->Integral();
    RooRealVar nsig("nsig", "nsig", nTotal*0.1, 0, nTotal);
    RooRealVar nbkg("nbkg", "nbkg", nTotal*0.9, 0, nTotal);
    
    RooAddPdf model("model", "model", RooArgList(dataSignalPdf, bkgPdf), RooArgList(nsig, nbkg));
    
    std::cout << "Performing data fit with tail params fixed..." << std::endl;
    std::cout << "Background PDF: Phenomenological2 (x-m0)^m * exp(lambda*(x-m0))" << std::endl;
    
    RooFitResult* dataFitRes = model.fitTo(rooHist, Save(), Extended(kTRUE), 
                                           PrintLevel(-1), Warnings(kFALSE), Strategy(1));
    
    if (dataFitRes) {
      std::cout << "Data Fit status: " << dataFitRes->status() << std::endl;
      std::cout << "  N_sig: " << nsig.getVal() << " +/- " << nsig.getError() << std::endl;
      std::cout << "  N_bkg: " << nbkg.getVal() << " +/- " << nbkg.getError() << std::endl;
      std::cout << "  Mean: " << dataMeanCB.getVal() << " +/- " << dataMeanCB.getError() << std::endl;
      std::cout << "  Sigma: " << dataSigmaCB.getVal() << " +/- " << dataSigmaCB.getError() << std::endl;
      std::cout << "  m (power): " << m_param.getVal() << " +/- " << m_param.getError() << std::endl;
      std::cout << "  lambda (exp slope): " << c_param.getVal() << " +/- " << c_param.getError() << std::endl;
      
      // Create and save fit frame
      RooPlot* frame = massPion.frame();
      rooHist.plotOn(frame, MarkerSize(0.6), MarkerColor(kBlack), LineColor(kBlack), Name("data"));
      model.plotOn(frame, LineColor(kBlack), LineWidth(2), Name("model"));
      model.plotOn(frame, Components(dataSignalPdf), LineStyle(kDashed), LineColor(kRed), LineWidth(2), Name("signal"));
      model.plotOn(frame, Components(bkgPdf), LineStyle(kDashed), LineColor(kBlue), LineWidth(2), Name("bkg"));
      
      frame->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
      frame->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
      
      // === COMBINED MC & DATA PLOT (four pads) ===
      std::cout << "Creating combined MC/Data fit canvas..." << std::endl;
      
      std::string fitPlotName = outPath;
      fitPlotName.replace(fitPlotName.end()-5, fitPlotName.end(), "_fit.png");
      
      TCanvas* cFit = new TCanvas("cFit", "MC + Data Fit", 2400, 600);
      cFit->Divide(4, 1, 0.005, 0.0);
      
      // Pad 1: MC spectrum
      cFit->cd(1);
      TPad* padMC = (TPad*)gPad;
      padMC->SetLeftMargin(0.12);
      padMC->SetRightMargin(0.01);
      padMC->SetTopMargin(0.05);
      padMC->SetBottomMargin(0.12);
      RooPlot* mcFrameCombined = mcMassPion.frame();
      mcRooHist.plotOn(mcFrameCombined, MarkerSize(0.8), MarkerColor(kBlack), LineColor(kBlack),
                       MarkerStyle(20), Name("mc_signal"));
      mcSignalPdf.plotOn(mcFrameCombined, LineColor(kRed), LineWidth(3), Name("mc_fit"));
      mcSignalPdf.plotOn(mcFrameCombined, LineStyle(kDashed), LineColor(kBlue), LineWidth(2), Name("mc_sig_comp"));
      mcFrameCombined->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
      mcFrameCombined->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
      mcFrameCombined->GetXaxis()->SetTitleSize(0.045);
      mcFrameCombined->GetYaxis()->SetTitleSize(0.045);
      mcFrameCombined->Draw();
      TLegend* legMC = new TLegend(0.60, 0.15, 0.88, 0.40);
      legMC->SetBorderSize(0);
      legMC->SetFillStyle(0);
      legMC->SetTextSize(0.035);
      legMC->AddEntry((TObject*)nullptr, Form("MC | MVA > %.3f", mvaThr), "");
      legMC->AddEntry(mcFrameCombined->findObject("mc_signal"), "Signal", "lep");
      legMC->AddEntry(mcFrameCombined->findObject("mc_fit"), "Fit", "l");
      legMC->AddEntry(mcFrameCombined->findObject("mc_sig_comp"), "CB shape", "l");
      legMC->Draw();
      // delete mcFrameCombined;
      
      // Pad 2: MC parameters
      cFit->cd(2);
      TPad* padMCParam = (TPad*)gPad;
      padMCParam->SetLeftMargin(0.05);
      padMCParam->SetRightMargin(0.05);
      padMCParam->SetTopMargin(0.08);
      padMCParam->SetBottomMargin(0.12);
      padMCParam->SetFillColor(kWhite);
      TLatex mcText;
      mcText.SetTextFont(42);
      mcText.SetTextAlign(12);
      double ymc = 0.95;
      const double dymc = 0.095;
      mcText.SetTextSize(0.045);
      mcText.DrawLatex(0.10, ymc, "#bf{MC}");
      ymc -= dymc;
      mcText.SetTextSize(0.038);
      mcText.DrawLatex(0.10, ymc, Form("m = %.5f", mcMeanCB.getVal())); ymc -= dymc;
      mcText.DrawLatex(0.10, ymc, Form("#sigma = %.5f", mcSigmaCB.getVal())); ymc -= dymc;
      mcText.DrawLatex(0.10, ymc, Form("#alpha_{L} = %.3f", mcAlphaL.getVal())); ymc -= dymc;
      mcText.DrawLatex(0.10, ymc, Form("n_{L} = %.2f", mcNL.getVal())); ymc -= dymc;
      mcText.DrawLatex(0.10, ymc, Form("#alpha_{R} = %.3f", mcAlphaR.getVal())); ymc -= dymc;
      mcText.DrawLatex(0.10, ymc, Form("n_{R} = %.2f", mcNR.getVal()));
      
      // Pad 3: Data spectrum
      cFit->cd(3);
      TPad* padData = (TPad*)gPad;
      padData->SetLeftMargin(0.12);
      padData->SetRightMargin(0.01);
      padData->SetTopMargin(0.05);
      padData->SetBottomMargin(0.12);
      frame->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
      frame->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
      frame->GetXaxis()->SetTitleSize(0.045);
      frame->GetYaxis()->SetTitleSize(0.045);
      frame->Draw();
      TLegend* legData = new TLegend(0.50, 0.60, 0.88, 0.90);
      legData->SetBorderSize(0);
      legData->SetFillStyle(0);
      legData->SetTextSize(0.035);
      legData->AddEntry((TObject*)nullptr, Form("Data | MVA > %.3f", mvaThr), "");
      legData->AddEntry((TObject*)nullptr, Form("cent: [%.0f, %.0f]%%", centMin, centMax), "");
      legData->AddEntry((TObject*)nullptr, Form("cosEP: [%.2f, %.2f]", cosEPMin, cosEPMax), "");
      legData->AddEntry(frame->findObject("data"), "Data", "lep");
      legData->AddEntry(frame->findObject("model"), "Fit", "l");
      legData->AddEntry(frame->findObject("signal"), "Signal", "l");
      legData->AddEntry(frame->findObject("bkg"), "Background", "l");
      legData->Draw();
      
      // Pad 4: Data parameters
      cFit->cd(4);
      TPad* padDataParam = (TPad*)gPad;
      padDataParam->SetLeftMargin(0.05);
      padDataParam->SetRightMargin(0.05);
      padDataParam->SetTopMargin(0.08);
      padDataParam->SetBottomMargin(0.12);
      padDataParam->SetFillColor(kWhite);
      TLatex dataText;
      dataText.SetTextFont(42);
      dataText.SetTextAlign(12);
      double ydata = 0.95;
      const double dydata = 0.095;
      dataText.SetTextSize(0.045);
      dataText.DrawLatex(0.10, ydata, "#bf{Data}");
      ydata -= dydata;
      dataText.SetTextSize(0.038);
      dataText.DrawLatex(0.10, ydata, Form("N_{sig} = %.0f", nsig.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.10, ydata, Form("N_{bkg} = %.0f", nbkg.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.10, ydata, Form("m = %.5f", dataMeanCB.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.10, ydata, Form("#sigma = %.5f", dataSigmaCB.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.10, ydata, Form("#alpha_{L} = %.3f (f)", dataAlphaL.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.10, ydata, Form("n_{L} = %.2f (f)", dataNL.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.10, ydata, Form("#alpha_{R} = %.3f (f)", dataAlphaR.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.10, ydata, Form("n_{R} = %.2f (f)", dataNR.getVal())); ydata = 0.95-dydata;
      dataText.DrawLatex(0.40, ydata, Form("m_{pow} = %.3f", m_param.getVal())); ydata -= dydata;
      dataText.DrawLatex(0.40, ydata, Form("#lambda = %.3f", c_param.getVal()));
      
      cFit->SaveAs(fitPlotName.c_str());
      
      // Save fit result
      std::string fitResName = outPath;
      fitResName.replace(fitResName.end()-5, fitResName.end(), "_fitResult.root");
      TFile* fitFile = TFile::Open(fitResName.c_str(), "RECREATE");
      dataFitRes->Write("fitResult");
      frame->Write("massFrame");
      fitFile->Close();
      delete fitFile;
      
      std::cout << "Data fit plot saved: " << fitPlotName << std::endl;
      delete frame;
      delete cFit;
      delete dataFitRes;
    } else {
      std::cerr << "Data fit failed!" << std::endl;
    }
    
    delete mcFitRes;
  }

  std::cout << "================================\n" << std::endl;

  fin->Close();
  fout->Close();
}
