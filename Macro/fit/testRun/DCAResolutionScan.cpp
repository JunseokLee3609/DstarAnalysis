#include "../DStarFitOpt.h"
#include "../DCAFitter.h"
#include "../../Tools/ConfigManagerPP.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include "TSystem.h"
#include "TFile.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooAddPdf.h"
#include "RooFitResult.h"
#include "RooChi2Var.h"
#include "RooBinning.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <map>
#include <utility>
#include <limits>
#include <cmath>
#include "../Opt.h"

using namespace RooFit;

namespace {
std::string MakeBinName(const KinematicBin& bin) {
    FitOpt dummy;
    dummy.pTMin = bin.pTMin;
    dummy.pTMax = bin.pTMax;
    dummy.cosMin = bin.cosMin;
    dummy.cosMax = bin.cosMax;
    dummy.centMin = bin.centralityMin;
    dummy.centMax = bin.centralityMax;
    return dummy.GetBinName();
}

std::unique_ptr<TH1D> LoadDataYieldHistogram(const std::string& filePath,
                                             const std::string& histName) {
    std::unique_ptr<TFile> fin(TFile::Open(filePath.c_str(), "READ"));
    if (!fin || fin->IsZombie()) {
        std::cerr << "[DCAResolutionScan] Failed to open data histogram file: " << filePath << std::endl;
        return nullptr;
    }
    TH1D* hist = dynamic_cast<TH1D*>(fin->Get(histName.c_str()));
    if (!hist) {
        std::cerr << "[DCAResolutionScan] Histogram '" << histName
                  << "' not found in file " << filePath << std::endl;
        return nullptr;
    }
    auto clone = std::unique_ptr<TH1D>(static_cast<TH1D*>(hist->Clone("dataYieldHist_scan")));
    clone->SetDirectory(nullptr);
    clone->Sumw2();
    return clone;
}

}

