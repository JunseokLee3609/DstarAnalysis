#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.h" // 기존 헤더, 플랫 구조체 정의가 필요하면 수정 또는 추가
// #include "../interface/simpleDMCTreeflat.h" // 플랫 구조체 정의 헤더 (가정)
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"


TH2* DrawCosThetaPhiDistribution(TH2D *histo, TCanvas *canvas,  const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
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
        // if (eff > 0){
            // cout << "eff: " << eff << endl;
            weight = 1.0/eff;
        // }
    // else {
    //     cout << "No weight found for pt: " << pt << " y: " << y << " phi: " << phi << endl;} 
    }
    return weight;
}
double findWeight(Float_t x, Float_t y, TH2D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(x,y);
        // cout << "bin: " << bin << endl;
        double eff = effMap->GetBinContent(bin);
        if (eff > 0){
            // cout << "eff: " << eff << endl;
            weight = 1.0/eff;
        }
    // else {
    //     cout << "No weight found for pt: " << pt " y: " << y << " phi: " << phi << endl;} 
    }
    return weight;
}
double findWeight(Float_t x, TH1D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(x);
        // cout << "bin: " << bin << endl;
        double eff = effMap->GetBinContent(bin);
        // if (eff > 0){
            // cout << "eff: " << eff << endl;
            weight = 1.0/eff;
        // }
    // else {
    //     cout << "No weight found for pt: " << pt << " y: " << y << " phi: " << phi << endl;} 
    }
    return weight;
}

