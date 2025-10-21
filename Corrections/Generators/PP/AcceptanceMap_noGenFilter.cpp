#include "../Tools/BasicHeaders.h"

#include "../Tools/Parameters/AnalysisParameters.h"

#include "../Tools/Parameters/PhaseSpace.h"

#include "AccEffHelpers_copy.h"

//#include "../Tools/Style/FitDistributions.h"
#include "../Tools/Style/Legends.h"

#include "../Tools/Transformations.h"

void DrawAcceptanceMap(TEfficiency* accMap, Int_t ptMin, Int_t ptMax, Float_t rho_00=0) {
	TCanvas* canvas = new TCanvas(accMap->GetName(), "", 700, 600);
	accMap->Draw("COLZ");

	CMS_lumi(canvas, Form("Pythia 8 (5.02 TeV)"));

	TLatex legend;
	legend.SetTextAlign(22);
	legend.SetTextSize(0.05);
	legend.DrawLatexNDC(.48, .88, Form("%s < 2.4, %s", gDimuonRapidityVarTitle, DimuonPtRangeText(ptMin, ptMax)));
	legend.DrawLatexNDC(.48, .8, Form("D^{*} acc. for |#eta^{#mu}| < 2.4, %s", gDstarPtCutText));
	legend.DrawLatexNDC(.48, .72, Form("#rho_{00}#th = %.2f", rho_00));

	gPad->Update();

	accMap->GetPaintedHistogram()->GetXaxis()->CenterTitle();
	accMap->GetPaintedHistogram()->GetYaxis()->CenterTitle();

	accMap->GetPaintedHistogram()->GetYaxis()->SetRangeUser(-180, 180);
	accMap->GetPaintedHistogram()->GetZaxis()->SetRangeUser(0, 1);

	gSystem->mkdir(Form("AcceptanceMaps/Dstar"), kTRUE);
	canvas->SaveAs(Form("AcceptanceMaps/Dstar/%s.png", accMap->GetName()), "RECREATE");
}

void DrawAcceptance1DHist(TEfficiency* accHist, Int_t ptMin, Int_t ptMax, Float_t rho_00=0) {
	TCanvas* canvas = new TCanvas(accHist->GetName(), "", 600, 600);
	canvas->SetRightMargin(0.05);

	// empty frame for the axes
	TH1D* frameHist = new TH1D("frameHist", "", NCosThetaBinsHX, CosThetaBinningHX);

	frameHist->Draw();

	accHist->SetLineWidth(3);
	accHist->Draw("PL E0 SAME");

	CMS_lumi(canvas, Form("Pythia 8 (5.36 TeV)"));

	TLatex legend;
	legend.SetTextAlign(22);
	legend.SetTextSize(0.05);
	legend.DrawLatexNDC(.55, .88, Form("%s < 2.4, %s", gDimuonRapidityVarTitle, DimuonPtRangeText(ptMin, ptMax)));
	legend.DrawLatexNDC(.55, .8, Form("D^{*} acc. for |#eta^{#mu}| < 2.4, %s", gDstarPtCutText));
	legend.DrawLatexNDC(.48, .72, Form("#rho_{00}#th = %.2f", rho_00));

	if (strstr(accHist->GetName(), "CS"))
		frameHist->SetXTitle(CosThetaVarTitle("CS"));
	else
		frameHist->SetXTitle(CosThetaVarTitle("HX"));

	frameHist->SetYTitle(TEfficiencyAccMainTitle());

	frameHist->GetXaxis()->CenterTitle();
	frameHist->GetYaxis()->CenterTitle();

	frameHist->GetXaxis()->SetRangeUser(-1, 1);
	frameHist->GetYaxis()->SetRangeUser(0, 1);

	frameHist->GetXaxis()->SetNdivisions(510, kTRUE);

	gSystem->mkdir(Form("AcceptanceMaps/Dstar"), kTRUE);
	canvas->SaveAs(Form("AcceptanceMaps/Dstar/%s.png", accHist->GetName()), "RECREATE");
}

const char* Acceptance2DAxisTitle(const char* refFrameName = "CS") {
	return Form(";%s;%s;acceptance", CosThetaVarTitle(refFrameName), PhiAxisTitle(refFrameName));
}

// (cos theta, phi) acceptance maps based on Y events generated without any decay kinematic cut
// MC files available here: /eos/cms/store/group/phys_heavyions/dileptons/MC2015/pp502TeV/TTrees/ (This file was deleted:/)

