//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Feb 26 18:10:11 2025 by ROOT version 6.24/09
// from TTree PATCompositeNtuple/PATCompositeNtuple
// found on file: d0ana_tree.root
//////////////////////////////////////////////////////////

#ifndef PATCompositeNtuple_h
#define PATCompositeNtuple_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
const int __MAXCAND_MC__ = 22999;

// Header file for the classes stored in the TTree if any.
namespace DataFormat{	
	class PATCompositeNtuple {
		protected:
			const int __MAXCAND_MC__ = 22999;
		public :
			TTree          *fChain;   //!pointer to the analyzed TTree or TChain
			Int_t           fCurrent; //!current Tree number in a TChain

			// Fixed size dimensions of array or collections stored in the TTree if any.
			static constexpr Int_t kMaxmatchGen_D1ancestorId = 1;
			static constexpr Int_t kMaxmatchGen_D1ancestorFlavor = 1;
			static constexpr Int_t kMaxgen_D0ancestorId = 1;
			static constexpr Int_t kMaxgen_D0ancestorFlavor = 1;

			// Declaration of leaf types
			Int_t           Ntrkoffline;
			Int_t           Npixel;
			Float_t         HFsumETPlus;
			Float_t         HFsumETMinus;
			Float_t         ZDCPlus;
			Float_t         ZDCMinus;
			Float_t         bestvtxX;
			Float_t         bestvtxY;
			Float_t         bestvtxZ;
			Int_t           candSize;
			Float_t         pT[MAXCAND];   //[candSize]
			Float_t         y[MAXCAND];   //[candSize]
			Float_t         eta[MAXCAND];   //[candSize]
			Float_t         phi[MAXCAND];   //[candSize]
			Float_t         mass[MAXCAND];   //[candSize]
			Float_t         flavor[MAXCAND];   //[candSize]
			Float_t         VtxProb[MAXCAND];   //[candSize]
			Float_t         VtxChi2[MAXCAND];   //[candSize]
			Float_t         VtxNDF[MAXCAND];   //[candSize]
			Float_t         v3DCosPointingAngle[MAXCAND];   //[candSize]
			Float_t         v3DPointingAngle[MAXCAND];   //[candSize]
			Float_t         v2DCosPointingAngle[MAXCAND];   //[candSize]
			Float_t         v2DPointingAngle[MAXCAND];   //[candSize]
			Float_t         v3DDecayLengthSignificance[MAXCAND];   //[candSize]
			Float_t         v3DDecayLength[MAXCAND];   //[candSize]
			Float_t         v2DDecayLengthSignificance[MAXCAND];   //[candSize]
			Float_t         v2DDecayLength[MAXCAND];   //[candSize]
			Bool_t          isSwap[MAXCAND];   //[candSize]
			Int_t           idmom_reco[MAXCAND];   //[candSize]
			Int_t           idBAnc_reco[MAXCAND];   //[candSize]
			Bool_t          matchGEN[MAXCAND];   //[candSize]
			Float_t         matchGen3DPointingAngle[MAXCAND];   //[candSize]
			Float_t         matchGen2DPointingAngle[MAXCAND];   //[candSize]
			Float_t         matchGen3DDecayLength[MAXCAND];   //[candSize]
			Float_t         matchGen2DDecayLength[MAXCAND];   //[candSize]
			Float_t         matchGen_D0pT[MAXCAND];   //[candSize]
			Float_t         matchGen_D0eta[MAXCAND];   //[candSize]
			Float_t         matchGen_D0phi[MAXCAND];   //[candSize]
			Float_t         matchGen_D0mass[MAXCAND];   //[candSize]
			Float_t         matchGen_D0y[MAXCAND];   //[candSize]
			Float_t         matchGen_D0charge[MAXCAND];   //[candSize]
			Int_t           matchGen_D0pdgId[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau1_pT[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau1_eta[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau1_phi[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau1_mass[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau1_y[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau1_charge[MAXCAND];   //[candSize]
			Int_t           matchGen_D0Dau1_pdgId[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau2_pT[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau2_eta[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau2_phi[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau2_mass[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau2_y[MAXCAND];   //[candSize]
			Float_t         matchGen_D0Dau2_charge[MAXCAND];   //[candSize]
			Int_t           matchGen_D0Dau2_pdgId[MAXCAND];   //[candSize]
			Int_t           matchGen_D1ancestorId_[MAXCAND];   //[candSize]
			Int_t           matchGen_D1ancestorFlavor_[MAXCAND];   //[candSize]
			Float_t         zDCASignificanceDaugther1[MAXCAND];   //[candSize]
			Float_t         xyDCASignificanceDaugther1[MAXCAND];   //[candSize]
			Float_t         NHitD1[MAXCAND];   //[candSize]
			Bool_t          HighPurityDaugther1[MAXCAND];   //[candSize]
			Float_t         pTD1[MAXCAND];   //[candSize]
			Float_t         pTerrD1[MAXCAND];   //[candSize]
			Float_t         EtaD1[MAXCAND];   //[candSize]
			Float_t         PhiD1[MAXCAND];   //[candSize]
			Float_t         dedxHarmonic2D1[MAXCAND];   //[candSize]
			Float_t         zDCASignificanceDaugther2[MAXCAND];   //[candSize]
			Float_t         xyDCASignificanceDaugther2[MAXCAND];   //[candSize]
			Float_t         NHitD2[MAXCAND];   //[candSize]
			Bool_t          HighPurityDaugther2[MAXCAND];   //[candSize]
			Float_t         pTD2[MAXCAND];   //[candSize]
			Float_t         pTerrD2[MAXCAND];   //[candSize]
			Float_t         EtaD2[MAXCAND];   //[candSize]
			Float_t         PhiD2[MAXCAND];   //[candSize]
			Float_t         dedxHarmonic2D2[MAXCAND];   //[candSize]
			Int_t           candSize_gen;
			Float_t         gen_mass[MAXCAND];   //[candSize_gen]
			Float_t         gen_pT[MAXCAND];   //[candSize_gen]
			Float_t         gen_eta[MAXCAND];   //[candSize_gen]
			Float_t         gen_phi[MAXCAND];   //[candSize_gen]
			Float_t         gen_y[MAXCAND];   //[candSize_gen]
			Int_t           gen_status[MAXCAND];   //[candSize_gen]
			Int_t           gen_pdgId[MAXCAND];   //[candSize_gen]
			Int_t           gen_charge[MAXCAND];   //[candSize_gen]
			Int_t           gen_MotherID[MAXCAND];   //[candSize_gen]
			Int_t           gen_DauID1[MAXCAND];   //[candSize_gen]
			Int_t           gen_DauID2[MAXCAND];   //[candSize_gen]
			Int_t           gen_DauID3[MAXCAND];   //[candSize_gen]
			Int_t           gen_D0ancestorId_[MAXCAND];   //[candSize_gen]
			Int_t           gen_D0ancestorFlavor_[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau1_pT[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau1_eta[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau1_phi[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau1_mass[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau1_y[MAXCAND];   //[candSize_gen]
			Int_t           gen_D0Dau1_charge[MAXCAND];   //[candSize_gen]
			Int_t           gen_D0Dau1_pdgId[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau2_pT[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau2_eta[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau2_phi[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau2_mass[MAXCAND];   //[candSize_gen]
			Float_t         gen_D0Dau2_y[MAXCAND];   //[candSize_gen]
			Int_t           gen_D0Dau2_charge[MAXCAND];   //[candSize_gen]
			Int_t           gen_D0Dau2_pdgId[MAXCAND];   //[candSize_gen]

			// List of branches
			TBranch        *b_Ntrkoffline;   //!
			TBranch        *b_Npixel;   //!
			TBranch        *b_HFsumETPlus;   //!
			TBranch        *b_HFsumETMinus;   //!
			TBranch        *b_ZDCPlus;   //!
			TBranch        *b_ZDCMinus;   //!
			TBranch        *b_bestvtxX;   //!
			TBranch        *b_bestvtxY;   //!
			TBranch        *b_bestvtxZ;   //!
			TBranch        *b_candSize;   //!
			TBranch        *b_pT;   //!
			TBranch        *b_y;   //!
			TBranch        *b_eta;   //!
			TBranch        *b_phi;   //!
			TBranch        *b_mass;   //!
			TBranch        *b_flavor;   //!
			TBranch        *b_VtxProb;   //!
			TBranch        *b_VtxChi2;   //!
			TBranch        *b_VtxNDF;   //!
			TBranch        *b_3DCosPointingAngle;   //!
			TBranch        *b_3DPointingAngle;   //!
			TBranch        *b_2DCosPointingAngle;   //!
			TBranch        *b_2DPointingAngle;   //!
			TBranch        *b_3DDecayLengthSignificance;   //!
			TBranch        *b_3DDecayLength;   //!
			TBranch        *b_2DDecayLengthSignificance;   //!
			TBranch        *b_2DDecayLength;   //!
			TBranch        *b_isSwap;   //!
			TBranch        *b_idmom_reco;   //!
			TBranch        *b_idBAnc_reco;   //!
			TBranch        *b_matchGEN;   //!
			TBranch        *b_matchGen3DPointingAngle;   //!
			TBranch        *b_matchGen2DPointingAngle;   //!
			TBranch        *b_matchGen3DDecayLength;   //!
			TBranch        *b_matchGen2DDecayLength;   //!
			TBranch        *b_matchGen_D0pT;   //!
			TBranch        *b_matchGen_D0eta;   //!
			TBranch        *b_matchGen_D0phi;   //!
			TBranch        *b_matchGen_D0mass;   //!
			TBranch        *b_matchGen_D0y;   //!
			TBranch        *b_matchGen_D0charge;   //!
			TBranch        *b_matchGen_D0pdgId;   //!
			TBranch        *b_matchGen_D0Dau1_pT;   //!
			TBranch        *b_matchGen_D0Dau1_eta;   //!
			TBranch        *b_matchGen_D0Dau1_phi;   //!
			TBranch        *b_matchGen_D0Dau1_mass;   //!
			TBranch        *b_matchGen_D0Dau1_y;   //!
			TBranch        *b_matchGen_D0Dau1_charge;   //!
			TBranch        *b_matchGen_D0Dau1_pdgId;   //!
			TBranch        *b_matchGen_D0Dau2_pT;   //!
			TBranch        *b_matchGen_D0Dau2_eta;   //!
			TBranch        *b_matchGen_D0Dau2_phi;   //!
			TBranch        *b_matchGen_D0Dau2_mass;   //!
			TBranch        *b_matchGen_D0Dau2_y;   //!
			TBranch        *b_matchGen_D0Dau2_charge;   //!
			TBranch        *b_matchGen_D0Dau2_pdgId;   //!
			TBranch        *b_matchGen_D1ancestorId_;   //!
			TBranch        *b_matchGen_D1ancestorFlavor_;   //!
			TBranch        *b_zDCASignificanceDaugther1;   //!
			TBranch        *b_xyDCASignificanceDaugther1;   //!
			TBranch        *b_NHitD1;   //!
			TBranch        *b_HighPurityDaugther1;   //!
			TBranch        *b_pTD1;   //!
			TBranch        *b_pTerrD1;   //!
			TBranch        *b_EtaD1;   //!
			TBranch        *b_PhiD1;   //!
			TBranch        *b_dedxHarmonic2D1;   //!
			TBranch        *b_zDCASignificanceDaugther2;   //!
			TBranch        *b_xyDCASignificanceDaugther2;   //!
			TBranch        *b_NHitD2;   //!
			TBranch        *b_HighPurityDaugther2;   //!
			TBranch        *b_pTD2;   //!
			TBranch        *b_pTerrD2;   //!
			TBranch        *b_EtaD2;   //!
			TBranch        *b_PhiD2;   //!
			TBranch        *b_dedxHarmonic2D2;   //!
			TBranch        *b_candSize_gen;   //!
			TBranch        *b_gen_mass;   //!
			TBranch        *b_gen_pT;   //!
			TBranch        *b_gen_eta;   //!
			TBranch        *b_gen_phi;   //!
			TBranch        *b_gen_y;   //!
			TBranch        *b_gen_status;   //!
			TBranch        *b_gen_pdgId;   //!
			TBranch        *b_gen_charge;   //!
			TBranch        *b_gen_MotherID;   //!
			TBranch        *b_gen_DauID1;   //!
			TBranch        *b_gen_DauID2;   //!
			TBranch        *b_gen_DauID3;   //!
			TBranch        *b_gen_D0ancestorId_;   //!
			TBranch        *b_gen_D0ancestorFlavor_;   //!
			TBranch        *b_gen_D0Dau1_pT;   //!
			TBranch        *b_gen_D0Dau1_eta;   //!
			TBranch        *b_gen_D0Dau1_phi;   //!
			TBranch        *b_gen_D0Dau1_mass;   //!
			TBranch        *b_gen_D0Dau1_y;   //!
			TBranch        *b_gen_D0Dau1_charge;   //!
			TBranch        *b_gen_D0Dau1_pdgId;   //!
			TBranch        *b_gen_D0Dau2_pT;   //!
			TBranch        *b_gen_D0Dau2_eta;   //!
			TBranch        *b_gen_D0Dau2_phi;   //!
			TBranch        *b_gen_D0Dau2_mass;   //!
			TBranch        *b_gen_D0Dau2_y;   //!
			TBranch        *b_gen_D0Dau2_charge;   //!
			TBranch        *b_gen_D0Dau2_pdgId;   //!

			PATCompositeNtuple(TTree *tree=0);
			virtual ~PATCompositeNtuple();
			virtual Int_t    Cut(Long64_t entry);
			virtual Int_t    GetEntry(Long64_t entry);
			virtual Long64_t LoadTree(Long64_t entry);
			virtual void     Init(TTree *tree);
			virtual void     Loop();
			virtual Bool_t   Notify();
			virtual void     Show(Long64_t entry = -1);
	};

#endif

#ifdef PATCompositeNtuple_cxx
	PATCompositeNtuple::PATCompositeNtuple(TTree *tree) : fChain(0) 
	{
		// if parameter tree is not specified (or zero), connect the file
		// used to generate this class and read the Tree.
		if (tree == 0) {
			TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("d0ana_tree.root");
			if (!f || !f->IsOpen()) {
				f = new TFile("d0ana_tree.root");
			}
			TDirectory * dir = (TDirectory*)f->Get("d0ana_tree.root:/d0ana_newreduced");
			dir->GetObject("PATCompositeNtuple",tree);

		}
		Init(tree);
	}

	PATCompositeNtuple::~PATCompositeNtuple()
	{
		if (!fChain) return;
		delete fChain->GetCurrentFile();
	}

	Int_t PATCompositeNtuple::GetEntry(Long64_t entry)
	{
		// Read contents of entry.
		if (!fChain) return 0;
		return fChain->GetEntry(entry);
	}
	Long64_t PATCompositeNtuple::LoadTree(Long64_t entry)
	{
		// Set the environment to read one entry
		if (!fChain) return -5;
		Long64_t centry = fChain->LoadTree(entry);
		if (centry < 0) return centry;
		if (fChain->GetTreeNumber() != fCurrent) {
			fCurrent = fChain->GetTreeNumber();
			Notify();
		}
		return centry;
	}

	void PATCompositeNtuple::Init(TTree *tree)
	{
		// The Init() function is called when the selector needs to initialize
		// a new tree or chain. Typically here the branch addresses and branch
		// pointers of the tree will be set.
		// It is normally not necessary to make changes to the generated
		// code, but the routine can be extended by the user if needed.
		// Init() will be called many times when running on PROOF
		// (once per file to be processed).

		// Set branch addresses and branch pointers
		if (!tree) return;
		fChain = tree;
		fCurrent = -1;
		fChain->SetMakeClass(1);

		fChain->SetBranchAddress("Ntrkoffline", &Ntrkoffline, &b_Ntrkoffline);
		fChain->SetBranchAddress("Npixel", &Npixel, &b_Npixel);
		fChain->SetBranchAddress("HFsumETPlus", &HFsumETPlus, &b_HFsumETPlus);
		fChain->SetBranchAddress("HFsumETMinus", &HFsumETMinus, &b_HFsumETMinus);
		fChain->SetBranchAddress("ZDCPlus", &ZDCPlus, &b_ZDCPlus);
		fChain->SetBranchAddress("ZDCMinus", &ZDCMinus, &b_ZDCMinus);
		fChain->SetBranchAddress("bestvtxX", &bestvtxX, &b_bestvtxX);
		fChain->SetBranchAddress("bestvtxY", &bestvtxY, &b_bestvtxY);
		fChain->SetBranchAddress("bestvtxZ", &bestvtxZ, &b_bestvtxZ);
		fChain->SetBranchAddress("candSize", &candSize, &b_candSize);
		fChain->SetBranchAddress("pT", pT, &b_pT);
		fChain->SetBranchAddress("y", y, &b_y);
		fChain->SetBranchAddress("eta", eta, &b_eta);
		fChain->SetBranchAddress("phi", phi, &b_phi);
		fChain->SetBranchAddress("mass", mass, &b_mass);
		fChain->SetBranchAddress("flavor", flavor, &b_flavor);
		fChain->SetBranchAddress("VtxProb", VtxProb, &b_VtxProb);
		fChain->SetBranchAddress("VtxChi2", VtxChi2, &b_VtxChi2);
		fChain->SetBranchAddress("VtxNDF", VtxNDF, &b_VtxNDF);
		fChain->SetBranchAddress("3DCosPointingAngle", v3DCosPointingAngle, &b_3DCosPointingAngle);
		fChain->SetBranchAddress("3DPointingAngle", v3DPointingAngle, &b_3DPointingAngle);
		fChain->SetBranchAddress("2DCosPointingAngle", v2DCosPointingAngle, &b_2DCosPointingAngle);
		fChain->SetBranchAddress("2DPointingAngle", v2DPointingAngle, &b_2DPointingAngle);
		fChain->SetBranchAddress("3DDecayLengthSignificance", v3DDecayLengthSignificance, &b_3DDecayLengthSignificance);
		fChain->SetBranchAddress("3DDecayLength", v3DDecayLength, &b_3DDecayLength);
		fChain->SetBranchAddress("2DDecayLengthSignificance", v2DDecayLengthSignificance, &b_2DDecayLengthSignificance);
		fChain->SetBranchAddress("2DDecayLength", v2DDecayLength, &b_2DDecayLength);
		fChain->SetBranchAddress("isSwap", isSwap, &b_isSwap);
		fChain->SetBranchAddress("idmom_reco", idmom_reco, &b_idmom_reco);
		fChain->SetBranchAddress("idBAnc_reco", idBAnc_reco, &b_idBAnc_reco);
		fChain->SetBranchAddress("matchGEN", matchGEN, &b_matchGEN);
		fChain->SetBranchAddress("matchGen3DPointingAngle", matchGen3DPointingAngle, &b_matchGen3DPointingAngle);
		fChain->SetBranchAddress("matchGen2DPointingAngle", matchGen2DPointingAngle, &b_matchGen2DPointingAngle);
		fChain->SetBranchAddress("matchGen3DDecayLength", matchGen3DDecayLength, &b_matchGen3DDecayLength);
		fChain->SetBranchAddress("matchGen2DDecayLength", matchGen2DDecayLength, &b_matchGen2DDecayLength);
		fChain->SetBranchAddress("matchGen_D0pT", matchGen_D0pT, &b_matchGen_D0pT);
		fChain->SetBranchAddress("matchGen_D0eta", matchGen_D0eta, &b_matchGen_D0eta);
		fChain->SetBranchAddress("matchGen_D0phi", matchGen_D0phi, &b_matchGen_D0phi);
		fChain->SetBranchAddress("matchGen_D0mass", matchGen_D0mass, &b_matchGen_D0mass);
		fChain->SetBranchAddress("matchGen_D0y", matchGen_D0y, &b_matchGen_D0y);
		fChain->SetBranchAddress("matchGen_D0charge", matchGen_D0charge, &b_matchGen_D0charge);
		fChain->SetBranchAddress("matchGen_D0pdgId", matchGen_D0pdgId, &b_matchGen_D0pdgId);
		fChain->SetBranchAddress("matchGen_D0Dau1_pT", matchGen_D0Dau1_pT, &b_matchGen_D0Dau1_pT);
		fChain->SetBranchAddress("matchGen_D0Dau1_eta", matchGen_D0Dau1_eta, &b_matchGen_D0Dau1_eta);
		fChain->SetBranchAddress("matchGen_D0Dau1_phi", matchGen_D0Dau1_phi, &b_matchGen_D0Dau1_phi);
		fChain->SetBranchAddress("matchGen_D0Dau1_mass", matchGen_D0Dau1_mass, &b_matchGen_D0Dau1_mass);
		fChain->SetBranchAddress("matchGen_D0Dau1_y", matchGen_D0Dau1_y, &b_matchGen_D0Dau1_y);
		fChain->SetBranchAddress("matchGen_D0Dau1_charge", matchGen_D0Dau1_charge, &b_matchGen_D0Dau1_charge);
		fChain->SetBranchAddress("matchGen_D0Dau1_pdgId", matchGen_D0Dau1_pdgId, &b_matchGen_D0Dau1_pdgId);
		fChain->SetBranchAddress("matchGen_D0Dau2_pT", matchGen_D0Dau2_pT, &b_matchGen_D0Dau2_pT);
		fChain->SetBranchAddress("matchGen_D0Dau2_eta", matchGen_D0Dau2_eta, &b_matchGen_D0Dau2_eta);
		fChain->SetBranchAddress("matchGen_D0Dau2_phi", matchGen_D0Dau2_phi, &b_matchGen_D0Dau2_phi);
		fChain->SetBranchAddress("matchGen_D0Dau2_mass", matchGen_D0Dau2_mass, &b_matchGen_D0Dau2_mass);
		fChain->SetBranchAddress("matchGen_D0Dau2_y", matchGen_D0Dau2_y, &b_matchGen_D0Dau2_y);
		fChain->SetBranchAddress("matchGen_D0Dau2_charge", matchGen_D0Dau2_charge, &b_matchGen_D0Dau2_charge);
		fChain->SetBranchAddress("matchGen_D0Dau2_pdgId", matchGen_D0Dau2_pdgId, &b_matchGen_D0Dau2_pdgId);
		fChain->SetBranchAddress("matchGen_D1ancestorId_", matchGen_D1ancestorId_, &b_matchGen_D1ancestorId_);
		fChain->SetBranchAddress("matchGen_D1ancestorFlavor_", matchGen_D1ancestorFlavor_, &b_matchGen_D1ancestorFlavor_);
		fChain->SetBranchAddress("zDCASignificanceDaugther1", zDCASignificanceDaugther1, &b_zDCASignificanceDaugther1);
		fChain->SetBranchAddress("xyDCASignificanceDaugther1", xyDCASignificanceDaugther1, &b_xyDCASignificanceDaugther1);
		fChain->SetBranchAddress("NHitD1", NHitD1, &b_NHitD1);
		fChain->SetBranchAddress("HighPurityDaugther1", HighPurityDaugther1, &b_HighPurityDaugther1);
		fChain->SetBranchAddress("pTD1", pTD1, &b_pTD1);
		fChain->SetBranchAddress("pTerrD1", pTerrD1, &b_pTerrD1);
		fChain->SetBranchAddress("EtaD1", EtaD1, &b_EtaD1);
		fChain->SetBranchAddress("PhiD1", PhiD1, &b_PhiD1);
		fChain->SetBranchAddress("dedxHarmonic2D1", dedxHarmonic2D1, &b_dedxHarmonic2D1);
		fChain->SetBranchAddress("zDCASignificanceDaugther2", zDCASignificanceDaugther2, &b_zDCASignificanceDaugther2);
		fChain->SetBranchAddress("xyDCASignificanceDaugther2", xyDCASignificanceDaugther2, &b_xyDCASignificanceDaugther2);
		fChain->SetBranchAddress("NHitD2", NHitD2, &b_NHitD2);
		fChain->SetBranchAddress("HighPurityDaugther2", HighPurityDaugther2, &b_HighPurityDaugther2);
		fChain->SetBranchAddress("pTD2", pTD2, &b_pTD2);
		fChain->SetBranchAddress("pTerrD2", pTerrD2, &b_pTerrD2);
		fChain->SetBranchAddress("EtaD2", EtaD2, &b_EtaD2);
		fChain->SetBranchAddress("PhiD2", PhiD2, &b_PhiD2);
		fChain->SetBranchAddress("dedxHarmonic2D2", dedxHarmonic2D2, &b_dedxHarmonic2D2);
		fChain->SetBranchAddress("candSize_gen", &candSize_gen, &b_candSize_gen);
		fChain->SetBranchAddress("gen_mass", gen_mass, &b_gen_mass);
		fChain->SetBranchAddress("gen_pT", gen_pT, &b_gen_pT);
		fChain->SetBranchAddress("gen_eta", gen_eta, &b_gen_eta);
		fChain->SetBranchAddress("gen_phi", gen_phi, &b_gen_phi);
		fChain->SetBranchAddress("gen_y", gen_y, &b_gen_y);
		fChain->SetBranchAddress("gen_status", gen_status, &b_gen_status);
		fChain->SetBranchAddress("gen_pdgId", gen_pdgId, &b_gen_pdgId);
		fChain->SetBranchAddress("gen_charge", gen_charge, &b_gen_charge);
		fChain->SetBranchAddress("gen_MotherID", gen_MotherID, &b_gen_MotherID);
		fChain->SetBranchAddress("gen_DauID1", gen_DauID1, &b_gen_DauID1);
		fChain->SetBranchAddress("gen_DauID2", gen_DauID2, &b_gen_DauID2);
		fChain->SetBranchAddress("gen_DauID3", gen_DauID3, &b_gen_DauID3);
		fChain->SetBranchAddress("gen_D0ancestorId_", gen_D0ancestorId_, &b_gen_D0ancestorId_);
		fChain->SetBranchAddress("gen_D0ancestorFlavor_", gen_D0ancestorFlavor_, &b_gen_D0ancestorFlavor_);
		fChain->SetBranchAddress("gen_D0Dau1_pT", gen_D0Dau1_pT, &b_gen_D0Dau1_pT);
		fChain->SetBranchAddress("gen_D0Dau1_eta", gen_D0Dau1_eta, &b_gen_D0Dau1_eta);
		fChain->SetBranchAddress("gen_D0Dau1_phi", gen_D0Dau1_phi, &b_gen_D0Dau1_phi);
		fChain->SetBranchAddress("gen_D0Dau1_mass", gen_D0Dau1_mass, &b_gen_D0Dau1_mass);
		fChain->SetBranchAddress("gen_D0Dau1_y", gen_D0Dau1_y, &b_gen_D0Dau1_y);
		fChain->SetBranchAddress("gen_D0Dau1_charge", gen_D0Dau1_charge, &b_gen_D0Dau1_charge);
		fChain->SetBranchAddress("gen_D0Dau1_pdgId", gen_D0Dau1_pdgId, &b_gen_D0Dau1_pdgId);
		fChain->SetBranchAddress("gen_D0Dau2_pT", gen_D0Dau2_pT, &b_gen_D0Dau2_pT);
		fChain->SetBranchAddress("gen_D0Dau2_eta", gen_D0Dau2_eta, &b_gen_D0Dau2_eta);
		fChain->SetBranchAddress("gen_D0Dau2_phi", gen_D0Dau2_phi, &b_gen_D0Dau2_phi);
		fChain->SetBranchAddress("gen_D0Dau2_mass", gen_D0Dau2_mass, &b_gen_D0Dau2_mass);
		fChain->SetBranchAddress("gen_D0Dau2_y", gen_D0Dau2_y, &b_gen_D0Dau2_y);
		fChain->SetBranchAddress("gen_D0Dau2_charge", gen_D0Dau2_charge, &b_gen_D0Dau2_charge);
		fChain->SetBranchAddress("gen_D0Dau2_pdgId", gen_D0Dau2_pdgId, &b_gen_D0Dau2_pdgId);
		Notify();
		fChain->SetBranchStatus("*", 0);
	}

	Bool_t PATCompositeNtuple::Notify()
	{
		// The Notify() function is called when a new file is opened. This
		// can be either for a new TTree in a TChain or when when a new TTree
		// is started when using PROOF. It is normally not necessary to make changes
		// to the generated code, but the routine can be extended by the
		// user if needed. The return value is currently not used.

		return kTRUE;
	}

	void PATCompositeNtuple::Show(Long64_t entry)
	{
		// Print contents of entry.
		// If entry is not specified, print current entry
		if (!fChain) return;
		fChain->Show(entry);
	}
	Int_t PATCompositeNtuple::Cut(Long64_t entry)
	{
		// This function may be called from Loop.
		// returns  1 if entry is accepted.
		// returns -1 otherwise.
		return 1;
	}
	void PATCompositeNtuple::SetStatus(vector<std::string> branches)
	{
		for (auto branch : branches){
			fChain->SetBranchStatus(branch.c_str(), 1);	
		}
	}
	class simpleDMCTreeflat{
        public:
            Int_t           Ntrkoffline;
   Int_t           Npixel;
   Float_t         HFsumETPlus;
   Float_t         HFsumETMinus;
   Float_t         ZDCPlus;
   Float_t         ZDCMinus;
   Float_t         bestvtxX;
   Float_t         bestvtxY;
   Float_t         bestvtxZ;
   Int_t           candSize;
   Float_t         pT;   //[candSize]
   Float_t         y;   //[candSize]
   Float_t         eta;   //[candSize]
   Float_t         phi;   //[candSize]
   Float_t         mass;   //[candSize]
   Float_t         flavor;   //[candSize]
   Float_t         VtxProb;   //[candSize]
   Float_t         VtxChi2;   //[candSize]
   Float_t         VtxNDF;   //[candSize]
   Float_t         _3DCosPointingAngle;   //[candSize]
   Float_t         _3DPointingAngle;   //[candSize]
   Float_t         _2DCosPointingAngle;   //[candSize]
   Float_t         _2DPointingAngle;   //[candSize]
   Float_t         _3DDecayLengthSignificance;   //[candSize]
   Float_t         _3DDecayLength;   //[candSize]
   Float_t         _2DDecayLengthSignificance;   //[candSize]
   Float_t         _2DDecayLength;   //[candSize]
   Bool_t          isSwap;   //[candSize]
   Int_t           idmom_reco;   //[candSize]
   Int_t           idBAnc_reco;   //[candSize]
   Bool_t          matchGEN;   //[candSize]
   Float_t         matchGen3DPointingAngle;   //[candSize]
   Float_t         matchGen2DPointingAngle;   //[candSize]
   Float_t         matchGen3DDecayLength;   //[candSize]
   Float_t         matchGen2DDecayLength;   //[candSize]
   Float_t         matchGen_D0pT;   //[candSize]
   Float_t         matchGen_D0eta;   //[candSize]
   Float_t         matchGen_D0phi;   //[candSize]
   Float_t         matchGen_D0mass;   //[candSize]
   Float_t         matchGen_D0y;   //[candSize]
   Float_t         matchGen_D0charge;   //[candSize]
   Int_t           matchGen_D0pdgId;   //[candSize]
   Float_t         matchGen_D0Dau1_pT;   //[candSize]
   Float_t         matchGen_D0Dau1_eta;   //[candSize]
   Float_t         matchGen_D0Dau1_phi;   //[candSize]
   Float_t         matchGen_D0Dau1_mass;   //[candSize]
   Float_t         matchGen_D0Dau1_y;   //[candSize]
   Float_t         matchGen_D0Dau1_charge;   //[candSize]
   Int_t           matchGen_D0Dau1_pdgId;   //[candSize]
   Float_t         matchGen_D0Dau2_pT;   //[candSize]
   Float_t         matchGen_D0Dau2_eta;   //[candSize]
   Float_t         matchGen_D0Dau2_phi;   //[candSize]
   Float_t         matchGen_D0Dau2_mass;   //[candSize]
   Float_t         matchGen_D0Dau2_y;   //[candSize]
   Float_t         matchGen_D0Dau2_charge;   //[candSize]
   Int_t           matchGen_D0Dau2_pdgId;   //[candSize]
   Int_t           matchGen_D1ancestorId_;   //[candSize]
   Int_t           matchGen_D1ancestorFlavor_;   //[candSize]
   Float_t         zDCASignificanceDaugther1;   //[candSize]
   Float_t         xyDCASignificanceDaugther1;   //[candSize]
   Float_t         NHitD1;   //[candSize]
   Bool_t          HighPurityDaugther1;   //[candSize]
   Float_t         pTD1;   //[candSize]
   Float_t         pTerrD1;   //[candSize]
   Float_t         EtaD1;   //[candSize]
   Float_t         PhiD1;   //[candSize]
   Float_t         dedxHarmonic2D1;   //[candSize]
   Float_t         zDCASignificanceDaugther2;   //[candSize]
   Float_t         xyDCASignificanceDaugther2;   //[candSize]
   Float_t         NHitD2;   //[candSize]
   Bool_t          HighPurityDaugther2;   //[candSize]
   Float_t         pTD2;   //[candSize]
   Float_t         pTerrD2;   //[candSize]
   Float_t         EtaD2;   //[candSize]
   Float_t         PhiD2;   //[candSize]
   Float_t         dedxHarmonic2D2;   //[candSize]
   Int_t           candSize_gen;
   Float_t         gen_mass;   //[candSize_gen]
   Float_t         gen_pT;   //[candSize_gen]
   Float_t         gen_eta;   //[candSize_gen]
   Float_t         gen_phi;   //[candSize_gen]
   Float_t         gen_y;   //[candSize_gen]
   Int_t           gen_status;   //[candSize_gen]
   Int_t           gen_pdgId;   //[candSize_gen]
   Int_t           gen_charge;   //[candSize_gen]
   Int_t           gen_MotherID;   //[candSize_gen]
   Int_t           gen_DauID1;   //[candSize_gen]
   Int_t           gen_DauID2;   //[candSize_gen]
   Int_t           gen_DauID3;   //[candSize_gen]
   Int_t           gen_D0ancestorId_;   //[candSize_gen]
   Int_t           gen_D0ancestorFlavor_;   //[candSize_gen]
   Float_t         gen_D0Dau1_pT;   //[candSize_gen]
   Float_t         gen_D0Dau1_eta;   //[candSize_gen]
   Float_t         gen_D0Dau1_phi;   //[candSize_gen]
   Float_t         gen_D0Dau1_mass;   //[candSize_gen]
   Float_t         gen_D0Dau1_y;   //[candSize_gen]
   Int_t           gen_D0Dau1_charge;   //[candSize_gen]
   Int_t           gen_D0Dau1_pdgId;   //[candSize_gen]
   Float_t         gen_D0Dau2_pT;   //[candSize_gen]
   Float_t         gen_D0Dau2_eta;   //[candSize_gen]
   Float_t         gen_D0Dau2_phi;   //[candSize_gen]
   Float_t         gen_D0Dau2_mass;   //[candSize_gen]
   Float_t         gen_D0Dau2_y;   //[candSize_gen]
   Int_t           gen_D0Dau2_charge;   //[candSize_gen]
   Int_t           gen_D0Dau2_pdgId;   //[candSize_gen]
    simpleDMCTreeflat* getEventHandle(){ return this;}
    template <typename T>
    void setTree(T *fChain){
   fChain->SetBranchAddress("Ntrkoffline", &Ntrkoffline);
   fChain->SetBranchAddress("Npixel", &Npixel);
   fChain->SetBranchAddress("HFsumETPlus", &HFsumETPlus);
   fChain->SetBranchAddress("HFsumETMinus", &HFsumETMinus);
   fChain->SetBranchAddress("ZDCPlus", &ZDCPlus);
   fChain->SetBranchAddress("ZDCMinus", &ZDCMinus);
   fChain->SetBranchAddress("bestvtxX", &bestvtxX);
   fChain->SetBranchAddress("bestvtxY", &bestvtxY);
   fChain->SetBranchAddress("bestvtxZ", &bestvtxZ);
   fChain->SetBranchAddress("candSize", &candSize);
   fChain->SetBranchAddress("pT",&pT);
   fChain->SetBranchAddress("y",&y);
   fChain->SetBranchAddress("eta",&eta);
   fChain->SetBranchAddress("phi",&phi);
   fChain->SetBranchAddress("mass",&mass);
   fChain->SetBranchAddress("flavor",&flavor);
   fChain->SetBranchAddress("VtxProb",&VtxProb);
   fChain->SetBranchAddress("VtxChi2",&VtxChi2);
   fChain->SetBranchAddress("VtxNDF",&VtxNDF);
   fChain->SetBranchAddress("3DCosPointingAngle",&_3DCosPointingAngle);
   fChain->SetBranchAddress("3DPointingAngle",&_3DPointingAngle);
   fChain->SetBranchAddress("2DCosPointingAngle",&_2DCosPointingAngle);
   fChain->SetBranchAddress("2DPointingAngle",&_2DPointingAngle);
   fChain->SetBranchAddress("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
   fChain->SetBranchAddress("3DDecayLength",&_3DDecayLength);
   fChain->SetBranchAddress("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
   fChain->SetBranchAddress("2DDecayLength",&_2DDecayLength);
   fChain->SetBranchAddress("isSwap",&isSwap);
   fChain->SetBranchAddress("idmom_reco",&idmom_reco);
   fChain->SetBranchAddress("idBAnc_reco",&idBAnc_reco);
   fChain->SetBranchAddress("matchGEN",&matchGEN);
   fChain->SetBranchAddress("matchGen3DPointingAngle",&matchGen3DPointingAngle);
   fChain->SetBranchAddress("matchGen2DPointingAngle",&matchGen2DPointingAngle);
   fChain->SetBranchAddress("matchGen3DDecayLength",&matchGen3DDecayLength);
   fChain->SetBranchAddress("matchGen2DDecayLength",&matchGen2DDecayLength);
   fChain->SetBranchAddress("matchGen_D0pT",&matchGen_D0pT);
   fChain->SetBranchAddress("matchGen_D0eta",&matchGen_D0eta);
   fChain->SetBranchAddress("matchGen_D0phi",&matchGen_D0phi);
   fChain->SetBranchAddress("matchGen_D0mass",&matchGen_D0mass);
   fChain->SetBranchAddress("matchGen_D0y",&matchGen_D0y);
   fChain->SetBranchAddress("matchGen_D0charge",&matchGen_D0charge);
   fChain->SetBranchAddress("matchGen_D0pdgId",&matchGen_D0pdgId);
   fChain->SetBranchAddress("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT);
   fChain->SetBranchAddress("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta);
   fChain->SetBranchAddress("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi);
   fChain->SetBranchAddress("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass);
   fChain->SetBranchAddress("matchGen_D0Dau1_y",&matchGen_D0Dau1_y);
   fChain->SetBranchAddress("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge);
   fChain->SetBranchAddress("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId);
   fChain->SetBranchAddress("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT);
   fChain->SetBranchAddress("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta);
   fChain->SetBranchAddress("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi);
   fChain->SetBranchAddress("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass);
   fChain->SetBranchAddress("matchGen_D0Dau2_y",&matchGen_D0Dau2_y);
   fChain->SetBranchAddress("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge);
   fChain->SetBranchAddress("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId);
   fChain->SetBranchAddress("matchGen_D1ancestorId_",&matchGen_D1ancestorId_);
   fChain->SetBranchAddress("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_);
   fChain->SetBranchAddress("zDCASignificanceDaugther1",&zDCASignificanceDaugther1);
   fChain->SetBranchAddress("xyDCASignificanceDaugther1",&xyDCASignificanceDaugther1);
   fChain->SetBranchAddress("NHitD1",&NHitD1);
   fChain->SetBranchAddress("HighPurityDaugther1",&HighPurityDaugther1);
   fChain->SetBranchAddress("pTD1",&pTD1);
   fChain->SetBranchAddress("pTerrD1",&pTerrD1);
   fChain->SetBranchAddress("EtaD1",&EtaD1);
   fChain->SetBranchAddress("PhiD1",&PhiD1);
   fChain->SetBranchAddress("dedxHarmonic2D1",&dedxHarmonic2D1);
   fChain->SetBranchAddress("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
   fChain->SetBranchAddress("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
   fChain->SetBranchAddress("NHitD2",&NHitD2);
   fChain->SetBranchAddress("HighPurityDaugther2",&HighPurityDaugther2);
   fChain->SetBranchAddress("pTD2",&pTD2);
   fChain->SetBranchAddress("pTerrD2",&pTerrD2);
   fChain->SetBranchAddress("EtaD2",&EtaD2);
   fChain->SetBranchAddress("PhiD2",&PhiD2);
   fChain->SetBranchAddress("dedxHarmonic2D2",&dedxHarmonic2D2);
   fChain->SetBranchAddress("candSize_gen",&candSize_gen);
   fChain->SetBranchAddress("gen_mass",&gen_mass);
   fChain->SetBranchAddress("gen_pT",&gen_pT);
   fChain->SetBranchAddress("gen_eta",&gen_eta);
   fChain->SetBranchAddress("gen_phi",&gen_phi);
   fChain->SetBranchAddress("gen_y",&gen_y);
   fChain->SetBranchAddress("gen_status",&gen_status);
   fChain->SetBranchAddress("gen_pdgId",&gen_pdgId);
   fChain->SetBranchAddress("gen_charge",&gen_charge);
   fChain->SetBranchAddress("gen_MotherID",&gen_MotherID);
   fChain->SetBranchAddress("gen_DauID1",&gen_DauID1);
   fChain->SetBranchAddress("gen_DauID2",&gen_DauID2);
   fChain->SetBranchAddress("gen_DauID3",&gen_DauID3);
   fChain->SetBranchAddress("gen_D0ancestorId_",&gen_D0ancestorId_);
   fChain->SetBranchAddress("gen_D0ancestorFlavor_",&gen_D0ancestorFlavor_);
   fChain->SetBranchAddress("gen_D0Dau1_pT",&gen_D0Dau1_pT);
   fChain->SetBranchAddress("gen_D0Dau1_eta",&gen_D0Dau1_eta);
   fChain->SetBranchAddress("gen_D0Dau1_phi",&gen_D0Dau1_phi);
   fChain->SetBranchAddress("gen_D0Dau1_mass",&gen_D0Dau1_mass);
   fChain->SetBranchAddress("gen_D0Dau1_y",&gen_D0Dau1_y);
   fChain->SetBranchAddress("gen_D0Dau1_charge",&gen_D0Dau1_charge);
   fChain->SetBranchAddress("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId);
   fChain->SetBranchAddress("gen_D0Dau2_pT",&gen_D0Dau2_pT);
   fChain->SetBranchAddress("gen_D0Dau2_eta",&gen_D0Dau2_eta);
   fChain->SetBranchAddress("gen_D0Dau2_phi",&gen_D0Dau2_phi);
   fChain->SetBranchAddress("gen_D0Dau2_mass",&gen_D0Dau2_mass);
   fChain->SetBranchAddress("gen_D0Dau2_y",&gen_D0Dau2_y);
   fChain->SetBranchAddress("gen_D0Dau2_charge",&gen_D0Dau2_charge);
   fChain->SetBranchAddress("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId);
   fChain->SetBranchStatus("*", 0);
 	};
	void SetStatus(vector<std::string> branches){
		for (auto branch : branches){
			fChain->SetBranchStatus(branch.c_str(), 1);	
		}
	}
    template <typename T>
    void setOutputTree(T *fChain, vector<std::string> branches){
		for (auto branch : branches){
			fChain->Branch(branch.c_str(), &branch);
		}
	}


	}
	
}
#endif
