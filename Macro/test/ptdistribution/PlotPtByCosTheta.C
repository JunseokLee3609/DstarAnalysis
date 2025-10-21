// PlotPtByCosTheta.C
// - Loads datasetHX from given MC and Data RDS files
// - Splits by |cosThetaHX| bins [0,0.2,0.4,0.6,0.8,1]
// - MC: for each bin, overlays pt distributions for prompt (matchGen_D1ancestorFlavor_!=5) and nonprompt (==5) with matchGEN==1
// - Data: for each bin, draws pt distribution

#include "TFile.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TLegend.h"
#include "TSystem.h"
#include "TString.h"
#include "TROOT.h"
#include "TDirectory.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooArgSet.h"
#include <memory>
#include <vector>
#include <iostream>

using namespace RooFit;

static RooDataSet* LoadDataSet(const char* file, const char* dsname) {
    std::unique_ptr<TFile> f(TFile::Open(file, "READ"));
    if (!f || f->IsZombie()) {
        std::cerr << "[Error] Cannot open file: " << file << std::endl;
        return nullptr;
    }
    RooDataSet* ds = nullptr;
    f->GetObject(dsname, ds);
    if (!ds) {
        std::cerr << "[Error] Dataset '" << dsname << "' not found in file: " << file << std::endl;
        f->ls();
        return nullptr;
    }
    // Clone to detach from file (including internal storage)
    TDirectory* savdir = gDirectory;
    gROOT->cd();
    RooDataSet* dsClone = dynamic_cast<RooDataSet*>(ds->Clone(Form("%s_clone", dsname)));
    if (savdir) savdir->cd();
    if (!dsClone) {
        std::cerr << "[Error] Failed to clone dataset '" << dsname << "' from file: " << file << std::endl;
        return nullptr;
    }
    dsClone->SetName(Form("%s_from_%s", dsname, gSystem->BaseName(file)));
    dsClone->SetTitle(dsClone->GetName());
    return dsClone; // caller owns the clone
}

static TH1* MakePtHist(RooDataSet* ds, const char* hname, int nbins = 50, double xmin = 0.0, double xmax = 50.0) {
    if (!ds) return nullptr;
    RooArgSet* vars = (RooArgSet*)ds->get();
    if (!vars) return nullptr;
    RooRealVar* pt = dynamic_cast<RooRealVar*>(vars->find("pT"));
    if (!pt) {
        std::cerr << "[Error] Variable 'pT' not found in dataset." << std::endl;
        return nullptr;
    }
    TH1* h = ds->createHistogram(hname, *pt, Binning(nbins, xmin, xmax));
    return h;
}

