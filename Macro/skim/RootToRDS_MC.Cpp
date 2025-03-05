#include "RooFit.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "RooWorkspace.h"
#include <iostream>
#include <cmath>
#include <ctime>
// #include "AnalysisParameters.h"
#include "Transformations.h"

void RootToRDS_MC() {
    TFile *fin = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Data/flatSkimForBDT_isMC1_OnlyGEN_0_250108.root"); // MC
    TFile *fout = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Data/DstarMC_Rds_pp.root", "RECREATE"); // MC
    TTree *t = (TTree*)fin->Get("skimGENTreeFlat");
    const char* gphiUnit = "#circ";

    fout->cd();
    RooRealVar mass("mass", "Invariant mass", 1.74, 2.1);
    RooRealVar pt("pT", "Transverse Momentum", 0, 100);
    RooRealVar eta("eta", "", -2.4, 2.4);
    RooRealVar y("y", "", -2.4, 2.4);
    RooRealVar phi("phi", "", -TMath::Pi(), TMath::Pi());
    RooRealVar massDaughter1("massDaughter1", "massDaughter1", 1.74, 2.00);
    // RooRealVar mva("mva", "", -1, 1);
    RooFormulaVar massPion("massPion", "Parent mass - Daughter mass", "mass - massDaughter1", RooArgList(mass, massDaughter1));
    RooRealVar cosThetaLabVar("cosThetaLabVar", "CosThetaLabVar", -1, 1);
	RooRealVar phiLabVar("phiLabVar", "phiLabVar", -180, 180, gphiUnit);
	RooRealVar etaLabDau1Var("etaLabD0", "eta of #D^{0} in the lab frame", -2.4, 2.4);
	RooRealVar etaLabDau2Var("etaLabSlowPion", "eta of #pi_{slow] in the lab frame", -2.4, 2.4);

	RooDataSet datasetLab("datasetLab", "skimmed dataset for the Lab frame", RooArgSet(mass, y, pt, cosThetaLabVar, phiLabVar, etaLabDau1Var));
    char* refFrameName = nullptr;

	refFrameName = (char*)"CS";
	RooRealVar cosThetaCSVar(Form("cosTheta%sVar",refFrameName), Form("CosTheta%s",refFrameName), -1, 1);
	RooRealVar phiCSVar(Form("phi%sVar",refFrameName), Form("phi%s",refFrameName), -180, 180, gphiUnit);
	RooRealVar phiTildeCSVar(Form("phiTilde%sVar",refFrameName), Form("phi%s",refFrameName), -180, 180, gphiUnit);

	RooDataSet datasetCS("datasetCS", "skimmed dataset for the CS frame", RooArgSet(mass, y, pt, cosThetaCSVar, phiCSVar, phiTildeCSVar));

	refFrameName = (char*)"HX";
	RooRealVar cosThetaHXVar(Form("cosTheta%sVar",refFrameName), Form("CosTheta%s",refFrameName), -1, 1);
	RooRealVar phiHXVar(Form("phi%sVar",refFrameName), Form("phi%s",refFrameName), -180, 180, gphiUnit);
	RooRealVar phiTildeHXVar(Form("phiTilde%sVar",refFrameName), Form("phiTilde%s",refFrameName), -180, 180, gphiUnit);
    RooDataSet datasetHX("datasetHX", "skimmed dataset for the HX frame", RooArgSet(mass, y, pt, cosThetaHXVar, phiHXVar, phiTildeHXVar));
    // RooArgSet variables(mass, pt, eta, phi, massDaughter1, mva);

    // RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    int nEntries = t->GetEntries();
    cout << "Total entries: " << nEntries << endl;
    // RooDataSet data("dataset", "dataset from tree", variables);

    Float_t Dstar_mass, Dstar_y, Dstar_pT, Dstar_Eta, Dstar_Phi;
    Float_t Dstar_Dau1_pT, Dstar_Dau1_y, Dstar_Dau1_Eta, Dstar_Dau1_Phi, Dstar_Dau1_mass;
    Float_t Dstar_Dau2_pT, Dstar_Dau2_y, Dstar_Dau2_Eta, Dstar_Dau2_Phi, Dstar_Dau2_mass;

    // Set branch addresses
    t->SetBranchAddress("gen_mass", &Dstar_mass);
    t->SetBranchAddress("gen_y", &Dstar_y);
    t->SetBranchAddress("gen_pT", &Dstar_pT);
    t->SetBranchAddress("gen_eta", &Dstar_Eta);
    t->SetBranchAddress("gen_phi", &Dstar_Phi);
    t->SetBranchAddress("gen_D0pT", &Dstar_Dau1_pT);
    t->SetBranchAddress("gen_D0eta", &Dstar_Dau1_Eta);
    t->SetBranchAddress("gen_D0phi", &Dstar_Dau1_Phi);
    t->SetBranchAddress("gen_D0mass", &Dstar_Dau1_mass);
    t->SetBranchAddress("gen_D1pT", &Dstar_Dau2_pT);
    t->SetBranchAddress("gen_D1eta", &Dstar_Dau2_Eta);
    t->SetBranchAddress("gen_D1phi", &Dstar_Dau2_Phi);
    t->SetBranchAddress("gen_D1mass", &Dstar_Dau2_mass);
    const int chunkSize = 100000; // Adjust chunk size as needed

    for (int i = 0; i < nEntries; i += chunkSize) {
        int end = std::min(i + chunkSize, nEntries);
        // cout << i << " " << end << endl;
        for (int j = i; j < end; ++j) {

            if(j%1000==0) cout << "Processing event: " << j << endl;
            t->GetEntry(j);
            if(Dstar_mass<1.74 || Dstar_mass>2.1) continue;

            // Perform calculations
            TLorentzVector dau1, dau2, dstar;
            dau1.SetPtEtaPhiM(Dstar_Dau1_pT, Dstar_Dau1_Eta, Dstar_Dau1_Phi, Dstar_Dau1_mass);
            dau2.SetPtEtaPhiM(Dstar_Dau2_pT, Dstar_Dau2_Eta, Dstar_Dau2_Phi, Dstar_Dau2_mass);
            dstar.SetPtEtaPhiM(Dstar_pT, Dstar_Eta, Dstar_Phi, Dstar_mass);

            // Set RooRealVar values
            // if(dstar.M()!=Dstar_mass){
                // cout << "Dstar mass: " << dstar.M() << endl;
            // }
            // mass.setVal(dstar.M());
            // pt.setVal(dstar.Pt());
            // eta.setVal(dstar.Eta());
            // phi.setVal(dstar.Phi());
            // cout <<Dstar_mass << " " << Dstar_pT << " " << Dstar_Eta << " " << Dstar_Phi << " " << Dstar_Dau1_mass << " " << Dstar_Dau1_pT << " " << Dstar_Dau1_Eta << " " << Dstar_Dau1_Phi << " " << Dstar_Dau2_mass << " " << Dstar_Dau2_pT << " " << Dstar_Dau2_Eta << " " << Dstar_Dau2_Phi << endl; 
            mass = dstar.M();
            pt= dstar.Pt();
            eta= dstar.Eta();
            phi= dstar.Phi();
            // massDaughter1.setVal(Dstar_Dau1_mass);
            TVector3 DstarDau1_CS = DstarDau1Vector_CollinsSoper(dstar, dau1);

			TVector3 DstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
            // mva.setVal(0); // Set MVA value if available
            cosThetaLabVar = dau1.CosTheta();
			phiLabVar = dau1.Phi() * 180 / TMath::Pi();
			etaLabDau1Var = dau1.PseudoRapidity();
			// etaLabDau2Var = dau2.PseudoRapidity();
            datasetLab.add(RooArgSet(mass, y, pt, cosThetaLabVar, phiLabVar, etaLabDau1Var));
            cosThetaCSVar =DstarDau1_CS.CosTheta();
			phiCSVar = DstarDau1_CS.Phi() * 180 / TMath::Pi();
            if (cosThetaCSVar.getVal() < 0) {
				// if phi value is smaller than -pi, add 2pi
				if ((phiCSVar.getVal() - 135) < -180)
					phiTildeCSVar.setVal(phiCSVar.getVal() + 225);
				else
					phiTildeCSVar.setVal(phiCSVar.getVal() - 135);
			}

			else if (cosThetaCSVar.getVal() > 0) {
				// if phi value is smaller than -pi, add 2pi
				if ((phiCSVar.getVal() - 45) < -180)
					phiTildeCSVar.setVal(phiCSVar.getVal() + 315);
				else
					phiTildeCSVar.setVal(phiCSVar.getVal() - 45);
			}
            // cout << "cosThetaCSVar: " << cosThetaCSVar.getVal() << " phiCSVar: " << phiCSVar.getVal() << " phiTildeCSVar: " << phiTildeCSVar.getVal() << endl;

			datasetCS.add(RooArgSet(mass, y, pt, cosThetaCSVar, phiCSVar, phiTildeCSVar));
            cosThetaHXVar = DstarDau1_HX.CosTheta();
			phiHXVar = DstarDau1_HX.Phi() * 180 / TMath::Pi();

			if (cosThetaHXVar.getVal() < 0) {
				// if phi value is smaller than -pi, add 2pi
				if ((phiHXVar.getVal() - 135) < -180)
					phiTildeHXVar.setVal(phiHXVar.getVal() + 225);
				else
					phiTildeHXVar.setVal(phiHXVar.getVal() - 135);
			}

			else if (cosThetaHXVar.getVal() > 0) {
				// if phi value is smaller than -pi, add 2pi
				if ((phiHXVar.getVal() - 45) < -180)
					phiTildeHXVar.setVal(phiHXVar.getVal() + 315);
				else
					phiTildeHXVar.setVal(phiHXVar.getVal() - 45);
			}

			datasetHX.add(RooArgSet(mass, y, pt, cosThetaHXVar, phiHXVar, phiTildeHXVar));

			// dataset.add(RooArgSet(massVa, yVar, ptVar, cosThetaCSVar, phiCSVar, cosThetaHXVar, phiHXVar));

            // Add event to dataset
            // data.add(variables);
        }
    }
    // data.addColumn(massPion);

    RooWorkspace *ws = new RooWorkspace("workspace");
    ws->import(mass);
    ws->import(pt);
    ws->import(eta);
    ws->import(phi);
    // ws->import(mva);
    // ws->import(massPion);
    // ws->import(data);
    ws->import(datasetCS);
    ws->import(datasetLab);
    ws->import(datasetHX);
    ws->Write();
    fout->Close();
}