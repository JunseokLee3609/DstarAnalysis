#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <cmath>
#include <iomanip>

#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TString.h"

namespace {

bool LoadInputFiles(TChain& chain, const std::string& input) {
    if (input.empty()) return false;
    if (input.rfind(".root") != std::string::npos) {
        chain.Add(input.c_str());
        return chain.GetEntries() > 0;
    }

    std::ifstream fin(input);
    if (!fin.is_open()) {
        std::cerr << "Error: cannot open input list " << input << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        if (line.rfind(".root") == std::string::npos) continue;
        chain.Add(line.c_str());
    }
    fin.close();
    if (chain.GetEntries() == 0) {
        std::cerr << "Error: no ROOT files were added from " << input << std::endl;
        return false;
    }
    return true;
}

} // namespace

/**
 * Build track-event-plane centering/flattening histograms directly from the input sample.
 *
 * Usage (ROOT batch):
 *   root -l -b -q 'Preprocessing/Common/EventPlaneCalibratorBuilder.cpp("input.list","trkEvtPlaneCalib.root")'
 *   root -l -b -q 'Preprocessing/Common/EventPlaneCalibratorBuilder.cpp("input.list","trkEvtPlaneCalib.root","eventplane/EventPlane",123,0,180)'
 *
 * The macro reads the TChain "eventplane/EventPlane" and produces:
 *   - hQxvsQyRaw_Trk      : raw Q-vector scatter (for mean extraction)
 *   - hQxvsQyRec_Trk      : recentered Q-vector scatter (diagnostic)
 *   - hPsiRaw_Trk         : raw Psi2 distribution
 *   - hPsiRec_Trk         : recentered Psi2 distribution
 *   - hPsiFlat_Trk        : flattened Psi2 distribution
 *   - hsin2iPsi_Trk_%d    : flattening sine moments (i=1..10)
 *   - hcos2iPsi_Trk_%d    : flattening cosine moments (i=1..10)
 */
