#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TStyle.h>

void drawplot() {
    // Open the ROOT file
    TFile *file = TFile::Open("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_PbPb_250102_v1.root");
    if (!file || file->IsZombie()) {
        printf("Error opening file\n");
        return;
    }

    // Get the tree from the file
    TTree *tree = (TTree*)file->Get("skimTreeFlat");
    if (!tree) {
        printf("Error getting tree\n");
        return;
    }

    // Create histograms for pT and mass
    TH1F *h_pT = new TH1F("h_pT", "pT Distribution; pT (GeV/c); Entries", 100, 0, 50);
    TH1F *h_mass = new TH1F("h_mass", "Mass Distribution; Mass (GeV/c^2); Entries", 100, 1.8, 2.2);

    // Apply cut and fill histograms
    tree->Draw("pT >> h_pT", "pTD2>0.8");
    tree->Draw("mass >> h_mass", "pTD2>0.8");

    // Create a canvas to draw the histograms
    TCanvas *c1 = new TCanvas("c1", "Distributions", 800, 600);
    c1->Divide(2, 1);

    // Draw pT histogram
    c1->cd(1);
    gStyle->SetOptStat(0); // Disable stat box
    h_pT->SetLineColor(kBlue);
    h_pT->Draw();

    // Draw mass histogram
    c1->cd(2);
    h_mass->SetLineColor(kRed);
    h_mass->Draw();

    // Save the canvas as an image
    c1->SaveAs("distributions.png");

    // Clean up
    delete h_pT;
    delete h_mass;
    delete c1;
    file->Close();
    delete file;
}