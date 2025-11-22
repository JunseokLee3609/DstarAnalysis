#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <memory>
#include <cmath>
#include <glob.h>
#include <algorithm>

#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TLine.h"
#include "TStyle.h"
#include "TSystem.h"
#include "RooFitResult.h"
#include "RooRealVar.h"

#include "../../../../Interface/simpleDMC.h"
#include "../../../../Interface/simpleAlgos.hxx"
#include "../../../../Interface/ConfigPbPb.h"
#include "../../../../Tools/Transformations.h"
#include "../../../../Tools/Utils/JSONLoader.h"

#include "KinematicBinConfig.h"

using namespace KinematicBinConfig;

namespace {

constexpr double kCentralityScale = 0.5;
std::unique_ptr<TH2D> gLastEfficiencyMap;
std::unique_ptr<TH2D> gLastRecoMap;
std::unique_ptr<TH2D> gLastGenMap;

bool IsLikelyDatasetKey(const std::string& token) {
    if (token.empty()) return false;
    if (token.find(".root") != std::string::npos) return false;
    return token.find('/') == std::string::npos;
}

std::unique_ptr<TFile> LoadDatasetFileFromCatalog(const std::string& datasetKey,
                                                  const std::string& datasetCatalog,
                                                  bool quietOnMissingKey = false) {
    if (datasetKey.empty() || datasetCatalog.empty()) return nullptr;
    try {
        const auto info = JsonLoader::LoadObjectMap(datasetCatalog, "datasets", datasetKey);
        const auto pathIt = info.find("file");
        if (pathIt == info.end()) {
            std::cerr << "[LoadDatasetFileFromCatalog] dataset '" << datasetKey
                      << "' is missing 'file' entry" << std::endl;
            return nullptr;
        }
        TFile* handle = TFile::Open(pathIt->second.c_str(), "READ");
        if (!handle || handle->IsZombie()) {
            std::cerr << "[LoadDatasetFileFromCatalog] failed to open " << pathIt->second
                      << " for dataset '" << datasetKey << "'" << std::endl;
            if (handle) { handle->Close(); delete handle; }
            return nullptr;
        }
        return std::unique_ptr<TFile>(handle);
    } catch (const std::exception& ex) {
        const std::string msg = ex.what();
        const bool missingEntry = (msg.find("cannot find entry") != std::string::npos);
        if (!quietOnMissingKey || !missingEntry) {
            std::cerr << "[LoadDatasetFileFromCatalog] " << msg << std::endl;
        }
    }
    return nullptr;
}

double ComputeEventPlaneCos(double dstarPt, double dstarEta, double dstarPhi, double dstarMass,
                            double daughterPt, double daughterEta, double daughterPhi, double daughterMass,
                            double psi2) {
    TLorentzVector dstar;
    TLorentzVector daughter;
    dstar.SetPtEtaPhiM(dstarPt, dstarEta, dstarPhi, dstarMass);
    daughter.SetPtEtaPhiM(daughterPt, daughterEta, daughterPhi, daughterMass);
    const TVector3 vec = DstarDau1Vector_EventPlane(dstar, daughter, psi2);
    return vec.CosTheta();
}

inline double ExtractCentralityPercent(double rawCent) {
    return kCentralityScale * rawCent;
}

bool CentralityWithinAnalysisWindow(double centPercent) {
    return centPercent >= CENT_EDGES.front() && centPercent <= CENT_EDGES.back();
}

bool PassGenBaseline(const DataFormat::simpleDStarMCTreeflat& evt) {
    if (std::abs(evt.gen_y) > SelectionCuts::Y_PARENT_MAX) return false;
    if (evt.gen_pT < PT_EDGES.front() || evt.gen_pT > PT_EDGES.back()) return false;
    const double centPercent = ExtractCentralityPercent(evt.centrality);
    if (!CentralityWithinAnalysisWindow(centPercent)) return false;
    return true;
}

bool PassRecoBaseline(const DataFormat::simpleDStarMCTreeflat& evt, double mvaSelect) {
    if (!evt.matchGEN || !evt.matchGen_validDstarChain || evt.isSwap) return false;
    if (evt.mva < mvaSelect) return false;
    if (std::abs(evt.y) > SelectionCuts::Y_PARENT_MAX) return false;
    if (evt.pT < PT_EDGES.front() || evt.pT > PT_EDGES.back()) return false;
    if (std::abs(evt.EtaD2) > SelectionCuts::ETA_D2_MAX) return false;
    if (evt.pTD2 < SelectionCuts::PT_D2_MIN) return false;
    if (std::abs(evt.EtaGrandD1) > SelectionCuts::ETA_GRANDD1_MAX) return false;
    if (std::abs(evt.EtaGrandD2) > SelectionCuts::ETA_GRANDD2_MAX) return false;
    if (evt.pTGrandD1 < SelectionCuts::PT_GRANDD1_MIN) return false;
    if (evt.pTGrandD2 < SelectionCuts::PT_GRANDD2_MIN) return false;
    const double centPercent = ExtractCentralityPercent(evt.centrality);
    if (!CentralityWithinAnalysisWindow(centPercent)) return false;
    return true;
}

inline bool IsPromptGen(const DataFormat::simpleDStarMCTreeflat& evt) {
    return evt.gen_D0ancestorFlavor_ != 5;
}

inline bool IsPromptReco(const DataFormat::simpleDStarMCTreeflat& evt) {
    return evt.matchGen_D1ancestorFlavor_ != 5;
}

bool ComputeGenCos(const DataFormat::simpleDStarMCTreeflat& evt, double& cosVal) {
    if (evt.gen_pT <= 0 || evt.gen_D0pT <= 0) return false;
    cosVal = ComputeEventPlaneCos(evt.gen_pT, evt.gen_eta, evt.gen_phi, evt.gen_mass,
                                  evt.gen_D0pT, evt.gen_D0eta, evt.gen_D0phi, evt.gen_D0mass,
                                  evt.Psi2Raw_Trk);
    return std::isfinite(cosVal);
}

bool ComputeRecoCos(const DataFormat::simpleDStarMCTreeflat& evt, double& cosVal) {
    if (evt.pT <= 0 || evt.pTD1 <= 0) return false;
    cosVal = ComputeEventPlaneCos(evt.pT, evt.eta, evt.phi, evt.mass,
                                  evt.pTD1, evt.EtaD1, evt.PhiD1, evt.massDaugther1,
                                  evt.Psi2Raw_Trk);
    return std::isfinite(cosVal);
}

double GetCorrectionWeight(const TH2D* corrMap,
                           double ptMin, double ptMax,
                           double cosMin, double cosMax) {
    if (!corrMap) return 1.0;
    const double ptMid = 0.5 * (ptMin + ptMax);
    const double cosMid = 0.5 * (cosMin + cosMax);
    const int binX = corrMap->GetXaxis()->FindBin(ptMid);
    const int binY = corrMap->GetYaxis()->FindBin(cosMid);
    double weight = corrMap->GetBinContent(binX, binY);
    if (!(weight > 0.0)) weight = 1.0;
    if (weight > 1e3) weight = 1e3;
    return weight;
}

std::unique_ptr<TH2D> BuildPtCosHist(const char* name) {
    auto hist = std::make_unique<TH2D>(name, "", N_PT_BINS, &PT_EDGES[0],
                                       N_COSTHETA_BINS, &COSTHETA_EDGES[0]);
    hist->Sumw2();
    hist->SetDirectory(nullptr);
    return hist;
}

std::string BuildHistFileStem(const std::string& datasetKey,
                              double ptMin, double ptMax,
                              double yMin, double yMax,
                              double centMin, double centMax,
                              double cosMin, double cosMax,
                              double mvaSelect,
                              int dcaMode) {
    std::ostringstream oss;
    oss << "hist_" << (datasetKey.empty() ? "dataset" : datasetKey)
        << "_pt" << FormatValP(ptMin, 1) << "_" << FormatValP(ptMax, 1)
        << "_y" << FormatValP(yMin, 1) << "_" << FormatValP(yMax, 1)
        << "_cent" << FormatValP(centMin, 0) << "_" << FormatValP(centMax, 0)
        << "_cosEP" << FormatValP(cosMin, 2) << "_" << FormatValP(cosMax, 2)
        << "_mva" << FormatValP(mvaSelect, 3)
        << "_" << DCAModeToString(dcaMode);
    std::string stem = oss.str();
    std::replace(stem.begin(), stem.end(), '.', 'p');
    return stem;
}

std::string BuildHistFileName(const std::string& datasetKey,
                              double ptMin, double ptMax,
                              double yMin, double yMax,
                              double centMin, double centMax,
                              double cosMin, double cosMax,
                              double mvaSelect,
                              int dcaMode,
                              const std::string& suffix = "_fitResult.root") {
    std::string stem = BuildHistFileStem(datasetKey, ptMin, ptMax, yMin, yMax,
                                         centMin, centMax, cosMin, cosMax, mvaSelect, dcaMode);
    return stem + suffix;
}

std::string BuildRunOutputDir(const std::string& baseDir,
                              const std::string& dataKey,
                              double mvaSelect) {
    std::string keySafe = dataKey;
    std::replace(keySafe.begin(), keySafe.end(), '/', '_');
    std::replace(keySafe.begin(), keySafe.end(), ' ', '_');
    std::replace(keySafe.begin(), keySafe.end(), '.', 'p');
    return Form("%s/%s_prompt_mva%03d", baseDir.c_str(), keySafe.c_str(), int(std::round(mvaSelect*1000.0)));
}

TH2D* BuildPtCosEffCorrMapFromDatasetKey(const char* datasetKey,
                                         const char* datasetCatalog,
                                         double mvaSelect) {
    if (!datasetKey || !datasetCatalog) return nullptr;
    gLastEfficiencyMap.reset();
    gLastRecoMap.reset();
    gLastGenMap.reset();

    std::unique_ptr<TFile> datasetFile =
        LoadDatasetFileFromCatalog(datasetKey, datasetCatalog, false);
    if (!datasetFile) return nullptr;

    TTree* genTree = dynamic_cast<TTree*>(datasetFile->Get("skimGENTreeFlat"));
    TTree* recoTree = dynamic_cast<TTree*>(datasetFile->Get("skimTreeFlat"));
    if (!genTree || !recoTree) {
        std::cerr << "[BuildPtCosEffCorrMap] Missing skimGENTreeFlat/skimTreeFlat in dataset "
                  << datasetKey << std::endl;
        return nullptr;
    }

    DataFormat::simpleDStarMCTreeflat genEvt;
    DataFormat::simpleDStarMCTreeflat recoEvt;
    genEvt.setGENTree(genTree);
    recoEvt.setTree(recoTree);

    auto hGen = BuildPtCosHist("hPtCosGen");
    auto hReco = BuildPtCosHist("hPtCosReco");

    const Long64_t nGen = genTree->GetEntries();
    for (Long64_t idx = 0; idx < nGen; ++idx) {
        genTree->GetEntry(idx);
        if (!PassGenBaseline(genEvt)) continue;
        if (!IsPromptGen(genEvt)) continue;
        double cosVal = 0.0;
        if (!ComputeGenCos(genEvt, cosVal)) continue;
        const double clampedCos = std::max(-0.999, std::min(0.999, cosVal));
        hGen->Fill(genEvt.gen_pT, clampedCos);
    }

    const double effMvaCut = 0.99;
    const double recoMvaCut = std::max(mvaSelect, effMvaCut);

    const Long64_t nReco = recoTree->GetEntries();
    for (Long64_t idx = 0; idx < nReco; ++idx) {
        recoTree->GetEntry(idx);
        if (!PassRecoBaseline(recoEvt, recoMvaCut)) continue;
        if (!IsPromptReco(recoEvt)) continue;
        double cosVal = 0.0;
        if (!ComputeRecoCos(recoEvt, cosVal)) continue;
        const double clampedCos = std::max(-0.999, std::min(0.999, cosVal));
        hReco->Fill(recoEvt.pT, clampedCos);
    }

    if (hGen->GetSumOfWeights() == 0 || hReco->GetSumOfWeights() == 0) {
        std::cerr << "[BuildPtCosEffCorrMap] Not enough entries to build efficiency map for dataset "
                  << datasetKey << std::endl;
        return nullptr;
    }

    auto effMap = std::make_unique<TH2D>("hEffPtCos", "Efficiency; p_{T} (GeV/c);cos(#theta)",
                                         N_PT_BINS, &PT_EDGES[0], N_COSTHETA_BINS, &COSTHETA_EDGES[0]);
    effMap->SetDirectory(nullptr);
    auto corrMap = std::make_unique<TH2D>("hEffCorrPtCos", "Eff. correction (1/eff);p_{T} (GeV/c);cos(#theta)",
                                          N_PT_BINS, &PT_EDGES[0], N_COSTHETA_BINS, &COSTHETA_EDGES[0]);
    corrMap->SetDirectory(nullptr);

    for (int ipt = 1; ipt <= N_PT_BINS; ++ipt) {
        for (int icos = 1; icos <= N_COSTHETA_BINS; ++icos) {
            const double num = hReco->GetBinContent(ipt, icos);
            const double den = hGen->GetBinContent(ipt, icos);
            double eff = (den > 0.0) ? num / den : 0.0;
            if (eff < 1e-6) eff = 0.0;
            effMap->SetBinContent(ipt, icos, eff);
            double weight = (eff > 0.0) ? (1.0 / eff) : 1.0;
            if (weight > 1e3) weight = 1e3;
            corrMap->SetBinContent(ipt, icos, weight);
        }
    }

    gLastEfficiencyMap = std::move(effMap);
    gLastRecoMap.reset((TH2D*)hReco->Clone("hRecoPtCos"));
    gLastGenMap.reset((TH2D*)hGen->Clone("hGenPtCos"));
    if (gLastRecoMap) gLastRecoMap->SetDirectory(nullptr);
    if (gLastGenMap) gLastGenMap->SetDirectory(nullptr);

    return corrMap.release();
}

void DrawEfficiencyAnnotations(const TH2D* valueMap, const TH2D* reco, const TH2D* gen,
                               const char* valueLabel, double textSize = 0.02) {
    if (!valueMap || !reco || !gen || !valueLabel) return;
    TLatex latex;
    latex.SetTextAlign(22);
    latex.SetTextSize(textSize);
    latex.SetTextFont(42);
    for (int ipt = 1; ipt <= N_PT_BINS; ++ipt) {
        double x = 0.5 * (PT_EDGES[ipt-1] + PT_EDGES[ipt]);
        for (int icos = 1; icos <= N_COSTHETA_BINS; ++icos) {
            double y = 0.5 * (COSTHETA_EDGES[icos-1] + COSTHETA_EDGES[icos]);
            double r = reco->GetBinContent(ipt, icos);
            double g = gen->GetBinContent(ipt, icos);
            if (r <= 0.0 && g <= 0.0) continue;
            double val = valueMap->GetBinContent(ipt, icos);
            latex.SetTextColor(kBlack);
            latex.DrawLatex(x, y + 0.035, Form("%s=%.2f", valueLabel, val));
            latex.SetTextColor(kBlue+2);
            latex.DrawLatex(x, y + 0.01, Form("R=%.0f", r));
            latex.SetTextColor(kRed+1);
            latex.DrawLatex(x, y - 0.015, Form("G=%.0f", g));
        }
    }
}

} // namespace

