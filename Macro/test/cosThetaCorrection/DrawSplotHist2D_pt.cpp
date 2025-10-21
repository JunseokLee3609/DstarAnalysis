#include "../../fit/Opt.h"
#include "../../fit/DStarFitConfig.h"
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
#include "../../interface/simpleDMC.h"
#include "../../Tools/Transformations.h"
// #include <TSeq.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath> // for fabs
float getEfficiencyVal(double pt, double cos, TH2D *effmap){
    float eff = 0.001; // Default minimum efficiency to avoid division by zero
    int bin = effmap->FindBin(pt, cos);
    // if (bin >= 0 && bin <= effmap->GetNbinsX() * effmap->GetNbinsY() * effmap->GetNbinsZ()) {
        double binContent = effmap->GetBinContent(bin);
        if (binContent > 1e-9) { // Check if efficiency is reasonably non-zero
            eff = binContent;
        } else {
            // Handle zero or near-zero efficiency - return a very small efficiency
            // which results in a very large weight. Or, you might want to skip
            // such events or assign a default large weight.
            // Let's return the minimum value used for initialization.
            //  std::cerr << "Warning: Efficiency map bin content is near zero or zero for bin " << bin
            //            << " (pt=" << pt << ", y=" << y << ", mva=" << mva << "). Using eff = 0.001." << std::endl;
             // eff remains 0.001
        // }
    // }
    // } else {
    // cout << "pt: " << pt << ", y: " << y << ", mva: " << mva << ", bin: " << bin << " effmap->GetNbinsX() * effmap->GetNbinsY() * effmap->GetNbinsZ() : " <<  effmap->GetNbinsX() * effmap->GetNbinsY() * effmap->GetNbinsZ() << endl;
    //     std::cerr << "Warning: Could not find bin for pt=" << pt << ", y=" << y << ", mva=" << mva
    //               << ". Using eff = 0.001." << std::endl;
    //     // eff remains 0.001
    }

    return 1.0 / eff;
}

