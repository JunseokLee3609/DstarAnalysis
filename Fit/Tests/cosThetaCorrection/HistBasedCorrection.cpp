// Replace the existing CalSplotweight function with histogram-based correction functions

#include "../../fit/Opt.h"
#include "../../fit/DStarFitConfig.h"
#include "../../fit/ErrorHandler.h"
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TF1.h>
#include <TFitResultPtr.h>
#include "../../Common/Analysis/SimpleDatasetManager.h"
#include "../../Tools/Utils/JSONLoader.h"
#include <TGraphErrors.h>
#include <TLine.h>
#include <TMultiGraph.h>
#include <TMarker.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TParameter.h>
#include <TTree.h>
#include <TLatex.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include "../../fit/DCAFitter.h"
#include "../../Tools/Transformations.h"

#if __cplusplus > 201703L && defined(__GNUG__)
#  if defined(__has_include)
#    if __has_include(<ext/codecvt_specializations.h>)
// GCC removes std::codecvt facets in C++20; re-export the compatibility
// specialisations so <filesystem> continues to compile under cling.
#      include <ext/codecvt_specializations.h>
namespace std {
    using __gnu_cxx::codecvt_utf8;
    using __gnu_cxx::codecvt_utf16;
    using __gnu_cxx::codecvt_utf8_utf16;
}
#    endif
#  endif
#endif

// #include <filesystem>
#include <RooAddPdf.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>
#include <RooStats/SPlot.h>
#include <RooArgSet.h>
#include <RooArgList.h>
#include <RooFitResult.h>
#include "../../interface/simpleDMC.h"
#include "../../interface/simpleAlgos.hxx"
#include "../../Tools/ConfigManagerPP.h"
// #include <TSeq.h>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <algorithm>
#include <cmath> // for fabs
#include <optional>
#include <limits>
#include <tuple>
#include <fstream>
#include <iomanip>

namespace {

std::string JoinPath(const std::string& lhs, const std::string& rhs) {
    if (lhs.empty()) return rhs;
    if (rhs.empty()) return lhs;
    const bool lhsEndsWithSlash = lhs.back() == '/' || lhs.back() == '\\';
    const bool rhsStartsWithSlash = rhs.front() == '/' || rhs.front() == '\\';
    if (lhsEndsWithSlash && rhsStartsWithSlash) {
        return lhs + rhs.substr(1);
    }
    if (!lhsEndsWithSlash && !rhsStartsWithSlash) {
        return lhs + '/' + rhs;
    }
    return lhs + rhs;
}

std::string ParentDir(const std::string& path) {
    if (path.empty()) return {};
    const auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return {};
    if (pos == 0) return std::string("/");
    return path.substr(0, pos);
}

std::string ScriptDirectory() {
    static const std::string dir = ParentDir(__FILE__);
    return dir;
}

std::string FitRootDirectory() {
    static const std::string fitDir = ParentDir(ParentDir(ScriptDirectory()));
    return fitDir;
}

std::string DefaultDatasetConfigPath() {
    return JoinPath(FitRootDirectory(), "Common/Analysis/datasets.json");
}

std::string DefaultEfficiencyCatalogPath() {
    const std::string projectRoot = ParentDir(FitRootDirectory());
    return JoinPath(projectRoot, "Data/datasets.json");
}

void ConfigureGlobalDatasetManager() {
    if (gDatasetManager.GetConfigFile().empty()) {
        gDatasetManager.SetConfigFile(DefaultDatasetConfigPath(), false);
    }
    if (!gDatasetManager.IsLoaded()) {
        gDatasetManager.EnsureLoaded();
    }
}

std::string LoadDatasetFromCatalog(const std::string& catalogPath, const std::string& datasetKey) {
    if (catalogPath.empty() || datasetKey.empty()) {
        return {};
    }
    try {
        const auto info = JsonLoader::LoadObjectMap(catalogPath, "datasets", datasetKey);
        const auto it = info.find("file");
        if (it != info.end()) {
            return it->second;
        }
        std::cerr << "[DatasetCatalog] dataset '" << datasetKey << "' missing 'file' entry in " << catalogPath << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "[DatasetCatalog] " << ex.what() << std::endl;
    }
    return {};
}


inline double projectCosTheta(double cosTheta, bool useAbsTheta) {
    return useAbsTheta ? std::abs(cosTheta) : cosTheta;
}

inline std::string cosAxisLabel(bool useAbsTheta) {
    return useAbsTheta ? "|cos#theta|" : "cos#theta";
}

inline std::string cosDisplayLabel(bool useAbsTheta) {
    return useAbsTheta ? "||cos#theta||" : "cos#theta";
}

} // namespace

struct FitResultSummary {
    bool success = false;
    double signalYield = 0.0;
    double signalError = 0.0;
    double backgroundYield = 0.0;
    std::unique_ptr<RooWorkspace> workspace; // cloned workspace for optional downstream use
    std::string filePath;
};

enum class ProductionType {
    Prompt,
    NonPrompt
};

std::string ToString(ProductionType type) {
    switch (type) {
        case ProductionType::Prompt: return "prompt";
        case ProductionType::NonPrompt: return "nonprompt";
    }
    return "unknown";
}

std::string PrettyLabel(ProductionType type) {
    switch (type) {
        case ProductionType::Prompt: return "Prompt";
        case ProductionType::NonPrompt: return "Nonprompt";
    }
    return "Unknown";
}

bool PassProductionSelection(ProductionType type, int ancestorFlavor) {
    switch (type) {
        case ProductionType::Prompt:
            return ancestorFlavor != 5;
        case ProductionType::NonPrompt:
            return ancestorFlavor == 5;
    }
    return false;
}

struct ChannelHistograms {
    std::vector<TH1D*> corrected;
    std::vector<TH1D*> correctedCB;
    std::vector<TH1D*> weights;
    std::vector<TH1D*> weightsCB;
    std::vector<TH1D*> raw;
    std::vector<TH1D*> match;
    std::vector<TH1D*> matchCorrected;
    std::vector<TH1D*> rawOverMatch;
    std::vector<TH1D*> genCounts;
    std::vector<TF1*>   cosThetaFits;
    std::vector<double> rho00;
    std::vector<double> rho00Err;
};

struct PromptFractionScanResult {
    bool success = false;
    double bestScale = std::numeric_limits<double>::quiet_NaN();
    double bestChi2OverNdf = std::numeric_limits<double>::quiet_NaN();
    double bestPromptFraction = 1.0;        // fraction between 0 and 1
    double bestPromptFractionError = 0.0;   // absolute error on fraction
    std::vector<double> scales;
    std::vector<double> chi2Values;
    std::vector<double> ndfValues;
    std::vector<double> promptFractions;    // fractions per scale (0-1)
    std::vector<double> promptFractionErrors;
    std::string outputDirectory;
};

struct McGenRho00Result {
    bool success = false;
    std::vector<double> rho00;
    std::vector<double> rho00Err;
};

std::map<ProductionType, McGenRho00Result>
ExtractMcGenRho00(const std::string& mcFilePath,
                  const std::vector<std::pair<double, double>>& ptBins,
                  const std::string& outputDir = std::string(),
                  double maxAbsRapidity = 1.0,
                  bool useAbsTheta = true) {
    std::map<ProductionType, McGenRho00Result> results;


    auto appendPathLocal = [](const std::string& base, const std::string& piece) {
        if (piece.empty()) {
            return base;
        }
        std::size_t first = piece.find_first_not_of("/\\");
        std::string cleaned = (first == std::string::npos) ? std::string() : piece.substr(first);
        if (cleaned.empty()) {
            return base;
        }
        if (base.empty()) {
            return cleaned;
        }
        std::string result = base;
        if (result.back() != '/' && result.back() != '\\') {
            result.push_back('/');
        }
        result += cleaned;
        return result;
    };

    std::string baseOutputDir;
    if (!outputDir.empty()) {
        baseOutputDir = appendPathLocal(outputDir, "mc_gen");
        gSystem->mkdir(baseOutputDir.c_str(), true);
    }

    if (mcFilePath.empty()) {
        std::cerr << "[MC GEN] Warning: empty MC file path provided." << std::endl;
        return results;
    }

    const std::string axisLabel = cosAxisLabel(useAbsTheta);
    const std::string displayLabel = cosDisplayLabel(useAbsTheta);
    const std::string axisLabelHx = useAbsTheta ? "|cos#theta_{HX}|" : "cos#theta_{HX}";

    std::unique_ptr<TFile> mcFile(TFile::Open(mcFilePath.c_str(), "READ"));
    if (!mcFile || mcFile->IsZombie()) {
        std::cerr << "[MC GEN] Error: unable to open MC file " << mcFilePath << std::endl;
        return results;
    }

    TTree* genTree = dynamic_cast<TTree*>(mcFile->Get("skimGENTree"));
    if (!genTree) {
        genTree = dynamic_cast<TTree*>(mcFile->Get("skimGENTreeFlat"));
    }
    if (!genTree) {
        std::cerr << "[MC GEN] Error: could not locate skimGENTree or skimGENTreeFlat in "
                  << mcFilePath << std::endl;
        return results;
    }

    DataFormat::simpleDStarMCTreeflat genEvt;
    genEvt.setGENTree(genTree);

    const size_t nPtBins = ptBins.size();
    if (nPtBins == 0) {
        std::cerr << "[MC GEN] Warning: no pT bins configured; skipping GEN extraction." << std::endl;
        return results;
    }

    std::vector<std::unique_ptr<TH1D>> promptHists(nPtBins);
    std::vector<std::unique_ptr<TH1D>> nonpromptHists(nPtBins);
    const int nCosHistBins = 20;
    const double cosHistMin = useAbsTheta ? 0.0 : -1.0;
    const double cosHistMax = 1.0;

    for (size_t ipt = 0; ipt < nPtBins; ++ipt) {
        const double ptMin = ptBins[ipt].first;
        const double ptMax = ptBins[ipt].second;
        const std::string ptLabel = Form("%.0fto%.0f", ptMin, ptMax);

        promptHists[ipt] = std::make_unique<TH1D>(
            Form("mc_gen_prompt_costheta_pt_%s", ptLabel.c_str()),
            Form("Prompt GEN %s (%.0f < p_{T} < %.0f GeV/c);%s;Entries",
                 displayLabel.c_str(), ptMin, ptMax, axisLabel.c_str()),
            nCosHistBins, cosHistMin, cosHistMax);
        nonpromptHists[ipt] = std::make_unique<TH1D>(
            Form("mc_gen_nonprompt_costheta_pt_%s", ptLabel.c_str()),
            Form("Nonprompt GEN %s (%.0f < p_{T} < %.0f GeV/c);%s;Entries",
                 displayLabel.c_str(), ptMin, ptMax, axisLabel.c_str()),
            nCosHistBins, cosHistMin, cosHistMax);

        promptHists[ipt]->Sumw2();
        promptHists[ipt]->SetDirectory(nullptr);
        nonpromptHists[ipt]->Sumw2();
        nonpromptHists[ipt]->SetDirectory(nullptr);
    }

    const Long64_t nEntries = genTree->GetEntries();
    std::cout << "[MC GEN] Processing " << nEntries << " GEN entries from " << mcFilePath << std::endl;

    for (Long64_t iEntry = 0; iEntry < nEntries; ++iEntry) {
        genTree->GetEntry(iEntry);

        if (!std::isfinite(genEvt.gen_pT) || !std::isfinite(genEvt.gen_y)) {
            continue;
        }

        if (std::fabs(genEvt.gen_y) >= maxAbsRapidity) {
            continue;
        }

        TLorentzVector dstar, d0;
        dstar.SetPtEtaPhiM(genEvt.gen_pT, genEvt.gen_eta, genEvt.gen_phi, genEvt.gen_mass);
        d0.SetPtEtaPhiM(genEvt.gen_D0pT, genEvt.gen_D0eta, genEvt.gen_D0phi, genEvt.gen_D0mass);

        TVector3 helicityVec = DstarDau1Vector_Helicity(dstar, d0);
        double cosTheta = helicityVec.CosTheta();

        ProductionType channel = (genEvt.gen_D0ancestorFlavor_ == 5)
                                 ? ProductionType::NonPrompt
                                 : ProductionType::Prompt;

        for (size_t ipt = 0; ipt < nPtBins; ++ipt) {
            const double ptMin = ptBins[ipt].first;
            const double ptMax = ptBins[ipt].second;
            const bool isLastBin = (ipt + 1 == nPtBins);
            const bool inBin = genEvt.gen_pT >= ptMin && (genEvt.gen_pT < ptMax || (isLastBin && genEvt.gen_pT <= ptMax));
            if (!inBin) {
                continue;
            }

            TH1D* targetHist = (channel == ProductionType::Prompt)
                               ? promptHists[ipt].get()
                               : nonpromptHists[ipt].get();
            if (targetHist) {
                targetHist->Fill(projectCosTheta(cosTheta, useAbsTheta));
            }
            break;
        }
    }


    auto fillResults = [&](ProductionType channel,
                           const std::vector<std::unique_ptr<TH1D>>& hists) {
        auto& result = results[channel];
        result.rho00.assign(nPtBins, std::numeric_limits<double>::quiet_NaN());
        result.rho00Err.assign(nPtBins, std::numeric_limits<double>::quiet_NaN());

        std::string channelDir;
        if (!baseOutputDir.empty()) {
            channelDir = appendPathLocal(baseOutputDir, ToString(channel));
            gSystem->mkdir(channelDir.c_str(), true);
        }

        for (size_t ipt = 0; ipt < nPtBins; ++ipt) {
            TH1D* hist = hists[ipt].get();
            if (!hist || hist->GetEntries() < 10) {
                continue;
            }

            const double fitRangeMin = useAbsTheta ? 0.0 : -0.99;
            const double fitRangeMax = 0.99;
            TF1 fitFunc(Form("mc_gen_fit_%s_%zu", ToString(channel).c_str(), ipt),
                        "[0]*(1 - [1] + (3*[1] - 1)*x*x)", fitRangeMin, fitRangeMax);
            fitFunc.SetParName(0, "N_{0}");
            fitFunc.SetParName(1, "#rho_{00}");
            fitFunc.SetParameter(0, std::max(1.0, hist->Integral()));
            fitFunc.SetParameter(1, 1.0 / 3.0);
            fitFunc.SetParLimits(1, 0.0, 1.0);

            TFitResultPtr fitRes = hist->Fit(&fitFunc, "RQ0S");
            int fitStatus = fitRes ? fitRes->Status() : -1;
            if (fitStatus != 0) {
                std::cout << "[MC GEN] Warning: fit failed for " << PrettyLabel(channel)
                          << " pT bin " << ptBins[ipt].first << "-" << ptBins[ipt].second
                          << " (status " << fitStatus << ")" << std::endl;
                // continue;
            }

            const double rho = fitFunc.GetParameter(1);
            const double rhoErr = fitFunc.GetParError(1);
            if (!std::isfinite(rho) || !std::isfinite(rhoErr)) {
                continue;
            }

            result.rho00[ipt] = rho;
            result.rho00Err[ipt] = rhoErr;
            result.success = true;
            std::cout << "[MC GEN] " << PrettyLabel(channel) << " pT "
                      << ptBins[ipt].first << "-" << ptBins[ipt].second
                      << " GeV/c: rho00 = " << rho << " ± " << rhoErr << std::endl;

            if (channelDir.empty()) {
                continue;
            }

            const double ptMin = ptBins[ipt].first;
            const double ptMax = ptBins[ipt].second;
            Color_t histColor = (channel == ProductionType::Prompt)
                                ? kRed
                                : static_cast<Color_t>(kBlue + 1);
            Style_t markerStyle = (channel == ProductionType::Prompt) ? 20 : 21;

            TCanvas c_mc(Form("c_mc_gen_%s_pt_%zu", ToString(channel).c_str(), ipt),
                         Form("%s GEN %s (%.0f < p_{T} < %.0f GeV/c)",
                              PrettyLabel(channel).c_str(), displayLabel.c_str(), ptMin, ptMax),
                         700, 600);
            c_mc.SetLeftMargin(0.14);
            c_mc.SetBottomMargin(0.14);
            c_mc.SetTopMargin(0.05);
            c_mc.SetRightMargin(0.05);

            hist->SetMarkerStyle(markerStyle);
            hist->SetMarkerSize(0.9);
            hist->SetMarkerColor(histColor);
            hist->SetLineColor(histColor);
            hist->GetXaxis()->SetTitle(axisLabel.c_str());
            hist->GetYaxis()->SetTitle("Entries");
            if (hist->GetMaximum() > 0.0) {
                hist->GetYaxis()->SetRangeUser(0.0, hist->GetMaximum() * 1.3);
            }
            hist->Draw("E1");

            fitFunc.SetLineColor(kBlack);
            fitFunc.SetLineWidth(2);
            fitFunc.Draw("SAME");

            TLatex latex;
            latex.SetNDC();
            latex.SetTextSize(0.045);
            latex.DrawLatex(0.18, 0.88, PrettyLabel(channel).c_str());
            latex.DrawLatex(0.18, 0.80, Form("%.0f < p_{T} < %.0f GeV/c", ptMin, ptMax));
            latex.DrawLatex(0.18, 0.72, Form("#rho_{00} = %.3f #pm %.3f", rho, rhoErr));
            latex.SetTextSize(0.035);
            latex.DrawLatex(0.18, 0.64, "N_{0}(1-#rho_{00}+(3#rho_{00}-1)cos^{2}#theta)");

            c_mc.SetGrid(1, 1);

            std::string fileStem = Form("mc_gen_costheta_%s_pt%.0fto%.0f",
                                         ToString(channel).c_str(), ptMin, ptMax);
            std::string pngOut = appendPathLocal(channelDir, fileStem + ".png");
            std::string pdfOut = appendPathLocal(channelDir, fileStem + ".pdf");
            c_mc.SaveAs(pngOut.c_str());
            c_mc.SaveAs(pdfOut.c_str());
        }
    };
    fillResults(ProductionType::Prompt, promptHists);
    fillResults(ProductionType::NonPrompt, nonpromptHists);

    return results;
}

