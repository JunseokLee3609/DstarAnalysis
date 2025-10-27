// Fit a single kinematic bin for all MVA thresholds (0.990 - 0.999)
// Supports MC and Data with cascading dataset reduction
// 
// MC WORKFLOW:
// Step 1: Generate MC histograms
//   root -l -b -q 'FitSingleBin.cpp(0, ptmin, ptmax, 0, 1, 0)'
// 
// Step 2: Fit MC histograms (extract tail parameters)
//   root -l -b -q 'FitSingleBin.cpp(0, ptmin, ptmax, 0, 0, 1)'
//
// DATA WORKFLOW:
// Step 1: Generate Data histograms
//   root -l -b -q 'FitSingleBin.cpp(0, ptmin, ptmax, 1, 1, 0)'
// 
// Step 2: Fit Data histograms (tail params fixed from MC)
//   root -l -b -q 'FitSingleBin.cpp(0, ptmin, ptmax, 1, 0, 1)'
//
// Usage: 
//   FitSingleBin(kinVarInt, varMin, varMax, dataType, generateHistograms, doFit)
//   kinVarInt: 0=pT, 1=rapidity, 2=centrality
//   dataType: 0=MC, 1=Data (default=1)
//   generateHistograms: 1=create histograms from dataset, 0=load from cache (default=1)
//   doFit: 1=perform fitting, 0=skip fitting (default=1)

#include <iostream>
#include <fstream>
#include <vector>
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TSystem.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooCrystalBall.h"
#include "RooGenericPdf.h"
#include "RooMsgService.h"
#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"
#include "BDTParameterLoader.h"

// Histogram binning configuration
const int NBINS = 80;  // Number of bins for mass histogram
const double MASS_MIN = 0.14;
const double MASS_MAX = 0.155;

enum class KinematicVar {
    PT = 0,
    RAPIDITY = 1,
    CENTRALITY = 2
};

std::string GetVarName(KinematicVar var) {
    switch(var) {
        case KinematicVar::PT: return "pT";
        case KinematicVar::RAPIDITY: return "y";
        case KinematicVar::CENTRALITY: return "centrality";
        default: return "unknown";
    }
}

std::string GetVarLabel(KinematicVar var) {
    switch(var) {
        case KinematicVar::PT: return "p_{T} [GeV/c]";
        case KinematicVar::RAPIDITY: return "|y|";
        case KinematicVar::CENTRALITY: return "Centrality [%]";
        default: return "Unknown";
    }
}

