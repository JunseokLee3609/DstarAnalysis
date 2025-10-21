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
        if(!pt_var || !y_var) {
             std::cerr << "Event " << idx << ": Missing pT or y variable!" << std::endl;
             continue;
        }

        double corr_ = 0.0;
        if (hist_eff) {
             corr_ = hist_eff->GetBinContent(hist_eff->FindBin(pt_var->getVal(), y_var->getVal(), mvaCut+1e-9));
        } else {
            std::cerr << "Error: hist_eff is null!" << std::endl;
        }

        sWeight_val = wds_sig->weight();
        if (fabs(sWeight_val) > 1e-9 && fabs(corr_) > 1e-9) {
             corr_ /= sWeight_val;
             corr2 += 1.0/corr_;
             count2+= sWeight_val;
        }
    }

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

void DrawSplotHist2D(){
    TFile *effFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output.root","read");
    if (!effFile || effFile->IsZombie()) {
        std::cerr << "Error opening efficiency file." << std::endl;
        return;
    }
    TH3D *effmap = (TH3D*)effFile->Get("pt_y_mva_pr_ratio");
    if (!effmap) {
        std::cerr << "Error getting efficiency histogram." << std::endl;
        effFile->Close();
        return;
    }

    TFile *yieldFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/D0_Yield_mva.root","read");
     if (!yieldFile || yieldFile->IsZombie()) {
        std::cerr << "Error opening yield file." << std::endl;
        effFile->Close();
        return;
    }
    TH2D *h_raw_yield_2d = (TH2D*)yieldFile->Get("D0_Yield_mva");
     if (!h_raw_yield_2d) {
        std::cerr << "Error getting raw yield histogram." << std::endl;
        yieldFile->Close();
        effFile->Close();
        return;
    }

    FitOpt D0opt;
    std::string particleType = "D0";
    std::string subDir = "D0_Apr30/";
    std::vector<std::pair<double,double>> ptBins = {
        {2,4},
        {4,9},
        {9,50}
    };
    std::vector<std::pair<double,double>> centBins = {
        {0,90}
    };
    std::vector<double> mvaBinEdges = {0.90,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    // std::vector<double> mvaBinEdges = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    std::vector<double> mvaBinCenters;
    for(size_t i = 0; i < mvaBinEdges.size() - 1; ++i) {
        mvaBinCenters.push_back( (mvaBinEdges[i] + mvaBinEdges[i+1]) / 2.0 );
    }

    std::vector<TH1D*> h_corrected_yields(ptBins.size());
    std::vector<TH1D*> h_weights(ptBins.size());
    std::vector<TH1D*> h_raw_yields(ptBins.size());

    for(size_t ipT = 0; ipT < ptBins.size(); ++ipT) {
        std::string ptLabel = std::to_string((int)ptBins[ipT].first) + "to" + std::to_string((int)ptBins[ipT].second);
        h_corrected_yields[ipT] = new TH1D(("corrected_yield_pt_" + ptLabel).c_str(),
                                           ("Corrected Yield vs MVA; MVA Cut; Corrected Yield (" + ptLabel + " GeV/c)").c_str(),
                                           mvaBinEdges.size()-1, mvaBinEdges.data());
        h_weights[ipT] = new TH1D(("weight_pt_" + ptLabel).c_str(),
                                  ("Weighting Factor vs MVA; MVA Cut; Weight (" + ptLabel + " GeV/c)").c_str(),
                                  mvaBinEdges.size()-1, mvaBinEdges.data());
        h_raw_yields[ipT] = new TH1D(("raw_yield_pt_" + ptLabel).c_str(),
                                     ("Raw Yield vs MVA; MVA Cut; Raw Yield (" + ptLabel + " GeV/c)").c_str(),
                                     mvaBinEdges.size()-1, mvaBinEdges.data());
        h_corrected_yields[ipT]->SetMarkerStyle(20);
        h_corrected_yields[ipT]->SetMarkerSize(1.0);
        h_weights[ipT]->SetMarkerStyle(20);
        h_weights[ipT]->SetMarkerSize(1.0);
        h_raw_yields[ipT]->SetMarkerStyle(24);
        h_raw_yields[ipT]->SetMarkerSize(1.0);
        h_raw_yields[ipT]->SetLineColor(kRed);
        h_raw_yields[ipT]->SetMarkerColor(kRed);
    }

    for(size_t imva = 0; imva < mvaBinEdges.size() - 1; ++imva) {
         double mva_cut = mvaBinEdges[imva];

        for(size_t ipT = 0; ipT < ptBins.size(); ++ipT) {
            for(auto centbin : centBins){
                D0opt.pTMin = ptBins[ipT].first;
                D0opt.pTMax = ptBins[ipT].second;
                D0opt.centMin = centbin.first;
                D0opt.centMax = centbin.second;
                D0opt.mvaMin = mva_cut;
                D0opt.D0MCDefault();
                D0opt.outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/"+D0opt.outputDir;

                double corr = CalSplotweight(D0opt, effmap, mva_cut, subDir);

                double raw_yield = h_raw_yield_2d->GetBinContent(imva + 1, ipT + 1);
                double raw_yield_err = h_raw_yield_2d->GetBinError(imva + 1, ipT + 1);

                double corrected_yield = raw_yield * corr;
                double corrected_yield_err = raw_yield_err * corr;

                h_corrected_yields[ipT]->SetBinContent(imva + 1, corrected_yield);
                h_corrected_yields[ipT]->SetBinError(imva + 1, corrected_yield_err);
                h_weights[ipT]->SetBinContent(imva + 1, corr);
                h_raw_yields[ipT]->SetBinContent(imva + 1, raw_yield);
                h_raw_yields[ipT]->SetBinError(imva + 1, raw_yield_err);
            }
        }
    }

    gStyle->SetOptStat(0);

    for(size_t ipT = 0; ipT < ptBins.size(); ++ipT) {
        std::string ptLabel = std::to_string((int)ptBins[ipT].first) + "to" + std::to_string((int)ptBins[ipT].second);
        TLatex latex;

        TCanvas *c_yield = new TCanvas(("c_yield_pt_" + ptLabel).c_str(), ("Corrected Yield pt " + ptLabel).c_str(), 800, 600);
        h_corrected_yields[ipT]->GetYaxis()->SetRangeUser(h_corrected_yields[ipT]->GetMinimum()*0.2,h_corrected_yields[ipT]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_corrected_yields[ipT]->Draw("PE1");
        latex.DrawLatex(0.1, 0.9, Form("p_{T} = %.1f to %.1f GeV/c", ptBins[ipT].first, ptBins[ipT].second));
        c_yield->SetGrid(1,1);
        c_yield->SaveAs(("splot_corrected_yield_pt_" + ptLabel + ".png").c_str());
        delete c_yield;

        TCanvas *c_weight = new TCanvas(("c_weight_pt_" + ptLabel).c_str(), ("Weighting Factor pt " + ptLabel).c_str(), 800, 600);
        h_weights[ipT]->Draw("PE1");
        latex.DrawLatex(0.1, 0.9, Form("p_{T} = %.1f to %.1f GeV/c", ptBins[ipT].first, ptBins[ipT].second));
        c_weight->SetGrid(1,1);
        c_weight->SaveAs(("splot_weight_pt_" + ptLabel + ".png").c_str());
        delete c_weight;

        TCanvas *c_comp = new TCanvas(("c_comp_pt_" + ptLabel).c_str(), ("Yield Comparison pt " + ptLabel).c_str(), 800, 600);
        // h_corrected_yields[ipT]->GetYaxis()->SetRangeUser(h_corrected_yields[ipT]->GetMinimum()*0.2,h_corrected_yields[ipT]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_raw_yields[ipT]->Scale(h_corrected_yields[ipT]->GetBinContent(1)/h_raw_yields[ipT]->GetBinContent(1));
        

        
        h_corrected_yields[ipT]->Draw("PE1");
        h_raw_yields[ipT]->Draw("PE1 SAME");
        TLegend *leg = new TLegend(0.7, 0.8, 0.9, 0.9);
        leg->AddEntry(h_corrected_yields[ipT], "Corrected Yield", "pe");
        leg->AddEntry(h_raw_yields[ipT], "Raw Yield", "pe");
        leg->Draw();
        latex.DrawLatex(0.1, 0.9, Form("p_{T} = %.1f to %.1f GeV/c", ptBins[ipT].first, ptBins[ipT].second));
        c_comp->SetGrid(1,1);
        c_comp->SaveAs(("splot_yield_comparison_pt_" + ptLabel + ".png").c_str());
        delete leg;
        delete c_comp;
    }

    for(size_t ipT = 0; ipT < ptBins.size(); ++ipT) {
        delete h_corrected_yields[ipT];
        delete h_weights[ipT];
        delete h_raw_yields[ipT];
    }

    yieldFile->Close();
    effFile->Close();
    delete yieldFile;
    delete effFile;

    std::cout << "Finished drawing and saving histograms." << std::endl;
}