namespace {
std::string ResolveAnalysisBaseDir() {
    const std::string fitDir = FitRootDirectory();
    if (!fitDir.empty() && gSystem->AccessPathName(fitDir.c_str(), kReadPermission) == 0) {
        return fitDir;
    }

    const std::string fallback = JoinPath("/home/jun502s/DstarAna/DStarAnalysis", "Fit");
    if (gSystem->AccessPathName(fallback.c_str(), kReadPermission) == 0) {
        return fallback;
    }

    return std::string(gSystem->WorkingDirectory());
}

std::string AppendPath(const std::string& base, const std::string& piece) {
    if (piece.empty()) {
        return base;
    }

    std::size_t firstNonSep = piece.find_first_not_of("/\\");
    const std::string cleanedPiece = (firstNonSep == std::string::npos)
                                     ? std::string()
                                     : piece.substr(firstNonSep);

    if (cleanedPiece.empty()) {
        return base;
    }

    if (base.empty()) {
        return cleanedPiece;
    }

    std::string result = base;
    if (result.back() != '/' && result.back() != '\\') {
        result.push_back('/');
    }
    result += cleanedPiece;
    return result;
}
}

FitResultSummary LoadFitResultSummary(const FitOpt& fitOpt,
                                      const std::string& analysisBaseDir,
                                      const std::string& category = "Data") {
    FitResultSummary summary;

    std::string filePath = analysisBaseDir.empty() ? "." : analysisBaseDir;
    filePath = AppendPath(filePath, fitOpt.outputDir);
    filePath = AppendPath(filePath, fitOpt.subDir);
    filePath = AppendPath(filePath, category);
    filePath = AppendPath(filePath, fitOpt.outputFile + ".root");
    summary.filePath = filePath;

    std::unique_ptr<TFile> fitFile(TFile::Open(summary.filePath.c_str(), "READ"));
    if (!fitFile || fitFile->IsZombie()) {
        std::cerr << "Error: Could not open fit result file " << summary.filePath << std::endl;
        return summary;
    }

    auto loadParameter = [&](const char* name) -> double {
        if (auto* param = dynamic_cast<TParameter<double>*>(fitFile->Get(name))) {
            return param->GetVal();
        }
        return 0.0;
    };

    summary.signalYield = loadParameter("nsig");
    summary.signalError = loadParameter("nsig_err");
    summary.backgroundYield = loadParameter("nbkg");

    RooWorkspace* ws = dynamic_cast<RooWorkspace*>(fitFile->Get("workspace"));
    if (!ws && !fitOpt.wsName.empty()) {
        ws = dynamic_cast<RooWorkspace*>(fitFile->Get(fitOpt.wsName.c_str()));
    }

    if (ws) {
        if (summary.signalYield == 0.0) {
            if (auto* nSig = ws->var("nsig")) {
                summary.signalYield = nSig->getVal();
                summary.signalError = nSig->getError();
            }
        }
        if (summary.backgroundYield == 0.0) {
            if (auto* nBkg = ws->var("nbkg")) {
                summary.backgroundYield = nBkg->getVal();
            }
        }

        if (auto* cloned = dynamic_cast<RooWorkspace*>(ws->Clone())) {
            summary.workspace.reset(cloned);
        }
    }

    summary.success = (summary.signalYield != 0.0) || static_cast<bool>(summary.workspace);
    if (summary.success) {
        std::cout << "Loaded fit summary from " << summary.filePath
                  << " → raw yield: " << summary.signalYield
                  << " ± " << summary.signalError << std::endl;
    }

    return summary;
}

std::unique_ptr<TH1D> LoadDCADataHistogram(const std::string& filePath,
                                           const std::string& histName) {
    std::unique_ptr<TFile> fin(TFile::Open(filePath.c_str(), "READ"));
    if (!fin || fin->IsZombie()) {
        std::cerr << "[HistBasedCorrection] Failed to open DCA histogram file: "
                  << filePath << std::endl;
        return nullptr;
    }

    TH1D* hist = dynamic_cast<TH1D*>(fin->Get(histName.c_str()));
    if (!hist) {
        std::cerr << "[HistBasedCorrection] Histogram '" << histName
                  << "' not found in file " << filePath << std::endl;
        return nullptr;
    }

    auto clone = std::unique_ptr<TH1D>(static_cast<TH1D*>(hist->Clone("dcaYieldHist")));
    clone->SetDirectory(nullptr);
    clone->Sumw2();
    return clone;
}

// Function to apply histogram-based correction using yield values
std::vector<double> ApplyHistogramCorrection(double rawYield, double rawYieldError, TH2D* effMap, double ptMin, double ptMax, double cosMin, double cosMax) {
    if (!effMap) {
        std::cerr << "Error: Null efficiency map pointer" << std::endl;
        return {0.0, 0.0, rawYield, rawYieldError};
    }
    
    // Get the center of the pt and cos bins
    double ptCenter = (ptMin + ptMax) / 2.0;
    double cosCenter = (cosMin + cosMax) / 2.0;
    
    // Get efficiency from the map
    int bin = effMap->FindBin(ptCenter, cosCenter);
    double efficiency = effMap->GetBinContent(bin);
    cout << "efficiency: " << efficiency << endl;
    
    if (efficiency <= 1e-9) {
        std::cerr << "Warning: Efficiency is near zero for pt=" << ptCenter << ", cos=" << cosCenter << std::endl;
        efficiency = 0.001; // Set minimum efficiency
    }
    
    // Calculate correction factor
    double correctionFactor = 1.0 / efficiency;
    
    // Apply correction
    double correctedYield = rawYield * correctionFactor;
    double correctedYieldError = rawYieldError * correctionFactor;
    
    std::cout << "PT: " << ptMin << "-" << ptMax << ", cos: " << cosMin << "-" << cosMax 
              << ", efficiency: " << efficiency << ", correction factor: " << correctionFactor << std::endl;
    
    return {correctedYield, correctedYieldError, rawYield, rawYieldError};
}

TH2D* BuildEfficiencyMap(ProductionType type,
                         const std::string& mcFilePath,
                         const std::vector<std::pair<double, double>>& ptBins,
                         const std::vector<double>& cosBins,
                         bool useAbsTheta,
                         double directionY,
                         std::vector<TH1D*>* genProjections,
                         std::vector<TH1D*>* matchProjections) {
    if (ptBins.empty() || cosBins.size() < 2) {
        std::cerr << "Error: Invalid binning configuration for efficiency map." << std::endl;
        return nullptr;
    }

    (void)directionY;

    std::vector<double> ptEdges;
    ptEdges.reserve(ptBins.size() + 1);
    ptEdges.push_back(ptBins.front().first);
    for (const auto& bin : ptBins) {
        if (!ptEdges.empty() && std::abs(ptEdges.back() - bin.first) > 1e-6 && ptEdges.back() != bin.first) {
            ptEdges.push_back(bin.first);
        }
        ptEdges.push_back(bin.second);
    }
    ptEdges.erase(std::unique(ptEdges.begin(), ptEdges.end(), [](double a, double b) {
        return std::abs(a - b) < 1e-9;
    }), ptEdges.end());

    std::vector<double> cosEdges = cosBins;

    if (ptEdges.size() < 2 || cosEdges.size() < 2) {
        std::cerr << "Error: Failed to build bin edges for efficiency map." << std::endl;
        return nullptr;
    }

    TFile* mcFile = TFile::Open(mcFilePath.c_str(), "READ");
    if (!mcFile || mcFile->IsZombie()) {
        std::cerr << "Error: could not open MC file " << mcFilePath << std::endl;
        if (mcFile) { mcFile->Close(); delete mcFile; }
        return nullptr;
    }

    auto* genTree = dynamic_cast<TTree*>(mcFile->Get("skimGENTreeFlat"));
    auto* recoTree = dynamic_cast<TTree*>(mcFile->Get("skimTreeFlat"));
    if (!genTree || !recoTree) {
        std::cerr << "Error: missing required trees (skimGENTreeFlat/skimTreeFlat) in MC file." << std::endl;
        mcFile->Close();
        delete mcFile;
        return nullptr;
    }

    const std::string channelTag = ToString(type);
    const std::string channelLabel = PrettyLabel(type);
    const std::string axisLabel = cosAxisLabel(useAbsTheta);
    const std::string displayLabel = cosDisplayLabel(useAbsTheta);

    TH2D* channelReco = new TH2D(Form("pt_cos_%s_reco", channelTag.c_str()),
                                 Form("%s reconstruction numerator; p_{T} (GeV/c); %s",
                                      channelLabel.c_str(), axisLabel.c_str()),
                                 static_cast<int>(ptEdges.size() - 1), ptEdges.data(),
                                 static_cast<int>(cosEdges.size() - 1), cosEdges.data());
    TH2D* channelGen = new TH2D(Form("pt_cos_%s_gen", channelTag.c_str()),
                                Form("%s generation denominator; p_{T} (GeV/c); %s",
                                     channelLabel.c_str(), axisLabel.c_str()),
                                static_cast<int>(ptEdges.size() - 1), ptEdges.data(),
                                static_cast<int>(cosEdges.size() - 1), cosEdges.data());
    TH2D* channelMatchReco = nullptr;
    if (matchProjections) {
        channelMatchReco = new TH2D(Form("pt_cos_%s_match_reco", channelTag.c_str()),
                                    Form("%s matched reco counts; p_{T} (GeV/c); %s",
                                         channelLabel.c_str(), axisLabel.c_str()),
                                    static_cast<int>(ptEdges.size() - 1), ptEdges.data(),
                                    static_cast<int>(cosEdges.size() - 1), cosEdges.data());
        channelMatchReco->Sumw2();
    }
    channelReco->Sumw2();
    channelGen->Sumw2();

    DataFormat::simpleDStarMCTreeflat genEvt;
    DataFormat::simpleDStarMCTreeflat recoEvt;
    genEvt.setGENTree(genTree);
    recoEvt.setTree(recoTree);

    TLorentzVector Dstar, D0;
    Long64_t genCount = 0;
    Long64_t recoCount = 0;

    const Long64_t nGen = genTree->GetEntries();
    for (Long64_t i = 0; i < nGen; ++i) {
        genTree->GetEntry(i);

        if (std::abs(genEvt.gen_y) >= SelectionCuts::Y_PARENT_MAX) {
            continue;
        }
        if (!PassProductionSelection(type, genEvt.gen_D0ancestorFlavor_)) {
            continue;
        }

        Dstar.SetPtEtaPhiM(genEvt.gen_pT, genEvt.gen_eta, genEvt.gen_phi, genEvt.gen_mass);
        D0.SetPtEtaPhiM(genEvt.gen_D0pT, genEvt.gen_D0eta, genEvt.gen_D0phi, genEvt.gen_D0mass);
        const auto helicityVec = DstarDau1Vector_Helicity(Dstar, D0);
        const double cosVal = projectCosTheta(helicityVec.CosTheta(), useAbsTheta);

        channelGen->Fill(genEvt.gen_pT, cosVal);
        ++genCount;
    }

    const Long64_t nReco = recoTree->GetEntries();
    for (Long64_t i = 0; i < nReco; ++i) {
        recoTree->GetEntry(i);

        if (!recoEvt.matchGEN || recoEvt.isSwap) {
            continue;
        }
        if (!PassProductionSelection(type, recoEvt.matchGen_D1ancestorFlavor_)) {
            continue;
        }
        if (std::abs(recoEvt.y) >= SelectionCuts::Y_PARENT_MAX) {
            continue;
        }
        if (recoEvt.pT <= SelectionCuts::PT_PARENT_MIN) {
            continue;
        }
        if (std::abs(recoEvt.EtaD2) >= SelectionCuts::ETA_D2_MAX ||
            std::abs(recoEvt.EtaGrandD1) >= SelectionCuts::ETA_GRANDD1_MAX ||
            std::abs(recoEvt.EtaGrandD2) >= SelectionCuts::ETA_GRANDD2_MAX) {
            continue;
        }
        if (recoEvt.pTD2 <= SelectionCuts::PT_D2_MIN ||
            recoEvt.pTGrandD1 <= SelectionCuts::PT_GRANDD1_MIN ||
            recoEvt.pTGrandD2 <= SelectionCuts::PT_GRANDD2_MIN) {
            continue;
        }
        if( recoEvt.massDaugther1 <= 1.82 || recoEvt.massDaugther1 >= 1.92 ) {
            continue;
        }

        Dstar.SetPtEtaPhiM(recoEvt.pT, recoEvt.eta, recoEvt.phi, recoEvt.mass);
        D0.SetPtEtaPhiM(recoEvt.pTD1, recoEvt.EtaD1, recoEvt.PhiD1, recoEvt.massDaugther1);
        const auto helicityVec = DstarDau1Vector_Helicity(Dstar, D0);
        const double cosVal = projectCosTheta(helicityVec.CosTheta(), useAbsTheta);

        channelReco->Fill(recoEvt.pT, cosVal);
        if (channelMatchReco) {
            channelMatchReco->Fill(recoEvt.pT, cosVal);
        }
        ++recoCount;
    }

    TH2D* efficiency = static_cast<TH2D*>(channelReco->Clone(Form("pt_cos_%s_efficiency", channelTag.c_str())));
    if (efficiency) {
        efficiency->SetTitle(Form("%s efficiency map; p_{T} (GeV/c); %s",
                                   channelLabel.c_str(), axisLabel.c_str()));
        efficiency->Divide(channelReco, channelGen, 1.0, 1.0, "cl=0.683 b(1,1) mode");
        efficiency->SetDirectory(nullptr);
    }

    std::cout << "Built " << channelLabel << " efficiency map from " << mcFilePath
              << " (gen entries=" << genCount
              << ", reco entries=" << recoCount << ")" << std::endl;

    if (genProjections) {
        genProjections->clear();
        genProjections->reserve(ptBins.size());

        auto* xAxis = channelGen->GetXaxis();
        auto* yAxis = channelGen->GetYaxis();

        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            const auto& ptRange = ptBins[ipt];
            std::string histName = Form("%s_gen_count_pt_%zu", channelTag.c_str(), ipt);
            std::string histTitle = Form("Generated %s Count vs %s; %s; Generated Count (%.0f-%.0f GeV/c)",
                                         channelLabel.c_str(), displayLabel.c_str(), axisLabel.c_str(),
                                         ptRange.first, ptRange.second);
            TH1D* genHist = new TH1D(histName.c_str(), histTitle.c_str(), static_cast<int>(cosBins.size() - 1), cosBins.data());
            genHist->Sumw2();
            genHist->SetDirectory(nullptr);

            int xBinMin = xAxis->FindBin(ptRange.first + 1e-6);
            int xBinMax = xAxis->FindBin(ptRange.second - 1e-6);
            xBinMin = std::max(1, std::min(xBinMin, xAxis->GetNbins()));
            xBinMax = std::max(1, std::min(xBinMax, xAxis->GetNbins()));
            if (xBinMax < xBinMin) std::swap(xBinMax, xBinMin);

            for (size_t icos = 0; icos < cosBins.size() - 1; ++icos) {
                double cosMin = cosBins[icos];
                double cosMax = cosBins[icos + 1];
                int yBinMin = yAxis->FindBin(cosMin + 1e-6);
                int yBinMax = yAxis->FindBin(cosMax - 1e-6);
                yBinMin = std::max(1, std::min(yBinMin, yAxis->GetNbins()));
                yBinMax = std::max(1, std::min(yBinMax, yAxis->GetNbins()));
                if (yBinMax < yBinMin) std::swap(yBinMax, yBinMin);

                double err = 0.0;
                double count = channelGen->IntegralAndError(xBinMin, xBinMax, yBinMin, yBinMax, err);
                genHist->SetBinContent(static_cast<int>(icos) + 1, count);
                genHist->SetBinError(static_cast<int>(icos) + 1, err);
            }

            genProjections->push_back(genHist);
        }
    }

    if (matchProjections && channelMatchReco) {
        matchProjections->clear();
        matchProjections->reserve(ptBins.size());

        auto* xAxis = channelMatchReco->GetXaxis();
        auto* yAxis = channelMatchReco->GetYaxis();

        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            const auto& ptRange = ptBins[ipt];
            std::string histName = Form("%s_match_count_pt_%zu", channelTag.c_str(), ipt);
            std::string histTitle = Form("Matched %s Count vs %s; %s; Matched Count (%.0f-%.0f GeV/c)",
                                         channelLabel.c_str(), displayLabel.c_str(), axisLabel.c_str(),
                                         ptRange.first, ptRange.second);
            TH1D* matchHist = new TH1D(histName.c_str(), histTitle.c_str(), static_cast<int>(cosBins.size() - 1), cosBins.data());
            matchHist->Sumw2();
            matchHist->SetDirectory(nullptr);

            int xBinMin = xAxis->FindBin(ptRange.first + 1e-6);
            int xBinMax = xAxis->FindBin(ptRange.second - 1e-6);
            xBinMin = std::max(1, std::min(xBinMin, xAxis->GetNbins()));
            xBinMax = std::max(1, std::min(xBinMax, xAxis->GetNbins()));
            if (xBinMax < xBinMin) std::swap(xBinMax, xBinMin);

            for (size_t icos = 0; icos < cosBins.size() - 1; ++icos) {
                double cosMin = cosBins[icos];
                double cosMax = cosBins[icos + 1];
                int yBinMin = yAxis->FindBin(cosMin + 1e-6);
                int yBinMax = yAxis->FindBin(cosMax - 1e-6);
                yBinMin = std::max(1, std::min(yBinMin, yAxis->GetNbins()));
                yBinMax = std::max(1, std::min(yBinMax, yAxis->GetNbins()));
                if (yBinMax < yBinMin) std::swap(yBinMax, yBinMin);

                double err = 0.0;
                double count = channelMatchReco->IntegralAndError(xBinMin, xBinMax, yBinMin, yBinMax, err);
                matchHist->SetBinContent(static_cast<int>(icos) + 1, count);
                matchHist->SetBinError(static_cast<int>(icos) + 1, err);
            }

            matchProjections->push_back(matchHist);
        }
    }

    delete channelReco;
    delete channelGen;
    if (channelMatchReco) delete channelMatchReco;

    if (mcFile) {
        mcFile->Close();
        delete mcFile;
    }

    return efficiency;
}

