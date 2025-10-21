#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGenericPdf.h"
#include "RooFitResult.h"
#include "RooPlot.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1.h"
#include <iostream>

using namespace RooFit;

// Function to fit DCA distribution with Error Function * Exponential
void dcafitter(const char* datasetName = "datasetHX") {
    const char* fileName = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_pp_pr_16Sep25_v1.root";
    // Open the ROOT file
    TFile* file = TFile::Open(fileName);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return;
    }

    // Load the RooDataSet
    RooDataSet* fullData = (RooDataSet*)file->Get(datasetName);
    if (!fullData) {
        std::cerr << "Error: Cannot find RooDataSet " << datasetName << " in file " << fileName << std::endl;
        file->Close();
        return;
    }

    std::cout << "Loaded " << fullData->numEntries() << " entries from " << fileName << std::endl;

    // Reduce the dataset with the specified cuts
    // matchGen_D1ancestorFlavor_ == 5 && matchGEN == 1 && dca3D >= 0 && dca3D <= 0.1
    RooDataSet* data = (RooDataSet*)fullData->reduce("matchGen_D1ancestorFlavor_ == 5 && matchGEN == 1 && dca3D >= 0 && dca3D <= 0.1");
    if (!data || data->numEntries() == 0) {
        std::cerr << "Error: No entries match the cuts or reduction failed." << std::endl;
        file->Close();
        return;
    }

    std::cout << "After reduction: " << data->numEntries() << " entries" << std::endl;

    // Define RooRealVar for DCA (assuming it's already in the dataset)
    RooRealVar* dca = (RooRealVar*)data->get()->find("dca3D");
    if (!dca) {
        std::cerr << "Error: Cannot find dca3D variable in the dataset." << std::endl;
        file->Close();
        return;
    }

    // Set range for DCA from 0 to 0.1
    dca->setRange("fitRange", 0, 0.1);

    // Define parameters for Error Function * Exponential
    RooRealVar mean("mean", "Mean", 0.01, -0.5, 0.5);
    RooRealVar sigma("sigma", "Sigma", 0.01, 0.001, 1);
    RooRealVar lambda("lambda", "Lambda", 0.1, 0, 100);

    // Define PDF: Error Function * Exponential
    // erf((x-mean)/sigma) * exp(-lambda * x)
    RooGenericPdf pdf("pdf", "Error Function * Exponential",
                      "TMath::Erf((@0-@1)/@2) * TMath::Exp(-@3*@0)",
                      RooArgList(*dca, mean, sigma, lambda));

    // Perform fit
    RooFitResult* fitResult = pdf.fitTo(*data, Range("fitRange"), Save());
    fitResult->Print();

    // Create plot
    RooPlot* frame = dca->frame(Range("fitRange"));
    data->plotOn(frame);
    pdf.plotOn(frame);

    TCanvas* canvas = new TCanvas("canvas", "DCA Fit", 800, 600);
    frame->Draw();
    canvas->SaveAs("dca_fit_result.png");

    // Clean up
    delete fitResult;
    delete data;  // Note: fullData is not deleted as it's owned by the file
    delete canvas;
    delete frame;
    file->Close();
}

