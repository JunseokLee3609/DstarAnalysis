#include "../Opt.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TLegend.h"
#include "TRatioPlot.h"
#include "TPad.h"
#include "TROOT.h"
#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "RooFitResult.h"
#include <vector>
#include <string>
#include <iostream>

vector<std::pair<double, double>> GetSignalYield(FitOpt opt, vector<float> mvabin, RooDataset *data) {


    // 파일 열기 및 nsig 값 읽기
    TFile *file = TFile::Open(opt.outputFile.c_str());
    if(file){
    for ( mva : mvabin){
            std::string cutExpr = Form("mva > %f && matchGEN==1 && isMC==1", mva);

            sig = data->sumEntries();
            sigError = sqrt(sig); // Poisson error
            std::cout << "MC pT: " << ptMin << "-" << ptMax << " nsig: " << sig
                      << " ± " << sigError << std::endl;
            RooFitResult *fitResult = (RooFitResult*)file->Get(opt.fitResultName.c_str());
            if (fitResult) {
                RooRealVar *nsig = (RooRealVar*)fitResult->floatParsFinal().find("nSig");
                if (nsig) {
                    sig = nsig->getVal();
                    sigError = nsig->getError();
                    std::cout << "Data pT: " << ptMin << "-" << ptMax << " nsig: " << sig 
                              << " ± " << sigError << std::endl;
            }
            }
    }} else {
        std::cerr << "Cannot open file: " << opt.outputFile << std::endl;
    }

    return std::make_pair(sig, sigError);
}