void PlotPtByCosTheta(
    const char* mcfile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_noPreselectionCut_ppRef_Aug18_v1.root",
    const char* datafile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_ppRef_Aug01_v1.root",
    const char* outdir = "plots")
{
    // Prepare output directory
    if (gSystem->AccessPathName(outdir)) gSystem->MakeDirectory(outdir);

    // Load datasets
    std::unique_ptr<RooDataSet> dsMC(LoadDataSet(mcfile, "datasetHX"));
    std::unique_ptr<RooDataSet> dsData(LoadDataSet(datafile, "datasetHX"));
    if (!dsMC) {
        std::cerr << "[Error] MC dataset not available. Aborting." << std::endl;
        return;
    }
    if (!dsData) {
        std::cerr << "[Warn] Data dataset not available. Data plots will be skipped." << std::endl;
    }

    // CosTheta bins (apply to abs(cosThetaHX))
    std::vector<double> edges = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};

    // Colors per cosTheta bin
    int colors[] = {kBlack, kRed+1, kBlue+1, kGreen+2, kOrange+1};
    auto nbins = edges.size() - 1;

    // MC Prompt: overlay all cos bins in one canvas
    std::vector<std::unique_ptr<TH1>> hPrompt; hPrompt.reserve(nbins);
    double maxPrompt = 0.0;
    for (size_t i = 0; i < nbins; ++i) {
        double lo = edges[i];
        double hi = edges[i + 1];
        TString cut = Form("matchGEN == 1 && abs(cosThetaHX) >= %g && abs(cosThetaHX) < %g && matchGen_D1ancestorFlavor_ != 5", lo, hi);
        std::unique_ptr<RooDataSet> subset(dynamic_cast<RooDataSet*>(dsMC->reduce(cut)));
        TString hname = Form("h_pt_MC_prompt_%g_%g", lo, hi);
        auto h = std::unique_ptr<TH1>(MakePtHist(subset.get(), hname));
        if (!h) { hPrompt.push_back(nullptr); continue; }
        h->SetLineColor(colors[i % 5]);
        h->SetLineWidth(2);
        maxPrompt = std::max(maxPrompt, h->GetMaximum());
        hPrompt.push_back(std::move(h));
    }
    if (!hPrompt.empty()) {
        std::unique_ptr<TCanvas> cP(new TCanvas("c_MC_prompt_all", "MC Prompt pt by |cosThetaHX|", 900, 700));
        std::unique_ptr<TLegend> legP(new TLegend(0.58, 0.65, 0.88, 0.88));
        bool first = true;
        for (size_t i = 0; i < nbins; ++i) {
            double lo = edges[i];
            double hi = edges[i + 1];
            TH1* h = hPrompt[i].get();
            if (!h) continue;
            h->SetMaximum(maxPrompt * 1.25);
            h->GetXaxis()->SetTitle("p_{T} [GeV]");
            h->GetYaxis()->SetTitle("Entries");
            h->SetTitle("MC Prompt: p_{T} distributions by |cos#theta_{HX}| bin");
            if (first) { h->Draw("HIST"); first = false; }
            else { h->Draw("HIST SAME"); }
            legP->AddEntry(h, Form("|cos#theta| [%0.1f,%0.1f)", lo, hi), "l");
        }
        legP->Draw();
        TString outP = Form("%s/pt_MC_prompt_allCos.pdf", outdir);
        cP->SaveAs(outP);
    }

    // MC Nonprompt: overlay all cos bins in one canvas
    std::vector<std::unique_ptr<TH1>> hNonP; hNonP.reserve(nbins);
    double maxNonP = 0.0;
    for (size_t i = 0; i < nbins; ++i) {
        double lo = edges[i];
        double hi = edges[i + 1];
        TString cut = Form("matchGEN == 1 && abs(cosThetaHX) >= %g && abs(cosThetaHX) < %g && matchGen_D1ancestorFlavor_ == 5", lo, hi);
        std::unique_ptr<RooDataSet> subset(dynamic_cast<RooDataSet*>(dsMC->reduce(cut)));
        TString hname = Form("h_pt_MC_nonprompt_%g_%g", lo, hi);
        auto h = std::unique_ptr<TH1>(MakePtHist(subset.get(), hname));
        if (!h) { hNonP.push_back(nullptr); continue; }
        h->SetLineColor(colors[i % 5]);
        h->SetLineWidth(2);
        maxNonP = std::max(maxNonP, h->GetMaximum());
        hNonP.push_back(std::move(h));
    }
    if (!hNonP.empty()) {
        std::unique_ptr<TCanvas> cNP(new TCanvas("c_MC_nonprompt_all", "MC Nonprompt pt by |cosThetaHX|", 900, 700));
        std::unique_ptr<TLegend> legNP(new TLegend(0.58, 0.65, 0.88, 0.88));
        bool first = true;
        for (size_t i = 0; i < nbins; ++i) {
            double lo = edges[i];
            double hi = edges[i + 1];
            TH1* h = hNonP[i].get();
            if (!h) continue;
            h->SetMaximum(maxNonP * 1.25);
            h->GetXaxis()->SetTitle("p_{T} [GeV]");
            h->GetYaxis()->SetTitle("Entries");
            h->SetTitle("MC Nonprompt: p_{T} distributions by |cos#theta_{HX}| bin");
            if (first) { h->Draw("HIST"); first = false; }
            else { h->Draw("HIST SAME"); }
            legNP->AddEntry(h, Form("|cos#theta| [%0.1f,%0.1f)", lo, hi), "l");
        }
        legNP->Draw();
        TString outNP = Form("%s/pt_MC_nonprompt_allCos.pdf", outdir);
        cNP->SaveAs(outNP);
    }

    // Data: overlay all cos bins in one canvas
    if (dsData) {
        std::vector<std::unique_ptr<TH1>> hData; hData.reserve(nbins);
        double maxData = 0.0;
        for (size_t i = 0; i < nbins; ++i) {
            double lo = edges[i];
            double hi = edges[i + 1];
            TString cut = Form("abs(cosThetaHX) >= %g && abs(cosThetaHX) < %g", lo, hi);
            std::unique_ptr<RooDataSet> subset(dynamic_cast<RooDataSet*>(dsData->reduce(cut)));
            TString hname = Form("h_pt_Data_%g_%g", lo, hi);
            auto h = std::unique_ptr<TH1>(MakePtHist(subset.get(), hname));
            if (!h) { hData.push_back(nullptr); continue; }
            h->SetLineColor(colors[i % 5]);
            h->SetLineWidth(2);
            maxData = std::max(maxData, h->GetMaximum());
            hData.push_back(std::move(h));
        }
        std::unique_ptr<TCanvas> cD(new TCanvas("c_Data_all", "Data pt by |cosThetaHX|", 900, 700));
        std::unique_ptr<TLegend> legD(new TLegend(0.58, 0.65, 0.88, 0.88));
        bool first = true;
        for (size_t i = 0; i < nbins; ++i) {
            double lo = edges[i];
            double hi = edges[i + 1];
            TH1* h = hData[i].get();
            if (!h) continue;
            h->SetMaximum(maxData * 1.25);
            h->GetXaxis()->SetTitle("p_{T} [GeV]");
            h->GetYaxis()->SetTitle("Entries");
            h->SetTitle("Data: p_{T} distributions by |cos#theta_{HX}| bin");
            if (first) { h->Draw("HIST"); first = false; }
            else { h->Draw("HIST SAME"); }
            legD->AddEntry(h, Form("|cos#theta| [%0.1f,%0.1f)", lo, hi), "l");
        }
        legD->Draw();
        TString outD = Form("%s/pt_Data_allCos.pdf", outdir);
        cD->SaveAs(outD);
    }

    std::cout << "[Done] Saved overlaid pt distributions (prompt/nonprompt/data) to: " << outdir << std::endl;
}