void AcceptanceMap_noGenFilter(Int_t ptMin = 0, Int_t ptMax = 50, Float_t rho_00 = 0) {
	// // Read GenOnly Nofilter file
	// const char* filename = Form("../Files/OniaTree_Y%dS_GENONLY_NoFilter.root", iState, lambdaTheta, lambdaPhi, lambdaThetaPhi);

	// Read GenOnly Nofilter file with polarization weights
	const char* filename = "/home/jun502s/DstarAna/DStarAnalysis/Data/flatSkimForBDT_isMC1_noFilter_wOnlyGEN_0_250108.root";

	TFile* file = TFile::Open(filename, "READ");
	if (!file) {
		cout << "File " << filename << " not found. Check the directory of the file." << endl;
		return;
	}

	cout << "File " << filename << " opened" << endl;

	// Put the text, "CMS Internal", on the right top of the plot
	writeExtraText = true; // if extra text
	extraText = "       Internal";
	TTree* GenTree = (TTree*)file->Get("skimGENTreeFlat");
    Float_t Dstar_mass, Dstar_y, Dstar_pT, Dstar_eta, Dstar_phi;
    Float_t Dstar_Dau1_pT, Dstar_Dau1_y, Dstar_Dau1_eta, Dstar_Dau1_phi, Dstar_Dau1_mass;
    Float_t Dstar_Dau2_pT, Dstar_Dau2_y, Dstar_Dau2_eta, Dstar_Dau2_phi, Dstar_Dau2_mass;

    // Set branch addresses
    GenTree->SetBranchAddress("gen_mass", &Dstar_mass);
    GenTree->SetBranchAddress("gen_y", &Dstar_y);
    GenTree->SetBranchAddress("gen_pT", &Dstar_pT);
    GenTree->SetBranchAddress("gen_eta", &Dstar_eta);
    GenTree->SetBranchAddress("gen_phi", &Dstar_phi);
    GenTree->SetBranchAddress("gen_D0pT", &Dstar_Dau1_pT);
    GenTree->SetBranchAddress("gen_D0eta", &Dstar_Dau1_eta);
    GenTree->SetBranchAddress("gen_D0phi", &Dstar_Dau1_phi);
    GenTree->SetBranchAddress("gen_D0mass", &Dstar_Dau1_mass);
    GenTree->SetBranchAddress("gen_D1pT", &Dstar_Dau2_pT);
    GenTree->SetBranchAddress("gen_D1eta", &Dstar_Dau2_eta);
    GenTree->SetBranchAddress("gen_D1phi", &Dstar_Dau2_phi);
    GenTree->SetBranchAddress("gen_D1mass", &Dstar_Dau2_mass);


	/// OniaTree variables, quite old version (since this genonly file from 2015)
	// Int_t Gen_QQ_size;


	// TClonesArray* Gen_QQ_4mom = nullptr;
	// TClonesArray* Gen_QQ_mumi_4mom = nullptr;
	// TClonesArray* Gen_QQ_mupl_4mom = nullptr;

	// OniaTree->SetBranchAddress("Gen_QQ_size", &Gen_QQ_size);
	// OniaTree->SetBranchAddress("Gen_QQ_4mom", &Gen_QQ_4mom);

	// OniaTree->SetBranchAddress("Gen_Dstar_mumi_4mom", &Gen_Dstar_mumi_4mom);
	// OniaTree->SetBranchAddress("Gen_Dstar_mupl_4mom", &Gen_Dstar_mupl_4mom);

	// (cos theta, phi, pT) 3D maps for final acceptance correction, variable size binning for the stats
	TEfficiency* accMatrixCS = TEfficiency3D(NominalTEfficiency3DName("CS", rho_00), "CS");

	TEfficiency* accMatrixHX = TEfficiency3D(NominalTEfficiency3DName("HX", rho_00), "HX");

	// (cos theta, phi) 2D distribution maps for Lab, CS and HX frames

	TEfficiency* hGranularLab = CosThetaPhiAcceptance2D("Lab", ptMin, ptMax, rho_00);
	TEfficiency* hGranularCS = CosThetaPhiAcceptance2D("CS", ptMin, ptMax, rho_00);
	TEfficiency* hGranularHX = CosThetaPhiAcceptance2D("HX", ptMin, ptMax, rho_00);

	// actual analysis binning (defined in AnalysisParameters.h)
	TEfficiency* hAnalysisLab = new TEfficiency(Form("Analysis%s", TEfficiencyEndName("Lab", ptMin, ptMax, rho_00)), Acceptance2DAxisTitle("Lab"), NCosThetaBinsLab, CosThetaBinningLab, NPhiBinsLab, PhiBinningLab);
	TEfficiency* hAnalysisCS = new TEfficiency(Form("Analysis%s", TEfficiencyEndName("CS",   ptMin, ptMax, rho_00)), Acceptance2DAxisTitle("CS"),  NCosThetaBinsCS, CosThetaBinningCS, NPhiBinsCS, PhiBinningCS);
	TEfficiency* hAnalysisHX = new TEfficiency(Form("Analysis%s", TEfficiencyEndName("HX",   ptMin, ptMax, rho_00)), Acceptance2DAxisTitle("HX"),  NCosThetaBinsHX, CosThetaBinningHX, NPhiBinsHX, PhiBinningHX);

	// vs cos theta, for investigation

	TEfficiency* hAccCS1D = CosThetaTEfficiency1D("CS", ptMin, ptMax, kTRUE, rho_00);

	TEfficiency* hAccHX1D = CosThetaTEfficiency1D("HX", ptMin, ptMax, kTRUE, rho_00);

	TLorentzVector* gen_Dstar_LV = new TLorentzVector();
	TLorentzVector* gen_Dau1_LV = new TLorentzVector();
	TLorentzVector* gen_Dau2_LV = new TLorentzVector();

	Bool_t withinAcceptance=0;

	Double_t cosThetaCS, phiCS, cosThetaHX, phiHX;

	Float_t weightCS = 0, weightHX = 0;

	Long64_t totEntries = GenTree->GetEntries();
	//  Long64_t totEntries = 100000;

	// Loop over the events
	for (Long64_t iEvent = 0; iEvent < (totEntries); iEvent++) {
		if (iEvent % 10000 == 0) {
			cout << Form("\rProcessing event %lld / %lld (%.0f%%)", iEvent, totEntries, 100. * iEvent / totEntries) << flush;
		}

		GenTree->GetEntry(iEvent);

		// loop over all gen upsilons
		// for (int iGen = 0; iGen < Gen_Dstar_size; iGen++) {
		gen_Dstar_LV->SetPtEtaPhiM(Dstar_pT, Dstar_eta, Dstar_phi, Dstar_mass);

		// if (fabs(gen_Dstar_LV->Rapidity()) < gRapidityMin || fabs(gen_Dstar_LV->Rapidity()) > gRapidityMax) continue; // upsilon within fiducial region

		// single-muon acceptance cuts
		gen_Dau1_LV->SetPtEtaPhiM(Dstar_Dau1_pT, Dstar_Dau1_eta, Dstar_Dau1_phi, Dstar_Dau1_mass);
		gen_Dau2_LV->SetPtEtaPhiM(Dstar_Dau2_pT, Dstar_Dau2_eta, Dstar_Dau2_phi, Dstar_Dau2_mass);

		withinAcceptance = DstarSimpleAcc(*gen_Dau1_LV) && DstarSimpleAcc(*gen_Dau2_LV);
		// cout << "withinAcceptance: " << withinAcceptance << endl;
			// withinAcceptance = MuonUpsilonTriggerAcc(*gen_mupl_LV) && MuonUpsilonTriggerAcc(*gen_mumi_LV);

			// Reference frame transformations
		TVector3 Dau1_CS = DstarDau1Vector_CollinsSoper(*gen_Dstar_LV, *gen_Dau1_LV);

			cosThetaCS = Dau1_CS.CosTheta();
			// phiCS = fabs(muPlus_CS.phi() * 180 / TMath::Pi());
			phiCS = Dau1_CS.Phi() * 180 / TMat22h::Pi();

			// weightCS = 1 + lambdaTheta * TMath::Power(Dau1_CS.CosTheta(), 2) + lambdaPhi * TMath::Power(std::sin(Dau1_CS.Theta()), 2) * std::cos(2 * Dau1_CS.Phi()) + lambdaThetaPhi * std::sin(2 * Dau1_CS.Theta()) * std::cos(Dau1_CS.Phi());
			weightCS = 1;
			// weightCS = 1 + lambdaTheta * TMath::Power(Dau1us_CS.CosTheta(), 2) + lambdaphi * TMath::Power(std::sin(Dau1us_CS.Theta()), 2) * std::cos(2 * fabs(Dau1us_CS.Phi())) + lambdaThetaPhi * std::sin(2 * Dau1us_CS.Theta()) * std::cos(fabs(Dau1us_CS.Phi()));

			accMatrixCS->FillWeighted(withinAcceptance, weightCS, cosThetaCS, phiCS, gen_Dstar_LV->Pt());

			TVector3 Dau1_HX = DstarDau1Vector_Helicity(*gen_Dstar_LV, *gen_Dau1_LV);

			cosThetaHX = Dau1_HX.CosTheta();
			// phiHX = fabs(Dau1us_HX.phi() * 180 / TMath::Pi());
			phiHX = Dau1_HX.Phi() * 180 / TMath::Pi();

			// weightHX = 1 + lambdaTheta * TMath::Power(Dau1_HX.CosTheta(), 2) + lambdaphi * TMath::Power(std::sin(Dau1_HX.Theta()), 2) * std::cos(2 * Dau1_HX.Phi()) + lambdaThetaPhi * std::sin(2 * Dau1_HX.Theta()) * std::cos(Dau1_HX.Phi());
			weightHX=1;
			// cout << "weightHX: " << weightHX << endl;
			// weightHX = 1 + lambdaTheta * TMath::Power(Dau1us_HX.CosTheta(), 2) + lambdaphi * TMath::Power(std::sin(Dau1us_HX.Theta()), 2) * std::cos(2 * fabs(Dau1us_HX.Phi())) + lambdaThetaPhi * std::sin(2 * Dau1us_HX.Theta()) * std::cos(fabs(Dau1us_HX.Phi()));

			accMatrixHX->FillWeighted(withinAcceptance, weightHX, cosThetaHX, phiHX, gen_Dstar_LV->Pt());

			if (gen_Dstar_LV->Pt() > ptMin && gen_Dstar_LV->Pt() < ptMax) { // pt bin of interest for the other distributions

				hGranularCS->FillWeighted(withinAcceptance, weightCS, cosThetaCS, phiCS);
				hAnalysisCS->FillWeighted(withinAcceptance, weightHX, cosThetaCS, phiCS);

				hGranularHX->FillWeighted(withinAcceptance, weightCS, cosThetaHX, phiHX);
				hAnalysisHX->FillWeighted(withinAcceptance, weightHX, cosThetaHX, phiHX);

				hGranularLab->Fill(withinAcceptance, gen_Dau1_LV->CosTheta(), gen_Dau1_LV->Phi() * 180 / TMath::Pi());
				hAnalysisLab->Fill(withinAcceptance, gen_Dau1_LV->CosTheta(), gen_Dau1_LV->Phi() * 180 / TMath::Pi());

				hAccCS1D->FillWeighted(withinAcceptance, weightCS, cosThetaCS);
				hAccHX1D->FillWeighted(withinAcceptance, weightHX, cosThetaHX);

				// cout << "withinAcceptance: " << withinAcceptance << endl;
				// cout << "weightHX: " << weightHX << endl;
				// cout << "cosThetaHX: " << cosThetaHX << endl;

				// TH1* passedHist = (TH1*)hAccHX1D->GetPassedHistogram();
				// cout << passedHist->FindBin(cosThetaHX) << endl;
				// cout << passedHist->GetBinContent(passedHist->FindBin(cosThetaHX)) << endl;
			}
		// }
	}

	// Set the plot styles
	gStyle->SetPadLeftMargin(.15);
	//gStyle->SetTitleYOffset(.9);
	gStyle->SetPadRightMargin(0.18);
	SetColorPalette(gAcceptanceColorPaletteName);

	// Draw and save the acceptance map for Lab frame
	DrawAcceptanceMap(hGranularLab, ptMin, ptMax, rho_00);
	DrawAcceptanceMap(hAnalysisLab, ptMin, ptMax, rho_00);

	// Draw and save the acceptance map for CS frame
	DrawAcceptanceMap(hGranularCS, ptMin, ptMax, rho_00);
	DrawAcceptanceMap(hAnalysisCS, ptMin, ptMax, rho_00);
	DrawAcceptance1DHist(hAccCS1D, ptMin, ptMax, rho_00);

	// Draw and save the acceptance map for HX frrho_00
	DrawAcceptanceMap(hGranularHX, ptMin, ptMax, rho_00);
	DrawAcceptanceMap(hAnalysisHX, ptMin, ptMax, rho_00);
	DrawAcceptance1DHist(hAccHX1D, ptMin, ptMax, rho_00);

	/// save the results in a file for later usage
	gSystem->mkdir(Form("AcceptanceMaps/Dstar"), kTRUE);
	const char* outputFileName = Form("AcceptanceMaps/Dstar/AcceptanceResults%s.root", gDstarAccName);
	TFile outputFile(outputFileName, "UPDATE");

	accMatrixCS->Write();
	accMatrixHX->Write();

	hGranularLab->Write();
	hAnalysisLab->Write();
	hGranularCS->Write();
	hAnalysisCS->Write();
	hGranularHX->Write();
	hAnalysisHX->Write();

	hAccCS1D->Write();
	hAccHX1D->Write();

	outputFile.Close();

	if (BeVerbose) cout << "\nAcceptance maps saved in " << outputFileName << endl;
}