bool LoadPromptFraction(const FitOpt& fitOpt,
                        const KinematicBin& bin,
                        const std::string& analysisBaseDir,
                        double& promptFraction,
                        double& promptFractionError) {
    promptFraction = 1.0;
    promptFractionError = 0.0;

    std::string basePath = analysisBaseDir.empty() ? "." : analysisBaseDir;
    basePath = AppendPath(basePath, fitOpt.outputDir);
    basePath = AppendPath(basePath, fitOpt.subDir);
    basePath = AppendPath(basePath, "Data/dcaroot");
    const std::string dcaFileName = AppendPath(basePath, "DStar_DCA_" + bin.GetBinName() + ".root");

    std::unique_ptr<TFile> dcaFile(TFile::Open(dcaFileName.c_str(), "READ"));
    if (!dcaFile || dcaFile->IsZombie()) {
        std::cerr << "Warning: could not open DCA file for prompt fraction: " << dcaFileName << std::endl;
        return false;
    }

    auto* promptFracParam = dynamic_cast<TParameter<double>*>(dcaFile->Get("PromptFraction"));
    auto* promptFracErrParam = dynamic_cast<TParameter<double>*>(dcaFile->Get("PromptFractionError"));
    if (!promptFracParam) {
        std::cerr << "Warning: PromptFraction parameter missing in " << dcaFileName << std::endl;
        return false;
    }

    promptFraction = promptFracParam->GetVal();
    promptFractionError = promptFracErrParam ? promptFracErrParam->GetVal() : 0.0;
    return true;
}

