#include "TCanvas.h"
#include "RooPlot.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "RooWorkspace.h"
#include "TFile.h"
// ...existing code...

void drawMassDistribution(RooDataSet* data) {
    // Define the mass variable
    RooRealVar mass("mass", "mass", 1.9, 2.1);

    // Create a frame for the mass variable
    RooPlot* massFrame = mass.frame();

    // Plot the data on the frame
    data->plotOn(massFrame);

    // Create a canvas to draw the plot
    TCanvas* canvas = new TCanvas("canvas", "Mass Distribution", 800, 600);

    // Draw the frame on the canvas
    massFrame->Draw();

    // Save the canvas as an image
    canvas->SaveAs("mass_distribution.png");
}

// ...existing code...

void DrawDistributionFast() {
    // ...existing code...

    // Load the data from the ROOT file
    TFile* file = TFile::Open("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_PbPb_250102_v1.root");
    RooWorkspace* workspace = (RooWorkspace*)file->Get("workspace");
    RooDataSet* data = (RooDataSet*)workspace->data("dataset");
    std::vector<std::pair<double, double>> pTBins = {
        {4.0, 5.0},
        {5.0, 6.0},
        {6.0, 7.0},
        {7.0, 8.0},
        {8.0,9.0},
        {9.0,10.0},
        {10.0,12.0},
        {12.0,16.0},
        {16.0,20.0},
        {20.0,30.0},
        {30.0,50.0},
        {50.0,100.0}
    }

    // Draw the mass distribution
    for(auto pTBin : pTBins) {
        RooDataSet* reduced_data = (RooDataSet*)data->reduce(Form("pT>%f && pT < %f && -1 < eta && eta < 1 ", pTBin.first, pTBin.second));
        drawMassDistribution(reduced_data);
        delete reduced_data;
    }
    // ...existing code...
}