// Structure to hold rho_00 analysis results (renamed from old KinematicBin)
struct Rho00AnalysisBin {
    double ptMin, ptMax;
    double yMin, yMax;
    double centMin, centMax;
    int dcaMode;
    double cosEPMin, cosEPMax;
    double mva;
    
    double yield;
    double yieldErr;
    
    std::string toString() const {
        std::ostringstream oss;
        oss << "pt[" << ptMin << "," << ptMax << "]_"
            << "y[" << yMin << "," << yMax << "]_"
            << "cent[" << centMin << "," << centMax << "]_"
            << "dca" << dcaMode << "_"
            << "cosEP[" << cosEPMin << "," << cosEPMax << "]_"
            << "mva" << mva;
        return oss.str();
    }
    
    double getCosThetaMid() const {
        return 0.5 * (cosEPMin + cosEPMax);
    }
    
    double getCosThetaWidth() const {
        return 0.5 * (cosEPMax - cosEPMin);
    }
    
    double getPtMid() const {
        return 0.5 * (ptMin + ptMax);
    }
};

// Structure to hold rho_00 fit results
struct Rho00Result {
    double rho00;
    double rho00Err;
    double chi2ndf;
    TF1* fitFunc;
    
    // Kinematic bin info
    double ptMin, ptMax;
    double yMin, yMax;
    double centMin, centMax;
    int dcaMode;
    
