#include "../interface/simpleDMC.h"
#include "../fit/Opt.h"
#include <iostream>
#include <vector>
#include <map>
#include <cmath> // For std::abs

#include "TFile.h"
#include "TTree.h"
#include "TH3D.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TString.h" // For Form

// Include RooFit headers
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooFormulaVar.h" // Although we might not use it directly for the average
#include "RooGlobalFunc.h" // For RooFit namespace stuff like RooFit::Cut()

// Assuming DataFormat::simpleDStarMCTreeflat and its setTree method are defined in simpleDMC.h
// If not, we'll load branches directly. Let's assume we load directly for clarity.
// #include "../interface/simpleDMC.h"
// #include "../fit/Opt.h" // Seems unused in the original snippet

// Using namespace RooFit; // Common practice, but be mindful of potential conflicts

// Keep the original efficiency lookup function
float getEfficiencyVal(double pt, double y, double mva, TH3D *effmap) {
    float eff = 0.001; // Default minimum efficiency to avoid division by zero
    int bin = effmap->FindBin(pt, y, mva);
    // if (bin >= 0 && bin <= effmap->GetNbinsX() * effmap->GetNbinsY() * effmap->GetNbinsZ()) {
        double binContent = effmap->GetBinContent(bin);
        if (binContent > 1e-9) { // Check if efficiency is reasonably non-zero
            eff = binContent;
        } else {
            // Handle zero or near-zero efficiency - return a very small efficiency
            // which results in a very large weight. Or, you might want to skip
            // such events or assign a default large weight.
            // Let's return the minimum value used for initialization.
             std::cerr << "Warning: Efficiency map bin content is near zero or zero for bin " << bin
                       << " (pt=" << pt << ", y=" << y << ", mva=" << mva << "). Using eff = 0.001." << std::endl;
             // eff remains 0.001
        // }
    // }
    // } else {
    // cout << "pt: " << pt << ", y: " << y << ", mva: " << mva << ", bin: " << bin << " effmap->GetNbinsX() * effmap->GetNbinsY() * effmap->GetNbinsZ() : " <<  effmap->GetNbinsX() * effmap->GetNbinsY() * effmap->GetNbinsZ() << endl;
    //     std::cerr << "Warning: Could not find bin for pt=" << pt << ", y=" << y << ", mva=" << mva
    //               << ". Using eff = 0.001." << std::endl;
    //     // eff remains 0.001
    }

    // Return the INVERSE efficiency (weight)
    return 1.0 / eff;
}