int EventPlaneCalibratorBuilder(const char* input = "",
                                const char* output = "trkEvtPlaneCalib.root",
                                const char* treeName = "eventplane/EventPlane",
                                int jobIdx = -1,
                                int centMin = 0,
                                int centMax = 180) {
    if (!input || std::string(input).empty()) {
        std::cerr << "Error: input file or list is required." << std::endl;
        return 1;
    }

    // Track event-plane tree
    TChain trkChain(treeName);
    if (!LoadInputFiles(trkChain, input)) {
        std::cerr << "Error: failed to load any entries for tree " << treeName << std::endl;
        return 1;
    }

    // CS ntuple for J/psi selection (match eventplaneana)
    TChain csChain("dStarana/PATCompositeNtuple");
    if (!LoadInputFiles(csChain, input)) {
        std::cerr << "Error: failed to load CS ntuple entries." << std::endl;
        return 1;
    }

    // Centrality tree
    TChain centChain("eventinfoana/EventInfoNtuple");
    if (!LoadInputFiles(centChain, input)) {
        std::cerr << "Error: failed to load centrality entries." << std::endl;
        return 1;
    }

    Double_t trkQx = 0.0;
    Double_t trkQy = 0.0;
    trkChain.SetBranchAddress("trkQx", &trkQx);
    trkChain.SetBranchAddress("trkQy", &trkQy);

    // Manually set up CS branches we need (candSize, pT, y, mass)
    // Don't use simpleDMC setTree() as it tries to set eventplane branches that don't exist in CS ntuple
    Int_t candSize = 0;
    Float_t pT[5000], y[5000], mass[5000];
    csChain.SetBranchAddress("candSize", &candSize);
    csChain.SetBranchAddress("pT", pT);
    csChain.SetBranchAddress("y", y);
    csChain.SetBranchAddress("mass", mass);

    Short_t cen = -99;
    centChain.SetBranchAddress("centrality", &cen);

    // Use the minimum entries across chains, start at 1 to match eventplaneana
    Long64_t nTrk = trkChain.GetEntries();
    Long64_t nCs  = csChain.GetEntries();
    Long64_t nCent= centChain.GetEntries();
    Long64_t nEntries = nTrk;
    if (nCs < nEntries) nEntries = nCs;
    if (nCent < nEntries) nEntries = nCent;

    if (nEntries <= 1) {
        std::cerr << "Error: insufficient synchronized entries across chains." << std::endl;
        return 1;
    }
    std::cout << "Loaded synchronized entries: " << nEntries << std::endl;

    auto hQxvsQyRaw = std::make_unique<TH2D>("hQxvsQyRaw_Trk", "", 160, -2, 2, 160, -2, 2);
    auto hQxvsQyRec = std::make_unique<TH2D>("hQxvsQyRec_Trk", "", 160, -2, 2, 160, -2, 2);
    auto hPsiRaw = std::make_unique<TH1D>("hPsiRaw_Trk", "Raw Event Plane Angle;#Psi_{2}^{raw};Events", 180, -2, 2);
    auto hPsiRec = std::make_unique<TH1D>("hPsiRec_Trk", "Recentered Event Plane Angle;#Psi_{2}^{rec};Events", 180, -2, 2);
    auto hPsiFlat = std::make_unique<TH1D>("hPsiFlat_Trk", "Flattened Event Plane Angle;#Psi_{2}^{flat};Events", 180, -2, 2);

    std::array<std::unique_ptr<TH1D>, 10> hSin;
    std::array<std::unique_ptr<TH1D>, 10> hCos;
    for (int i = 1; i <= 10; ++i) {
        hSin[i - 1] = std::make_unique<TH1D>(Form("hsin2iPsi_Trk_%d", i), "", 100, -1.0, 1.0);
        hCos[i - 1] = std::make_unique<TH1D>(Form("hcos2iPsi_Trk_%d", i), "", 100, -1.0, 1.0);
    }

    auto passSelection = [&](Long64_t idx) -> bool {
        trkChain.GetEntry(idx);
        if (!std::isfinite(trkQx) || !std::isfinite(trkQy)) return false;
        if (csChain.GetEntry(idx) < 0) return false;
        if (centChain.GetEntry(idx) < 0) return false;
        if (cen < centMin || cen >= centMax) return false;
        bool isJpsi = false;
        for (Int_t icand = 0; icand < candSize; ++icand) {
            if (std::abs(y[icand]) >= 1) continue;
            Float_t pt = pT[icand];
            Float_t m = mass[icand];
            if (m > 1.7 && m < 2.1 && pt >= 0.2 && pt < 20) { isJpsi = true; break; }
        }
        if (!isJpsi) return false;
        return true;
    };

    long double sumQx = 0.0;
    long double sumQy = 0.0;
    Long64_t validRaw = 0;
    
    std::cout << "\n============================================" << std::endl;
    std::cout << "Pass 1: Computing Q-vector means" << std::endl;
    std::cout << "============================================" << std::endl;
    
    for (Long64_t i = 1; i < nEntries; ++i) {
        if (i > 0 && i % 1000000 == 0) {
            double progress = 100.0 * i / nEntries;
            std::cout << "  Processing: " << i << " / " << nEntries 
                      << " (" << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
        }
        if (!passSelection(i)) continue;
        hQxvsQyRaw->Fill(trkQx, trkQy);
        const double psiRaw = 0.5 * std::atan2(trkQy, trkQx);
        hPsiRaw->Fill(psiRaw);
        sumQx += trkQx;
        sumQy += trkQy;
        ++validRaw;
    }
    if (validRaw == 0) {
        std::cerr << "Error: no entries passed selection for mean calculation." << std::endl;
        return 1;
    }
    const double meanQx = static_cast<double>(sumQx / validRaw);
    const double meanQy = static_cast<double>(sumQy / validRaw);
    std::cout << "\nQ-vector means (from " << validRaw << " entries):" << std::endl;
    std::cout << "  <Qx> = " << meanQx << std::endl;
    std::cout << "  <Qy> = " << meanQy << std::endl;

    std::cout << "\n============================================" << std::endl;
    std::cout << "Pass 2: Computing flattening moments" << std::endl;
    std::cout << "============================================" << std::endl;
    
    Long64_t validRec = 0;
    for (Long64_t i = 1; i < nEntries; ++i) {
        if (i > 0 && i % 1000000 == 0) {
            double progress = 100.0 * i / nEntries;
            std::cout << "  Processing: " << i << " / " << nEntries 
                      << " (" << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
        }
        if (!passSelection(i)) continue;
        const double qxRec = trkQx - meanQx;
        const double qyRec = trkQy - meanQy;
        const double psiRec = 0.5 * std::atan2(qyRec, qxRec);
        hQxvsQyRec->Fill(qxRec, qyRec);
        hPsiRec->Fill(psiRec);
        for (int harm = 1; harm <= 10; ++harm) {
            const double arg = 2.0 * harm * psiRec;
            hSin[harm - 1]->Fill(std::sin(arg));
            hCos[harm - 1]->Fill(std::cos(arg));
        }
        ++validRec;
    }
    
    std::cout << "\nComputing flattening corrections from " << validRec << " entries..." << std::endl;
    std::array<double, 10> meanSin, meanCos;
    for (int i = 0; i < 10; ++i) {
        meanSin[i] = hSin[i]->GetMean();
        meanCos[i] = hCos[i]->GetMean();
        std::cout << "  Harm " << (i+1) << ": <sin> = " << meanSin[i] 
                  << ", <cos> = " << meanCos[i] << std::endl;
    }
    
    std::cout << "\n============================================" << std::endl;
    std::cout << "Pass 3: Applying flattening corrections" << std::endl;
    std::cout << "============================================" << std::endl;
    
    for (Long64_t i = 1; i < nEntries; ++i) {
        if (i > 0 && i % 1000000 == 0) {
            double progress = 100.0 * i / nEntries;
            std::cout << "  Processing: " << i << " / " << nEntries 
                      << " (" << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
        }
        if (!passSelection(i)) continue;
        const double qxRec = trkQx - meanQx;
        const double qyRec = trkQy - meanQy;
        const double psiRecHere = 0.5 * std::atan2(qyRec, qxRec);
        
        // Apply flattening correction following EventPlaneAnalyzerFinal: PsiFlat = PsiRec + sum_i (1/i)*(-<sin>*cos(2i PsiRec) + <cos>*sin(2i PsiRec))
        double deltaPsi2 = 0.0;
        for (int harm = 1; harm <= 10; ++harm) {
            const double arg = 2.0 * harm * psiRecHere;
            deltaPsi2 += (1.0 / harm) * ((-meanSin[harm-1]) * std::cos(arg) + (meanCos[harm-1]) * std::sin(arg));
        }
        const double psiFlat = psiRecHere + deltaPsi2;
        
        hPsiFlat->Fill(psiFlat);
    }

    std::string outputPath(output);
    if (jobIdx >= 0) {
        size_t dotPos = outputPath.rfind('.');
        if (dotPos != std::string::npos) {
            outputPath = outputPath.substr(0, dotPos) + "_" + std::to_string(jobIdx) + outputPath.substr(dotPos);
        } else {
            outputPath += "_" + std::to_string(jobIdx);
        }
    }

    std::cout << "\n============================================" << std::endl;
    std::cout << "Saving histograms..." << std::endl;
    std::cout << "============================================" << std::endl;
    
    TFile fout(outputPath.c_str(), "RECREATE");
    if (fout.IsZombie()) {
        std::cerr << "Error: failed to create output file " << outputPath << std::endl;
        return 1;
    }

    hQxvsQyRaw->Write();
    hQxvsQyRec->Write();
    hPsiRaw->Write();
    hPsiRec->Write();
    hPsiFlat->Write();
    for (int i = 0; i < 10; ++i) {
        hSin[i]->Write();
        hCos[i]->Write();
    }
    fout.Close();
    
    std::cout << "\n============================================" << std::endl;
    std::cout << "Event-plane calibration histograms saved to:" << std::endl;
    std::cout << "  " << outputPath << std::endl;
    std::cout << "\nHistograms saved:" << std::endl;
    std::cout << "  - hQxvsQyRaw_Trk, hQxvsQyRec_Trk" << std::endl;
    std::cout << "  - hPsiRaw_Trk, hPsiRec_Trk, hPsiFlat_Trk" << std::endl;
    std::cout << "  - hsin2iPsi_Trk_1-10, hcos2iPsi_Trk_1-10" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return 0;
}