std::vector<double> CalSplotweight(FitOpt D0Opt, TH2D* hist_eff, std::string subDir){
    TString subStr = "";
    //TString kinCutStr = Form("massPion > 0.144 && massPion < 0.148 && pT > %f && pT < %f && cosThetaHX >= %f && cosThetaHX < %f && abs(y) < 1.0",D0Opt.pTMin, D0Opt.pTMax, D0Opt.cosMin, D0Opt.cosMax);
    TString kinCutStr = Form("massPion > 0.144 && massPion < 0.148");
    D0Opt.outputDir = D0Opt.outputDir + subDir + "/";

    gStyle->SetOptStat(0);
    TFile *FitResultFile = TFile::Open((D0Opt.outputDir+"/"+D0Opt.outputFile).c_str());
    if (!FitResultFile || FitResultFile->IsZombie()) {
        std::cerr << "Error: Could not open file " << (D0Opt.outputDir+"/"+D0Opt.outputFile) << std::endl;
        // return 0.0;
    }
    RooWorkspace* ws = (RooWorkspace*)FitResultFile->Get("ws_DStar");
    if (!ws) {
        std::cerr << "Error: Could not retrieve workspace ws_DStar from file." << std::endl;
        FitResultFile->Close();
        delete FitResultFile;
        // return 0.0;
    }
    cout << "file Name : " << FitResultFile->GetName() << endl;
    RooAddPdf *total_pdf = (RooAddPdf*)ws->pdf("total_pdf");
    RooDataSet *data = (RooDataSet*)ws->data("datasetHX");
    RooRealVar *nSig = (RooRealVar*)ws->var("nsig");
    RooRealVar *nBkg = (RooRealVar*)ws->var("nbkg");
    RooRealVar *mass = (RooRealVar*)ws->var("massPion");
    RooDataSet* subset = (RooDataSet*)data->reduce(kinCutStr);
    long long nEntriesSubset = subset->numEntries();
    std::cout << "  Subset size: " << nEntriesSubset << " entries." << std::endl;
     double sumInvEff = 0.0;
    long long countValidEff = 0;

    // Iterate through the selected events (the subset)
    for (int i = 0; i < nEntriesSubset; ++i) {
        // Get the variables for the current event
        const RooArgSet* row = subset->get(i);
        if (!row) {
            //  std::cerr << "Warning: Could not get row " << i << " from subset for MVA cut " << mvaCut << std::endl;
             continue; // Skip this event
        }

        // Extract pT and y (use find() for safety)
        const RooAbsArg* pTArg = row->find("pT");
        const RooAbsArg* yArg = row->find("y");
        const RooAbsArg* cosArg = row->find("cosThetaHX");

        if (!pTArg || !yArg) {
            //  std::cerr << "Warning: Could not find pT or y variable in row " << i << " for MVA cut " << mvaCut << std::endl;
             continue; // Skip this event
        }

        // Cast to RooRealVar to get value (more robust)
         const RooRealVar* pTVar = dynamic_cast<const RooRealVar*>(pTArg);
         const RooRealVar* yVar = dynamic_cast<const RooRealVar*>(yArg);
         const RooRealVar* cosVar = dynamic_cast<const RooRealVar*>(cosArg);
            // const RooRealVar* ncolVar = dynamic_cast<const RooRealVar*>(ncolArg);
         if (!pTVar || !yVar) {
            // std::cerr << "Warning: Could not cast pT or y to RooRealVar in row " << i << " for MVA cut " << mvaCut << std::endl;
            continue; // Skip this event
        }

        double currentPt = pTVar->getVal();
        double currentY = yVar->getVal();
        double currentcos = cosVar->getVal();



        // Get efficiency using the EVENT's pT, y, but the MVA CUT value (+offset)
        // This mimics the original code's logic
        float invEff = getEfficiencyVal(currentPt, currentcos, hist_eff);

        sumInvEff += invEff;
        countValidEff ++; // Count events for which we calculated a weight

       // Optional: Print progress/values for debugging
       // if (i % 10000 == 0 && i > 0) {
       //     std::cout << "    Processed " << i << " events in subset for MVA cut " << mvaCut << std::endl;
       // }
    }

    if (!total_pdf || !data || !nSig || !nBkg || !mass) {
        std::cerr << "Error: Could not retrieve objects (pdf, data, vars) from workspace." << std::endl;
        FitResultFile->Close();
        delete FitResultFile;
        // return 0.0;
    }

    RooArgSet* params = total_pdf->getParameters(*data);
    mass->setRange("analysis", D0Opt.massMin, D0Opt.massMax);
    mass->setMin(D0Opt.massMin);
    mass->setMax(D0Opt.massMax);

    // RooStats::SPlot splot("splot", "", *data,
    //     total_pdf,
    //     RooArgList(*nSig, *nBkg),
    //     RooArgSet(),
    //     false,
    //     true,
    //     "rd_total",
    //     RooFit::NumCPU(8, 0),
    //     RooFit::Range("analysis")
    // );

    // auto newrd = splot.GetSDataSet();
    // if (!newrd) {
    //     std::cerr << "Error: Could not get sWeighted dataset." << std::endl;
    //     FitResultFile->Close();
    //     delete FitResultFile;
    //     // return 0.0;
    // }

    // int mismatch_count = 0;
    // for (auto idx : ROOT::TSeqI(newrd->numEntries())) {
    //     const RooArgSet* row = newrd->get(idx);
    //     if (!row) continue;
    //     RooRealVar* nsig_sw = (RooRealVar*)row->find("nsig_sw");
    //     RooRealVar* nbkg_sw = (RooRealVar*)row->find("nbkg_sw");
    //     if (!nsig_sw || !nbkg_sw) {
    //         continue;
    //     }
    //     double sum_sw = nsig_sw->getVal() + nbkg_sw->getVal();
    //     if (fabs(sum_sw - 1.0) > 1e-6) {
    //         mismatch_count++;
    //     }
    // }

    // RooDataSet* wds_sig = new RooDataSet("fitDataSWeighted_sig", "", newrd,
    //         *newrd->get(),
    //         nullptr, "nsig_sw"
    // );

    // double sWeight_val = 0.0;
    // double count2 = 0.0;
    // double corr2 = 0.0;
    // for( auto idx: ROOT::TSeqI(wds_sig->numEntries())){
    //     const RooArgSet* row = wds_sig->get(idx);
    //     if (!row) continue;
    //     RooRealVar* pt_var = (RooRealVar*) row->find("pT");
    //     RooRealVar* y_var = (RooRealVar*) row->find("y");
    //     // RooRealVar *ncoll = (RooRealVar*)row->find("Ncoll");
    //     RooRealVar *cos_var = (RooRealVar*)row->find("cosThetaHX");
    //     if(!pt_var || !y_var) {
    //          std::cerr << "Event " << idx << ": Missing pT or y variable!" << std::endl;
    //          continue;
    //     }

    //     double corr_ = 0.0;
    //     if (hist_eff) {
    //          corr_ = hist_eff->GetBinContent(hist_eff->FindBin(pt_var->getVal(), cos_var->getVal()));
    //     } else {
    //         std::cerr << "Error: hist_eff is null!" << std::endl;
    //     }

    //     sWeight_val = wds_sig->weight();
    //     if (fabs(sWeight_val) > 1e-9 && fabs(corr_) > 1e-9) {
    //          corr_ /= sWeight_val;
    //          corr2 += 1.0/corr_;
    //          count2+= sWeight_val;
    //     }
        // cout << "ncoll: " << ncoll->getVal() << endl;
    // }
    cout << (D0Opt.outputDir+"/"+D0Opt.outputFile) << endl;
    // cout << count2 << endl;
    return std::vector<double> {0, (sumInvEff/countValidEff), (double)nSig->getVal(), (double)nSig->getError()};
    // delete wds_sig;
    FitResultFile->Close();
    // delete FitResultFile;

    // if (fabs(count2) > 1e-9) {
    //     return corr2 / count2;
    // } else {
    //     std::cerr << "Warning: count2 is zero or very small, returning 0." << std::endl;
    //     return 0.0;
    // }
}

