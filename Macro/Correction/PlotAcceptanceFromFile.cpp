#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TMath.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

namespace {
std::unique_ptr<TH1D> makeEfficiencyHist(const TH1D* passed, const TH1D* total,
                                         const std::string& name) {
    if (!passed || !total) {
        std::cerr << "[makeEfficiencyHist] nullptr hist provided" << std::endl;
        return nullptr;
    }

    auto hist = std::unique_ptr<TH1D>(static_cast<TH1D*>(passed->Clone(name.c_str())));
    hist->Reset("ICESM");
    hist->Sumw2(false);

    const int nBins = total->GetNbinsX();
    for (int i = 1; i <= nBins; ++i) {
        const double den = total->GetBinContent(i);
        const double num = passed->GetBinContent(i);
        if (den <= 0.) {
            hist->SetBinContent(i, 0.);
            hist->SetBinError(i, 0.);
            continue;
        }
        const double value = num / den;
        const double err = std::sqrt(value * (1.0 - value) / den);
        hist->SetBinContent(i, value);
        hist->SetBinError(i, err);
    }
    return hist;
}

std::unique_ptr<TH2D> makeEfficiencyHist2D(const TH2D* passed, const TH2D* total,
                                           const std::string& name) {
    if (!passed || !total) {
        std::cerr << "[makeEfficiencyHist2D] nullptr hist provided" << std::endl;
        return nullptr;
    }

    auto hist = std::unique_ptr<TH2D>(static_cast<TH2D*>(passed->Clone(name.c_str())));
    hist->Reset("ICESM");
    const int nBinsX = total->GetNbinsX();
    const int nBinsY = total->GetNbinsY();

    for (int ix = 1; ix <= nBinsX; ++ix) {
        for (int iy = 1; iy <= nBinsY; ++iy) {
            const double den = total->GetBinContent(ix, iy);
            const double num = passed->GetBinContent(ix, iy);
            if (den <= 0.) {
                hist->SetBinContent(ix, iy, 0.);
                hist->SetBinError(ix, iy, 0.);
                continue;
            }
            const double value = num / den;
            const double err = std::sqrt(value * (1.0 - value) / den);
            hist->SetBinContent(ix, iy, value);
            hist->SetBinError(ix, iy, err);
        }
    }
    return hist;
}

void styleHist(TH1D* hist, Color_t color, int marker)
{
    if (!hist) return;
    hist->SetMarkerStyle(marker);
    hist->SetMarkerColor(color);
    hist->SetLineColor(color);
    hist->SetMarkerSize(1.15);
}
}

