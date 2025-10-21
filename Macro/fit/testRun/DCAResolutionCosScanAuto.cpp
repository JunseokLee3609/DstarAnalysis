#include "../DStarFitOpt.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TGaxis.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TString.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

// Forward declaration provided by DCAResolutionScan.cpp
void DCAResolutionScan(double ptMin, double ptMax,
                       double cosMin, double cosMax,
                       int centralityMin = 0, int centralityMax = 10,
                       double scaleMin = 0.7,
                       double scaleMax = 1.4,
                       double scaleStep = 0.01,
                       const std::vector<double>* customScales = nullptr,
                       bool makePerScalePlots = true,
                       std::vector<double>* outScales = nullptr,
                       std::vector<double>* outPromptFractions = nullptr,
                       std::vector<double>* outPromptFractionErrors = nullptr,
                       std::vector<double>* outChi2Values = nullptr,
                       std::vector<double>* outNdfValues = nullptr);

namespace {
std::vector<std::pair<double, double>> BuildCosBins(bool useAbsCos) {
    if (useAbsCos) {
        return {{0.0, 0.2}, {0.2, 0.4}, {0.4, 0.6}, {0.6, 0.8}, {0.8, 1.0}};
    }
    return {{-1.0, -0.6}, {-0.6, -0.2}, {-0.2, 0.2}, {0.2, 0.6}, {0.6, 1.0}};
}

std::vector<double> BuildScanScales(double min, double max, double step) {
    std::vector<double> scales;
    if (max <= min || step <= 0.0) {
        scales.push_back(min);
        return scales;
    }
    for (double val = min; val <= max + 1e-9; val += step) {
        double rounded = std::round(val * 1000.0) / 1000.0;
        if (!scales.empty() && std::abs(rounded - scales.back()) < 1e-6) {
            continue;
        }
        scales.push_back(rounded);
    }
    return scales;
}

int FindBestScaleIndex(const std::vector<double>& chi2,
                       const std::vector<double>& ndf) {
    double best = std::numeric_limits<double>::infinity();
    int bestIdx = -1;
    for (size_t i = 0; i < chi2.size(); ++i) {
        double dof = (i < ndf.size() && ndf[i] > 0) ? ndf[i] : 1.0;
        double value = chi2[i] / dof;
        if (!std::isfinite(value)) {
            continue;
        }
        if (value < best) {
            best = value;
            bestIdx = static_cast<int>(i);
        }
    }
    return bestIdx;
}
} // namespace