void DrawSplotHist2D_pt(){
    // std::string subDir = "D0_Apr30/";
    TFile *effFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output_Mix_Jun18.root","read");
    std::string subDir = "DStar_ppRef_Jun19_v1";
    std::string subStr = "bkgpol1";
    gSystem->mkdir("splot",kTRUE);

    if (!effFile || effFile->IsZombie()) {
        std::cerr << "Error opening efficiency file." << std::endl;
        return;
    }

    FitOpt D0opt;
    std::string particleType = "DStar";
    

    // Create histogram for Ncoll-weighted GEN yield passing cuts vs centrality
    // Need to define centrality bins matching centBins vector
    // std::vector<double> centEdges;
    // centEdges.push_back(centBins[0].first);
    // for(const auto& bin : centBins) { centEdges.push_back(bin.second); }
    // std::vector<double> mvaBin= {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    // std::vector<double> mvaBin= {0.8,0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    std::vector<double> cosBin= {-1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
    // std::vector<double> cosBin = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    // std::vector<double> cosBin = {0.8,0.90,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    // std::vector<double> cosBin; 
    // for(int i =0; i<mvaBin.size(); i++){
    //     cosBin.push_back(mvaBin[i]);
    //     if(mvaBin[i] == mvaBin.back()){
    //         cosBin.push_back(1.0);
    //     }
    // }
    
    // h_gen_yield_pass_ncoll->Sumw2(); // Enable error calculation
    // int jmva = 0;


    // fMCPR_gen->Close(); // Close the GEN file
    // // delete fMCPR_gen;
    // std::cout << "GEN level yield calculation finished." << std::endl;
    // std::string subDir = "D0_Apr30/";
    std::vector<std::pair<double,double>> ptBins = {
        {5,10},
        {5,7},
        {7,10},
        {10,20},
        {20,50}
        
    };
    // std::vector<std::pair<double,double>> cosBins = {
    //     {0,90}
        
    //     // {0,10}
    // };

    std::vector<TH1D*> h_corrected_yields(ptBins.size());
    std::vector<TH1D*> h_corrected_yields_cb(ptBins.size());
    std::vector<TH1D*> h_corrected_yields_gm(ptBins.size());
    std::vector<TH1D*> h_weights(ptBins.size());
    std::vector<TH1D*> h_weights_cb(ptBins.size());
    // std::vector<TH1D*> h_weights_gm(ptBins.size());
    std::vector<TH1D*> h_raw_yields(ptBins.size());
    // std::vector<TH1D*> h_raw_yields_gm(ptBins.size());
    std::vector<TH1D*> h_gen_yield_pass_ncoll(ptBins.size());
    for(size_t ipt = 0; ipt < ptBins.size(); ipt++) {
        std::string ptLabel = std::to_string((int)ptBins[ipt].first) + "to" + std::to_string((int)ptBins[ipt].second);
        h_corrected_yields[ipt] = new TH1D(("corrected_yield_pt_" + ptLabel).c_str(),
                                           ("Corrected Yield vs MVA; MVA Cut; Corrected Yield (" + ptLabel + " %)").c_str(),
                                           cosBin.size()-1, 0,1);
        h_corrected_yields_cb[ipt] = new TH1D(("corrected_yield_pt_cb_" + ptLabel).c_str(),
                                           ("Corrected Yield vs MVA; MVA Cut; Corrected Yield (" + ptLabel + " %)").c_str(),
                                           cosBin.size()-1, 0,1);
                                           
        h_weights[ipt] = new TH1D(("weight_pt_" + ptLabel).c_str(),
                                  ("Weighting Factor vs MVA; MVA Cut; Weight (" + ptLabel + " %)").c_str(),
                                  cosBin.size()-1, 0,1);
        h_weights_cb[ipt] = new TH1D(("weight_pt_cb_" + ptLabel).c_str(),
                                  ("Weighting Factor vs MVA; MVA Cut; Weight (" + ptLabel + " %)").c_str(),
                                  cosBin.size()-1, 0,1);
                                  
        h_raw_yields[ipt] = new TH1D(("raw_yield_pt_" + ptLabel).c_str(),
                                     ("Raw Yield vs MVA; MVA Cut; Raw Yield (" + ptLabel + " %)").c_str(),
                                     cosBin.size()-1,0,1);
                                     
                                    
        h_corrected_yields[ipt]->SetMarkerStyle(20);
        h_corrected_yields[ipt]->SetMarkerSize(1.0);
        h_corrected_yields_cb[ipt]->SetMarkerStyle(20);
        h_corrected_yields_cb[ipt]->SetMarkerSize(1.0);
        h_weights[ipt]->SetMarkerStyle(20);
        h_weights[ipt]->SetMarkerSize(1.0);
        h_weights_cb[ipt]->SetMarkerStyle(20);
        h_weights_cb[ipt]->SetMarkerSize(1.0);
        h_raw_yields[ipt]->SetMarkerStyle(24);
        h_raw_yields[ipt]->SetMarkerSize(1.0);
        h_raw_yields[ipt]->SetLineColor(kRed);
        h_raw_yields[ipt]->SetMarkerColor(kRed);
        
        
        


    }


    // for(size_t imva = 0; imva < cosBin.size() - 1; ++imva) {
    // std::vector<double> cosBin = {0.90,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    


    for(size_t imva = 0; imva < cosBin.size() - 1; ++imva) {
        //  double mva_cut = cosBin[imva];
         int ipt =0;
    TH2D *effmap = (TH2D*)effFile->Get(Form("pt_cos_pr_pass_ratio"));
    if (!effmap) {
        std::cerr << "Error getting efficiency histogram." << std::endl;
        effFile->Close();
        return;
    }



            for(auto ptbin : ptBins){
                D0opt.pTMin = ptbin.first;
                D0opt.pTMax = ptbin.second;
                D0opt.cosMin = cosBin[imva];
                D0opt.cosMax = cosBin[imva+1];
                // D0opt.mvaMin = mva_cut;
                ConfigureDStarMCFitOpt(D0opt);
                D0opt.outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/"+D0opt.outputDir;


                std::vector<double> weight = CalSplotweight(D0opt, effmap, subDir);
                cout << weight[0] << " " << weight[1] << " " << weight[2] << " " << weight[3] << endl;
                // continue;

                // cout << "cent " << centbin.first << " to " << centbin.second << " mva " << mva_cut << " corr " << corr << endl;

                // double raw_yield = h_raw_yield_2d->GetBinContent(imva + 1, icent + 1);
                // double raw_yield_err = h_raw_yield_2d->GetBinError(imva + 1, icent + 1);
                double raw_yield = weight[2];
                double raw_yield_err = weight[3];
             
                

                double corrected_yield = raw_yield * weight[0];
                double corrected_yield_err = raw_yield_err * weight[0];
                double corrected_yield_cb = raw_yield * weight[1];
                double corrected_yield_cb_err = raw_yield_err * weight[1];
                // double corrected_yield_gm_err = weight[3] * weight[2];
                // cout << "corrected_yield_cb: " << corrected_yield_cb << ", corrected_yield_gm: " << corrected_yield_gm << endl;
                // return;

                h_corrected_yields[ipt]->SetBinContent(imva + 1, corrected_yield);
                h_corrected_yields[ipt]->SetBinError(imva + 1, corrected_yield_err);
                h_corrected_yields_cb[ipt]->SetBinContent(imva + 1, corrected_yield_cb);
                h_corrected_yields_cb[ipt]->SetBinError(imva + 1, corrected_yield_cb_err);
                // h_corrected_yields_gm[ipt]->SetBinError(imva + 1, corrected_yield_gm_err);
                h_weights[ipt]->SetBinContent(imva + 1, weight[0]);
                h_weights_cb[ipt]->SetBinContent(imva + 1, weight[1]);
                h_raw_yields[ipt]->SetBinContent(imva + 1, raw_yield);
                h_raw_yields[ipt]->SetBinError(imva + 1, raw_yield_err);
                ipt++;
            }
            delete effmap;
    }
    for(size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
        std::vector<TH1D*> hists_to_label = {h_corrected_yields[ipt], h_weights[ipt], h_raw_yields[ipt],h_gen_yield_pass_ncoll[ipt], h_corrected_yields_cb[ipt], h_weights_cb[ipt], h_corrected_yields_gm[ipt]};
        for (TH1D* h : hists_to_label) {
            if (!h) continue;
            TAxis* xAxis = h->GetXaxis();
            for (int bin_idx = 1; bin_idx <= xAxis->GetNbins(); ++bin_idx) {
                if (bin_idx -1 < cosBin.size() && bin_idx < cosBin.size()) {
                     std::string binLabel = Form("%.3f", cosBin[bin_idx-1]);
                    xAxis->SetBinLabel(bin_idx, binLabel.c_str());
                    
                    xAxis->ChangeLabel(bin_idx, 305., -1, -1, -1, -1, binLabel);
                    


                }
            }
            xAxis->SetLabelOffset(0.01);
        }
    }

    TLatex latex;
    gStyle->SetOptStat(0);
    latex.SetNDC();
    latex.SetTextSize(0.04);

    for(size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
        std::string ptLabel = std::to_string((int)ptBins[ipt].first) + "to" + std::to_string((int)ptBins[ipt].second);

        TCanvas *c_raw = new TCanvas(("c_raw_pt_" + ptLabel).c_str(), ("Raw Yield pt " + ptLabel).c_str(), 800, 600);
        TLegend *leg1 = new TLegend(0.11, 0.11, 0.24, 0.24);
        leg1->AddEntry(h_raw_yields[ipt], "Raw Yield", "p");
        // leg1->AddEntry(h_raw_yields_gm[ipt], "Gen matched Yield", "p");
        leg1->SetBorderSize(0);
        h_raw_yields[ipt]->GetYaxis()->SetRangeUser(h_raw_yields[ipt]->GetMinimum()*0.2,h_raw_yields[ipt]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_raw_yields[ipt]->Draw("PE1");
        // h_raw_yields_gm[ipt]->Draw("PE1same");
        latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f", ptBins[ipt].first, ptBins[ipt].second));
        leg1->Draw("same");
        c_raw->SetGrid(1,1);
        c_raw->SaveAs(Form("splot/splot_raw_yield_pt_%s_%s.png",subStr.c_str(),ptLabel.c_str()));
        // delete leg1;

        TCanvas *c_yield = new TCanvas(("c_yield_pt_" + ptLabel).c_str(), ("Corrected Yield pt " + ptLabel).c_str(), 800, 600);
        h_corrected_yields[ipt]->GetYaxis()->SetRangeUser(h_corrected_yields[ipt]->GetMinimum()*0.2,h_corrected_yields[ipt]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_corrected_yields[ipt]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f", ptBins[ipt].first, ptBins[ipt].second));
        c_yield->SetGrid(1,1);
        // c_yield->SaveAs(("splot_corrected_yield_pt_MC_ncoll" + ptLabel + ".png").c_str());
        c_yield->SaveAs(Form("splot/splot_corrected_yield_pt_%s_%s.png",subStr.c_str(),ptLabel.c_str()));
        // delete c_yield;
        TCanvas *c_yield_cb = new TCanvas(("c_yield_pt_cb_" + ptLabel).c_str(), ("Corrected Yield CutBased pt " + ptLabel).c_str(), 800, 600);
        h_corrected_yields_cb[ipt]->GetYaxis()->SetRangeUser(h_corrected_yields_cb[ipt]->GetMinimum()*0.2,h_corrected_yields_cb[ipt]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_corrected_yields_cb[ipt]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f", ptBins[ipt].first, ptBins[ipt].second));
        c_yield_cb->SetGrid(1,1);
        // c_yield_cb->SaveAs(("splot_corrected_yield_pt_cb_MC_ncoll" + ptLabel + ".png").c_str());
        c_yield_cb->SaveAs(Form("splot/splot_corrected_yield_pt_cb_%s_%s.png",subStr.c_str(),ptLabel.c_str()));


        

        TCanvas *c_weight = new TCanvas(("c_weight_pt_" + ptLabel).c_str(), ("Weighting Factor pt " + ptLabel).c_str(), 800, 600);
        h_weights[ipt]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f", ptBins[ipt].first, ptBins[ipt].second));
        c_weight->SetGrid(1,1);
        // c_weight->SaveAs(("splot_weight_pt_MC_ncoll" + ptLabel + ".png").c_str());
        c_weight->SaveAs(Form("splot/splot_weight_pt_%s_%s.png",subStr.c_str(),ptLabel.c_str()));

        // delete c_weight;
        TCanvas *c_weight_cb = new TCanvas(("c_weight_pt_cb_" + ptLabel).c_str(), ("Weighting Factor CutBased pt " + ptLabel).c_str(), 800, 600);
        h_weights_cb[ipt]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f", ptBins[ipt].first, ptBins[ipt].second));
        c_weight_cb->SetGrid(1,1);
        // c_weight_cb->SaveAs(("splot_weight_pt_cb_MC_ncoll" + ptLabel + ".png").c_str());
        c_weight_cb->SaveAs(Form("splot/splot_weight_pt_cb_%s_%s.png",subStr.c_str(),ptLabel.c_str()));


        TCanvas *c_comp = new TCanvas(("c_comp_pt_" + ptLabel).c_str(), ("Yield Comparison pt " + ptLabel).c_str(), 800, 600);
        // h_corrected_yields[ipt]->GetYaxis()->SetRangeUser(h_corrected_yields[ipt]->GetMinimum()*0.2,h_corrected_yields[ipt]->GetBinContent(1) * 1.2); // Adjust max for visibility

        TLegend *leg = new TLegend(0.11, 0.11, 0.31, 0.31);
        leg->SetBorderSize(0);
        leg->AddEntry(h_corrected_yields[ipt], "Corrected Yield", "pe");
        leg->AddEntry(h_raw_yields[ipt], "Raw Yield(Scaled)", "pe");
        // leg->AddEntry(h_gen_yield_pass_ncoll[ipt], "GEN Yield", "pe");
        leg->AddEntry(h_corrected_yields_cb[ipt], "Corrected Yield CutBased", "pe");
        // leg->AddEntry(h_corrected_yields_gm[ipt], "Corrected Yield GM", "pe");
        latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f", ptBins[ipt].first, ptBins[ipt].second));
        // legend->Draw();
    
        // // 캔버스 저장 (선택 사항)
        // canvas->SaveAs("histogram_comparison.png");
        // Create pads
        // h_raw_yields[ipt]->Scale(h_corrected_yields_cb[ipt]->GetBinContent(1)/h_raw_yields[ipt]->GetBinContent(1));
        h_raw_yields[ipt]->SetLineColor(kRed);
        h_raw_yields[ipt]->SetMarkerColor(kRed);
        h_raw_yields[ipt]->SetMarkerStyle(24);
        h_raw_yields[ipt]->SetMarkerSize(1.0);
        h_corrected_yields[ipt]->SetLineColor(kBlack);
        h_corrected_yields[ipt]->SetMarkerColor(kBlack);
        h_corrected_yields[ipt]->SetMarkerStyle(20);
        h_corrected_yields[ipt]->SetMarkerSize(1.0);
        
        h_corrected_yields_cb[ipt]->SetLineColor(kGreen);
        h_corrected_yields_cb[ipt]->SetMarkerColor(kGreen);
        h_corrected_yields_cb[ipt]->SetMarkerStyle(20);
        h_corrected_yields_cb[ipt]->SetMarkerSize(1.0);
        

        
        double maxyield = std::max({h_corrected_yields[ipt]->GetMaximum(),  h_corrected_yields_cb[ipt]->GetMaximum()});
        double minyield = std::min({h_corrected_yields[ipt]->GetMinimum(), h_corrected_yields_cb[ipt]->GetMinimum()});
        // h_corrected_yields[ipt]->GetYaxis()->SetRangeUser(0.8*minyield, maxyield * 1.2); // Adjust max for visibility
        h_corrected_yields_cb[ipt]->GetYaxis()->SetRangeUser(0.8*minyield, maxyield * 1.2); // Adjust max for visibility
        h_corrected_yields_cb[ipt]->GetXaxis()->SetTitle("cos(#theta)");

        
        h_corrected_yields_cb[ipt]->Draw("PE1");
        h_corrected_yields[ipt]->Draw("PE1 SAME");
        // h_raw_yields[ipt]->Draw("PE1 SAME");
        leg->Draw();
        latex.DrawLatex(0.25, 0.85, Form("pt = %.1f to %.1f", ptBins[ipt].first, ptBins[ipt].second));
        c_comp->SetGrid(1,1);
        c_comp->SaveAs(("splot/splot_yield_comparison_pt_" + ptLabel + ".png").c_str());
        // delete leg;
        // delete c_comp;
    }

    // for(size_t ipt = 0; ipt < ptBins.size(); ++ipt) {
    //     delete h_corrected_yields[ipt];
    //     delete h_weights[ipt];
    //     delete h_raw_yields[ipt];
    // }

    // yieldFile->Close();
    effFile->Close();

    std::cout << "Finished drawing and saving histograms." << std::endl;
}