PromptFractionScanResult RunPromptFractionScan(FitOpt fitOpt,
                                               const KinematicBin& bin,
                                               const std::string& analysisBaseDir,
                                               double scaleMin,
                                               double scaleMax,
                                               double scaleStep,
                                               const std::vector<double>& customScales = {},
                                               bool makePerScalePlots = false) {
    PromptFractionScanResult result;

    if (analysisBaseDir.empty()) {
        std::cerr << "[HistBasedCorrection] Invalid analysis base directory; skipping prompt scan." << std::endl;
        return result;
    }

    fitOpt.GenerateLegends();

    std::string basePath = AppendPath(analysisBaseDir, fitOpt.outputDir);
    basePath = AppendPath(basePath, fitOpt.subDir);
    std::string dataDir = AppendPath(basePath, "Data");
    std::string mcDir = AppendPath(basePath, "MC");

    std::string outputBase = AppendPath(dataDir, "dca_resolution_scan");
    outputBase = AppendPath(outputBase, bin.GetBinName());
    gSystem->mkdir(outputBase.c_str(), true);
    result.outputDirectory = outputBase;

    FitOpt dcaOpt = fitOpt;
    if (dcaOpt.dcaBins.size() < 2) {
        std::cerr << "[HistBasedCorrection] DCA binning not configured for prompt scan." << std::endl;
        return result;
    }

    DCAFitter dcaFitter(dcaOpt,
                        "HistCorrDCAFitter",
                        dcaOpt.massVar,
                        dcaOpt.dcaMin,
                        dcaOpt.dcaMax,
                        static_cast<int>(dcaOpt.dcaBins.size()) - 1);

    std::string mcResultFile = AppendPath(mcDir, fitOpt.outputFile + ".root");
    std::string dataResultFile = AppendPath(dataDir, fitOpt.outputFile + ".root");
    dcaFitter.setMCResultFile(mcResultFile, "workspace", "");
    dcaFitter.setDataResultFile(dataResultFile, "workspace", "");

    dcaFitter.setDCABranchName("dca3D");
    dcaFitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_");
    dcaFitter.setPromptPdgIds({4, 2});
    dcaFitter.setNonPromptPdgIds({5});

    auto replaceCentrality = [](std::string expr) {
        const std::string key = "Centrality";
        size_t pos = 0;
        while ((pos = expr.find(key, pos)) != std::string::npos) {
            expr.replace(pos, key.size(), "1");
            pos += 1;
        }
        return expr;
    };

    dcaFitter.setMCCuts(replaceCentrality(dcaOpt.cutMCExpr));
    dcaFitter.setDataCuts(replaceCentrality(dcaOpt.cutExpr));

    if (!dcaFitter.createTemplatesFromMC()) {
        std::cerr << "[HistBasedCorrection] Failed to create MC templates for prompt scan." << std::endl;
        return result;
    }

    std::string dcaHistDir = AppendPath(dataDir, "dcahist");
    std::string dcaHistPath = AppendPath(dcaHistDir, "DStar_DCA_Yield_" + bin.GetBinName() + ".root");
    auto dataHist = LoadDCADataHistogram(dcaHistPath, "dataYieldHist");
    if (!dataHist || dataHist->Integral() <= 0) {
        std::cerr << "[HistBasedCorrection] Data DCA histogram invalid for prompt scan: "
                  << dcaHistPath << std::endl;
        return result;
    }

    const std::vector<double>& dcaBins = dcaOpt.dcaBins;
    RooRealVar dca("dca", "D^{0} DCA (cm)", dcaBins.front(), dcaBins.back());
    dca.setBins(static_cast<int>(dcaBins.size()) - 1);
    RooDataHist dataDH("dataDH", "dataDH", RooArgList(dca), dataHist.get());
    double dataIntegral = dataHist->Integral();

    std::vector<double> scanScales;
    if (!customScales.empty()) {
        scanScales = customScales;
    } else {
        if (scaleStep <= 0.0) {
            scaleStep = 0.01;
        }
        for (double scale = scaleMin; scale <= scaleMax + 1e-6; scale += scaleStep) {
            scanScales.push_back(scale);
        }
    }
    if (scanScales.empty()) {
        scanScales.push_back(1.0);
    }

    std::sort(scanScales.begin(), scanScales.end());
    scanScales.erase(std::unique(scanScales.begin(), scanScales.end(), [](double a, double b) {
        return std::abs(a - b) < 1e-6;
    }), scanScales.end());

    const std::string ptLabel = fitOpt.pTLegend.empty() ?
        Form("%0.1f < p_{T} < %0.1f GeV/c", bin.pTMin, bin.pTMax) : fitOpt.pTLegend;
    const std::string cosLabel = fitOpt.cosLegend.empty() ?
        Form("%0.2f < cos#theta_{HX} < %0.2f", bin.cosMin, bin.cosMax) : fitOpt.cosLegend;

    for (double scale : scanScales) {
        double recordedChi2 = std::numeric_limits<double>::quiet_NaN();
        int recordedNdf = 0;
        double recordedPromptFraction = std::numeric_limits<double>::quiet_NaN();
        double recordedPromptError = 0.0;

        std::unique_ptr<TH1D> hPrompt;
        std::unique_ptr<TH1D> hNonPrompt;
        if (!dcaFitter.GenerateTemplatesWithScale(scale, hPrompt, hNonPrompt)) {
            std::cerr << "[HistBasedCorrection] Failed to build DCA templates for scale "
                      << scale << std::endl;
        } else if (!hPrompt || !hNonPrompt || hPrompt->Integral() <= 0.0 || hNonPrompt->Integral() <= 0.0) {
            std::cerr << "[HistBasedCorrection] Empty DCA template at scale " << scale << std::endl;
        } else {
            RooDataHist promptDH("promptDH", "promptDH", RooArgList(dca), hPrompt.get());
            RooDataHist nonPromptDH("nonPromptDH", "nonPromptDH", RooArgList(dca), hNonPrompt.get());
            RooHistPdf promptPdf("promptPdf", "promptPdf", RooArgSet(dca), promptDH);
            RooHistPdf nonPromptPdf("nonPromptPdf", "nonPromptPdf", RooArgSet(dca), nonPromptDH);

            double initPrompt = std::max(1.0, dataIntegral * 0.8);
            double initNon = std::max(1.0, dataIntegral * 0.2);

            RooRealVar nPrompt("nPrompt", "Prompt yield", initPrompt, 0.0, dataIntegral * 5.0);
            RooRealVar nNonPrompt("nNonPrompt", "Non-prompt yield", initNon, 0.0, dataIntegral * 5.0);

            RooAddPdf model("model", "Prompt + NonPrompt", RooArgList(promptPdf, nonPromptPdf),
                            RooArgList(nPrompt, nNonPrompt));

            auto fitRes = std::unique_ptr<RooFitResult>(model.fitTo(dataDH,
                                                                    Extended(true),
                                                                    SumW2Error(true),
                                                                    PrintLevel(-1),
                                                                    Save(true)));
            if (!fitRes) {
                std::cerr << "[HistBasedCorrection] RooFit failed for scale " << scale << std::endl;
            } else {
                RooChi2Var chi2Var("chi2Var", "chi2Var", model, dataDH);
                recordedChi2 = chi2Var.getVal();
                recordedNdf = static_cast<int>(dataHist->GetNbinsX()) - fitRes->floatParsFinal().getSize();

                double nPromptVal = nPrompt.getVal();
                double nNonPromptVal = nNonPrompt.getVal();
                double totalYield = nPromptVal + nNonPromptVal;
                if (totalYield > 0.0) {
                    recordedPromptFraction = nPromptVal / totalYield;
                    double denom = totalYield;
                    double dfdPrompt = (totalYield - nPromptVal) / (denom * denom);
                    double dfdNonPrompt = -nPromptVal / (denom * denom);
                    double errPrompt = nPrompt.getError();
                    double errNonPrompt = nNonPrompt.getError();
                    double variance = (dfdPrompt * errPrompt) * (dfdPrompt * errPrompt)
                                      + (dfdNonPrompt * errNonPrompt) * (dfdNonPrompt * errNonPrompt);
                    recordedPromptError = std::sqrt(std::max(0.0, variance));
                }

                if (makePerScalePlots) {
                    auto canvas = std::make_unique<TCanvas>(Form("%s/scale_%0.2f_c", outputBase.c_str(), scale), "", 800, 600);
                    canvas->cd();
                    auto hPromptFit = std::unique_ptr<TH1D>(static_cast<TH1D*>(hPrompt->Clone("hPromptFit")));
                    auto hNonPromptFit = std::unique_ptr<TH1D>(static_cast<TH1D*>(hNonPrompt->Clone("hNonPromptFit")));
                    if (hPromptFit->Integral() > 0.0) {
                        hPromptFit->Scale(nPromptVal / hPromptFit->Integral());
                    }
                    if (hNonPromptFit->Integral() > 0.0) {
                        hNonPromptFit->Scale(nNonPromptVal / hNonPromptFit->Integral());
                    }
                    auto hTotal = std::unique_ptr<TH1D>(static_cast<TH1D*>(hPromptFit->Clone("hTotalFit")));
                    hTotal->Add(hNonPromptFit.get());

                    dataHist->SetMarkerStyle(20);
                    dataHist->Draw("E1");
                    hTotal->SetLineColor(kRed + 1);
                    hTotal->Draw("HIST SAME");
                    hPromptFit->SetLineColor(kBlue + 1);
                    hPromptFit->Draw("HIST SAME");
                    hNonPromptFit->SetLineColor(kGreen + 2);
                    hNonPromptFit->Draw("HIST SAME");

                    TLatex latex;
                    latex.SetNDC();
                    latex.SetTextFont(42);
                    latex.SetTextSize(0.04);
                    latex.DrawLatex(0.18, 0.88, ptLabel.c_str());
                    latex.DrawLatex(0.18, 0.82, cosLabel.c_str());
                    latex.DrawLatex(0.18, 0.76, Form("scale = %.2f", scale));
                    latex.DrawLatex(0.18, 0.70, Form("#chi^{2}/NDF = %.2f/%d", recordedChi2, recordedNdf));

                    std::string perScalePath = Form("%s/scale_%0.2f_fit.png", outputBase.c_str(), scale);
                    canvas->SaveAs(perScalePath.c_str());
                    std::string perScalePdf = Form("%s/scale_%0.2f_fit.pdf", outputBase.c_str(), scale);
                    canvas->SaveAs(perScalePdf.c_str());
                }
            }
        }

        result.scales.push_back(scale);
        result.chi2Values.push_back(recordedChi2);
        result.ndfValues.push_back(recordedNdf);
        result.promptFractions.push_back(recordedPromptFraction);
        result.promptFractionErrors.push_back(recordedPromptError);
    }

    double bestChi2Ndf = std::numeric_limits<double>::infinity();
    int bestIndex = -1;
    for (size_t i = 0; i < result.scales.size(); ++i) {
        double chi2 = (i < result.chi2Values.size()) ? result.chi2Values[i] : std::numeric_limits<double>::quiet_NaN();
        double ndf = (i < result.ndfValues.size()) ? result.ndfValues[i] : 0.0;
        if (!std::isfinite(chi2) || ndf <= 0.0) {
            continue;
        }
        double chi2OverNdf = chi2 / ndf;
        if (chi2OverNdf < bestChi2Ndf) {
            bestChi2Ndf = chi2OverNdf;
            bestIndex = static_cast<int>(i);
        }
    }

    if (bestIndex >= 0 && bestIndex < static_cast<int>(result.promptFractions.size())) {
        double bestFraction = result.promptFractions[bestIndex];
        if (std::isfinite(bestFraction)) {
            result.success = true;
            result.bestScale = result.scales[bestIndex];
            result.bestChi2OverNdf = bestChi2Ndf;
            result.bestPromptFraction = bestFraction;
            if (bestIndex < static_cast<int>(result.promptFractionErrors.size())) {
                result.bestPromptFractionError = result.promptFractionErrors[bestIndex];
            }
        }
    }

    if (!result.scales.empty()) {
        auto chi2Graph = std::make_unique<TGraph>(static_cast<int>(result.scales.size()));
        for (int i = 0; i < chi2Graph->GetN(); ++i) {
            double scale = result.scales[i];
            double chi2 = result.chi2Values[i];
            double ndf = result.ndfValues[i] > 0.0 ? result.ndfValues[i] : 1.0;
            chi2Graph->SetPoint(i, scale, chi2 / ndf);
        }
        chi2Graph->SetTitle(";Resolution scale;#chi^{2}/NDF");
        chi2Graph->SetMarkerStyle(21);
        chi2Graph->SetMarkerSize(1.1);
        chi2Graph->SetLineWidth(2);

        auto cChi2 = std::make_unique<TCanvas>((outputBase + "/chi2_vs_scale_c").c_str(), "", 800, 600);
        cChi2->SetGrid();
        chi2Graph->Draw("APL");
        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.04);
        latex.DrawLatex(0.18, 0.88, ptLabel.c_str());
        latex.DrawLatex(0.18, 0.82, cosLabel.c_str());
        cChi2->SaveAs((outputBase + "/chi2_vs_scale.png").c_str());
        cChi2->SaveAs((outputBase + "/chi2_vs_scale.pdf").c_str());

        auto fracGraph = std::make_unique<TGraphErrors>(static_cast<int>(result.scales.size()));
        double minFracPercent = std::numeric_limits<double>::infinity();
        double maxFracPercent = -std::numeric_limits<double>::infinity();
        for (int i = 0; i < fracGraph->GetN(); ++i) {
            double frac = result.promptFractions[i];
            double err = result.promptFractionErrors[i];
            double fracPercent = std::numeric_limits<double>::quiet_NaN();
            double errPercent = std::numeric_limits<double>::quiet_NaN();
            if (std::isfinite(frac)) {
                fracPercent = frac * 100.0;
                errPercent = err * 100.0;
                minFracPercent = std::min(minFracPercent, fracPercent - errPercent);
                maxFracPercent = std::max(maxFracPercent, fracPercent + errPercent);
            }
            fracGraph->SetPoint(i, result.scales[i], std::isfinite(fracPercent) ? fracPercent : 0.0);
            fracGraph->SetPointError(i, 0.0, std::isfinite(errPercent) ? errPercent : 0.0);
        }
        if (!std::isfinite(minFracPercent) || !std::isfinite(maxFracPercent)) {
            minFracPercent = 0.0;
            maxFracPercent = 100.0;
        }
        double fracRange = maxFracPercent - minFracPercent;
        double fracPadding = (fracRange > 1e-3) ? fracRange * 0.15 : 5.0;
        double fracMin = std::max(0.0, minFracPercent - fracPadding);
        double fracMax = maxFracPercent + fracPadding;
        auto minmaxScale = std::minmax_element(result.scales.begin(), result.scales.end());
        double minScale = *minmaxScale.first;
        double maxScale = *minmaxScale.second;
        double scaleRange = maxScale - minScale;
        double scalePadding = (scaleRange > 1e-6) ? scaleRange * 0.1 : 0.05;

        fracGraph->SetTitle(" ");
        fracGraph->SetMarkerStyle(20);
        fracGraph->SetMarkerSize(1.0);
        fracGraph->SetMarkerColor(kBlack);
        fracGraph->SetLineColor(kBlack);
        fracGraph->SetLineWidth(2);

        auto cFrac = std::make_unique<TCanvas>((outputBase + "/prompt_fraction_vs_scale_c").c_str(), "", 800, 700);
        cFrac->SetLeftMargin(0.15);
        cFrac->SetRightMargin(0.05);
        cFrac->SetBottomMargin(0.13);
        cFrac->SetTopMargin(0.10);
        cFrac->SetGridy();

        fracGraph->Draw("AP");
        auto* xAxis = fracGraph->GetXaxis();
        auto* yAxis = fracGraph->GetYaxis();
        xAxis->SetLimits(minScale - scalePadding, maxScale + scalePadding);
        xAxis->SetTitle("Resolution scale");
        xAxis->SetTitleSize(0.05);
        xAxis->SetLabelSize(0.045);
        xAxis->SetTitleOffset(1.1);
        yAxis->SetRangeUser(fracMin, fracMax);
        yAxis->SetTitle("Prompt fraction (%)");
        yAxis->SetTitleSize(0.05);
        yAxis->SetLabelSize(0.045);
        yAxis->SetTitleOffset(1.3);

        std::unique_ptr<TLine> bestScaleLine;
        std::unique_ptr<TMarker> bestScaleMarker;
        if (result.success && std::isfinite(result.bestScale) && std::isfinite(result.bestPromptFraction)) {
            bestScaleLine = std::make_unique<TLine>(result.bestScale, fracMin, result.bestScale, fracMax);
            bestScaleLine->SetLineColor(kRed + 1);
            bestScaleLine->SetLineStyle(kDashed);
            bestScaleLine->SetLineWidth(2);
            bestScaleLine->Draw("same");

            bestScaleMarker = std::make_unique<TMarker>(result.bestScale, result.bestPromptFraction * 100.0, 29);
            bestScaleMarker->SetMarkerColor(kRed + 1);
            bestScaleMarker->SetMarkerSize(1.4);
            bestScaleMarker->Draw("P");
        }

        TLegend legend(0.55, 0.68, 0.92, 0.88);
        legend.SetBorderSize(0);
        legend.SetFillStyle(0);
        legend.SetTextSize(0.035);
        legend.AddEntry(fracGraph.get(), "Fit results", "lp");
        if (bestScaleMarker) {
            legend.AddEntry(bestScaleMarker.get(), Form("Best scale = %.2f", result.bestScale), "p");
        }
        legend.Draw();

        latex.SetTextFont(42);
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.18, 0.93, "#bf{CMS} #it{Internal}");
        latex.SetTextSize(0.04);
        latex.DrawLatex(0.18, 0.87, "pp #sqrt{s_{NN}} = 5.36 TeV");
        latex.DrawLatex(0.18, 0.82, ptLabel.c_str());
        latex.DrawLatex(0.18, 0.77, cosLabel.c_str());
        if (result.success) {
            latex.SetTextSize(0.035);
            latex.DrawLatex(0.18, 0.71, Form("f_{prompt} = %.1f #pm %.1f%%", result.bestPromptFraction * 100.0, result.bestPromptFractionError * 100.0));
            if (std::isfinite(result.bestChi2OverNdf)) {
                latex.DrawLatex(0.18, 0.66, Form("#chi^{2}/NDF = %.2f", result.bestChi2OverNdf));
            }
        }

        cFrac->SaveAs((outputBase + "/prompt_fraction_vs_scale.png").c_str());
        cFrac->SaveAs((outputBase + "/prompt_fraction_vs_scale.pdf").c_str());
    }

    return result;
}

