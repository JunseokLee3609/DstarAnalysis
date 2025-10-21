#include "../fit/Opt.h"
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <RooAddPdf.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>
#include <RooStats/SPlot.h>
#include <RooArgSet.h>
#include <RooArgList.h>
#include <RooFitResult.h>
// #include <TSeq.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath> // for fabs
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

double CalSplotweight(FitOpt D0Opt, TH3D* hist_eff, double mvaCut, std::string subDir){
    D0Opt.outputDir = D0Opt.outputDir + subDir + "/";

    gStyle->SetOptStat(0);
    TFile *FitResultFile = TFile::Open((D0Opt.outputDir+"/"+D0Opt.outputFile).c_str());
    if (!FitResultFile || FitResultFile->IsZombie()) {
        std::cerr << "Error: Could not open file " << (D0Opt.outputDir+"/"+D0Opt.outputFile) << std::endl;
        return 0.0;
    }
    RooWorkspace* ws = (RooWorkspace*)FitResultFile->Get("ws_D0");
    if (!ws) {
        std::cerr << "Error: Could not retrieve workspace ws_D0 from file." << std::endl;
        FitResultFile->Close();
        delete FitResultFile;
        return 0.0;
    }
    RooAddPdf *total_pdf = (RooAddPdf*)ws->pdf("total_pdf");
    RooDataSet *data = (RooDataSet*)ws->data("dataset");
    RooRealVar *nSig = (RooRealVar*)ws->var("nsig");
    RooRealVar *nBkg = (RooRealVar*)ws->var("nbkg");
    RooRealVar *mass = (RooRealVar*)ws->var("mass");

    if (!total_pdf || !data || !nSig || !nBkg || !mass) {
        std::cerr << "Error: Could not retrieve objects (pdf, data, vars) from workspace." << std::endl;
        FitResultFile->Close();
        delete FitResultFile;
        return 0.0;
    }

    RooArgSet* params = total_pdf->getParameters(*data);
    mass->setRange("analysis", D0Opt.massMin, D0Opt.massMax);
    mass->setMin(D0Opt.massMin);
    mass->setMax(D0Opt.massMax);

    RooStats::SPlot splot("splot", "", *data,
        total_pdf,
        RooArgList(*nSig, *nBkg),
        RooArgSet(),
        false,
        true,
        "rd_total",
        RooFit::NumCPU(8, 0),
        RooFit::Range("analysis")
    );

    auto newrd = splot.GetSDataSet();
    if (!newrd) {
        std::cerr << "Error: Could not get sWeighted dataset." << std::endl;
        FitResultFile->Close();
        delete FitResultFile;
        return 0.0;
    }

    int mismatch_count = 0;
    for (auto idx : ROOT::TSeqI(newrd->numEntries())) {
        const RooArgSet* row = newrd->get(idx);
        if (!row) continue;
        RooRealVar* nsig_sw = (RooRealVar*)row->find("nsig_sw");
        RooRealVar* nbkg_sw = (RooRealVar*)row->find("nbkg_sw");
        if (!nsig_sw || !nbkg_sw) {
            continue;
        }
        double sum_sw = nsig_sw->getVal() + nbkg_sw->getVal();
        if (fabs(sum_sw - 1.0) > 1e-6) {
            mismatch_count++;
        }
    }

    RooDataSet* wds_sig = new RooDataSet("fitDataSWeighted_sig", "", newrd,
            *newrd->get(),
            nullptr, "nsig_sw"
    );

    double sWeight_val = 0.0;
    double count2 = 0.0;
    double corr2 = 0.0;
    for( auto idx: ROOT::TSeqI(wds_sig->numEntries())){
        const RooArgSet* row = wds_sig->get(idx);
        if (!row) continue;
        RooRealVar* pt_var = (RooRealVar*) row->find("pT");
        RooRealVar* y_var = (RooRealVar*) row->find("y");
        RooRealVar* cent_var = (RooRealVar*) row->find("Centrality");
        RooRealVar *ncoll = (RooRealVar*)row->find("Ncoll");
        if(!pt_var || !y_var) {
             std::cerr << "Event " << idx << ": Missing pT or y variable!" << std::endl;
             continue;
        }

        double corr_ = 0.0;
        if (hist_eff) {
             corr_ = hist_eff->GetBinContent(hist_eff->FindBin(pt_var->getVal(), y_var->getVal(),cent_var->getVal()));
        } else {
            std::cerr << "Error: hist_eff is null!" << std::endl;
        }

        sWeight_val = wds_sig->weight();
        if (fabs(sWeight_val) > 1e-9 && fabs(corr_) > 1e-9) {
             corr_ /= sWeight_val;
             corr2 += 1.0/corr_;
             count2+= sWeight_val;
        }
        // cout << "ncoll: " << ncoll->getVal() << endl;
    }
    cout << (D0Opt.outputDir+"/"+D0Opt.outputFile) << endl;
    cout << count2 << endl;

    delete wds_sig;
    FitResultFile->Close();
    delete FitResultFile;

    if (fabs(count2) > 1e-9) {
        return corr2 / count2;
    } else {
        std::cerr << "Warning: count2 is zero or very small, returning 0." << std::endl;
        return 0.0;
    }
}

