#include "../../fit/Opt.h"
void RAADraw(){
    FitOpt ppOpt;
    FitOpt ooOpt;
    std::string subDirppRef = "/DStar_ppRef_RAA_Jul24_v1/";
    std::string subDirOO = "/DStar_OO_RAA_Jul24_v1/";
    std::string baseDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/";
    
    vector<std::pair<double,double>> ptBins = {
        // {4,5},
        {5,6},
        {6,7},
        {7,8},
        {8,9},
        {9,11},
        {11,13},
        {13,15},
        {15,17},
        {17,20},
        {20,23},
        {23,26},
        {26,30},
        {30,40},
        {40,50}
        
    };
    auto a = new TGraphAsymmErrors();
    for(auto ptbin : ptBins){
        ppOpt.pTMin = ptbin.first;
        ppOpt.pTMax = ptbin.second;
        ooOpt.pTMin = ptbin.first;
        ooOpt.pTMax = ptbin.second;
        ppOpt.DStarOODefault();
        ooOpt.DStarOODefault();
        ppOpt.setSubDir(subDirppRef);
        ooOpt.setSubDir(subDirOO);
        TFile *ppFile = TFile::Open((baseDir+ppOpt.outputDir+"/"+ppOpt.outputFile).c_str());
        TFile *ooFile = TFile::Open((baseDir+ooOpt.outputDir+"/"+ooOpt.outputFile).c_str());
        RooWorkspace* wspp = (RooWorkspace*)ppFile->Get("ws_DStar");
        RooWorkspace* wsoo = (RooWorkspace*)ooFile->Get("ws_DStar");
        RooRealVar *nSigpp = (RooRealVar*)wspp->var("nsig");
        RooRealVar *nSigoo = (RooRealVar*)wsoo->var("nsig");
        
        // Get values and errors
        double nSigpp_val = nSigpp->getVal();
        double nSigpp_err = nSigpp->getError();
        double nSigoo_val = nSigoo->getVal();
        double nSigoo_err = nSigoo->getError();
        
        // Calculate ratio
        // double ratio = (nSigoo_val/(pow(16,4/3)*7)) / (nSigpp_val/1070);
        double ratio = (nSigoo_val/(pow(16,4./3.)*7)) / (nSigpp_val/1070);
        
        // Calculate error using proper error propagation for ratio R = A/B
        // σ_R = R * sqrt((σ_A/A)^2 + (σ_B/B)^2)
        double relative_error_oo = nSigoo_err / nSigoo_val;
        double relative_error_pp = nSigpp_err / nSigpp_val;
        double ratio_error = ratio * sqrt(relative_error_oo * relative_error_oo + 
                                         relative_error_pp * relative_error_pp);
        
        // Set point with proper error propagation
        a->SetPoint(a->GetN(), (ptbin.first + ptbin.second) / 2, ratio);
        a->SetPointError(a->GetN()-1, 0, 0, ratio_error, ratio_error);
        ppFile->Close();
        ooFile->Close();    
        
    }
    TCanvas *c1 = new TCanvas("c1", "RAA Draw", 800, 600);
    a->SetTitle("D* ratio; p_{T} (GeV/c); RAA");
    a->SetMarkerStyle(20);
    a->SetMarkerSize(1.0);
    a->SetLineColor(kBlue);
    a->SetLineWidth(2);
    a->GetYaxis()->SetRangeUser(0, 2);
    a->GetXaxis()->SetRangeUser(0, 50);
    a->GetYaxis()->SetTitleOffset(1);
    a->GetXaxis()->SetTitleOffset(1);
    a->GetYaxis()->SetTitle("ratio");
    a->GetXaxis()->SetTitle("p_{T} (GeV/c)");
    a->GetYaxis()->SetTitleSize(0.05);
    a->GetXaxis()->SetTitleSize(0.05);
    a->GetYaxis()->SetLabelSize(0.00);
    a->GetXaxis()->SetLabelSize(0.04);
    a->GetYaxis()->SetTitleFont(42);        
    a->GetYaxis()->SetLabelFont(42);
    a->GetYaxis()->SetNdivisions(505);
    a->GetXaxis()->SetNdivisions(505);
    a->Draw("AEP");
    c1->SetGrid();
    c1->SaveAs("DStar_RAA_DBCB_Jul24_v1.png");
    c1->SaveAs("DStar_RAA_DBCB_Jul24_v1.pdf");
    delete c1;
}
