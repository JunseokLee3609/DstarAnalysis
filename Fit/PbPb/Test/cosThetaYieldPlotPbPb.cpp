#include <cstdlib>
#include "../Common/Analysis/DStarSingleBinRunner.h"
#include "../Common/Analysis/DatasetRegistry.h"
#include "../Common/Analysis/ExecutionProfiles.h"
#include "../../Interface/ConfigPbPb.h"

#include "RooRealVar.h"
#include "RooWorkspace.h"
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TParameter.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TString.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace DStarAnalysis;

namespace {

struct AnalysisProfile {
    DStarFitOpt config;
    ParameterConfigOptions paramOpts;
    ExecutionOptions execOpts;
    KinematicBin bin;
};

std::string JoinPath(const std::string &lhs, const std::string &rhs) {
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

bool EndsWith(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

struct YieldComponents {
    double rawYield = 0.0;
    double rawError = 0.0;
    double promptFraction = 1.0;
    double promptFractionErr = 0.0;
};

YieldComponents ExtractYield(const FitOpt &fitOpt,
                             const std::string &baseDir,
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

    RooWorkspace *ws = dynamic_cast<RooWorkspace *>(fitFile->Get("ws_DStar"));
    if (!ws) {
        std::cerr << "[YieldExtractor] Workspace ws_DStar missing in " << fitFilePath << std::endl;
        return result;
    }

    RooRealVar *nsig = ws->var("nsig");
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

    auto *promptFrac = dynamic_cast<TParameter<double> *>(dcaFile->Get("PromptFraction"));
    auto *promptFracErr = dynamic_cast<TParameter<double> *>(dcaFile->Get("PromptFractionError"));
    if (!promptFrac || !promptFracErr) {
        std::cerr << "[YieldExtractor] Prompt fraction objects missing in " << dcaFilePath
                  << ". Returning raw yields." << std::endl;
        return result;
    }

    result.promptFraction = promptFrac->GetVal();
    result.promptFractionErr = promptFracErr->GetVal();
    return result;
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

    // 간단한 데이터셋 설정 (테스트용)
    DatasetConfig dataset;
    dataset.dataFile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS/PbPb/Data/DStar.root";
    dataset.mcFile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS/PbPb/MC/DStar.root";
    dataset.datasetName = "PbPb_Data_EPtransformation";
    
    profile.config.SetDatasetName("PbPb_Data_EPtransformation");
    profile.config.SetMCdatasetName("PbPb_MC");
    
    ErrorHandlerManager::Instance().LogInfo("Using hardcoded dataset configuration:", "cosThetaYieldPlotPbPb");
    ErrorHandlerManager::Instance().LogInfo("  Data: " + dataset.dataFile, "cosThetaYieldPlotPbPb");
    ErrorHandlerManager::Instance().LogInfo("  MC: " + dataset.mcFile, "cosThetaYieldPlotPbPb");
    ErrorHandlerManager::Instance().LogInfo("  Dataset Name: " + dataset.datasetName, "cosThetaYieldPlotPbPb");
    
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

void StyleHistogram(TH1D *hist) {
    if (!hist) return;
    hist->Sumw2(true);
    hist->SetMarkerStyle(20);
    hist->SetMarkerSize(1.1);
    hist->SetMarkerColor(kBlue + 1);
    hist->SetLineColor(kBlue + 1);
    hist->SetLineWidth(2);
    hist->GetXaxis()->SetTitle("|cos#vartheta*|");
    hist->GetYaxis()->SetTitle("Yield");
    hist->GetXaxis()->SetTitleSize(0.045);
    hist->GetXaxis()->SetLabelSize(0.04);
    hist->GetYaxis()->SetTitleSize(0.045);
    hist->GetYaxis()->SetLabelSize(0.04);
}

} // namespace

void cosThetaYieldPlotPbPb(bool isMC = false,
                           bool applyPromptFraction = true,
                           const std::string &baseFitDir = "/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/results",
                           const std::string &parameterFile = "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Parameters/PbPb/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_Oct21_v1.json") {
    gStyle->SetOptStat(0);

    const std::vector<std::pair<double, double>> ptBins = {
        {5.0, 7.0},
        {7.0, 10.0},
        {10.0, 20.0},
        {20.0, 50.0}
    };

    const std::vector<std::pair<double, double>> cosBins = {
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

    const std::vector<double> cosEdges = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};

    std::vector<TH1D *> yieldHists;
    std::vector<TCanvas *> canvases;

    const std::string fullBaseDir = JoinPath(baseFitDir, "");

    for (size_t centIdx = 0; centIdx < centralityBins.size(); ++centIdx) {
        const auto &cent = centralityBins[centIdx];
        
        for (size_t ptIdx = 0; ptIdx < ptBins.size(); ++ptIdx) {
            const auto &pt = ptBins[ptIdx];
            
            auto *hist = new TH1D(
                Form("hYield_pt_%zu_cent_%zu", ptIdx, centIdx),
                Form("Yield vs |cos#vartheta*|;|cos#vartheta*|;Yield, p_{T} %.0f-%.0f GeV/c, Cent %d-%d%%", 
                     pt.first, pt.second, cent.first, cent.second),
                static_cast<int>(cosEdges.size() - 1),
                cosEdges.data());
            StyleHistogram(hist);
            yieldHists.push_back(hist);

            auto *canvas = new TCanvas(Form("cYield_pt_%zu_cent_%zu", ptIdx, centIdx),
                                       Form("Yield distribution, p_{T} %.0f-%.0f GeV/c, Cent %d-%d%%", 
                                            pt.first, pt.second, cent.first, cent.second),
                                       800, 600);
            canvas->SetLeftMargin(0.14);
            canvas->SetBottomMargin(0.13);
            canvas->SetRightMargin(0.05);
            canvas->SetTopMargin(0.08);
            canvases.push_back(canvas);

            for (size_t cosIdx = 0; cosIdx < cosBins.size(); ++cosIdx) {
                const auto &cosBin = cosBins[cosIdx];
                
                // Build PbPb analysis profile
                auto profile = BuildPbPbAnalysisProfile(
                    false, // doReFit
                    true,  // doDCA
                    true,  // plotFit
                    true,  // useCUDA
                    pt.first, pt.second, cosBin.first, cosBin.second,
                    cent.first, cent.second,
                    parameterFile,
                    isMC
                );

                auto fitOpt = profile.config.CreateFitOpt(profile.bin);

                YieldComponents comp = ExtractYield(fitOpt, fullBaseDir, isMC, applyPromptFraction);
                double yieldVal = comp.rawYield;
                double yieldErr = comp.rawError;

                if (applyPromptFraction && !isMC && comp.promptFraction > 0.0) {
                    const double fraction = comp.promptFraction;
                    const double fracErr = comp.promptFractionErr;
                    yieldErr = std::sqrt(std::pow(yieldErr * fraction, 2) +
                                         std::pow(yieldVal * fracErr, 2));
                    yieldVal *= fraction;

                    std::cout << Form("pT %.0f-%.0f, |cos| %.1f-%.1f, Cent %d-%d%% : nsig %.2f ± %.2f, f_prompt %.3f ± %.3f, yield %.2f ± %.2f",
                                      pt.first, pt.second, cosBin.first, cosBin.second, cent.first, cent.second,
                                      comp.rawYield, comp.rawError, fraction, fracErr,
                                      yieldVal, yieldErr) << std::endl;
                } else {
                    std::cout << Form("pT %.0f-%.0f, |cos| %.1f-%.1f, Cent %d-%d%% : yield %.2f ± %.2f",
                                      pt.first, pt.second, cosBin.first, cosBin.second, cent.first, cent.second,
                                      yieldVal, yieldErr) << std::endl;
                }

                const int binIndex = static_cast<int>(cosIdx) + 1;
                hist->SetBinContent(binIndex, yieldVal);
                hist->SetBinError(binIndex, yieldErr);
            }

            canvases.back()->cd();
            hist->Draw("E1P");
            TLegend *leg = new TLegend(0.55, 0.70, 0.88, 0.88);
            leg->SetBorderSize(0);
            leg->SetFillStyle(0);
            leg->SetTextSize(0.035);
            leg->AddEntry(hist, isMC ? "MC yield" : (applyPromptFraction ? "Prompt yield" : "Raw nsig"), "lep");
            leg->Draw();

            const char *suffix = isMC ? "_MC" : "";
            const char *centSuffix = Form("_cent_%d_%d", cent.first, cent.second);
            canvases.back()->SaveAs(Form("yield_vs_costheta_pt_%.0f_%.0f%s%s.png", 
                                        pt.first, pt.second, centSuffix, suffix));
            canvases.back()->SaveAs(Form("yield_vs_costheta_pt_%.0f_%.0f%s%s.pdf", 
                                        pt.first, pt.second, centSuffix, suffix));
        }
    }
}

#ifdef __CLING__
void cosThetaYieldPlotPbPb_macro(bool isMC = false,
                                 bool applyPromptFraction = true,
                                 const char* baseFitDir = "/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/results",
                                 const char* parameterFile = "/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Parameters/PbPb/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_Oct21_v1.json") {
    const std::string baseFitDirStr = baseFitDir ? std::string(baseFitDir) : std::string("/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/results");
    const std::string paramFileStr = parameterFile ? std::string(parameterFile) : std::string("/home/jun502s/DstarAna/DStarAnalysis/Fit/Common/Parameters/PbPb/dstar_parameters_DBCrystalBall_Phenomenological2_dca_pbpb_Oct21_v1.json");

    cosThetaYieldPlotPbPb(isMC, applyPromptFraction, baseFitDirStr, paramFileStr);
}

void cosThetaYieldPlotPbPb_macro() {
    cosThetaYieldPlotPbPb();
}
#endif
