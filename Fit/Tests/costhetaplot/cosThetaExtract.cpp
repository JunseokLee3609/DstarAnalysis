#include "../../fit/Opt.h"
#include "../../fit/DStarFitConfig.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLatex.h"

void cosThetaExtract(bool isMC){

    gStyle->SetOptStat(0);
    FitOpt ppOpt;
    // std::string subDirppRef = "/DStar_ppRef_absCos_absEta_1p2_Jul21_test_v1/";
    // std::string subDirppRef = "/DStar_ppRef_Data_absCos_absEta_1p2_Aug01_v1/";
    std::string subDirppRef;
    std::string subDirppRefMC = "DStar_ppRef_MC_absCos_absEta_1p2_Jul29_test_v1";
    std::string subDirppRefData = "/DStar_ppRef_Data_absCos_absEta_1p6_Aug01_pTD1_0p5_v2/";
    subDirppRef = isMC ? subDirppRefMC : subDirppRefData;


    std::string baseDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/";

    
    vector<std::pair<double,double>> ptBins = {
        {5,7},
        {7,10},
        {10,20},
        {20,50}

    };
    
    vector<std::pair<double,double>> cosBins = {
        {0,0.2},
        {0.2,0.4},
        {0.4,0.6},
        {0.6,0.8},
        {0.8,1}
    };
    
    vector<double> cosbin = {0,0.2,0.4,0.6,0.8,1};
        vector<double> ptCenter, ptError;
    vector<double> rho00_values, rho00_errors;
    
    TH1D *h[ptBins.size()];
    TH1D *hgen[ptBins.size()];
    TF1 *fitFunc[ptBins.size()];
    
    for (int i = 0; i < ptBins.size(); i++) {
        h[i] = new TH1D(Form("h_pt_%d", i), 
                       Form("cos#theta distribution, p_{T} = %.0f-%.0f GeV/c", ptBins[i].first, ptBins[i].second), 
                       cosbin.size()-1, cosbin.data());
        h[i]->Sumw2(true);
    }
    for (int i = 0; i < ptBins.size(); i++) {
        hgen[i] = new TH1D(Form("hgen_pt_%d", i), 
                       Form("cos#theta distribution, p_{T} = %.0f-%.0f GeV/c", ptBins[i].first, ptBins[i].second), 
                       cosbin.size()-1, cosbin.data());
        hgen[i]->Sumw2(true);
    }
    for(int i = 0; i < ptBins.size(); i++){
        
        
    fitFunc[i] = new TF1("fitFunc", "[0]*(1 - [1] + (3*[1] - 1)*x*x)", -1, 1);
    fitFunc[i]->SetParName(0, "N_{0}");
    fitFunc[i]->SetParName(1, "#rho_{00}");

    fitFunc[i]->SetParameter(0, 10);
    fitFunc[i]->SetParameter(1, 1/3);
    fitFunc[i]->SetParLimits(0, 0,h[i]->GetMaximum());
    fitFunc[i]->SetParLimits(1, 0.0, 1.0);

    }
    for(int ptIdx = 0; ptIdx < ptBins.size(); ptIdx++){
        auto ptbin = ptBins[ptIdx];
        
        for(int cosIdx = 0; cosIdx < cosBins.size(); cosIdx++){
            auto cosbin = cosBins[cosIdx];
            
            ppOpt.pTMin = ptbin.first;
            ppOpt.pTMax = ptbin.second;
            ppOpt.cosMin = cosbin.first;
            ppOpt.cosMax = cosbin.second;
            ConfigureDStarMCAbsFitOpt(ppOpt);
            ppOpt.setSubDir(subDirppRef);
            
            TFile *ppFile = TFile::Open((baseDir+ppOpt.outputDir+"/"+ppOpt.outputFile).c_str());
            if(!ppFile || ppFile->IsZombie()){
                std::cerr << "Cannot open file: " << baseDir+ppOpt.outputDir+"/"+ppOpt.outputFile << std::endl;
                continue;
            }
            else{
                std::cout << "Opened file: " << baseDir+ppOpt.outputDir+"/"+ppOpt.outputFile << std::endl;
            }
            
            RooWorkspace* wspp = (RooWorkspace*)ppFile->Get("ws_DStar");
            if(!wspp){
                std::cerr << "Cannot find workspace ws_DStar" << std::endl;
                ppFile->Close();
                continue;
            }
            
            RooRealVar *nSigpp = (RooRealVar*)wspp->var("nsig");
            RooDataSet* data = (RooDataSet*)ppFile->Get("reducedData");
            RooDataSet* reducedData;
            if(isMC){
                reducedData = (RooDataSet*)data->reduce("matchGEN==1");
            }

            
            TFile *dcaFile = TFile::Open((baseDir+ppOpt.outputDir+"/"+ppOpt.outputDCAFile).c_str());
            if(!dcaFile || dcaFile->IsZombie()){
                std::cerr << "Cannot open DCA file: " << baseDir+ppOpt.outputDir+"/"+ppOpt.outputDCAFile << std::endl;
                ppFile->Close();
                continue;
            }
            
            RooWorkspace* wsdca = (RooWorkspace*)dcaFile->Get("ws_DStar");
            if(!wsdca){
                std::cerr << "Cannot find workspace ws_dcaFitter" << std::endl;
                ppFile->Close();
                dcaFile->Close();
                continue;
            }

            TParameter <double> *fracPromptParam = (TParameter<double>*)dcaFile->Get("PromptFraction");
            TParameter <double> *fracPromptErrParam = (TParameter<double>*)dcaFile->Get("PromptFractionError");
            cout << "Prompt Fraction: " << fracPromptParam->GetVal() << " ± " << fracPromptErrParam->GetVal() << endl;

            double yield = nSigpp->getVal() * fracPromptParam->GetVal();
                        double yieldError = sqrt(pow(nSigpp->getError() * fracPromptParam->GetVal(), 2) + 
                                   pow(nSigpp->getVal() * fracPromptErrParam->GetVal(), 2));
            std::cout << "nsig : " << nSigpp->getVal() << " Yield for pT bin " << ptIdx << ": " << "pT : " << ptbin.first << "-" << ptbin.second 
                      << " cosTheta: " << cosbin.first << "-" << cosbin.second 
                      << " Yield: " << yield << " ± " << yieldError << std::endl;

            double binCenter = (cosbin.first + cosbin.second) / 2.0;
            int binNum = cosIdx + 1;
            
            h[ptIdx]->SetBinContent(binNum, yield);
            h[ptIdx]->SetBinError(binNum, yieldError);
            if(isMC){
                hgen[ptIdx]->SetBinContent(binNum, reducedData->reduce(Form("matchGEN==1 && abs(cosThetaHX) >= %f && abs(cosThetaHX) < %f", cosbin.first, cosbin.second))->sumEntries()); 
                hgen[ptIdx]->SetBinError(binNum, sqrt(reducedData->reduce(Form("matchGEN==1 && abs(cosThetaHX) >= %f && abs(cosThetaHX) < %f", cosbin.first, cosbin.second))->sumEntries()));
            }
            
            
            
            ppFile->Close();
            dcaFile->Close();
        }
        
        h[ptIdx]->SetMarkerStyle(20);
        h[ptIdx]->SetMarkerSize(1.2);
        h[ptIdx]->SetMarkerColor(kBlue);
        h[ptIdx]->SetLineColor(kBlue);
        h[ptIdx]->GetXaxis()->SetTitle("|cos#theta_{HX}|");
        h[ptIdx]->GetYaxis()->SetTitle("Prompt D* yield");
        h[ptIdx]->GetYaxis()->SetTitleOffset(1.4);
        
        std::cout << "Fitting histogram for pT bin " << ptIdx << std::endl;
        TFitResultPtr fitResult = isMC ? hgen[ptIdx]->Fit(fitFunc[ptIdx], "SER", "", 0.1, 0.9) : h[ptIdx]->Fit(fitFunc[ptIdx], "SER", "", 0.1, 0.9);

        // if(fitResult->IsValid()){
            double rho00 = fitFunc[ptIdx]->GetParameter(1);
            double rho00_err = fitFunc[ptIdx]->GetParError(1);
            double pt_center = (ptbin.first + ptbin.second) / 2.0;
            double pt_width = (ptbin.second - ptbin.first) / 2.0;
            
            ptCenter.push_back(pt_center);
            ptError.push_back(pt_width);
            rho00_values.push_back(rho00);
            rho00_errors.push_back(rho00_err);
            
            std::cout << "  pT = " << pt_center << " GeV/c: ρ₀₀ = " << rho00 << " ± " << rho00_err << std::endl;
        // }
    }
    
    TCanvas *c1[ptBins.size()];
    for(int i = 0; i < ptBins.size(); i++){
        c1[i] = new TCanvas(Form("c1_pt_%d", i),
                                Form("CosTheta Fit for pT = %.0f-%.0f GeV   /c", ptBins[i].first, ptBins[i].second),
                                800, 600);
        c1[i]->SetLeftMargin(0.15);
        c1[i]->SetBottomMargin(0.15);
        c1[i]->SetTopMargin(0.10);
        c1[i]->SetRightMargin(0.05);    
        c1[i]->SetGrid(0,0);
    }
        
    
    for(int i = 0; i < ptBins.size(); i++){
        c1[i]->cd();
        h[i]->SetMarkerStyle(20);
        h[i]->SetMarkerSize(1.0);
        h[i]->SetMarkerColor(kBlack);
        h[i]->SetLineColor(kBlack);
        h[i]->SetLineWidth(1);
        
        h[i]->GetXaxis()->SetTitle("|cos#vartheta*|");
        h[i]->GetYaxis()->SetTitle("dN/dcos#vartheta*");
        
        h[i]->GetXaxis()->SetTitleSize(0.05);
        h[i]->GetXaxis()->SetTitleOffset(1.0);
        h[i]->GetXaxis()->SetLabelSize(0.04);
        h[i]->GetXaxis()->SetLabelFont(42);
        h[i]->GetXaxis()->SetTitleFont(42);
        h[i]->GetXaxis()->SetNdivisions(505);
        
        h[i]->GetYaxis()->SetTitleSize(0.05);
        h[i]->GetYaxis()->SetTitleOffset(1.2);
        h[i]->GetYaxis()->SetLabelSize(0.04);
        h[i]->GetYaxis()->SetLabelFont(42);
        h[i]->GetYaxis()->SetTitleFont(42);
        h[i]->GetYaxis()->SetNdivisions(505);
        
        h[i]->GetXaxis()->SetRangeUser(0, 1);
        h[i]->GetYaxis()->SetRangeUser(0, h[i]->GetMaximum() * 1.3);
        h[i]->Draw("EP");
        hgen[i]->SetMarkerStyle(20);
        hgen[i]->SetMarkerSize(1.2);
        hgen[i]->SetMarkerColor(kRed);
        hgen[i]->SetLineColor(kRed);
        hgen[i]->SetLineWidth(1);
        hgen[i]->SetLineStyle(2);
        hgen[i]->Draw("EP same");
        
        fitFunc[i]->SetLineColor(kBlue);
        fitFunc[i]->SetLineWidth(2);
        fitFunc[i]->SetLineStyle(1);
        
            fitFunc[i]->Draw("same");
            
    
    TLegend *leg = new TLegend(0.15, 0.15, 0.55, 0.48);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.04);
    leg->SetTextFont(42);
    
    leg->AddEntry(h[i], "Data", "EP");
    
        double a_param = fitFunc[i]->GetParameter(1);
        double norm_param = fitFunc[i]->GetParameter(0);
        
        TString fitLabel = Form("N_{0}[1#minus#rho_{00}^{obs}+(3#rho_{00}^{obs}#minus1)cos^{2}#vartheta*]");
        leg->AddEntry(fitFunc[i], fitLabel, "L");
        leg->AddEntry((TObject*)0, Form("#rho_{00}^{obs} = %.3f #pm %.3f", a_param, fitFunc[i]->GetParError(1)), "");

    leg->Draw();
    
    
        if(fitFunc[i]->GetParameter(1) != 0){
            double rho00 = fitFunc[i]->GetParameter(1);
            double rho00_err = fitFunc[i]->GetParError(1);
            std::cout << "rho_00^obs = " << rho00 << " ± " << rho00_err << std::endl;
        }

    c1[i]->SaveAs( isMC ? Form("cosTheta_fit_pt_%f_%f_MC.png", ptBins[i].first, ptBins[i].second) : Form("cosTheta_fit_pt_%f_%f.png", ptBins[i].first, ptBins[i].second));
    c1[i]->SaveAs( isMC ? Form("cosTheta_fit_pt_%f_%f_MC.pdf", ptBins[i].first, ptBins[i].second) : Form("cosTheta_fit_pt_%f_%f.pdf", ptBins[i].first, ptBins[i].second));
        //  c1[i]->SaveAs(Form("cosTheta_fit_pt_%f_%f.png", ptBins[i].first, ptBins[i].second));
        //
    // c1[i]->SaveAs(Form("cosTheta_fit_pt_%f_%f.pdf", ptBins[i].first, ptBins[i].second));

    // delete c1[i];
    delete leg;
    }
    TCanvas *c2 = new TCanvas("c2", "rho00 vs pT", 800, 600);
    c2->SetLeftMargin(0.12);
    c2->SetBottomMargin(0.12);
    c2->SetTopMargin(0.08);
    c2->SetRightMargin(0.05);
    
    // TGraphErrors 생성
    TGraphErrors *gr_rho00 = new TGraphErrors(ptCenter.size(), 
                                              &ptCenter[0], &rho00_values[0],
                                              &ptError[0], &rho00_errors[0]);
    
    // 그래프 스타일 설정 (ALICE 스타일 참고)
    gr_rho00->SetMarkerStyle(20);
    gr_rho00->SetMarkerSize(1.2);
    gr_rho00->SetMarkerColor(kBlue);
    gr_rho00->SetLineColor(kBlue);
    gr_rho00->SetLineWidth(2);
    
    // 축 설정
    gr_rho00->GetXaxis()->SetTitle("p_{T} (GeV/c)");
    gr_rho00->GetYaxis()->SetTitle("#rho_{00}");
    gr_rho00->GetXaxis()->SetRangeUser(0, 50);
    gr_rho00->GetYaxis()->SetRangeUser(0, 0.7);
    
    // 축 레이블 스타일
    gr_rho00->GetXaxis()->SetTitleSize(0.05);
    gr_rho00->GetXaxis()->SetTitleOffset(1.0);
    gr_rho00->GetXaxis()->SetLabelSize(0.04);
    gr_rho00->GetXaxis()->SetLabelFont(42);
    gr_rho00->GetXaxis()->SetTitleFont(42);
    
    gr_rho00->GetYaxis()->SetTitleSize(0.05);
    gr_rho00->GetYaxis()->SetTitleOffset(1.2);
    gr_rho00->GetYaxis()->SetLabelSize(0.04);
    gr_rho00->GetYaxis()->SetLabelFont(42);
    gr_rho00->GetYaxis()->SetTitleFont(42);
    
    gr_rho00->Draw("AP");
    
    // 1/3 수평선 추가 (unpolarized limit)
    double xmin = gr_rho00->GetXaxis()->GetXmin();
    double xmax = gr_rho00->GetXaxis()->GetXmax();

    // 1/3 수평선 추가 (unpolarized limit) - 플롯 범위에 맞춰서
    TLine *line_unpol = new TLine(xmin, 1./3., xmax, 1./3.);
    line_unpol->SetLineStyle(2);
    line_unpol->SetLineWidth(2);
    line_unpol->SetLineColor(kGray+2);
    line_unpol->Draw();
    
    // 범례 추가
    TLegend *leg2 = new TLegend(0.15, 0.75, 0.55, 0.92);
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    leg2->SetTextSize(0.035);
    leg2->SetTextFont(42);
    
    // 실험 정보 추가 (ALICE 스타일 참고)
    leg2->AddEntry((TObject*)0, "CMS", "");
    leg2->AddEntry((TObject*)0, "pp, #sqrt{s} = 5.32 TeV", "");
    leg2->AddEntry((TObject*)0, "Prompt D*^{+}", "");
    leg2->AddEntry((TObject*)0, "Helicity frame", "");
    leg2->Draw();
    
    // unpolarized limit 라벨
    TLatex *unpol_label = new TLatex(25, 0.36, "no spin alignment");
    unpol_label->SetTextSize(0.03);
    unpol_label->SetTextFont(42);
    unpol_label->SetTextColor(kGray+2);
    unpol_label->Draw();
    
    // c2->SaveAs("rho00_vs_pT_MC.png");
    c2->SaveAs(isMC ? "rho00_vs_pT_MC.png" : "rho00_vs_pT.png");
    c2->SaveAs(isMC ? "rho00_vs_pT_MC.pdf" : "rho00_vs_pT.pdf");
    // c2->SaveAs("rho00_vs_p
    // c2->SaveAs("rho00_vs_pT_MC.pdf");
    
    // 메모리 정리
    delete c2;
    delete leg2;
    delete line_unpol;
    delete unpol_label;
    delete gr_rho00;

}