void DrawEfficiencyCurve() {
    gStyle->SetOptStat(0);
    
    // PT 빈 정의
    const int nBins = 10;
    double ptBins[nBins+1] = {2, 3, 4, 5, 6, 8, 10, 15, 20, 30, 50};
    
    // MC와 Data용 FitOpt 객체 생성
    FitOpt dataOpt;
    FitOpt mcOpt;
    
    dataOpt.D0DataDefault();
    mcOpt.D0MCDefault();
    
    // 출력 디렉토리 설정
    dataOpt.outputDir = "results/data/";
    mcOpt.outputDir = "results/mc/";
    
    // 결과를 저장할 효율성 히스토그램 생성
    TH1D *efficiencyHist = new TH1D("efficiencyHist", "D0 Efficiency Curve;p_{T} (GeV/c);Data/MC", nBins, ptBins);
    TH1D *dataSigHist = new TH1D("dataSigHist", "Data Signal Yield;p_{T} (GeV/c);Signal Yield", nBins, ptBins);
    TH1D *mcSigHist = new TH1D("mcSigHist", "MC Signal Yield;p_{T} (GeV/c);Signal Yield", nBins, ptBins);
    
    // 각 pT 빈에 대해 파일을 열고 nsig 값 추출
    for (int i = 0; i < nBins; i++) {
        double ptMin = ptBins[i];
        double ptMax = ptBins[i+1];
        
        // Data 파일 경로 및 컷 표현식 설정
        dataOpt.cutExpr = Form("%s > %.1f && %s < %.1f && abs(%s) < 1.6 && %s > %f", 
                              dataOpt.ptVar.c_str(), ptMin, 
                              dataOpt.ptVar.c_str(), ptMax,
                              dataOpt.etaVar.c_str(), 
                              dataOpt.mvaVar.c_str(), dataOpt.mvaMin);
        
        dataOpt.outputFile = Form("%s/D0_pT%.1f_%.1f_%s_%.2f.root", 
                                 dataOpt.outputDir.c_str(), ptMin, ptMax,
                                 dataOpt.mvaVar.c_str(), dataOpt.mvaMin);
        
        // MC 파일 경로 및 컷 표현식 설정
        mcOpt.cutExpr = Form("%s > %.1f && %s < %.1f && abs(%s) < 1.6 && matchGEN==1", 
                            mcOpt.ptVar.c_str(), ptMin, 
                            mcOpt.ptVar.c_str(), ptMax,
                            mcOpt.etaVar.c_str());
        
        mcOpt.outputFile = Form("%s/D0_MC_pT%.1f_%.1f.root", 
                               mcOpt.outputDir.c_str(), ptMin, ptMax);
        
        // 파일 열기 및 nsig 값 읽기
        double dataSig = 0, mcSig = 0;
        double dataSigError = 0, mcSigError = 0;
        
        // Data 파일 처리
        TFile *dataFile = TFile::Open(dataOpt.outputFile.c_str());
        if (dataFile && !dataFile->IsZombie()) {
            RooWorkspace *ws = (RooWorkspace*)dataFile->Get(dataOpt.wsName.c_str());
            if (ws) {
                RooRealVar *nsig = ws->var("nsig");
                if (nsig) {
                    dataSig = nsig->getVal();
                    dataSigError = nsig->getError();
                    std::cout << "Data pT: " << ptMin << "-" << ptMax << " nsig: " << dataSig 
                              << " ± " << dataSigError << std::endl;
                }
            } else {
                // 워크스페이스가 없는 경우, 다른 방법으로 nsig 값 찾기
                RooFitResult *fitResult = (RooFitResult*)dataFile->Get(dataOpt.fitResultName.c_str());
                if (fitResult) {
                    RooRealVar *nsig = (RooRealVar*)fitResult->floatParsFinal().find("nsig");
                    if (nsig) {
                        dataSig = nsig->getVal();
                        dataSigError = nsig->getError();
                        std::cout << "Data pT: " << ptMin << "-" << ptMax << " nsig: " << dataSig 
                                  << " ± " << dataSigError << std::endl;
                    }
                }
            }
            dataFile->Close();
            delete dataFile;
        } else {
            std::cerr << "Cannot open data file: " << dataOpt.outputFile << std::endl;
        }
        
        // MC 파일 처리
        TFile *mcFile = TFile::Open(mcOpt.outputFile.c_str());
        if (mcFile && !mcFile->IsZombie()) {
            RooWorkspace *ws = (RooWorkspace*)mcFile->Get(mcOpt.wsName.c_str());
            if (ws) {
                RooRealVar *nsig = ws->var("nsig");
                if (nsig) {
                    mcSig = nsig->getVal();
                    mcSigError = nsig->getError();
                    std::cout << "MC pT: " << ptMin << "-" << ptMax << " nsig: " << mcSig 
                              << " ± " << mcSigError << std::endl;
                }
            } else {
                // 워크스페이스가 없는 경우, 다른 방법으로 nsig 값 찾기
                RooFitResult *fitResult = (RooFitResult*)mcFile->Get(mcOpt.fitResultName.c_str());
                if (fitResult) {
                    RooRealVar *nsig = (RooRealVar*)fitResult->floatParsFinal().find("nsig");
                    if (nsig) {
                        mcSig = nsig->getVal();
                        mcSigError = nsig->getError();
                        std::cout << "MC pT: " << ptMin << "-" << ptMax << " nsig: " << mcSig 
                                  << " ± " << mcSigError << std::endl;
                    }
                }
            }
            mcFile->Close();
            delete mcFile;
        } else {
            std::cerr << "Cannot open MC file: " << mcOpt.outputFile << std::endl;
        }
        
        // 히스토그램에 값 채우기
        if (mcSig > 0) {
            // bin 인덱스는 1부터 시작 (ROOT의 관례)
            int binIdx = i + 1;
            
            // 개별 히스토그램에 값 설정
            dataSigHist->SetBinContent(binIdx, dataSig);
            dataSigHist->SetBinError(binIdx, dataSigError);
            
            mcSigHist->SetBinContent(binIdx, mcSig);
            mcSigHist->SetBinError(binIdx, mcSigError);
            
            // 효율성 계산 (Data/MC)
            double efficiency = dataSig / mcSig;
            // 오차 전파 공식: (A/B) 오차 = (A/B) * sqrt((error_A/A)^2 + (error_B/B)^2)
            double relErrorData = dataSigError / dataSig;
            double relErrorMC = mcSigError / mcSig;
            double effError = efficiency * sqrt(relErrorData*relErrorData + relErrorMC*relErrorMC);
            
            efficiencyHist->SetBinContent(binIdx, efficiency);
            efficiencyHist->SetBinError(binIdx, effError);
        }
    }
    
    // 시각화
    TCanvas *c1 = new TCanvas("c1", "Signal Yields and Efficiency", 900, 700);
    c1->Divide(1, 2);
    
    // 상단 패드 - Signal Yields
    c1->cd(1);
    gPad->SetLogy();
    gPad->SetGridx();
    gPad->SetGridy();
    
    dataSigHist->SetMarkerStyle(20);
    dataSigHist->SetMarkerColor(kBlue);
    dataSigHist->SetLineColor(kBlue);
    dataSigHist->Draw("E1P");
    
    mcSigHist->SetMarkerStyle(21);
    mcSigHist->SetMarkerColor(kRed);
    mcSigHist->SetLineColor(kRed);
    mcSigHist->Draw("E1P SAME");
    
    TLegend *leg1 = new TLegend(0.65, 0.7, 0.85, 0.85);
    leg1->AddEntry(dataSigHist, "Data", "lp");
    leg1->AddEntry(mcSigHist, "MC", "lp");
    leg1->SetBorderSize(0);
    leg1->Draw();
    
    // 하단 패드 - 효율성
    c1->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    
    efficiencyHist->SetMarkerStyle(22);
    efficiencyHist->SetMarkerColor(kBlack);
    efficiencyHist->SetLineColor(kBlack);
    efficiencyHist->SetMinimum(0);
    efficiencyHist->SetMaximum(2);
    efficiencyHist->GetXaxis()->SetTitleOffset(1.2);
    efficiencyHist->GetYaxis()->SetTitleOffset(1.3);
    efficiencyHist->Draw("E1P");
    
    // 효율성 = 1 기준선 추가
    TLine *line = new TLine(ptBins[0], 1.0, ptBins[nBins], 1.0);
    line->SetLineStyle(2);
    line->SetLineColor(kRed);
    line->Draw();
    
    c1->SaveAs("D0_efficiency_curve.pdf");
    c1->SaveAs("D0_efficiency_curve.png");
    
    std::cout << "Efficiency curve created successfully!" << std::endl;
}