void DCAResolutionScan(double ptMin, double ptMax,
                       double cosMin, double cosMax,
                       int centralityMin=0., int centralityMax=10,
                       double scaleMin = 0.7,
                       double scaleMax = 1.4,
                       double scaleStep = 0.01,
                       const std::vector<double>* customScales = nullptr,
                       bool makePerScalePlots = true,
                       std::vector<double>* outScales = nullptr,
                       std::vector<double>* outPromptFractions = nullptr,
                       std::vector<double>* outPromptFractionErrors = nullptr,
                       std::vector<double>* outChi2Values = nullptr,
                       std::vector<double>* outNdfValues = nullptr) {
    gStyle->SetOptStat(0);

    // --- Configure D* analysis (mirrors DStarAnalysisV2forpp) ---
    DStarFitOpt config;
    config.SetYieldModeAutoTuning(false);
    config.SetUseIndependentYields(true);

    // File paths should match the existing pp configuration
    config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_pp_12Sep25_v1.root");
    config.SetMCFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_ppPU_np_16Sep25_v1.root");
    config.SetDatasetName("datasetHX");
    config.SetOutputSubDir(SelectionCuts::SUB_DIR);
    config.SetFitMethod(FitMethod::GaussianConstraintWithSB);
    config.SetUseCUDA(true);
    config.SetVerbose(true);

    // Define single bin to analyse
    config.AddPtBin(ptMin, ptMax);
    config.AddCosBin(cosMin, cosMax);
    // config.AddCentralityBin(centralityMin, centralityMax);
    KinematicBin bin(ptMin, ptMax, cosMin, cosMax);

    FitOpt fitOpt = config.CreateFitOpt(bin);
    fitOpt.GenerateLegends();

    // Configure DCA-specific FitOpt
    FitOpt dcaOpt = fitOpt;
    // dcaOpt.massVar = "massDaugther1";
    // dcaOpt.massMin = 1.75;
    // dcaOpt.massMax = 2.00;
    dcaOpt.dcaVar = "dca3D";

    // std::string binName = MakeBinName(bin);
    std::string binName = bin.GetBinName();
    std::cout << "[DCAResolutionScan] Analysing bin: " << binName << std::endl;

    const std::string ptLabel = fitOpt.pTLegend.empty() ?
        Form("%0.1f < p_{T} < %0.1f GeV/c", ptMin, ptMax) : fitOpt.pTLegend;
    const std::string cosLabel = fitOpt.cosLegend.empty() ?
        Form("%0.2f < cos#theta_{HX} < %0.2f", cosMin, cosMax) : fitOpt.cosLegend;

    // Instantiate DCAFitter to access MC datasets
    DCAFitter dcaFitter(dcaOpt, "DCAFitterResolution", dcaOpt.massVar,
                        dcaOpt.dcaMin, dcaOpt.dcaMax, static_cast<int>(dcaOpt.dcaBins.size()) - 1);

    // Configure MC/data sources (reuse previously produced workspaces)
    {
        std::string mcOutDir = fitOpt.outputDir + fitOpt.subDir + "/MC";
        std::string dataOutDir = fitOpt.outputDir + fitOpt.subDir + "/Data";
        std::string mcResultFile = mcOutDir + "/" + fitOpt.outputFile + ".root";
        std::string dataResultFile = dataOutDir + "/" + fitOpt.outputFile + ".root";
        dcaFitter.setMCResultFile(mcResultFile, "workspace", "");
        dcaFitter.setDataResultFile(dataResultFile, "workspace", "");
    }

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
    std::string mcCuts = replaceCentrality(dcaOpt.cutMCExpr);
    std::string dataCuts = replaceCentrality(dcaOpt.cutExpr);
    dcaFitter.setMCCuts(mcCuts);
    dcaFitter.setDataCuts(dataCuts);

    if (!dcaFitter.createTemplatesFromMC()) {
        std::cerr << "[DCAResolutionScan] Failed to build base MC templates." << std::endl;
        return;
    }

    // Load data-yield histogram
    std::string dataHistFile = fitOpt.outputDir + fitOpt.subDir + "/Data/dcahist/DStar_DCA_Yield_" + binName + ".root";
    auto dataHist = LoadDataYieldHistogram(dataHistFile, "dataYieldHist");
    if (!dataHist || dataHist->Integral() <= 0) {
        std::cerr << "[DCAResolutionScan] Invalid data histogram." << std::endl;
        return;
    }

    const std::vector<double>& dcaBins = dcaOpt.dcaBins;
    RooRealVar dca("dca", "D^{0} DCA (cm)", dcaBins.front(), dcaBins.back());
    dca.setBins(static_cast<int>(dcaBins.size()) - 1);

    RooDataHist dataDH("dataDH", "dataDH", RooArgList(dca), dataHist.get());
    double dataIntegral = dataHist->Integral();

    // Prepare output directories
    std::string outputBase = fitOpt.outputDir + fitOpt.subDir + "/Data/dca_resolution_scan/" + binName;
    gSystem->mkdir(outputBase.c_str(), true);

    std::vector<double> scales;
    std::vector<double> chi2Values;
    std::vector<double> ndfValues;
    std::vector<double> promptFractions;
    std::vector<double> promptFractionErrors;

    std::vector<double> scanScales;
    if (customScales && !customScales->empty()) {
        scanScales = *customScales;
    } else {
        for (double scale = scaleMin; scale <= scaleMax + 1e-6; scale += scaleStep) {
            scanScales.push_back(scale);
        }
    }

    auto cloneHist = [](const TH1D* src, const std::string& name) -> std::unique_ptr<TH1D> {
        if (!src) {
            return nullptr;
        }
        auto cloned = std::unique_ptr<TH1D>(static_cast<TH1D*>(src->Clone(name.c_str())));
        if (cloned) {
            cloned->SetDirectory(nullptr);
        }
        return cloned;
    };

    auto formatScaleTag = [](double value) {
        std::string tag = Form("%.3f", value);
        std::replace(tag.begin(), tag.end(), '.', 'p');
        if (!tag.empty() && tag.front() == '-') {
            tag.front() = 'm';
        }
        return tag;
    };

    auto saveFitPlot = [&](const std::string& baseName,
                           double scaleValue,
                           double chi2Value,
                           int ndfValue,
                           double promptFracPct,
                           double nonPromptFracPct,
                           RooPlot* frame,
                           TH1D* ratioHist) {
        if (!frame || !ratioHist) {
            return;
        }

        static int plotCounter = 0;
        ++plotCounter;
        std::string padMainName = Form("padMain_%d", plotCounter);
        std::string padRatioName = Form("padRatio_%d", plotCounter);

        auto canvas = std::make_unique<TCanvas>((baseName + "_c").c_str(), "", 800, 900);

        TPad* padMain = new TPad(padMainName.c_str(), padMainName.c_str(), 0, 0.30, 1, 1);
        padMain->SetBottomMargin(0.02);
        padMain->SetLeftMargin(0.15);
        padMain->SetRightMargin(0.05);
        padMain->SetTopMargin(0.08);
        padMain->SetLogy();
        padMain->Draw();

        TPad* padRatio = new TPad(padRatioName.c_str(), padRatioName.c_str(), 0, 0.0, 1, 0.30);
        padRatio->SetTopMargin(0.05);
        padRatio->SetBottomMargin(0.35);
        padRatio->SetLeftMargin(0.15);
        padRatio->SetRightMargin(0.05);
        padRatio->Draw();

        padMain->cd();
        frame->Draw();

        TLegend leg(0.65, 0.65, 0.92, 0.88);
        leg.SetBorderSize(0);
        leg.SetFillStyle(0);
        leg.SetTextSize(0.035);

        if (auto dataObj = frame->findObject("data_yield_hist")) {
            leg.AddEntry(dataObj, "Data yield", "lep");
        }
        if (auto totalObj = frame->findObject("model_total")) {
            leg.AddEntry(totalObj, "Total fit", "l");
        }
        if (auto promptObj = frame->findObject("mc_prompt_comp")) {
            leg.AddEntry(promptObj, Form("Prompt (%.0f%%)", promptFracPct), "f");
        }
        if (auto nonPromptObj = frame->findObject("mc_nonprompt_comp")) {
            leg.AddEntry(nonPromptObj, Form("Non-prompt (%.0f%%)", nonPromptFracPct), "f");
        }
        leg.Draw();

        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.15, 0.94, "#bf{CMS} #it{Internal}");
        latex.SetTextSize(0.04);
        latex.DrawLatex(0.65, 0.56, Form("%0.1f < p_{T} < %0.1f GeV/c", ptMin, ptMax));
        latex.DrawLatex(0.65, 0.52, Form("%0.2f < cos#theta_{HX} < %0.2f", cosMin, cosMax));
        latex.DrawLatex(0.65, 0.48, Form("Scale = %.3f", scaleValue));
        if (ndfValue > 0 && std::isfinite(chi2Value)) {
            latex.DrawLatex(0.65, 0.44, Form("#chi^{2}/NDF = %.2f / %d", chi2Value, ndfValue));
        }

        padRatio->cd();
        ratioHist->SetTitle("");
        ratioHist->SetMarkerStyle(20);
        ratioHist->SetMarkerSize(1.0);
        ratioHist->SetMarkerColor(kBlack);
        ratioHist->SetLineColor(kBlack);
        ratioHist->GetYaxis()->SetTitle("Data / Fit");
        ratioHist->GetYaxis()->SetTitleSize(0.12);
        ratioHist->GetYaxis()->SetLabelSize(0.11);
        ratioHist->GetYaxis()->SetTitleOffset(0.5);
        ratioHist->GetYaxis()->SetNdivisions(505);
        ratioHist->GetXaxis()->SetTitle("D^{0} DCA (cm)");
        ratioHist->GetXaxis()->SetTitleSize(0.12);
        ratioHist->GetXaxis()->SetLabelSize(0.11);
        ratioHist->GetXaxis()->SetTitleOffset(1.0);
        ratioHist->SetMinimum(0.0);
        ratioHist->SetMaximum(2.0);
        ratioHist->Draw("E1");
        if (!dcaBins.empty()) {
            TLine ratioLine(dcaBins.front(), 1.0, dcaBins.back(), 1.0);
            ratioLine.SetLineColor(kRed);
            ratioLine.SetLineStyle(2);
            ratioLine.Draw();
        }

        canvas->cd();
        canvas->SaveAs((baseName + ".png").c_str());
        canvas->SaveAs((baseName + ".pdf").c_str());
    };

    double bestChi2Ndf = std::numeric_limits<double>::infinity();
    double bestScale = 0.0;
    double bestChi2 = std::numeric_limits<double>::quiet_NaN();
    int bestNdf = 0;
    double bestPromptFraction = std::numeric_limits<double>::quiet_NaN();
    double bestNonPromptFraction = std::numeric_limits<double>::quiet_NaN();
    bool haveBestPlot = false;
    std::unique_ptr<TH1D> bestPromptFit;
    std::unique_ptr<TH1D> bestNonPromptFit;
    std::unique_ptr<TH1D> bestTotalFit;
    std::unique_ptr<TH1D> bestRatioHist;
    std::unique_ptr<TH1D> bestDataHist;
    std::unique_ptr<RooPlot> bestMainFrame;

    for (double scale : scanScales) {
        double recordedChi2 = std::numeric_limits<double>::quiet_NaN();
        int recordedNdf = 0;
        double recordedPromptFraction = std::numeric_limits<double>::quiet_NaN();
        double recordedPromptError = 0.0;
        std::unique_ptr<TH1D> hPrompt;
        std::unique_ptr<TH1D> hNonPrompt;
        if (!dcaFitter.GenerateTemplatesWithScale(scale, hPrompt, hNonPrompt)) {
            std::cerr << "[DCAResolutionScan] Failed to build templates for scale " << scale << std::endl;
        } else if (!hPrompt || !hNonPrompt || hPrompt->Integral() <= 0 || hNonPrompt->Integral() <= 0) {
            std::cerr << "[DCAResolutionScan] Empty template at scale " << scale << std::endl;
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
                std::cerr << "[DCAResolutionScan] Fit failed at scale " << scale << std::endl;
            } else {
                RooChi2Var chi2Var("chi2Var", "chi2Var", model, dataDH);
                recordedChi2 = chi2Var.getVal();
                recordedNdf = static_cast<int>(dataHist->GetNbinsX()) - fitRes->floatParsFinal().getSize();

                auto hPromptFit = std::unique_ptr<TH1D>(static_cast<TH1D*>(hPrompt->Clone("hPromptFit")));
                auto hNonPromptFit = std::unique_ptr<TH1D>(static_cast<TH1D*>(hNonPrompt->Clone("hNonPromptFit")));
                const char* widthOpt = "width";
                if (hPromptFit->Integral() > 0) {
                    hPromptFit->Scale(nPrompt.getVal() / hPromptFit->Integral());
                    hPromptFit->Scale(1.0, widthOpt);
                }
                if (hNonPromptFit->Integral() > 0) {
                    hNonPromptFit->Scale(nNonPrompt.getVal() / hNonPromptFit->Integral());
                    hNonPromptFit->Scale(1.0, widthOpt);
                }
                auto hTotalFit = std::unique_ptr<TH1D>(static_cast<TH1D*>(hPromptFit->Clone("hTotalFit")));
                hTotalFit->Add(hNonPromptFit.get());

                auto scaledDataHist = std::unique_ptr<TH1D>(static_cast<TH1D*>(dataHist->Clone("hDataScaled")));
                scaledDataHist->Scale(1.0, widthOpt);

                auto hRatio = std::unique_ptr<TH1D>(static_cast<TH1D*>(scaledDataHist->Clone("hRatio")));
                for (int ib = 1; ib <= hRatio->GetNbinsX(); ++ib) {
                    double fitVal = hTotalFit->GetBinContent(ib);
                    double dataVal = scaledDataHist->GetBinContent(ib);
                    double dataErr = scaledDataHist->GetBinError(ib);
                    if (fitVal > 0) {
                        hRatio->SetBinContent(ib, dataVal / fitVal);
                        hRatio->SetBinError(ib, (dataVal > 0) ? dataErr / fitVal : 0.0);
                    } else {
                        hRatio->SetBinContent(ib, 0.0);
                        hRatio->SetBinError(ib, 0.0);
                    }
                }

                double totalYield = nPrompt.getVal() + nNonPrompt.getVal();
                double promptFraction = (totalYield > 0) ? nPrompt.getVal() / totalYield : 0.0;
                double recordedNonPromptFraction = (totalYield > 0)
                    ? 100.0 * nNonPrompt.getVal() / totalYield : 0.0;
                recordedPromptFraction = promptFraction * 100.0;
                if (totalYield > 0) {
                    double denom = totalYield;
                    double dfdPrompt = (totalYield - nPrompt.getVal()) / (denom * denom);
                    double dfdNonPrompt = -nPrompt.getVal() / (denom * denom);
                    double errPrompt = nPrompt.getError();
                    double errNonPrompt = nNonPrompt.getError();
                    double variance = (dfdPrompt * errPrompt) * (dfdPrompt * errPrompt)
                                      + (dfdNonPrompt * errNonPrompt) * (dfdNonPrompt * errNonPrompt);
                    recordedPromptError = std::sqrt(std::max(0.0, variance)) * 100.0;
                }
                double chi2ndfCandidate = (recordedNdf > 0)
                    ? recordedChi2 / recordedNdf : std::numeric_limits<double>::quiet_NaN();
                if (makePerScalePlots && std::isfinite(chi2ndfCandidate)
                    && chi2ndfCandidate < bestChi2Ndf) {
                    bestChi2Ndf = chi2ndfCandidate;
                    bestScale = scale;
                    bestChi2 = recordedChi2;
                    bestNdf = recordedNdf;
                    bestPromptFraction = recordedPromptFraction;
                    bestNonPromptFraction = recordedNonPromptFraction;
                    haveBestPlot = true;
                    bestPromptFit = cloneHist(hPromptFit.get(), "bestPromptFit");
                    bestNonPromptFit = cloneHist(hNonPromptFit.get(), "bestNonPromptFit");
                    bestTotalFit = cloneHist(hTotalFit.get(), "bestTotalFit");
                    bestRatioHist = cloneHist(hRatio.get(), "bestRatioHist");
                    bestDataHist = cloneHist(scaledDataHist.get(), "bestDataHist");

                    auto frame = std::unique_ptr<RooPlot>(dca.frame(Bins(static_cast<int>(dcaBins.size()) - 1), RooFit::Title("")));
                    frame->SetName("dca_resolution_frame");
                    frame->SetTitle("");
                    

                    RooBinning customBinning(static_cast<int>(dcaBins.size()) - 1, dcaBins.data());

                    dataDH.plotOn(frame.get(), RooFit::Binning(customBinning),
                                  RooFit::Name("data_yield_hist"),
                                  RooFit::DataError(RooAbsData::SumW2));

                    model.plotOn(frame.get(), RooFit::Name("model_total"), RooFit::LineColor(kRed + 1));
                    model.plotOn(frame.get(), RooFit::Components(nonPromptPdf), RooFit::Name("mc_nonprompt_comp"),
                                 RooFit::FillStyle(3354), RooFit::FillColor(kBlue - 9),
                                 RooFit::LineColor(kBlue + 1), RooFit::DrawOption("F"));
                    model.plotOn(frame.get(), RooFit::Components(promptPdf), RooFit::Name("mc_prompt_comp"),
                                 RooFit::FillStyle(3345), RooFit::FillColor(kRed - 9),
                                 RooFit::LineColor(kRed + 1), RooFit::DrawOption("F"));
                    model.plotOn(frame.get(), RooFit::Name("model_total"), RooFit::LineColor(kRed + 1));

                    double frameBinWidth = dcaBins.size() > 1 ? (dcaBins[1] - dcaBins[0]) : 0.0;
                    if (!bestDataHist->IsZombie() && bestDataHist->GetNbinsX() > 0) {
                        frameBinWidth = bestDataHist->GetBinWidth(1);
                    }
                    frame->GetYaxis()->SetTitle(Form("Yield / (%.3f cm)", frameBinWidth));
                    frame->GetYaxis()->SetTitleOffset(1.4);
                    frame->GetXaxis()->SetLabelSize(0.0);
                    frame->SetMinimum(1);
                    frame->SetMaximum(frame->GetMaximum() * 1.8);

                    bestMainFrame.reset(static_cast<RooPlot*>(frame->Clone()));
                }
            }
        }

        scales.push_back(scale);
        chi2Values.push_back(recordedChi2);
        ndfValues.push_back(recordedNdf);
        promptFractions.push_back(recordedPromptFraction);
        promptFractionErrors.push_back(recordedPromptError);
    }

    if (makePerScalePlots && haveBestPlot && bestPromptFit && bestNonPromptFit
        && bestTotalFit && bestRatioHist && bestDataHist && bestMainFrame) {
        std::string canvasName = Form("%s/scale_bestfit", outputBase.c_str());
        auto canvas = std::make_unique<TCanvas>((canvasName + "_c").c_str(), "", 800, 900);

        TPad* padMain = new TPad("padMain", "padMain", 0, 0.30, 1, 1);
        padMain->SetBottomMargin(0.02);
        padMain->SetLeftMargin(0.15);
        padMain->SetRightMargin(0.05);
        padMain->SetTopMargin(0.08);
        padMain->SetLogy();
        padMain->Draw();

        TPad* padRatio = new TPad("padRatio", "padRatio", 0, 0.0, 1, 0.30);
        padRatio->SetTopMargin(0.05);
        padRatio->SetBottomMargin(0.35);
        padRatio->SetLeftMargin(0.15);
        padRatio->SetRightMargin(0.05);
        padRatio->Draw();

        padMain->cd();

        bestMainFrame->Draw();

        TLegend leg(0.65, 0.65, 0.92, 0.88);
        leg.SetBorderSize(0);
        leg.SetFillStyle(0);
        leg.SetTextSize(0.035);

        if (auto dataObj = bestMainFrame->findObject("data_yield_hist")) {
            leg.AddEntry(dataObj, "Data yield", "lep");
        }
        if (auto totalObj = bestMainFrame->findObject("model_total")) {
            leg.AddEntry(totalObj, "Total fit", "l");
        }
        if (auto promptObj = bestMainFrame->findObject("mc_prompt_comp")) {
            leg.AddEntry(promptObj, Form("Prompt (%.0f%%)", bestPromptFraction), "f");
        }
        if (auto nonPromptObj = bestMainFrame->findObject("mc_nonprompt_comp")) {
            leg.AddEntry(nonPromptObj, Form("Non-prompt (%.0f%%)", bestNonPromptFraction), "f");
        }
        leg.Draw();

        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.15, 0.94, "#bf{CMS} #it{Internal}");
        latex.SetTextSize(0.04);
        latex.DrawLatex(0.65, 0.56, Form("%0.1f < p_{T} < %0.1f GeV/c", ptMin, ptMax));
        latex.DrawLatex(0.65, 0.52, Form("%0.2f < cos#theta_{HX} < %0.2f", cosMin, cosMax));
        latex.DrawLatex(0.65, 0.48, Form("Scale = %.2f", bestScale));
        latex.DrawLatex(0.65, 0.44, Form("#chi^{2}/NDF = %.2f / %d", bestChi2, bestNdf));

        padRatio->cd();
        bestRatioHist->SetTitle("");
        bestRatioHist->SetMarkerStyle(20);
        bestRatioHist->SetMarkerSize(1.0);
        bestRatioHist->SetMarkerColor(kBlack);
        bestRatioHist->SetLineColor(kBlack);
        bestRatioHist->GetYaxis()->SetTitle("Data / Fit");
        bestRatioHist->GetYaxis()->SetTitleSize(0.12);
        bestRatioHist->GetYaxis()->SetLabelSize(0.11);
        bestRatioHist->GetYaxis()->SetTitleOffset(0.5);
        bestRatioHist->GetYaxis()->SetNdivisions(505);
        bestRatioHist->GetXaxis()->SetTitle("D^{0} DCA (cm)");
        bestRatioHist->GetXaxis()->SetTitleSize(0.12);
        bestRatioHist->GetXaxis()->SetLabelSize(0.11);
        bestRatioHist->GetXaxis()->SetTitleOffset(1.0);
        bestRatioHist->SetMinimum(0.0);
        bestRatioHist->SetMaximum(2.0);
        bestRatioHist->Draw("E1");
        TLine ratioLine(dcaBins.front(), 1.0, dcaBins.back(), 1.0);
        ratioLine.SetLineColor(kRed);
        ratioLine.SetLineStyle(2);
        ratioLine.Draw();

        canvas->cd();
        canvas->SaveAs((canvasName + ".png").c_str());
        canvas->SaveAs((canvasName + ".pdf").c_str());
    } else if (makePerScalePlots && !haveBestPlot) {
        std::cerr << "[DCAResolutionScan] Warning: no successful fit to plot." << std::endl;
    }

    if (!scales.empty()) {
        std::vector<double> chi2ndf(scales.size());
        for (size_t i = 0; i < scales.size(); ++i) {
            double ndf = (ndfValues[i] > 0) ? ndfValues[i] : 1.0;
            chi2ndf[i] = chi2Values[i] / ndf;
        }
        auto graph = std::make_unique<TGraph>(static_cast<int>(scales.size()));
        for (int i = 0; i < graph->GetN(); ++i) {
            graph->SetPoint(i, scales[i], chi2ndf[i]);
        }
        graph->SetTitle(";Resolution scale;#chi^{2}/NDF");
        graph->SetMarkerStyle(21);
        graph->SetMarkerSize(1.1);
        graph->SetLineWidth(2);

        std::string graphPath = outputBase + "/chi2_vs_scale";
        auto cGraph = std::make_unique<TCanvas>((graphPath + "_c").c_str(), "", 800, 600);
        cGraph->SetGrid();
        graph->Draw("APL");
        TLatex graphLatex;
        graphLatex.SetNDC();
        graphLatex.SetTextFont(42);
        graphLatex.SetTextSize(0.04);
        graphLatex.DrawLatex(0.18, 0.88, ptLabel.c_str());
        graphLatex.DrawLatex(0.18, 0.83, cosLabel.c_str());
        cGraph->SaveAs((graphPath + ".png").c_str());
        cGraph->SaveAs((graphPath + ".pdf").c_str());

        auto minIt = std::min_element(chi2ndf.begin(), chi2ndf.end());
        if (minIt != chi2ndf.end()) {
            size_t idx = std::distance(chi2ndf.begin(), minIt);
            std::cout << "[DCAResolutionScan] Best scale = " << scales[idx]
                      << " with chi2/ndf = " << chi2ndf[idx] << std::endl;
        }
    }

    if (!promptFractions.empty()) {
        auto fracGraph = std::make_unique<TGraphErrors>(static_cast<int>(promptFractions.size()));
        for (int i = 0; i < fracGraph->GetN(); ++i) {
            double errY = (i < static_cast<int>(promptFractionErrors.size())) ? promptFractionErrors[i] : 0.0;
            fracGraph->SetPoint(i, scales[i], promptFractions[i]);
            fracGraph->SetPointError(i, 0.0, errY);
        }
        fracGraph->SetTitle(";Resolution scale;Prompt fraction (%)");
        fracGraph->SetMarkerStyle(21);
        fracGraph->SetMarkerSize(1.1);
        fracGraph->SetLineWidth(2);

        std::string fracGraphPath = outputBase + "/prompt_fraction_vs_scale";
        auto cFracGraph = std::make_unique<TCanvas>((fracGraphPath + "_c").c_str(), "", 800, 600);
        cFracGraph->SetGrid();
        fracGraph->Draw("APL");
        cFracGraph->SaveAs((fracGraphPath + ".png").c_str());
        cFracGraph->SaveAs((fracGraphPath + ".pdf").c_str());
    }

    if (outScales) {
        *outScales = scales;
    }
    if (outPromptFractions) {
        *outPromptFractions = promptFractions;
    }
    if (outPromptFractionErrors) {
        *outPromptFractionErrors = promptFractionErrors;
    }
    if (outChi2Values) {
        *outChi2Values = chi2Values;
    }
    if (outNdfValues) {
        *outNdfValues = ndfValues;
    }
}