    Rho00Result() : rho00(-999), rho00Err(-999), chi2ndf(-999), fitFunc(nullptr),
                   ptMin(0), ptMax(0), yMin(0), yMax(0), centMin(0), centMax(0), dcaMode(0) {}
};

// Parse filename to extract kinematic bin info
bool parseFilename(const std::string& filename, Rho00AnalysisBin& bin) {
    // Expected format: hist_PbPb_Data_Oct29_mva0p9_pt30p0_50p0_y0p0_0p3_cent0_10_cosEP-1p00_1p00_mva0p990_inclusive_fitResult.root
    
    size_t pos_pt = filename.find("_pt");
    size_t pos_y = filename.find("_y", pos_pt);
    size_t pos_cent = filename.find("_cent", pos_y);
    size_t pos_cosEP = filename.find("_cosEP", pos_cent);
    size_t pos_mva = filename.find("_mva0p", pos_cosEP);
    size_t pos_dca = filename.find("_inclusive");
    if (pos_dca == std::string::npos) pos_dca = filename.find("_prompt_rich");
    if (pos_dca == std::string::npos) pos_dca = filename.find("_nonprompt_rich");
    
    if (pos_pt == std::string::npos || pos_y == std::string::npos || 
        pos_cent == std::string::npos || pos_cosEP == std::string::npos) {
        return false;
    }
    
    try {
        // Parse pT
        std::string pt_str = filename.substr(pos_pt + 3, pos_y - pos_pt - 3);
        std::replace(pt_str.begin(), pt_str.end(), 'p', '.');
        sscanf(pt_str.c_str(), "%lf_%lf", &bin.ptMin, &bin.ptMax);
        
        // Parse y
        std::string y_str = filename.substr(pos_y + 2, pos_cent - pos_y - 2);
        std::replace(y_str.begin(), y_str.end(), 'p', '.');
        sscanf(y_str.c_str(), "%lf_%lf", &bin.yMin, &bin.yMax);
        
        // Parse centrality
        std::string cent_str = filename.substr(pos_cent + 5, pos_cosEP - pos_cent - 5);
        sscanf(cent_str.c_str(), "%lf_%lf", &bin.centMin, &bin.centMax);
        
        // Parse cosThetaEP - handle negative values with '-' prefix
        // Format: cosEP-1p00_1p00 or cosEP-0p80_-0p60, etc.
        std::string cosEP_raw = filename.substr(pos_cosEP + 6, pos_mva - pos_cosEP - 6);  // Skip "_cosEP"
        std::string cosEP_str = cosEP_raw;
        
        // Find the underscore separator (but not in negative numbers)
        size_t underscore_pos = cosEP_str.find('_');
        if (underscore_pos != std::string::npos) {
            std::string min_str = cosEP_str.substr(0, underscore_pos);
            std::string max_str = cosEP_str.substr(underscore_pos + 1);
            
            // Replace 'p' with '.'
            std::replace(min_str.begin(), min_str.end(), 'p', '.');
            std::replace(max_str.begin(), max_str.end(), 'p', '.');
            
            bin.cosEPMin = std::stod(min_str);
            bin.cosEPMax = std::stod(max_str);
        } else {
            return false;
        }
        
        // Parse MVA
        std::string mva_str = filename.substr(pos_mva + 5, pos_dca - pos_mva - 5);
        std::replace(mva_str.begin(), mva_str.end(), 'p', '.');
        bin.mva = std::stod(mva_str);
        
        // Parse DCA mode
        if (filename.find("_inclusive") != std::string::npos) bin.dcaMode = 0;
        else if (filename.find("_prompt_rich") != std::string::npos) bin.dcaMode = 1;
        else if (filename.find("_nonprompt_rich") != std::string::npos) bin.dcaMode = 2;
        else bin.dcaMode = -1;
        
        return true;
    } catch (...) {
        return false;
    }
}

// Extract yield from fit result file
bool extractYield(const std::string& filepath, double& yield, double& yieldErr) {
    TFile* file = TFile::Open(filepath.c_str(), "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return false;
    }
    
    RooFitResult* fitResult = (RooFitResult*)file->Get("fitResult");
    if (!fitResult) {
        std::cerr << "Cannot find fitResult in " << filepath << std::endl;
        file->Close();
        return false;
    }
    
    RooRealVar* nsig = (RooRealVar*)fitResult->floatParsFinal().find("nsig");
    if (!nsig) {
        std::cerr << "Cannot find nsig parameter in " << filepath << std::endl;
        file->Close();
        return false;
    }
    
    yield = nsig->getVal();
    yieldErr = nsig->getError();
    
    file->Close();
    return true;
}

// Fit cosTheta distribution to extract rho_00
Rho00Result fitCosThetaDistribution(TGraphErrors* graph, const std::string& label) {
    Rho00Result result;
    
    if (!graph || graph->GetN() < 3) {
        std::cerr << "Not enough points for fitting: " << label << std::endl;
        return result;
    }
    
    // Fitting function: N0 * (1 - rho_00 + (3*rho_00 - 1) * cos^2(theta))
    TF1* fitFunc = new TF1(Form("fit_%s", label.c_str()),
                           "[0]*(1 - [1] + (3*[1] - 1)*x*x)", -1.0, 1.0);
    fitFunc->SetParName(0, "N_{0}");
    fitFunc->SetParName(1, "#rho_{00}");
    fitFunc->SetParameter(0, graph->GetMean(2));  // Approximate N0
    fitFunc->SetParameter(1, 1.0/3.0);  // Initial rho_00
    fitFunc->SetParLimits(1, 0.0, 1.0);  // Physical limit
    
    TFitResultPtr fitRes = graph->Fit(fitFunc, "RQS");
    
    if (fitRes.Get() && fitRes->IsValid()) {
        result.rho00 = fitFunc->GetParameter(1);
        result.rho00Err = fitFunc->GetParError(1);
        result.chi2ndf = fitFunc->GetChisquare() / fitFunc->GetNDF();
        result.fitFunc = (TF1*)fitFunc->Clone();
        
        std::cout << label << ": rho_00 = " << result.rho00 << " ± " << result.rho00Err 
                  << ", chi2/ndf = " << result.chi2ndf << std::endl;
    } else {
        std::cerr << "Fit failed for " << label << std::endl;
        delete fitFunc;
    }
    
    return result;
}

