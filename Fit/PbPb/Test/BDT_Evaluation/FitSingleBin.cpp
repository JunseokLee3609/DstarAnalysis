// Fit a single kinematic bin for all MVA thresholds (0.990 - 0.999)
// Supports MC and Data with cascading dataset reduction
// 
// UNIFIED WORKFLOW:
// Step 1: Generate histograms for both MC and Data
//   root -l -b -q 'FitSingleBin.cpp(0, ptmin, ptmax, 1, 0)'
// 
// Step 2: Fit (MC fit -> Data fit with MC params fixed)
//   root -l -b -q 'FitSingleBin.cpp(0, ptmin, ptmax, 0, 1)'
//
// Step 3: Generate and fit in one go
//   root -l -b -q 'FitSingleBin.cpp(0, ptmin, ptmax, 1, 1)'
//
// Usage: 
//   FitSingleBin(kinVarInt, varMin, varMax, generateHistograms, doFit, dcaMode, dcaCut)
//   kinVarInt: 0=pT, 1=rapidity, 2=centrality
//   generateHistograms: 1=create histograms from datasets, 0=load from cache (default=1)
//   doFit: 1=perform fitting (MC then Data), 0=skip fitting (default=1)
//   dcaMode: 0=inclusive, 1=prompt (dca3D <= dcaCut), 2=nonprompt (dca3D > dcaCut); ignored if dcaCut<0
//   dcaCut: threshold value for dca3D (e.g., 0.01); set <0 for inclusive

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
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
#include "RooDstD0BG.h"
#include "RooMsgService.h"
#include "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/SimpleDatasetManager.h"
#include "BDTKinematicConfig.h"

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

