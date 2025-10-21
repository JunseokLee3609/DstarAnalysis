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
#include "../interface/simpleDMC.h"
// #include <TSeq.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath> // for fabs
float getEfficiencyVal(double pt, double y, double centrality, TH3D *effmap) {
    float eff = 0.001; // Default minimum efficiency to avoid division by zero
    int bin = effmap->FindBin(pt, y, centrality);
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

    // Return the INVERSE efficiency (weight)
    return 1.0 / eff;
}

std::vector<double> CalSplotweight(FitOpt D0Opt, TH3D* hist_eff, double mvaCut, std::string subDir){
    TString subStr = "";
    TString kinCutStr = Form("mass > 1.84 && mass < 1.88 && pT > %f && pT < %f && Centrality >= %f && Centrality < %f && abs(y) < 1.0",D0Opt.pTMin, D0Opt.pTMax, D0Opt.centMin, D0Opt.centMax);
    D0Opt.outputDir = D0Opt.outputDir + subDir + "/";

    gStyle->SetOptStat(0);
    TFile *FitResultFile = TFile::Open((D0Opt.outputDir+"/"+D0Opt.outputFile).c_str());
    if (!FitResultFile || FitResultFile->IsZombie()) {
        std::cerr << "Error: Could not open file " << (D0Opt.outputDir+"/"+D0Opt.outputFile) << std::endl;
        // return 0.0;
    }
    RooWorkspace* ws = (RooWorkspace*)FitResultFile->Get("ws_D0");
    if (!ws) {
        std::cerr << "Error: Could not retrieve workspace ws_D0 from file." << std::endl;
        FitResultFile->Close();
        delete FitResultFile;
        // return 0.0;
    }
    RooAddPdf *total_pdf = (RooAddPdf*)ws->pdf("total_pdf");
    RooDataSet *data = (RooDataSet*)ws->data("dataset");
    RooRealVar *nSig = (RooRealVar*)ws->var("nsig");
    RooRealVar *nBkg = (RooRealVar*)ws->var("nbkg");
    RooRealVar *mass = (RooRealVar*)ws->var("mass");
    RooDataSet* subset = (RooDataSet*)data->reduce(kinCutStr);
    RooDataSet* subsetgm = (RooDataSet*)data->reduce("matchGEN == 1 && !isSwap");

    long long nEntriesSubset = subset->numEntries();
    long long nEntriesSubsetgm = subsetgm->numEntries();
    std::cout << "  Subset size: " << nEntriesSubset << " entries." << std::endl;

    // if (nEntriesSubset == 0) {
    //     // avgInvEffMap[mvaCut] = 1.0; // Assign default weight (or handle as needed)
    //     std::cout << "  No entries passed the cut. Assigning default average inverse efficiency = 1.0" << std::endl;
    //     delete subset; // Clean up the empty subset
    //     continue;
    // }

    double sumInvEff = 0.0;
    long long countValidEff = 0;
    double ncoll_ =0;

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
        const RooAbsArg* centArg = row->find("Centrality");
        const RooAbsArg* ncolArg = row->find("Ncoll");

        if (!pTArg || !yArg) {
             std::cerr << "Warning: Could not find pT or y variable in row " << i << " for MVA cut " << mvaCut << std::endl;
             continue; // Skip this event
        }

        // Cast to RooRealVar to get value (more robust)
         const RooRealVar* pTVar = dynamic_cast<const RooRealVar*>(pTArg);
         const RooRealVar* yVar = dynamic_cast<const RooRealVar*>(yArg);
         const RooRealVar* centVar = dynamic_cast<const RooRealVar*>(centArg);
            const RooRealVar* ncolVar = dynamic_cast<const RooRealVar*>(ncolArg);
         if (!pTVar || !yVar) {
            std::cerr << "Warning: Could not cast pT or y to RooRealVar in row " << i << " for MVA cut " << mvaCut << std::endl;
            continue; // Skip this event
        }

        double currentPt = pTVar->getVal();
        double currentY = yVar->getVal();
        double currentCent = centVar->getVal();
        double ncoll = ncolVar->getVal();

        ncoll_ += ncoll;



        // Get efficiency using the EVENT's pT, y, but the MVA CUT value (+offset)
        // This mimics the original code's logic
        float invEff = getEfficiencyVal(currentPt, currentY, currentCent, hist_eff);

        sumInvEff += invEff;
        countValidEff ++; // Count events for which we calculated a weight

       // Optional: Print progress/values for debugging
       // if (i % 10000 == 0 && i > 0) {
       //     std::cout << "    Processed " << i << " events in subset for MVA cut " << mvaCut << std::endl;
       // }
    }

    double sumInvEffgm = 0.0;
    long long countValidEffgm = 0;
    double ncollgm_ =0;

    // Iterate through the selected events (the subset)
    for (int i = 0; i < nEntriesSubsetgm; ++i) {
        // Get the variables for the current event
        const RooArgSet* row = subsetgm->get(i);
        if (!row) {
             std::cerr << "Warning: Could not get row " << i << " from subset for MVA cut " << mvaCut << std::endl;
             continue; // Skip this event
        }

        // Extract pT and y (use find() for safety)
        const RooAbsArg* pTArg = row->find("pT");
        const RooAbsArg* yArg = row->find("y");
        const RooAbsArg* centArg = row->find("Centrality");
        const RooAbsArg* ncolArg = row->find("Ncoll");

        if (!pTArg || !yArg) {
             std::cerr << "Warning: Could not find pT or y variable in row " << i << " for MVA cut " << mvaCut << std::endl;
             continue; // Skip this event
        }

        // Cast to RooRealVar to get value (more robust)
         const RooRealVar* pTVar = dynamic_cast<const RooRealVar*>(pTArg);
         const RooRealVar* yVar = dynamic_cast<const RooRealVar*>(yArg);
         const RooRealVar* centVar = dynamic_cast<const RooRealVar*>(centArg);
         const RooRealVar* ncolVar = dynamic_cast<const RooRealVar*>(ncolArg);
         if (!pTVar || !yVar) {
            std::cerr << "Warning: Could not cast pT or y to RooRealVar in row " << i << " for MVA cut " << mvaCut << std::endl;
            continue; // Skip this event
        }

        double currentPt = pTVar->getVal();
        double currentY = yVar->getVal();
        double currentCent = centVar->getVal();
        double ncoll = ncolVar->getVal();
        ncollgm_ += ncoll;


        // Get efficiency using the EVENT's pT, y, but the MVA CUT value (+offset)
        // This mimics the original code's logic
        float invEff = getEfficiencyVal(currentPt, currentY, currentCent, hist_eff);

        sumInvEffgm += invEff;
        countValidEffgm++;

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

    RooStats::SPlot splot("splot", "", *data,
        total_pdf,
        RooArgList(*nSig, *nBkg),
        RooArgSet(),
        false,
        true,
        "rd_total",
        RooFit::NumCPU(24, 0),
        RooFit::Range("analysis")
    );

    auto newrd = splot.GetSDataSet();
    if (!newrd) {
        std::cerr << "Error: Could not get sWeighted dataset." << std::endl;
        FitResultFile->Close();
        delete FitResultFile;
        // return 0.0;
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
        RooRealVar *ncoll = (RooRealVar*)row->find("Ncoll");
        RooRealVar *Centrality = (RooRealVar*)row->find("Centrality");
        if(!pt_var || !y_var) {
             std::cerr << "Event " << idx << ": Missing pT or y variable!" << std::endl;
             continue;
        }

        double corr_ = 0.0;
        if (hist_eff) {
             corr_ = hist_eff->GetBinContent(hist_eff->FindBin(pt_var->getVal(), y_var->getVal(), Centrality->getVal()));
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
    return std::vector<double> {corr2/count2, (sumInvEff/countValidEff), sumInvEffgm / countValidEffgm, (double)nEntriesSubsetgm, (double)nSig->getVal(), (double)nSig->getError()};
    delete wds_sig;
    FitResultFile->Close();
    delete FitResultFile;

    if (fabs(count2) > 1e-9) {
        // return corr2 / count2;
    } else {
        std::cerr << "Warning: count2 is zero or very small, returning 0." << std::endl;
        // return 0.0;
    }
}

void DrawSplotHist2D_centMC(){
    TFile *effFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output_MC_May14_noncoll_v1.root","read");
    // TFile *effFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output_MC_May14_ncoll_v1.root","read");
    std::string subDir = "D0_May14_MC_pol1_v1/";
    std::string subStr = "bkgpol1";
    if (!effFile || effFile->IsZombie()) {
        std::cerr << "Error opening efficiency file." << std::endl;
        return;
    }
    TFile *yieldFile =TFile::Open(Form("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/roots/Data_D0_PbPb/%s/D0_Yield_mva.root",subDir.c_str()),"read");
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
    const char* fileMCPR = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/D0/flatSkimForBDT_D0_PbPb_MC_ONNX_withOfficialCent_DpT018_30Apr25.root";
    const double Direction_y = 1.0; // Match the setting in FlatEffAccCalculator_D0.cpp if needed
    std::cout << "Calculating GEN level yield..." << std::endl;
    TFile* fMCPR_gen = TFile::Open(fileMCPR);
    if (!fMCPR_gen || fMCPR_gen->IsZombie()) { std::cerr << "Error opening MC PR file for GEN: " << fileMCPR << std::endl; /* cleanup */ return; }
    TTree* genTree = (TTree*)fMCPR_gen->Get("skimGENTreeFlat");
    if (!genTree) { std::cerr << "Error: Could not find skimGENTreeFlat." << std::endl; fMCPR_gen->Close(); /* cleanup */ return; }

    DataFormat::simpleDMCTreeflat evtGenFlat;
    evtGenFlat.setGENTree(genTree);
    long long numGenEntries = genTree->GetEntries();

    // Create histogram for Ncoll-weighted GEN yield passing cuts vs centrality
    // Need to define centrality bins matching centBins vector
    // std::vector<double> centEdges;
    // centEdges.push_back(centBins[0].first);
    // for(const auto& bin : centBins) { centEdges.push_back(bin.second); }
    // std::vector<double> mvaBin= {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    std::vector<double> mvaBin= {0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    // std::vector<double> mvaBinEdges = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    // std::vector<double> mvaBinEdges = {0.8,0.90,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    std::vector<double> mvaBinEdges; 
    for(int i =0; i<mvaBin.size(); i++){
        mvaBinEdges.push_back(mvaBin[i]);
        if(mvaBin[i] == mvaBin.back()){
            mvaBinEdges.push_back(1.0);
        }
    }
    
    // h_gen_yield_pass_ncoll->Sumw2(); // Enable error calculation
    // int jmva = 0;


    // fMCPR_gen->Close(); // Close the GEN file
    // // delete fMCPR_gen;
    std::cout << "GEN level yield calculation finished." << std::endl;
    // std::string subDir = "D0_Apr30/";
    std::vector<std::pair<double,double>> ptBins = {
        {2,50}
        
    };
    std::vector<std::pair<double,double>> centBins = {
        {0,90},
        {0,10},
        {10,40},
        {40,90}
        // {0,10}
    };

    std::vector<TH1D*> h_corrected_yields(centBins.size());
    std::vector<TH1D*> h_corrected_yields_cb(centBins.size());
    std::vector<TH1D*> h_corrected_yields_gm(centBins.size());
    std::vector<TH1D*> h_weights(centBins.size());
    std::vector<TH1D*> h_weights_cb(centBins.size());
    std::vector<TH1D*> h_weights_gm(centBins.size());
    std::vector<TH1D*> h_raw_yields(centBins.size());
    std::vector<TH1D*> h_raw_yields_gm(centBins.size());
    std::vector<TH1D*> h_gen_yield_pass_ncoll(centBins.size());
    for(size_t icent = 0; icent < centBins.size(); icent++) {
        std::string centLabel = std::to_string((int)centBins[icent].first) + "to" + std::to_string((int)centBins[icent].second);
        h_corrected_yields[icent] = new TH1D(("corrected_yield_cent_" + centLabel).c_str(),
                                           ("Corrected Yield vs MVA; MVA Cut; Corrected Yield (" + centLabel + " %)").c_str(),
                                           mvaBinEdges.size()-1, 0,1);
        h_corrected_yields_cb[icent] = new TH1D(("corrected_yield_cent_cb_" + centLabel).c_str(),
                                           ("Corrected Yield vs MVA; MVA Cut; Corrected Yield (" + centLabel + " %)").c_str(),
                                           mvaBinEdges.size()-1, 0,1);
        h_corrected_yields_gm[icent] = new TH1D(("corrected_yield_cent_gm_" + centLabel).c_str(),
                                           ("Corrected Yield vs MVA; MVA Cut; Corrected Yield (" + centLabel + " %)").c_str(),  
                                             mvaBinEdges.size()-1, 0,1);
        h_weights[icent] = new TH1D(("weight_cent_" + centLabel).c_str(),
                                  ("Weighting Factor vs MVA; MVA Cut; Weight (" + centLabel + " %)").c_str(),
                                  mvaBinEdges.size()-1, 0,1);
        h_weights_cb[icent] = new TH1D(("weight_cent_cb_" + centLabel).c_str(),
                                  ("Weighting Factor vs MVA; MVA Cut; Weight (" + centLabel + " %)").c_str(),
                                  mvaBinEdges.size()-1, 0,1);
        h_weights_gm[icent] = new TH1D(("weight_gm_cent_" + centLabel).c_str(),
                                  ("Weighting Factor vs MVA; MVA Cut; Weight (" + centLabel + " %)").c_str(),
                                  mvaBinEdges.size()-1, 0,1);
        h_raw_yields[icent] = new TH1D(("raw_yield_cent_" + centLabel).c_str(),
                                     ("Raw Yield vs MVA; MVA Cut; Raw Yield (" + centLabel + " %)").c_str(),
                                     mvaBinEdges.size()-1,0,1);
        h_raw_yields_gm[icent] = new TH1D(("raw_yield_gm_cent_" + centLabel).c_str(),
                                     ("Raw Yield vs MVA; MVA Cut; Raw Yield (" + centLabel + " %)").c_str(),
                                        mvaBinEdges.size()-1,0,1);
        h_gen_yield_pass_ncoll[icent] = new TH1D(("gen_yield_pass_ncoll_cent_" + centLabel).c_str(),
                                           ("GEN Yield vs MVA; MVA Cut; GEN Yield (" + centLabel + " %)").c_str(),
                                           mvaBinEdges.size()-1, 0,1);
        h_gen_yield_pass_ncoll[icent]->SetLineColor(kBlue);
        h_gen_yield_pass_ncoll[icent]->SetMarkerColor(kBlue);
        h_gen_yield_pass_ncoll[icent]->SetMarkerStyle(20);
        h_gen_yield_pass_ncoll[icent]->SetMarkerSize(1.0);
        h_corrected_yields[icent]->SetMarkerStyle(20);
        h_corrected_yields[icent]->SetMarkerSize(1.0);
        h_corrected_yields_cb[icent]->SetMarkerStyle(20);
        h_corrected_yields_cb[icent]->SetMarkerSize(1.0);
        h_weights[icent]->SetMarkerStyle(20);
        h_weights[icent]->SetMarkerSize(1.0);
        h_weights_cb[icent]->SetMarkerStyle(20);
        h_weights_cb[icent]->SetMarkerSize(1.0);
        h_raw_yields[icent]->SetMarkerStyle(24);
        h_raw_yields[icent]->SetMarkerSize(1.0);
        h_raw_yields[icent]->SetLineColor(kRed);
        h_raw_yields[icent]->SetMarkerColor(kRed);
        h_raw_yields_gm[icent]->SetMarkerStyle(24);
        h_raw_yields_gm[icent]->SetMarkerSize(1.0);
        h_raw_yields_gm[icent]->SetLineColor(kAzure-9);
        h_raw_yields_gm[icent]->SetMarkerColor(kAzure-9);
        int counts =0;


        for( auto idx : ROOT::TSeqUL(numGenEntries)){ // Use TSeqUL for unsigned long long
            if (idx % 500000 == 0) std::cout << " GEN Entry: " << idx << "/" << numGenEntries << std::endl;
            genTree->GetEntry(idx);
            // double dzW = 1; // Ncoll weight from MC event
            // int icent = 0;
            if(evtGenFlat.centrality/2 < centBins[icent].first || evtGenFlat.centrality/2 >= centBins[icent].second || evtGenFlat.gen_pT<2 || evtGenFlat.gen_pT> 50 ||abs(evtGenFlat.y)>1) continue;
            counts ++;
                // icent++;
        
        }    
        cout << "counts: " << counts << endl;
        for(int i =0; i<mvaBinEdges.size(); i++){
            h_gen_yield_pass_ncoll[icent]->SetBinContent(i+1, counts);
            h_gen_yield_pass_ncoll[icent]->SetBinError(i+1, sqrt(counts));
        }

    }


    // for(size_t imva = 0; imva < mvaBinEdges.size() - 1; ++imva) {
    // std::vector<double> mvaBinEdges = {0.90,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999, 1.0};
    
    std::vector<double> mvaBinCenters;
    for(size_t i = 0; i < mvaBinEdges.size() - 1; ++i) {
        mvaBinCenters.push_back( (mvaBinEdges[i] + mvaBinEdges[i+1]) / 2.0 );
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
                if(centbin.first == 0 && centbin.second == 90) {
                    h_raw_yield_2d=(TH2D*)yieldFile->Get("D0_Yield_mva_cent");
                } 
                else{
                    h_raw_yield_2d=(TH2D*)yieldFile->Get("D0_Yield_mva_cent");
                }
                D0opt.mvaMin = mva_cut;
                D0opt.D0MCDefault();
                D0opt.outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/"+D0opt.outputDir;
                

                std::vector<double> weight = CalSplotweight(D0opt, effmap, mva_cut, subDir);

                // cout << "cent " << centbin.first << " to " << centbin.second << " mva " << mva_cut << " corr " << corr << endl;

                // double raw_yield = h_raw_yield_2d->GetBinContent(imva + 1, icent + 1);
                // double raw_yield_err = h_raw_yield_2d->GetBinError(imva + 1, icent + 1);
                double raw_yield = weight[4];
                double raw_yield_err = weight[5];
                

                double corrected_yield = raw_yield * weight[0];
                double corrected_yield_err = raw_yield_err * weight[0];
                double corrected_yield_cb = raw_yield * weight[1];
                double corrected_yield_cb_err = raw_yield_err * weight[1];
                double corrected_yield_gm = weight[3] * weight[2];
                // double corrected_yield_gm_err = weight[3] * weight[2];

                h_corrected_yields[icent]->SetBinContent(imva + 1, corrected_yield);
                h_corrected_yields[icent]->SetBinError(imva + 1, corrected_yield_err);
                h_corrected_yields_cb[icent]->SetBinContent(imva + 1, corrected_yield_cb);
                h_corrected_yields_cb[icent]->SetBinError(imva + 1, corrected_yield_cb_err);
                h_corrected_yields_gm[icent]->SetBinContent(imva + 1, corrected_yield_gm);
                // h_corrected_yields_gm[icent]->SetBinError(imva + 1, corrected_yield_gm_err);
                h_weights[icent]->SetBinContent(imva + 1, weight[0]);
                h_weights_cb[icent]->SetBinContent(imva + 1, weight[1]);
                h_weights_gm[icent]->SetBinContent(imva + 1, weight[2]);
                h_raw_yields[icent]->SetBinContent(imva + 1, raw_yield);
                h_raw_yields[icent]->SetBinError(imva + 1, raw_yield_err);
                h_raw_yields_gm[icent]->SetBinContent(imva + 1, weight[3]);
                icent++;
            }
            delete effmap;
    }
    for(size_t icent = 0; icent < ptBins.size(); ++icent) {
        std::vector<TH1D*> hists_to_label = {h_corrected_yields[icent], h_weights[icent], h_raw_yields[icent]};
        for (TH1D* h : hists_to_label) {
            if (!h) continue;
            TAxis* xAxis = h->GetXaxis();
            for (int bin_idx = 1; bin_idx <= xAxis->GetNbins(); ++bin_idx) {
                if (bin_idx -1 < mvaBinEdges.size() && bin_idx < mvaBinEdges.size()) {
                     std::string binLabel = Form("%.3f", mvaBinEdges[bin_idx-1]);
                    xAxis->SetBinLabel(bin_idx, binLabel.c_str());
                    
                    xAxis->ChangeLabel(bin_idx, 305., -1, -1, -1, -1, binLabel);
                    


                }
            }
            xAxis->SetLabelOffset(0.01);
        }
    }

    TLatex latex;
    // gStyle->SetOptStat(0);
    latex.SetNDC();
    latex.SetTextSize(0.04);
    

    for(size_t icent = 0; icent < centBins.size(); icent++) {
        std::string centLabel = std::to_string((int)centBins[icent].first) + "to" + std::to_string((int)centBins[icent].second);

        TCanvas *c_gen = new TCanvas("c_gen", "GEN Yield", 800, 600);
        h_gen_yield_pass_ncoll[icent]->SetLineColor(kBlue);
        h_gen_yield_pass_ncoll[icent]->SetMarkerColor(kBlue);
        h_gen_yield_pass_ncoll[icent]->SetMarkerStyle(20);
        h_gen_yield_pass_ncoll[icent]->SetMarkerSize(1.0);
        h_gen_yield_pass_ncoll[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, "GEN Yield");
        c_gen->SetGrid(1,1);
        c_gen->SaveAs(Form("splotMC/splot_gen_yield_%s_%s.png",subStr.c_str(),centLabel.c_str()));
        TCanvas *c_raw = new TCanvas(("c_raw_cent_" + centLabel).c_str(), ("Raw Yield cent " + centLabel).c_str(), 800, 600);
        TLegend *leg1 = new TLegend(0.11, 0.11, 0.24, 0.24);
        leg1->AddEntry(h_raw_yields[icent], "Raw Yield", "p");
        leg1->AddEntry(h_raw_yields_gm[icent], "Gen matched Yield", "p");
        leg1->SetBorderSize(0);
        h_raw_yields[icent]->GetYaxis()->SetRangeUser(h_raw_yields[icent]->GetMinimum()*0.2,h_raw_yields[icent]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_raw_yields[icent]->Draw("PE1");
        h_raw_yields_gm[icent]->Draw("PE1same");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        leg1->Draw("same");
        c_raw->SetGrid(1,1);
        c_raw->SaveAs(Form("splotMC/splot_raw_yield_cent_%s_%s.png",subStr.c_str(),centLabel.c_str()));
        delete leg1;

        TCanvas *c_yield = new TCanvas(("c_yield_cent_" + centLabel).c_str(), ("Corrected Yield cent " + centLabel).c_str(), 800, 600);
        h_corrected_yields[icent]->GetYaxis()->SetRangeUser(h_corrected_yields[icent]->GetMinimum()*0.2,h_corrected_yields[icent]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_corrected_yields[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_yield->SetGrid(1,1);
        // c_yield->SaveAs(("splot_corrected_yield_cent_MC_ncoll" + centLabel + ".png").c_str());
        c_yield->SaveAs(Form("splotMC/splot_corrected_yield_cent_%s_%s.png",subStr.c_str(),centLabel.c_str()));
        // delete c_yield;
        TCanvas *c_yield_cb = new TCanvas(("c_yield_cent_cb_" + centLabel).c_str(), ("Corrected Yield CutBased cent " + centLabel).c_str(), 800, 600);
        h_corrected_yields_cb[icent]->GetYaxis()->SetRangeUser(h_corrected_yields_cb[icent]->GetMinimum()*0.2,h_corrected_yields_cb[icent]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_corrected_yields_cb[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_yield_cb->SetGrid(1,1);
        // c_yield_cb->SaveAs(("splot_corrected_yield_cent_cb_MC_ncoll" + centLabel + ".png").c_str());
        c_yield_cb->SaveAs(Form("splotMC/splot_corrected_yield_cent_cb_%s_%s.png",subStr.c_str(),centLabel.c_str()));

        TCanvas *c_yield_gm = new TCanvas(("c_yield_cent_gm_" + centLabel).c_str(), ("Corrected Yield GM cent " + centLabel).c_str(), 800, 600);
        h_corrected_yields_gm[icent]->GetYaxis()->SetRangeUser(h_corrected_yields_gm[icent]->GetMinimum()*0.2,h_corrected_yields_gm[icent]->GetBinContent(1) * 1.2); // Adjust max for visibility
        h_corrected_yields_gm[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_yield_gm->SetGrid(1,1);
        // c_yield_gm->SaveAs(("splot_corrected_yield_cent_gm_MC_ncoll" + centLabel + ".png").c_str());
        c_yield_gm->SaveAs(Form("splotMC/splot_corrected_yield_cent_gm_%s_%s.png",subStr.c_str(),centLabel.c_str()));

        

        TCanvas *c_weight = new TCanvas(("c_weight_cent_" + centLabel).c_str(), ("Weighting Factor cent " + centLabel).c_str(), 800, 600);
        h_weights[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_weight->SetGrid(1,1);
        // c_weight->SaveAs(("splot_weight_cent_MC_ncoll" + centLabel + ".png").c_str());
        c_weight->SaveAs(Form("splotMC/splot_weight_cent_%s_%s.png",subStr.c_str(),centLabel.c_str()));

        // delete c_weight;
        TCanvas *c_weight_cb = new TCanvas(("c_weight_cent_cb_" + centLabel).c_str(), ("Weighting Factor CutBased cent " + centLabel).c_str(), 800, 600);
        h_weights_cb[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_weight_cb->SetGrid(1,1);
        // c_weight_cb->SaveAs(("splot_weight_cent_cb_MC_ncoll" + centLabel + ".png").c_str());
        c_weight_cb->SaveAs(Form("splotMC/splot_weight_cent_cb_%s_%s.png",subStr.c_str(),centLabel.c_str()));

        TCanvas *c_weight_gm = new TCanvas(("c_weight_cent_gm_" + centLabel).c_str(), ("Weighting Factor GM cent " + centLabel).c_str(), 800, 600);
        h_weights_gm[icent]->Draw("PE1");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        c_weight_gm->SetGrid(1,1);
        // c_weight_gm->SaveAs(("splot_weight_cent_gm_MC_ncoll" + centLabel + ".png").c_str());
        c_weight_gm->SaveAs(Form("splotMC/splot_weight_cent_gm_%s_%s.png",subStr.c_str(),centLabel.c_str()));

        TCanvas *c_comp = new TCanvas(("c_comp_cent_" + centLabel).c_str(), ("Yield Comparison cent " + centLabel).c_str(), 800, 600);
        // h_corrected_yields[icent]->GetYaxis()->SetRangeUser(h_corrected_yields[icent]->GetMinimum()*0.2,h_corrected_yields[icent]->GetBinContent(1) * 1.2); // Adjust max for visibility

        TLegend *leg = new TLegend(0.11, 0.11, 0.31, 0.31);
        leg->SetBorderSize(0);
        leg->AddEntry(h_corrected_yields[icent], "Corrected Yield", "pe");
        leg->AddEntry(h_raw_yields[icent], "Raw Yield(Scaled)", "pe");
        leg->AddEntry(h_gen_yield_pass_ncoll[icent], "GEN Yield", "pe");
        leg->AddEntry(h_corrected_yields_cb[icent], "Corrected Yield CutBased", "pe");
        leg->AddEntry(h_corrected_yields_gm[icent], "Corrected Yield GM", "pe");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));
        // legend->Draw();
    
        // // 캔버스 저장 (선택 사항)
        // canvas->SaveAs("histogram_comparison.png");
        // Create pads
        TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
        pad1->SetBottomMargin(0.00); // Upper pad bottom margin
        pad1->Draw();
        pad1->cd(); // Go into the upper pad
        h_raw_yields[icent]->Scale(h_corrected_yields[icent]->GetBinContent(1)/h_raw_yields[icent]->GetBinContent(1));
        h_raw_yields[icent]->SetLineColor(kRed);
        h_raw_yields[icent]->SetMarkerColor(kRed);
        h_raw_yields[icent]->SetMarkerStyle(24);
        h_raw_yields[icent]->SetMarkerSize(1.0);
        h_raw_yields_gm[icent]->SetLineColor(kAzure-9);
        h_raw_yields_gm[icent]->SetMarkerColor(kAzure-9);
        h_raw_yields_gm[icent]->SetMarkerStyle(24);
        h_raw_yields_gm[icent]->SetMarkerSize(1.0);
        h_raw_yields_gm[icent]->Scale(h_corrected_yields[icent]->GetBinContent(1)/h_raw_yields_gm[icent]->GetBinContent(1));
        h_corrected_yields[icent]->SetLineColor(kBlack);
        h_corrected_yields[icent]->SetMarkerColor(kBlack);
        h_corrected_yields[icent]->SetMarkerStyle(20);
        h_corrected_yields[icent]->SetMarkerSize(1.0);

        h_corrected_yields_cb[icent]->SetLineColor(kGreen);
        h_corrected_yields_cb[icent]->SetMarkerColor(kGreen);
        h_corrected_yields_cb[icent]->SetMarkerStyle(20);
        h_corrected_yields_cb[icent]->SetMarkerSize(1.0);

        h_corrected_yields_gm[icent]->SetLineColor(kMagenta);
        h_corrected_yields_gm[icent]->SetMarkerColor(kMagenta);
        h_corrected_yields_gm[icent]->SetMarkerStyle(20);
        h_corrected_yields_gm[icent]->SetMarkerSize(1.0);
        

        
        double maxyield = std::max({h_corrected_yields[icent]->GetMaximum(), h_gen_yield_pass_ncoll[icent]->GetMaximum(), h_corrected_yields_cb[icent]->GetMaximum(), h_corrected_yields_gm[icent]->GetMaximum()});
        double minyield = std::min({h_corrected_yields[icent]->GetMinimum(), h_gen_yield_pass_ncoll[icent]->GetMinimum(), h_corrected_yields_cb[icent]->GetMinimum(), h_corrected_yields_gm[icent]->GetMinimum()});
        h_corrected_yields_gm[icent]->GetYaxis()->SetRangeUser(0.8*minyield, maxyield * 1.2); // Adjust max for visibility
        
        h_corrected_yields_gm[icent]->Draw("PE1");
        h_corrected_yields[icent]->Draw("PE1 SAME");
        h_raw_yields[icent]->Draw("PE1 SAME");
        h_raw_yields_gm[icent]->Draw("PE1 SAME");
        h_gen_yield_pass_ncoll[icent]->SetLineColor(kBlue);
        h_gen_yield_pass_ncoll[icent]->SetMarkerColor(kBlue);
        h_gen_yield_pass_ncoll[icent]->SetMarkerStyle(20);
        h_gen_yield_pass_ncoll[icent]->SetMarkerSize(1.0);
        h_gen_yield_pass_ncoll[icent]->Draw("PE1 SAME");

        h_corrected_yields_cb[icent]->Draw("PE1 SAME");
        latex.DrawLatex(0.25, 0.85, Form("cent = %.1f to %.1f", centBins[icent].first, centBins[icent].second));

        leg->Draw();
    
        // Draw main histograms in the upper pad
        // h2f->GetXaxis()->SetLabelSize(0); // Hide x-axis labels on the main plot
        // h2f->Draw("HIST");
        // hist->Draw("HIST SAME");
        // legend->Draw(); // Draw legend in the upper pad
    
        // Go back to the main canvas before creating the second pad
        c_comp->cd();
    
        // Create the lower pad for the ratio plot
        TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
        pad2->SetTopMargin(0.00); // Lower pad top margin
        pad2->SetBottomMargin(0.2); // Lower pad bottom margin
        pad2->SetGridy(); // Add a grid for the ratio
        pad2->Draw();
        pad2->cd(); // Go into the lower pad
    
        // Create the ratio histogram
        TH1D *hRatio = (TH1D*)h_corrected_yields[icent]->Clone("hRatio");
        hRatio->SetTitle(""); // Remove the title
        hRatio->SetStats(0);
        // Calculate the ratio
        hRatio->Divide(h_gen_yield_pass_ncoll[icent]); // Divide corrected yield by GEN yield
        for(size_t icent = 0; icent < ptBins.size(); ++icent) {
            std::vector<TH1D*> hists_to_label = {hRatio};
            for (TH1D* h : hists_to_label) {
                if (!h) continue;
                TAxis* xAxis = h->GetXaxis();
                for (int bin_idx = 1; bin_idx <= xAxis->GetNbins(); ++bin_idx) {
                    // mvaBinEdges is 0-indexed. For bin_idx (1-based), edges are mvaBinEdges[bin_idx-1] and mvaBinEdges[bin_idx]
                    if (bin_idx -1 < mvaBinEdges.size() && bin_idx < mvaBinEdges.size()) {
                         std::string binLabel = Form("%.3f", mvaBinEdges[bin_idx-1]);
                         xAxis->SetBinLabel(bin_idx, binLabel.c_str());
                        xAxis->ChangeLabel(bin_idx, 305., -1, -1, -1, -1, binLabel);
                        //  xAxis->SetLabelAngle(45); 
                        //  xAxis->SetLabelRotation(45);
                        //  hist->GetXaxis()->SetLabelAngle(45); 
    
    
                    }
                }
                xAxis->SetLabelOffset(0.01); // Adjust label offset
            }
        }

        // Find the bin with the maximum ratio
        int maxBin = hRatio->GetMaximumBin();
        double maxRatio = hRatio->GetBinContent(maxBin);
        double maxRatioError = hRatio->GetBinError(maxBin);
        int minBin = hRatio->GetMinimumBin();
        double minRatio = hRatio->GetBinContent(minBin);
        double minRatioError = hRatio->GetBinError(minBin);

        // Set ratio plot appearance
        hRatio->SetLineColor(kBlack);
        hRatio->SetMarkerStyle(20);
        hRatio->SetMarkerSize(0.8);

        // Adjust ratio plot appearance
        hRatio->GetYaxis()->SetTitle("Corrected / GEN"); // Updated Y-axis title
        hRatio->GetYaxis()->SetNdivisions(505); // Nicer divisions
        hRatio->GetYaxis()->SetTitleSize(20);
        hRatio->GetYaxis()->SetTitleFont(43);
        hRatio->GetYaxis()->SetTitleOffset(1.55);
        hRatio->GetYaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
        hRatio->GetYaxis()->SetLabelSize(15);

        hRatio->GetXaxis()->SetTitle("MVA Score");
        hRatio->GetXaxis()->SetTitleSize(20);
        hRatio->GetXaxis()->SetTitleFont(43);
        hRatio->GetXaxis()->SetTitleOffset(3.0); // Increased offset
        hRatio->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
        hRatio->GetXaxis()->SetLabelSize(15);

        // Set Y-axis range for ratio plot (adjust as needed)
        // Find min/max ratio values excluding zero/empty bins for better range setting
        double minRatioVal = 1e10;
        double maxRatioVal = -1e10;
        for (int i = 1; i <= hRatio->GetNbinsX(); ++i) {
            double content = hRatio->GetBinContent(i);
            if (content != 0) { // Consider only non-zero bins for range
            if (content < minRatioVal) minRatioVal = 1-content;
            if (content > maxRatioVal) maxRatioVal = 1-content;
            }
        }
        // Add some padding to the range
        double rangePadding = (maxRatioVal - minRatioVal) * 0.1;
        hRatio->GetYaxis()->SetRangeUser(0.8, 1.2); // Dynamic range

        hRatio->Draw("ep"); // Draw ratio plot with error bars

        // Draw a line at y=1 for reference
        // Ensure mvabins1 is defined and accessible here. Assuming it's mvaBinEdges
        const std::vector<double>& mvabins1 = mvaBinEdges; // Use the correct variable name
        TLine *line = new TLine(mvabins1.front(), 1.0, mvabins1.back(), 1.0);
        line->SetLineStyle(2); // Dashed line
        line->SetLineColor(kGray+2);
        line->Draw();

        // Add TLatex for max ratio value and error
        TLatex *latexRatio = new TLatex();
        latexRatio->SetNDC(); // Use normalized coordinates
        latexRatio->SetTextFont(42);
        latexRatio->SetTextSize(0.06); // Adjust size as needed for the pad
        latexRatio->SetTextAlign(11); // Align bottom-left
        latexRatio->DrawLatex(0.15, 0.85, Form("Max Deviation: %.3f %% #pm %.3f", minRatio, minRatioError)); // Position top-left

        // hRatio->SetLineColor(kBlack);
        // hRatio->SetMarkerStyle(20);
        // hRatio->SetMarkerSize(0.8);
    
        // // Adjust ratio plot appearance
        // hRatio->GetYaxis()->SetTitle("MC / Data");
        // hRatio->GetYaxis()->SetNdivisions(505); // Nicer divisions
        // hRatio->GetYaxis()->SetTitleSize(20);
        // hRatio->GetYaxis()->SetTitleFont(43);
        // hRatio->GetYaxis()->SetTitleOffset(1.55);
        // hRatio->GetYaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
        // hRatio->GetYaxis()->SetLabelSize(15);
    
        // hRatio->GetXaxis()->SetTitle("MVA Score");
        // hRatio->GetXaxis()->SetTitleSize(20);
        // hRatio->GetXaxis()->SetTitleFont(43);
        // hRatio->GetXaxis()->SetTitleOffset(3.0); // Increased offset
        // hRatio->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
        // hRatio->GetXaxis()->SetLabelSize(15);
    
        // // Set Y-axis range for ratio plot (optional, adjust as needed)
        // hRatio->GetYaxis()->SetRangeUser(0.5, 1.5); // Example range
    
        // hRatio->Draw("ep"); // Draw ratio plot with error bars
    
        // // Draw a line at y=1 for reference
        // TLine *line = new TLine(mvabins1.front(), 1.0, mvabins1.back(), 1.0);
        // line->SetLineStyle(2); // Dashed line
        // line->SetLineColor(kGray+2);
        // line->Draw();
    
        // Update the canvas to show everything
        
        c_comp->Update();
        c_comp->SetGrid(1,1);
        // c_comp->SaveAs(("splot_yield_comparison_cent_MC_ncoll" + centLabel + ".png").c_str());
        c_comp->SaveAs(Form("splotMC/splot_yield_comparison_cent_%s_%s.png",subStr.c_str(),centLabel.c_str()));
        // TFile *outputFile = new TFile(("splot_yield_comparison_cent_MC_ncoll" + centLabel + ".root").c_str(), "RECREATE");
        TFile *outputFile = new TFile(Form("splotMC/splot_yield_comparison_cent_%s_%s.root",subStr.c_str(),centLabel.c_str()), "RECREATE");
        h_corrected_yields[icent]->Write();
        h_raw_yields[icent]->Write();
        h_gen_yield_pass_ncoll[icent]->Write();
        h_corrected_yields_cb[icent]->Write();
        h_corrected_yields_gm[icent]->Write();
        h_weights[icent]->Write();
        delete leg;
        delete c_comp;
    }

    // for(size_t icent = 0; icent < centBins.size(); ++icent) {
    //     delete h_corrected_yields[icent];
    //     delete h_weights[icent];
    //     delete h_raw_yields[icent];
    // }

    // delete c_gen;
    // cout << "Counts: " << counts << endl;

    yieldFile->Close();
    effFile->Close();
    // delete yieldFile;
    // delete effFile;

    std::cout << "Finished drawing and saving histograms." << std::endl;
}
