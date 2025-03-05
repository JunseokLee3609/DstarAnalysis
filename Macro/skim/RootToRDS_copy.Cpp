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

void RootToRDS_copy() {
    TFile *fin = TFile::Open("/home/jun502s/DstarAna/DStarAna/data/output_pp.root"); // Data
    TFile *fout = TFile::Open("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_pp.root", "RECREATE"); // Data
    TTree *t = (TTree*)fin->Get("skimTreeFlat");
    const char* gphiUnit = "#circ";

    fout->cd();
    RooRealVar mass("mass", "Invariant mass", 1.74, 2.1);
    RooRealVar pt("pT", "Transverse Momentum", 0, 100);
    RooRealVar eta("eta", "", -2.4, 2.4);
    RooRealVar y("y", "", -2.4, 2.4);
    RooRealVar phi("phi", "", -3.14, 3.14);
    RooRealVar massDaughter1("massDaughter1", "massDaughter1", 1.74, 2.00);
    RooRealVar RapD1("RapD1", "RapD1", -2.4, 2.4);
    RooRealVar EtaD1("EtaD1", "EtaD1", -2.4, 2.4);
    RooRealVar pTD1("pTD1", "pTD1", 0, 100);
    RooRealVar RapD2("RapD2", "RapD2", -2.4, 2.4);
    RooRealVar EtaD2("EtaD2", "EtaD2", -2.4, 2.4);
    RooRealVar pTD2("pTD2", "pTD2", 0, 100);
   
    RooRealVar mva("mva", "", -1, 1);
    RooFormulaVar massPion("massPion", "Parent mass - Daughter mass", "mass - massDaughter1", RooArgList(mass, massDaughter1));
    RooRealVar cosThetaLabVar("cosThetaLabVar", "CosThetaLabVar", -1, 1);
	RooRealVar phiLabVar("phiLabVar", "phiLabVar", -180, 180, gphiUnit);
	RooRealVar etaLabDau1Var("etaLabD0", "eta of #D^{0} in the lab frame", -2.4, 2.4);
	RooRealVar etaLabDau2Var("etaLabSlowPion", "eta of #pi_{slow] in the lab frame", -2.4, 2.4);

	RooDataSet datasetLab("datasetLab", "skimmed dataset for the Lab frame", RooArgSet(mass, y, pt, cosThetaLabVar, phiLabVar, etaLabDau1Var, etaLabDau2Var));
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
    RooArgSet variables(mass, pt, eta, phi, massDaughter1, mva);

    // RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    int nEntries = t->GetEntries();
    RooDataSet data("dataset", "dataset from tree", variables);

    Float_t Dstar_mass, Dstar_y, Dstar_pT, Dstar_Eta, Dstar_Phi;
    Float_t Dstar_Dau1_pT, Dstar_Dau1_y, Dstar_Dau1_Eta, Dstar_Dau1_Phi, Dstar_Dau1_mass;
    Float_t Dstar_Dau2_pT, Dstar_Dau2_y, Dstar_Dau2_Eta, Dstar_Dau2_Phi, Dstar_Dau2_mass;

    // Set branch addresses
    t->SetBranchAddress("mass", &Dstar_mass);
    t->SetBranchAddress("y", &Dstar_y);
    t->SetBranchAddress("pT", &Dstar_pT);
    t->SetBranchAddress("eta", &Dstar_Eta);
    t->SetBranchAddress("phi", &Dstar_Phi);
    t->SetBranchAddress("pTD1", &Dstar_Dau1_pT);
    t->SetBranchAddress("EtaD1", &Dstar_Dau1_Eta);
    t->SetBranchAddress("PhiD1", &Dstar_Dau1_Phi);
    t->SetBranchAddress("massDaugther1", &Dstar_Dau1_mass);
    t->SetBranchAddress("pTD2", &Dstar_Dau2_pT);
    t->SetBranchAddress("EtaD2", &Dstar_Dau2_Eta);
    t->SetBranchAddress("PhiD2", &Dstar_Dau2_Phi);
    // t->SetBranchAddress("massDaugther2", &Dstar_Dau2_mass);
    const int chunkSize = 100000; // Adjust chunk size as needed

    for (int i = 0; i < nEntries; i += chunkSize) {
        int end = std::min(i + chunkSize, nEntries);
        cout << i << " " << end << endl;
        for (int j = i; j < end; ++j) {
            t->GetEntry(j);

            // Perform calculations
            TLorentzVector dau1, dau2, dstar;
            dau1.SetPtEtaPhiM(Dstar_Dau1_pT, Dstar_Dau1_Eta, Dstar_Dau1_Phi, Dstar_Dau1_mass);
            dau2.SetPtEtaPhiM(Dstar_Dau2_pT, Dstar_Dau2_Eta, Dstar_Dau2_Phi, Dstar_Dau2_mass);
            dstar.SetPtEtaPhiM(Dstar_pT, Dstar_Eta, Dstar_Phi, Dstar_mass);

            // Set RooRealVar values
            // if(dstar.M()!=Dstar_mass){
                // cout << "Dstar mass: " << dstar.M() << Dstar_mass << endl;
            // }
            // mass.setVal(dstar.M());
            // pt.setVal(dstar.Pt());
            // eta.setVal(dstar.Eta());
            // phi.setVal(dstar.Phi());
            mass.setVal(Dstar_mass);
            pt.setVal(Dstar_pT);
            eta.setVal(Dstar_Eta);
            phi.setVal(Dstar_Phi);
            massDaughter1.setVal(Dstar_Dau1_mass);
            pTD1.setVal(Dstar_Dau1_pT);
            RapD1.setVal(dau1.Rapidity());
            EtaD1.setVal(Dstar_Dau1_Eta);
            pTD2.setVal(Dstar_Dau2_pT);
            RapD2.setVal(dau2.Rapidity());
            EtaD2.setVal(Dstar_Dau2_Eta);
            TVector3 DstarDau1_CS = DstarDau1Vector_CollinsSoper(dstar, dau1);

			TVector3 DstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
            // mva.setVal(0); // Set MVA value if available
            cosThetaLabVar = dau1.CosTheta();
			phiLabVar = dau2.Phi() * 180 / TMath::Pi();
			etaLabDau1Var = dau1.PseudoRapidity();
			etaLabDau2Var = dau2.PseudoRapidity();
            datasetLab.add(RooArgSet(mass, y, pt, pTD1, RapD1, EtaD1, pTD2, RapD2,EtaD2, cosThetaLabVar, phiLabVar, etaLabDau1Var, etaLabDau2Var));
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

			datasetCS.add(RooArgSet(mass, y, pt, pTD1, RapD1, EtaD1, pTD2, RapD2,EtaD2,  cosThetaCSVar, phiCSVar, phiTildeCSVar));
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

			datasetHX.add(RooArgSet(mass, y, pt, pTD1, RapD1, EtaD1, pTD2, RapD2,EtaD2, cosThetaHXVar, phiHXVar, phiTildeHXVar));

			// dataset.add(RooArgSet(massVa, yVar, ptVar, cosThetaCSVar, phiCSVar, cosThetaHXVar, phiHXVar));

            // Add event to dataset
            data.add(variables);
        }
    }
    data.addColumn(massPion);

    RooWorkspace *ws = new RooWorkspace("workspace");
    ws->import(mass);
    ws->import(pt);
    ws->import(eta);
    ws->import(phi);
    ws->import(mva);
    ws->import(massPion);
    ws->import(data);
    ws->import(datasetCS);
    ws->import(datasetLab);
    ws->import(datasetHX);
    ws->Write();
    fout->Close();
}