void test2_cp_Flat(
    // string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/output_PbPb_noFilter_wOnlyGEN.root"
    // string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root"
    // string fileName="d0ana_tree_step2.root"
    //string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Mar30NonSwap/d0ana_tree_nonswapsample_ppref_30Mar.root"
    string fileName="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/flatSkimForBDT_DStar_PPRef_PromptDStar_02Jun_0.root"
    // string fileName="/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/d0ana_tree_nokinematic_Pi0p4pT.root"
){
    TFile* f =  TFile::Open(fileName.c_str());
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Could not open file: " << fileName << std::endl;
        return;
    }
    TFile* feffMap = TFile::Open("output.root"); // Assuming this is still the efficiency map file
    if (!feffMap || feffMap->IsZombie()) {
        std::cerr << "Error: Could not open efficiency map file." << std::endl;
        f->Close();
        return;
    }

    // 가정한 플랫 트리의 이름. 실제 트리의 이름으로 변경해야 합니다.
    string name_gen_tree = "skimGENTreeFlat"; 
    string name_reco_tree = "skimTreeFlat";

    TTree* tGen = (TTree*) f->Get(name_gen_tree.c_str());
    TTree* tReco = (TTree*) f->Get(name_reco_tree.c_str());

    if (!tGen) {
        std::cerr << "Error: Could not find GEN tree: " << name_gen_tree << " in file " << fileName << std::endl;
        f->Close();
        feffMap->Close();
        return;
    }
    if (!tReco) {
        std::cerr << "Error: Could not find RECO tree: " << name_reco_tree << " in file " << fileName << std::endl;
        f->Close();
        feffMap->Close();
        return;
    }

    DataFormat::simpleDStarMCTreeflat evtGen; // 플랫 구조체 사용 (GEN용)
    DataFormat::simpleDStarMCTreeflat evtReco; // 플랫 구조체 사용 (RECO용)
    
    // setGENTree와 setTree는 simpleDMCTreeflat 클래스에 정의되어 있다고 가정
    evtGen.setGENTree(tGen); 
    evtReco.setTree(tReco);

    long long nGenEntries = tGen->GetEntries();
    long long nRecoEntries = tReco->GetEntries();

    TH1D* h = new TH1D("HX", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hNW = new TH1D("HX NoWeight", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN = new TH1D("HX Gen", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN2 = new TH1D("HX Gen2", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH2D* h2DGEN = new TH2D("HX 2D Gen", "HX;cos(#theta_{HX};#phi)", 30, -1, 1, 100, -300, 300);
    
    // fillGEN 람다 함수 수정: 인덱스 'i' 제거, evtGen의 직접 멤버 접근
    auto fillGEN = [&](){ // 인덱스 i 제거
            TLorentzVector Dstar, D0, D1,D0Dau1,D0Dau2;
            // evtGen의 멤버 변수명은 simpleDMCTreeflat 구조체 정의에 따라야 함
            // 여기서는 test2_cp_Flat.cpp의 원래 변수명에서 [i]를 제거하고 gen_ 접두사를 사용한 형태로 가정
            Dstar.SetPtEtaPhiM(evtGen.gen_pT, evtGen.gen_eta, evtGen.gen_phi, evtGen.gen_mass);
            D0.SetPtEtaPhiM(evtGen.gen_D0pT, evtGen.gen_D0eta, evtGen.gen_D0phi, evtGen.gen_D0mass);
            D1.SetPtEtaPhiM(evtGen.gen_D1pT, evtGen.gen_D1eta, evtGen.gen_D1phi, evtGen.gen_D1mass); // D1은 soft pion으로 가정
            D0Dau1.SetPtEtaPhiM(evtGen.gen_D0Dau1_pT, evtGen.gen_D0Dau1_eta, evtGen.gen_D0Dau1_phi, evtGen.gen_D0Dau1_mass);
            D0Dau2.SetPtEtaPhiM(evtGen.gen_D0Dau2_pT, evtGen.gen_D0Dau2_eta, evtGen.gen_D0Dau2_phi, evtGen.gen_D0Dau2_mass);
            
            // 검증 로직은 필요에 따라 유지 또는 수정
            if(fabs(evtGen.gen_pT-Dstar.Pt()) > 0.01) std::cout << "Dstar pT: " << Dstar.Pt() << " D0 pT: " << evtGen.gen_pT << std::endl;
            if(fabs(evtGen.gen_y-Dstar.Rapidity()) > 0.01) std::cout << "Dstar y: " << Dstar.Rapidity() << " D0 y: " << evtGen.gen_y << std::endl;
            // ... (다른 검증 로직)

            if(
            evtGen.gen_pT > DSGLPTLO && 
            fabs(evtGen.gen_y) < DSGLABSY && 
            DstarDauSimpleAcc(D0,D1) && // D1은 soft pion으로 가정
            D0DauAcc(D0Dau1,D0Dau2) &&
            true){
            TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);
            hGEN->Fill(vect.CosTheta());
            h2DGEN->Fill(vect.CosTheta(),vect.Phi()/TMath::Pi()*180);}
            if(
            evtGen.gen_pT > DSGLPTLO && 
            fabs(evtGen.gen_y) < DSGLABSY && 
            true){
            TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);
            hGEN2->Fill(vect.CosTheta());
            }
        };
    
    // fill 람다 함수 수정: 인덱스 'i' 제거, evtReco의 직접 멤버 접근
    auto fill = [&](){ // 인덱스 i 제거
            // evtReco의 멤버 변수명은 simpleDMCTreeflat 구조체 정의에 따라야 함
            // 예: evtReco.D0_y, evtReco.matchGEN, evtReco.isSwap 등
            auto D0y= algo::rapidity(evtReco.pTD1, evtReco.EtaD1, evtReco.PhiD1, evtReco.massDaugther1); // pTD1 등을 D0 관련 변수로 가정
        if(evtReco.matchGEN  && !evtReco.isSwap && // matchGEN, isSwap은 evtReco의 멤버로 가정
            // fabs(evtReco.y) < DSGLABSY &&
            // evtReco.pT > DSGLPTLO &&
            // fabs(D0y) < DSGLABSY_D0 &&
            // fabs(evtReco.EtaD2) < DSGLABSETA_D1 &&  // EtaD2 등을 soft pion 관련 변수로 가정
            // evtReco.pTD2 > DSGLPT_D1 && 
            // fabs(evtReco.EtaGrandD1) < DSGLABSETA_D0DAU1 && // GrandD1/2를 D0 딸입자 관련 변수로 가정
            // fabs(evtReco.EtaGrandD2) < DSGLABSETA_D0DAU2 &&
            // evtReco.pTGrandD1 > DSGLPT_D0DAU1 &&
            // evtReco.pTGrandD2 > DSGLPT_D0DAU2 &&
            true)
        {
            TLorentzVector Dstar, D0;
            // cout << "pT: " << evtReco.pT << ", eta: " << evtReco.eta << ", phi: " << evtReco.phi << ", mass: " << evtReco.mass << endl;
            // cout << "pTD1: " << evtReco.pTD1 << ", EtaD1: " << evtReco.EtaD1 << ", PhiD1: " << evtReco.PhiD1 << ", massDaugther1: " << evtReco.massDaugther1 << endl;
            Dstar.SetPtEtaPhiM(evtReco.pT, evtReco.eta, evtReco.phi, evtReco.mass);
            D0.SetPtEtaPhiM(evtReco.pTD1, evtReco.EtaD1, evtReco.PhiD1, evtReco.massDaugther1);
            TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);

            // double weight = findWeight(evtReco.pT, vect.CosTheta(), effMap);
            double weight;
            weight =1;
            hNW->Fill(vect.CosTheta());
            h->Fill(vect.CosTheta(), weight);
        }
    };

    std::cout << "Processing GEN entries: " << nGenEntries << std::endl;
    for( long long idx = 0; idx < nGenEntries; ++idx){
        if (idx % 100000 == 0) std::cout << " GEN Entry: " << idx << "/" << nGenEntries << std::endl;
        tGen->GetEntry(idx);
        fillGEN(); // 내부 루프 제거
    }

    std::cout << "Processing RECO entries: " << nRecoEntries << std::endl;
    for( long long idx = 0; idx < nRecoEntries; ++idx){
        if (idx % 100000 == 0) std::cout << " RECO Entry: " << idx << "/" << nRecoEntries << std::endl;
        tReco->GetEntry(idx);
        // fill 함수에 전달하는 effMap의 이름 확인 필요
        // fill((TH2D*)feffMap->Get("pt_cos_pr_pass_ratio")); // 또는 "pt_cos_pass_pr_ratio"
        fill();
    }
    
    TCanvas* c = new TCanvas("c", "c",800, 600);
    TCanvas* c1 = new TCanvas("c1", "c1",800, 600);
    // TH2 *htest = DrawCosThetaPhiDistribution(h2DGEN,c1, "HX");
    // TH1 *h1test2 = DrawCosThetaDistribution(hGEN2,c,"", "HX");
    // TH1 *h1test = DrawCosThetaDistribution(hGEN,c,"same", "HX");
    // CMS_lumi(c, gCMSLumiText);
    // CMS_lumi(c1, gCMSLumiText);
    // c1->SaveAs("test_2.png");
    // c->SaveAs("test2D_2.png");

    
    
    c->Divide(3,1);
    c->cd(1);
    h->GetYaxis()->SetRangeUser(0, h->GetMaximum()*1.2);
    h->Draw();
    c->cd(2);
    hNW->GetYaxis()->SetRangeUser(0, hNW->GetMaximum()*1.2);
    hNW->Draw();
    c->cd(3);
    
    hGEN2->GetYaxis()->SetRangeUser(0, hGEN2->GetMaximum()*1.2);
    hGEN2->SetMarkerStyle(20);
    hGEN2->SetMarkerSize(0.5);
    hGEN2->SetMarkerColor(kRed);
    hGEN2->SetLineColor(kRed);
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.5);
    h->SetMarkerColor(kBlue);
    h->SetLineColor(kBlue);
    TLegend* leg = new TLegend(0.11,0.11,0.28,0.25);
    leg->AddEntry(hGEN, "GEN", "l");
    leg->AddEntry(h, "reco with weight", "l");
    leg->SetBorderSize(0);
    hGEN2->Draw("E");
    h->Draw("same");
    leg->Draw("same");
    c->SaveAs("test_2.png");
    TCanvas* c2 = new TCanvas("c2", "c2",800, 600);
    c2->cd();
    hGEN2->GetYaxis()->SetRangeUser(0, hGEN2->GetMaximum()*1.2);
    hGEN2->SetMarkerStyle(20);
    hGEN2->SetMarkerSize(0.5);
    hGEN2->SetMarkerColor(kRed);
    hGEN2->SetLineColor(kRed);
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.5);
    h->SetMarkerColor(kBlue);
    h->SetLineColor(kBlue);
    TLegend* leg2 = new TLegend(0.11,0.11,0.28,0.25);
    leg2->AddEntry(hGEN2, "GEN", "l");
    leg2->AddEntry(h, "reco with weight", "l");
    leg2->SetBorderSize(0);
    hGEN2->Draw("E");
    h->Draw("same");
    leg2->Draw("same");
    gStyle->SetOptStat(0);
    c2->SaveAs("test_3.png");

    f->Close();
    feffMap->Close();
    // 생성된 객체들 delete 처리 추가 권장
    // delete h; delete hNW; delete hGEN; delete hGEN2; delete h2DGEN;
    // delete c; delete c1; delete c2; delete leg; delete leg2;
}