void MVAEfficiencyRooFit_cent(){
    // std::string subDir = "D0_Apr30/";
    
    std::string subDir = "D0_Apr30/";
    TFile *effFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output_MC_May09_ncoll_v2.root","read");
    if (!effFile || effFile->IsZombie()) {
        std::cerr << "Error opening efficiency file." << std::endl;
        return;
    }

    TFile *yieldFile = TFile::Open(Form("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/roots/Data_D0_PbPb/%s/D0_Yield_mva.root",subDir.c_str()),"read");
     if (!yieldFile || yieldFile->IsZombie()) {
        std::cerr << "Error opening yield file." << std::endl;
        effFile->Close();
        return;
    }
    TH2D *h_raw_yield_2d = (TH2D*)yieldFile->Get("D0_Yield_mva_cent");
     if (!h_raw_yield_2d) {
        std::cerr << "Error getting raw yield histogram." << std::endl;
        yieldFile->Close();
        effFile->Close();
        return;
    }

    FitOpt D0opt;
    std::string particleType = "D0";
    

    // Create histogram for Ncoll-weighted GEN yield passing cuts vs centrality
    // Need to define centrality bins matching centBins vector
    // std::vector<double> centEdges;
    // centEdges.push_back(centBins[0].first);
    // for(const auto& bin : centBins) { centEdges.push_back(bin.second); }
    std::vector<double> mvaBin= {0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    // std::vector<double> mvaBin= {0.990, 0.991};
    std::vector<double> mvaBinEdges = {0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    // std::vector<double> mvaBinEdges = {0.990, 0.991, 0.992};

    // std::string subDir = "D0_Apr30/";
    std::vector<std::pair<double,double>> ptBins = {
        {0,50}
        
    };
    std::vector<std::pair<double,double>> centBins = {
        // {0,90}
        {0,10},
        {10,40},
        {40,90}

        // {10,40},
        // {40,90}
    };
    // std::vector<double> mvaBinEdges = {0.90,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    
    std::vector<double> mvaBinCenters;
    for(size_t i = 0; i < mvaBinEdges.size() - 1; ++i) {
        mvaBinCenters.push_back( (mvaBinEdges[i] + mvaBinEdges[i+1]) / 2.0 );
    }

    std::vector<TH1D*> h_corrected_yields(centBins.size());
    std::vector<TH1D*> h_weights(centBins.size());
    std::vector<TH1D*> h_raw_yields(centBins.size());

    for(size_t icent = 0; icent < centBins.size(); ++icent) {
        std::string centLabel = std::to_string((int)centBins[icent].first) + "to" + std::to_string((int)centBins[icent].second);
        h_corrected_yields[icent] = new TH1D(("corrected_yield_cent_" + centLabel).c_str(),
                                           ("Corrected Yield vs MVA; MVA Cut; Corrected Yield (" + centLabel + " %)").c_str(),
                                           mvaBinEdges.size()-1,0,1);
        h_weights[icent] = new TH1D(("weight_cent_" + centLabel).c_str(),
                                  ("Weighting Factor vs MVA; MVA Cut; Weight (" + centLabel + " %)").c_str(),
                                  mvaBinEdges.size()-1,0, 1);
        h_raw_yields[icent] = new TH1D(("raw_yield_cent_" + centLabel).c_str(),
                                     ("Raw Yield vs MVA; MVA Cut; Raw Yield (" + centLabel + " %)").c_str(),
                                     mvaBinEdges.size()-1,0, 1);
        h_corrected_yields[icent]->SetMarkerStyle(20);
        h_corrected_yields[icent]->SetMarkerSize(1.0);
        h_weights[icent]->SetMarkerStyle(20);
        h_weights[icent]->SetMarkerSize(1.0);
        h_raw_yields[icent]->SetMarkerStyle(24);
        h_raw_yields[icent]->SetMarkerSize(1.0);
        h_raw_yields[icent]->SetLineColor(kRed);
        h_raw_yields[icent]->SetMarkerColor(kRed);
    }

    for(size_t imva = 0; imva < mvaBinEdges.size() - 1; ++imva) {
         double mva_cut = mvaBinEdges[imva];
         int icent =0;
    TH3D *effmap = (TH3D*)effFile->Get(Form("eff_pt_y_cent_pr_mvaCut_gt_%.4f", mvaBin[imva]));
    if (!effmap) {
        std::cerr << "Error getting efficiency histogram." << std::endl;
        effFile->Close();
        return;
    }

            for(auto centbin : centBins){
                D0opt.pTMin = ptBins[0].first;
                D0opt.pTMax = ptBins[0].second;
                D0opt.centMin = centbin.first;
                D0opt.centMax = centbin.second;
                D0opt.mvaMin = mva_cut;
                D0opt.D0MCDefault();
                D0opt.outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/"+D0opt.outputDir;
                TFile *FitResultFile = TFile::Open(("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/" + D0opt.outputDir+subDir+"/"+D0opt.outputFile).c_str());
                RooAddPdf *total_pdf = (RooAddPdf*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->pdf("total_pdf");
                RooDataSet *data = (RooDataSet*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->data("dataset");
                RooRealVar *nSig = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nsig");
                RooRealVar *nBkg = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nbkg");
                RooRealVar *mass = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("mass");
                RooRealVar *pT = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("pT");
                RooRealVar *y = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("y");
                RooRealVar *cent = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("Centrality");
                TString kinCutStr = "mass > 1.84 && mass < 1.88 && pT > 2 && pT < 50 && abs(y) < 1.0";
                std::cout << "Applying kinematic cuts: " << kinCutStr << std::endl;
                RooDataSet *subset = (RooDataSet*)data->reduce(kinCutStr + Form(" && %s > %f", D0opt.mvaVar.c_str(), mvaCut));
                long long nEntriesSubset = subset->numEntries();
                std::cout << "  Subset size: " << nEntriesSubset << " entries." << std::endl;
        

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
                double corr = sumInvEff/countValidEff++;

                // double corr = CalSplotweight(D0opt, effmap, mva_cut, subDir);
                // cout << "cent " << centbin.first << " to " << centbin.second << " mva " << mva_cut << " corr " << corr << endl;

                double raw_yield = h_raw_yield_2d->GetBinContent(imva + 1, icent + 1);
                double raw_yield_err = h_raw_yield_2d->GetBinError(imva + 1, icent + 1);

                double corrected_yield = raw_yield * corr;
                double corrected_yield_err = raw_yield_err * corr;

                h_corrected_yields[icent]->SetBinContent(imva + 1, corrected_yield);
                h_corrected_yields[icent]->SetBinError(imva + 1, corrected_yield_err);
                h_weights[icent]->SetBinContent(imva + 1, corr);
                h_raw_yields[icent]->SetBinContent(imva + 1, raw_yield);
                h_raw_yields[icent]->SetBinError(imva + 1, raw_yield_err);
                icent++;
            }
            delete effmap;
            // effmap = nullptr;
    }

    // // Set custom bin labels for X-axis to make bins appear equally wide
    for(size_t icent = 0; icent < centBins.size(); ++icent) {
        std::vector<TH1D*> hists_to_label = {h_corrected_yields[icent], h_weights[icent], h_raw_yields[icent]};
        for (TH1D* h : hists_to_label) {
            if (!h) continue;
            TAxis* xAxis = h->GetXaxis();
            for (int bin_idx = 1; bin_idx <= xAxis->GetNbins(); ++bin_idx) {
                // mvaBinEdges is 0-indexed. For bin_idx (1-based), edges are mvaBinEdges[bin_idx-1] and mvaBinEdges[bin_idx]
                if (bin_idx -1 < mvaBinEdges.size() && bin_idx < mvaBinEdges.size()) {
                     std::string binLabel = Form("%.3f-%.3f", mvaBinEdges[bin_idx-1], mvaBinEdges[bin_idx]);
                     xAxis->SetBinLabel(bin_idx, binLabel.c_str());
                }
            }
        }
    }

    TLatex latex;
    gStyle->SetOptStat(0);
    latex.SetNDC();
    latex.SetTextSize(0.04);

    for(size_t icent = 0; icent < centBins.size(); ++icent) {
        std::string centLabel = std::to_string((int)centBins[icent].first) + "to" + std::to_string((int)centBins[icent].second);

        TCanvas *c_yield = new TCanvas(("c_yield_cent_" + centLabel).c_str(), ("Corrected Yield cent " + centLabel).c_str(), 800, 600);
        h_corrected_yields[icent]->GetYaxis()->SetRangeUser(h_corrected_yields[icent]->GetMinimum()*0.2,h_corrected_yields[icent]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_corrected_yields[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_yield->SetGrid(1,1);
        c_yield->SaveAs(("cutBased_corrected_yield_cent_" + centLabel + ".png").c_str());
        delete c_yield;

        TCanvas *c_weight = new TCanvas(("c_weight_cent_" + centLabel).c_str(), ("Weighting Factor cent " + centLabel).c_str(), 800, 600);
        h_weights[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_weight->SetGrid(1,1);
        c_weight->SaveAs(("cutBased_weight_cent_" + centLabel + ".png").c_str());
        delete c_weight;

        TCanvas *c_comp = new TCanvas(("c_comp_cent_" + centLabel).c_str(), ("Yield Comparison cent " + centLabel).c_str(), 800, 600);
        // h_corrected_yields[icent]->GetYaxis()->SetRangeUser(h_corrected_yields[icent]->GetMinimum()*0.2,h_corrected_yields[icent]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_raw_yields[icent]->Scale(h_corrected_yields[icent]->GetBinContent(1)/h_raw_yields[icent]->GetBinContent(1));
        h_raw_yields[icent]->SetLineColor(kRed);
        h_raw_yields[icent]->SetMarkerColor(kRed);
        h_raw_yields[icent]->SetMarkerStyle(24);
        h_raw_yields[icent]->SetMarkerSize(1.0);
        h_corrected_yields[icent]->SetLineColor(kBlack);
        h_corrected_yields[icent]->SetMarkerColor(kBlack);
        h_corrected_yields[icent]->SetMarkerStyle(20);
        h_corrected_yields[icent]->SetMarkerSize(1.0);
        

        
        h_corrected_yields[icent]->Draw("PE1");
        h_raw_yields[icent]->Draw("PE1 SAME");
        TLegend *leg = new TLegend(0.7, 0.8, 0.9, 0.9);
        leg->AddEntry(h_corrected_yields[icent], "Corrected Yield", "pe");
        leg->AddEntry(h_raw_yields[icent], "Raw Yield", "pe");
        leg->Draw();
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_comp->SetGrid(1,1);
        c_comp->SaveAs(("cutBased_yield_comparison_cent_" + centLabel + ".png").c_str());
        delete leg;
        delete c_comp;
    }

    for(size_t icent = 0; icent < centBins.size(); ++icent) {
        delete h_corrected_yields[icent];
        delete h_weights[icent];
        delete h_raw_yields[icent];
    }

    yieldFile->Close();
    effFile->Close();

    std::cout << "Finished drawing and saving histograms." << std::endl;
}
