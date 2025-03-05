#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.hxx"
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
    string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root"
){
    TFile* f =  TFile::Open(fileName.c_str());
    TFile* feffMap = TFile::Open("output.root");

    string name_tree = "dStarana_mc/VertexCompositeNtuple";
    /* Get tree*/
    auto t = (TTree*) f->Get(name_tree.c_str());
    DataFormat::simpleDStarMCTreeevt evtMC;
    // DataFormat::simpleDStarDataTreeflat evtData;
    evtMC.setTree(t);
    evtMC.setGENTree(t);
    int nEvts;
    nEvts = t->GetEntries();
    TH1D* h = new TH1D("HX", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hNW = new TH1D("HX NoWeight", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN = new TH1D("HX Gen", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH1D* hGEN2 = new TH1D("HX Gen2", "HX;cos(#theta_{HX})", 10, -1, 1);
    TH2D* h2DGEN = new TH2D("HX 2D Gen", "HX;cos(#theta_{HX};#phi)", 30, -1, 1, 100, -300, 300);
    auto fillGEN = [&](int i){
            TLorentzVector Dstar, D0, D1,D0Dau1,D0Dau2;
            Dstar.SetPtEtaPhiM(evtMC.gen_pT[i], evtMC.gen_eta[i], evtMC.gen_phi[i], evtMC.gen_mass[i]);
            D0.SetPtEtaPhiM(evtMC.gen_D0pT[i], evtMC.gen_D0eta[i], evtMC.gen_D0phi[i], evtMC.gen_D0mass[i]);
            D1.SetPtEtaPhiM(evtMC.gen_D0pT[i], evtMC.gen_D0eta[i], evtMC.gen_D0phi[i], evtMC.gen_D0mass[i]);
            D0Dau1.SetPtEtaPhiM(evtMC.gen_D0Dau1_pT[i], evtMC.gen_D0Dau1_eta[i], evtMC.gen_D0Dau1_phi[i], evtMC.gen_D0Dau1_mass[i]);
            D0Dau2.SetPtEtaPhiM(evtMC.gen_D0Dau2_pT[i], evtMC.gen_D0Dau2_eta[i], evtMC.gen_D0Dau2_phi[i], evtMC.gen_D0Dau2_mass[i]);
                if(
            // abs(evtPR.gen_D1ancestorId[igen]) < 500 && 
            // std::min(abs(evtPR.DauID1_gen[igen]), abs(evtPR.DauID2_gen[igen])) ==PDG_DAU1 && 
            // std::max(abs(evtPR.DauID1_gen[igen]), abs(evtPR.DauID2_gen[igen])) ==PDG_DAU2 && 
            evtMC.gen_pT[i] > DSGLPTLO && 
            evtMC.gen_pT[i] < 6 && 
            // evtMC.gen_pT[i] < DSGLPTHI &&
            // evtMC.gen_D0pT[i] > DSGLPT_DAU1 &&
            // evtMC.gen_D1pT[i] > DSGLPT_DAU2 && 
            // fabs(evtMC.gen_D0eta[i]) < DSGLABSETA_DAU1 &&
            // fabs(evtMC.gen_D1eta[i]) < DSGLABSETA_DAU2 && 
            fabs(evtMC.gen_y[i]) < DSGLABSY && 
            DstarDauSimpleAcc(D0,D1) &&
            D0DauAcc(D0Dau1,D0Dau2) &&
            // evtPR.angle2D_gen[igen] < 1.0 && 
            // evtPR.angle3D_gen[igen] < 1.0 && 
            true){
            TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);
            hGEN->Fill(vect.CosTheta());
            h2DGEN->Fill(vect.CosTheta(),vect.Phi()/TMath::Pi()*180);}
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
        // for( auto i : ROOT::TSeqI(evtMC.candSize)){
        // fill(i,(TH3D*)feffMap->Get("pt_y_phi_pr_ratio"));
        // }
        for( auto i : ROOT::TSeqI(evtMC.candSize_gen)){
        fillGEN(i);
        }
    }
    TCanvas* c = new TCanvas("c", "c",800, 600);
    TCanvas* c1 = new TCanvas("c1", "c1",800, 600);
    TH2 *htest = DrawCosThetaPhiDistribution(h2DGEN,c1, "HX");
    TH1 *h1test2 = DrawCosThetaDistribution(hGEN2,c,"", "HX");
    TH1 *h1test = DrawCosThetaDistribution(hGEN,c,"same", "HX");
    CMS_lumi(c, gCMSLumiText);
    CMS_lumi(c1, gCMSLumiText);
    c1->SaveAs("test.png");
    c->SaveAs("test2D.png");

    
    
    // c->Divide(3,1);
    // c->cd(1);
    // h->GetYaxis()->SetRangeUser(0, h->GetMaximum()*1.2);
    // h->Draw();
    // c->cd(2);
    // hNW->GetYaxis()->SetRangeUser(0, hNW->GetMaximum()*1.2);
    // hNW->Draw();
    // c->cd(3);
    
    // hGEN->GetYaxis()->SetRangeUser(0, hGEN->GetMaximum()*1.2);
    // hGEN->SetMarkerStyle(20);
    // hGEN->SetMarkerSize(0.5);
    // hGEN->SetMarkerColor(kRed);
    // hGEN->SetLineColor(kRed);
    // h->SetMarkerStyle(20);
    // h->SetMarkerSize(0.5);
    // h->SetMarkerColor(kBlue);
    // h->SetLineColor(kBlue);
    // TLegend* leg = new TLegend(0.11,0.11,0.28,0.25);
    // leg->AddEntry(hGEN, "GEN", "l");
    // leg->AddEntry(h, "reco with weight", "l");
    // leg->SetBorderSize(0);
    // hGEN->Draw("E");
    // h->Draw("same");
    // leg->Draw("same");
    // c->SaveAs("test.png");


}