// Modified main function with histogram-based correction
void HistBasedCorrection(bool isMC /*=false*/, bool useAbsTheta /*=true*/){
    std::string subDir = SelectionCuts::SUB_DIR;
    std::string subStr = "bkgpol1";
    const std::string basePlotDir = "splot";
    gSystem->mkdir(basePlotDir.c_str(), kTRUE);
    std::string outputPlotDir = AppendPath(basePlotDir, subDir);
    gSystem->mkdir(outputPlotDir.c_str(), kTRUE);
    auto makeOutputPath = [&](const std::string& filename) {
        return AppendPath(outputPlotDir, filename);
    };
    ConfigureGlobalDatasetManager();

    DStarFitOpt config;
    config.SetDatasetName("datasetHX");
    if (gDatasetManager.IsLoaded()) {
        auto dataInfo = gDatasetManager.GetDataset("PbPb_Data_EPtransformation");
        if (!dataInfo.file.empty()) {
            config.SetDataFilePath(dataInfo.file);
            config.SetDatasetName(dataInfo.dataset_name.empty() ? "datasetHX" : dataInfo.dataset_name);
        }
        auto mcRdsInfo = gDatasetManager.GetDataset("PbPb_MC");
        if (!mcRdsInfo.dataset_name.empty()) {
            config.SetMCdatasetName(mcRdsInfo.dataset_name);
        }
    }
    config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
    config.SetOutputSubDir(SelectionCuts::SUB_DIR);
    config.SetUseAbsCosCuts(useAbsTheta);

    const std::string analysisBaseDir = ResolveAnalysisBaseDir();
    std::cout << "Using analysis output base directory: " << analysisBaseDir << std::endl;
    const std::string axisLabel = cosAxisLabel(useAbsTheta);
    const std::string displayLabel = cosDisplayLabel(useAbsTheta);

    std::vector<double> cosBin;
    if (useAbsTheta) {
        cosBin = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
    } else {
        cosBin = {-0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6};
    }

    std::vector<std::pair<double,double>> ptBins = {
        {5,7},
        {7,10},
        {10,20},
        // {20,50}
    };

    // Resolve efficiency MC sample from catalog
    const std::string efficiencyCatalog = DefaultEfficiencyCatalogPath();
    const std::string efficiencyDatasetKey = "PbPb_MC_Oct22";
    std::string mcFilePath = LoadDatasetFromCatalog(efficiencyCatalog, efficiencyDatasetKey);
    if (mcFilePath.empty()) {
        std::cerr << "[HistBasedCorrection] Failed to resolve efficiency dataset '" << efficiencyDatasetKey
                  << "' from " << efficiencyCatalog << std::endl;
        return;
    }
    config.SetMCFilePath(mcFilePath);

    auto mcGenRhoResults = ExtractMcGenRho00(mcFilePath, ptBins, outputPlotDir, 1.0, useAbsTheta);

    const std::vector<ProductionType> channels = {ProductionType::Prompt, ProductionType::NonPrompt};

    std::vector<double> ptCenters;
    std::vector<double> ptHalfWidths;
    ptCenters.reserve(ptBins.size());
    ptHalfWidths.reserve(ptBins.size());
    for (const auto& bin : ptBins) {
        ptCenters.push_back(0.5 * (bin.first + bin.second));
        ptHalfWidths.push_back(0.5 * (bin.second - bin.first));
    }

    std::map<ProductionType, TH2D*> efficiencyMaps;
    std::map<ProductionType, std::vector<TH1D*>> genProjectionMap;
    std::map<ProductionType, std::vector<TH1D*>> matchProjectionMap;

    for (ProductionType channel : channels) {
        std::vector<TH1D*> genProj;
        std::vector<TH1D*> matchProj;
        TH2D* eff = BuildEfficiencyMap(channel, mcFilePath, ptBins, cosBin, useAbsTheta, 1.0,
                                       isMC ? &genProj : nullptr,
                                       isMC ? &matchProj : nullptr);
        if (!eff) {
            std::cerr << "Warning: unable to construct " << PrettyLabel(channel)
                      << " efficiency map from MC." << std::endl;
            continue;
        }
        efficiencyMaps[channel] = eff;

        auto* effCanvas = new TCanvas(Form("c_effmap_%s", ToString(channel).c_str()),
                                      (PrettyLabel(channel) + " Efficiency Map").c_str(),
                                      800, 600);
        eff->Draw("COLZ");
        std::string effFileName = Form("efficiency_map_%s.png", ToString(channel).c_str());
        effCanvas->SaveAs(makeOutputPath(effFileName).c_str());
        delete effCanvas;

        if (isMC) {
            genProjectionMap[channel] = std::move(genProj);
            matchProjectionMap[channel] = std::move(matchProj);
        }
    }

    if (efficiencyMaps.empty() || !efficiencyMaps.count(ProductionType::Prompt)) {
        std::cerr << "Error: missing prompt efficiency map; aborting correction." << std::endl;
        return;
    }

    std::map<ProductionType, ChannelHistograms> channelHistMap;
    for (ProductionType channel : channels) {
        auto& bundle = channelHistMap[channel];
        bundle.corrected.resize(ptBins.size());
        bundle.correctedCB.resize(ptBins.size());
        bundle.weights.resize(ptBins.size());
        bundle.weightsCB.resize(ptBins.size());
        bundle.raw.resize(ptBins.size());
        bundle.match.resize(ptBins.size());
        bundle.matchCorrected.resize(ptBins.size());
        bundle.rawOverMatch.resize(ptBins.size());
        bundle.genCounts.resize(ptBins.size());
        bundle.cosThetaFits.resize(ptBins.size());
        bundle.rho00.resize(ptBins.size(), std::numeric_limits<double>::quiet_NaN());
        bundle.rho00Err.resize(ptBins.size(), std::numeric_limits<double>::quiet_NaN());

        const std::string channelTag = ToString(channel);
        const std::string channelLabel = PrettyLabel(channel);
        const Color_t rawColor = (channel == ProductionType::Prompt) ? kRed : kBlue + 1;
        const Color_t correctedColor = (channel == ProductionType::Prompt) ? kBlack : kAzure + 2;
        const Color_t matchColor = (channel == ProductionType::Prompt) ? static_cast<Color_t>(kMagenta + 2)
                                                                       : static_cast<Color_t>(kOrange + 7);
        const Color_t matchCorrectedColor = (channel == ProductionType::Prompt) ? static_cast<Color_t>(kOrange + 10)
                                                                               : static_cast<Color_t>(kGreen + 2);

        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            const std::string ptLabel = std::to_string(static_cast<int>(ptBins[ipt].first)) +
                                        "to" + std::to_string(static_cast<int>(ptBins[ipt].second));

            auto makeHistName = [&](const std::string& base) {
                return base + "_" + channelTag + "_pt_" + ptLabel;
            };
            auto makeHistTitle = [&](const std::string& quantity) {
                return channelLabel + " " + quantity + " vs " + displayLabel + "; " + axisLabel + "; " +
                       channelLabel + " " + quantity + " (" + ptLabel + " GeV/c)";
            };

            bundle.corrected[ipt] = new TH1D(makeHistName("corrected_yield").c_str(),
                                             makeHistTitle("Corrected Yield").c_str(),
                                             static_cast<int>(cosBin.size() - 1), cosBin.data());
            bundle.corrected[ipt]->Sumw2();
            bundle.corrected[ipt]->SetDirectory(nullptr);
            bundle.corrected[ipt]->SetMarkerStyle(20);
            bundle.corrected[ipt]->SetMarkerSize(1.0);
            bundle.corrected[ipt]->SetLineColor(correctedColor);
            bundle.corrected[ipt]->SetMarkerColor(correctedColor);

            bundle.correctedCB[ipt] = new TH1D(makeHistName("corrected_yield_cb").c_str(),
                                               makeHistTitle("Corrected Yield").c_str(),
                                               static_cast<int>(cosBin.size() - 1), cosBin.data());
            bundle.correctedCB[ipt]->Sumw2();
            bundle.correctedCB[ipt]->SetDirectory(nullptr);
            bundle.correctedCB[ipt]->SetMarkerStyle(20);
            bundle.correctedCB[ipt]->SetMarkerSize(1.0);
            bundle.correctedCB[ipt]->SetLineColor(correctedColor);
            bundle.correctedCB[ipt]->SetMarkerColor(correctedColor);

            bundle.weights[ipt] = new TH1D(makeHistName("weight").c_str(),
                                           makeHistTitle("Weighting Factor").c_str(),
                                           static_cast<int>(cosBin.size() - 1), cosBin.data());
            bundle.weights[ipt]->Sumw2();
            bundle.weights[ipt]->SetDirectory(nullptr);
            bundle.weights[ipt]->SetMarkerStyle(20);
            bundle.weights[ipt]->SetMarkerSize(1.0);
            bundle.weights[ipt]->SetMarkerColor(correctedColor);
            bundle.weights[ipt]->SetLineColor(correctedColor);

            bundle.weightsCB[ipt] = new TH1D(makeHistName("weight_cb").c_str(),
                                             makeHistTitle("Weighting Factor").c_str(),
                                             static_cast<int>(cosBin.size() - 1), cosBin.data());
            bundle.weightsCB[ipt]->Sumw2();
            bundle.weightsCB[ipt]->SetDirectory(nullptr);
            bundle.weightsCB[ipt]->SetMarkerStyle(20);
            bundle.weightsCB[ipt]->SetMarkerSize(1.0);
            bundle.weightsCB[ipt]->SetMarkerColor(correctedColor);
            bundle.weightsCB[ipt]->SetLineColor(correctedColor);

            bundle.raw[ipt] = new TH1D(makeHistName("raw_yield").c_str(),
                                       makeHistTitle("Raw Yield").c_str(),
                                       static_cast<int>(cosBin.size() - 1), cosBin.data());
            bundle.raw[ipt]->Sumw2();
            bundle.raw[ipt]->SetDirectory(nullptr);
            bundle.raw[ipt]->SetMarkerStyle(channel == ProductionType::Prompt ? 24 : 25);
            bundle.raw[ipt]->SetMarkerSize(1.0);
            bundle.raw[ipt]->SetMarkerColor(rawColor);
            bundle.raw[ipt]->SetLineColor(rawColor);

            bundle.matchCorrected[ipt] = new TH1D(makeHistName("matched_corrected").c_str(),
                                                  (channelLabel + " Matched Count (eff-corrected); " + axisLabel + "; " +
                                                   "Corrected Matched (" + ptLabel + " GeV/c)").c_str(),
                                                  static_cast<int>(cosBin.size() - 1), cosBin.data());
            bundle.matchCorrected[ipt]->Sumw2();
            bundle.matchCorrected[ipt]->SetDirectory(nullptr);
            bundle.matchCorrected[ipt]->SetMarkerStyle(27);
            bundle.matchCorrected[ipt]->SetMarkerSize(1.0);
            bundle.matchCorrected[ipt]->SetMarkerColor(matchCorrectedColor);
            bundle.matchCorrected[ipt]->SetLineColor(matchCorrectedColor);

            bundle.rawOverMatch[ipt] = new TH1D(makeHistName("raw_over_match").c_str(),
                                                (channelLabel + " Raw Yield / Matched Count; " + axisLabel + "; " +
                                                 "Raw / Matched (" + ptLabel + " GeV/c)").c_str(),
                                                static_cast<int>(cosBin.size() - 1), cosBin.data());
            bundle.rawOverMatch[ipt]->Sumw2();
            bundle.rawOverMatch[ipt]->SetDirectory(nullptr);
            bundle.rawOverMatch[ipt]->SetMarkerStyle(20);
            bundle.rawOverMatch[ipt]->SetMarkerSize(1.0);
            bundle.rawOverMatch[ipt]->SetMarkerColor(matchColor);
            bundle.rawOverMatch[ipt]->SetLineColor(matchColor);

            bundle.match[ipt] = nullptr;
            bundle.genCounts[ipt] = nullptr;

            TF1* fitFunc = new TF1(Form("fit_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                   "[0]*(1 - [1] + (3*[1] - 1)*x*x)",
                                   cosBin.front(), cosBin.back());
            fitFunc->SetParName(0, "N_{0}");
            fitFunc->SetParName(1, "#rho_{00}");
            fitFunc->SetParameter(0, 1.0);
            fitFunc->SetParameter(1, 1.0 / 3.0);
            fitFunc->SetParLimits(1, 0.0, 1.0);
            fitFunc->SetLineColor(correctedColor);
            fitFunc->SetLineStyle(9);
            fitFunc->SetLineWidth(2);
            bundle.cosThetaFits[ipt] = fitFunc;
        }

        if (isMC) {
            auto genIt = genProjectionMap.find(channel);
            if (genIt != genProjectionMap.end()) {
                auto& genProjVec = genIt->second;
                for (size_t ipt = 0; ipt < ptBins.size() && ipt < genProjVec.size(); ++ipt) {
                    bundle.genCounts[ipt] = genProjVec[ipt];
                    genProjVec[ipt] = nullptr;
                    if (bundle.genCounts[ipt]) {
                        bundle.genCounts[ipt]->SetMarkerStyle(21);
                        bundle.genCounts[ipt]->SetMarkerSize(1.0);
                        bundle.genCounts[ipt]->SetLineColor(kBlue);
                        bundle.genCounts[ipt]->SetMarkerColor(kBlue);
                    }
                }
            }

            auto matchIt = matchProjectionMap.find(channel);
            if (matchIt != matchProjectionMap.end()) {
                auto& matchProjVec = matchIt->second;
                for (size_t ipt = 0; ipt < ptBins.size() && ipt < matchProjVec.size(); ++ipt) {
                    bundle.match[ipt] = matchProjVec[ipt];
                    matchProjVec[ipt] = nullptr;
                    if (bundle.match[ipt]) {
                        bundle.match[ipt]->SetMarkerStyle(25);
                        bundle.match[ipt]->SetMarkerSize(1.0);
                        bundle.match[ipt]->SetLineColor(matchColor);
                        bundle.match[ipt]->SetMarkerColor(matchColor);
                    }
                }
            }
        }
    }

    const double scanScaleMin = 0.70;
    const double scanScaleMax = 1.40;
    const double scanScaleStep = 0.03;
    const std::vector<double> customScanScales;
    const bool makePerScalePlots = true;

    std::vector<std::vector<PromptFractionScanResult>> promptScanResults(
        ptBins.size(),
        std::vector<PromptFractionScanResult>(cosBin.size() - 1));

    std::vector<std::string> summaryDirs(ptBins.size());
    std::vector<std::string> summaryPtLabels(ptBins.size());
    std::vector<std::string> summaryCentLabels(ptBins.size());

    const size_t nPtBins = ptBins.size();
    const size_t nCosBins = (cosBin.size() > 0) ? cosBin.size() - 1 : 0;
    std::vector<std::vector<double>> totalRawYields(nPtBins, std::vector<double>(nCosBins, 0.0));
    std::vector<std::vector<double>> totalRawErrors(nPtBins, std::vector<double>(nCosBins, 0.0));
    std::map<ProductionType, std::vector<std::vector<double>>> channelFractions;
    std::map<ProductionType, std::vector<std::vector<double>>> channelFractionErrors;
    for (ProductionType channel : channels) {
        channelFractions[channel] = std::vector<std::vector<double>>(nPtBins, std::vector<double>(nCosBins, std::numeric_limits<double>::quiet_NaN()));
        channelFractionErrors[channel] = std::vector<std::vector<double>>(nPtBins, std::vector<double>(nCosBins, std::numeric_limits<double>::quiet_NaN()));
    }
    for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
        if (cosBin.size() < 2) {
            continue;
        }
        KinematicBin summaryBin(ptBins[ipt].first, ptBins[ipt].second,
                                cosBin.front(), cosBin[1], 0.0, 100.0);
        FitOpt summaryOpt = config.CreateFitOpt(summaryBin);
        summaryOpt.GenerateLegends();
        summaryPtLabels[ipt] = summaryOpt.pTLegend.empty() ?
            Form("%0.1f < p_{T} < %0.1f GeV/c", summaryBin.pTMin, summaryBin.pTMax) : summaryOpt.pTLegend;
        summaryCentLabels[ipt] = summaryOpt.centLegend.empty() ?
            Form("%0.0f-%0.0f%% centrality", summaryBin.centralityMin, summaryBin.centralityMax)
            : summaryOpt.centLegend;

        std::string basePath = AppendPath(analysisBaseDir, summaryOpt.outputDir);
        basePath = AppendPath(basePath, summaryOpt.subDir);
        basePath = AppendPath(basePath, "Data/dca_resolution_scan");
        basePath = AppendPath(basePath, summaryBin.GetBinName() + "_cosMulti");
        gSystem->mkdir(basePath.c_str(), true);
        summaryDirs[ipt] = basePath;
    }

    // Process each cos theta bin and pt bin
    for (size_t imva = 0; imva < cosBin.size() - 1; ++imva) {
        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            KinematicBin bin(ptBins[ipt].first, ptBins[ipt].second,
                              cosBin[imva], cosBin[imva + 1], 0.0, 100.0);
            FitOpt fitOpt = config.CreateFitOpt(bin);

            std::cout << "\n--- Processing bin: pT[" << bin.pTMin << "-" << bin.pTMax
                      << "], cos[" << bin.cosMin << "-" << bin.cosMax << "] ---" << std::endl;

            auto fitSummary = LoadFitResultSummary(fitOpt, analysisBaseDir, "Data");
            if (!fitSummary.success) {
                std::cerr << "Error: Could not retrieve fit summary for pT bin " << ipt
                          << ", cos bin " << imva << std::endl;
                continue;
            }

            const double rawYield = fitSummary.signalYield;
            const double rawYieldError = fitSummary.signalError;

            if (ipt < totalRawYields.size() && imva < totalRawYields[ipt].size()) {
                totalRawYields[ipt][imva] = rawYield;
                totalRawErrors[ipt][imva] = rawYieldError;
            }

            double promptFraction = 1.0;
            double promptFractionErr = 0.0;
            PromptFractionScanResult scanResult = RunPromptFractionScan(
                fitOpt,
                bin,
                analysisBaseDir,
                scanScaleMin,
                scanScaleMax,
                scanScaleStep,
                customScanScales,
                makePerScalePlots);

            if (scanResult.success) {
                promptFraction = scanResult.bestPromptFraction;
                promptFractionErr = scanResult.bestPromptFractionError;
                std::cout << "   Prompt fraction (scan) = " << promptFraction
                          << " ± " << promptFractionErr
                          << " (scale = " << scanResult.bestScale
                          << ", chi2/ndf = " << scanResult.bestChi2OverNdf << ")" << std::endl;
            } else if (LoadPromptFraction(fitOpt, bin, analysisBaseDir, promptFraction, promptFractionErr)) {
                std::cout << "   Prompt fraction (file) = " << promptFraction
                          << " ± " << promptFractionErr << std::endl;
                scanResult.success = true;
                scanResult.bestPromptFraction = promptFraction;
                scanResult.bestPromptFractionError = promptFractionErr;
            } else {
                std::cout << "   Prompt fraction unavailable; defaulting to 1.0" << std::endl;
                scanResult.success = false;
            }

            promptFraction = std::max(0.0, std::min(1.0, promptFraction));
            promptFractionErr = std::max(0.0, promptFractionErr);
            scanResult.bestPromptFraction = std::max(0.0, std::min(1.0, scanResult.bestPromptFraction));
            scanResult.bestPromptFractionError = std::max(0.0, scanResult.bestPromptFractionError);

            promptScanResults[ipt][imva] = scanResult;

            std::cout << "   Raw fit yield (nsig) = " << rawYield
                      << " ± " << rawYieldError << std::endl;

            for (ProductionType channel : channels) {
                auto effIt = efficiencyMaps.find(channel);
                if (effIt == efficiencyMaps.end()) {
                    continue;
                }

                auto& bundle = channelHistMap[channel];
                TH1D* rawHist = bundle.raw[ipt];
                TH1D* correctedHist = bundle.corrected[ipt];
                TH1D* correctedCBHist = bundle.correctedCB[ipt];
                TH1D* weightHist = bundle.weights[ipt];
                TH1D* weightCBHist = bundle.weightsCB[ipt];

                if (!rawHist || !correctedHist || !correctedCBHist || !weightHist || !weightCBHist) {
                    continue;
                }

                double fraction = 1.0;
                double fractionErr = 0.0;
                    if (channel == ProductionType::Prompt) {
                        fraction = promptFraction;
                        fractionErr = promptFractionErr;
                    } else {
                        fraction = std::max(0.0, 1.0 - promptFraction);
                        fractionErr = promptFractionErr;
                    }
                    

                const double channelYield = rawYield * fraction;
                const double channelYieldErr = std::sqrt(std::pow(fraction * rawYieldError, 2.0) +
                                                         std::pow(rawYield * fractionErr, 2.0));

                std::cout << "   [" << PrettyLabel(channel) << "] raw yield = " << channelYield
                          << " ± " << channelYieldErr << std::endl;

                if (ipt < channelFractions[channel].size() && imva < channelFractions[channel][ipt].size()) {
                    channelFractions[channel][ipt][imva] = fraction;
                    channelFractionErrors[channel][ipt][imva] = fractionErr;
                }

                rawHist->SetBinContent(imva + 1, channelYield);
                rawHist->SetBinError(imva + 1, channelYieldErr);

                TH1D* matchHist = bundle.match[ipt];
                TH1D* matchCorrectedHist = bundle.matchCorrected[ipt];
                TH1D* ratioHist = bundle.rawOverMatch[ipt];

                double matchedCount = 0.0;
                double matchedError = 0.0;
                double ratioValue = 0.0;
                double ratioError = 0.0;

                if (matchHist) {
                    matchedCount = matchHist->GetBinContent(imva + 1);
                    matchedError = matchHist->GetBinError(imva + 1);
                    if (matchedCount > 0.0) {
                        ratioValue = channelYield / matchedCount;
                        ratioError = std::sqrt(std::pow(channelYieldErr / matchedCount, 2.0) +
                                               std::pow(channelYield * matchedError /
                                                        (matchedCount * matchedCount), 2.0));
                    }
                    if (ratioHist) {
                        ratioHist->SetBinContent(imva + 1, matchedCount > 0.0 ? ratioValue : 0.0);
                        ratioHist->SetBinError(imva + 1, matchedCount > 0.0 ? ratioError : 0.0);
                    }

                    std::cout << "   [" << PrettyLabel(channel) << "] Matched reco (matchGEN==1) = "
                              << matchedCount << " ± " << matchedError;
                    if (matchedCount > 0.0) {
                        std::cout << "; Raw/Matched = " << ratioValue << " ± " << ratioError;
                    } else {
                        std::cout << "; Raw/Matched = N/A (matched count is zero)";
                    }
                    std::cout << std::endl;
                }

                std::vector<double> correctionResults = ApplyHistogramCorrection(channelYield, channelYieldErr,
                                                                               effIt->second,
                                                                               bin.pTMin, bin.pTMax,
                                                                               bin.cosMin, bin.cosMax);

                const double correctedYield = correctionResults[0];
                const double correctedYieldErr = correctionResults[1];
                const double correctionFactor = (channelYield > 0.0) ? correctedYield / channelYield : 0.0;

                std::cout << "   [" << PrettyLabel(channel) << "] Corrected yield = " << correctedYield
                          << " ± " << correctedYieldErr << ", factor = " << correctionFactor << std::endl;

                correctedHist->SetBinContent(imva + 1, correctedYield);
                correctedHist->SetBinError(imva + 1, correctedYieldErr);
                correctedCBHist->SetBinContent(imva + 1, correctedYield);
                correctedCBHist->SetBinError(imva + 1, correctedYieldErr);
                weightHist->SetBinContent(imva + 1, correctionFactor);
                weightCBHist->SetBinContent(imva + 1, correctionFactor);

                if (matchCorrectedHist) {
                    const double correctedMatched = matchedCount * correctionFactor;
                    const double correctedMatchedErr = matchedError * correctionFactor;
                    matchCorrectedHist->SetBinContent(imva + 1, matchedCount > 0.0 ? correctedMatched : 0.0);
                    matchCorrectedHist->SetBinError(imva + 1, matchedCount > 0.0 ? correctedMatchedErr : 0.0);
                }
            }
        }
    }

    // Summarise optimal prompt fractions vs cos#theta per pT bin
    if (cosBin.size() > 1) {
        std::vector<double> cosCenters(cosBin.size() - 1);
        std::vector<double> cosHalfWidths(cosBin.size() - 1);
        for (size_t i = 0; i < cosBin.size() - 1; ++i) {
            cosCenters[i] = 0.5 * (cosBin[i] + cosBin[i + 1]);
            cosHalfWidths[i] = 0.5 * std::abs(cosBin[i + 1] - cosBin[i]);
        }

        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            if (summaryDirs[ipt].empty()) {
                continue;
            }

            auto graph = std::make_unique<TGraphErrors>();
            double yMax = 0.0;
            int pointIndex = 0;
            std::vector<std::tuple<double, double, double>> annotations;

            for (size_t icos = 0; icos < cosCenters.size(); ++icos) {
                const auto& scanRes = promptScanResults[ipt][icos];
                if (!scanRes.success) {
                    continue;
                }
                double frac = scanRes.bestPromptFraction;
                if (!std::isfinite(frac)) {
                    continue;
                }
                double err = scanRes.bestPromptFractionError;
                double yVal = frac * 100.0;
                double yErr = err * 100.0;
                double xCenter = cosCenters[icos];
                double xErr = cosHalfWidths[icos];

                graph->SetPoint(pointIndex, xCenter, yVal);
                graph->SetPointError(pointIndex, xErr, yErr);
                yMax = std::max(yMax, yVal + yErr);
                annotations.emplace_back(xCenter, yVal + yErr, scanRes.bestScale);
                ++pointIndex;
            }

            if (graph->GetN() == 0) {
                continue;
            }

            double yUpper = std::min(110.0, std::max(30.0, yMax + 8.0));

            std::string canvasName = Form("promptFrac_cos_pt_%zu_c", ipt);
            auto canvas = std::make_unique<TCanvas>(canvasName.c_str(), "", 900, 700);
            canvas->SetGrid();

            const std::string axisLabelHx = useAbsTheta ? "|cos#theta_{HX}|" : "cos#theta_{HX}";

            graph->SetTitle(Form(";%s;Prompt fraction (%%)", axisLabelHx.c_str()));
            graph->SetMarkerStyle(20);
            graph->SetMarkerSize(1.2);
            graph->SetLineWidth(2);
            graph->Draw("AP");
            graph->GetYaxis()->SetTitleOffset(1.3);
            graph->GetYaxis()->SetRangeUser(0.0, yUpper);

            TLatex header;
            header.SetNDC();
            header.SetTextFont(42);
            header.SetTextAlign(31);
            header.SetTextSize(0.045);
            header.DrawLatex(0.88, 0.88, "#bf{CMS} #it{Internal}");
            header.SetTextSize(0.040);
            if (!summaryPtLabels[ipt].empty()) {
                header.DrawLatex(0.88, 0.82, summaryPtLabels[ipt].c_str());
            }
            if (!summaryCentLabels[ipt].empty()) {
                header.DrawLatex(0.88, 0.76, summaryCentLabels[ipt].c_str());
            }

            TLatex scaleLatex;
            scaleLatex.SetTextFont(42);
            scaleLatex.SetTextSize(0.035);
            scaleLatex.SetTextAlign(21);
            scaleLatex.SetNDC(false);
            for (const auto& entry : annotations) {
                double x = std::get<0>(entry);
                double baseY = std::get<1>(entry);
                double scale = std::get<2>(entry);
                if (!std::isfinite(scale)) {
                    continue;
                }
                double yText = std::min(yUpper - 2.0, baseY + 4.0);
                scaleLatex.DrawLatex(x, yText, Form("scale = %.2f", scale));
            }

            std::string summaryFileBase = AppendPath(summaryDirs[ipt], "prompt_fraction_vs_cos");
            canvas->SaveAs((summaryFileBase + ".png").c_str());
            canvas->SaveAs((summaryFileBase + ".pdf").c_str());
        }
    }

    // Fit corrected yields with the polarisation-inspired function per channel/pT bin
    for (ProductionType channel : channels) {
        auto mapIt = channelHistMap.find(channel);
        if (mapIt == channelHistMap.end()) {
            continue;
        }
        auto& bundle = mapIt->second;
        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            TH1D* hist = bundle.corrected[ipt];
            TF1* func = bundle.cosThetaFits[ipt];
            if (!hist || !func) {
                continue;
            }
            if (hist->GetEntries() < 3) {
                continue;
            }

            const double xmin = hist->GetXaxis()->GetXmin();
            const double xmax = hist->GetXaxis()->GetXmax();
            func->SetRange(xmin, xmax);

            double initialNorm = hist->Integral("width");
            if (initialNorm <= 0.0) {
                initialNorm = hist->Integral();
            }
            if (initialNorm <= 0.0) {
                initialNorm = std::max(1.0, hist->GetMaximum());
            }
            func->SetParameter(0, initialNorm);
            func->SetParameter(1, 1.0 / 3.0);

            TFitResultPtr fitRes = hist->Fit(func, "RQ0S");
            int fitStatus = fitRes ? fitRes->Status() : -1;
            if (fitStatus != 0) {
                std::cout << "   [" << PrettyLabel(channel) << "] Warning: fit returned status "
                          << fitStatus << " for pT [" << ptBins[ipt].first << ", "
                          << ptBins[ipt].second << ")" << std::endl;
            }

            const double rho00 = func->GetParameter(1);
            const double rho00Err = func->GetParError(1);
            bundle.rho00[ipt] = rho00;
            bundle.rho00Err[ipt] = rho00Err;

            std::cout << "   [" << PrettyLabel(channel) << "] Fit result ρ_{00} (pT "
                      << ptBins[ipt].first << "-" << ptBins[ipt].second << ") = "
                      << rho00 << " ± " << rho00Err << std::endl;
        }
    }

    // Set custom bin labels for X-axis to make bins appear equally wide
    for (ProductionType channel : channels) {
        auto histIt = channelHistMap.find(channel);
        if (histIt == channelHistMap.end()) {
            continue;
        }
        auto& bundle = histIt->second;
        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            std::vector<TH1D*> histsToLabel = {
                bundle.corrected[ipt], bundle.weights[ipt], bundle.raw[ipt],
                bundle.correctedCB[ipt], bundle.weightsCB[ipt],
                bundle.match[ipt], bundle.matchCorrected[ipt], bundle.rawOverMatch[ipt], bundle.genCounts[ipt]
            };
            for (TH1D* h : histsToLabel) {
                if (!h) continue;
                TAxis* xAxis = h->GetXaxis();
                for (int binIdx = 1; binIdx <= xAxis->GetNbins(); ++binIdx) {
                    if (binIdx - 1 < static_cast<int>(cosBin.size())) {
                        xAxis->SetBinLabel(binIdx, Form("%.1f", cosBin[binIdx - 1]));
                    }
                }
                xAxis->SetLabelOffset(0.01);
            }
        }
    }

    // Export per-bin yields for convenient downstream use
    {
        const std::string yieldOutputPath = AppendPath(outputPlotDir, "yield_summary.csv");
        std::ofstream yieldCsv(yieldOutputPath);
        if (!yieldCsv) {
            std::cerr << "[YieldExport] Error: unable to open " << yieldOutputPath
                      << " for writing." << std::endl;
        } else {
            yieldCsv.setf(std::ios::fixed);
            yieldCsv << std::setprecision(6);
            yieldCsv << "channel,pt_bin,pt_min,pt_max,cos_bin,cos_min,cos_max,total_raw,total_raw_error,fraction,fraction_error,raw_yield,raw_error,corrected_yield,corrected_error,correction_factor\n";

            for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
                const double ptMin = ptBins[ipt].first;
                const double ptMax = ptBins[ipt].second;
                for (size_t icos = 0; icos < cosBin.size() - 1; ++icos) {
                    const double cosMin = cosBin[icos];
                    const double cosMax = cosBin[icos + 1];
                    const int histBin = static_cast<int>(icos) + 1;

                    bool wroteChannel = false;
                    double totalRaw = 0.0;
                    double totalRawErrSq = 0.0;
                    double totalCorrected = 0.0;
                    double totalCorrectedErrSq = 0.0;

                    for (ProductionType channel : channels) {
                        auto mapIt = channelHistMap.find(channel);
                        if (mapIt == channelHistMap.end()) {
                            continue;
                        }

                        const ChannelHistograms& bundle = mapIt->second;
                        TH1D* rawHist = (ipt < bundle.raw.size()) ? bundle.raw[ipt] : nullptr;
                        TH1D* correctedHist = (ipt < bundle.corrected.size()) ? bundle.corrected[ipt] : nullptr;
                        TH1D* weightHist = (ipt < bundle.weights.size()) ? bundle.weights[ipt] : nullptr;

                        if (!rawHist && !correctedHist) {
                            continue;
                        }

                        const double rawValue = rawHist ? rawHist->GetBinContent(histBin) : 0.0;
                        const double rawError = rawHist ? rawHist->GetBinError(histBin) : 0.0;
                        const double correctedValue = correctedHist ? correctedHist->GetBinContent(histBin) : 0.0;
                        const double correctedError = correctedHist ? correctedHist->GetBinError(histBin) : 0.0;
                        const double totalRawFit = (ipt < totalRawYields.size() && icos < totalRawYields[ipt].size())
                                                   ? totalRawYields[ipt][icos]
                                                   : 0.0;
                        const double totalRawFitErr = (ipt < totalRawErrors.size() && icos < totalRawErrors[ipt].size())
                                                       ? totalRawErrors[ipt][icos]
                                                       : 0.0;

                        double fractionVal = std::numeric_limits<double>::quiet_NaN();
                        double fractionErrVal = std::numeric_limits<double>::quiet_NaN();
                        auto fracIt = channelFractions.find(channel);
                        if (fracIt != channelFractions.end() && ipt < fracIt->second.size() && icos < fracIt->second[ipt].size()) {
                            fractionVal = fracIt->second[ipt][icos];
                        }
                        auto fracErrIt = channelFractionErrors.find(channel);
                        if (fracErrIt != channelFractionErrors.end() && ipt < fracErrIt->second.size() && icos < fracErrIt->second[ipt].size()) {
                            fractionErrVal = fracErrIt->second[ipt][icos];
                        }

                        double weightValue = 0.0;
                        if (weightHist) {
                            weightValue = weightHist->GetBinContent(histBin);
                        } else if (rawValue > 0.0) {
                            weightValue = correctedValue / rawValue;
                        }

                        yieldCsv << ToString(channel) << ','
                                 << ipt << ','
                                 << ptMin << ','
                                 << ptMax << ','
                                 << icos << ','
                                 << cosMin << ','
                                 << cosMax << ','
                                 << totalRawFit << ','
                                 << totalRawFitErr << ','
                                 << (std::isfinite(fractionVal) ? fractionVal : 0.0) << ','
                                 << (std::isfinite(fractionErrVal) ? fractionErrVal : 0.0) << ','
                                 << rawValue << ','
                                 << rawError << ','
                                 << correctedValue << ','
                                 << correctedError << ','
                                 << weightValue << '\n';

                        wroteChannel = true;
                        totalRaw += rawValue;
                        totalRawErrSq += rawError * rawError;
                        totalCorrected += correctedValue;
                        totalCorrectedErrSq += correctedError * correctedError;
                    }

                    if (wroteChannel) {
                        const double totalRawError = std::sqrt(totalRawErrSq);
                        const double totalCorrectedError = std::sqrt(totalCorrectedErrSq);
                        const double totalWeight = (totalRaw > 0.0) ? totalCorrected / totalRaw : 0.0;
                        yieldCsv << "total,"
                                 << ipt << ','
                                 << ptMin << ','
                                 << ptMax << ','
                                 << icos << ','
                                 << cosMin << ','
                                 << cosMax << ','
                                 << totalRaw << ','
                                 << totalRawError << ','
                                 << ','
                                 << ','
                                 << totalRaw << ','
                                 << totalRawError << ','
                                 << totalCorrected << ','
                                 << totalCorrectedError << ','
                                 << totalWeight << '\n';
                    }
                }
            }

            std::cout << "[YieldExport] Wrote yield summary to " << yieldOutputPath << std::endl;
        }
    }

    // Plotting section
    TLatex latex;
    gStyle->SetOptStat(0);
    latex.SetNDC();
    latex.SetTextSize(0.04);

    for (ProductionType channel : channels) {
        auto effIt = efficiencyMaps.find(channel);
        if (effIt == efficiencyMaps.end()) {
            continue;
        }

        auto& bundle = channelHistMap[channel];
        const std::string channelTag = ToString(channel);
        const std::string channelLabel = PrettyLabel(channel);
        const Color_t rawColor = (channel == ProductionType::Prompt) ? kRed : kBlue + 1;
        const Color_t correctedColor = (channel == ProductionType::Prompt) ? kBlack : kAzure + 2;
        const Color_t matchColor = (channel == ProductionType::Prompt) ? static_cast<Color_t>(kMagenta + 2)
                                                                       : static_cast<Color_t>(kOrange + 7);
        const Color_t matchCorrectedColor = (channel == ProductionType::Prompt) ? static_cast<Color_t>(kOrange + 10)
                                                                               : static_cast<Color_t>(kGreen + 2);

        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            auto* rawHist = bundle.raw[ipt];
            auto* correctedHist = bundle.corrected[ipt];
            auto* correctedCBHist = bundle.correctedCB[ipt];
            if (!rawHist || !correctedHist || !correctedCBHist) {
                continue;
            }

            const std::string ptLabel = std::to_string(static_cast<int>(ptBins[ipt].first)) +
                                        "to" + std::to_string(static_cast<int>(ptBins[ipt].second));

            TCanvas* c_raw = new TCanvas(Form("c_raw_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                         (channelLabel + " Raw Yield pt " + ptLabel).c_str(), 800, 600);
            if (rawHist->GetMaximum() > 0.0) {
                rawHist->GetYaxis()->SetRangeUser(0.0, rawHist->GetMaximum() * 1.2);
            }
            rawHist->SetLineColor(rawColor);
            rawHist->SetMarkerColor(rawColor);
            rawHist->Draw("PE1");
            latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f GeV/c", ptBins[ipt].first, ptBins[ipt].second));
            c_raw->SetGrid(1, 1);
            std::string rawFileName = Form("splot_%s_raw_yield_pt_%s_%s.png",
                                            channelTag.c_str(), subStr.c_str(), ptLabel.c_str());
            c_raw->SaveAs(makeOutputPath(rawFileName).c_str());
            delete c_raw;

            // Canvas for correction factor histogram
            TH1D* weightHist = bundle.weights[ipt];
            if (weightHist) {
                TCanvas* c_weight = new TCanvas(Form("c_weight_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                                (channelLabel + " Correction Factor pt " + ptLabel).c_str(), 800, 600);
                if (weightHist->GetMaximum() > 0.0) {
                    weightHist->GetYaxis()->SetRangeUser(0.0, weightHist->GetMaximum() * 1.2);
                }
                weightHist->SetLineColor(correctedColor);
                weightHist->SetMarkerColor(correctedColor);
                weightHist->Draw("PE1");
                weightHist->GetXaxis()->SetTitle(axisLabel.c_str());
                weightHist->GetYaxis()->SetTitle("Correction factor");
                latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f GeV/c", ptBins[ipt].first, ptBins[ipt].second));
                c_weight->SetGrid(1, 1);
                std::string weightFileName = Form("splot_%s_correction_factor_pt_%s_%s.png",
                                                   channelTag.c_str(), subStr.c_str(), ptLabel.c_str());
                c_weight->SaveAs(makeOutputPath(weightFileName).c_str());
                delete c_weight;
            }

            TCanvas* c_corr = new TCanvas(Form("c_corrected_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                          (channelLabel + " Corrected Yield pt " + ptLabel).c_str(), 800, 600);
            if (correctedHist->GetMaximum() > 0.0) {
                correctedHist->GetYaxis()->SetRangeUser(0.0, correctedHist->GetMaximum() * 1.2);
            }
            correctedHist->Draw("PE1");
            correctedHist->GetXaxis()->SetTitle(axisLabel.c_str());
            correctedHist->GetYaxis()->SetTitle("Yield");
            latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f GeV/c", ptBins[ipt].first, ptBins[ipt].second));
            c_corr->SetGrid(1, 1);
            std::string corrFileName = Form("splot_%s_corrected_yield_pt_%s_%s.png",
                                             channelTag.c_str(), subStr.c_str(), ptLabel.c_str());
            if (bundle.cosThetaFits[ipt]) {
                bundle.cosThetaFits[ipt]->SetLineColor(correctedColor);
                bundle.cosThetaFits[ipt]->Draw("SAME");

                // Add rho00 value and fit formula (FlatTest style)
                double rho00_val = bundle.cosThetaFits[ipt]->GetParameter(1);
                double rho00_err = bundle.cosThetaFits[ipt]->GetParError(1);
                TLatex texRho;
                texRho.SetNDC();
                texRho.SetTextSize(0.04);
                texRho.DrawLatex(0.25, 0.78, Form("#rho_{00} = %.3f #pm %.3f", rho00_val, rho00_err));
                TLatex texEq;
                texEq.SetNDC();
                texEq.SetTextSize(0.035);
                texEq.DrawLatex(0.25, 0.70, "N_{0}(1-#rho_{00}+(3#rho_{00}-1)cos^{2}#theta)");
            }
            c_corr->SaveAs(makeOutputPath(corrFileName).c_str());
            delete c_corr;

            TCanvas* c_comp = new TCanvas(Form("c_comp_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                          (channelLabel + " Yield Comparison pt " + ptLabel).c_str(), 800, 600);
            TLegend* leg = new TLegend(0.11, 0.11, 0.31, 0.27);
            leg->SetBorderSize(0);
            leg->AddEntry(correctedHist, (channelLabel + " Corrected Yield").c_str(), "pe");
            leg->AddEntry(rawHist, (channelLabel + " Raw Yield").c_str(), "pe");
            leg->AddEntry(correctedCBHist, (channelLabel + " Corrected Yield CutBased").c_str(), "pe");

            std::vector<double> maxima = {correctedHist->GetMaximum(), correctedCBHist->GetMaximum(), rawHist->GetMaximum()};
            std::vector<double> minima = {correctedHist->GetMinimum(), correctedCBHist->GetMinimum(), rawHist->GetMinimum()};

            TH1D* matchHist = bundle.match[ipt];
            TH1D* matchCorrectedHist = bundle.matchCorrected[ipt];
            if (matchHist) {
                maxima.push_back(matchHist->GetMaximum());
                minima.push_back(matchHist->GetMinimum());
                leg->AddEntry(matchHist, (channelLabel + " Matched Reco").c_str(), "pe");
            }
            if (matchCorrectedHist) {
                maxima.push_back(matchCorrectedHist->GetMaximum());
                minima.push_back(matchCorrectedHist->GetMinimum());
                leg->AddEntry(matchCorrectedHist, (channelLabel + " Matched (eff-corrected)").c_str(), "pe");
            }

            double maxYield = 0.0;
            for (double val : maxima) {
                if (val > maxYield) maxYield = val;
            }
            double minYield = 0.0;
            bool minInitialised = false;
            for (double val : minima) {
                if (!minInitialised) {
                    minYield = val;
                    minInitialised = true;
                } else if (val < minYield) {
                    minYield = val;
                }
            }
            if (maxYield > 0.0) {
                correctedCBHist->GetYaxis()->SetRangeUser(0.8 * minYield, maxYield * 1.2);
            }
            correctedCBHist->GetXaxis()->SetTitle(axisLabel.c_str());

            correctedCBHist->Draw("PE1");
            correctedHist->Draw("PE1 SAME");
            rawHist->Draw("PE1 SAME");
            if (matchHist) {
                matchHist->SetMarkerColor(matchColor);
                matchHist->SetLineColor(matchColor);
                matchHist->Draw("PE1 SAME");
            }
            if (matchCorrectedHist) {
                matchCorrectedHist->SetMarkerColor(matchCorrectedColor);
                matchCorrectedHist->SetLineColor(matchCorrectedColor);
                matchCorrectedHist->Draw("PE1 SAME");
            }
            if (bundle.cosThetaFits[ipt]) {
                bundle.cosThetaFits[ipt]->SetLineColor(correctedColor);
                bundle.cosThetaFits[ipt]->Draw("SAME");

                // Add rho00 and formula also on comparison canvas
                double rho00_val = bundle.cosThetaFits[ipt]->GetParameter(1);
                double rho00_err = bundle.cosThetaFits[ipt]->GetParError(1);
                TLatex texRho2;
                texRho2.SetNDC();
                texRho2.SetTextSize(0.04);
                texRho2.DrawLatex(0.25, 0.78, Form("#rho_{00} = %.3f #pm %.3f", rho00_val, rho00_err));
                TLatex texEq2;
                texEq2.SetNDC();
                texEq2.SetTextSize(0.035);
                texEq2.DrawLatex(0.25, 0.70, "N_{0}(1-#rho_{00}+(3#rho_{00}-1)cos^{2}#theta)");
            }
            leg->Draw();
            latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f GeV/c", ptBins[ipt].first, ptBins[ipt].second));
            c_comp->SetGrid(1, 1);
            std::string compFileName = Form("splot_%s_yield_comparison_pt_%s.png", channelTag.c_str(), ptLabel.c_str());
            c_comp->SaveAs(makeOutputPath(compFileName).c_str());
            delete c_comp;

            if (isMC && bundle.genCounts[ipt]) {
                TCanvas* c_gen = new TCanvas(Form("c_gen_vs_corrected_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                             (channelLabel + " MC Yield Comparison pt " + ptLabel).c_str(),
                                             800, 600);
                double maxVal = 0.0;
                std::vector<TH1D*> toConsider = {rawHist, correctedHist, bundle.genCounts[ipt], matchHist, matchCorrectedHist};
                for (TH1D* h : toConsider) {
                    if (h && h->GetMaximum() > maxVal) {
                        maxVal = h->GetMaximum();
                    }
                }
                if (maxVal > 0.0) {
                    rawHist->GetYaxis()->SetRangeUser(0.0, maxVal * 1.3);
                }
                rawHist->Draw("PE1");
                correctedHist->Draw("PE1 SAME");
                bundle.genCounts[ipt]->Draw("PE1 SAME");
                if (matchHist) matchHist->Draw("PE1 SAME");
                if (matchCorrectedHist) matchCorrectedHist->Draw("PE1 SAME");
                if (bundle.cosThetaFits[ipt]) {
                    bundle.cosThetaFits[ipt]->SetLineColor(correctedColor);
                    bundle.cosThetaFits[ipt]->Draw("SAME");
                }

                TLegend genLeg(0.12, 0.12, 0.46, 0.36);
                genLeg.SetBorderSize(0);
                genLeg.AddEntry(rawHist, (channelLabel + " Raw Fit Yield").c_str(), "pe");
                genLeg.AddEntry(correctedHist, (channelLabel + " Corrected Yield").c_str(), "pe");
                genLeg.AddEntry(bundle.genCounts[ipt], (channelLabel + " Generated").c_str(), "pe");
                if (matchHist) genLeg.AddEntry(matchHist, (channelLabel + " Matched Reco").c_str(), "pe");
                if (matchCorrectedHist) genLeg.AddEntry(matchCorrectedHist, (channelLabel + " Matched (eff-corrected)").c_str(), "pe");
                genLeg.Draw();
                latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f GeV/c", ptBins[ipt].first, ptBins[ipt].second));
                c_gen->SetGrid(1, 1);
                std::string genFileName = Form("splot_%s_gen_vs_corrected_pt_%s.png", channelTag.c_str(), ptLabel.c_str());
                c_gen->SaveAs(makeOutputPath(genFileName).c_str());
                delete c_gen;

                std::unique_ptr<TH1D> ratioHist(static_cast<TH1D*>(correctedHist->Clone(Form("ratio_corrected_gen_%s_pt_%s",
                                                                                               channelTag.c_str(), ptLabel.c_str()))));
                if (ratioHist) {
                    ratioHist->SetDirectory(nullptr);
                    ratioHist->Divide(bundle.genCounts[ipt]);
                    TCanvas* c_ratio = new TCanvas(Form("c_ratio_corrected_gen_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                                   (channelLabel + " Corrected / Generated pt " + ptLabel).c_str(),
                                                   800, 400);
                    ratioHist->SetMarkerStyle(20);
                    ratioHist->SetMarkerColor(correctedColor);
                    ratioHist->SetLineColor(correctedColor);
                    ratioHist->Draw("PE1");
                    ratioHist->GetYaxis()->SetTitle("Corrected / Generated");
                    ratioHist->GetYaxis()->SetRangeUser(0.0, std::max(2.0, ratioHist->GetMaximum() * 1.2));
                    latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f GeV/c", ptBins[ipt].first, ptBins[ipt].second));
                    c_ratio->SetGrid(1, 1);
                    std::string ratioFileName = Form("splot_%s_corrected_over_generated_pt_%s.png",
                                                     channelTag.c_str(), ptLabel.c_str());
                    c_ratio->SaveAs(makeOutputPath(ratioFileName).c_str());
                    delete c_ratio;
                }
            }

            if (isMC && matchHist && bundle.rawOverMatch[ipt]) {
                TCanvas* c_ratio_match = new TCanvas(Form("c_ratio_raw_match_%s_pt_%s", channelTag.c_str(), ptLabel.c_str()),
                                                     (channelLabel + " Raw / Matched pt " + ptLabel).c_str(),
                                                     800, 400);
                TH1D* ratioHist = bundle.rawOverMatch[ipt];
                double ratioMax = ratioHist->GetMaximum();
                double ratioMin = ratioHist->GetMinimum();
                if (ratioMax <= 0.0) ratioMax = 2.0;
                if (ratioMin < 0.0) {
                    ratioHist->GetYaxis()->SetRangeUser(ratioMin * 1.2, ratioMax * 1.2);
                } else {
                    ratioHist->GetYaxis()->SetRangeUser(0.0, ratioMax * 1.2);
                }
                ratioHist->Draw("PE1");
                ratioHist->GetYaxis()->SetTitle((channelLabel + " Raw / Matched").c_str());
                latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f GeV/c", ptBins[ipt].first, ptBins[ipt].second));
                c_ratio_match->SetGrid(1, 1);
                std::string ratioMatchFile = Form("splot_%s_raw_over_match_pt_%s.png", channelTag.c_str(), ptLabel.c_str());
                c_ratio_match->SaveAs(makeOutputPath(ratioMatchFile).c_str());
                delete c_ratio_match;
            }
        }


        // rho00 vs pT graph
        const McGenRho00Result* mcChannel = nullptr;
        auto mcIt = mcGenRhoResults.find(channel);
        if (mcIt != mcGenRhoResults.end()) {
            mcChannel = &mcIt->second;
        }

        std::vector<double> graphX;
        std::vector<double> graphY;
        std::vector<double> graphEX;
        std::vector<double> graphEY;
        std::vector<double> mcGraphX;
        std::vector<double> mcGraphY;
        std::vector<double> mcGraphEX;
        std::vector<double> mcGraphEY;

        for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
            const double val = bundle.rho00[ipt];
            const double err = bundle.rho00Err[ipt];
            if (std::isfinite(val) && std::isfinite(err)) {
                graphX.push_back(ptCenters[ipt]);
                graphY.push_back(val);
                graphEX.push_back(ptHalfWidths[ipt]);
                graphEY.push_back(err);
            }

            if (mcChannel && ipt < mcChannel->rho00.size()) {
                const double mcVal = mcChannel->rho00[ipt];
                const double mcErr = (ipt < mcChannel->rho00Err.size()) ? mcChannel->rho00Err[ipt] : std::numeric_limits<double>::quiet_NaN();
                if (std::isfinite(mcVal) && std::isfinite(mcErr)) {
                    mcGraphX.push_back(ptCenters[ipt]);
                    mcGraphY.push_back(mcVal);
                    mcGraphEX.push_back(ptHalfWidths[ipt]);
                    mcGraphEY.push_back(mcErr);
                }
            }
        }

        if (!graphX.empty()) {
            TGraphErrors rhoGraph(static_cast<int>(graphX.size()), graphX.data(), graphY.data(),
                                  graphEX.data(), graphEY.data());
            rhoGraph.SetTitle((channelLabel + " #rho_{00} vs p_{T};p_{T} (GeV/c);#rho_{00}").c_str());
            rhoGraph.SetMarkerStyle(channel == ProductionType::Prompt ? 20 : 21);
            rhoGraph.SetMarkerSize(1.1);
            Color_t dataColor = (channel == ProductionType::Prompt) ? static_cast<Color_t>(kRed)
                                                                    : static_cast<Color_t>(kBlue + 1);
            rhoGraph.SetMarkerColor(dataColor);
            rhoGraph.SetLineColor(dataColor);

            TGraphErrors* mcGraphPtr = nullptr;
            if (!mcGraphX.empty()) {
                mcGraphPtr = new TGraphErrors(static_cast<int>(mcGraphX.size()), mcGraphX.data(), mcGraphY.data(),
                                              mcGraphEX.data(), mcGraphEY.data());
                Color_t mcColor = (channel == ProductionType::Prompt) ? static_cast<Color_t>(kRed + 1)
                                                                      : static_cast<Color_t>(kBlue + 3);
                mcGraphPtr->SetLineColor(mcColor);
                mcGraphPtr->SetLineWidth(2);
                mcGraphPtr->SetFillColorAlpha(mcColor, 0.25);
                mcGraphPtr->SetMarkerStyle(0);
            }

            TCanvas c_rho(Form("c_rho00_%s", channelTag.c_str()),
                          (channelLabel + " #rho_{00} vs p_{T}").c_str(), 700, 600);
            rhoGraph.GetYaxis()->SetRangeUser(0.0, 1.0);
            rhoGraph.Draw("AP");
            if (mcGraphPtr) {
                mcGraphPtr->Draw("E SAME");
                // mcGraphPtr->Draw("L SAME");
                rhoGraph.Draw("P SAME");
            }
            if (!mcGraphPtr) {
                TLine lineUnpol(rhoGraph.GetXaxis()->GetXmin(), 1.0 / 3.0,
                                rhoGraph.GetXaxis()->GetXmax(), 1.0 / 3.0);
                lineUnpol.SetLineStyle(7);
                lineUnpol.SetLineColor(kGray + 2);
                lineUnpol.Draw();
            } else {
                TLine lineUnpol(rhoGraph.GetXaxis()->GetXmin(), 1.0 / 3.0,
                                rhoGraph.GetXaxis()->GetXmax(), 1.0 / 3.0);
                lineUnpol.SetLineStyle(7);
                lineUnpol.SetLineColor(kGray + 2);
                lineUnpol.Draw();
            }

            TLegend leg(0.15, 0.80, 0.50, 0.88);
            leg.SetBorderSize(0);
            leg.SetFillStyle(0);
            leg.AddEntry(&rhoGraph, (channelLabel + " data").c_str(), "pl");
            if (mcGraphPtr) {
                leg.AddEntry(mcGraphPtr, (channelLabel + " GEN MC").c_str(), "f");
            }
            leg.Draw();

            c_rho.SetGrid(1, 1);
            latex.DrawLatex(0.2, 0.86, channelLabel.c_str());
            std::string rhoFile = Form("splot_%s_rho00_vs_pt.png", channelTag.c_str());
            c_rho.SaveAs(makeOutputPath(rhoFile).c_str());
            if (mcGraphPtr) {
                delete mcGraphPtr;
            }
        }
    }


// === Combined rho00 vs pT (Prompt + Nonprompt) on a single canvas ===
{
    auto findChannel = [&](ProductionType ch) -> ChannelHistograms* {
        auto it = channelHistMap.find(ch);
        if (it == channelHistMap.end()) return nullptr;
        return &it->second;
    };
    ChannelHistograms* promptBundle    = findChannel(ProductionType::Prompt);
    ChannelHistograms* nonpromptBundle = findChannel(ProductionType::NonPrompt);

    const McGenRho00Result* mcPrompt = nullptr;
    const McGenRho00Result* mcNonPrompt = nullptr;
    auto mcPromptIt = mcGenRhoResults.find(ProductionType::Prompt);
    if (mcPromptIt != mcGenRhoResults.end()) {
        mcPrompt = &mcPromptIt->second;
    }
    auto mcNonIt = mcGenRhoResults.find(ProductionType::NonPrompt);
    if (mcNonIt != mcGenRhoResults.end()) {
        mcNonPrompt = &mcNonIt->second;
    }

    std::vector<double> x, ex, y_prompt, ey_prompt, y_non, ey_non;
    std::vector<double> y_prompt_mc, ey_prompt_mc, y_non_mc, ey_non_mc;
    x.reserve(ptBins.size()); ex.reserve(ptBins.size());
    y_prompt.reserve(ptBins.size()); ey_prompt.reserve(ptBins.size());
    y_non.reserve(ptBins.size());    ey_non.reserve(ptBins.size());
    y_prompt_mc.reserve(ptBins.size()); ey_prompt_mc.reserve(ptBins.size());
    y_non_mc.reserve(ptBins.size());    ey_non_mc.reserve(ptBins.size());

    for (size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
        double xc = ptCenters[ipt];
        double xw = ptHalfWidths[ipt];
        x.push_back(xc);
        ex.push_back(xw);

        if (promptBundle && std::isfinite(promptBundle->rho00[ipt]) && std::isfinite(promptBundle->rho00Err[ipt])) {
            y_prompt.push_back(promptBundle->rho00[ipt]);
            ey_prompt.push_back(promptBundle->rho00Err[ipt]);
        } else {
            y_prompt.push_back(std::numeric_limits<double>::quiet_NaN());
            ey_prompt.push_back(std::numeric_limits<double>::quiet_NaN());
        }

        if (nonpromptBundle && std::isfinite(nonpromptBundle->rho00[ipt]) && std::isfinite(nonpromptBundle->rho00Err[ipt])) {
            y_non.push_back(nonpromptBundle->rho00[ipt]);
            ey_non.push_back(nonpromptBundle->rho00Err[ipt]);
        } else {
            y_non.push_back(std::numeric_limits<double>::quiet_NaN());
            ey_non.push_back(std::numeric_limits<double>::quiet_NaN());
        }

        if (mcPrompt && ipt < mcPrompt->rho00.size()) {
            double val = mcPrompt->rho00[ipt];
            double err = (ipt < mcPrompt->rho00Err.size()) ? mcPrompt->rho00Err[ipt] : std::numeric_limits<double>::quiet_NaN();
            if (std::isfinite(val) && std::isfinite(err)) {
                y_prompt_mc.push_back(val);
                ey_prompt_mc.push_back(err);
            } else {
                y_prompt_mc.push_back(std::numeric_limits<double>::quiet_NaN());
                ey_prompt_mc.push_back(std::numeric_limits<double>::quiet_NaN());
            }
        } else {
            y_prompt_mc.push_back(std::numeric_limits<double>::quiet_NaN());
            ey_prompt_mc.push_back(std::numeric_limits<double>::quiet_NaN());
        }

        if (mcNonPrompt && ipt < mcNonPrompt->rho00.size()) {
            double val = mcNonPrompt->rho00[ipt];
            double err = (ipt < mcNonPrompt->rho00Err.size()) ? mcNonPrompt->rho00Err[ipt] : std::numeric_limits<double>::quiet_NaN();
            if (std::isfinite(val) && std::isfinite(err)) {
                y_non_mc.push_back(val);
                ey_non_mc.push_back(err);
            } else {
                y_non_mc.push_back(std::numeric_limits<double>::quiet_NaN());
                ey_non_mc.push_back(std::numeric_limits<double>::quiet_NaN());
            }
        } else {
            y_non_mc.push_back(std::numeric_limits<double>::quiet_NaN());
            ey_non_mc.push_back(std::numeric_limits<double>::quiet_NaN());
        }
    }

    auto compact = [](std::vector<double>& X, std::vector<double>& EX, std::vector<double>& Y, std::vector<double>& EY) {
        size_t out = 0;
        for (size_t i = 0; i < X.size(); ++i) {
            if (std::isfinite(Y[i]) && std::isfinite(EY[i])) {
                X[out] = X[i]; EX[out] = EX[i]; Y[out] = Y[i]; EY[out] = EY[i];
                ++out;
            }
        }
        X.resize(out); EX.resize(out); Y.resize(out); EY.resize(out);
    };

    std::vector<double> xP = x, exP = ex, yP = y_prompt, eyP = ey_prompt;
    std::vector<double> xN = x, exN = ex, yN = y_non,    eyN = ey_non;
    std::vector<double> xPmc = x, exPmc = ex, yPmc = y_prompt_mc, eyPmc = ey_prompt_mc;
    std::vector<double> xNmc = x, exNmc = ex, yNmc = y_non_mc,    eyNmc = ey_non_mc;
    compact(xP, exP, yP, eyP);
    compact(xN, exN, yN, eyN);
    compact(xPmc, exPmc, yPmc, eyPmc);
    compact(xNmc, exNmc, yNmc, eyNmc);

    if (!xP.empty() || !xN.empty()) {
        TCanvas cComb("c_rho00_prompt_nonprompt", "rho00 vs pT (Prompt & Nonprompt)", 1000, 800);
        cComb.SetLeftMargin(0.12);
        cComb.SetBottomMargin(0.12);
        cComb.SetTopMargin(0.08);
        cComb.SetRightMargin(0.05);

        TGraphErrors grP(static_cast<int>(xP.size()), xP.data(), yP.data(), exP.data(), eyP.data());
        TGraphErrors grN(static_cast<int>(xN.size()), xN.data(), yN.data(), exN.data(), eyN.data());
        TGraphErrors* grPmc = nullptr;
        TGraphErrors* grNmc = nullptr;
        if (!xPmc.empty()) {
            grPmc = new TGraphErrors(static_cast<int>(xPmc.size()), xPmc.data(), yPmc.data(), exPmc.data(), eyPmc.data());
            grPmc->SetMarkerStyle(0);
            grPmc->SetLineColor(kRed + 1);
            grPmc->SetLineWidth(2);
            grPmc->SetFillColorAlpha(kRed + 1, 0.25);
        }
        if (!xNmc.empty()) {
            grNmc = new TGraphErrors(static_cast<int>(xNmc.size()), xNmc.data(), yNmc.data(), exNmc.data(), eyNmc.data());
            grNmc->SetMarkerStyle(0);
            grNmc->SetLineColor(kBlue + 3);
            grNmc->SetLineWidth(2);
            grNmc->SetFillColorAlpha(kBlue + 3, 0.25);
        }

        grP.SetMarkerStyle(20);
        grP.SetMarkerSize(1.2);
        grP.SetMarkerColor(kRed);
        grP.SetLineColor(kRed);
        grP.SetLineWidth(2);

        grN.SetMarkerStyle(21);
        grN.SetMarkerSize(1.2);
        grN.SetMarkerColor(kBlue + 1);
        grN.SetLineColor(kBlue + 1);
        grN.SetLineWidth(2);

        grP.SetTitle("#rho_{00} vs p_{T};p_{T} (GeV/c);#rho_{00}");
        grP.GetXaxis()->SetRangeUser(0, 50);
        grP.GetYaxis()->SetRangeUser(0.1, 0.64);

        grP.GetXaxis()->SetTitleSize(0.05);
        grP.GetXaxis()->SetTitleOffset(1.0);
        grP.GetXaxis()->SetLabelSize(0.04);
        grP.GetXaxis()->SetLabelFont(42);
        grP.GetXaxis()->SetTitleFont(42);

        grP.GetYaxis()->SetTitleSize(0.05);
        grP.GetYaxis()->SetTitleOffset(1.2);
        grP.GetYaxis()->SetLabelSize(0.04);
        grP.GetYaxis()->SetLabelFont(42);
        grP.GetYaxis()->SetTitleFont(42);

        grP.Draw("AP");
        if (grPmc) {
            grPmc->Draw("E SAME");
            // grPmc->Draw("L SAME");
        }
        if (grNmc) {
            grNmc->Draw("E SAME");
            // grNmc->Draw("L SAME");
        }
        grN.Draw("P SAME");
        grP.Draw("P SAME");

        double xmin = grP.GetXaxis()->GetXmin();
        double xmax = grP.GetXaxis()->GetXmax();
        TLine line_unpol(xmin, 1.0/3.0, xmax, 1.0/3.0);
        line_unpol.SetLineStyle(2);
        line_unpol.SetLineWidth(2);
        line_unpol.SetLineColor(kGray+2);
        line_unpol.Draw();

        TLegend leg(0.15, 0.7, 0.5, 0.92);
        leg.SetBorderSize(0);
        leg.SetFillStyle(0);
        leg.SetTextSize(0.04);
        leg.SetTextFont(42);
        leg.AddEntry(&grP, "Prompt D* (data)", "lp");
        leg.AddEntry(&grN, "Non-prompt D* (data)", "lp");
        if (grPmc) leg.AddEntry(grPmc, "Prompt D* (GEN MC)", "f");
        if (grNmc) leg.AddEntry(grNmc, "Non-prompt D* (GEN MC)", "f");
        leg.AddEntry(&line_unpol, "No spin alignment (#rho_{00} = 1/3)", "l");
        leg.Draw();

        cComb.SetGrid(1, 1);
        cComb.SaveAs(makeOutputPath("rho00_vs_pT_prompt_nonprompt.png").c_str());
        cComb.SaveAs(makeOutputPath("rho00_vs_pT_prompt_nonprompt.pdf").c_str());
        if (grPmc) delete grPmc;
        if (grNmc) delete grNmc;
    }
}

    // Cleanup
    auto deleteVector = [](std::vector<TH1D*>& vec) {
        for (TH1D* h : vec) {
            if (h) delete h;
        }
        vec.clear();
    };
    auto deleteVectorTF1 = [](std::vector<TF1*>& vec) {
        for (TF1* f : vec) {
            if (f) delete f;
        }
        vec.clear();
    };

    for (auto& [channel, hists] : channelHistMap) {
        deleteVector(hists.corrected);
        deleteVector(hists.correctedCB);
        deleteVector(hists.weights);
        deleteVector(hists.weightsCB);
        deleteVector(hists.raw);
        deleteVector(hists.matchCorrected);
        deleteVector(hists.rawOverMatch);
        deleteVectorTF1(hists.cosThetaFits);

        for (TH1D*& h : hists.match) {
            if (h) delete h;
            h = nullptr;
        }
        for (TH1D*& h : hists.genCounts) {
            if (h) delete h;
            h = nullptr;
        }
    }

    for (auto& [channel, vec] : genProjectionMap) {
        for (TH1D* h : vec) {
            if (h) delete h;
        }
    }
    for (auto& [channel, vec] : matchProjectionMap) {
        for (TH1D* h : vec) {
            if (h) delete h;
        }
    }
    for (auto& [channel, eff] : efficiencyMaps) {
        delete eff;
    }

    std::cout << "Finished drawing and saving histograms with histogram-based correction." << std::endl;
}
