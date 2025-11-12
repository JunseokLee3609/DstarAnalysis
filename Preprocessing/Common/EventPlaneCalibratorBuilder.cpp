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
 *   root -l -b -q 'Preprocessing/Common/EventPlaneCalibratorBuilder.cpp("input.list","trkEvtPlaneCalib.root","eventplane/EventPlane",123)'
 *
 * The macro reads the TChain "eventplane/EventPlane" and produces:
 *   - hQxvsQyRaw_Trk      : raw Q-vector scatter (for mean extraction)
 *   - hQxvsQyRec_Trk      : recentered Q-vector scatter (diagnostic)
 *   - hPsiRec_Trk         : recentered Psi2 distribution
 *   - hsin2iPsi_Trk_%d    : flattening sine moments (i=1..10)
 *   - hcos2iPsi_Trk_%d    : flattening cosine moments (i=1..10)
 */
int EventPlaneCalibratorBuilder(const char* input = "",
                                const char* output = "trkEvtPlaneCalib.root",
                                const char* treeName = "eventplane/EventPlane",
                                int jobIdx = -1) {
    if (!input || std::string(input).empty()) {
        std::cerr << "Error: input file or list is required." << std::endl;
        return 1;
    }

    TChain chain(treeName);
    if (!LoadInputFiles(chain, input)) {
        std::cerr << "Error: failed to load any entries for tree " << treeName << std::endl;
        return 1;
    }

    Double_t trkQx = 0.0;
    Double_t trkQy = 0.0;
    chain.SetBranchAddress("trkQx", &trkQx);
    chain.SetBranchAddress("trkQy", &trkQy);

    const Long64_t nEntries = chain.GetEntries();
    if (nEntries <= 0) {
        std::cerr << "Error: empty chain after loading inputs." << std::endl;
        return 1;
    }
    std::cout << "Loaded " << nEntries << " events from " << treeName << std::endl;

    auto hQxvsQyRaw = std::make_unique<TH2D>("hQxvsQyRaw_Trk", "", 160, -2, 2, 160, -2, 2);
    auto hQxvsQyRec = std::make_unique<TH2D>("hQxvsQyRec_Trk", "", 160, -2, 2, 160, -2, 2);
    auto hPsiRaw = std::make_unique<TH1D>("hPsiRaw_Trk", "Raw Event Plane Angle;#Psi_{2}^{raw};Events", 180, -2, 2);
    auto hPsiRec = std::make_unique<TH1D>("hPsiRec_Trk", "Recentered Event Plane Angle;#Psi_{2}^{rec};Events", 180, -2, 2);
    auto hPsiFlat = std::make_unique<TH1D>("hPsiFlat_Trk", "Flattened Event Plane Angle;#Psi_{2}^{flat};Events", 180, -2, 2);

    std::array<std::unique_ptr<TH1D>, 10> hSin;
    std::array<std::unique_ptr<TH1D>, 10> hCos;
    for (int i = 1; i <= 10; ++i) {
        hSin[i - 1] = std::make_unique<TH1D>(
            Form("hsin2iPsi_Trk_%d", i), "", 100, -1.2, 1.2);
        hCos[i - 1] = std::make_unique<TH1D>(
            Form("hcos2iPsi_Trk_%d", i), "", 100, -1.2, 1.2);
    }

    long double sumQx = 0.0;
    long double sumQy = 0.0;
    Long64_t validRaw = 0;
    
    std::cout << "============================================" << std::endl;
    std::cout << "Pass 1: Computing Q-vector means" << std::endl;
    std::cout << "============================================" << std::endl;
    
    for (Long64_t i = 0; i < nEntries; ++i) {
        if (i > 0 && i % 1000000 == 0) {
            double progress = 100.0 * i / nEntries;
            std::cout << "  Processing: " << i << " / " << nEntries 
                      << " (" << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
        }
        chain.GetEntry(i);
        if (!std::isfinite(trkQx) || !std::isfinite(trkQy)) continue;
        hQxvsQyRaw->Fill(trkQx, trkQy);
        const double psiRaw = 0.5 * std::atan2(trkQy, trkQx);
        hPsiRaw->Fill(psiRaw);
        sumQx += trkQx;
        sumQy += trkQy;
        ++validRaw;
    }
    if (validRaw == 0) {
        std::cerr << "Error: no finite trkQx/trkQy entries were found." << std::endl;
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
    for (Long64_t i = 0; i < nEntries; ++i) {
        if (i > 0 && i % 1000000 == 0) {
            double progress = 100.0 * i / nEntries;
            std::cout << "  Processing: " << i << " / " << nEntries 
                      << " (" << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
        }
        chain.GetEntry(i);
        if (!std::isfinite(trkQx) || !std::isfinite(trkQy)) continue;
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
    
    for (Long64_t i = 0; i < nEntries; ++i) {
        if (i > 0 && i % 1000000 == 0) {
            double progress = 100.0 * i / nEntries;
            std::cout << "  Processing: " << i << " / " << nEntries 
                      << " (" << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
        }
        chain.GetEntry(i);
        if (!std::isfinite(trkQx) || !std::isfinite(trkQy)) continue;
        const double qxRec = trkQx - meanQx;
        const double qyRec = trkQy - meanQy;
        double psiFlat = 0.5 * std::atan2(qyRec, qxRec);
        
        for (int harm = 1; harm <= 10; ++harm) {
            const double arg = 2.0 * harm * psiFlat;
            psiFlat += (2.0 / (2.0 * harm)) * (meanSin[harm-1] * std::cos(arg) - meanCos[harm-1] * std::sin(arg));
        }
        
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
