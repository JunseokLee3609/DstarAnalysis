#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.hxx"
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"

TH2* DrawCosThetaPhiDistributionFromTree(
    TTree* tree,                     // 입력 트리
    const char* cosThetaVarName,     // cosTheta 변수명
    const char* phiVarName,          // phi 변수명
    const char* cutExpression,       // 적용할 컷 표현식
    TCanvas *canvas,                 // 그릴 캔버스
    const char* frameAcronym = "CS", // 프레임 표기
    Int_t ptMin = 0,                 // pT 최소값
    Int_t ptMax = 30,                // pT 최대값
    const char* extraString = ""     // 추가 문자열
) {
    Double_t massMin = 1.9, massMax = 2.1;

    Int_t nCosThetaBins = 20;
    Float_t cosThetaMin = -1, cosThetaMax = 1;

    Int_t nPhiBins = 25;
    Float_t phiMin = -200, phiMax = 300;
    
    // 히스토그램 이름 설정
    const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);
    
    // 트리에서 히스토그램 생성 (컷 적용)
    TH2D* histo = new TH2D(histoName, histoName, 
                           nCosThetaBins, cosThetaMin, cosThetaMax,
                           nPhiBins, phiMin, phiMax);
    
    // 트리에서 히스토그램으로 데이터 채우기 (컷 적용)
    TString drawCommand = Form("%s:%s>>%s", phiVarName, cosThetaVarName, histoName);
    tree->Draw(drawCommand, cutExpression, "goff");  // 그래픽 출력 없이 히스토그램만 채우기
    
    // 히스토그램이 정상적으로 생성되었는지 확인
    if (histo->GetEntries() == 0) {
        std::cout << "Warning: No entries passed cut '" << cutExpression << "'" << std::endl;
    }
    
    // 캔버스로 전환하여 그리기
    canvas->cd();
    
    // 이하 기존 그리기 코드와 동일
    histo->SetTitle(Form(";cos #theta_{%s}; #varphi_{%s} (#circ)", frameAcronym, frameAcronym));
    histo->Draw("COLZ");

    TLatex legend;
    legend.SetTextAlign(22);
    legend.SetTextSize(0.05);
    legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

    histo->GetXaxis()->CenterTitle();
    histo->GetYaxis()->SetRangeUser(-200, 300);
    histo->GetYaxis()->CenterTitle();
    histo->GetZaxis()->SetMaxDigits(3);

    gPad->Update();

    return histo;
}

TH2* DrawCosThetaPhiDistribution(TH2D *histo, TCanvas *canvas, const char* cutExpression, const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
	Double_t massMin = 1.9, massMax = 2.1;

	Int_t nCosThetaBins = 20;
	Float_t cosThetaMin = -1, cosThetaMax = 1;

	Int_t nPhiBins = 25;
	Float_t phiMin = -200, phiMax = 300;
	/// Draw and save the number of events(signal+background) plots in the 2D (costheta, phi) space
	const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

	// TCanvas* canvas = new TCanvas(Form("canvas%s", frameAcronym), "canvas", 700, 600);

	histo->SetTitle(Form(";cos #theta_{%s}; #varphi_{%s} (#circ)", frameAcronym, frameAcronym));
	histo->Draw("COLZ");

	TLatex legend;
	legend.SetTextAlign(22);
	legend.SetTextSize(0.05);
	// legend.DrawLatexNDC(.48, .86, Form("centrality %d-%d%%, %d < p_{T}^{#mu#mu} < %d GeV/c", gCentralityBinMin, gCentralityBinMax, ptMin, ptMax));
	legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

	histo->GetXaxis()->CenterTitle();
	histo->GetYaxis()->SetRangeUser(-200, 300);
	histo->GetYaxis()->CenterTitle();
	histo->GetZaxis()->SetMaxDigits(3);

	gPad->Update();

	// CMS_lumi(canvas, gCMSLumiText);
	// canvas->SaveAs(Form("%s.png", histoName), "RECREATE");

	return histo;
}