// Build pT–cos efficiency correction map from MC efficiency histograms.
// mcKeyOrFile: either a dataset-key-like name (used to guess Corrections/Data path) or a direct ROOT file path
TH2D* BuildPtCosEffCorrMapFromMC(const char* mcKeyOrFile, const char* datasetCatalog, double mvaSelect) {
    using namespace KinematicBinConfig;
    if (mcKeyOrFile && datasetCatalog && IsLikelyDatasetKey(mcKeyOrFile)) {
        if (TH2D* corr = BuildPtCosEffCorrMapFromDatasetKey(mcKeyOrFile, datasetCatalog, mvaSelect)) {
            std::cout << "[BuildPtCosEffCorrMapFromMC] Built pT–cos map directly from dataset key "
                      << mcKeyOrFile << std::endl;
            return corr;
        }
        std::cout << "[BuildPtCosEffCorrMapFromMC] Dataset-based map failed for key " << mcKeyOrFile
                  << ", falling back to file lookup." << std::endl;
    }

    // Try resolve MC efficiency ROOT file
    std::string mcPath = mcKeyOrFile ? std::string(mcKeyOrFile) : std::string("");
    bool fileExists = false;
    if (!mcPath.empty()) {
        if (!gSystem->AccessPathName(mcPath.c_str(), kReadPermission)) fileExists = true;
        if (!fileExists && mcPath.find(".root") == std::string::npos && mcPath.find('/') == std::string::npos) {
            // Guess a standard Corrections output path using the key name
            std::string guess = std::string("/home/jun502s/DstarAna/DStarAnalysis/Corrections/Data/PbPb/") + mcPath + "/output.root";
            if (!gSystem->AccessPathName(guess.c_str(), kReadPermission)) { mcPath = guess; fileExists = true; }
        }
    }
    if (!fileExists) {
        std::cerr << "[BuildPtCosEffCorrMapFromMC] Cannot resolve MC efficiency file from key/path: " << mcKeyOrFile << std::endl;
        return nullptr;
    }

    TFile* f = TFile::Open(mcPath.c_str(), "READ");
    if (!f || f->IsZombie()) { if (f) f->Close(); std::cerr << "[BuildPtCosEffCorrMapFromMC] Failed to open: " << mcPath << std::endl; return nullptr; }

    // Prefer a ready-made ratio if present
    TH2D* hEff = (TH2D*)f->Get("pt_cos_pr_pass_ratio");
    if (!hEff) hEff = (TH2D*)f->Get("pt_cos_pr_ratio");

    // Else build ratio from pass and denom
    if (!hEff) {
        TH2D* hPassMVA = (TH2D*)f->Get("pt_cos_pr_pass_mva0990");
        TH2D* hPass = (TH2D*)f->Get("pt_cos_pr_pass");
        TH2D* hDen = (TH2D*)f->Get("pt_cos_pr");
        TH2D* num = hPassMVA ? hPassMVA : hPass;
        if (num && hDen) {
            hEff = (TH2D*)num->Clone("pt_cos_eff");
            hEff->Divide(hDen);
        }
    }

    if (!hEff) { f->Close(); std::cerr << "[BuildPtCosEffCorrMapFromMC] No suitable efficiency histograms found in " << mcPath << std::endl; return nullptr; }

    // Build correction map on our binning using interpolation: w = 1/eff
    TH2D* hCorr = new TH2D("hEffCorrPtCos", "Eff. correction (1/eff);p_{T} (GeV/c);cos(#theta)",
                           N_PT_BINS, &PT_EDGES[0], N_COSTHETA_BINS, &COSTHETA_EDGES[0]);
    for (int ipt = 1; ipt <= N_PT_BINS; ++ipt) {
        double ptMid = 0.5*(PT_EDGES[ipt-1] + PT_EDGES[ipt]);
        for (int icos = 1; icos <= N_COSTHETA_BINS; ++icos) {
            double cosMid = 0.5*(COSTHETA_EDGES[icos-1] + COSTHETA_EDGES[icos]);
            double eff = hEff->Interpolate(ptMid, cosMid);
            double w = (eff > 1e-6) ? 1.0/eff : 1.0; // protect
            if (w > 1e3) w = 1e3;
            hCorr->SetBinContent(ipt, icos, w);
        }
    }

    f->Close();
    return hCorr;
}

// Apply histogram-based correction (aggregate Data over y,cent; use PromptRich correction)
void DoHistBasedCorrectionAndFit(const char* inputDir,
                                 const char* outputDir,
                                 double mvaSelect,
                                 TH2D* corrMap,
                                 const char* dataDatasetKey) {
    using namespace KinematicBinConfig;
    auto yBins = GetYBins();
    auto centBins = GetCentBins();
    const std::string dataKey = (dataDatasetKey && *dataDatasetKey) ? dataDatasetKey : "dataset";

    gSystem->mkdir(outputDir, true);

    for (int ipt = 0; ipt < N_PT_BINS; ++ipt) {
        double ptMin = PT_EDGES[ipt];
        double ptMax = PT_EDGES[ipt+1];
        TH1D* hCos = new TH1D(Form("hCos_pt%d_prompt", ipt), Form("Corrected Data;cos(#theta);Yield"),
                               N_COSTHETA_BINS, &COSTHETA_EDGES[0]);
        hCos->Sumw2();

        for (int icos = 0; icos < N_COSTHETA_BINS; ++icos) {
            double cosMin = COSTHETA_EDGES[icos];
            double cosMax = COSTHETA_EDGES[icos+1];
            double sumY = 0.0; double sumE2 = 0.0;
            for (const auto& [yMin, yMax] : yBins) {
                for (const auto& [centMin, centMax] : centBins) {
                    std::string fname = BuildHistFileName(dataKey, ptMin, ptMax,
                                                          yMin, yMax, centMin, centMax,
                                                          cosMin, cosMax, mvaSelect,
                                                          PROMPT_RICH, "_fitResult.root");
                    std::string path = std::string(inputDir) + "/" + fname;
                    TFile* f = TFile::Open(path.c_str(), "READ");
                    if (!f || f->IsZombie()) { if (f) f->Close(); continue; }
                    RooFitResult* fr = (RooFitResult*)f->Get("fitResult");
                    if (!fr) { f->Close(); continue; }
                    RooRealVar* nsig = (RooRealVar*)fr->floatParsFinal().find("nsig");
                    if (nsig) { double v = nsig->getVal(); double e = nsig->getError(); sumY += v; sumE2 += e*e; }
                    f->Close();
                }
            }
            double w = GetCorrectionWeight(corrMap, ptMin, ptMax, cosMin, cosMax);
            hCos->SetBinContent(icos+1, sumY * w);
            hCos->SetBinError(icos+1, std::sqrt(sumE2) * w);
        }

        // Fit histogram to extract rho00
        TF1* f = new TF1(Form("f_pt%d", ipt), "[0]*(1 - [1] + (3*[1] - 1)*x*x)", -1.0, 1.0);
        f->SetParName(0, "N0"); f->SetParName(1, "rho00");
        f->SetParameter(0, hCos->GetMaximum()); f->SetParameter(1, 1.0/3.0); f->SetParLimits(1, 0.0, 1.0);
        hCos->Fit(f, "RQ");

        TCanvas* c = new TCanvas(Form("c_corr_pt%d", ipt), "", 800, 600);
        hCos->SetMarkerStyle(20); hCos->SetMarkerSize(1.2);
        hCos->Draw("E1"); f->SetLineColor(kRed); f->SetLineWidth(2); f->Draw("same");
        TLatex tx; tx.SetNDC(); tx.SetTextSize(0.04);
        tx.DrawLatex(0.15, 0.85, Form("p_{T} [%.0f, %.0f] GeV/c (Prompt)", ptMin, ptMax));
        tx.DrawLatex(0.15, 0.80, Form("#rho_{00} = %.3f #pm %.3f", f->GetParameter(1), f->GetParError(1)));
        c->SaveAs(Form("%s/rho00_histcorr_pt%d_prompt.png", outputDir, ipt));
        c->SaveAs(Form("%s/rho00_histcorr_pt%d_prompt.pdf", outputDir, ipt));
        delete c;
    }
}