void DCAResolutionPromptFractionVsCosAuto(double ptMin, double ptMax,
                                          int centralityMin = 0,
                                          int centralityMax = 100,
                                          double scaleMin = 0.70,
                                          double scaleMax = 1.40,
                                          double scaleStep = 0.01,
                                          bool useAbsCos = true) {
    gStyle->SetOptStat(0);

    const auto cosBins = BuildCosBins(useAbsCos);
    const auto scanScales = BuildScanScales(scaleMin, scaleMax, scaleStep);

    DStarFitOpt config;
    config.SetYieldModeAutoTuning(false);
    config.SetUseIndependentYields(true);
    config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_pp_12Sep25_v1.root");
    config.SetMCFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_ppPU_np_16Sep25_v1.root");
    config.SetDatasetName("datasetHX");
    config.SetOutputSubDir(SelectionCuts::SUB_DIR);
    config.SetFitMethod(FitMethod::GaussianConstraintWithSB);
    config.SetUseCUDA(true);
    config.SetVerbose(true);

    config.AddPtBin(ptMin, ptMax);
    config.AddCosBin(cosBins.front().first, cosBins.front().second);

    KinematicBin firstBin(ptMin, ptMax,
                          cosBins.front().first, cosBins.front().second,
                          centralityMin, centralityMax);

    FitOpt fitOpt = config.CreateFitOpt(firstBin);
    fitOpt.GenerateLegends();

    std::string baseName = firstBin.GetBinName();
    std::string summaryDir = fitOpt.outputDir + fitOpt.subDir
        + "/Data/dca_resolution_scan/" + baseName + "_cosSummary";
    gSystem->mkdir(summaryDir.c_str(), true);

    std::vector<double> cosCenters;
    std::vector<double> cosHalfWidths;
    std::vector<double> bestPromptFractions;
    std::vector<double> bestPromptErrors;
    std::vector<double> bestScales;
    std::vector<double> bestChi2Ndf;

    for (const auto& bin : cosBins) {
        double cMin = bin.first;
        double cMax = bin.second;

        std::vector<double> scalesOut;
        std::vector<double> fractionsOut;
        std::vector<double> errorsOut;
        std::vector<double> chi2Out;
        std::vector<double> ndfOut;

        DCAResolutionScan(ptMin, ptMax,
                          cMin, cMax,
                          centralityMin, centralityMax,
                          scaleMin, scaleMax, scaleStep,
                          &scanScales, false,
                          &scalesOut, &fractionsOut, &errorsOut,
                          &chi2Out, &ndfOut);

        int bestIdx = FindBestScaleIndex(chi2Out, ndfOut);
        if (bestIdx < 0 || bestIdx >= static_cast<int>(fractionsOut.size())) {
            std::cerr << "[CosScanAuto] No valid best scale for cos bin ["
                      << cMin << ", " << cMax << "]" << std::endl;
            continue;
        }

        double dof = (bestIdx < static_cast<int>(ndfOut.size()) && ndfOut[bestIdx] > 0)
                         ? ndfOut[bestIdx] : 1.0;
        double chi2ndf = chi2Out[bestIdx] / dof;

        cosCenters.push_back(0.5 * (cMin + cMax));
        cosHalfWidths.push_back(0.5 * std::abs(cMax - cMin));
        bestPromptFractions.push_back(fractionsOut[bestIdx]);
        bestPromptErrors.push_back(errorsOut.size() > static_cast<size_t>(bestIdx)
                                        ? errorsOut[bestIdx] : 0.0);
        bestScales.push_back(scalesOut[bestIdx]);
        bestChi2Ndf.push_back(chi2ndf);

        std::cout << Form("[CosScanAuto] pT[%0.1f,%0.1f] cos[%0.2f,%0.2f] "
                          "scale %.3f, prompt %.2f +/- %.2f, chi2/ndf %.2f",
                          ptMin, ptMax, cMin, cMax,
                          scalesOut[bestIdx],
                          fractionsOut[bestIdx],
                          errorsOut[bestIdx],
                          chi2ndf)
                  << std::endl;
    }

    if (cosCenters.empty()) {
        std::cerr << "[CosScanAuto] No successful cos bins scanned." << std::endl;
        return;
    }

    double fracMax = 0.0;
    for (size_t i = 0; i < bestPromptFractions.size(); ++i) {
        double candidate = bestPromptFractions[i]
            + (i < bestPromptErrors.size() ? bestPromptErrors[i] : 0.0);
        if (candidate > fracMax) {
            fracMax = candidate;
        }
    }
    const double yMin = 0.0;
    const double yMax = std::min(110.0, std::max(50.0, fracMax + 10.0));

    auto promptGraph = std::make_unique<TGraphErrors>();
    auto scaleGraph = std::make_unique<TGraph>();

    for (size_t i = 0; i < cosCenters.size(); ++i) {
        int idx = promptGraph->GetN();
        double x = cosCenters[i];
        double y = bestPromptFractions[i];
        double xErr = (i < cosHalfWidths.size()) ? cosHalfWidths[i] : 0.0;
        double yErr = (i < bestPromptErrors.size()) ? bestPromptErrors[i] : 0.0;

        promptGraph->SetPoint(idx, x, y);
        promptGraph->SetPointError(idx, xErr, yErr);

        if (scaleMax > scaleMin) {
            double mappedY = yMin
                + (bestScales[i] - scaleMin) * (yMax - yMin) / (scaleMax - scaleMin);
            scaleGraph->SetPoint(idx, x, mappedY);
        }
    }

    auto canvas = std::make_unique<TCanvas>("c_promptFracVsCosAuto", "", 900, 700);
    canvas->SetGrid();

    const char* xTitle = useAbsCos ? "|cos#theta_{HX}|" : "cos#theta_{HX}";
    promptGraph->SetTitle(Form(";%s;Prompt fraction (%%)", xTitle));
    promptGraph->SetMarkerStyle(20);
    promptGraph->SetMarkerSize(1.2);
    promptGraph->SetLineWidth(2);

    promptGraph->Draw("AP");
    promptGraph->GetYaxis()->SetRangeUser(yMin, yMax);
    canvas->Update();

    TLegend legend(0.15, 0.75, 0.45, 0.88);
    legend.SetBorderSize(0);
    legend.SetFillStyle(0);
    legend.SetTextSize(0.04);
    legend.AddEntry(promptGraph.get(), "Prompt fraction", "pl");

    if (scaleGraph->GetN() == promptGraph->GetN() && scaleMax > scaleMin) {
        scaleGraph->SetLineColor(kBlue + 1);
        scaleGraph->SetMarkerColor(kBlue + 1);
        scaleGraph->SetMarkerStyle(24);
        scaleGraph->SetMarkerSize(1.1);
        scaleGraph->SetLineWidth(2);
        scaleGraph->Draw("LP SAME");
        legend.AddEntry(scaleGraph.get(), "Best scale", "pl");

        double xRight = gPad->GetUxmax();
        TGaxis rightAxis(xRight, yMin, xRight, yMax,
                         scaleMin, scaleMax, 510, "+L");
        rightAxis.SetLineColor(kBlue + 1);
        rightAxis.SetLabelColor(kBlue + 1);
        rightAxis.SetTitleColor(kBlue + 1);
        rightAxis.SetTitle("Best scale");
        rightAxis.SetTitleOffset(1.1);
        rightAxis.Draw();
    }

    legend.Draw();

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextSize(0.045);
    latex.DrawLatex(0.88, 0.93, "#bf{CMS} #it{Internal}");
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.88, 0.87,
                    Form("%0.1f < p_{T} < %0.1f GeV/c", ptMin, ptMax));
    latex.DrawLatex(0.88, 0.82,
                    Form("%d-%d%% centrality", centralityMin, centralityMax));

    TLatex scaleText;
    scaleText.SetTextFont(42);
    scaleText.SetTextSize(0.03);
    scaleText.SetTextAlign(21);
    scaleText.SetNDC(false);

    for (size_t i = 0; i < cosCenters.size(); ++i) {
        double baseY = bestPromptFractions[i];
        double errY = (i < bestPromptErrors.size()) ? bestPromptErrors[i] : 0.0;
        double labelY = std::min(yMax - 2.0, baseY + errY + 6.0);
        scaleText.DrawLatex(cosCenters[i], labelY,
                            Form("scale = %.2f", bestScales[i]));
    }

    std::string canvasPath = summaryDir + "/prompt_fraction_vs_cos_best";
    canvas->SaveAs((canvasPath + ".png").c_str());
    canvas->SaveAs((canvasPath + ".pdf").c_str());

    std::string tablePath = summaryDir + "/best_scale_summary.txt";
    std::ofstream table(tablePath);
    if (!table.good()) {
        std::cerr << "[CosScanAuto] Could not write summary table at "
                  << tablePath << std::endl;
        return;
    }

    table << "# pT_min pT_max cos_min cos_max best_scale prompt_frac prompt_err chi2_over_ndf\n";
    table << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < cosCenters.size(); ++i) {
        double cosMin = cosBins[i].first;
        double cosMax = cosBins[i].second;
        double frac = bestPromptFractions[i];
        double err = (i < bestPromptErrors.size()) ? bestPromptErrors[i] : 0.0;
        double chi2ndf = (i < bestChi2Ndf.size()) ? bestChi2Ndf[i]
                           : std::numeric_limits<double>::quiet_NaN();

        table << std::setw(8) << ptMin
              << " " << std::setw(8) << ptMax
              << " " << std::setw(8) << cosMin
              << " " << std::setw(8) << cosMax
              << " " << std::setw(8) << bestScales[i]
              << " " << std::setw(8) << frac
              << " " << std::setw(8) << err
              << " " << std::setw(8) << chi2ndf
              << "\n";
    }
}
