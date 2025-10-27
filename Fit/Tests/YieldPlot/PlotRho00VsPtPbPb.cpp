#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <limits>

#include "../../Common/Analysis/DStarSingleBinRunner.h"
#include "../../Common/Analysis/DatasetRegistry.h"
#include "../../Common/Analysis/SimpleDatasetManager.h"
#include "../../Common/Analysis/ExecutionProfiles.h"
#include "../../../Interface/ConfigPbPb.h"

#include "RooRealVar.h"
#include "RooWorkspace.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TLine.h"
#include "TH1D.h"
#include "TParameter.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TString.h"
#include "TMultiGraph.h"

using namespace DStarAnalysis;

namespace {

struct AnalysisProfile {
    DStarFitOpt config;
    ParameterConfigOptions paramOpts;
    ExecutionOptions execOpts;
    KinematicBin bin;
};

struct YieldComponents {
    double rawYield = 0.0;
    double rawError = 0.0;
    double promptFraction = 1.0;
    double promptFractionErr = 0.0;
};

std::string JoinPath(const std::string& lhs, const std::string& rhs) {
    if (lhs.empty()) return rhs;
    if (rhs.empty()) return lhs;
    const bool lhsEndsWithSlash = lhs.back() == '/' || lhs.back() == '\\';
    const bool rhsStartsWithSlash = rhs.front() == '/' || rhs.front() == '\\';
    if (lhsEndsWithSlash && rhsStartsWithSlash) {
        return lhs + rhs.substr(1);
    }
    if (!lhsEndsWithSlash && !rhsStartsWithSlash) {
        return lhs + "/" + rhs;
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
    const std::string filePath = __FILE__;
    return ParentDir(filePath);
}

std::string DefaultDatasetConfigPath() {
    const std::string scriptDir = ScriptDirectory();
    const std::string testsDir = ParentDir(scriptDir);
    const std::string fitDir = ParentDir(testsDir);
    return JoinPath(fitDir, "Common/Analysis/datasets.json");
}

std::string DefaultResultsBase() {
    const std::string scriptDir = ScriptDirectory();
    const std::string testsDir = ParentDir(scriptDir);
    const std::string fitDir = ParentDir(testsDir);
    return JoinPath(fitDir, "PbPb");
}

bool EndsWith(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

void ConfigureGlobalDatasetManager() {
    if (gDatasetManager.GetConfigFile().empty()) {
        gDatasetManager.SetConfigFile(DefaultDatasetConfigPath(), false);
    }
    if (!gDatasetManager.IsLoaded()) {
        gDatasetManager.EnsureLoaded();
    }
}

YieldComponents ExtractYield(const FitOpt& fitOpt,
                             const std::string& baseDir,
                             bool isMC,
                             bool applyPromptFraction) {
    YieldComponents result;

    std::string targetDir = JoinPath(baseDir, fitOpt.outputDir);
    targetDir = JoinPath(targetDir, fitOpt.subDir);
    targetDir = JoinPath(targetDir, isMC ? "MC" : "Data");

    std::string fileName = fitOpt.outputFile;
    if (!EndsWith(fileName, ".root")) {
        fileName += ".root";
    }

    const std::string fitFilePath = JoinPath(targetDir, fileName);
    std::unique_ptr<TFile> fitFile(TFile::Open(fitFilePath.c_str(), "READ"));
    if (!fitFile || fitFile->IsZombie()) {
        std::cerr << "[YieldExtractor] Failed to open fit file: " << fitFilePath << std::endl;
        return result;
    }

    RooWorkspace* ws = dynamic_cast<RooWorkspace*>(fitFile->Get("workspace"));
    if (!ws) {
        std::cerr << "[YieldExtractor] Workspace 'workspace' missing in " << fitFilePath << std::endl;
        return result;
    }

    RooRealVar* nsig = ws->var("nsig");
    if (!nsig) {
        std::cerr << "[YieldExtractor] nsig not found in workspace for " << fitFilePath << std::endl;
        return result;
    }

    result.rawYield = nsig->getVal();
    result.rawError = nsig->getError();

    if (!applyPromptFraction || isMC) {
        return result;
    }

    std::string dcaFileName = fitOpt.outputDCAFile;
    if (dcaFileName.empty()) {
        std::cerr << "[YieldExtractor] No DCA file configured for " << fitFilePath
                  << ". Returning raw yields." << std::endl;
        return result;
    }

    if (!EndsWith(dcaFileName, ".root")) {
        dcaFileName += ".root";
    }

    const std::string dcaDir = JoinPath(targetDir, "dcaroot");
    const std::string dcaFilePath = JoinPath(dcaDir, dcaFileName);

    std::unique_ptr<TFile> dcaFile(TFile::Open(dcaFilePath.c_str(), "READ"));
    if (!dcaFile || dcaFile->IsZombie()) {
        std::cerr << "[YieldExtractor] Failed to open DCA file: " << dcaFilePath
                  << ". Returning raw yields." << std::endl;
        return result;
    }

    auto* promptFrac = dynamic_cast<TParameter<double>*>(dcaFile->Get("PromptFraction"));
    auto* promptFracErr = dynamic_cast<TParameter<double>*>(dcaFile->Get("PromptFractionError"));
    if (!promptFrac || !promptFracErr) {
        std::cerr << "[YieldExtractor] Prompt fraction objects missing in " << dcaFilePath
                  << ". Returning raw yields." << std::endl;
        return result;
    }

    result.promptFraction = promptFrac->GetVal();
    result.promptFractionErr = promptFracErr->GetVal();
    return result;
}

struct FitSummary {
    double rho00 = std::numeric_limits<double>::quiet_NaN();
    double rho00Err = std::numeric_limits<double>::quiet_NaN();
    double norm = 0.0;
    double normErr = 0.0;
    double chi2 = std::numeric_limits<double>::quiet_NaN();
    double ndf = 0.0;
};

FitSummary FitCosThetaHistogram(TH1D* hist, TF1& func) {
    FitSummary summary;
    if (!hist) {
        return summary;
    }

    const double integral = hist->Integral("width");
    func.SetParameter(0, integral > 0.0 ? integral : 1.0);
    func.SetParLimits(0, 0.0, std::max(1.0, integral * 10.0));
    func.SetParameter(1, 1.0 / 3.0);
    func.SetParLimits(1, 0.0, 1.0);

    TFitResultPtr fitResult = hist->Fit(&func, "QS");
    if (!fitResult.Get() || fitResult->Status() != 0) {
        std::cerr << "[Fit] Fit failed with status " << (fitResult.Get() ? fitResult->Status() : -1) << std::endl;
        return summary;
    }

    summary.norm = func.GetParameter(0);
    summary.normErr = func.GetParError(0);
    summary.rho00 = func.GetParameter(1);
    summary.rho00Err = func.GetParError(1);
    summary.chi2 = fitResult->Chi2();
    summary.ndf = fitResult->Ndf();
    return summary;
}

AnalysisProfile BuildPbPbAnalysisProfile(bool doReFit, bool doDCA, bool plotFit, bool useCUDA,
                                         float pTMin, float pTMax, float cosMin, float cosMax,
                                         int centralityMin, int centralityMax,
                                         const std::string& parameterFile,
                                         bool isMC,
                                         bool usePrecomputedDcaYield = false,
                                         const std::string& dcaYieldHistFileOverride = std::string(),
                                         const std::string& dcaYieldHistName = "dataYieldHist") {
    AnalysisProfile profile;

    profile.config.SetYieldModeAutoTuning(false);
    profile.config.SetUseIndependentYields(true);

    ConfigureGlobalDatasetManager();
    DatasetConfig dataset;
    if (gDatasetManager.IsLoaded()) {
        auto dataInfo = gDatasetManager.GetDataset("PbPb_Data_EPtransformation");
        auto mcInfo = gDatasetManager.GetDataset("PbPb_MC");
        if (!dataInfo.file.empty() && !mcInfo.file.empty()) {
            dataset.dataFile = dataInfo.file;
            dataset.mcFile = mcInfo.file;
            dataset.datasetName = dataInfo.dataset_name;
            profile.config.SetDatasetName(dataInfo.dataset_name);
            profile.config.SetMCdatasetName(mcInfo.dataset_name);
        }
    }

    if (dataset.dataFile.empty() || dataset.mcFile.empty()) {
        dataset = DatasetRegistry::GetPbPbDataset();
        profile.config.SetDatasetName(dataset.datasetName);
        profile.config.SetMCdatasetName(dataset.datasetName);
    }

    ConfigureFiles(profile.config, dataset);

    profile.config.SetOutputSubDir(SelectionCuts::SubDir());
    ConfigureBaseFitOptions(profile.config, useCUDA, doReFit);
    profile.config.SetFitMethod(FitMethod::GaussianConstraint);
    profile.config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    profile.config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
    profile.config.SetMVACut(0.999);
    profile.config.SetUseAbsCosCuts(false);

    profile.bin = KinematicBin(pTMin, pTMax, cosMin, cosMax, centralityMin, centralityMax);
    profile.config.ClearCentralityBins();
    profile.config.AddPtBin(pTMin, pTMax);
    profile.config.AddCosBin(cosMin, cosMax);
    profile.config.AddCentralityBin(centralityMin, centralityMax);

    profile.paramOpts.parameterFile = parameterFile;
    profile.paramOpts.ignoreCentralityInMatching = false;
    profile.paramOpts.applyFallbackParameters = ApplyDefaultDoubleGaussianParameters;

    profile.execOpts = MakePbPbExecutionOptions(doReFit, doDCA, plotFit,
                                                usePrecomputedDcaYield, isMC,
                                                dcaYieldHistFileOverride, dcaYieldHistName,
                                                FitMode::GaussianConstraintWithSideband);
    profile.execOpts.enableGaussianConstraint = true;
    profile.execOpts.enableGaussianConstraintSideband = true;
    profile.execOpts.enableSidebandPrefitOnly = false;
    profile.execOpts.sidebandLowMin = 0.140;
    profile.execOpts.sidebandLowMax = 0.143;
    profile.execOpts.sidebandHighMin = 0.149;
    profile.execOpts.sidebandHighMax = 0.155;
    profile.execOpts.sidebandSigmaScale = 1.0;

    return profile;
}

struct RhoPoint {
    double ptCenter = 0.0;
    double ptHalfWidth = 0.0;
    double rho00 = std::numeric_limits<double>::quiet_NaN();
    double rho00Err = std::numeric_limits<double>::quiet_NaN();
    double chi2 = std::numeric_limits<double>::quiet_NaN();
    double ndf = 0.0;
};

} // namespace

void PlotRho00VsPtPbPb(bool isMC = false,
                       bool applyPromptFraction = true,
                       const std::string& baseResultsDir = DefaultResultsBase(),
                       const std::string& parameterFile = "") {
    gStyle->SetOptStat(0);

    const std::vector<std::pair<double, double>> ptBins = {
        {5.0, 7.0},
        {7.0, 10.0},
        {10.0, 20.0},
        {20.0, 50.0}
    };

    const std::vector<std::pair<double, double>> cosBins = {
        {-1.0, -0.8},
        {-0.8, -0.6},
        {-0.6, -0.4},
        {-0.4, -0.2},
        {-0.2, 0.0},
        {0.0, 0.2},
        {0.2, 0.4},
        {0.4, 0.6},
        {0.6, 0.8},
        {0.8, 1.0}
    };

    const std::vector<std::pair<int, int>> centralityBins = {
        {0, 10},
        {30, 50}
    };

    std::vector<double> cosEdges;
    cosEdges.push_back(cosBins.front().first);
    for (const auto& cosBin : cosBins) {
        cosEdges.push_back(cosBin.second);
    }
    // cosEdges.push_back(cosBins.back().second);

    const std::string scriptDir = ScriptDirectory();
    const std::string outputDir = JoinPath(scriptDir, "output");
    ConfigureGlobalDatasetManager();
    gSystem->mkdir(outputDir.c_str(), true);

    struct CentralityGraph {
        std::string label;
        Color_t color;
        std::vector<RhoPoint> points;
    };

    std::vector<CentralityGraph> graphData = {
        {"Centrality 0-10%", kRed},
        {"Centrality 30-50%", static_cast<Color_t>(kBlue + 1)}
    };

    for (size_t centIdx = 0; centIdx < centralityBins.size(); ++centIdx) {
        const auto& cent = centralityBins[centIdx];
        auto& centGraph = graphData[centIdx];
        centGraph.points.clear();
        std::cout << "\n=== Processing centrality bin " << cent.first << "-" << cent.second << "% ===\n";

        for (const auto& pt : ptBins) {
            auto hist = std::make_unique<TH1D>(
                Form("hRho_cent_%zu_pt_%0.0f_%0.0f", centIdx, pt.first, pt.second),
                Form("Yield distribution;cos#vartheta*;Yield (%.0f-%.0f GeV/c, %d-%d%%)",
                     pt.first, pt.second, cent.first, cent.second),
                static_cast<int>(cosEdges.size() - 1),
                cosEdges.data());
            hist->Sumw2(true);
            TF1 fitFunc("fitFunc", "[0]*(1 - [1] + (3*[1] - 1)*x*x)",
                        -1.0, 1.0);
            fitFunc.SetParName(0, "N_{0}");
            fitFunc.SetParName(1, "#rho_{00}");

            bool hasContent = false;
            for (size_t cosIdx = 0; cosIdx < cosBins.size(); ++cosIdx) {
                const auto& cosBin = cosBins[cosIdx];
                auto profile = BuildPbPbAnalysisProfile(
                    false,  // doReFit
                    true,   // doDCA
                    true,   // plotFit
                    true,   // useCUDA
                    pt.first, pt.second, cosBin.first, cosBin.second,
                    cent.first, cent.second,
                    parameterFile,
                    isMC);

                auto fitOpt = profile.config.CreateFitOpt(profile.bin);
                YieldComponents comp = ExtractYield(fitOpt, baseResultsDir, isMC, applyPromptFraction);
                double yieldVal = comp.rawYield;
                double yieldErr = comp.rawError;

                if (applyPromptFraction && !isMC && comp.promptFraction > 0.0) {
                    const double fraction = comp.promptFraction;
                    const double fracErr = comp.promptFractionErr;
                    const double scaledYield = yieldVal * fraction;
                    const double scaledError = std::sqrt(std::pow(yieldErr * fraction, 2) +
                                                         std::pow(yieldVal * fracErr, 2));
                    yieldVal = scaledYield;
                    yieldErr = scaledError;
                }

                const int binIndex = static_cast<int>(cosIdx) + 1;
                hist->SetBinContent(binIndex, yieldVal);
                hist->SetBinError(binIndex, yieldErr);
                if (yieldVal > 0.0) {
                    hasContent = true;
                }

                std::cout << Form("  pT %.0f-%.0f, |cos| %.1f-%.1f, Cent %d-%d%% : yield %.2f ± %.2f",
                                  pt.first, pt.second, cosBin.first, cosBin.second,
                                  cent.first, cent.second, yieldVal, yieldErr)
                          << std::endl;
            }

            RhoPoint rhoPoint;
            rhoPoint.ptCenter = 0.5 * (pt.first + pt.second);
            rhoPoint.ptHalfWidth = 0.5 * (pt.second - pt.first);

            if (hasContent) {
                auto summary = FitCosThetaHistogram(hist.get(), fitFunc);
                if (std::isfinite(summary.rho00)) {
                    rhoPoint.rho00 = summary.rho00;
                    rhoPoint.rho00Err = summary.rho00Err;
                    rhoPoint.chi2 = summary.chi2;
                    rhoPoint.ndf = summary.ndf;

                    std::cout << Form("    -> rho00 = %.4f ± %.4f (chi2/ndf = %.2f/%g)",
                                      summary.rho00, summary.rho00Err,
                                      summary.chi2, summary.ndf)
                              << std::endl;

                    auto canvas = std::make_unique<TCanvas>(
                        Form("c_fit_cent_%zu_pt_%0.0f_%0.0f", centIdx, pt.first, pt.second),
                        Form("Fit |cos#vartheta*| %.0f-%.0f GeV/c, %d-%d%%",
                             pt.first, pt.second, cent.first, cent.second),
                        700, 500);
                    hist->SetMarkerStyle(20);
                    hist->SetMarkerColor(centGraph.color);
                    hist->SetMinimum(0.0);
                    hist->Draw("E1");
                    fitFunc.SetLineColor(centGraph.color);
                    fitFunc.Draw("SAME");
                    TLegend leg(0.55, 0.75, 0.88, 0.88);
                    leg.SetBorderSize(0);
                    leg.SetFillStyle(0);
                    leg.AddEntry(hist.get(), "Yield", "pe");
                    leg.AddEntry(&fitFunc, "Fit", "l");
                    leg.AddEntry((TObject*)nullptr, Form("#rho_{00} = %.3f #pm %.3f", summary.rho00, summary.rho00Err), "");
                    leg.Draw();
                    const std::string fitName = Form("rho_fit_cent_%d_%d_pt_%.0f_%.0f.png",
                                                     cent.first, cent.second, pt.first, pt.second);
                    canvas->SaveAs(JoinPath(outputDir, fitName).c_str());
                    const std::string fitPdf = Form("rho_fit_cent_%d_%d_pt_%.0f_%.0f.pdf",
                                                    cent.first, cent.second, pt.first, pt.second);
                    canvas->SaveAs(JoinPath(outputDir, fitPdf).c_str());
                } else {
                    std::cerr << "[Fit] Invalid rho00 for pT " << pt.first << "-" << pt.second
                              << " GeV/c, cent " << cent.first << "-" << cent.second << "%\n";
                }
            } else {
                std::cerr << "[Fit] Histogram empty for pT " << pt.first << "-" << pt.second
                          << " GeV/c, cent " << cent.first << "-" << cent.second << "%\n";
            }

            centGraph.points.push_back(rhoPoint);
        }
    }

    auto makeGraph = [](const CentralityGraph& data) -> std::unique_ptr<TGraphErrors> {
        std::vector<double> x, y, ex, ey;
        for (const auto& pt : data.points) {
            if (!std::isfinite(pt.rho00) || !std::isfinite(pt.rho00Err)) {
                continue;
            }
            x.push_back(pt.ptCenter);
            ex.push_back(pt.ptHalfWidth);
            y.push_back(pt.rho00);
            ey.push_back(pt.rho00Err);
        }
        if (x.empty()) {
            return nullptr;
        }
        auto graph = std::make_unique<TGraphErrors>(static_cast<int>(x.size()),
                                                    x.data(), y.data(), ex.data(), ey.data());
        graph->SetName(Form("gr_rho00_%s", data.label.c_str()));
        graph->SetTitle(";p_{T} (GeV/c);#rho_{00}");
        graph->SetMarkerStyle(20);
        graph->SetMarkerSize(1.1);
        graph->SetMarkerColor(data.color);
        graph->SetLineColor(data.color);
        graph->SetLineWidth(2);
        return graph;
    };

    auto graph0 = makeGraph(graphData[0]);
    auto graph1 = makeGraph(graphData[1]);

    if (!graph0 && !graph1) {
        std::cerr << "[Plot] No valid rho00 points to plot.\n";
        return;
    }

    TCanvas canvas("c_rho00_vs_pt", "rho00 vs pT", 800, 600);
    canvas.SetLeftMargin(0.12);
    canvas.SetBottomMargin(0.13);
    canvas.SetTopMargin(0.08);
    canvas.SetRightMargin(0.05);

    TMultiGraph mg;
    if (graph0) mg.Add(graph0.get(), "PE");
    if (graph1) mg.Add(graph1.get(), "PE");
    mg.SetTitle("#rho_{00} vs p_{T};p_{T} (GeV/c);#rho_{00}");
    mg.Draw("A");
    mg.GetYaxis()->SetRangeUser(0.0, 0.8);

    TLine lineUnpol(mg.GetXaxis()->GetXmin(), 1.0 / 3.0,
                    mg.GetXaxis()->GetXmax(), 1.0 / 3.0);
    lineUnpol.SetLineStyle(7);
    lineUnpol.SetLineColor(kGray + 2);
    lineUnpol.Draw();

    TLegend legend(0.58, 0.72, 0.88, 0.88);
    legend.SetBorderSize(0);
    legend.SetFillStyle(0);
    if (graph0) legend.AddEntry(graph0.get(), graphData[0].label.c_str(), "pe");
    if (graph1) legend.AddEntry(graph1.get(), graphData[1].label.c_str(), "pe");
    legend.AddEntry(&lineUnpol, "#rho_{00} = 1/3", "l");
    legend.Draw();

    const std::string outPng = JoinPath(outputDir, "rho00_vs_pt_cent_0_10_30_50.png");
    const std::string outPdf = JoinPath(outputDir, "rho00_vs_pt_cent_0_10_30_50.pdf");
    canvas.SaveAs(outPng.c_str());
    canvas.SaveAs(outPdf.c_str());

    std::cout << "\nSaved rho00 vs pT plot to:\n - " << outPng << "\n - " << outPdf << std::endl;
}

