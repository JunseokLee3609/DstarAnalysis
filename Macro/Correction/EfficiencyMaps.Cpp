#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/PhaseSpace.h"

#include "AccEffHelpers.h"
#include "../Tools/Style/Legends.h"

#include "../Tools/Transformations.h"
void EfficiencyMaps(Int_t ptMin = 0, Int_t ptMax = 2, Int_t iState = gUpsilonState, Float_t lambdaTheta = 0, Float_t lambdaPhi = 0, Float_t lambdaThetaPhi = 0) {
	const char* filename = Form("/home/jun502s/DstarAna/DStarAnalysis/Data/flatSkimForBDT_isMC1_0_250113.root", iState);
	TFile* file = TFile::Open(filename, "READ");
	if (!file) {
		cout << "File " << filename << " not found. Check the directory of the file." << endl;
		return;
	}

	cout << "File " << filename << " opened" << endl;

	writeExtraText = true; // if extra text
	extraText = "       Internal";

	TTree* RecoTree = (TTree*)file->Get("skimTreeFlat");
	TTree* GenTree = (TTree*)file->Get("skimGENTreeFlat");

	/// OniaTree variables

	// Float_t Gen_weight;	

	Float_t zVtx;
	// ULong64_t HLTriggers;
	
	Int_t candSize;
	Int_t candSize_gen;
    Float_t Dstar_Gen_mass, Dstar_Gen_y, Dstar_Gen_pT, Dstar_Gen_eta, Dstar_Gen_phi;
    Float_t Dstar_Gen_Dau1_pT, Dstar_Gen_Dau1_y, Dstar_Gen_Dau1_eta, Dstar_Gen_Dau1_phi, Dstar_Gen_Dau1_mass;
    Float_t Dstar_Gen_Dau2_pT, Dstar_Gen_Dau2_y, Dstar_Gen_Dau2_eta, Dstar_Gen_Dau2_phi, Dstar_Gen_Dau2_mass;
    Float_t Dstar_mass, Dstar_y, Dstar_pT, Dstar_eta, Dstar_phi;
    Float_t Dstar_Dau1_pT, Dstar_Dau1_y, Dstar_Dau1_eta, Dstar_Dau1_phi, Dstar_Dau1_mass;
    Float_t Dstar_Dau2_pT, Dstar_Dau2_y, Dstar_Dau2_eta, Dstar_Dau2_phi, Dstar_Dau2_mass;
    GenTree->SetBranchAddress("gen_mass", &Dstar_Gen_mass);
    GenTree->SetBranchAddress("gen_y", &Dstar_Gen_y);
    GenTree->SetBranchAddress("gen_pT", &Dstar_Gen_pT);
    GenTree->SetBranchAddress("gen_eta", &Dstar_Gen_eta);
    GenTree->SetBranchAddress("gen_phi", &Dstar_Gen_phi);
    GenTree->SetBranchAddress("gen_D0pT", &Dstar_Gen_Dau1_pT);
    GenTree->SetBranchAddress("gen_D0eta", &Dstar_Gen_Dau1_eta);
    GenTree->SetBranchAddress("gen_D0phi", &Dstar_Gen_Dau1_phi);
    GenTree->SetBranchAddress("gen_D0mass", &Dstar_Gen_Dau1_mass);
    GenTree->SetBranchAddress("gen_D1pT", &Dstar_Gen_Dau2_pT);
    GenTree->SetBranchAddress("gen_D1eta", &Dstar_Gen_Dau2_eta);
    GenTree->SetBranchAddress("gen_D1phi", &Dstar_Gen_Dau2_phi);
    GenTree->SetBranchAddress("gen_D1mass", &Dstar_Gen_Dau2_mass);
    RecoTree->SetBranchAddress("matchGen_D0mass", &Dstar_mass);
    RecoTree->SetBranchAddress("matchGen_D0y", &Dstar_y);
    RecoTree->SetBranchAddress("matchGen_D0pT", &Dstar_pT);
    RecoTree->SetBranchAddress("matchGen_D0eta", &Dstar_eta);
    RecoTree->SetBranchAddress("matchGen_D0phi", &Dstar_phi);
    RecoTree->SetBranchAddress("matchGen_D0Dau1_pT", &Dstar_Dau1_pT);
    RecoTree->SetBranchAddress("matchGen_D0Dau1_eta", &Dstar_Dau1_eta);
    RecoTree->SetBranchAddress("matchGen_D0Dau1_phi", &Dstar_Dau1_phi);
    RecoTree->SetBranchAddress("matchGen_D0Dau2_pT", &Dstar_Dau2_pT);
    RecoTree->SetBranchAddress("matchGen_D0Dau2_eta", &Dstar_Dau2_eta);
    RecoTree->SetBranchAddress("matchGen_D0Dau2_phi", &Dstar_Dau2_phi);
	/// (cos theta, phi, pT) 3D distribution maps for CS and HX frames
	// TH3D* hGenLab3D = new TH3D("hGenLab3D", "Gen;cosTheta;phi;pT", 50, -1, 1, 50, -3.14, 3.14, 50, ptMin, ptMax);
    // TH3D* hRecoLab3D = new TH3D("hRecoLab3D", "Reco;cosTheta;phi;pT", 50, -1, 1, 50, -3.14, 3.14, 50, ptMin, ptMax);

    // TH2D* hGenLab2D = new TH2D("hGenLab2D", "Gen;cosTheta;phi", 50, -1, 1, 50, -3.14, 3.14);
    // TH2D* hRecoLab2D = new TH2D("hRecoLab2D", "Reco;cosTheta;phi", 50, -1, 1, 50, -3.14, 3.14);

    TH1D* hGenLab1D = new TH1D("hGenLab1D", "Gen;cosTheta", NPtFineBinning, gPtFineBinning);
    TH1D* hRecoLab1D = new TH1D("hRecoLab1D", "Reco;cosTheta", NPtFineBinning, gPtFineBinning);

    // TH3D* hGenCS3D = new TH3D("hGenCS3D", "Gen;cosTheta;phi;pT", 50, -1, 1, 50, -3.14, 3.14, 50, ptMin, ptMax);
    // TH3D* hRecoCS3D = new TH3D("hRecoCS3D", "Reco;cosTheta;phi;pT", 50, -1, 1, 50, -3.14, 3.14, 50, ptMin, ptMax);

    // TH2D* hGenCS2D = new TH2D("hGenCS2D", "Gen;cosTheta;phi", 50, -1, 1, 50, -3.14, 3.14);
    // TH2D* hRecoCS2D = new TH2D("hRecoCS2D", "Reco;cosTheta;phi", 50, -1, 1, 50, -3.14, 3.14);

    TH1D* hGenCS1D = new TH1D("hGenCS1D", "Gen;cosTheta", 50, -1, 1);
    TH1D* hRecoCS1D = new TH1D("hRecoCS1D", "Reco;cosTheta", 50, -1, 1);

    // TH3D* hGenHX3D = new TH3D("hGenHX3D", "Gen;cosTheta;phi;pT", 50, -1, 1, 50, -3.14, 3.14, 50, ptMin, ptMax);
    // TH3D* hRecoHX3D = new TH3D("hRecoHX3D", "Reco;cosTheta;phi;pT", 50, -1, 1, 50, -3.14, 3.14, 50, ptMin, ptMax);

    // TH2D* hGenHX2D = new TH2D("hGenHX2D", "Gen;cosTheta;phi", 50, -1, 1, 50, -3.14, 3.14);
    // TH2D* hRecoHX2D = new TH2D("hRecoHX2D", "Reco;cosTheta;phi", 50, -1, 1, 50, -3.14, 3.14);

    TH1D* hGenHX1D = new TH1D("hGenHX1D", "Gen;cosTheta", 50, -1, 1);
    TH1D* hRecoHX1D = new TH1D("hRecoHX1D", "Reco;cosTheta", 50, -1, 1);

    // Set branch addresses for GenTre

	// Collins-Soper

	TEfficiency* hNominalEffCS;

	// TEfficiency* hCS_trk_systUp = TEfficiency3D(Form("hCS_trk_systUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_trk_systDown = TEfficiency3D(Form("hCS_trk_systDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_trk_statUp = TEfficiency3D(Form("hCS_trk_statUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_trk_statDown = TEfficiency3D(Form("hCS_trk_statDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);

	// TEfficiency* hCS_muId_systUp = TEfficiency3D(Form("hCS_muId_systUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_muId_systDown = TEfficiency3D(Form("hCS_muId_systDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_muId_statUp = TEfficiency3D(Form("hCS_muId_statUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_muId_statDown = TEfficiency3D(Form("hCS_muId_statDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);

	// TEfficiency* hCS_trig_systUp = TEfficiency3D(Form("hCS_trig_systUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_trig_systDown = TEfficiency3D(Form("hCS_trig_systDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_trig_statUp = TEfficiency3D(Form("hCS_trig_statUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);
	// TEfficiency* hCS_trig_statDown = TEfficiency3D(Form("hCS_trig_statDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "CS", iState);

	// Helicity
	// TEfficiency* hNominalEffHX = TEfficiency3D(NominalTEfficiency3DName("HX", lambdaTheta, lambdaPhi, lambdaThetaPhi), "HX", iState);
	TEfficiency* hNominalEffHX; 

	// TEfficiency* hHX_trk_systUp = TEfficiency3D(Form("hHX_trk_systUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_trk_systDown = TEfficiency3D(Form("hHX_trk_systDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_trk_statUp = TEfficiency3D(Form("hHX_trk_statUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_trk_statDown = TEfficiency3D(Form("hHX_trk_statDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);

	// TEfficiency* hHX_muId_systUp = TEfficiency3D(Form("hHX_muId_systUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_muId_systDown = TEfficiency3D(Form("hHX_muId_systDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_muId_statUp = TEfficiency3D(Form("hHX_muId_statUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_muId_statDown = TEfficiency3D(Form("hHX_muId_statDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);

	// TEfficiency* hHX_trig_systUp = TEfficiency3D(Form("hHX_trig_systUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_trig_systDown = TEfficiency3D(Form("hHX_trig_systDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_trig_statUp = TEfficiency3D(Form("hHX_trig_statUp_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);
	// TEfficiency* hHX_trig_statDown = TEfficiency3D(Form("hHX_trig_statDown_%s", PolaWeightName(lambdaTheta, lambdaPhi, lambdaThetaPhi)), "HX", iState);

	// (cos theta, phi) for the given pt range
	// TEfficiency* hEffCS2D = CosThetaPhiTEfficiency2D("CS", ptMin, ptMax, iState, lambdaTheta, lambdaPhi, lambdaThetaPhi);
// 
	// TEfficiency* hEffHX2D = CosThetaPhiTEfficiency2D("HX", ptMin, ptMax, iState, lambdaTheta, lambdaPhi, lambdaThetaPhi);

	// vs cos theta, to investigate

	TEfficiency* hEffCS1D = CosThetaTEfficiency1D("CS", ptMin, ptMax, iState, false, lambdaTheta, lambdaPhi, lambdaThetaPhi);

	TEfficiency* hEffHX1D = CosThetaTEfficiency1D("HX", ptMin, ptMax, iState, false, lambdaTheta, lambdaPhi, lambdaThetaPhi);
	TEfficiency* hEffLab1D=new TEfficiency("hEffLab1D", "Efficiency;cosTheta", NPtFineBinning, gPtFineBinning);
	// TEfficiency* hEffHX1D;
    // TEfficiency* hEffCS1D;
    // TEfficiency* hEffHX2D;
    // TEfficiency* hEffCS2D;

	// we want to estimate the uncertainties from scale factors at the same time
	// instructions can be found here: https://twiki.cern.ch/twiki/pub/CMS/HIMuonTagProbe/TnpHeaderFile.pdf#page=5

	// for a given type of muon scale factor (i.e., tracking, muId, trigger) we need to compute the efficiency with up and down variations, for stat and syst uncertainties

	// int indexNominal = 0;
	// int indexSystUp = -1, indexSystDown = -2;
	// int indexStatUp = +1, indexStatDown = +2;

	// double dimuWeight_nominal = 0;
	// double dimuWeight_trk_systUp, dimuWeight_trk_systDown, dimuWeight_trk_statUp, dimuWeight_trk_statDown;
	// double dimuWeight_muId_systUp, dimuWeight_muId_systDown, dimuWeight_muId_statUp, dimuWeight_muId_statDown;
	// double dimuWeight_trig_systUp, dimuWeight_trig_systDown, dimuWeight_trig_statUp, dimuWeight_trig_statDown;

	// loop variables
	TLorentzVector* genLorentzVector = new TLorentzVector();
	TLorentzVector* recoLorentzVector = new TLorentzVector();

	// double eventWeight, dimuonPtWeight, totalWeightCS, totalWeightHX;
	// double dimuTrigWeight_nominal = -1, dimuTrigWeight_systUp = -1, dimuTrigWeight_systDown = -1, dimuTrigWeight_statUp = -1, dimuTrigWeight_statDown = -1;

	// Float_t weightCS = 0, weightHX = 0;

	// Bool_t allGood, firesTrigger, isRecoMatched, dimuonMatching, goodVertexProba, passHLTFilterMuons, trackerAndGlobalMuons, hybridSoftMuons;

	Long64_t totGenEntries = GenTree->GetEntries();
	totGenEntries = 100000;
	Long64_t totEntries = RecoTree->GetEntries();
	totEntries = 100000;


	for (Long64_t iEvent = 0; iEvent < totEntries; iEvent++) {
		if (iEvent % 10000 == 0) {
			cout << Form("\rProcessing Reco event %lld / %lld (%.0f%%)", iEvent, totEntries, 100. * iEvent / totEntries) << flush;
		}
	    RecoTree->GetEntry(iEvent);
        if(Dstar_mass < 1 || Dstar_pT==0.0) continue;
		hRecoLab1D->Fill(Dstar_pT);
        TLorentzVector dau1, dau2, dstar;
        dau1.SetPtEtaPhiM(Dstar_Dau1_pT, Dstar_Dau1_eta, Dstar_Dau1_phi, Dstar_Dau1_mass);
        dau2.SetPtEtaPhiM(Dstar_Dau2_pT, Dstar_Dau2_eta, Dstar_Dau2_phi, Dstar_Dau2_mass);
        dstar.SetPtEtaPhiM(Dstar_pT, Dstar_eta, Dstar_phi, Dstar_mass);
        TVector3 DstarDau1_CS = DstarDau1Vector_CollinsSoper(dstar, dau1);
        TVector3 DstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
        // cout << DstarDau1_CS.CosTheta() << " " << DstarDau1_CS.Phi() << " " << DstarDau1_CS.Pt() << endl;
        // hRecoCS3D->Fill(DstarDau1_CS.CosTheta(), DstarDau1_CS.Phi(), DstarDau1_CS.Pt());
        // hRecoCS2D->Fill(DstarDau1_CS.CosTheta(), DstarDau1_CS.Phi());
        // hRecoHX3D->Fill(DstarDau1_HX.CosTheta(), DstarDau1_HX.Phi(), DstarDau1_HX.Pt());
        // hRecoHX2D->Fill(DstarDau1_HX.CosTheta(), DstarDau1_HX.Phi());
        hRecoHX1D->Fill(DstarDau1_HX.CosTheta());
        hRecoCS1D->Fill(DstarDau1_CS.CosTheta());
    }
    for (Long64_t iEvent = 0; iEvent < totGenEntries; iEvent++) {
        if (iEvent % 10000 == 0) {
            cout << Form("\rProcessing Gen event %lld / %lld (%.0f%%)", iEvent, totGenEntries, 100. * iEvent / totGenEntries) << flush;
        }
        GenTree->GetEntry(iEvent);
		hGenLab1D->Fill(Dstar_Gen_pT);
        TLorentzVector dau1, dau2, dstar;
        dau1.SetPtEtaPhiM(Dstar_Gen_Dau1_pT, Dstar_Gen_Dau1_eta, Dstar_Gen_Dau1_phi, Dstar_Gen_Dau1_mass);
        dau2.SetPtEtaPhiM(Dstar_Gen_Dau2_pT, Dstar_Gen_Dau2_eta, Dstar_Gen_Dau2_phi, Dstar_Gen_Dau2_mass);
        dstar.SetPtEtaPhiM(Dstar_Gen_pT, Dstar_Gen_eta, Dstar_Gen_phi, Dstar_Gen_mass);
        TVector3 DstarDau1_CS = DstarDau1Vector_CollinsSoper(dstar, dau1);
        TVector3 DstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
        // hGenCS3D->Fill(DstarDau1_CS.CosTheta(), DstarDau1_CS.Phi(), DstarDau1_CS.Pt());
        // hGenCS2D->Fill(DstarDau1_CS.CosTheta(), DstarDau1_CS.Phi());
        // hGenHX3D->Fill(DstarDau1_HX.CosTheta(), DstarDau1_HX.Phi(), DstarDau1_HX.Pt());
        // hGenHX2D->Fill(DstarDau1_HX.CosTheta(), DstarDau1_HX.Phi());
        hGenHX1D->Fill(DstarDau1_HX.CosTheta());
        hGenCS1D->Fill(DstarDau1_CS.CosTheta());
    }
    hEffCS1D->SetTotalHistogram(*hGenCS1D, "f");
    hEffCS1D->SetPassedHistogram(*hRecoCS1D, "f");
    hEffHX1D->SetTotalHistogram(*hGenHX1D, "f");
    hEffHX1D->SetPassedHistogram(*hRecoHX1D, "f");
    // hEffCS2D->SetTotalHistogram(*hGenCS2D, "f");
    // hEffCS2D->SetPassedHistogram(*hRecoCS2D, "f");
    // hEffHX2D->SetTotalHistogram(*hGenHX2D, "f");
    // hEffHX2D->SetPassedHistogram(*hRecoHX2D, "f");
    // hEffCS3D->SetTotalHistogram(*hGenCS3D, "f");
    // hEffCS3D->SetPassedHistogram(*hRecoCS3D, "f");
    // hEffHX3D->SetTotalHistogram(*hGenHX3D, "f");
    // hEffHX3D->SetPassedHistogram(*hRecoHX3D, "f");
	/*
    TCanvas* canvas = new TCanvas("canvas", "", 1400, 600);
    canvas->Divide(2);
    canvas->cd(1);
    hRecoCS2D->Draw();
    canvas->cd(2);
    hGenCS2D->Draw();
	canvas->SaveAs("EfficiencyMaps/AccCS2D.png");
    TCanvas* canvas1 = new TCanvas("canvas", "", 1400, 600);
    canvas1->Divide(2);
    // canvas->cd(3);
	canvas1->cd(1);
	hRecoHX2D->Draw();
	canvas1->cd(2);
	hGenHX2D->Draw();
	canvas1->SaveAs("EfficiencyMaps/AccHX2D.png");
	*/
    
    // auto h =(TH1D*)hRecoCS1D->Clone();
    // h->Divide(hGenCS1D);
    // hEffCS2D->Draw();

    // canvas->SaveAs("EfficiencyMaps/AccCS1D.png");
    // hEffCS1D = new TEfficiency(*hRecoCS1D, *hGenCS1D);
    // hEffCS2D = new TEfficiency(*hRecoCS2D, *hGenCS2D);
    // hEffHX1D = new TEfficiency(*hRecoHX1D, *hGenHX1D);
    // hEffHX2D = new TEfficiency(*hRecoHX2D, *hGenHX2D);
	// TEfficiency* eff3D = new TEfficiency("eff3D", "Efficiency;cosTheta;phi;pT", 50, -1, 1, 50, -3.14, 3.14, 50, ptMin, ptMax);
    // eff3D->SetTotalHistogram(*hGen3D, "f");
    // eff3D->SetPassedHistogram(*hReco3D, "f");

    // TEfficiency* eff2D = new TEfficiency("eff2D", "Efficiency;cosTheta;phi", 50, -1, 1, 50, -3.14, 3.14);
    // eff2D->SetTotalHistogram(*hGen2D, "f");
    // eff2D->SetPassedHistogram(*hReco2D, "f");

    // TEfficiency* EffCS1D = new TEfficiency("eff1D", "Efficiency;cosTheta", 50, -1, 1);
	// hEffLab1D->SetTotalHistogram(*hGenLab1D, "f");
    // hEffLab1D->SetPassedHistogram(*hRecoLab1D, "f");
    
    
	// EffCS1D
    // TEfficiency* EffHX1D = new TEfficiency("eff1D", "Efficiency;cosTheta", 50, -1, 1);
    

    // Draw TEfficiency objects
    // TCanvas* c1 = new TCanvas("c1", "3D Efficiency", 800, 600);
    // eff3D->Draw("COLZ");

    // TCanvas* c2 = new TCanvas("c2", "2D Efficiency", 800, 600);
    // eff2D->Draw("COLZ");

    // TCanvas* c3 = new TCanvas("c3", "1D Efficiency", 800, 600);
    // eff1D->Draw();






	gStyle->SetPadLeftMargin(.15);
	gStyle->SetPadRightMargin(0.18);
	SetColorPalette(gEfficiencyColorPaletteName);

	/// display the nominal results

	// DrawEfficiencyMap(hEffCS2D, ptMin, ptMax, iState);
	// DrawEfficiency1DHist(hEffCS1D, "CosTheta", false, lambdaTheta, lambdaPhi, lambdaThetaPhi);
	const std::vector<Double_t> gCosThetaFineBinning = {-1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};
	int NCosThetaFineBins = gCosThetaFineBinning.size() - 1;

	std::vector<Double_t> gPhiFineBinning = {-180, -160, -140, -120, -100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100, 120, 140, 160, 180};
	int NPhiFineBins = gPhiFineBinning.size()-1; // i.e. 20 degrees wide bins
	for(auto &i : gPhiFineBinning){
		i = i * TMath::Pi()/180;
	}
	// DrawEfficiency2DHist(hEffCS2D, 0, 50, NCosThetaBins, gCosThetaFineBinning, NPhiFineBins,gPhiFineBinning,iState);
	// DrawEfficiency2DHist(hEffHX2D, 0, 50, NCosThetaBins, gCosThetaFineBinning, NPhiFineBins,gPhiFineBinning,iState);

	// DrawEfficiencyMap(hEffHX2D, ptMin, ptMax, iState);
	// DrawEfficiency1DHist(hEffHX1D,"CosTheta", false, lambdaTheta, lambdaPhi, lambdaThetaPhi);
	// DrawEfficiency1DHist(hEffLab1D,"pT", false, lambdaTheta, lambdaPhi, lambdaThetaPhi);

	/// compute the systematics in this macro since we have all the ingredients for that
	// instructions can be found here: https://twiki.cern.ch/twiki/pub/CMS/HIMuonTagProbe/TnpHeaderFile.pdf#page=5

	// store the RELATIVE SYSTEMATIC UNCERTAINTIES (not in % though) with respect to the nominal efficiency, more useful at the end

	// gStyle->SetPadRightMargin(0.19);
	// gStyle->SetTitleOffset(1.3, "Z");

	// const char* legendText = EfficiencyLegendText(ptMin, ptMax);

	// TLatex* legend = new TLatex(.5, .88, legendText);
	// legend->SetTextAlign(22);
	// legend->SetTextSize(0.042);

	// // Collins-Soper
	// // auto* hSystCS3D = RelSystEffHist3D(hNominalEffCS, hCS_trk_systUp, hCS_trk_systDown, hCS_muId_systUp, hCS_muId_systDown, hCS_trig_systUp, hCS_trig_systDown, hCS_trk_statUp, hCS_trk_statDown, hCS_muId_statUp, hCS_muId_statDown, hCS_trig_statUp, hCS_trig_statDown, "CS", lambdaTheta = 0, lambdaPhi = 0, lambdaThetaPhi = 0);

	// auto* canvasCSsyst = new TCanvas("canvasCSsyst", "", 700, 600);
	// hSystCS3D->Draw("COLZ");

	// CMS_lumi(canvasCSsyst, Form("#varUpsilon(%dS) Hydjet-embedded MC", iState));

	// legend->DrawLatexNDC(.5, .88, legendText);

	// gPad->Update();

	// //hSystCS3D->GetYaxis()->SetRangeUser(-190, 240);

	// //canvasCSsyst->SaveAs(Form("EfficiencyMaps/%dS/RelatSystEff_CS.png", gUpsilonState), "RECREATE");

	// // HelicityTEfficiency
	// auto* hSystHX2D = RelSystEffHist(hNominalEffHX, hHX_trk_systUp, hHX_trk_systDown, hHX_muId_systUp, hHX_muId_systDown, hHX_trig_systUp, hHX_trig_systDown, hHX_trk_statUp, hHX_trk_statDown, hHX_muId_statUp, hHX_muId_statDown, hHX_trig_statUp, hHX_trig_statDown);

	// auto* canvasHXsyst2D = new TCanvas("canvasHXsyst2D", "", 700, 600);
	// hSystHX2D->Draw();

	// auto* hSystHX3D = RelSystEffHist3D(hNominalEffHX, hHX_trk_systUp, hHX_trk_systDown, hHX_muId_systUp, hHX_muId_systDown, hHX_trig_systUp, hHX_trig_systDown, hHX_trk_statUp, hHX_trk_statDown, hHX_muId_statUp, hHX_muId_statDown, hHX_trig_statUp, hHX_trig_statDown, "HX");

	// auto* canvasHXsyst = new TCanvas("canvasHXsyst", "", 700, 600);
	// hSystHX3D->Draw("COLZ");

	// CMS_lumi(canvasHXsyst, Form("#varUpsilon(%dS) Hydjet-embedded MC", iState));

	// legend->DrawLatexNDC(.5, .88, legendText);

	// gPad->Update();

	// //hSystHX3D->GetYaxis()->SetRangeUser(-190, 240);

	// //canvasHXsyst->SaveAs(Form("EfficiencyMaps/%dS/RelatSystEff_HX.png", gUpsilonState), "RECREATE");

	// /// save the nominal efficiency results and the corresponding systematics in a file for later usage
	gSystem->mkdir(Form("EfficiencyMaps/%dS", iState), kTRUE);
	const char* outputFileName = Form("EfficiencyMaps/%dS/EfficiencyResults%s.root", iState, gMuonAccName);
	TFile outputFile(outputFileName, "UPDATE");

	// hNominalEffCS->Write();
	// hNominalEffHX->Write();
	// hSystCS3D->Write();
	// hSystHX3D->Write();

	// hSystHX2D->Write();

	// hEffCS2D->Write();
	// hEffHX2D->Write();

	hEffCS1D->Write();
	hEffHX1D->Write();

	outputFile.Close();

	file->Close();

	if (BeVerbose) cout << "\nNominal efficiency and corresponding systematic uncertainty maps saved in " << outputFileName << endl;
}