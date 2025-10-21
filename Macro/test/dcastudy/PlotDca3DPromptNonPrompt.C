// PlotDca3DPromptNonPrompt.C
// Draws normalized dca3D distributions for prompt and nonprompt on one canvas
// - Tree name: skimTreeFlat
// - Branches used: dca3D, matchGEN, matchGen_D1ancestorFlavor_
// - Prompt:  matchGEN==1 && (matchGen_D1ancestorFlavor_==2 || matchGen_D1ancestorFlavor_==4)
// - Nonprompt: matchGEN==1 && matchGen_D1ancestorFlavor_==5

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TSystem.h"
#include "TString.h"
#include "TStyle.h"
#include <iostream>

void PlotDca3DPromptNonPrompt(
    // const char* infile = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/0908/flatSkimForBDT_DStar_Dstar_NonPrompt_MC_08Sep25_0_.root",
    const char* infile = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/0908/flatSkimForBDT_DStar_Dstar_Prompt_MC_08Sep25_0_.root",
    const char* treename = "skimTreeFlat",
    const char* outdir = "plots_dca_prompt_nonprompt",
    int nbins = 50,
    double xmin = 0.0,
    double xmax = 0.2)
{
    gStyle->SetOptStat(0);

    // Open input file
    std::unique_ptr<TFile> fin(TFile::Open(infile, "READ"));
    if (!fin || fin->IsZombie()) {
        std::cerr << "[Error] Cannot open input file: " << infile << std::endl;
        return;
    }

    // Get tree
    TTree* tree = dynamic_cast<TTree*>(fin->Get(treename));
    if (!tree) {
        std::cerr << "[Error] Tree not found: " << treename << std::endl;
        fin->ls();
        return;
    }

    // Sanity check branches
    if (!tree->GetBranch("dca3D") || !tree->GetBranch("matchGEN") || !tree->GetBranch("matchGen_D1ancestorFlavor_")) {
        std::cerr << "[Error] Required branches missing. Expecting 'dca3D', 'matchGEN', 'matchGen_D1ancestorFlavor_'." << std::endl;
        tree->Print();
        return;
    }

    // Prepare output directory
    gSystem->mkdir(outdir, kTRUE);

    // Define histograms
    TH1F* hPrompt = new TH1F("hPrompt", ";dca3D (cm);Normalized entries", nbins, xmin, xmax);
    TH1F* hNonPrompt = new TH1F("hNonPrompt", ";dca3D (cm);Normalized entries", nbins, xmin, xmax);
    hPrompt->SetLineColor(kRed+1);      hPrompt->SetLineWidth(3);
    hNonPrompt->SetLineColor(kBlue+1);  hNonPrompt->SetLineWidth(3);

    // Define selections
    TCut cutPrompt    = "matchGEN==1 && (matchGen_D1ancestorFlavor_==2 || matchGen_D1ancestorFlavor_==4)";
    TCut cutNonPrompt = "matchGEN==1 && matchGen_D1ancestorFlavor_==5";

    // Fill histograms using TTree::Draw (fast and simple)
    tree->Draw(Form("dca3D>>%s", hPrompt->GetName()), cutPrompt, "goff");
    tree->Draw(Form("dca3D>>%s", hNonPrompt->GetName()), cutNonPrompt, "goff");

    // Normalize (unit area)
    if (hPrompt->Integral() > 0)    hPrompt->Scale(1.0 / hPrompt->Integral(), "width");
    if (hNonPrompt->Integral() > 0) hNonPrompt->Scale(1.0 / hNonPrompt->Integral(), "width");

    // Determine y-axis max
    double ymax = std::max(hPrompt->GetMaximum(), hNonPrompt->GetMaximum());
    hPrompt->SetMaximum(ymax * 1.25);

    // Draw on one canvas
    TCanvas* c = new TCanvas("cDCA", "DCA3D Prompt vs Nonprompt", 900, 700);
    hPrompt->Draw("HIST");
    hNonPrompt->Draw("HIST SAME");

    // Legend
    TLegend* leg = new TLegend(0.60, 0.70, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(hPrompt,    "Prompt (flavor 2 or 4)", "l");
    leg->AddEntry(hNonPrompt, "Nonprompt (flavor 5)",    "l");
    leg->Draw();

    // Save
    TString outPDF = TString::Format("%s/dca3D_prompt_vs_nonprompt_promptsample.pdf", outdir);
    TString outPNG = TString::Format("%s/dca3D_prompt_vs_nonprompt_promptsample.png", outdir);
    c->SaveAs(outPDF);
    c->SaveAs(outPNG);

    std::cout << "[Done] Saved: " << outPDF << " and " << outPNG << std::endl;
}