TH1* DrawCosThetaDistribution(TH1D *histo, TCanvas *canvas, TString drawoption="", const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
	Double_t massMin = 1.9, massMax = 2.1;

	Int_t nCosThetaBins = 20;
	Float_t cosThetaMin = -1, cosThetaMax = 1;

	const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

	/// Draw and save the number of events(signal+background) plots in the 2D (costheta, phi) space

	// TCanvas* canvas = new TCanvas(Form("canvas1D%s", frameAcronym), "canvas", 700, 600);

	// TH1* histo = dynamic_cast<TH1*>(reducedDataset->createHistogram(histoName, cosThetaVar, RooFit::Binning(nCosThetaBins, cosThetaMin, cosThetaMax)));

	histo->SetTitle(Form(";cos (#theta_{%s})", frameAcronym));
	histo->Draw(drawoption.Data());
	histo->GetYaxis()->SetRangeUser(0,histo->GetMaximum()*1.2);

	TLatex legend;
	legend.SetTextAlign(22);
	legend.SetTextSize(0.05);
	// legend.DrawLatexNDC(.48, .86, Form("centrality %d-%d%%, %d < p_{T}^{#mu#mu} < %d GeV/c", gCentralityBinMin, gCentralityBinMax, ptMin, ptMax));
	legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

	histo->GetXaxis()->CenterTitle();
    histo->GetYaxis()->SetRangeUser(0, histo->GetMaximum()*1.2);
	// histo->GetYaxis()->SetRangeUser(-200, 200);
	// histo->GetYaxis()->CenterTitle();
	// histo->GetZaxis()->SetMaxDigits(3);

	gPad->Update();

	// CMS_lumi(canvas, gCMSLumiText);
	// canvas->SaveAs(Form("%s_1D.png", histoName), "RECREATE");
    // delete canvas;

	return histo;
}

double findWeight(Float_t pt, Float_t y, Float_t phi, TH3D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(pt, y,phi);
        // cout << "bin: " << bin << endl;
        double eff = effMap->GetBinContent(bin);
        if (eff > 0){
            // cout << "eff: " << eff << endl;
            weight = 1.0/eff;
        }
    // else {
    //     cout << "No weight found for pt: " << pt << " y: " << y << " phi: " << phi << endl;} 
    }
    return weight;
}