void PlotAcceptanceFromFile(const char* inputFile,
                            const char* ptPromptName = "pt_pr_pass",
                            const char* ptPromptDen  = "pt_pr",
                            const char* ptFeedName   = "pt_np_pass",
                            const char* ptFeedDen    = "pt_np",
                            const char* cosPromptName = "cos_pr_pass",
                            const char* cosPromptDen  = "cos_pr",
                            const char* cosFeedName   = "cos_np_pass",
                            const char* cosFeedDen    = "cos_np",
                            const char* ptCosPromptName = "pt_cos_pr_pass",
                            const char* ptCosPromptDen  = "pt_cos_pr",
                            const char* ptCosFeedName   = "pt_cos_np_pass",
                            const char* ptCosFeedDen    = "pt_cos_np",
                            const char* outTag        = "DStar_ppRef_PU_NoCorrection_Sep20_pTGrand_1p0_EtaGrand_2p4_pTD2_0p4")
{
    if (!inputFile) {
        std::cerr << "[PlotAcceptanceFromFile] inputFile is null" << std::endl;
        return;
    }

    std::unique_ptr<TFile> file(TFile::Open(inputFile, "READ"));
    if (!file || file->IsZombie()) {
        std::cerr << "Failed to open file: " << inputFile << std::endl;
        return;
    }

    auto ptPromptPass = dynamic_cast<TH1D*>(file->Get(ptPromptName));
    auto ptPromptTot  = dynamic_cast<TH1D*>(file->Get(ptPromptDen));
    auto ptFeedPass   = dynamic_cast<TH1D*>(file->Get(ptFeedName));
    auto ptFeedTot    = dynamic_cast<TH1D*>(file->Get(ptFeedDen));
    auto cosPromptPass = dynamic_cast<TH1D*>(file->Get(cosPromptName));
    auto cosPromptTot  = dynamic_cast<TH1D*>(file->Get(cosPromptDen));
    auto cosFeedPass   = dynamic_cast<TH1D*>(file->Get(cosFeedName));
    auto cosFeedTot    = dynamic_cast<TH1D*>(file->Get(cosFeedDen));
    auto ptCosPromptPass = dynamic_cast<TH2D*>(file->Get(ptCosPromptName));
    auto ptCosPromptTot  = dynamic_cast<TH2D*>(file->Get(ptCosPromptDen));
    auto ptCosFeedPass   = dynamic_cast<TH2D*>(file->Get(ptCosFeedName));
    auto ptCosFeedTot    = dynamic_cast<TH2D*>(file->Get(ptCosFeedDen));

    if (!ptPromptPass || !ptPromptTot || !ptFeedPass || !ptFeedTot ||
        !cosPromptPass || !cosPromptTot || !cosFeedPass || !cosFeedTot ||
        !ptCosPromptPass || !ptCosPromptTot || !ptCosFeedPass || !ptCosFeedTot) {
        std::cerr << "One or more histograms are missing in the file." << std::endl;
        return;
    }

    auto ptPromptEff = makeEfficiencyHist(ptPromptPass, ptPromptTot, "pt_prompt_eff");
    auto ptFeedEff   = makeEfficiencyHist(ptFeedPass, ptFeedTot, "pt_feed_eff");
    auto cosPromptEff = makeEfficiencyHist(cosPromptPass, cosPromptTot, "cos_prompt_eff");
    auto cosFeedEff   = makeEfficiencyHist(cosFeedPass, cosFeedTot, "cos_feed_eff");
    auto ptCosPromptEff = makeEfficiencyHist2D(ptCosPromptPass, ptCosPromptTot, "pt_cos_prompt_eff");
    auto ptCosFeedEff   = makeEfficiencyHist2D(ptCosFeedPass, ptCosFeedTot, "pt_cos_feed_eff");

    if (!ptPromptEff || !ptFeedEff || !cosPromptEff || !cosFeedEff ||
        !ptCosPromptEff || !ptCosFeedEff) {
        std::cerr << "Failed to build efficiency histograms." << std::endl;
        return;
    }

    gStyle->SetOptStat(0);
    styleHist(ptPromptEff.get(), kRed, 20);
    styleHist(ptFeedEff.get(), kBlue + 1, 21);
    styleHist(cosPromptEff.get(), kRed, 20);
    styleHist(cosFeedEff.get(), kBlue + 1, 21);

    TCanvas canvas("cPtCos1D_out", "A\\times\\epsilon vs pT and cos", 1200, 500);
    canvas.Divide(2, 1);

    canvas.cd(1);
    gPad->SetLeftMargin(0.14);
    gPad->SetBottomMargin(0.12);
    ptPromptEff->SetTitle(";p_{T} (GeV/c);A\\times\\epsilon");
    ptPromptEff->GetYaxis()->SetRangeUser(0., 1.2 * std::max(ptPromptEff->GetMaximum(), ptFeedEff->GetMaximum()));
    ptPromptEff->Draw("P E1");
    ptFeedEff->Draw("P E1 same");
    {
        TLegend *leg = new TLegend(0.60, 0.15, 0.88, 0.33);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->AddEntry(ptPromptEff.get(), "prompt D^{*}", "lp");
        leg->AddEntry(ptFeedEff.get(), "B #rightarrow D^{*}", "lp");
        leg->Draw();
    }

    canvas.cd(2);
    gPad->SetLeftMargin(0.14);
    gPad->SetBottomMargin(0.12);
    cosPromptEff->SetTitle(";|cos#theta|;A\\times\\epsilon");
    cosPromptEff->GetYaxis()->SetRangeUser(0., 1.2 * std::max(cosPromptEff->GetMaximum(), cosFeedEff->GetMaximum()));
    cosPromptEff->Draw("P E1");
    cosFeedEff->Draw("P E1 same");
    {
        auto leg = new TLegend(0.60, 0.15, 0.88, 0.33);
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        leg->AddEntry(cosPromptEff.get(), "prompt D^{*}", "lp");
        leg->AddEntry(cosFeedEff.get(), "B #rightarrow D^{*}", "lp");
        leg->Draw();
    }

    const std::string baseName1D = std::string("distributions_pt_cos1d_") + outTag;
    canvas.SaveAs((baseName1D + ".pdf").c_str());
    canvas.SaveAs((baseName1D + ".png").c_str());

    TCanvas canvas2D("cPtCos2D_out", "Acceptance maps", 1200, 500);
    canvas2D.Divide(2, 1);
    canvas2D.cd(1);
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.16);
    gPad->SetBottomMargin(0.12);
    ptCosPromptEff->SetTitle("prompt D^{*};p_{T} (GeV/c);|cos#theta|;A\\times\\epsilon");
    ptCosPromptEff->Draw("colz");

    canvas2D.cd(2);
    gPad->SetLeftMargin(0.12);
    gPad->SetRightMargin(0.16);
    gPad->SetBottomMargin(0.12);
    ptCosFeedEff->SetTitle("B #rightarrow D^{*};p_{T} (GeV/c);|cos#theta|;A\\times\\epsilon");
    ptCosFeedEff->Draw("colz");

    const std::string baseName2D = std::string("distributions_pt_cos2d_") + outTag;
    canvas2D.SaveAs((baseName2D + ".pdf").c_str());
    canvas2D.SaveAs((baseName2D + ".png").c_str());
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input-file> [out-tag]" << std::endl;
        return 1;
    }
    const char* tag = (argc > 2) ? argv[2] : "DStar_ppRef_PU_NoCorrection_Sep20_pTGrand_1p0_EtaGrand_2p4_pTD2_0p4";
    PlotAcceptanceFromFile(argv[1],
                           "pt_pr_pass", "pt_pr",
                           "pt_np_pass", "pt_np",
                           "cos_pr_pass", "cos_pr",
                           "cos_np_pass", "cos_np",
                           "pt_cos_pr_pass", "pt_cos_pr",
                           "pt_cos_np_pass", "pt_cos_np",
                           tag);
    return 0;
}
