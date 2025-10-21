#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.hxx"
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"

// 효율성 맵에서 가중치 찾기
double findWeight(Float_t pt, Float_t y, Float_t phi, TH3D* effMap) {
    double weight = 1.0;
    if (effMap) {
        Int_t bin = effMap->FindBin(pt, y, phi);
        double eff = effMap->GetBinContent(bin);
        // if (eff > 0) {
            weight = 1.0/eff;
        // }
    }
    return weight;
}

void reproduce() {
    // 파일 열기
    string fileName = "/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Mar30NonSwap/d0ana_tree_nonswapsample_ppref_30Mar.root";
    TFile* f = TFile::Open(fileName.c_str());
    TFile* feffMap = TFile::Open("output.root");
    
    // 트리 가져오기
    string name_tree = "dStarana_mc/PATCompositeNtuple";
    auto t = (TTree*) f->Get(name_tree.c_str());
    DataFormat::simpleDStarMCTreeevt evtMC;
    evtMC.setTree(t);
    evtMC.setGENTree(t);
    int nEvts = t->GetEntries();
    
    // pT 분포를 위한 히스토그램 생성
    vector<double> pTBins = {0,4, 6, 8, 10, 14, 20, 30};
    vector<double> yBins = {-1, -0.4, 0, 0.4, 1};
    vector<double> phiBins = {-TMath::Pi(), -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, TMath::Pi()};
    TH1D* hGenPt = new TH1D("hGenPt", "Generated p_{T} Distribution;p_{T} [GeV/c];Events", pTBins.size()-1, &pTBins[0]);
    TH1D* hRecoPt = new TH1D("hRecoPt", "Reconstructed p_{T} Distribution (No Weight);p_{T} [GeV/c];Events", pTBins.size()-1, &pTBins[0]);
    TH1D* hRecoPtWeighted = new TH1D("hRecoPtWeighted", "Reconstructed p_{T} Distribution (Weighted);p_{T} [GeV/c];Events", pTBins.size()-1, &pTBins[0]);
    
    // y(rapidity) 분포를 위한 히스토그램 생성
    TH1D* hGenY = new TH1D("hGenY", "Generated Rapidity Distribution;y;Events", yBins.size()-1, &yBins[0]);
    TH1D* hRecoY = new TH1D("hRecoY", "Reconstructed Rapidity Distribution (No Weight);y;Events",  yBins.size()-1, &yBins[0]);
    TH1D* hRecoYWeighted = new TH1D("hRecoYWeighted", "Reconstructed Rapidity Distribution (Weighted);y;Events",  yBins.size()-1, &yBins[0]);
    
    // phi(방위각) 분포를 위한 히스토그램 생성
    TH1D* hGenPhi = new TH1D("hGenPhi", "Generated #phi Distribution;#phi [rad];Events", phiBins.size()-1, &phiBins[0]);
    TH1D* hRecoPhi = new TH1D("hRecoPhi", "Reconstructed #phi Distribution (No Weight);#phi [rad];Events",  phiBins.size()-1, &phiBins[0]);
    TH1D* hRecoPhiWeighted = new TH1D("hRecoPhiWeighted", "Reconstructed #phi Distribution (Weighted);#phi [rad];Events",  phiBins.size()-1, &phiBins[0]);
    
    // 효율성 맵 가져오기
    TH3D* effMap = (TH3D*)feffMap->Get("pt_y_phi_pr_ratio");
    
    cout << "Processing " << nEvts << " events..." << endl;
    
    // 이벤트 루프
    for (int evt = 0; evt < nEvts; evt++) {
        if (evt % 10000 == 0) cout << "Processing event " << evt << "/" << nEvts << endl;
        t->GetEntry(evt);
        
        // 생성된 이벤트 처리 (GEN)
        for (int i = 0; i < evtMC.candSize_gen; i++) {
            TLorentzVector Dstar, D0, D1, D0Dau1, D0Dau2;
            Dstar.SetPtEtaPhiM(evtMC.gen_pT[i], evtMC.gen_eta[i], evtMC.gen_phi[i], evtMC.gen_mass[i]);
            D0.SetPtEtaPhiM(evtMC.gen_D0pT[i], evtMC.gen_D0eta[i], evtMC.gen_D0phi[i], evtMC.gen_D0mass[i]);
            D1.SetPtEtaPhiM(evtMC.gen_D1pT[i], evtMC.gen_D1eta[i], evtMC.gen_D1phi[i], evtMC.gen_D1mass[i]);
            D0Dau1.SetPtEtaPhiM(evtMC.gen_D0Dau1_pT[i], evtMC.gen_D0Dau1_eta[i], evtMC.gen_D0Dau1_phi[i], evtMC.gen_D0Dau1_mass[i]);
            D0Dau2.SetPtEtaPhiM(evtMC.gen_D0Dau2_pT[i], evtMC.gen_D0Dau2_eta[i], evtMC.gen_D0Dau2_phi[i], evtMC.gen_D0Dau2_mass[i]);
            
            // 선택 기준 적용
            if (evtMC.gen_pT[i] > DSGLPTLO && 
                fabs(evtMC.gen_y[i]) < DSGLABSY && 
                // DstarDauSimpleAcc(D0, D1) &&
                // D0DauAcc(D0Dau1, D0Dau2)) {
                true){
                
                // GEN 히스토그램에 채우기
                hGenPt->Fill(evtMC.gen_pT[i]);
                hGenY->Fill(evtMC.gen_y[i]);
                hGenPhi->Fill(evtMC.gen_phi[i]);
            }
        }
        
        // 재구성된 이벤트 처리 (RECO)
        for (int i = 0; i < evtMC.candSize; i++) {
            auto D0y = algo::rapidity(evtMC.pTD1[i], evtMC.EtaD1[i], evtMC.PhiD1[i], evtMC.massDaugther1[i]);
            
            // 선택 기준 적용
            if (evtMC.matchGEN[i]){  
                if(fabs(evtMC.y[i]) < DSGLABSY &&
                evtMC.pT[i] > DSGLPTLO &&
                fabs(D0y) < DSGLABSY_D0 &&
                fabs(evtMC.EtaD2[i]) < 1.5 && 
                evtMC.pTD2[i] > 0.4 && 
                fabs(evtMC.EtaGrandD1[i]) < 2.4 &&
                fabs(evtMC.EtaGrandD2[i]) < 2.4 &&
                evtMC.pTGrandD1[i] > 1 &&
                evtMC.pTGrandD2[i] > 1 &&
                true) {
                
                // 가중치 계산
                double weight = findWeight(evtMC.pT[i], evtMC.y[i], evtMC.phi[i], effMap);
                
                // RECO 히스토그램에 채우기 (가중치 적용 전/후)
                hRecoPt->Fill(evtMC.pT[i]);
                hRecoPtWeighted->Fill(evtMC.pT[i], weight);
                
                hRecoY->Fill(evtMC.y[i]);
                hRecoYWeighted->Fill(evtMC.y[i], weight);
                
                hRecoPhi->Fill(evtMC.phi[i]);
                hRecoPhiWeighted->Fill(evtMC.phi[i], weight);
                   }   }
        }
    }
    
    // ROOT 파일 생성
    TFile* outFile = new TFile("reproduce_results.root", "RECREATE");
    
    // 각 변수(pT, y, phi)에 대한 플롯 및 분석 생성 함수
    auto createPlotsForVariable = [&outFile](TH1D* hGen, TH1D* hReco, TH1D* hRecoWeighted, const char* varName) {
        // 결과 시각화
        TCanvas* c = new TCanvas(Form("c_%s", varName), Form("%s Distribution Comparison", varName), 800, 600);
        
        // 히스토그램 스타일 설정
        hGen->SetLineColor(kRed);
        hGen->SetLineWidth(2);
        hGen->SetMarkerColor(kRed);
        hGen->SetMarkerStyle(20);
        hGen->SetMarkerSize(0.8);
        
        hReco->SetLineColor(kBlue);
        hReco->SetLineWidth(2);
        hReco->SetMarkerColor(kBlue);
        hReco->SetMarkerStyle(21);
        hReco->SetMarkerSize(0.8);
        
        hRecoWeighted->SetLineColor(kGreen+2);
        hRecoWeighted->SetLineWidth(2);
        hRecoWeighted->SetMarkerColor(kGreen+2);
        hRecoWeighted->SetMarkerStyle(22);
        hRecoWeighted->SetMarkerSize(0.8);
        
        // 히스토그램 그리기
        hGen->Draw("HIST");
        hReco->Draw("HIST SAME");
        hRecoWeighted->Draw("HIST SAME");
        
        // 범례 추가
        TLegend* legend = new TLegend(0.65, 0.65, 0.88, 0.85);
        legend->AddEntry(hGen, "Generated", "lp");
        legend->AddEntry(hReco, "Reconstructed", "lp");
        legend->AddEntry(hRecoWeighted, "Reconstructed (Weighted)", "lp");
        legend->SetBorderSize(0);
        legend->Draw();
        
        // 비율 플롯 (복원 성능 평가)
        TCanvas* cRatio = new TCanvas(Form("cRatio_%s", varName), Form("Ratio to Generated (%s)", varName), 800, 300);
        
        TH1D* hRatioNoWeight = (TH1D*)hReco->Clone(Form("hRatioNoWeight_%s", varName));
        hRatioNoWeight->Divide(hGen);
        hRatioNoWeight->SetTitle(Form("Ratio to Generated;%s;Ratio", hGen->GetXaxis()->GetTitle()));
        hRatioNoWeight->SetLineColor(kBlue);
        hRatioNoWeight->SetMarkerColor(kBlue);
        
        TH1D* hRatioWeight = (TH1D*)hRecoWeighted->Clone(Form("hRatioWeight_%s", varName));
        hRatioWeight->Divide(hGen);
        hRatioWeight->SetLineColor(kGreen+2);
        hRatioWeight->SetMarkerColor(kGreen+2);
        
        hRatioNoWeight->GetYaxis()->SetRangeUser(0.5, 1.5);
        hRatioNoWeight->Draw("HIST");
        hRatioWeight->Draw("HIST SAME");
        
        TLegend* legendRatio = new TLegend(0.65, 0.65, 0.88, 0.85);
        legendRatio->AddEntry(hRatioNoWeight, "Reconstructed/Generated", "l");
        legendRatio->AddEntry(hRatioWeight, "Weighted/Generated", "l");
        legendRatio->SetBorderSize(0);
        legendRatio->Draw();
        
        // Create a single canvas with two panels
        TCanvas* cCombined = new TCanvas(Form("cCombined_%s", varName), Form("%s Distributions and Ratios", varName), 800, 800);
        cCombined->Divide(1, 2); // 1 column, 2 rows
        
        // Upper panel for distributions
        cCombined->cd(1);
        gPad->SetLogy(1);
        gPad->SetBottomMargin(0.1);
        gPad->SetLeftMargin(0.15);
        
        hGen->Draw("HIST");
        hReco->Draw("HIST SAME");
        hRecoWeighted->Draw("HIST SAME");
        
        TLegend* legendCombined = new TLegend(0.65, 0.65, 0.88, 0.85);
        legendCombined->AddEntry(hGen, "Generated", "lp");
        legendCombined->AddEntry(hReco, "Reconstructed", "lp");
        legendCombined->AddEntry(hRecoWeighted, "Reconstructed (Weighted)", "lp");
        legendCombined->SetBorderSize(0);
        legendCombined->Draw();
        
        // Lower panel for ratios
        cCombined->cd(2);
        gPad->SetBottomMargin(0.15);
        gPad->SetLeftMargin(0.15);
        gPad->SetTopMargin(0.05);
        
        hRatioWeight->GetYaxis()->SetRangeUser(0.5, 1.5);
        hRatioWeight->GetYaxis()->SetTitle("Ratio to Gen");
        hRatioWeight->GetYaxis()->SetTitleSize(0.06);
        hRatioWeight->GetXaxis()->SetTitleSize(0.06);
        hRatioWeight->Draw("line");
        
        TLine* line = new TLine(hRatioWeight->GetXaxis()->GetXmin(), 1, hRatioWeight->GetXaxis()->GetXmax(), 1);
        line->SetLineColor(kRed);
        line->SetLineStyle(2);
        line->Draw("same");
        
        TLegend* legendRatioCombined = new TLegend(0.65, 0.65, 0.88, 0.85);
        legendRatioCombined->AddEntry(hRatioWeight, "Weighted/Generated", "l");
        legendRatioCombined->SetBorderSize(0);
        legendRatioCombined->Draw();
        
        // 결과 저장
        c->SaveAs(Form("%s_distribution_comparison.png", varName));
        cRatio->SaveAs(Form("%s_ratio_comparison.png", varName));
        cCombined->SaveAs(Form("%s_distributions_and_ratios.png", varName));
        
        // ROOT 파일에 저장
        c->Write();
        cRatio->Write();
        cCombined->Write();
        hGen->Write();
        hReco->Write();
        hRecoWeighted->Write();
        hRatioNoWeight->Write();
        hRatioWeight->Write();
    };
    
    // 각 변수에 대한 플롯 생성
    createPlotsForVariable(hGenPt, hRecoPt, hRecoPtWeighted, "pT");
    createPlotsForVariable(hGenY, hRecoY, hRecoYWeighted, "y");
    createPlotsForVariable(hGenPhi, hRecoPhi, hRecoPhiWeighted, "phi");
    
    outFile->Close();
    
    cout << "Results saved to reproduce_results.root" << endl;
    cout << "Plots saved for pT, y, and phi variables" << endl;
}