void test(
// string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/output_PbPb_noFilter_wOnlyGEN.root"
// string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root"
    // string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/output_PbPb_noFilter_wOnlyGEN.root"
    // string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root"
    string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Mar30NonSwap/d0ana_tree_nonswapsample_ppref_30Mar.root"
){
    TFile* f = TFile::Open(fileName.c_str());
    TFile* feffMap = TFile::Open("output.root");

    string name_tree = "dStarana_mc/PATCompositeNtuple";
    /* Get tree*/
    auto t = (TTree*) f->Get(name_tree.c_str());
    
    DataFormat::simpleDStarMCTreeevt evtMC;
    evtMC.setTree(t);
    evtMC.setGENTree(t);
    int nEvts;
    nEvts = t->GetEntries();
    
    // 기존 히스토그램
    TH1D* h = new TH1D("HX", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hNW = new TH1D("HX NoWeight", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN = new TH1D("HX Gen", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN2 = new TH1D("HX Gen2", "HX;cos(#theta_{HX})", 10, -1, 1);
    
    // D1 전하에 따른 분리 히스토그램 추가
    TH1D* hGEN_pos = new TH1D("HX Gen D1+", "HX D^{+};cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN_neg = new TH1D("HX Gen D1-", "HX D^{-};cos(#theta_{HX})", 10, -1, 1);
    
    // 2D 히스토그램도 전하별로 분리
    TH2D* h2DGEN = new TH2D("HX 2D Gen", "HX;cos(#theta_{HX});#phi", 30, -1, 1, 100, -300, 300);
    TH2D* h2DGEN_pos = new TH2D("HX 2D Gen D1+", "HX D^{+};cos(#theta_{HX});#phi", 30, -1, 1, 100, -300, 300);
    TH2D* h2DGEN_neg = new TH2D("HX 2D Gen D1-", "HX D^{-};cos(#theta_{HX});#phi", 30, -1, 1, 100, -300, 300);
    
    auto fillGEN = [&](int i){
        TLorentzVector Dstar, D0, D1, D0Dau1, D0Dau2;
        Dstar.SetPtEtaPhiM(evtMC.gen_pT[i], evtMC.gen_eta[i], evtMC.gen_phi[i], evtMC.gen_mass[i]);
        D0.SetPtEtaPhiM(evtMC.gen_D0pT[i], evtMC.gen_D0eta[i], evtMC.gen_D0phi[i], evtMC.gen_D0mass[i]);
        D1.SetPtEtaPhiM(evtMC.gen_D0pT[i], evtMC.gen_D0eta[i], evtMC.gen_D0phi[i], evtMC.gen_D0mass[i]);
        D0Dau1.SetPtEtaPhiM(evtMC.gen_D0Dau1_pT[i], evtMC.gen_D0Dau1_eta[i], evtMC.gen_D0Dau1_phi[i], evtMC.gen_D0Dau1_mass[i]);
        D0Dau2.SetPtEtaPhiM(evtMC.gen_D0Dau2_pT[i], evtMC.gen_D0Dau2_eta[i], evtMC.gen_D0Dau2_phi[i], evtMC.gen_D0Dau2_mass[i]);
        
        // 기본 조건 검사
        if(evtMC.gen_pT[i] > DSGLPTLO && fabs(evtMC.gen_y[i]) < DSGLABSY) {

            TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);
            
            // 공통 히스토그램 채우기
            hGEN->Fill(vect.CosTheta());
            h2DGEN->Fill(vect.CosTheta(), vect.Phi()/TMath::Pi()*180);
            
            // D1 전하에 따라 분리
            if(evtMC.gen_D1pdgId[i] == 211) {
                // 양전하 히스토그램 채우기
                hGEN_pos->Fill(vect.CosTheta());
                h2DGEN_pos->Fill(vect.CosTheta(), vect.Phi()/TMath::Pi()*180);
            }             else if(evtMC.gen_D1pdgId[i] == -211) {
                // 음전하 히스토그램 채우기
                hGEN_neg->Fill(vect.CosTheta());
                h2DGEN_neg->Fill(vect.CosTheta(), vect.Phi()/TMath::Pi()*180);
            }
        }
        if(
        
            evtMC.gen_pT[i] > DSGLPTLO && 
            evtMC.gen_pT[i] < 6 && 
            true){
            TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);
            hGEN2->Fill(vect.CosTheta());
            }
    };
auto fill2DGEN = [&](int i){
        TLorentzVector Dstar, D0;
        Dstar.SetPtEtaPhiM(evtMC.gen_pT[i], evtMC.gen_eta[i], evtMC.gen_phi[i], evtMC.gen_mass[i]);
        D0.SetPtEtaPhiM(evtMC.gen_D0pT[i], evtMC.gen_D0eta[i], evtMC.gen_D0phi[i], evtMC.gen_D0mass[i]);
           if(
        evtMC.gen_pT[i] > DSGLPTLO && 
        fabs(evtMC.gen_y[i]) < DSGLABSY && 
        true){
        
        TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);
    }
};

auto fill = [&](int i, TH3D* effMap){
    if(evtMC.matchGEN[i]  && !evtMC.isSwap[i])
    {
        TLorentzVector Dstar, D0;
        Dstar.SetPtEtaPhiM(evtMC.pT[i], evtMC.eta[i], evtMC.phi[i], evtMC.mass[i]);
        D0.SetPtEtaPhiM(evtMC.pTD1[i], evtMC.EtaD1[i], evtMC.PhiD1[i], evtMC.massDaugther1[i]);
        TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);

        double weight = findWeight(evtMC.pT[i], evtMC.y[i], evtMC.phi[i], effMap);
        hNW->Fill(vect.CosTheta());
        h->Fill(vect.CosTheta(), weight);
    }
};
for( auto idx : ROOT::TSeqU(nEvts)){
    t->GetEntry(idx);
    for( auto i : ROOT::TSeqI(evtMC.candSize)){
    fill(i,(TH3D*)feffMap->Get("pt_y_phi_pr_ratio"));
    }
    for( auto i : ROOT::TSeqI(evtMC.candSize_gen)){
    fillGEN(i);
    }
} 
    
    // 시각화
    TCanvas* c = new TCanvas("c", "c", 800, 600);
    c->Divide(1, 2);
    
    // 1D 히스토그램 그리기
    c->cd(1);
    hGEN->SetStats(0);
    hGEN->SetLineColor(kBlack);
    hGEN->SetLineWidth(2);
    hGEN->GetYaxis()->SetRangeUser(0, std::max(hGEN_pos->GetMaximum(), hGEN_neg->GetMaximum())*1.2);
    hGEN->Draw("HIST");
    
    // 양전하 히스토그램
    hGEN_pos->SetLineColor(kRed);
    hGEN_pos->SetLineWidth(2);
    hGEN_pos->Draw("HIST SAME");
    
    // 음전하 히스토그램
    hGEN_neg->SetLineColor(kBlue);
    hGEN_neg->SetLineWidth(2);
    hGEN_neg->Draw("HIST SAME");
    
    TLegend* leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(hGEN, "All D*", "l");
    leg->AddEntry(hGEN_pos, "D1 charge +", "l");
    leg->AddEntry(hGEN_neg, "D1 charge -", "l");
    leg->SetBorderSize(0);
    leg->Draw();
    
    // 2D 히스토그램 비교 (예시로 양전하만)
    c->cd(2);
    h2DGEN_pos->Draw("COLZ");
    
    // 두 번째 캔버스에 2D 히스토그램 비교
    TCanvas* c2 = new TCanvas("c2", "c2", 1200, 400);
    c2->Divide(3, 1);
    
    c2->cd(1);
    h2DGEN->Draw("COLZ");
    c2->cd(1)->SetTitle("All D*");
    
    c2->cd(2);
    h2DGEN_pos->Draw("COLZ");
    c2->cd(2)->SetTitle("D1 charge +");
    
    c2->cd(3);
    h2DGEN_neg->Draw("COLZ");
    c2->cd(3)->SetTitle("D1 charge -");
    
    c->SaveAs("dstar_costheta_by_charge_1d.pdf");
    c2->SaveAs("dstar_costheta_by_charge_2d.pdf");
    
    // 비율 히스토그램 계산
    TH1D* hGEN_ratio = (TH1D*)hGEN_pos->Clone("HX Gen Ratio");
    hGEN_ratio->SetTitle("D^{+}/D^{-};cos(#theta_{HX});Ratio");
    hGEN_ratio->Divide(hGEN_neg);
    
    TCanvas* c3 = new TCanvas("c3", "c3", 800, 600);
    hGEN_ratio->SetLineColor(kViolet);
    hGEN_ratio->SetMarkerStyle(20);
    hGEN_ratio->Draw("E");
    
    // 비율 = 1 기준선 그리기
    TLine* line = new TLine(-1, 1, 1, 1);
    line->SetLineStyle(2);
    line->SetLineColor(kRed);
    line->Draw();
    
    c3->SaveAs("dstar_costheta_charge_ratio.pdf");

    TCanvas* c4 = new TCanvas("c4", "c4", 800, 600);
       
    c4->Divide(3,1);
    c4->cd(1);
    h->GetYaxis()->SetRangeUser(0, h->GetMaximum()*1.2);
    h->Draw();
    c4->cd(2);
    hNW->GetYaxis()->SetRangeUser(0, hNW->GetMaximum()*1.2);
    hNW->Draw();
    c4->cd(3);
    
    hGEN->GetYaxis()->SetRangeUser(0, hGEN->GetMaximum()*1.2);
    hGEN->SetMarkerStyle(20);
    hGEN->SetMarkerSize(0.5);
    hGEN->SetMarkerColor(kRed);
    hGEN->SetLineColor(kRed);
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.5);
    h->SetMarkerColor(kBlue);
    h->SetLineColor(kBlue);
    TLegend* leg2 = new TLegend(0.11,0.11,0.28,0.25);
    leg2->AddEntry(hGEN, "GEN", "l");
    leg2->AddEntry(h, "reco with weight", "l");
    leg2->SetBorderSize(0);
    hGEN->Draw("E");
    h->Draw("same");
    leg->Draw("same");
    c4->SaveAs("test.png");
}