void DCAResolutionScanPromptFractionVsCos(double ptMin, double ptMax,
                                          const std::vector<std::pair<double, double>>& cosBins,
                                          int centralityMin,
                                          int centralityMax,
                                          const std::vector<double>& scaleValues) {
    if (scaleValues.empty()) {
        std::cerr << "[DCAResolutionScanPromptFractionVsCos] No scale values provided." << std::endl;
        return;
    }

    std::vector<double> plotScales = scaleValues;
    std::sort(plotScales.begin(), plotScales.end());
    plotScales.erase(std::unique(plotScales.begin(), plotScales.end()), plotScales.end());

    std::vector<double> scanScales = plotScales;
    if (plotScales.size() >= 2) {
        double minScale = plotScales.front();
        double maxScale = plotScales.back();
        if (maxScale < minScale) {
            std::swap(minScale, maxScale);
        }
        const double step = 0.01;
        const double eps = 1e-6;
        double start = std::floor(minScale * 100.0 + eps) / 100.0;
        double end = std::ceil(maxScale * 100.0 - eps) / 100.0;
        scanScales.clear();
        for (double val = start; val <= end + eps; val += step) {
            double rounded = std::round(val * 100.0) / 100.0;
            if (!scanScales.empty() && std::abs(rounded - scanScales.back()) < 1e-6) {
                continue;
            }
            scanScales.push_back(rounded);
        }
    }
    if (scanScales.empty()) {
        scanScales.push_back(plotScales.front());
    }

    std::vector<std::pair<double, double>> workingCosBins = cosBins;
    if (workingCosBins.empty()) {
        static const double defaultEdges[] = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
        const size_t edgeCount = sizeof(defaultEdges) / sizeof(defaultEdges[0]);
        workingCosBins.reserve(edgeCount - 1);
        for (size_t i = 0; i + 1 < edgeCount; ++i) {
            workingCosBins.emplace_back(defaultEdges[i], defaultEdges[i + 1]);
        }
        std::cout << "[DCAResolutionScanPromptFractionVsCos] Using default cos#theta_{HX} bins: "
                  << "[0.0,0.2,0.4,0.6,0.8,1.0]" << std::endl;
    }

    // Configure once to obtain output directory and label templates (using first cos bin)
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
    config.AddCosBin(workingCosBins.front().first, workingCosBins.front().second);
    // config.AddCentralityBin(centralityMin, centralityMax);
    KinematicBin firstBin(ptMin, ptMax, workingCosBins.front().first, workingCosBins.front().second);

    FitOpt fitOpt = config.CreateFitOpt(firstBin);
    fitOpt.GenerateLegends();

    std::string baseBinName = firstBin.GetBinName();
    std::string combinedBinName = baseBinName + "_cosMulti";
    std::string outputBase = fitOpt.outputDir + fitOpt.subDir + "/Data/dca_resolution_scan/" + combinedBinName;
    gSystem->mkdir(outputBase.c_str(), true);

    std::string ptLabel = fitOpt.pTLegend.empty() ?
        Form("%0.1f < p_{T} < %0.1f GeV/c", ptMin, ptMax) : fitOpt.pTLegend;
    std::string centLabel = fitOpt.centLegend.empty() ?
        Form("%d-%d%% centrality", centralityMin, centralityMax) : fitOpt.centLegend;

    std::map<double, std::vector<double>> fractionByScale;
    std::map<double, std::vector<double>> errorByScale;
    std::vector<double> cosCenters;
    std::vector<double> cosHalfWidths;
    std::vector<double> optimalPromptFractions;
    std::vector<double> optimalPromptErrors;
    std::vector<double> optimalScales;

    const auto matchPlotScale = [&](double scale, double& matched) {
        for (double target : plotScales) {
            if (std::abs(scale - target) < 5e-4) {
                matched = target;
                return true;
            }
        }
        return false;
    };

    for (const auto& cb : workingCosBins) {
        double cMin = cb.first;
        double cMax = cb.second;
        cosCenters.push_back(0.5 * (cMin + cMax));
        cosHalfWidths.push_back(0.5 * std::abs(cMax - cMin));

        std::vector<double> scalesOut;
        std::vector<double> fractionsOut;
        std::vector<double> errorsOut;
        std::vector<double> chi2Out;
        std::vector<double> ndfOut;

        double step = (scanScales.size() > 1) ? std::abs(scanScales[1] - scanScales[0]) : 1.0;
        DCAResolutionScan(ptMin, ptMax, cMin, cMax,
                          centralityMin, centralityMax,
                          scanScales.front(), scanScales.back(),
                          step,
                          &scanScales, false,
                          &scalesOut, &fractionsOut, &errorsOut,
                          &chi2Out, &ndfOut);

        if (scalesOut.size() != scanScales.size()) {
            std::cerr << "[DCAResolutionScanPromptFractionVsCos] Warning: requested "
                      << scanScales.size() << " scales but obtained " << scalesOut.size()
                      << " for cos bin " << cMin << "-" << cMax << std::endl;
        }

        int bestIndex = -1;
        double bestChi2Ndf = std::numeric_limits<double>::infinity();

        for (size_t idx = 0; idx < scalesOut.size(); ++idx) {
            double scale = scalesOut[idx];
            double frac = (idx < fractionsOut.size()) ? fractionsOut[idx] : std::numeric_limits<double>::quiet_NaN();
            double err = (idx < errorsOut.size()) ? errorsOut[idx] : 0.0;
            double chi2 = (idx < chi2Out.size()) ? chi2Out[idx] : std::numeric_limits<double>::quiet_NaN();
            double ndf = (idx < ndfOut.size()) ? ndfOut[idx] : 0.0;
            if (std::isfinite(chi2) && ndf > 0.0) {
                double chi2ndf = chi2 / ndf;
                if (chi2ndf < bestChi2Ndf) {
                    bestChi2Ndf = chi2ndf;
                    bestIndex = static_cast<int>(idx);
                }
            }
            double matchedScale = 0.0;
            if (matchPlotScale(scale, matchedScale)) {
                fractionByScale[matchedScale].push_back(frac);
                errorByScale[matchedScale].push_back(err);
            }
        }

        if (bestIndex >= 0 && bestIndex < static_cast<int>(fractionsOut.size())) {
            double bestFrac = fractionsOut[bestIndex];
            double bestErr = (bestIndex < static_cast<int>(errorsOut.size())) ? errorsOut[bestIndex] : 0.0;
            double bestScale = (bestIndex < static_cast<int>(scalesOut.size())) ? scalesOut[bestIndex] : std::numeric_limits<double>::quiet_NaN();
            optimalPromptFractions.push_back(bestFrac);
            optimalPromptErrors.push_back(bestErr);
            optimalScales.push_back(bestScale);
        } else {
            optimalPromptFractions.push_back(std::numeric_limits<double>::quiet_NaN());
            optimalPromptErrors.push_back(0.0);
            optimalScales.push_back(std::numeric_limits<double>::quiet_NaN());
        }
    }

    for (double scale : plotScales) {
        auto& vals = fractionByScale[scale];
        auto& errs = errorByScale[scale];
        if (vals.size() < cosCenters.size()) {
            vals.resize(cosCenters.size(), std::numeric_limits<double>::quiet_NaN());
        }
        if (errs.size() < cosCenters.size()) {
            errs.resize(cosCenters.size(), 0.0);
        }
    }

    auto canvas = std::make_unique<TCanvas>((combinedBinName + "_promptFracCos_c").c_str(), "", 900, 700);
    canvas->SetGrid();

    TMultiGraph multiGraph;
    std::vector<std::unique_ptr<TGraphErrors>> graphs;
    TLegend legend(0.55, 0.15, 0.90, 0.38);
    legend.SetBorderSize(0);
    legend.SetFillStyle(0);
    legend.SetTextSize(0.035);

    std::vector<int> colors = {kBlack, kRed + 1, kGreen + 2, kMagenta + 1,
                               kBlue + 1, kOrange + 7, kCyan + 1, kViolet + 7};
    size_t colorIndex = 0;

    for (const auto& kv : fractionByScale) {
        double scale = kv.first;
        const auto& values = kv.second;
        auto graph = std::make_unique<TGraphErrors>();
        int pointIndex = 0;
        const auto& errs = errorByScale[scale];
        for (size_t i = 0; i < cosCenters.size(); ++i) {
            if (i >= values.size()) {
                continue;
            }
            double y = values[i];
            if (std::isnan(y)) {
                continue;
            }
            double errY = (i < errs.size()) ? errs[i] : 0.0;
            double errX = (i < cosHalfWidths.size()) ? cosHalfWidths[i] : 0.0;
            graph->SetPoint(pointIndex, cosCenters[i], y);
            graph->SetPointError(pointIndex, errX, errY);
            ++pointIndex;
        }
        if (graph->GetN() == 0) {
            continue;
        }

        int color = colors[colorIndex % colors.size()];
        graph->SetMarkerColor(color);
        graph->SetLineColor(color);
        graph->SetMarkerStyle(20 + (colorIndex % 5));
        graph->SetMarkerSize(1.1);
        graph->SetLineWidth(2);

        legend.AddEntry(graph.get(), Form("scale %.2f", scale), "pl");
        multiGraph.Add(graph.get(), "P");
        graphs.push_back(std::move(graph));
        ++colorIndex;
    }

    multiGraph.Draw("A P");
    multiGraph.GetXaxis()->SetTitle("cos#theta_{HX}");
    multiGraph.GetYaxis()->SetTitle("Prompt fraction (%)");
    multiGraph.GetYaxis()->SetTitleOffset(1.2);
    multiGraph.GetYaxis()->SetRangeUser(0.0, 100.0);

    legend.Draw();

    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(42);
    latex.SetTextAlign(31);
    latex.SetTextSize(0.045);
    latex.DrawLatex(0.88, 0.26, "#bf{CMS} #it{Internal}");
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.88, 0.20, ptLabel.c_str());
    latex.DrawLatex(0.88, 0.15, centLabel.c_str());

    std::string canvasPath = outputBase + "/prompt_fraction_vs_cos_scales";
    canvas->SaveAs((canvasPath + ".png").c_str());
    canvas->SaveAs((canvasPath + ".pdf").c_str());

    auto optimalCanvas = std::make_unique<TCanvas>((combinedBinName + "_optimalPromptFracCos_c").c_str(), "", 900, 700);
    optimalCanvas->SetGrid();

    auto optimalGraph = std::make_unique<TGraphErrors>();
    double maxY = 0.0;
    int pointIndex = 0;
    for (size_t i = 0; i < cosCenters.size(); ++i) {
        if (i >= optimalPromptFractions.size()) {
            continue;
        }
        double frac = optimalPromptFractions[i];
        if (!std::isfinite(frac)) {
            continue;
        }
        double errY = (i < optimalPromptErrors.size()) ? optimalPromptErrors[i] : 0.0;
        double errX = (i < cosHalfWidths.size()) ? cosHalfWidths[i] : 0.0;
        optimalGraph->SetPoint(pointIndex, cosCenters[i], frac);
        optimalGraph->SetPointError(pointIndex, errX, errY);
        maxY = std::max(maxY, frac + errY);
        ++pointIndex;
    }

    if (optimalGraph->GetN() > 0) {
        optimalCanvas->cd();
        optimalGraph->SetTitle(";cos#theta_{HX};Prompt fraction (%)");
        optimalGraph->SetMarkerStyle(20);
        optimalGraph->SetMarkerSize(1.2);
        optimalGraph->SetLineWidth(2);
        optimalGraph->SetLineColor(kBlack);
        optimalGraph->Draw("AP");
        optimalGraph->GetXaxis()->SetTitle("cos#theta_{HX}");
        optimalGraph->GetYaxis()->SetTitle("Prompt fraction (%)");
        optimalGraph->GetYaxis()->SetTitleOffset(1.2);
        double yMax = std::min(110.0, std::max(50.0, maxY + 10.0));
        optimalGraph->GetYaxis()->SetRangeUser(0.0, yMax);

        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextAlign(31);
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.88, 0.26, "#bf{CMS} #it{Internal}");
        latex.SetTextSize(0.04);
        latex.DrawLatex(0.88, 0.20, ptLabel.c_str());
        latex.DrawLatex(0.88, 0.15, centLabel.c_str());

        TLatex scaleLatex;
        scaleLatex.SetTextFont(42);
        scaleLatex.SetTextSize(0.035);
        scaleLatex.SetTextAlign(21);
        scaleLatex.SetNDC(false);
        for (size_t i = 0; i < cosCenters.size(); ++i) {
            if (i >= optimalPromptFractions.size()) {
                continue;
            }
            double frac = optimalPromptFractions[i];
            if (!std::isfinite(frac)) {
                continue;
            }
            double errY = (i < optimalPromptErrors.size()) ? optimalPromptErrors[i] : 0.0;
            double yText = std::min(yMax - 1.0, frac + errY + 5.0);
            double scale = (i < optimalScales.size()) ? optimalScales[i] : std::numeric_limits<double>::quiet_NaN();
            if (!std::isfinite(scale)) {
                continue;
            }
            scaleLatex.DrawLatex(cosCenters[i], yText, Form("scale = %.2f", scale));
        }

        std::string optimalPath = outputBase + "/prompt_fraction_vs_cos_optimal";
        optimalCanvas->SaveAs((optimalPath + ".png").c_str());
        optimalCanvas->SaveAs((optimalPath + ".pdf").c_str());
    } else {
        std::cerr << "[DCAResolutionScanPromptFractionVsCos] No valid optimal points to plot." << std::endl;
    }
}