void FitSingleBin(int kinVarInt, double varMin, double varMax, int dataType = 1, int generateHistograms = 1, int doFit = 1) {
    
    using namespace RooFit;
    
    // Suppress all RooFit messages
    RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
    
    KinematicVar kinVar = static_cast<KinematicVar>(kinVarInt);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    
    std::string varName = GetVarName(kinVar);
    std::string varLabel = GetVarLabel(kinVar);
    std::string dataTypeStr = (dataType == 0) ? "MC" : "Data";
    
    std::cout << "\n===========================================" << std::endl;
    std::cout << "=== BDT Evaluation: " << dataTypeStr << " Analysis ===" << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << "Variable: " << varName << " [" << varMin << ", " << varMax << "]" << std::endl;
    std::cout << "Data Type: " << dataTypeStr << std::endl;
    std::cout << "Histogram binning: " << NBINS << " bins from " << MASS_MIN << " to " << MASS_MAX << " GeV" << std::endl;
    std::cout << "Mode: " << (generateHistograms ? "GENERATE" : "LOAD") << " histograms, " 
              << (doFit ? "PERFORM" : "SKIP") << " fitting" << std::endl;
    
    // MVA thresholds to scan - ASCENDING ORDER for cascading reduction
    // Process from lowest MVA to highest, reusing reduced dataset
    // 0.990 -> reduce 12.4M to 5.9M
    // 0.991 -> reduce 5.9M to 5.7M (reuse previous)
    // 0.999 -> reduce 1.5M to 1.4M (reuse previous)
    std::vector<double> mvaThresholds;
    for (int i = 990; i <= 999; ++i) {
        mvaThresholds.push_back(i / 1000.0);
    }
    
    std::cout << "MVA thresholds: " << mvaThresholds.size() << " points from " 
              << mvaThresholds[0] << " to " << mvaThresholds.back() 
              << " (ascending order for cascading reduction)" << std::endl;
    
    // Histogram cache file path - separate for MC and Data
    std::string histCacheDir = "results/histogram_cache";
    gSystem->mkdir(histCacheDir.c_str(), true);
    
    std::string typePrefix = (dataType == 0) ? "mc" : "data";
    std::ostringstream histFileName;
    histFileName << histCacheDir << "/" << typePrefix << "_histograms_" << varName 
                 << "_" << std::fixed << std::setprecision(2) << varMin 
                 << "_" << varMax << ".root";
    std::string histFilePath = histFileName.str();
    
    std::cout << "Histogram cache: " << histFilePath << std::endl;
    
    TFile* histCacheFile = nullptr;
    TFile* dataFile = nullptr;
    RooDataSet* fullDataset = nullptr;
    std::map<double, TH1D*> cachedHistograms;
    
    // === STEP 1: GENERATE OR LOAD HISTOGRAMS ===
    if (generateHistograms) {
        std::cout << "\n=== Step 1: Loading " << dataTypeStr << " Dataset ===" << std::endl;
        
        // Load appropriate dataset based on dataType
        SimpleDatasetManager localDatasetManager("/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/datasets.json");
        
        std::string datasetKey = (dataType == 0) ? "PbPb_MC" : "PbPb_Data_EPtransformation";
        auto dataInfo = localDatasetManager.GetDataset(datasetKey);
        
        if (dataInfo.file.empty()) {
            std::cerr << "ERROR: Dataset '" << datasetKey << "' not found!" << std::endl;
            return;
        }
        
        std::cout << "Opening " << dataTypeStr << " dataset: " << dataInfo.file << std::endl;
        dataFile = TFile::Open(dataInfo.file.c_str());
        if (!dataFile || dataFile->IsZombie()) {
            std::cerr << "ERROR: Cannot open file!" << std::endl;
            return;
        }
        
        fullDataset = (RooDataSet*)dataFile->Get(dataInfo.dataset_name.c_str());
        if (!fullDataset) {
            std::cerr << "ERROR: Cannot load dataset: " << dataInfo.dataset_name << std::endl;
            dataFile->Close();
            return;
        }
        
        std::cout << "Total events in dataset: " << fullDataset->numEntries() << std::endl;
        
    } else {
        std::cout << "\n=== Step 2: Loading Histograms from Cache ===" << std::endl;
        
        histCacheFile = TFile::Open(histFilePath.c_str(), "READ");
        if (!histCacheFile || histCacheFile->IsZombie()) {
            std::cerr << "ERROR: Cannot open histogram cache file: " << histFilePath << std::endl;
            std::cerr << "Please run with generateHistograms=true first!" << std::endl;
            return;
        }
        
        std::cout << "Loading histograms from: " << histFilePath << std::endl;
        
        // Load all histograms from cache
        for (double mva : mvaThresholds) {
            std::string histName = Form("hist_mva_%.3f", mva);
            TH1D* hist = (TH1D*)histCacheFile->Get(histName.c_str());
            if (!hist) {
                std::cerr << "ERROR: Histogram '" << histName << "' not found in cache!" << std::endl;
                histCacheFile->Close();
                return;
            }
            cachedHistograms[mva] = (TH1D*)hist->Clone();
            std::cout << "  Loaded " << histName << " with " << hist->GetEntries() << " entries" << std::endl;
        }
        
        std::cout << "Successfully loaded " << cachedHistograms.size() << " histograms from cache" << std::endl;
    }
    
    // If not fitting, just save histograms and exit
    if (!doFit) {
        std::cout << "\n=== Generating Histograms Without Fitting (doFit=0) ===" << std::endl;
        
        if (!generateHistograms) {
            std::cout << "ERROR: Cannot skip fitting when loading from cache!" << std::endl;
            std::cout << "Use generateHistograms=1 with doFit=0 to generate histograms only" << std::endl;
            return;
        }
        
        // Create massPion variable
        RooRealVar massPion("massPion", "#Delta M(D*-D^{0})", MASS_MIN, MASS_MAX, "GeV/c^{2}");
        massPion.setBins(NBINS);
        
        // Build kinematic cut
        std::string kinCut;
        if (kinVar == KinematicVar::PT) {
            kinCut = Form("pT > %f && pT <= %f", varMin, varMax);
        } else if (kinVar == KinematicVar::RAPIDITY) {
            kinCut = Form("abs(y) > %f && abs(y) <= %f", varMin, varMax);
        } else if (kinVar == KinematicVar::CENTRALITY) {
            kinCut = Form("centrality > %f && centrality <= %f", varMin, varMax);
        }
        std::cout << "Kinematic cut: " << kinCut << std::endl;
        
        // Cascading reduction: start with full dataset, then reuse reduced dataset
        RooDataSet* currentDataset = fullDataset;
        
        // Process each MVA threshold
        for (size_t mvaIdx = 0; mvaIdx < mvaThresholds.size(); ++mvaIdx) {
            double mva = mvaThresholds[mvaIdx];
            
            std::cout << "\n>>> Generating histogram for MVA > " << mva << " (" << (mvaIdx+1) << "/" << mvaThresholds.size() << ")" << std::endl;
            
            // Build cut - only MVA cut (kinematic already applied to currentDataset)
            std::string mvaCut = Form("mva > %.3f", mva);
            
            // For first iteration, apply both kinematic and MVA cut
            std::string completeCut;
            if (mvaIdx == 0) {
                completeCut = Form("mva > %.3f && %s", mva, kinCut.c_str());
                std::cout << "  Full cut: " << completeCut << std::endl;
            } else {
                completeCut = mvaCut;
                std::cout << "  Incremental cut: " << completeCut << " (reusing previous reduced dataset)" << std::endl;
            }
            
            // Reduce dataset
            RooDataSet* cutDataset = (RooDataSet*)currentDataset->reduce(RooFit::Cut(completeCut.c_str()));
            
            if (!cutDataset || cutDataset->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events: " << (cutDataset ? cutDataset->numEntries() : 0) << std::endl;
                if (cutDataset) delete cutDataset;
                continue;
            }
            
            std::cout << "  Events after cut: " << cutDataset->numEntries() << std::endl;
            
            // Update currentDataset for next iteration (delete old one if not the original)
            if (mvaIdx > 0 && currentDataset != fullDataset) {
                delete currentDataset;
            }
            currentDataset = cutDataset;
            
            // Filter to mass range
            std::string massRangeCut = Form("massPion >= %f && massPion < %f", MASS_MIN, MASS_MAX);
            RooDataSet* filteredData = (RooDataSet*)currentDataset->reduce(massRangeCut.c_str());
            
            if (!filteredData || filteredData->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events in mass range: " << (filteredData ? filteredData->numEntries() : 0) << std::endl;
                if (filteredData) delete filteredData;
                continue;
            }
            
            std::cout << "  Events in delta mass range: " << filteredData->numEntries() << std::endl;
            
            // Create histogram
            RooDataSet mappedData("mappedData", "mappedData", RooArgSet(massPion));
            for (int i = 0; i < filteredData->numEntries(); ++i) {
                const RooArgSet* row = filteredData->get(i);
                RooRealVar* mpOrig = (RooRealVar*)row->find("massPion");
                if (mpOrig && mpOrig->getVal() >= MASS_MIN && mpOrig->getVal() < MASS_MAX) {
                    massPion.setVal(mpOrig->getVal());
                    mappedData.add(RooArgSet(massPion));
                }
            }
            // delete filteredData;
            
            std::cout << "  Events after mapping: " << mappedData.numEntries() << std::endl;
            
            // Create and store histogram
            RooDataHist binnedData("binnedData", "binnedData", RooArgSet(massPion), mappedData);
            TH1D* hist = (TH1D*)binnedData.createHistogram("rawHist", massPion);
            cachedHistograms[mva] = (TH1D*)hist->Clone(Form("hist_mva_%.3f", mva));
        }
        
        // Save histograms to cache
        std::cout << "\n=== Saving Histogram Cache ===" << std::endl;
        std::cout << "Saving " << cachedHistograms.size() << " histograms to cache..." << std::endl;
        TFile* cacheFile = TFile::Open(histFilePath.c_str(), "RECREATE");
        if (cacheFile && !cacheFile->IsZombie()) {
            for (const auto& pair : cachedHistograms) {
                TH1D* hist = pair.second;
                hist->Write();
                std::cout << "  Saved " << hist->GetName() << " (" << hist->GetEntries() << " entries)" << std::endl;
            }
            cacheFile->Close();
            std::cout << "Histogram cache saved: " << histFilePath << std::endl;
        } else {
            std::cerr << "ERROR: Cannot create histogram cache file!" << std::endl;
        }
        
        if (dataFile) dataFile->Close();
        if (histCacheFile) histCacheFile->Close();
        
        std::cout << "\n===========================================" << std::endl;
        std::cout << "=== Histogram Generation Complete ===" << std::endl;
        std::cout << "===========================================" << std::endl;
        return;
    }
    
    // === STEP 2: PERFORM FITTING ===
    std::cout << "\n=== Step 2: Performing Fits ===" << std::endl;
    
    // Load fit parameters from JSON
    BDTParameterLoader paramLoader;
    paramLoader.loadFromFile("bdt_fit_parameters.json");
    
    std::cout << "\n=== Fit Parameters (MVA-specific tuning enabled) ===" << std::endl;
    std::cout << "Each MVA threshold uses independently tuned parameters" << std::endl;
    
    // Create new massPion variable with proper range and bins for fitting
    RooRealVar massPion("massPion", "#Delta M(D*-D^{0})", MASS_MIN, MASS_MAX, "GeV/c^{2}");
    massPion.setBins(NBINS);
    
    // Build kinematic cut (only needed for generateHistograms mode)
    std::string kinCut;
    if (kinVar == KinematicVar::PT) {
        kinCut = Form("pT > %f && pT <= %f", varMin, varMax);
    } else if (kinVar == KinematicVar::RAPIDITY) {
        kinCut = Form("abs(y) > %f && abs(y) <= %f", varMin, varMax);
    } else if (kinVar == KinematicVar::CENTRALITY) {
        kinCut = Form("centrality > %f && centrality <= %f", varMin, varMax);
    }
    
    if (generateHistograms) {
        std::cout << "Kinematic cut: " << kinCut << std::endl;
    }
    std::cout << "===========================================" << std::endl;
    
    // Create output directories - separate for MC and Data
    gSystem->mkdir("results", true);
    std::string fitDir = (dataType == 0) ? "results/mc_fits" : "results/data_fits";
    std::string histDir = (dataType == 0) ? "results/mc_histograms" : "results/data_histograms";
    std::string rawHistDir = (dataType == 0) ? "results/mc_raw_histograms" : "results/data_raw_histograms";
    
    gSystem->mkdir(fitDir.c_str(), true);
    gSystem->mkdir(histDir.c_str(), true);
    gSystem->mkdir(rawHistDir.c_str(), true);
    
    // Remove the old multi-canvas approach
    // Individual canvases will be created for each MVA window
    
    // Storage for yields
    std::vector<double> yields, yield_errors;
    std::vector<int> colors = {kRed, kBlue, kGreen+2, kMagenta, kCyan+2, 
                               kOrange+1, kViolet, kTeal, kPink+2, kSpring};
    
    // Loop over MVA thresholds
    for (size_t mvaIdx = 0; mvaIdx < mvaThresholds.size(); ++mvaIdx) {
        double mva = mvaThresholds[mvaIdx];
        int color = colors[mvaIdx % colors.size()];
        
        std::cout << "\n>>> Processing MVA > " << std::fixed << std::setprecision(3) << mva 
                  << " (" << mvaIdx << "/" << mvaThresholds.size() << ")" << std::endl;
        
        // Create bin identifier with MVA threshold
        BDTBinIdentifier binId(kinVarInt, varMin, varMax, mva);
        
        // Print matched parameters for this MVA
        paramLoader.printMatchedBinForMVA(binId, mva);
        
        TH1D* rawHist = nullptr;
        RooDataHist* binnedData = nullptr;
        
        if (generateHistograms) {
            // === GENERATE MODE: Process dataset ===
            // Build complete cut (cumulative: mva > threshold)
            std::string completeCut = Form("mva > %.3f && %s", mva, kinCut.c_str());
            
            // Reduce dataset
            RooDataSet* cutDataset = (RooDataSet*)fullDataset->reduce(RooFit::Cut(completeCut.c_str()));
            
            if (!cutDataset || cutDataset->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events: " << (cutDataset ? cutDataset->numEntries() : 0) << std::endl;
                yields.push_back(0);
                yield_errors.push_back(0);
                if (cutDataset) delete cutDataset;
                continue;
            }
            
            std::cout << "  Events after cut: " << cutDataset->numEntries() << std::endl;
            
            // Filter dataset to mass range
            std::string massRangeCut = Form("massPion >= %f && massPion < %f", MASS_MIN, MASS_MAX);
            RooDataSet* filteredData = (RooDataSet*)cutDataset->reduce(massRangeCut.c_str());
            delete cutDataset;
            
            if (!filteredData || filteredData->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events in mass range: " 
                          << (filteredData ? filteredData->numEntries() : 0) << std::endl;
                yields.push_back(0);
                yield_errors.push_back(0);
                if (filteredData) delete filteredData;
                continue;
            }
            
            std::cout << "  Events in delta mass range: " << filteredData->numEntries() << std::endl;
            
            // Create new dataset with massPion mapped to our new variable
            RooDataSet mappedData("mappedData", "mappedData", RooArgSet(massPion));
            for (int i = 0; i < filteredData->numEntries(); ++i) {
                const RooArgSet* row = filteredData->get(i);
                RooRealVar* mpOrig = (RooRealVar*)row->find("massPion");
                if (mpOrig && mpOrig->getVal() >= MASS_MIN && mpOrig->getVal() < MASS_MAX) {
                    massPion.setVal(mpOrig->getVal());
                    mappedData.add(RooArgSet(massPion));
                }
            }
            delete filteredData;
            
            std::cout << "  Events after mapping: " << mappedData.numEntries() << std::endl;
            
            // Create binned histogram
            binnedData = new RooDataHist("binnedData", "binnedData", RooArgSet(massPion), mappedData);
            
            // Create TH1D for saving (this will be owned by ROOT)
            rawHist = (TH1D*)binnedData->createHistogram("rawHist", massPion);
            
            // Store a SEPARATE clone in cache map for later saving (deep copy, detached from binnedData)
            TH1D* cacheHist = (TH1D*)rawHist->Clone(Form("hist_mva_%.3f", mva));
            cacheHist->SetDirectory(0);  // Detach from current directory
            cachedHistograms[mva] = cacheHist;
            
        } else {
            // === LOAD MODE: Use cached histogram ===
            rawHist = cachedHistograms[mva];
            if (!rawHist) {
                std::cout << "  ERROR: Cached histogram not found for MVA " << mva << std::endl;
                yields.push_back(0);
                yield_errors.push_back(0);
                continue;
            }
            
            std::cout << "  Using cached histogram with " << rawHist->GetEntries() << " entries" << std::endl;
            
            // Create RooDataHist from cached TH1D
            binnedData = new RooDataHist("binnedData", "binnedData", RooArgList(massPion), rawHist);
            
            if (!binnedData || binnedData->numEntries() == 0) {
                std::cout << "  ERROR: Failed to create RooDataHist from histogram for MVA " << mva << std::endl;
                yields.push_back(0);
                yield_errors.push_back(0);
                if (binnedData) delete binnedData;
                continue;
            }
        }
        
        // === LOAD MVA-SPECIFIC PARAMETERS FROM JSON ===
        auto meanP = paramLoader.getParameter(binId, "signal_mean");
        auto sigmaP = paramLoader.getParameter(binId, "signal_sigma");
        auto alphaLP = paramLoader.getParameter(binId, "signal_alphaL");
        auto alphaRP = paramLoader.getParameter(binId, "signal_alphaR");
        auto nLP = paramLoader.getParameter(binId, "signal_nL");
        auto nRP = paramLoader.getParameter(binId, "signal_nR");
        auto mP = paramLoader.getParameter(binId, "background_m");
        auto lambdaP = paramLoader.getParameter(binId, "background_lambda");
        auto p0P = paramLoader.getParameter(binId, "background_p0");
        auto p1P = paramLoader.getParameter(binId, "background_p1");
        
        // === SIGNAL PDF: DBCrystalBall from JSON parameters ===
        RooRealVar meanCB("meanCB", "mean", meanP.value, meanP.min, meanP.max);
        RooRealVar sigmaCB("sigmaCB", "sigma", sigmaP.value, sigmaP.min, sigmaP.max);
        RooRealVar alphaL("alphaL", "alphaL", alphaLP.value, alphaLP.min, alphaLP.max);
        RooRealVar alphaR("alphaR", "alphaR", alphaRP.value, alphaRP.min, alphaRP.max);
        RooRealVar nL("nL", "nL", nLP.value, nLP.min, nLP.max);
        RooRealVar nR("nR", "nR", nRP.value, nRP.min, nRP.max);
        
        // Apply fixed flags from JSON
        if (meanP.fixed) meanCB.setConstant(true);
        if (sigmaP.fixed) sigmaCB.setConstant(true);
        if (alphaLP.fixed) alphaL.setConstant(true);
        if (alphaRP.fixed) alphaR.setConstant(true);
        if (nLP.fixed) nL.setConstant(true);
        if (nRP.fixed) nR.setConstant(true);
        
        // Double-sided Crystal Ball (using RooCrystalBall from RooFit)
        RooCrystalBall signalPdf("signalPdf", "signalPdf", massPion, meanCB, sigmaCB, 
                                 alphaL, nL, alphaR, nR);
        
        // === BACKGROUND PDF: RooDstD0BG from JSON parameters ===
        RooRealVar dm0("dm0", "dm0_pion_mass", 0.13957);
        dm0.setConstant(true);
        
        RooRealVar m_param("m_param", "m", mP.value, mP.min, mP.max);
        RooRealVar c_param("c_param", "c", lambdaP.value, lambdaP.min, lambdaP.max);
        RooRealVar a_param("a_param", "a", p0P.value, p0P.min, p0P.max);
        RooRealVar b_param("b_param", "b", p1P.value, p1P.min, p1P.max);
        
        // Apply fixed flags from JSON
        if (mP.fixed) m_param.setConstant(true);
        if (lambdaP.fixed) c_param.setConstant(true);
        if (p0P.fixed) a_param.setConstant(true);
        if (p1P.fixed) b_param.setConstant(true);
        
        RooDstD0BG bkgPdf("bkgPdf", "bkgPdf", massPion, dm0, c_param, a_param, b_param);
        
        // === COMBINED PDF ===
        // Get yield ratios from parameter file (default 0.5 if not specified)
        auto nsigRatioP = paramLoader.getParameter(binId, "yield_nsig_ratio");
        auto nbkgRatioP = paramLoader.getParameter(binId, "yield_nbkg_ratio");
        double nsigRatio = (nsigRatioP.value > 0) ? nsigRatioP.value : 0.5;
        double nbkgRatio = (nbkgRatioP.value > 0) ? nbkgRatioP.value : 0.5;
        
        double nTotal = binnedData->sumEntries();
        RooRealVar nsig("nsig", "nsig", nTotal*nsigRatio, 0, nTotal*1);
        RooRealVar nbkg("nbkg", "nbkg", nTotal*nbkgRatio, 0, nTotal*1);
        RooAddPdf model("model", "model", RooArgList(signalPdf, bkgPdf), RooArgList(nsig, nbkg));
        
        // === FIT ===
        std::cout << "  Performing binned fit..." << std::endl;
        RooFitResult* fitRes = model.fitTo(*binnedData, Save(), PrintLevel(-1), Warnings(kFALSE), Strategy(2));
        
        int fitStatus = fitRes ? fitRes->status() : -1;
        std::cout << "  Fit status: " << fitStatus << std::endl;
        std::cout << "  N_sig: " << nsig.getVal() << " +/- " << nsig.getError() << std::endl;
        std::cout << "  N_bkg: " << nbkg.getVal() << " +/- " << nbkg.getError() << std::endl;
        
        // Store yield
        if (fitStatus == 0 && nsig.getVal() > 0) {
            yields.push_back(nsig.getVal());
            yield_errors.push_back(nsig.getError());
        } else {
            yields.push_back(0);
            yield_errors.push_back(0);
        }
        
        // === DRAW RAW HISTOGRAM ===
        // Create a new canvas for each MVA window
        TCanvas* cRaw = new TCanvas(Form("cRaw_%zu", mvaIdx), Form("Raw Mass Distribution for MVA > %.3f", mva), 800, 600);
        cRaw->SetLeftMargin(0.14);
        cRaw->SetBottomMargin(0.12);
        
        rawHist->SetLineColor(color);
        rawHist->SetLineWidth(2);
        rawHist->SetMarkerStyle(20);
        rawHist->SetMarkerSize(0.5);
        rawHist->SetMarkerColor(color);
        rawHist->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
        rawHist->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
        rawHist->GetXaxis()->SetTitleSize(0.045);
        rawHist->GetYaxis()->SetTitleSize(0.045);
        rawHist->Draw("E");
        
        TLatex* texRaw = new TLatex();
        texRaw->SetNDC();
        texRaw->SetTextSize(0.04);
        texRaw->DrawLatex(0.18, 0.85, Form("MVA > %.3f", mva));
        texRaw->DrawLatex(0.18, 0.80, Form("N = %.0f", rawHist->GetEntries()));
        
        // === DRAW MASS FIT ===
        // Create a new canvas for each MVA window
        TCanvas* cMass = new TCanvas(Form("cMass_%zu", mvaIdx), Form("Mass Fit for MVA > %.3f", mva), 800, 600);
        cMass->SetLeftMargin(0.14);
        cMass->SetBottomMargin(0.12);
        
        // Create fresh frame for this pad to avoid overlapping plots
        RooPlot* frame = massPion.frame(Title(" "));
        
        // Plot data points (black)
        binnedData->plotOn(frame, MarkerSize(0.6), MarkerColor(kBlack), LineColor(kBlack), Name("data"));
        
        // Plot total model (black solid line)
        model.plotOn(frame, LineColor(kBlack), LineWidth(2), Name("model"));
        
        // Plot signal component (red dashed)
        model.plotOn(frame, Components(signalPdf), LineStyle(kDashed), LineColor(kRed), LineWidth(2), Name("signal"));
        
        // Plot background component (blue dashed)
        model.plotOn(frame, Components(bkgPdf), LineStyle(kDashed), LineColor(kBlue), LineWidth(2), Name("bkg"));
        
        frame->Draw();
        
        frame->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
        frame->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
        frame->GetXaxis()->SetTitleSize(0.045);
        frame->GetYaxis()->SetTitleSize(0.045);
        
        TLegend* leg = new TLegend(0.55, 0.60, 0.88, 0.88);
        leg->SetBorderSize(0);  // Remove border
        leg->SetFillColor(0);   // Transparent background
        leg->SetFillStyle(4000);  // Transparent fill style
        leg->SetTextSize(0.032);
        leg->AddEntry((TObject*)nullptr, Form("MVA > %.3f", mva), "");
        leg->AddEntry((TObject*)nullptr, Form("N_{sig}: %.0f #pm %.0f", nsig.getVal(), nsig.getError()), "");
        leg->AddEntry((TObject*)nullptr, Form("N_{bkg}: %.0f #pm %.0f", nbkg.getVal(), nbkg.getError()), "");
        leg->AddEntry(frame->findObject("model"), "Total fit", "l");
        leg->AddEntry(frame->findObject("signal"), "Signal", "l");
        leg->AddEntry(frame->findObject("bkg"), "Background", "l");
        leg->Draw();
        
        // === SAVE INDIVIDUAL FIT RESULT ===
        std::string fitFileName = Form("results/fit_results/fitresult_%s_%.2f_%.2f_mva%.3f", 
                                       varName.c_str(), varMin, varMax, mva);
        for (char& c : fitFileName) if (c == '.') c = 'p';
        fitFileName += ".root";
        
        TFile* fitFile = TFile::Open(fitFileName.c_str(), "RECREATE");
        if (fitRes) fitRes->Write("fitResult");
        frame->Write("massFrame");
        TH1D* hRawClone = (TH1D*)rawHist->Clone("rawHist");
        hRawClone->Write();
        fitFile->Close();
        
        // === SAVE INDIVIDUAL RAW HISTOGRAM ===
        std::string rawPlotName = Form("results/raw_histograms/raw_hist_%s_%.2f_%.2f_mva%.3f", 
                                       varName.c_str(), varMin, varMax, mva);
        for (char& c : rawPlotName) if (c == '.') c = 'p';
        cRaw->SaveAs((rawPlotName + ".png").c_str());
        cRaw->SaveAs((rawPlotName + ".pdf").c_str());
        cRaw->SaveAs((rawPlotName + ".root").c_str());
        std::cout << "Saved raw histogram: " << rawPlotName << ".png" << std::endl;
        delete cRaw;

        // === SAVE INDIVIDUAL MASS FIT ===
        std::string massPlotName = Form("results/mass_fits/mass_fits_%s_%.2f_%.2f_mva%.3f", 
                                       varName.c_str(), varMin, varMax, mva);
        for (char& c : massPlotName) if (c == '.') c = 'p';
        cMass->SaveAs((massPlotName + ".png").c_str());
        cMass->SaveAs((massPlotName + ".pdf").c_str());
        cMass->SaveAs((massPlotName + ".root").c_str());
        std::cout << "Saved mass fit: " << massPlotName << ".png" << std::endl;
        delete cMass;
        
    }
    
    if (dataFile) dataFile->Close();
    if (histCacheFile) histCacheFile->Close();
    
    // === SAVE HISTOGRAM CACHE (if generateHistograms mode and fitting was done) ===
    if (generateHistograms && doFit && cachedHistograms.size() > 0) {
        std::cout << "\n=== Saving Histogram Cache ===" << std::endl;
        TFile* cacheFile = TFile::Open(histFilePath.c_str(), "RECREATE");
        if (cacheFile && !cacheFile->IsZombie()) {
            for (const auto& pair : cachedHistograms) {
                double mva = pair.first;
                TH1D* hist = pair.second;
                hist->Write();
                std::cout << "  Saved " << hist->GetName() << " (" << hist->GetEntries() << " entries)" << std::endl;
            }
            cacheFile->Close();
            std::cout << "Histogram cache saved: " << histFilePath << std::endl;
        } else {
            std::cerr << "ERROR: Cannot create histogram cache file!" << std::endl;
        }
    }
    
    // === CREATE YIELD vs MVA PLOT ===
    std::cout << "\n=== Creating Yield vs MVA Plot ===" << std::endl;
    TCanvas* cYield = new TCanvas("cYield", "Signal Yield vs MVA", 1000, 700);
    cYield->SetLeftMargin(0.12);
    cYield->SetRightMargin(0.05);
    cYield->SetBottomMargin(0.12);
    
    TH1D* hYield = new TH1D("hYield", 
        Form("Signal Yield vs MVA;MVA threshold;N_{sig}"), 
        mvaThresholds.size(), 0, mvaThresholds.size());
    
    for (size_t i = 0; i < mvaThresholds.size(); ++i) {
        hYield->SetBinContent(i+1, yields[i]);
        hYield->SetBinError(i+1, yield_errors[i]);
        hYield->GetXaxis()->SetBinLabel(i+1, Form("%.3f", mvaThresholds[i]));
    }
    
    hYield->SetMarkerStyle(20);
    hYield->SetMarkerSize(1.2);
    hYield->SetLineWidth(2);
    hYield->SetMarkerColor(kBlack);
    hYield->SetLineColor(kBlack);
    hYield->Draw("E");
    
    TLatex* tex = new TLatex();
    tex->SetNDC();
    tex->SetTextSize(0.04);
    tex->DrawLatex(0.15, 0.85, Form("%s: [%.2f, %.2f]", varLabel.c_str(), varMin, varMax));
    
    std::string yieldPlotName = Form("results/yield_vs_mva_%s_%.2f_%.2f", 
                                     varName.c_str(), varMin, varMax);
    for (char& c : yieldPlotName) if (c == '.') c = 'p';
    cYield->SaveAs((yieldPlotName + ".png").c_str());
    cYield->SaveAs((yieldPlotName + ".pdf").c_str());
    cYield->SaveAs((yieldPlotName + ".root").c_str());
    std::cout << "Saved yield plot: " << yieldPlotName << ".png" << std::endl;
    delete cYield;
    
    // === SAVE YIELDS TO TEXT FILE ===
    std::string txtFileName = Form("results/yields_%s_%.2f_%.2f.txt", 
                                   varName.c_str(), varMin, varMax);
    for (char& c : txtFileName) if (c == '.') c = 'p';
    
    std::ofstream out(txtFileName);
    out << "# " << varName << " bin: [" << varMin << ", " << varMax << "]\n";
    out << "# Histogram binning: " << NBINS << " bins from " << MASS_MIN << " to " << MASS_MAX << " GeV\n";
    out << "# mva nsig nsig_err\n";
    for (size_t i = 0; i < mvaThresholds.size(); ++i) {
        out << mvaThresholds[i] << " " << yields[i] << " " << yield_errors[i] << "\n";
    }
    out.close();
    std::cout << "Saved yields: " << txtFileName << std::endl;
    
    std::cout << "\n===========================================" << std::endl;
    std::cout << "=== Completed Successfully ===" << std::endl;
    std::cout << "===========================================" << std::endl;
}
