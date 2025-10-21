#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TAxis.h>
#include <vector>
#include <string>
#include <iostream>
#include <numeric> // For std::iota

// Include headers needed for reading fit results
#include "../fit/Opt.h" // Assuming FitOpt is defined here
#include <RooWorkspace.h>
#include <RooRealVar.h>

void DrawYieldUnroll() {
    // --- Configuration ---
    // Base path where the individual fit result directories are located
    const std::string fitResultBasePath = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/";
    // Subdirectory within the base path (if any, consistent with DrawSplotHist2D)
    const std::string subDir = "D0_Apr30/"; // Or adjust as needed
    const char* outputFileName = "unrolled_yield_from_files.png";
    const char* outputHistName = "h_yield_unrolled_files";
    const char* outputHistTitle = "Unrolled D0 Yield (from Swap1 files); Bin (pT, MVA); Raw Yield";

    std::vector<std::pair<double, double>> ptBins = {
        {2, 4},
        {4, 9},
        {9, 50}
    };
    // Define MVA bin edges explicitly, matching those used for fitting
    std::vector<double> mvaBinEdges = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    // Centrality bins (if needed for file path generation)
    std::vector<std::pair<double, double>> centBins = {
        {0, 90}
    };


    // --- Prepare Unrolled Histogram ---
    int nPtBins = ptBins.size();
    int nMvaBins = mvaBinEdges.size() - 1; // Number of bins is edges - 1
    int nTotalBins = nPtBins * nMvaBins;

    TH1D *h_unrolled = new TH1D(outputHistName, outputHistTitle, nTotalBins, 0, nTotalBins);
    h_unrolled->SetStats(0); // Disable statistics box
    TH1D *h_unrolleddata = new TH1D("h_yield_unrolled_files_data" , outputHistTitle, nTotalBins, 0, nTotalBins);
    h_unrolleddata->SetStats(0); // Disable statistics box

    // --- Fill Unrolled Histogram and Set Labels ---
    int currentBin = 1; // ROOT bin numbering starts from 1
    FitOpt D0opt; // Create FitOpt object

    for (int ipT = 0; ipT < nPtBins; ++ipT) {
        std::string ptLabel = std::to_string((int)ptBins[ipT].first) + "-" + std::to_string((int)ptBins[ipT].second) + " GeV";

        for (int imva = 0; imva < nMvaBins; ++imva) {
            double mvaLowEdge = mvaBinEdges[imva];
            double mvaUpEdge = mvaBinEdges[imva + 1];
            std::string mvaLabel = Form("%.3f-%.3f", mvaLowEdge, mvaUpEdge);

            // Configure FitOpt for this specific bin
            D0opt.pTMin = ptBins[ipT].first;
            D0opt.pTMax = ptBins[ipT].second;
            D0opt.mvaMin = mvaLowEdge; // Use the lower edge to identify the bin's fit file
            // Assuming only one centrality bin for now
            D0opt.centMin = centBins[0].first;
            D0opt.centMax = centBins[0].second;
            D0opt.D0MCDefault(); // Set defaults, including relative outputDir and outputFile names

            // Construct the full path to the ROOT file
            // std::string fitResultFilePath = fitResultBasePath  + D0opt.outputDir + "/" +subDir +"/" +D0opt.outputFile;
            std::string fitResultFilePath = fitResultBasePath  + D0opt.outputMCDir + "/" +subDir +"/" +D0opt.outputMCSwap0File;

            double yield = 0.0;
            double error = 0.0;

            // --- Load Yield from individual file ---
            TFile *fitFile = TFile::Open(fitResultFilePath.c_str(), "read");
            if (!fitFile || fitFile->IsZombie()) {
                std::cerr << "Warning: Could not open file: " << fitResultFilePath << " for bin (pT=" << ipT << ", mva=" << imva << "). Setting yield=0." << std::endl;
                // Keep yield and error as 0
            } else {
                RooWorkspace* ws = (RooWorkspace*)fitFile->Get("ws_D0");
                if (!ws) {
                    std::cerr << "Warning: Could not get workspace 'ws_D0' from file: " << fitResultFilePath << ". Setting yield=0." << std::endl;
                } else {
                    RooRealVar* nSig = (RooRealVar*)ws->var("nsig");
                    RooRealVar* sigma1_swap0 = (RooRealVar*)ws->var("sigma1_Swap0");
                    RooRealVar* sigma2_swap0 = (RooRealVar*)ws->var("sigma2_Swap0");
                    RooRealVar* frac_swap0 = (RooRealVar*)ws->var("frac_Swap0");
                    if (!nSig) {
                        std::cerr << "Warning: Could not get variable 'nsig' from workspace in file: " << fitResultFilePath << ". Setting yield=0." << std::endl;
                    } else {
                        yield = nSig->getVal();
                        yield = sigma1_swap0->getVal() *frac_swap0->getVal() + sigma2_swap0->getVal() * (1 - frac_swap0->getVal());
                        error = nSig->getError(); // Get the error associated with the variable
                        error = 0 ;
                    }
                    // Workspace and variable are owned by the file, no need to delete ws or nSig here
                }
                fitFile->Close();
                delete fitFile; // Close and delete the file pointer
            }

            // Set content and error in the 1D unrolled histogram
            h_unrolled->SetBinContent(currentBin, yield);
            h_unrolled->SetBinError(currentBin, error);

            // Set bin label
            // if (imva == 0) {
            //      h_unrolled->GetXaxis()->SetBinLabel(currentBin, (ptLabel + ", MVA:" + mvaLabel).c_str());
            // } else if (imva == nMvaBins / 2) { // Label middle MVA bin too?
            //      h_unrolled->GetXaxis()->SetBinLabel(currentBin, mvaLabel.c_str());
            // }

            currentBin++;
        }
    }
    
    currentBin = 1; // ROOT bin numbering starts from 1
    for (int ipT = 0; ipT < nPtBins; ++ipT) {
        std::string ptLabel = std::to_string((int)ptBins[ipT].first) + "-" + std::to_string((int)ptBins[ipT].second) + " GeV";

        for (int imva = 0; imva < nMvaBins; ++imva) {
            double mvaLowEdge = mvaBinEdges[imva];
            double mvaUpEdge = mvaBinEdges[imva + 1];
            std::string mvaLabel = Form("%.3f-%.3f", mvaLowEdge, mvaUpEdge);

            // Configure FitOpt for this specific bin
            D0opt.pTMin = ptBins[ipT].first;
            D0opt.pTMax = ptBins[ipT].second;
            D0opt.mvaMin = mvaLowEdge; // Use the lower edge to identify the bin's fit file
            // Assuming only one centrality bin for now
            D0opt.centMin = centBins[0].first;
            D0opt.centMax = centBins[0].second;
            D0opt.D0MCDefault(); // Set defaults, including relative outputDir and outputFile names

            // Construct the full path to the ROOT file
            std::string fitResultFilePath = fitResultBasePath  + D0opt.outputDir + "/" +subDir +"/" +D0opt.outputFile;
            // std::string fitResultFilePath = fitResultBasePath  + D0opt.outputMCDir + "/" +subDir +"/" +D0opt.outputMCSwap0File;

            double yield = 0.0;
            double error = 0.0;

            // --- Load Yield from individual file ---
            TFile *fitFile = TFile::Open(fitResultFilePath.c_str(), "read");
            if (!fitFile || fitFile->IsZombie()) {
                std::cerr << "Warning: Could not open file: " << fitResultFilePath << " for bin (pT=" << ipT << ", mva=" << imva << "). Setting yield=0." << std::endl;
                // Keep yield and error as 0
            } else {
                RooWorkspace* ws = (RooWorkspace*)fitFile->Get("ws_D0");
                if (!ws) {
                    std::cerr << "Warning: Could not get workspace 'ws_D0' from file: " << fitResultFilePath << ". Setting yield=0." << std::endl;
                } else {
                    RooRealVar* nSig = (RooRealVar*)ws->var("nsig");
                    RooRealVar* sigma1_swap0 = (RooRealVar*)ws->var("sigma1_Swap0");
                    RooRealVar* sigma2_swap0 = (RooRealVar*)ws->var("sigma2_Swap0");
                    RooRealVar* frac_swap0 = (RooRealVar*)ws->var("frac_Swap0");
                    if (!nSig) {
                        std::cerr << "Warning: Could not get variable 'nsig' from workspace in file: " << fitResultFilePath << ". Setting yield=0." << std::endl;
                    } else {
                        yield = nSig->getVal();
                        yield = sigma1_swap0->getVal() *frac_swap0->getVal() + sigma2_swap0->getVal() * (1 - frac_swap0->getVal());
                        error = nSig->getError(); // Get the error associated with the variable
                        error = 0 ;
                    }
                    // Workspace and variable are owned by the file, no need to delete ws or nSig here
                }
                fitFile->Close();
                delete fitFile; // Close and delete the file pointer
            }

            // Set content and error in the 1D unrolled histogram
            h_unrolleddata->SetBinContent(currentBin, yield);
            h_unrolleddata->SetBinError(currentBin, error);

            // Set bin label
            // if (imva == 0) {
            //      h_unrolled->GetXaxis()->SetBinLabel(currentBin, (ptLabel + ", MVA:" + mvaLabel).c_str());
            // } else if (imva == nMvaBins / 2) { // Label middle MVA bin too?
            //      h_unrolled->GetXaxis()->SetBinLabel(currentBin, mvaLabel.c_str());
            // }

            currentBin++;
        }
    }

    // Adjust label appearance
    h_unrolled->GetXaxis()->SetLabelSize(0.025); // Smaller font size
    h_unrolled->GetXaxis()->LabelsOption("v"); // Vertical labels

    // --- Draw and Save ---
    gStyle->SetOptStat(0);
    TCanvas *canvas = new TCanvas("c_unroll_files", "Unrolled D0 Yield (from files)", 1200, 600);
    canvas->SetBottomMargin(0.25); // Increase bottom margin for labels
    h_unrolled->GetYaxis()->SetRangeUser(0, h_unrolled->GetMaximum() * 1.2); // Set y-axis range
    h_unrolled->Draw("E1"); // Draw with error bars
    h_unrolleddata->SetLineColor(kBlack);
    h_unrolleddata->SetMarkerColor(kBlack);
    h_unrolleddata->SetMarkerStyle(20);
    h_unrolleddata->SetMarkerSize(1.0);
    h_unrolleddata->Draw("E1 same"); // Draw the data histogram on top
    TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->AddEntry(h_unrolled, "Unrolled MC", "l");
    legend->AddEntry(h_unrolleddata, "Unrolled Data", "p");
    canvas->Update(); // Ensure changes are reflected
    double y_min = gPad->GetUymin(); // Get minimum y from the current pad
    double y_max = gPad->GetUymax(); // Get maximum y from the current pad

    std::vector<TLine*> lines; // Keep track of lines to delete later if needed
    for (int ipT = 0; ipT < nPtBins - 1; ++ipT) { // Loop up to nPtBins-1 to draw lines between bins
        double x_boundary = (ipT + 1) * nMvaBins; // Calculate x position for the line
        TLine *line = new TLine(x_boundary, y_min, x_boundary, y_max);
        line->SetLineColor(kRed); // Set line color (e.g., red)
        line->SetLineStyle(2);    // Set line style (e.g., dashed)
        line->SetLineWidth(2);    // Set line width
        line->Draw("same");       // Draw the line on the same canvas
        lines.push_back(line);    // Store the line pointer (optional)
    }

    canvas->RedrawAxis(); // Redraw axis on top of lines if needed
    canvas->SaveAs(outputFileName);

    // --- Cleanup ---
    delete canvas;
    // delete h_unrolled; // Optional: delete if not saving to a file

    std::cout << "Unrolled histogram saved to " << outputFileName << std::endl;
}