void MVAEfficiencyRooFit() {
    TString kinCutStr = "mass > 1.84 && mass < 1.88 && pT > 2 && pT < 50 && abs(y) < 1.0";
    gStyle->SetOptStat(0);

    // --- Input Files ---
    TFile *feff = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output.root");
    if (!feff || feff->IsZombie()) {
        std::cerr << "Error: Cannot open efficiency file output.root" << std::endl;
        return;
    }
    // --- Get Efficiency Map ---
    TH3D *effmap = (TH3D*)feff->Get("pt_y_mva_pr_ratio");
    if (!effmap) {
        std::cerr << "Error: Cannot find TH3D 'pt_y_mva_pr_ratio' in output.root" << std::endl;
        feff->Close();
        return;
    }
    // Clone the map to ensure it stays available even if feff is closed early by mistake (optional)
    effmap = (TH3D*)effmap->Clone("effmap_clone");
    effmap->SetDirectory(0); // Decouple from file
    feff->Close(); // Close efficiency file as map is loaded


    // --- Define MVA Cuts and Kinematic Cuts ---
    FitOpt D0opt;
    std::string particleType = "D0";
    std::vector<std::pair<double,double>> ptBins = {
    // {2,3},
    // {3,5},
    // {5,7},
    // {7,10},
    // {10,12.5},
    // {12.5,15},
    // {15,20},
    // {20,25},
    // {25,30},
    // {30,40},
    // {40,100}
    {2,50}
    };
    std::vector<std::pair<double,double>> centBins = {
    // {0,10},
    // {10,30},
    // {30,50},
    // {50,100}
    {0,100}
    };
    std::vector<double> mvaCutBin = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    std::vector<double> mvaCutBin1 = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999,1.00};
    std::map<double, double> avgInvEffMap; // Map: mvaCut -> average_inverse_efficiency
    std::string subDir = "/D0_May03/";

    // Define the base kinematic cut string (applies to all MVA cuts)
    // Use RooFit variable names defined above
    TString kinCutStr = "mass > 1.84 && mass < 1.88 && pT > 2 && pT < 50 && abs(y) < 1.0";
    std::cout << "Applying kinematic cuts: " << kinCutStr << std::endl;

    // --- Loop Through MVA Cuts ---
    for (double mvaCut : mvaCutBin) {
        for(auto ptbin : ptBins){
            for(auto centbin : centBins){
                std::cout << "Processing MVA cut: " << mvaCut << " with pT bin: [" << ptbin.first << ", " << ptbin.second << "] and centrality bin: [" << centbin.first << ", " << centbin.second << "]" << std::endl;
                D0opt.pTMin = ptbin.first;
                D0opt.pTMax = ptbin.second;
                D0opt.centMin = centbin.first;
                D0opt.centMax = centbin.second;
                D0opt.mvaMin = mvaCut;
                D0opt.D0MCDefault();
                TFile *FitResultFile = TFile::Open(("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/" + D0opt.outputDir+subDir+"/"+D0opt.outputFile).c_str());
                RooAddPdf *total_pdf = (RooAddPdf*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->pdf("total_pdf");
                RooDataSet *data = (RooDataSet*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->data("dataset");
                RooRealVar *nSig = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nsig");
                RooRealVar *nBkg = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nbkg");
                RooRealVar *mass = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("mass");
                RooRealVar *pT = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("pT");
                RooRealVar *y = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("y");
                

        RooDataSet *subset = (RooDataSet*)data->reduce(kinCutStr + Form(" && %s > %f", D0opt.mvaVar.c_str(), mvaCut));

        long long nEntriesSubset = subset->numEntries();
        std::cout << "  Subset size: " << nEntriesSubset << " entries." << std::endl;

        if (nEntriesSubset == 0) {
            avgInvEffMap[mvaCut] = 1.0; // Assign default weight (or handle as needed)
            std::cout << "  No entries passed the cut. Assigning default average inverse efficiency = 1.0" << std::endl;
            delete subset; // Clean up the empty subset
            continue;
        }

        double sumInvEff = 0.0;
        long long countValidEff = 0;

        // Iterate through the selected events (the subset)
        for (int i = 0; i < nEntriesSubset; ++i) {
            // Get the variables for the current event
            const RooArgSet* row = subset->get(i);
            if (!row) {
                 std::cerr << "Warning: Could not get row " << i << " from subset for MVA cut " << mvaCut << std::endl;
                 continue; // Skip this event
            }

            // Extract pT and y (use find() for safety)
            const RooAbsArg* pTArg = row->find("pT");
            const RooAbsArg* yArg = row->find("y");

            if (!pTArg || !yArg) {
                 std::cerr << "Warning: Could not find pT or y variable in row " << i << " for MVA cut " << mvaCut << std::endl;
                 continue; // Skip this event
            }

            // Cast to RooRealVar to get value (more robust)
             const RooRealVar* pTVar = dynamic_cast<const RooRealVar*>(pTArg);
             const RooRealVar* yVar = dynamic_cast<const RooRealVar*>(yArg);
             if (!pTVar || !yVar) {
                std::cerr << "Warning: Could not cast pT or y to RooRealVar in row " << i << " for MVA cut " << mvaCut << std::endl;
                continue; // Skip this event
            }

            double currentPt = pTVar->getVal();
            double currentY = yVar->getVal();


            // Get efficiency using the EVENT's pT, y, but the MVA CUT value (+offset)
            // This mimics the original code's logic
            float invEff = getEfficiencyVal(currentPt, currentY, mvaCut + 0.0001, effmap);

            sumInvEff += invEff;
            countValidEff++; // Count events for which we calculated a weight

           // Optional: Print progress/values for debugging
           // if (i % 10000 == 0 && i > 0) {
           //     std::cout << "    Processed " << i << " events in subset for MVA cut " << mvaCut << std::endl;
           // }
        }

        // Calculate average inverse efficiency for this MVA cut
        if (countValidEff > 0) {
            avgInvEffMap[mvaCut] = sumInvEff / countValidEff;
        } else {
             avgInvEffMap[mvaCut] = 1.0; // Default if somehow no valid events were processed
             std::cout << "  Warning: No valid events processed for average inverse efficiency calculation (countValidEff=0) for MVA cut " << mvaCut << ". Assigning default 1.0." << std::endl;
        }

        std::cout << "  Calculated Average Inverse Efficiency = " << avgInvEffMap[mvaCut] << std::endl;

        // Clean up the subset for this iteration
        delete subset;
        delete FitResultFile; // Close the FitResult file
        delete total_pdf; // Clean up the PDF
        delete data; // Clean up the dataset
        delete nSig; // Clean up the signal variable
        delete nBkg; // Clean up the background variable
        delete mass; // Clean up the mass variable
        delete pT; // Clean up the pT variable
        delete y; // Clean up the y variable
            } // End loop over ptbin
        } // End loop over centbin

    } // End loop over mvaCutBin

    // --- Apply Corrections and Create Output Histograms ---
    TFile *fh1 = new TFile("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/D0_Yield_mva.root", "READ");
     if (!fh1 || fh1->IsZombie()) {
        std::cerr << "Error: Cannot open input yield file D0_Yield_mva.root" << std::endl;
        delete effmap; // Clean up cloned map
        return;
    }
    TH1D* h1 = (TH1D*)fh1->Get("D0_Yield_mva");
     if (!h1) {
        std::cerr << "Error: Cannot find TH1D 'D0_Yield_mva' in D0_Yield_mva.root" << std::endl;
        fh1->Close();
        delete effmap; // Clean up cloned map
        return;
    }

    // Ensure the binning matches mvaCutBin (mvaCutBin defines lower edges + upper edge of last bin)
    int nBins = mvaCutBin1.size(); // Number of edges
    if (h1->GetNbinsX() != nBins -1) {
         std::cerr << "Warning: Number of bins in D0_Yield_mva histogram (" << h1->GetNbinsX()
                   << ") does not match the number of MVA cut intervals (" << nBins - 1 << ")." << std::endl;
         // Consider resizing or rebinning h1, or adjusting mvaCutBin if appropriate.
         // For now, proceed assuming the user knows they align conceptually.
    }

    // Create output histograms with the same binning structure
    // Need nBins-1 bins, using the mvaCutBin vector for the edges
    TH1D* hCorrect = new TH1D("hCorrect", "Corrected Yield vs MVA Cut;MVA Cut Lower Edge;Corrected Yield",
                              nBins-1 , mvaCutBin1.data());
    TH1D* hEff = new TH1D("hEff", "Average Inverse Efficiency vs MVA Cut;MVA Cut Lower Edge;Avg. Inv. Efficiency",
                           nBins-1, mvaCutBin1.data());
    hCorrect->Sumw2(); // Enable errors for corrected histogram
    hEff->Sumw2();     // Enable errors for efficiency histogram (though errors aren't calculated here)


    std::cout << "\nApplying corrections:" << std::endl;
    // Fill the output histograms
    for (int i = 0; i < nBins - 1; ++i) { // Loop through bins (intervals)
        double mvaCutLowerEdge = mvaCutBin[i];
        if (avgInvEffMap.count(mvaCutLowerEdge)) {
            double avgInvEff = avgInvEffMap[mvaCutLowerEdge];
            double rawYield = h1->GetBinContent(i + 1); // Bins are 1-indexed
            double rawError = h1->GetBinError(i + 1);

            hEff->SetBinContent(i + 1, avgInvEff);
            // Error on avgInvEff is not calculated here, set to 0
            hEff->SetBinError(i + 1, 0.0);

            hCorrect->SetBinContent(i + 1, rawYield * avgInvEff);
            // Basic error propagation (assuming error on avgInvEff is negligible)
            hCorrect->SetBinError(i + 1, rawError * avgInvEff);

            std::cout << "  MVA Cut Bin [" << mvaCutLowerEdge << ", " << mvaCutBin[i+1] << "): "
                      << "Raw Yield = " << rawYield << " +/- " << rawError
                      << ", AvgInvEff = " << avgInvEff
                      << ", Corrected Yield = " << hCorrect->GetBinContent(i + 1)
                      << " +/- " << hCorrect->GetBinError(i + 1) << std::endl;

        } else {
            std::cerr << "Warning: No average inverse efficiency found for MVA cut " << mvaCutLowerEdge << ". Skipping bin " << i+1 << std::endl;
             hEff->SetBinContent(i + 1, 1.0); // Default value
             hCorrect->SetBinContent(i + 1, h1->GetBinContent(i + 1)); // No correction
             hCorrect->SetBinError(i + 1, h1->GetBinError(i + 1));
        }
    }
    // fh1->Close(); // Close the yield file


    // --- Plotting and Saving ---
    TCanvas *c = new TCanvas("c0", "Raw Yield", 800, 600);
    h1->Draw("");
    c->SaveAs("h1_RooFit_raw.png");
    TCanvas * c0 = new TCanvas("c0", "Raw Yield", 800, 600);
    hCorrect->Draw(""); // Draw with error bars
    c0->SaveAs("hCorrect_RooFit_raw.png");
    TCanvas *c1 = new TCanvas("c1", "Corrected Yield", 800, 600);
    TLegend *leg = new TLegend(0.7, 0.8, 0.9, 0.9);
    // leg->SetBorderSize(0);
    hCorrect->GetYaxis()->SetRangeUser(h1->GetMinimum()*0,hCorrect->GetBinContent(1) * 1.2); // Adjust max for visibility
    hCorrect->Draw(""); // Draw with error bars
    h1->SetLineColor(kRed);
    h1->Scale(hCorrect->GetBinContent(1)/h1->GetBinContent(1));
    h1->Draw("same");
    leg->AddEntry(hCorrect, "Corrected Yield", "lep");
    leg->AddEntry(h1, "Raw Yield (scaled)", "l");
    leg->Draw();
    
    c1->SaveAs("hCorrect_RooFit.png");
    c1->SaveAs("hCorrect_RooFit.pdf");

    TCanvas *c2 = new TCanvas("c2", "Average Inverse Efficiency", 800, 600);
    hEff->Draw();
    c2->SaveAs("hAvgInvEff_RooFit.png");
    c2->SaveAs("hAvgInvEff_RooFit.pdf");

    TFile *fout = new TFile("efficiency_RooFit.root", "RECREATE");
    fout->cd();
    hEff->Write();
    hCorrect->Write(); // Also save the corrected yield histogram
    fout->Close();

    std::cout << "\nSaved average inverse efficiency to efficiency_RooFit.root (hEff)" << std::endl;
    std::cout << "Saved corrected yield to efficiency_RooFit.root (hCorrect)" << std::endl;
    std::cout << "Saved plots: hCorrect_RooFit.png/pdf, hAvgInvEff_RooFit.png/pdf" << std::endl;


    // --- Cleanup ---
    // delete c1;
    // delete c2;
    // delete hCorrect;
    // delete hEff;
    // delete effmap; // Delete the cloned map
    // RooFit objects (dataSet, vars) are usually managed by ROOT or go out of scope.
    // Explicit deletion of dataSet might be needed if allocated differently.
    // The subsets were deleted inside the loop.

    std::cout << "RooFit-based MVA Efficiency calculation finished." << std::endl;
}

