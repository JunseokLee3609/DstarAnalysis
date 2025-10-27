#include "../../fit/DStarFitConfig.h"
#include "../../Tools/ConfigManagerPP.h"

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

namespace {

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

void ConfigureForPP(DStarFitOpt &config) {
    config.SetYieldModeAutoTuning(false);
    config.SetUseIndependentYields(true);
    config.SetOutputSubDir(SelectionCuts::SUB_DIR);
    config.SetFitMethod(FitMethod::GaussianConstraint);
    config.SetUseCUDA(true);
    config.SetVerbose(false);
    config.SetDoRefit(false);
    config.SetUseAbsCosCuts(true);
    config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
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

void cosThetaYieldPlot(bool isMC = false,
                       bool applyPromptFraction = true,
                       const std::string &baseFitDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun") {
    gStyle->SetOptStat(0);

    DStarFitOpt config;
    ConfigureForPP(config);

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

    const std::vector<double> cosEdges = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};

    std::vector<TH1D *> yieldHists;
    yieldHists.reserve(ptBins.size());

    std::vector<TCanvas *> canvases;
    canvases.reserve(ptBins.size());

    const std::string fullBaseDir = JoinPath(baseFitDir, "");

    for (size_t ptIdx = 0; ptIdx < ptBins.size(); ++ptIdx) {
        const auto &pt = ptBins[ptIdx];
        auto *hist = new TH1D(
            Form("hYield_pt_%zu", ptIdx),
            Form("Yield vs |cos#vartheta*|;|cos#vartheta*|;Yield, p_{T} %.0f-%.0f GeV/c", pt.first, pt.second),
            static_cast<int>(cosEdges.size() - 1),
            cosEdges.data());
        StyleHistogram(hist);
        yieldHists.push_back(hist);

        auto *canvas = new TCanvas(Form("cYield_pt_%zu", ptIdx),
                                   Form("Yield distribution, p_{T} %.0f-%.0f GeV/c", pt.first, pt.second),
                                   800, 600);
        canvas->SetLeftMargin(0.14);
        canvas->SetBottomMargin(0.13);
        canvas->SetRightMargin(0.05);
        canvas->SetTopMargin(0.08);
        canvases.push_back(canvas);

        for (size_t cosIdx = 0; cosIdx < cosBins.size(); ++cosIdx) {
            const auto &cosBin = cosBins[cosIdx];
            KinematicBin kinBin(pt.first, pt.second, cosBin.first, cosBin.second);
            auto fitOpt = config.CreateFitOpt(kinBin);

            YieldComponents comp = ExtractYield(fitOpt, fullBaseDir, isMC, applyPromptFraction);
            double yieldVal = comp.rawYield;
            double yieldErr = comp.rawError;

            if (applyPromptFraction && !isMC && comp.promptFraction > 0.0) {
                const double fraction = comp.promptFraction;
                const double fracErr = comp.promptFractionErr;
                yieldErr = std::sqrt(std::pow(yieldErr * fraction, 2) +
                                     std::pow(yieldVal * fracErr, 2));
                yieldVal *= fraction;

                std::cout << Form("pT %.0f-%.0f, |cos| %.1f-%.1f : nsig %.2f ± %.2f, f_prompt %.3f ± %.3f, yield %.2f ± %.2f",
                                  pt.first, pt.second, cosBin.first, cosBin.second,
                                  comp.rawYield, comp.rawError, fraction, fracErr,
                                  yieldVal, yieldErr) << std::endl;
            } else {
                std::cout << Form("pT %.0f-%.0f, |cos| %.1f-%.1f : yield %.2f ± %.2f",
                                  pt.first, pt.second, cosBin.first, cosBin.second,
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
        canvases.back()->SaveAs(Form("yield_vs_costheta_pt_%.0f_%.0f%s.png", pt.first, pt.second, suffix));
        canvases.back()->SaveAs(Form("yield_vs_costheta_pt_%.0f_%.0f%s.pdf", pt.first, pt.second, suffix));
    }
}
