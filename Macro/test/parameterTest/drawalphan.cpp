#include "../../fit/Opt.h"
#include "../../fit/DStarFitConfig.h"

void drawalphan(){
    gStyle->SetOptStat(0);
    FitOpt D0opt;
    std::string particleType = "D0";
    // Function to draw alpha parameter
    std::cout << "Drawing alpha parameter..." << std::endl;
    // Add drawing logic here
    std::vector<std::pair<double,double>> ptBins = {
        {5,7},
        {7,10},
        {10,20},
        {20,50}

    };
    std::vector<std::pair<double,double>> cosBins = {
        {0.0,0.2},
        {0.2,0.4},
        {0.4,0.6},
        {0.6,0.8},
        {0.8,1.0},
        

    };
    cout << "ptBins size: " << ptBins.size() << endl;
    cout << "cosBins size: " << cosBins.size() << endl;
    TH1D *halphaR[ptBins.size()];
    TH1D *halphaRData[ptBins.size()];

    TH1D *hnR[ptBins.size()];
    TH1D *hnRData[ptBins.size()];

    TH1D *halphaL[ptBins.size()];
    TH1D *halphaLData[ptBins.size()];

    TH1D *hnL[ptBins.size()];
    TH1D *hnLData[ptBins.size()];

    for(int i = 0; i < ptBins.size(); i++){
        halphaR[i] = new TH1D(Form("halphaR_%d",i),Form("AlphaR Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        halphaRData[i] = new TH1D(Form("halphaRData_%d",i),Form("AlphaR Data Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        hnR[i] = new TH1D(Form("hnR_%d",i),Form("nR Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        hnRData[i] = new TH1D(Form("hnRData_%d",i),Form("nR Data Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        halphaL[i] = new TH1D(Form("halphaL_%d",i),Form("AlphaL Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        halphaLData[i] = new TH1D(Form("halphaLData_%d",i),Form("AlphaL Data Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        hnL[i] = new TH1D(Form("hnL_%d",i),Form("nL Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        hnLData[i] = new TH1D(Form("hnLData_%d",i),Form("nL Data Parameter Distribution for %f <pT < %f ",ptBins[i].first,ptBins[i].second),cosBins.size(),0,cosBins.size());
        
        // Set bin labels for cosine ranges
        for(int bin = 0; bin < cosBins.size(); bin++){
            std::string label = Form("%.1f-%.1f", cosBins[bin].first, cosBins[bin].second);
            halphaR[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
            halphaRData[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
            hnR[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
            hnRData[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
            halphaL[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
            halphaLData[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
            hnL[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
            hnLData[i]->GetXaxis()->SetBinLabel(bin+1, label.c_str());
        }
    }
    int a =0;


    std::string subDir = "/DStar_ppRef_absCos_Jul15_test_v1/";
    int j =0;
        for(auto ptbin : ptBins){
            int i =0;
        for(auto cosbin : cosBins){
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            D0opt.cosMin = cosbin.first;
            D0opt.cosMax = cosbin.second;
            
            ConfigureDStarMCAbsFitOpt(D0opt);
            D0opt.setBaseDirectories("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/roots/Data_DStar_ppRef/",
                                     "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/roots/MC_DStar_ppRef/",
                                     "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/plots/");
            D0opt.setSubDir(subDir);
            TFile* fileMC = new TFile((D0opt.outputMCDir + D0opt.outputMCFile).c_str(), "READ");
            TFile* fileData = new TFile((D0opt.outputDir + D0opt.outputFile).c_str(), "READ");
            cout << "fileMC: " << (D0opt.outputMCDir + D0opt.outputMCFile) << endl;
            cout << "fileData: " << (D0opt.outputDir + D0opt.outputFile) << endl;
            RooFitResult* fitResult = (RooFitResult*)fileMC->Get("fitResult");
            if (fitResult) {
                fitResult->Print("v");
            }
            RooFitResult* fitResultData = (RooFitResult*)fileData->Get("fitResult");
            if (fitResultData) {
                fitResultData->Print("v");
            }
            // for( auto& param : fitResult->floatParsFinal()) {
            //     std::cout << "MC Parameter: " << param->GetName() << std::endl;
            //     cout << fitResult->floatParsFinal().find("alphaR_sig_DStar") << endl;
            // }
            RooRealVar* alphaR = (RooRealVar*)fitResult->floatParsFinal().find("alphaR_sig_DStar");
            RooRealVar* alphaRData = (RooRealVar*)fitResultData->floatParsFinal().find("alphaR_sig_DStar");
            RooRealVar* alphaL = (RooRealVar*)fitResult->floatParsFinal().find("alphaL_sig_DStar");
            RooRealVar* alphaLData = (RooRealVar*)fitResultData->floatParsFinal().find("alphaL_sig_DStar");
                halphaR[j]->SetBinContent(i+1, alphaR->getVal());
                halphaR[j]->SetBinError(i+1, alphaR->getError());
                halphaRData[j]->SetBinContent(i+1, alphaRData->getVal());
                halphaRData[j]->SetBinError(i+1, alphaRData->getError());
                
                halphaL[j]->SetBinContent(i+1, alphaL->getVal());       
                halphaL[j]->SetBinError(i+1, alphaL->getError());
                halphaLData[j]->SetBinContent(i+1, alphaLData->getVal());
                halphaLData[j]->SetBinError(i+1, alphaLData->getError());
                // hnL[j]->SetBinContent(i+1, alphaLData->getVal());
                // hnL[j]->SetBinError(i+1, alphaLData->getError());
                // hnLData[j]->SetBinContent(i+1, alphaLData->getVal());
                // hnLData[j]->SetBinError(i+1, alphaLData->getError());
            RooRealVar* nR = (RooRealVar*)fitResult->floatParsFinal().find("nR_sig_DStar");
            RooRealVar* nRData = (RooRealVar*)fitResultData->floatParsFinal().find("nR_sig_DStar");
            RooRealVar* nL = (RooRealVar*)fitResult->floatParsFinal().find("nL_sig_DStar");
            RooRealVar* nLData = (RooRealVar*)fitResultData->floatParsFinal().find("nL_sig_DStar");
                hnR[j]->SetBinContent(i+1, nR->getVal());
                hnR[j]->SetBinError(i+1, nR->getError());
                hnRData[j]->SetBinContent(i+1, nRData->getVal());
                hnRData[j]->SetBinError(i+1, nRData->getError());  
                hnL[j]->SetBinContent(i+1, nL->getVal());
                hnL[j]->SetBinError(i+1, nL->getError());
                hnLData[j]->SetBinContent(i+1, nLData->getVal());
                hnLData[j]->SetBinError(i+1, nLData->getError());
            i++;   
        

        }
        j++;
    }
    cout << "AlphaR and AlphaL parameters drawn successfully." << endl;
    TFile* outputFile = new TFile("alphaParameters.root", "RECREATE");
    for(int i = 0; i < ptBins.size(); i++){
        halphaR[i]->Write();
        halphaRData[i]->Write();
        hnR[i]->Write();
        hnRData[i]->Write();
        halphaL[i]->Write();
        halphaLData[i]->Write();
        hnL[i]->Write();
        hnLData[i]->Write();
    }
    for(int i = 0; i < ptBins.size(); i++){
    TCanvas* c1 = new TCanvas(Form("c1%d",i), "Alpha Parameters", 800, 600);
    c1->Divide(2, 2);
    c1->cd(1);
    TLegend* leg = new TLegend(0.11,0.89,0.2,0.8);
    leg->SetBorderSize(0);
    leg->AddEntry(halphaR[i], "MC AlphaR", "l");
    leg->AddEntry(halphaRData[i], "Data AlphaR", "l");
    halphaR[i]->SetTitle(Form("AlphaR for %f < pT < %f", ptBins[i].first, ptBins[i].second));
    halphaR[i]->GetXaxis()->SetTitle("|cos#theta_{HX}|");
    halphaR[i]->GetYaxis()->SetTitle("AlphaR Value");
    halphaR[i]->GetYaxis()->SetRangeUser(0,std::max(halphaR[i]->GetMaximum(), halphaRData[i]->GetMaximum()) * 1.2);
    halphaR[i]->Draw();
    halphaRData[i]->SetLineColor(kRed);
    halphaRData[i]->Draw("same");   
    leg->Draw();
    c1->cd(2);
    TLegend* leg2 = new TLegend(0.11,0.89,0.2,0.8);
    leg2->SetBorderSize(0);
    leg2->AddEntry(hnR[i], "MC nR", "l");
    leg2->AddEntry(hnRData[i], "Data nR", "l");
    hnR[i]->SetTitle(Form("nR for %f < pT < %f", ptBins[i].first, ptBins[i].second));
    hnR[i]->GetXaxis()->SetTitle("|cos#theta_{HX}|");
    hnR[i]->GetYaxis()->SetTitle("nR Value");
    hnR[i]->GetYaxis()->SetRangeUser(0,std::max(hnR[i]->GetMaximum(), hnRData[i]->GetMaximum()) * 1.2);
    hnR[i]->Draw();
    hnRData[i]->SetLineColor(kRed);
    hnRData[i]->Draw("same");
    leg2->Draw();
    c1->cd(3);
    TLegend* leg3 = new TLegend(0.11,0.89,0.2,0.8);
    leg3->SetBorderSize(0);
    leg3->AddEntry(halphaL[i], "MC AlphaL", "l");
    leg3->AddEntry(halphaLData[i], "Data AlphaL", "l");
    halphaL[i]->SetTitle(Form("AlphaL for %f < pT < %f", ptBins[i].first, ptBins[i].second));
    halphaL[i]->GetXaxis()->SetTitle("|cos#theta_{HX}|");
    halphaL[i]->GetYaxis()->SetTitle("AlphaL Value");
    halphaL[i]->GetYaxis()->SetRangeUser(0,std::max(halphaL[i]->GetMaximum(), halphaLData[i]->GetMaximum()) * 1.2);
    halphaL[i]->Draw();
    halphaLData[i]->SetLineColor(kRed);
    halphaLData[i]->Draw("same");
    leg3->Draw();
    c1->cd(4);
    TLegend* leg4 = new TLegend(0.11,0.89,0.2,0.8);
    leg4->SetBorderSize(0);
    leg4->AddEntry(hnL[i], "MC nL", "l");
    leg4->AddEntry(hnLData[i], "Data nL", "l");
    hnL[i]->SetTitle(Form("nL for %f < pT < %f", ptBins[i].first, ptBins[i].second));
    hnL[i]->GetXaxis()->SetTitle("|cos#theta_{HX}|");
    hnL[i]->GetYaxis()->SetTitle("nL Value");
    hnL[i]->GetYaxis()->SetRangeUser(0,std::max(hnL[i]->GetMaximum(), hnLData[i]->GetMaximum()) * 1.2);
    hnL[i]->Draw();
    hnLData[i]->SetLineColor(kRed);
    hnLData[i]->Draw("same");
    leg4->Draw();
    c1->SaveAs(Form("alphaParameters_%f_%f.pdf", ptBins[i].first, ptBins[i].second));
    c1->Write();
    }
    
        
        

    
    

}