void FitSingleBin(int kinVarInt, double varMin, double varMax, int generateHistograms = 1, int doFit = 1, int dcaMode = 0, double dcaCut = 0.01) {
    
    using namespace RooFit;
    
    // Suppress all RooFit messages
    // RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
    
    KinematicVar kinVar = static_cast<KinematicVar>(kinVarInt);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    
    std::string varName = GetVarName(kinVar);
    std::string varLabel = GetVarLabel(kinVar);
    
    std::cout << "\n===========================================" << std::endl;
    std::cout << "=== BDT Evaluation: MC & Data Analysis ===" << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << "Variable: " << varName << " [" << varMin << ", " << varMax << "]" << std::endl;
    std::cout << "Histogram binning: " << NBINS << " bins from " << MASS_MIN << " to " << MASS_MAX << " GeV" << std::endl;
    std::cout << "Mode: " << (generateHistograms ? "GENERATE" : "LOAD") << " histograms, " 
              << (doFit ? "PERFORM" : "SKIP") << " fitting" << std::endl;
    std::string catLabel = "incl";
    if (dcaCut >= 0) {
        if (dcaMode == 1) catLabel = "prompt";
        else if (dcaMode == 2) catLabel = "nonprompt";
        std::cout << "DCA split: " << catLabel << " (threshold=" << dcaCut << ")" << std::endl;
    } else {
        std::cout << "DCA split: inclusive" << std::endl;
    }
    
    // MVA thresholds to scan - ASCENDING ORDER for cascading reduction
    // Read from BDTKinematicConfig.h
    const auto& mvaThresholdsArray = BDTKinematicConfig::GetMVAThresholds();
    std::vector<double> mvaThresholds(mvaThresholdsArray.begin(), mvaThresholdsArray.end());
    
    // Sort MVA thresholds in ascending order for cascading reduction
    std::sort(mvaThresholds.begin(), mvaThresholds.end());
    
    std::cout << "MVA thresholds (from BDTKinematicConfig): " << mvaThresholds.size() << " points" << std::endl;
    std::cout << "  Range: " << std::fixed << std::setprecision(3) 
              << mvaThresholds[0] << " to " << mvaThresholds.back() << std::endl;
    
    std::cout << "  Values: ";
    for (size_t i = 0; i < mvaThresholds.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::fixed << std::setprecision(3) << mvaThresholds[i];
    }
    std::cout << std::endl;
    
    // Histogram cache file paths - separate for MC and Data
    std::string histCacheDir = "results/histogram_cache";
    gSystem->mkdir(histCacheDir.c_str(), true);
    
    std::ostringstream mcHistFileName, dataHistFileName;
    mcHistFileName << histCacheDir << "/mc_histograms_" << varName 
                   << "_" << std::fixed << std::setprecision(2) << varMin 
                   << "_" << varMax << "_" << catLabel << ".root";
    dataHistFileName << histCacheDir << "/data_histograms_" << varName 
                     << "_" << std::fixed << std::setprecision(2) << varMin 
                     << "_" << varMax << "_" << catLabel << ".root";
    
    std::string mcHistFilePath = mcHistFileName.str();
    std::string dataHistFilePath = dataHistFileName.str();
    
    std::cout << "MC histogram cache: " << mcHistFilePath << std::endl;
    std::cout << "Data histogram cache: " << dataHistFilePath << std::endl;
    
    TFile* mcDataFile = nullptr;
    TFile* dataDataFile = nullptr;
    RooDataSet* mcDataset = nullptr;
    RooDataSet* dataDataset = nullptr;
    std::map<double, TH1D*> mcCachedHistograms;
    std::map<double, TH1D*> dataCachedHistograms;
    
    // === STEP 1: GENERATE OR LOAD HISTOGRAMS ===
    if (generateHistograms) {
        std::cout << "\n=== Step 1: Loading MC & Data Datasets ===" << std::endl;
        
        // Load datasets
        SimpleDatasetManager localDatasetManager("/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Analysis/datasets.json");
        
        // Load MC dataset
        auto mcDataInfo = localDatasetManager.GetDataset("PbPb_MC");
        if (mcDataInfo.file.empty()) {
            std::cerr << "ERROR: MC dataset not found!" << std::endl;
            return;
        }
        
        std::cout << "Opening MC dataset: " << mcDataInfo.file << std::endl;
        mcDataFile = TFile::Open(mcDataInfo.file.c_str());
        if (!mcDataFile || mcDataFile->IsZombie()) {
            std::cerr << "ERROR: Cannot open MC file!" << std::endl;
            return;
        }
        
        mcDataset = (RooDataSet*)mcDataFile->Get(mcDataInfo.dataset_name.c_str());
        if (!mcDataset) {
            std::cerr << "ERROR: Cannot load MC dataset!" << std::endl;
            mcDataFile->Close();
            return;
        }
        
        // Apply matchGEN==1 filter for MC signal only
        std::cout << "MC dataset before matchGEN filter: " << mcDataset->numEntries() << " events" << std::endl;
        RooDataSet* mcSignalOnly = (RooDataSet*)mcDataset->reduce(RooFit::Cut("matchGEN==1"));
        if (mcSignalOnly && mcSignalOnly->numEntries() > 0) {
            delete mcDataset;
            mcDataset = mcSignalOnly;
            std::cout << "MC dataset after matchGEN==1 filter: " << mcDataset->numEntries() << " events (signal only)" << std::endl;
        } else {
            std::cerr << "ERROR: No signal events found with matchGEN==1" << std::endl;
            return;
        }
        
        // Load Data dataset (use real PbPb data)
        auto dataDataInfo = localDatasetManager.GetDataset("PbPb_Data_EPtransformation");
        if (dataDataInfo.file.empty()) {
            std::cerr << "ERROR: Data dataset not found!" << std::endl;
            return;
        }
        
        std::cout << "\nOpening Data dataset: " << dataDataInfo.file << std::endl;
        dataDataFile = TFile::Open(dataDataInfo.file.c_str());
        if (!dataDataFile || dataDataFile->IsZombie()) {
            std::cerr << "ERROR: Cannot open Data file!" << std::endl;
            return;
        }
        
        dataDataset = (RooDataSet*)dataDataFile->Get(dataDataInfo.dataset_name.c_str());
        if (!dataDataset) {
            std::cerr << "ERROR: Cannot load Data dataset!" << std::endl;
            dataDataFile->Close();
            return;
        }
        std::cout << "Data events: " << dataDataset->numEntries() << std::endl;
        
    } else {
        std::cout << "\n=== Step 1: Loading Histograms from Cache ===" << std::endl;
        
        // Load MC histograms
        TFile* mcHistFile = TFile::Open(mcHistFilePath.c_str(), "READ");
        if (!mcHistFile || mcHistFile->IsZombie()) {
            std::cerr << "ERROR: Cannot open MC histogram cache: " << mcHistFilePath << std::endl;
            std::cerr << "Please run with generateHistograms=1 first!" << std::endl;
            return;
        }
        
        std::cout << "Loading MC histograms from: " << mcHistFilePath << std::endl;
        for (double mva : mvaThresholds) {
            std::string histName = Form("hist_mva_%.3f", mva);
            TH1D* hist = (TH1D*)mcHistFile->Get(histName.c_str());
            if (!hist) {
                std::cerr << "ERROR: MC histogram '" << histName << "' not found!" << std::endl;
                mcHistFile->Close();
                return;
            }
            TH1D* clonedHist = (TH1D*)hist->Clone();
            clonedHist->SetDirectory(0); // Detach from file
            mcCachedHistograms[mva] = clonedHist;
        }
        mcHistFile->Close();
        std::cout << "Successfully loaded " << mcCachedHistograms.size() << " MC histograms" << std::endl;
        
        // Load Data histograms
        TFile* dataHistFile = TFile::Open(dataHistFilePath.c_str(), "READ");
        if (!dataHistFile || dataHistFile->IsZombie()) {
            std::cerr << "ERROR: Cannot open Data histogram cache: " << dataHistFilePath << std::endl;
            std::cerr << "Please run with generateHistograms=1 first!" << std::endl;
            return;
        }
        
        std::cout << "Loading Data histograms from: " << dataHistFilePath << std::endl;
        for (double mva : mvaThresholds) {
            std::string histName = Form("hist_mva_%.3f", mva);
            TH1D* hist = (TH1D*)dataHistFile->Get(histName.c_str());
            if (!hist) {
                std::cerr << "ERROR: Data histogram '" << histName << "' not found!" << std::endl;
                dataHistFile->Close();
        return;
            }
            TH1D* clonedHist = (TH1D*)hist->Clone();
            clonedHist->SetDirectory(0); // Detach from file
            dataCachedHistograms[mva] = clonedHist;
        }
        dataHistFile->Close();
        std::cout << "Successfully loaded " << dataCachedHistograms.size() << " Data histograms" << std::endl;
    }
    
    // === STEP 2: GENERATE HISTOGRAMS (if requested) ===
    if (generateHistograms) {
        std::cout << "\n=== Step 2: Generating Histograms with Cascading Reduction ===" << std::endl;
        
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
        std::string dcaCutStr;
        if (dcaCut >= 0) {
            if (dcaMode == 1) dcaCutStr = Form(" && dca3D <= %.6f", dcaCut);
            else if (dcaMode == 2) dcaCutStr = Form(" && dca3D > %.6f", dcaCut);
        }
        
        // Process MC histograms with cascading reduction
        std::cout << "\n--- Processing MC Histograms ---" << std::endl;
        RooDataSet* currentMCDataset = mcDataset;
        
        for (size_t mvaIdx = 0; mvaIdx < mvaThresholds.size(); ++mvaIdx) {
            double mva = mvaThresholds[mvaIdx];
            
            std::cout << "\n>>> MC MVA > " << mva << " (" << (mvaIdx+1) << "/" << mvaThresholds.size() << ")" << std::endl;
            
            // Build cut
            std::string completeCut;
            if (mvaIdx == 0) {
                completeCut = Form("mva > %.3f && %s%s", mva, kinCut.c_str(), dcaCutStr.c_str());
                std::cout << "  Full cut: " << completeCut << std::endl;
            } else {
                completeCut = Form("mva > %.3f%s", mva, dcaCutStr.c_str());
                std::cout << "  Incremental cut: " << completeCut << " (reusing previous dataset)" << std::endl;
            }
            
            // Reduce dataset
            RooDataSet* cutDataset = (RooDataSet*)currentMCDataset->reduce(RooFit::Cut(completeCut.c_str()));
            
            if (!cutDataset || cutDataset->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events" << std::endl;
                if (cutDataset) delete cutDataset;
                continue;
            }
            
            std::cout << "  Events after cut: " << cutDataset->numEntries() << std::endl;
            
            // Update current dataset
            if (mvaIdx > 0 && currentMCDataset != mcDataset) {
                delete currentMCDataset;
            }
            currentMCDataset = cutDataset;
            
            // Filter to mass range and create histogram
            std::string massRangeCut = Form("massPion >= %f && massPion < %f", MASS_MIN, MASS_MAX);
            RooDataSet* filteredData = (RooDataSet*)currentMCDataset->reduce(massRangeCut.c_str());
            
            if (!filteredData || filteredData->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events in mass range" << std::endl;
                if (filteredData) delete filteredData;
                continue;
            }
            
            // Create mapped dataset
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
            
            // Create histogram
            RooDataHist binnedData("binnedData", "binnedData", RooArgSet(massPion), mappedData);
            TH1D* hist = (TH1D*)binnedData.createHistogram("rawHist", massPion);
            
            TH1D* cacheHist = (TH1D*)hist->Clone(Form("hist_mva_%.3f", mva));
            cacheHist->SetDirectory(0);
            mcCachedHistograms[mva] = cacheHist;
            
            delete hist;
        }
        
        // Process Data histograms with cascading reduction
        std::cout << "\n--- Processing Data Histograms ---" << std::endl;
        RooDataSet* currentDataDataset = dataDataset;
        
        for (size_t mvaIdx = 0; mvaIdx < mvaThresholds.size(); ++mvaIdx) {
            double mva = mvaThresholds[mvaIdx];
            
            std::cout << "\n>>> Data MVA > " << mva << " (" << (mvaIdx+1) << "/" << mvaThresholds.size() << ")" << std::endl;
            
            // Build cut
            std::string completeCut;
            if (mvaIdx == 0) {
                completeCut = Form("mva > %.3f && %s%s", mva, kinCut.c_str(), dcaCutStr.c_str());
                std::cout << "  Full cut: " << completeCut << std::endl;
            } else {
                completeCut = Form("mva > %.3f%s", mva, dcaCutStr.c_str());
                std::cout << "  Incremental cut: " << completeCut << " (reusing previous dataset)" << std::endl;
            }
            
            // Reduce dataset
            RooDataSet* cutDataset = (RooDataSet*)currentDataDataset->reduce(RooFit::Cut(completeCut.c_str()));
            
            if (!cutDataset || cutDataset->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events" << std::endl;
                if (cutDataset) delete cutDataset;
                continue;
            }
            
            std::cout << "  Events after cut: " << cutDataset->numEntries() << std::endl;
            
            // Update current dataset
            if (mvaIdx > 0 && currentDataDataset != dataDataset) {
                delete currentDataDataset;
            }
            currentDataDataset = cutDataset;
            
            // Filter to mass range and create histogram
            std::string massRangeCut = Form("massPion >= %f && massPion < %f", MASS_MIN, MASS_MAX);
            RooDataSet* filteredData = (RooDataSet*)currentDataDataset->reduce(massRangeCut.c_str());
            
            if (!filteredData || filteredData->numEntries() < 50) {
                std::cout << "  WARNING: Not enough events in mass range" << std::endl;
                if (filteredData) delete filteredData;
                continue;
            }
            
            // Create mapped dataset
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
            
            // Create histogram
            RooDataHist binnedData("binnedData", "binnedData", RooArgSet(massPion), mappedData);
            TH1D* hist = (TH1D*)binnedData.createHistogram("rawHist", massPion);
            
            TH1D* cacheHist = (TH1D*)hist->Clone(Form("hist_mva_%.3f", mva));
            cacheHist->SetDirectory(0);
            dataCachedHistograms[mva] = cacheHist;
            
            delete hist;
        }
        
        // Save histograms to cache files
        std::cout << "\n=== Saving Histogram Caches ===" << std::endl;
        
        TFile* mcCacheFile = TFile::Open(mcHistFilePath.c_str(), "RECREATE");
        if (mcCacheFile && !mcCacheFile->IsZombie()) {
            for (const auto& pair : mcCachedHistograms) {
                pair.second->Write();
            }
            mcCacheFile->Close();
            std::cout << "MC histogram cache saved: " << mcHistFilePath << std::endl;
        }
        
        TFile* dataCacheFile = TFile::Open(dataHistFilePath.c_str(), "RECREATE");
        if (dataCacheFile && !dataCacheFile->IsZombie()) {
            for (const auto& pair : dataCachedHistograms) {
                pair.second->Write();
            }
            dataCacheFile->Close();
            std::cout << "Data histogram cache saved: " << dataHistFilePath << std::endl;
        }
    }
    
    // If doFit=0, stop here
    if (!doFit) {
        std::cout << "\n=== Histogram generation complete (doFit=0) ===" << std::endl;
        std::cout << "Run with doFit=1 to perform fitting" << std::endl;
        return;
    }
    
    // === STEP 3: PERFORM FITTING ===
    if (!mcCachedHistograms.empty() && !dataCachedHistograms.empty()) {
        std::cout << "\n=== Step 3: Performing Fits (MC then Data) ===" << std::endl;
    } else {
        std::cerr << "ERROR: Histograms not available for fitting!" << std::endl;
        return;
    }
    
    // Use simple uniform parameter initialization (same as MakeHistKinematicBin.cpp)
    std::cout << "\n=== Fit Parameters (uniform initialization for all bins) ===" << std::endl;
    std::cout << "Using standard parameter ranges for all MVA thresholds" << std::endl;
    
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
    
    // Create output directories
    gSystem->mkdir("results", true);
    gSystem->mkdir("results/mc_fits", true);
    gSystem->mkdir("results/data_fits", true);
    
    // Storage for yields
    std::vector<double> yields, yield_errors;
    std::vector<int> colors = {kRed, kBlue, kGreen+2, kMagenta, kCyan+2, 
                               kOrange+1, kViolet, kTeal, kPink+2, kSpring};
    
    // Loop over MVA thresholds - fit MC first, then Data
    for (size_t mvaIdx = 0; mvaIdx < mvaThresholds.size(); ++mvaIdx) {
        double mva = mvaThresholds[mvaIdx];
        int color = colors[mvaIdx % colors.size()];
        
        std::cout << "\n>>> Processing MVA > " << std::fixed << std::setprecision(3) << mva 
                  << " (" << mvaIdx << "/" << mvaThresholds.size() << ")" << std::endl;
        
        // === MC FIT ===
        std::cout << "\n--- MC Fit ---" << std::endl;
        TH1D* mcHist = mcCachedHistograms[mva];
        if (!mcHist) {
            std::cout << "  ERROR: MC histogram not found" << std::endl;
            continue;
        }
        
        std::cout << "  Using MC histogram with " << mcHist->GetEntries() << " entries" << std::endl;
        
        // Create RooDataHist from MC TH1D
        RooDataHist mcBinnedData("mcBinnedData", "mcBinnedData", massPion, RooFit::Import(*mcHist));
        
        if (mcBinnedData.numEntries() == 0) {
            std::cout << "  ERROR: Failed to create RooDataHist" << std::endl;
            continue;
        }
        
        // === INITIALIZE MC SIGNAL PARAMETERS (uniform for all bins) ===
        // Using same initialization as MakeHistKinematicBin.cpp
        RooRealVar mcMeanCB("meanCB", "mean", 0.1455, 0.145, 0.146);
        RooRealVar mcSigmaCB("sigmaCB", "sigma", 0.0005, 0.0001, 0.01);
        RooRealVar mcAlphaL("alphaL", "alphaL", 1.1, 0.1, 5.0);
        RooRealVar mcAlphaR("alphaR", "alphaR", 1.1, 0.1, 5.0);
        RooRealVar mcNL("nL", "nL", 2.0, 1.0, 100.0);
        RooRealVar mcNR("nR", "nR", 2.5, 1.0, 100.0);
        
        // MC fit: Allow ALL parameters to float in MC, ignore JSON fixed flags
        // All tail parameters (alphaL, alphaR, nL, nR) must be floating for MC
        // to properly extract their values from the data
        
        RooCrystalBall mcSignalPdf("signalPdf", "signalPdf", massPion, mcMeanCB, mcSigmaCB, 
                                   mcAlphaL, mcNL, mcAlphaR, mcNR);
        
        // Data fit: signal-only fit to extract tail parameters
        std::cout << "  Performing MC signal-only fit..." << std::endl;
        RooFitResult* mcFitRes = mcSignalPdf.fitTo(mcBinnedData, Save(), PrintLevel(-1), Warnings(kFALSE), Strategy(2));
        
        int mcFitStatus = mcFitRes ? mcFitRes->status() : -1;
        std::cout << "  MC Fit status: " << mcFitStatus << std::endl;
        std::cout << "  Extracted tail parameters:" << std::endl;
        std::cout << "    meanCB = " << mcMeanCB.getVal() << " +/- " << mcMeanCB.getError() << std::endl;
        std::cout << "    sigmaCB = " << mcSigmaCB.getVal() << " +/- " << mcSigmaCB.getError() << std::endl;
        std::cout << "    alphaL = " << mcAlphaL.getVal() << " +/- " << mcAlphaL.getError() << std::endl;
        std::cout << "    alphaR = " << mcAlphaR.getVal() << " +/- " << mcAlphaR.getError() << std::endl;
        std::cout << "    nL = " << mcNL.getVal() << " +/- " << mcNL.getError() << std::endl;
        std::cout << "    nR = " << mcNR.getVal() << " +/- " << mcNR.getError() << std::endl;
        
        // Save MC fit result
        std::string mcFitFileNameBase = Form("results/mc_fits/fitresult_%s_%.2f_%.2f_%s_mva%.3f", 
                                       varName.c_str(), varMin, varMax, catLabel.c_str(), mva);
        for (char& c : mcFitFileNameBase) if (c == '.') c = 'p';
        std::string mcFitFileName = mcFitFileNameBase + ".root";
        
        TFile* mcFitFile = TFile::Open(mcFitFileName.c_str(), "RECREATE");
        if (mcFitRes) mcFitRes->Write("fitResult");
        
        // Create MC plot frame
        RooPlot* mcFitFrame = massPion.frame(Title(" "));
        mcBinnedData.plotOn(mcFitFrame, MarkerSize(0.6), MarkerColor(kBlack), LineColor(kBlack), Name("mc_signal"));
        mcSignalPdf.plotOn(mcFitFrame, LineColor(kBlack), LineWidth(2), Name("mc_fit"));
        mcSignalPdf.plotOn(mcFitFrame, LineStyle(kDashed), LineColor(kRed), LineWidth(2), Name("mc_sig_comp"));
        mcFitFrame->Write("massFrame");
        mcFitFile->Close();
        
        std::cout << "  MC fit result saved: " << mcFitFileName << std::endl;
        
        // === DATA FIT ===
        std::cout << "\n--- Data Fit ---" << std::endl;
        TH1D* dataHist = dataCachedHistograms[mva];
        if (!dataHist) {
            std::cout << "  ERROR: Data histogram not found" << std::endl;
            yields.push_back(0);
            yield_errors.push_back(0);
            continue;
        }
        
        std::cout << "  Using Data histogram with " << dataHist->GetEntries() << " entries" << std::endl;
        
        // Create RooDataHist from Data TH1D
        RooDataHist dataBinnedData("dataBinnedData", "dataBinnedData", massPion, RooFit::Import(*dataHist));
        
        if (dataBinnedData.numEntries() == 0) {
            std::cout << "  ERROR: Failed to create RooDataHist" << std::endl;
            yields.push_back(0);
            yield_errors.push_back(0);
            continue;
        }
        
        // === INITIALIZE DATA SIGNAL PARAMETERS (uniform for all bins) ===
        // Start with same initialization as MC
        RooRealVar dataMeanCB("meanCB", "mean", 0.1455, 0.145, 0.146);
        RooRealVar dataSigmaCB("sigmaCB", "sigma", 0.0005, 0.0001, 0.01);
        RooRealVar dataAlphaL("alphaL", "alphaL", 1.1, 0.1, 5.0);
        RooRealVar dataAlphaR("alphaR", "alphaR", 1.1, 0.1, 5.0);
        RooRealVar dataNL("nL", "nL", 2.0, 1.0, 100.0);
        RooRealVar dataNR("nR", "nR", 2.5, 1.0, 100.0);
        
        // Load MC fit results and fix tail parameters (nL, nR always fixed)
        std::cout << "  Loading MC fit results to fix tail parameters..." << std::endl;
        TFile* mcFitFileLoad = TFile::Open(mcFitFileName.c_str(), "READ");
        if (mcFitFileLoad && !mcFitFileLoad->IsZombie()) {
            RooFitResult* mcFitResLoad = (RooFitResult*)mcFitFileLoad->Get("fitResult");
            if (mcFitResLoad) {
                // Fix all tail parameters (nL, nR, alphaL, alphaR) from MC fit
                RooRealVar* mcNLval = (RooRealVar*)mcFitResLoad->floatParsFinal().find("nL");
                if (mcNLval) {
                    dataNL.setVal(mcNLval->getVal());
                    dataNL.setConstant(true);
                    std::cout << "    Fixed nL = " << dataNL.getVal() << " (from MC)" << std::endl;
                }
                RooRealVar* mcNRval = (RooRealVar*)mcFitResLoad->floatParsFinal().find("nR");
                if (mcNRval) {
                    dataNR.setVal(mcNRval->getVal());
                    dataNR.setConstant(true);
                    std::cout << "    Fixed nR = " << dataNR.getVal() << " (from MC)" << std::endl;
                }
                RooRealVar* mcAlphaLval = (RooRealVar*)mcFitResLoad->floatParsFinal().find("alphaL");
                if (mcAlphaLval) {
                    dataAlphaL.setVal(mcAlphaLval->getVal());
                    dataAlphaL.setConstant(true);
                    std::cout << "    Fixed alphaL = " << dataAlphaL.getVal() << " (from MC)" << std::endl;
                }
                RooRealVar* mcAlphaRval = (RooRealVar*)mcFitResLoad->floatParsFinal().find("alphaR");
                if (mcAlphaRval) {
                    dataAlphaR.setVal(mcAlphaRval->getVal());
                    dataAlphaR.setConstant(true);
                    std::cout << "    Fixed alphaR = " << dataAlphaR.getVal() << " (from MC)" << std::endl;
                }
            }
            mcFitFileLoad->Close();
        }
        
        // Create Data signal PDF
        RooCrystalBall dataSignalPdf("signalPdf", "signalPdf", massPion, dataMeanCB, dataSigmaCB, 
                                     dataAlphaL, dataNL, dataAlphaR, dataNR);
        
        // === BACKGROUND PDF: Phenomenological2 (uniform for all bins) ===
        // (x - m_pi)^m * exp(lambda * (x - m_pi))
        std::cout << "  Background PDF type: Phenomenological2" << std::endl;
        
        RooRealVar m_param("m_param", "m", 1.0, 0.0, 10.0);
        RooRealVar c_param("c_param", "lambda", -2.0, -50.0, 50.0);
        
        // Formula: (x - m0)^m * exp(lambda * (x - m0)) with threshold at pion mass
        RooArgList phenom2Args(massPion, RooFit::RooConst(0.13957), m_param, c_param);
        std::string formula = "(@0>@1) * TMath::Power(TMath::Max(@0-@1, 1e-9), @2) * TMath::Exp(@3 * TMath::Max(@0-@1, 0.0))";
        RooGenericPdf bkgPdf("bkgPdf", "Phenomenological2", formula.c_str(), phenom2Args);
        
        // Combined model (signal + background)
        // Initialize yields based on histogram entries
        double nTotal = dataBinnedData.sumEntries();
        double nsigRatio = 0.5;  // Initial guess: 50% signal
        double nbkgRatio = 0.5;  // Initial guess: 50% background
        
        RooRealVar nsig("nsig", "nsig", nTotal*nsigRatio, 0, nTotal);
        RooRealVar nbkg("nbkg", "nbkg", nTotal*nbkgRatio, 0, nTotal);
        
        std::cout << "  Initial yield estimates: nsig = " << nsig.getVal() 
                  << ", nbkg = " << nbkg.getVal() << " (total = " << nTotal << ")" << std::endl;
        
        RooAddPdf model("model", "model", RooArgList(dataSignalPdf, bkgPdf), RooArgList(nsig, nbkg));
        
        // Data fit with Extended likelihood and no SumW2 errors
        std::cout << "  Performing Data fit with tail params fixed from MC..." << std::endl;
        RooFitResult* dataFitRes = model.fitTo(dataBinnedData, Save(), Extended(kTRUE), SumW2Error(kTRUE), PrintLevel(-1), Warnings(kTRUE), Strategy(1));
        
        int dataFitStatus = dataFitRes ? dataFitRes->status() : -1;
        std::cout << "  Data Fit status: " << dataFitStatus << std::endl;
        std::cout << "  N_sig (Data): " << nsig.getVal() << " +/- " << nsig.getError() << std::endl;
        std::cout << "  N_bkg (Data): " << nbkg.getVal() << " +/- " << nbkg.getError() << std::endl;
        
        // Compute fitted ratios for display
        double nTotFit = std::max(1e-12, nsig.getVal() + nbkg.getVal());
        double nsigRatioFit = nsig.getVal() / nTotFit;
        double nbkgRatioFit = nbkg.getVal() / nTotFit;
        
        // Save Data fit result
        std::string dataFitFileNameBase = Form("results/data_fits/fitresult_%s_%.2f_%.2f_%s_mva%.3f", 
                                         varName.c_str(), varMin, varMax, catLabel.c_str(), mva);
        for (char& c : dataFitFileNameBase) if (c == '.') c = 'p';
        std::string dataFitFileName = dataFitFileNameBase + ".root";
        
        TFile* dataFitFile = TFile::Open(dataFitFileName.c_str(), "RECREATE");
        if (dataFitRes) dataFitRes->Write("fitResult");
        
        // Create Data plot frame and save
        RooPlot* dataFrame = massPion.frame(Title(" "));
        dataBinnedData.plotOn(dataFrame, MarkerSize(0.6), MarkerColor(kBlack), LineColor(kBlack), Name("data"));
        model.plotOn(dataFrame, LineColor(kBlack), LineWidth(2), Name("model"));
        model.plotOn(dataFrame, Components(dataSignalPdf), LineStyle(kDashed), LineColor(kRed), LineWidth(2), Name("signal"));
        model.plotOn(dataFrame, Components(bkgPdf), LineStyle(kDashed), LineColor(kBlue), LineWidth(2), Name("bkg"));
        dataFrame->Write("massFrame");
        dataFitFile->Close();
        
        std::cout << "  Data fit result saved: " << dataFitFileName << std::endl;
        
        // === CREATE 4-PAD COMPARISON PLOT (MC + Data) ===
        std::cout << "  Creating MC vs Data comparison plot..." << std::endl;
        
        // Load MC frame from saved file
        TFile* mcFitFileForPlot = TFile::Open(mcFitFileName.c_str(), "READ");
        RooPlot* mcFrame = nullptr;
        RooFitResult* mcFitResForPlot = nullptr;
        if (mcFitFileForPlot && !mcFitFileForPlot->IsZombie()) {
            mcFrame = (RooPlot*)mcFitFileForPlot->Get("massFrame");
            mcFitResForPlot = (RooFitResult*)mcFitFileForPlot->Get("fitResult");
        }
        
        // Recreate dataFrame for plotting (must be after fit to get updated parameters)
        RooPlot* dataFramePlot = massPion.frame(Title(" "));
        dataBinnedData.plotOn(dataFramePlot, MarkerSize(0.6), MarkerColor(kBlack), LineColor(kBlack), Name("data"));
        model.plotOn(dataFramePlot, LineColor(kBlack), LineWidth(2), Name("model"));
        model.plotOn(dataFramePlot, Components(dataSignalPdf), LineStyle(kDashed), LineColor(kRed), LineWidth(2), Name("signal"));
        model.plotOn(dataFramePlot, Components(bkgPdf), LineStyle(kDashed), LineColor(kBlue), LineWidth(2), Name("bkg"));
        
        if (mcFrame && mcFitResForPlot && dataFramePlot) {
            // Create canvas with MC (left) and Data (right) - 4 pads
            gROOT->cd();  // Make sure we're not in a file directory
            TCanvas* cMass = new TCanvas(Form("cMass_%zu", mvaIdx), Form("MC vs Data: MVA > %.3f", mva), 2400, 600);
            cMass->Divide(4, 1, 0.005, 0);
            
            // === MC PLOT (pad 1) ===
            cMass->cd(1);
            TPad* padMC = (TPad*)gPad;
            padMC->SetRightMargin(0.01);
            padMC->SetLeftMargin(0.12);
            padMC->SetBottomMargin(0.12);
            padMC->SetTopMargin(0.05);
            mcFrame->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
            mcFrame->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
            mcFrame->GetXaxis()->SetTitleSize(0.045);
            mcFrame->GetYaxis()->SetTitleSize(0.045);
            mcFrame->Draw();
            
            // Add legend for MC
            TLegend* legMC = new TLegend(0.50, 0.65, 0.88, 0.90);
            legMC->SetBorderSize(0);
            legMC->SetFillStyle(0);
            legMC->SetTextSize(0.035);
            legMC->AddEntry((TObject*)nullptr, Form("MC | MVA > %.3f", mva), "");
            legMC->AddEntry(mcFrame->findObject("mc_signal"), "MC Signal", "lep");
            legMC->AddEntry(mcFrame->findObject("mc_fit"), "Fit", "l");
            legMC->Draw();
            padMC->Update();
            
            // === MC PARAM (pad 2) ===
            cMass->cd(2);
            TPad* padMCParam = (TPad*)gPad;
            padMCParam->SetLeftMargin(0.02);
            padMCParam->SetRightMargin(0.02);
            padMCParam->SetFillColor(kWhite);
            
            TLatex mcParamText;
            mcParamText.SetTextFont(42);
            mcParamText.SetTextAlign(12);
            
            double ymc = 0.95;
            const double dymc = 0.095;
            
            mcParamText.SetTextSize(0.050);
            mcParamText.DrawLatex(0.10, ymc, "#bf{MC}");
            ymc -= dymc;
            mcParamText.SetTextSize(0.040);
            
            RooRealVar* mcMeanPlot = (RooRealVar*)mcFitResForPlot->floatParsFinal().find("meanCB");
            RooRealVar* mcSigmaPlot = (RooRealVar*)mcFitResForPlot->floatParsFinal().find("sigmaCB");
            RooRealVar* mcAlphaLPlot = (RooRealVar*)mcFitResForPlot->floatParsFinal().find("alphaL");
            RooRealVar* mcAlphaRPlot = (RooRealVar*)mcFitResForPlot->floatParsFinal().find("alphaR");
            RooRealVar* mcNLPlot = (RooRealVar*)mcFitResForPlot->floatParsFinal().find("nL");
            RooRealVar* mcNRPlot = (RooRealVar*)mcFitResForPlot->floatParsFinal().find("nR");
            
            if (mcMeanPlot) mcParamText.DrawLatex(0.10, ymc, Form("m: %.4f", mcMeanPlot->getVal()));
            ymc -= dymc;
            if (mcSigmaPlot) mcParamText.DrawLatex(0.10, ymc, Form("#sigma: %.5f", mcSigmaPlot->getVal()));
            ymc -= dymc;
            if (mcAlphaLPlot) mcParamText.DrawLatex(0.10, ymc, Form("#alpha_{L}: %.2f", mcAlphaLPlot->getVal()));
            ymc -= dymc;
            if (mcAlphaRPlot) mcParamText.DrawLatex(0.10, ymc, Form("#alpha_{R}: %.2f", mcAlphaRPlot->getVal()));
            ymc -= dymc;
            if (mcNLPlot) mcParamText.DrawLatex(0.10, ymc, Form("n_{L}: %.1f", mcNLPlot->getVal()));
            ymc -= dymc;
            if (mcNRPlot) mcParamText.DrawLatex(0.10, ymc, Form("n_{R}: %.1f", mcNRPlot->getVal()));
            padMCParam->Update();
            
            // === DATA PLOT (pad 3) ===
            cMass->cd(3);
            TPad* padData = (TPad*)gPad;
            padData->SetRightMargin(0.01);
            padData->SetLeftMargin(0.12);
            padData->SetBottomMargin(0.12);
            padData->SetTopMargin(0.05);
            
            // Draw data frame
            dataFramePlot->GetXaxis()->SetTitle("#Delta M [GeV/c^{2}]");
            dataFramePlot->GetYaxis()->SetTitle(Form("Events / %.4f GeV/c^{2}", (MASS_MAX-MASS_MIN)/NBINS));
            dataFramePlot->GetXaxis()->SetTitleSize(0.045);
            dataFramePlot->GetYaxis()->SetTitleSize(0.045);
            dataFramePlot->Draw();
            
            // Add legend for Data
            TLegend* legData = new TLegend(0.50, 0.60, 0.88, 0.90);
            legData->SetBorderSize(0);
            legData->SetFillStyle(0);
            legData->SetTextSize(0.035);
            legData->AddEntry((TObject*)nullptr, Form("Data | MVA > %.3f", mva), "");
            
            TObject* dataObj = dataFramePlot->findObject("data");
            TObject* modelObj = dataFramePlot->findObject("model");
            TObject* sigObj = dataFramePlot->findObject("signal");
            TObject* bkgObj = dataFramePlot->findObject("bkg");
            
            if (dataObj) legData->AddEntry(dataObj, "Data", "lep");
            if (modelObj) legData->AddEntry(modelObj, "Fit", "l");
            if (sigObj) legData->AddEntry(sigObj, "Signal", "l");
            if (bkgObj) legData->AddEntry(bkgObj, "Background", "l");
            legData->Draw();
            padData->Update();
            
            // === DATA PARAM (pad 4) ===
            cMass->cd(4);
            TPad* padDataParam = (TPad*)gPad;
            padDataParam->SetLeftMargin(0.02);
            padDataParam->SetRightMargin(0.05);
            padDataParam->SetFillColor(kWhite);
            
            TLatex dataParamText;
            dataParamText.SetTextFont(42);
            dataParamText.SetTextAlign(12);
            
            double y = 0.95;
            const double dy = 0.095;
            double x_left = 0.05;
            double x_right = 0.55;
            
            // Left: Signal (Data)
            dataParamText.SetTextSize(0.050);
            dataParamText.DrawLatex(x_left, y, "#bf{Data}");
            y -= dy;
            dataParamText.SetTextSize(0.040);
            dataParamText.DrawLatex(x_left, y, Form("m: %.4f", dataMeanCB.getVal()));
            y -= dy;
            dataParamText.DrawLatex(x_left, y, Form("#sigma: %.5f", dataSigmaCB.getVal()));
            y -= dy;
            dataParamText.DrawLatex(x_left, y, Form("#alpha_{L}: %.2f%s", dataAlphaL.getVal(), dataAlphaL.isConstant() ? " (f)" : ""));
            y -= dy;
            dataParamText.DrawLatex(x_left, y, Form("#alpha_{R}: %.2f%s", dataAlphaR.getVal(), dataAlphaR.isConstant() ? " (f)" : ""));
            y -= dy;
            dataParamText.DrawLatex(x_left, y, Form("n_{L}: %.1f%s", dataNL.getVal(), dataNL.isConstant() ? " (f)" : ""));
            y -= dy;
            dataParamText.DrawLatex(x_left, y, Form("n_{R}: %.1f%s", dataNR.getVal(), dataNR.isConstant() ? " (f)" : ""));
            y -= dy;
            dataParamText.SetTextSize(0.035);
            dataParamText.DrawLatex(x_left, y, Form("N_{s}: %.0f", nsig.getVal()));
            y -= dy;
            dataParamText.DrawLatex(x_left, y, Form("Ratio: %.3f", nsigRatioFit));
            
            // Right: Background
            y = 0.95;
            dataParamText.SetTextSize(0.050);
            dataParamText.DrawLatex(x_right, y, "#bf{Bkg} (Phenomenological2)");
            y -= dy;
            dataParamText.SetTextSize(0.040);
            dataParamText.DrawLatex(x_right, y, Form("m: %.3f%s", m_param.getVal(), m_param.isConstant() ? " (f)" : ""));
            y -= dy;
            dataParamText.DrawLatex(x_right, y, Form("#lambda: %.3f%s", c_param.getVal(), c_param.isConstant() ? " (f)" : ""));
            y -= dy * 3;  // Skip unused params
            dataParamText.SetTextSize(0.035);
            dataParamText.DrawLatex(x_right, y, Form("N_{b}: %.0f", nbkg.getVal()));
            y -= dy;
            dataParamText.DrawLatex(x_right, y, Form("Ratio: %.3f", nbkgRatioFit));
            padDataParam->Update();
            
            // Save plot
            cMass->cd();  // Go back to main canvas
            cMass->Update();  // Force update before saving
            std::string plotName = Form("results/data_fits/mass_fits_%s_%.2f_%.2f_%s_mva%.3f", 
                                       varName.c_str(), varMin, varMax, catLabel.c_str(), mva);
            for (char& c : plotName) if (c == '.') c = 'p';
            cMass->SaveAs((plotName + ".png").c_str());
            cMass->SaveAs((plotName + ".pdf").c_str());
            
            // Save to ROOT file with Write()
            TFile* plotFile = TFile::Open((plotName + ".root").c_str(), "RECREATE");
            cMass->Write();
            plotFile->Close();
            delete plotFile;
            
            std::cout << "  Saved comparison plot: " << plotName << ".png" << std::endl;
            
            delete cMass;
        } else {
            std::cout << "  WARNING: Could not load MC or Data frame for comparison plot" << std::endl;
        }
        
        if (mcFitFileForPlot) mcFitFileForPlot->Close();
        // dataFrame is created in this scope, don't close file
        
        // Cleanup background PDF
        // if (bkgPdf) delete bkgPdf;
        
        // Store yield
        if (dataFitStatus == 0 && nsig.getVal() > 0) {
            yields.push_back(nsig.getVal());
            yield_errors.push_back(nsig.getError());
        } else {
            yields.push_back(0);
            yield_errors.push_back(0);
        }
    }
    
    // === STEP 4: CREATE YIELD PLOT ===
    std::cout << "\n=== Step 4: Creating Yield vs MVA Plot ===" << std::endl;
    
    TCanvas* cYield = new TCanvas("cYield", "Signal Yield vs MVA", 800, 600);
    cYield->SetLeftMargin(0.15);
    cYield->SetBottomMargin(0.15);
    
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
    tex->DrawLatex(0.20, 0.88, Form("%s: [%.2f, %.2f]", varLabel.c_str(), varMin, varMax));
    
    std::string yieldPlotName = Form("results/data_fits/yield_vs_mva_%s_%.2f_%.2f_%s", 
                                     varName.c_str(), varMin, varMax, catLabel.c_str());
    for (char& c : yieldPlotName) if (c == '.') c = 'p';
    cYield->SaveAs((yieldPlotName + ".png").c_str());
    cYield->SaveAs((yieldPlotName + ".pdf").c_str());
    cYield->SaveAs((yieldPlotName + ".root").c_str());
    std::cout << "Saved yield plot: " << yieldPlotName << ".png" << std::endl;
    delete cYield;
    
    // Save yields to text file
    std::string yieldsFileName = Form("results/data_fits/yields_%s_%.2f_%.2f_%s.txt", 
                                   varName.c_str(), varMin, varMax, catLabel.c_str());
    for (char& c : yieldsFileName) if (c == '.') c = 'p';
    std::ofstream out(yieldsFileName);
    out << "# MVA & Data | " << varName << " bin: [" << varMin << ", " << varMax << "]\n";
    out << "# mva\tyield\tyield_err\n";
    for (size_t i = 0; i < mvaThresholds.size(); ++i) {
        out << mvaThresholds[i] << "\t" << yields[i] << "\t" << yield_errors[i] << "\n";
    }
    out.close();
    std::cout << "Saved yields: " << yieldsFileName << std::endl;
    
    std::cout << "\n=== Fitting Complete ===" << std::endl;
    std::cout << "Processed " << yields.size() << " MVA bins" << std::endl;
    
    std::cout << "\n===========================================" << std::endl;
    std::cout << "=== Completed Successfully ===" << std::endl;
    std::cout << "===========================================" << std::endl;
}