// Main function to extract and plot rho_00 using KinematicBinConfig
void ExtractRho00FromFits(const char* inputDir = "results/reduced_hist",
                          const char* outputDir = "results/rho00_analysis",
                          double mvaSelect = 0.990,
                          const char* mcDatasetKeyOrFile = "PbPb_MC_PromptRich_Nov9",
                          const char* datasetCatalog = "/home/jun502s/DstarAna/DStarAnalysis/Data/datasets.json",
                          const char* dataDatasetKey = "PbPb_Data_Oct29_mva0p9") {
    
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    const std::string dataKey = (dataDatasetKey && *dataDatasetKey) ? dataDatasetKey : "dataset";
    const std::string outputBase = outputDir ? outputDir : ".";
    const std::string runOutputDir = BuildRunOutputDir(outputBase, dataKey, mvaSelect);
    gSystem->mkdir(runOutputDir.c_str(), true);
    const char* outDir = runOutputDir.c_str();
    
    std::cout << "\n=== Extracting rho_00 from fit results ===" << std::endl;
    std::cout << "Input directory: " << inputDir << std::endl;
    std::cout << "MVA selection: " << mvaSelect << std::endl;
    std::cout << "Using kinematic bins from KinematicBinConfig" << std::endl;
    std::cout << "MC key/file: " << mcDatasetKeyOrFile << std::endl;
    std::cout << "Data dataset key: " << dataKey << std::endl;
    std::cout << "Final output dir: " << runOutputDir << std::endl;
    
    // Get all kinematic bins from config
    auto ptBins = GetPtBins();
    auto yBins = GetYBins();
    auto centBins = GetCentBins();
    auto cosBins = GetCosThetaBins();
    const std::vector<int> activeDcaModes = {PROMPT_RICH};
    std::string corrTag = (mcDatasetKeyOrFile && *mcDatasetKeyOrFile) ? mcDatasetKeyOrFile : "MC";
    std::replace(corrTag.begin(), corrTag.end(), '/', '_');
    std::replace(corrTag.begin(), corrTag.end(), '.', 'p');
    std::replace(corrTag.begin(), corrTag.end(), ' ', '_');

    TH2D* corrMap = BuildPtCosEffCorrMapFromMC(mcDatasetKeyOrFile, datasetCatalog, mvaSelect);
if (corrMap) {
    TCanvas* cCorr = new TCanvas("cCorrMap", "", 800, 700);
    corrMap->SetTitle("Efficiency correction map; p_{T} (GeV/c); cos(#theta)");
    corrMap->Draw("COLZ TEXT45");
    DrawEfficiencyAnnotations(corrMap, gLastRecoMap.get(), gLastGenMap.get(), "w");
    cCorr->SaveAs(Form("%s/effCorr_ptcos_%s.png", outDir, corrTag.c_str()));
    cCorr->SaveAs(Form("%s/effCorr_ptcos_%s.pdf", outDir, corrTag.c_str()));
    TFile fout(Form("%s/effCorr_ptcos_%s.root", outDir, corrTag.c_str()), "RECREATE");
    corrMap->Write();
    if (gLastEfficiencyMap) {
        gLastEfficiencyMap->Write("hEffPtCos");
    }
    fout.Close();
    delete cCorr;
    if (gLastEfficiencyMap) {
        TCanvas* cEff = new TCanvas("cEffMap", "", 800, 700);
        gLastEfficiencyMap->SetTitle("Efficiency map; p_{T} (GeV/c); cos(#theta)");
        gLastEfficiencyMap->Draw("COLZ TEXT45");
        DrawEfficiencyAnnotations(gLastEfficiencyMap.get(), gLastRecoMap.get(), gLastGenMap.get(), "eff");
        cEff->SaveAs(Form("%s/effMap_ptcos_%s.png", outDir, corrTag.c_str()));
        cEff->SaveAs(Form("%s/effMap_ptcos_%s.pdf", outDir, corrTag.c_str()));
        delete cEff;
    }
}
    
    std::cout << "\n=== Kinematic Bin Configuration ===" << std::endl;
    std::cout << "pT bins: " << N_PT_BINS << " bins" << std::endl;
    std::cout << "|y| bins: " << N_Y_BINS << " bins" << std::endl;
    std::cout << "Centrality bins: " << N_CENT_BINS << " bins" << std::endl;
    std::cout << "cos(theta) bins: " << N_COSTHETA_BINS << " bins" << std::endl;
    std::cout << "DCA modes: " << N_DCA_MODES << " modes" << std::endl;
    
    // Store rho_00 results for final plots
    // Map: bin name (includes pT, y, cent, cos, DCA) -> (yield, error)
    std::map<std::string, std::pair<double, double>> rho00_results;
    
    // Iterate over all kinematic bins defined in KinematicBinConfig
    int totalBins = ptBins.size() * yBins.size() * centBins.size() * cosBins.size() * activeDcaModes.size();
    int processedBins = 0;
    
    std::cout << "\n=== Processing " << totalBins << " kinematic bins ===" << std::endl;
    
    // Loop over DCA modes
    for (int dcaIdx : activeDcaModes) {
        // Loop over kinematic bins
        for (const auto& [ptMin, ptMax] : ptBins) {
            for (const auto& [yMin, yMax] : yBins) {
                for (const auto& [centMin, centMax] : centBins) {
                    for (const auto& [cosMin, cosMax] : cosBins) {
                        processedBins++;
                        
                        // Build filename for this kinematic bin
                        std::string binLabel = BuildHistFileName(dataKey,
                                                                 ptMin, ptMax,
                                                                 yMin, yMax,
                                                                 centMin, centMax,
                                                                 cosMin, cosMax,
                                                                 mvaSelect,
                                                                 dcaIdx,
                                                                 "_fitResult.root");
                        
                        std::string filepath = std::string(inputDir) + "/" + binLabel;
                        
                        // Try to open the file
                        TFile* file = TFile::Open(filepath.c_str(), "READ");
                        if (!file || file->IsZombie()) {
                            if (processedBins % 100 == 0) {
                                std::cout << "  [" << processedBins << "/" << totalBins << "] "
                                         << "File not found: " << binLabel << std::endl;
                            }
                            continue;
                        }
                        
                        // Extract fit result
                        RooFitResult* fitResult = (RooFitResult*)file->Get("fitResult");
                        if (!fitResult) {
                            file->Close();
                            continue;
                        }
                        
                        // Get yield
                        RooRealVar* nsig = (RooRealVar*)fitResult->floatParsFinal().find("nsig");
                        if (!nsig) {
                            file->Close();
                            continue;
                        }
                        
                        double yield = nsig->getVal();
                        double yieldErr = nsig->getError();
                        double w = GetCorrectionWeight(corrMap, ptMin, ptMax, cosMin, cosMax);
                        yield *= w;
                        yieldErr *= w;
                        
                        file->Close();
                        
                        // Store result for this kinematic bin
                        double ptMid = 0.5 * (ptMin + ptMax);
                        double yMid = 0.5 * (yMin + yMax);
                        double centMid = 0.5 * (centMin + centMax);
                        double cosMid = 0.5 * (cosMin + cosMax);
                        
                        // For now, store yield as a proxy (will be used to create graph)
                        // In actual analysis, would accumulate yields vs cosTheta
                        // and fit to extract rho_00
                    }
                }
            }
        }
    }
    
    // Now do the actual rho_00 extraction by organizing yields by kinematic category
    std::cout << "\n=== Creating yield graphs and extracting rho_00 ===" << std::endl;
    
    // Create plots organized by (pT, y, cent, DCA)
    // Map: (ptIdx, yIdx, centIdx, dcaIdx) -> TGraphErrors for cos bins
    std::map<std::tuple<int, int, int, int>, TGraphErrors*> cosGraphs;
    
    // Loop over all kinematic combinations
    for (int dcaIdx : activeDcaModes) {
        for (int ptIdx = 0; ptIdx < (int)ptBins.size(); ++ptIdx) {
            for (int yIdx = 0; yIdx < (int)yBins.size(); ++yIdx) {
                for (int centIdx = 0; centIdx < (int)centBins.size(); ++centIdx) {
                    double ptMin = PT_EDGES[ptIdx];
                    double ptMax = PT_EDGES[ptIdx + 1];
                    double yMin = Y_EDGES[yIdx];
                    double yMax = Y_EDGES[yIdx + 1];
                    double centMin = CENT_EDGES[centIdx];
                    double centMax = CENT_EDGES[centIdx + 1];
                    
                    // Create graph for this kinematic bin across cos bins
                    TGraphErrors* graph = new TGraphErrors();
                    auto graphKey = std::make_tuple(ptIdx, yIdx, centIdx, dcaIdx);
                    cosGraphs[graphKey] = graph;
                    
                    int pointCount = 0;
                    
                    // Loop over cosTheta sub-bins to collect yields
                    for (int cosIdx = 0; cosIdx < N_COSTHETA_BINS; ++cosIdx) {
                        double cosMin = COSTHETA_EDGES[cosIdx];
                        double cosMax = COSTHETA_EDGES[cosIdx + 1];
                        double cosMid = 0.5 * (cosMin + cosMax);
                        double cosWidth = 0.5 * (cosMax - cosMin);
                        
                        // Build filename for this specific cosTheta bin
                        std::string binLabel = BuildHistFileName(dataKey,
                                                                 ptMin, ptMax,
                                                                 yMin, yMax,
                                                                 centMin, centMax,
                                                                 cosMin, cosMax,
                                                                 mvaSelect,
                                                                 dcaIdx,
                                                                 "_fitResult.root");
                        
                        std::string filepath = std::string(inputDir) + "/" + binLabel;
                        
                        // Try to open and extract yield
                        TFile* file = TFile::Open(filepath.c_str(), "READ");
                        if (!file || file->IsZombie()) {
                            continue;
                        }
                        
                        RooFitResult* fitResult = (RooFitResult*)file->Get("fitResult");
                        if (!fitResult) {
                            file->Close();
                            continue;
                        }
                        
                        RooRealVar* nsig = (RooRealVar*)fitResult->floatParsFinal().find("nsig");
                        if (!nsig) {
                            file->Close();
                            continue;
                        }
                        
                        double yield = nsig->getVal();
                        double yieldErr = nsig->getError();
                        
                        file->Close();
                        
                        // Add point to graph
                        graph->SetPoint(pointCount, cosMid, yield);
                        graph->SetPointError(pointCount, cosWidth, yieldErr);
                        pointCount++;
                        
                        // Store rho_00 result with full bin information
                        std::ostringstream binName;
                        binName << "rho00_pt" << FormatValP(ptMin, 1) << "_" << FormatValP(ptMax, 1)
                               << "_y" << FormatValP(yMin, 1) << "_" << FormatValP(yMax, 1)
                               << "_cent" << FormatValP(centMin, 0) << "_" << FormatValP(centMax, 0)
                               << "_cos" << FormatValP(cosMin, 2) << "_" << FormatValP(cosMax, 2)
                               << "_" << DCAModeToString(dcaIdx);
                        
                        rho00_results[binName.str()] = {yield, yieldErr};
                    }
                }
            }
        }
    }
    
    // Create summary plots
    std::cout << "\n=== Creating summary plots ===" << std::endl;
    
    int canvasCount = 0;
    std::vector<Rho00Result> fitResults;
    
    for (const auto& [graphKey, graph] : cosGraphs) {
        if (graph->GetN() < 3) continue; // Need at least 3 points for fitting
        
        auto [ptIdx, yIdx, centIdx, dcaIdx] = graphKey;
        double ptMin = PT_EDGES[ptIdx];
        double ptMax = PT_EDGES[ptIdx + 1];
        double yMin = Y_EDGES[yIdx];
        double yMax = Y_EDGES[yIdx + 1];
        double centMin = CENT_EDGES[centIdx];
        double centMax = CENT_EDGES[centIdx + 1];
        
        // Create label with proper DCA mode naming
        std::string dcaLabel = DCAModeToString(dcaIdx);  // inclusive, prompt_rich, nonprompt_rich
        std::string label = "pt" + std::to_string((int)ptMin) + "_" + std::to_string((int)ptMax)
                          + "_y" + FormatValP(yMin, 1) + "_" + FormatValP(yMax, 1)
                          + "_cent" + std::to_string((int)centMin) + "_" + std::to_string((int)centMax)
                          + "_" + dcaLabel;
        
        // Fit cosTheta distribution to extract rho_00
        Rho00Result fitResult = fitCosThetaDistribution(graph, label);
        
        if (fitResult.fitFunc) {
            // Store kinematic bin info
            fitResult.ptMin = ptMin;
            fitResult.ptMax = ptMax;
            fitResult.yMin = yMin;
            fitResult.yMax = yMax;
            fitResult.centMin = centMin;
            fitResult.centMax = centMax;
            fitResult.dcaMode = dcaIdx;
            
            fitResults.push_back(fitResult);
            
            // Create canvas
            TCanvas* c = new TCanvas(Form("c_%s", label.c_str()), label.c_str(), 800, 600);
            c->SetLeftMargin(0.12);
            c->SetRightMargin(0.05);
            
            graph->SetMarkerStyle(20);
            graph->SetMarkerSize(1.2);
            graph->SetMarkerColor(kBlack);
            graph->SetLineColor(kBlack);
            graph->SetTitle(Form("Yield vs cos(#theta);cos(#theta);Yield"));
            graph->Draw("APE");
            
            // Draw fit function
            fitResult.fitFunc->SetLineColor(kRed);
            fitResult.fitFunc->SetLineWidth(2);
            fitResult.fitFunc->Draw("same");
            
            // Add labels
            TLatex tex;
            tex.SetNDC();
            tex.SetTextSize(0.04);
            tex.DrawLatex(0.15, 0.85, "D*^{#pm} #rightarrow D^{0}#pi^{#pm}");
            tex.DrawLatex(0.15, 0.80, Form("p_{T}: [%.0f, %.0f] GeV/c", ptMin, ptMax));
            tex.DrawLatex(0.15, 0.75, Form("|y|: [%.1f, %.1f]", yMin, yMax));
            tex.DrawLatex(0.15, 0.70, Form("Cent: [%.0f, %.0f]%%", centMin, centMax));
            tex.DrawLatex(0.15, 0.65, Form("DCA: %s", dcaLabel.c_str()));
            tex.DrawLatex(0.15, 0.60, Form("MVA > %.3f", mvaSelect));
            
            tex.SetTextColor(kRed);
            tex.DrawLatex(0.60, 0.85, Form("#rho_{00} = %.3f #pm %.3f", 
                                           fitResult.rho00, fitResult.rho00Err));
            tex.DrawLatex(0.60, 0.80, Form("#chi^{2}/ndf = %.2f", fitResult.chi2ndf));
            tex.SetTextColor(kBlack);
            tex.DrawLatex(0.60, 0.75, "N_{0}(1 - #rho_{00} + (3#rho_{00} - 1)cos^{2}#theta)");
            
            // Add horizontal line at 1/3 (unpolarized)
            TLine* line = new TLine(-1.0, 1.0/3.0, 1.0, 1.0/3.0);
            line->SetLineStyle(2);
            line->SetLineColor(kBlue);
            line->Draw();
            
            c->SaveAs(Form("%s/yield_vs_cos_%s.png", outDir, label.c_str()));
            c->SaveAs(Form("%s/yield_vs_cos_%s.pdf", outDir, label.c_str()));
            
            delete c;
            canvasCount++;
        }
    }
    
    std::cout << "Created " << canvasCount << " yield vs cos(theta) plots with rho_00 fits" << std::endl;
    
    // ========== NEW: Create rho_00 vs pT graphs (separate rapidity and centrality) ==========
    std::cout << "\n=== Creating rho_00 vs pT graphs (rapidity and centrality separate) ===" << std::endl;
    
    // Map: (yIdx, centIdx, dcaMode) -> vector of (ptMid, rho00, rho00Err, ptHalfWidth)
    std::map<std::tuple<int, int, int>, std::vector<std::tuple<double, double, double, double>>> rho00ByPt;
    
    // Collect rho_00 results
    for (const auto& fitResult : fitResults) {
        int yIdx = -1, centIdx = -1, ptIdx = -1;
        
        for (int i = 0; i < (int)ptBins.size(); ++i) {
            if (std::abs(PT_EDGES[i] - fitResult.ptMin) < 0.01) ptIdx = i;
        }
        for (int i = 0; i < (int)yBins.size(); ++i) {
            if (std::abs(Y_EDGES[i] - fitResult.yMin) < 0.01) yIdx = i;
        }
        for (int i = 0; i < (int)centBins.size(); ++i) {
            if (std::abs(CENT_EDGES[i] - fitResult.centMin) < 0.01) centIdx = i;
        }
        
        if (ptIdx >= 0 && yIdx >= 0 && centIdx >= 0) {
            double ptMid = 0.5 * (fitResult.ptMin + fitResult.ptMax);
            double ptHalfWidth = 0.5 * (fitResult.ptMax - fitResult.ptMin);
            auto key = std::make_tuple(yIdx, centIdx, fitResult.dcaMode);
            rho00ByPt[key].push_back(std::make_tuple(ptMid, fitResult.rho00, fitResult.rho00Err, ptHalfWidth));
        }
    }
    
    // Sort all data by pT
    for (auto& [key, data] : rho00ByPt) {
        std::sort(data.begin(), data.end(),
            [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
    }
    
    const int rapidityColors[] = {kAzure + 4, kAzure - 2, kBlue + 2};
    const int rapidityMarkers[] = {21, 20, 33};
    const double rapidityMarkerSizes[] = {1.4, 1.4, 1.6};
    const int centralityColors[] = {kRed - 4, kOrange + 7, kAzure + 1};
    const int centralityMarkers[] = {21, 25, 20};
    const double centralityMarkerSizes[] = {1.5, 1.4, 1.5};
    int ptCanvasCount = 0;
    
    // ===== CASE 1: rho_00 vs pT with varying rapidity (centrality fixed) =====
    for (int dcaMode : activeDcaModes) {
        for (int centIdx = 0; centIdx < N_CENT_BINS; ++centIdx) {
            TCanvas* c = new TCanvas(Form("rho00_vs_pt_vs_y_%d_%d", dcaMode, centIdx), "", 900, 700);
            c->SetGridx();
            c->SetGridy();
            c->SetLeftMargin(0.12);
            c->SetBottomMargin(0.12);
            
            std::string dcaLabel = KinematicBinConfig::DCAModeToString(dcaMode);
            
            bool firstGraph = true;
            std::vector<std::unique_ptr<TBox>> bandBoxes;
            
            // Draw all rapidity bins (with fixed centrality)
            for (int yIdx = 0; yIdx < N_Y_BINS; ++yIdx) {
                auto key = std::make_tuple(yIdx, centIdx, dcaMode);
                auto it = rho00ByPt.find(key);
                
                if (it != rho00ByPt.end() && it->second.size() >= 2) {
                    auto& data = it->second;
                    
                    TGraphErrors* gr = new TGraphErrors(data.size());
                    for (size_t i = 0; i < data.size(); ++i) {
                        double ptMid = std::get<0>(data[i]);
                        double rho00 = std::get<1>(data[i]);
                        double rho00Err = std::get<2>(data[i]);
                        double ptHalfWidth = std::get<3>(data[i]);
                        gr->SetPoint(i, ptMid, rho00);
                        gr->SetPointError(i, ptHalfWidth, rho00Err);
                        auto box = std::make_unique<TBox>(ptMid - ptHalfWidth, rho00 - rho00Err,
                                                          ptMid + ptHalfWidth, rho00 + rho00Err);
                        int fillColor = rapidityColors[yIdx];
                        box->SetFillColorAlpha(fillColor, 0.25);
                        box->SetLineColor(fillColor);
                        box->SetLineWidth(1);
                        box->Draw("same");
                        bandBoxes.push_back(std::move(box));
                    }
                    
                    gr->SetMarkerStyle(rapidityMarkers[yIdx]);
                    gr->SetMarkerColor(rapidityColors[yIdx]);
                    gr->SetLineColor(rapidityColors[yIdx]);
                    gr->SetMarkerSize(rapidityMarkerSizes[yIdx]);
                    gr->SetLineWidth(2);
                    
                    if (firstGraph) {
                        gr->Draw("AEP");
                        gr->GetXaxis()->SetTitle("p_{T} (GeV/c)");
                        gr->GetYaxis()->SetTitle("#rho_{00}");
                        gr->GetXaxis()->SetLimits(0, 50);
                        gr->GetYaxis()->SetRangeUser(0, 0.6);
                        firstGraph = false;
                    } else {
                        gr->Draw("EP SAME");
                    }
                }
            }
            
            // Reference line
            TLine* line = new TLine(0, 1.0/3.0, 50, 1.0/3.0);
            line->SetLineStyle(7);
            line->SetLineWidth(2);
            line->SetLineColorAlpha(kGray + 1, 0.8);
            line->Draw();
            
            // Title
            TLatex title;
            title.SetNDC();
            title.SetTextFont(62);
            title.SetTextSize(0.04);
            title.DrawLatex(0.15, 0.96, Form("D*^{#pm} - %s, Cent [%.0f,%.0f]%%", 
                dcaLabel.c_str(), CENT_EDGES[centIdx], CENT_EDGES[centIdx+1]));
            TLatex cms;
            cms.SetNDC();
            cms.SetTextFont(62);
            cms.SetTextSize(0.045);
            cms.DrawLatex(0.15, 0.90, "CMS");
            cms.SetTextFont(42);
            cms.SetTextSize(0.035);
            cms.DrawLatex(0.15, 0.85, "#it{PbPb}, #sqrt{s_{NN}} = 5.32 TeV");
            cms.DrawLatex(0.15, 0.80, "Prompt D*^{#pm}");
            cms.DrawLatex(0.15, 0.75, "Event plane");
            
            // Legend with proper colors
            TLegend* leg = new TLegend(0.62, 0.68, 0.92, 0.92);
            leg->SetBorderSize(0);
            leg->SetFillStyle(0);
            leg->SetTextSize(0.035);
            leg->SetHeader("|y| ranges", "C");
            
            // Create persistent dummy graphs for legend
            std::vector<TGraph*> legGraphs;
            for (int i = 0; i < N_Y_BINS; ++i) {
                TGraph* dummy = new TGraph();
                dummy->SetMarkerStyle(rapidityMarkers[i]);
                dummy->SetMarkerColor(rapidityColors[i]);
                dummy->SetLineColor(rapidityColors[i]);
                dummy->SetMarkerSize(rapidityMarkerSizes[i]);
                legGraphs.push_back(dummy);
                leg->AddEntry(dummy, Form("[%.1f, %.1f]", Y_EDGES[i], Y_EDGES[i+1]), "P");
            }
            leg->Draw();
            
            std::string label = Form("rho00_vs_pt_vs_y_%s_cent%d_%d", 
                dcaLabel.c_str(), (int)CENT_EDGES[centIdx], (int)CENT_EDGES[centIdx+1]);
            c->SaveAs(Form("%s/%s.png", outDir, label.c_str()));
            c->SaveAs(Form("%s/%s.pdf", outDir, label.c_str()));
            
            delete c;
            ptCanvasCount++;
        }
    }
    
    // ===== CASE 2: rho_00 vs pT with varying centrality (rapidity fixed) =====
    for (int dcaMode : activeDcaModes) {
        for (int yIdx = 0; yIdx < N_Y_BINS; ++yIdx) {
            TCanvas* c = new TCanvas(Form("rho00_vs_pt_vs_cent_%d_%d", dcaMode, yIdx), "", 900, 700);
            c->SetGridx();
            c->SetGridy();
            c->SetLeftMargin(0.12);
            c->SetBottomMargin(0.12);
            
            std::string dcaLabel = KinematicBinConfig::DCAModeToString(dcaMode);
            
            bool firstGraph = true;
            std::vector<std::unique_ptr<TBox>> bandBoxes;
            
            // Draw all centrality bins (with fixed rapidity)
            for (int centIdx = 0; centIdx < N_CENT_BINS; ++centIdx) {
                auto key = std::make_tuple(yIdx, centIdx, dcaMode);
                auto it = rho00ByPt.find(key);
                
                if (it != rho00ByPt.end() && it->second.size() >= 2) {
                    auto& data = it->second;
                    
                    TGraphErrors* gr = new TGraphErrors(data.size());
                    for (size_t i = 0; i < data.size(); ++i) {
                        double ptMid = std::get<0>(data[i]);
                        double rho00 = std::get<1>(data[i]);
                        double rho00Err = std::get<2>(data[i]);
                        double ptHalfWidth = std::get<3>(data[i]);
                        gr->SetPoint(i, ptMid, rho00);
                        gr->SetPointError(i, ptHalfWidth, rho00Err);
                        auto box = std::make_unique<TBox>(ptMid - ptHalfWidth, rho00 - rho00Err,
                                                          ptMid + ptHalfWidth, rho00 + rho00Err);
                        int fillColor = centralityColors[centIdx];
                        box->SetFillColorAlpha(fillColor, 0.25);
                        box->SetLineColor(fillColor);
                        box->SetLineWidth(1);
                        box->Draw("same");
                        bandBoxes.push_back(std::move(box));
                    }
                    
                    gr->SetMarkerStyle(centralityMarkers[centIdx]);
                    gr->SetMarkerColor(centralityColors[centIdx]);
                    gr->SetLineColor(centralityColors[centIdx]);
                    gr->SetMarkerSize(centralityMarkerSizes[centIdx]);
                    gr->SetLineWidth(2);
                    
                    if (firstGraph) {
                        gr->Draw("AEP");
                        gr->GetXaxis()->SetTitle("p_{T} (GeV/c)");
                        gr->GetYaxis()->SetTitle("#rho_{00}");
                        gr->GetXaxis()->SetLimits(0, 50);
                        gr->GetYaxis()->SetRangeUser(0, 0.6);
                        firstGraph = false;
                    } else {
                        gr->Draw("EP SAME");
                    }
                }
            }
            
            // Reference line
            TLine* line = new TLine(0, 1.0/3.0, 50, 1.0/3.0);
            line->SetLineStyle(7);
            line->SetLineWidth(2);
            line->SetLineColorAlpha(kGray + 1, 0.8);
            line->Draw();
            
            // Title
            TLatex title;
            title.SetNDC();
            title.SetTextFont(62);
            title.SetTextSize(0.04);
            title.DrawLatex(0.15, 0.96, Form("D*^{#pm} - %s, |y| [%.1f,%.1f]", 
                dcaLabel.c_str(), Y_EDGES[yIdx], Y_EDGES[yIdx+1]));
            TLatex cms;
            cms.SetNDC();
            cms.SetTextFont(62);
            cms.SetTextSize(0.045);
            cms.DrawLatex(0.15, 0.90, "CMS");
            cms.SetTextFont(42);
            cms.SetTextSize(0.035);
            cms.DrawLatex(0.15, 0.85, "#it{PbPb}, #sqrt{s_{NN}} = 5.32 TeV");
            cms.DrawLatex(0.15, 0.80, "Prompt D*^{#pm}");
            cms.DrawLatex(0.15, 0.75, "Event plane");
            
            // Legend with proper colors
            TLegend* leg = new TLegend(0.62, 0.68, 0.92, 0.92);
            leg->SetBorderSize(0);
            leg->SetFillStyle(0);
            leg->SetTextSize(0.035);
            leg->SetHeader("Centrality", "C");
            
            // Create persistent dummy graphs for legend
            std::vector<TGraph*> legGraphs;
            for (int i = 0; i < N_CENT_BINS; ++i) {
                TGraph* dummy = new TGraph();
                dummy->SetMarkerStyle(centralityMarkers[i]);
                dummy->SetMarkerColor(centralityColors[i]);
                dummy->SetLineColor(centralityColors[i]);
                dummy->SetMarkerSize(centralityMarkerSizes[i]);
                legGraphs.push_back(dummy);
                leg->AddEntry(dummy, Form("[%.0f, %.0f]%%", CENT_EDGES[i], CENT_EDGES[i+1]), "P");
            }
            leg->Draw();
            
            std::string label = Form("rho00_vs_pt_vs_cent_%s_y%d_%d", 
                dcaLabel.c_str(), (int)(Y_EDGES[yIdx]*10), (int)(Y_EDGES[yIdx+1]*10));
            c->SaveAs(Form("%s/%s.png", outDir, label.c_str()));
            c->SaveAs(Form("%s/%s.pdf", outDir, label.c_str()));
            
            delete c;
            ptCanvasCount++;
        }
    }
    
    std::cout << "Created " << ptCanvasCount << " rho_00 vs pT canvases (rapidity & centrality separated)" << std::endl;
    
    // Histogram-based correction and fit (aggregated y,cent; PromptRich)
DoHistBasedCorrectionAndFit(inputDir, outDir, mvaSelect, corrMap, dataDatasetKey);

std::cout << "\n=== Results Summary ===" << std::endl;
    std::cout << "Total kinematic bins processed: " << processedBins << std::endl;
    std::cout << "Successful bin extractions: " << rho00_results.size() << std::endl;
    std::cout << "Successful rho_00 fits: " << fitResults.size() << std::endl;
    
    // Save rho_00 fitting results to text file
    std::ofstream rho00File(Form("%s/rho00_fit_results_mva%.3f.txt", outDir, mvaSelect));
    rho00File << "# Rho_00 fitting results from KinematicBinConfig\n";
    rho00File << "# Columns: pt_min pt_max y_min y_max cent_min cent_max dca_mode rho00 rho00Err chi2ndf\n";
    
    for (const auto& fitResult : fitResults) {
        rho00File << std::fixed << std::setprecision(3) << fitResult.rho00 << " " 
                  << fitResult.rho00Err << " " << fitResult.chi2ndf << "\n";
    }
    rho00File.close();
    
    // Save yield results to text file (as before)
    std::ofstream outFile(Form("%s/yield_results_mva%.3f.txt", outDir, mvaSelect));
    outFile << "# Yield extraction results from KinematicBinConfig (cos sub-bins)\n";
    outFile << "# Columns: binName yield yieldErr\n";
    
    for (const auto& [binName, result] : rho00_results) {
        outFile << std::fixed << std::setprecision(3)
                << binName << " " << result.first << " " << result.second << "\n";
    }
    outFile.close();
    
    // Also create a summary with counts
    std::cout << "\n=== Extraction Summary ===" << std::endl;
    std::cout << "Total successful extractions: " << rho00_results.size() << " / " << totalBins << std::endl;
    if (rho00_results.size() > 0) {
        std::cout << "Sample results (first 5):\n";
        int count = 0;
        for (const auto& [binName, result] : rho00_results) {
            std::cout << "  " << binName << " -> yield=" << result.first << "+/-" << result.second << "\n";
            if (++count >= 5) break;
        }
    }
    
    std::cout << "\n=== Complete ===" << std::endl;
    std::cout << "Output directory: " << runOutputDir << std::endl;
    std::cout << "Yield results saved to: yield_results_mva" << mvaSelect << ".txt" << std::endl;
    std::cout << "Rho_00 fit results saved to: rho00_fit_results_mva" << mvaSelect << ".txt" << std::endl;
    std::cout << "Plots saved to:" << std::endl;
    std::cout << "  - yield_vs_cos_*.png and *.pdf (rho_00 vs cos(EP) for each kinematic bin)" << std::endl;
    std::cout << "  - rho00_vs_pt_*.png and *.pdf (rho_00 vs pT for fixed y and centrality)" << std::endl;
}
