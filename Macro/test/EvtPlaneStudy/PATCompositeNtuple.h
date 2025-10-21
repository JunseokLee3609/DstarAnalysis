//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jun 30 18:32:37 2025 by ROOT version 6.24/09
// from TTree PATCompositeNtuple/PATCompositeNtuple
// found on file: output.root
//////////////////////////////////////////////////////////

#ifndef PATCompositeNtuple_h
#define PATCompositeNtuple_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

class PATCompositeNtuple {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

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
   Float_t         ephfpAngle[3];
   Float_t         ephfmAngle[3];
   Float_t         ephfpQ[3];
   Float_t         ephfmQ[3];
   Float_t         ephfpSumW;
   Float_t         ephfmSumW;
   Float_t         ephfAngle[2];
   Float_t         ephfAngleoff[2];
   Float_t         ephfmAngleoff[2];
   Float_t         ephfpAngleoff[2];
   Float_t         ephfAngleRaw[2];
   Float_t         ephfmAngleRaw[2];
   Float_t         ephfpAngleRaw[2];
   Float_t         ephfQ[2];
   Float_t         ephfSumW;
   Float_t         ephfmsumCosRaw[2];
   Float_t         ephfmsumSinRaw[2];
   Float_t         ephfmsumPtOrEt[2];
   Float_t         ephfpsumCosRaw[2];
   Float_t         ephfpsumSinRaw[2];
   Float_t         ephfpsumPtOrEt[2];
   Float_t         ephfsumCosRaw[2];
   Float_t         ephfsumSinRaw[2];
   Float_t         ephfsumSin[2];
   Float_t         ephfsumCos[2];
   Float_t         ephfsumPtOrEt[2];
   Float_t         pT[10];   //[candSize]
   Float_t         y[10];   //[candSize]
   Float_t         eta[10];   //[candSize]
   Float_t         phi[10];   //[candSize]
   Float_t         mass[10];   //[candSize]
   Float_t         mva[10];   //[candSize]
   Float_t         flavor[10];   //[candSize]
   Float_t         VtxProb[10];   //[candSize]
   Float_t         VtxChi2[10];   //[candSize]
   Float_t         VtxNDF[10];   //[candSize]
   Float_t         v3DCosPointingAngle[10];   //[candSize]
   Float_t         v3DPointingAngle[10];   //[candSize]
   Float_t         v2DCosPointingAngle[10];   //[candSize]
   Float_t         v2DPointingAngle[10];   //[candSize]
   Float_t         v3DDecayLengthSignificance[10];   //[candSize]
   Float_t         v3DDecayLength[10];   //[candSize]
   Float_t         v2DDecayLengthSignificance[10];   //[candSize]
   Float_t         v2DDecayLength[10];   //[candSize]
   Float_t         Trk3DDCA[10];   //[candSize]
   Float_t         Trk3DDCAErr[10];   //[candSize]
   Float_t         dca3D[10];   //[candSize]
   Float_t         dca3DErr[10];   //[candSize]
   Float_t         massDaugther1[10];   //[candSize]
   Float_t         pTD1[10];   //[candSize]
   Float_t         EtaD1[10];   //[candSize]
   Float_t         PhiD1[10];   //[candSize]
   Float_t         VtxProbDaugther1[10];   //[candSize]
   Float_t         VtxChi2Daugther1[10];   //[candSize]
   Float_t         VtxNDFDaugther1[10];   //[candSize]
   Float_t         v3DCosPointingAngleDaugther1[10];   //[candSize]
   Float_t         v3DPointingAngleDaugther1[10];   //[candSize]
   Float_t         v2DCosPointingAngleDaugther1[10];   //[candSize]
   Float_t         v2DPointingAngleDaugther1[10];   //[candSize]
   Float_t         v3DDecayLengthSignificanceDaugther1[10];   //[candSize]
   Float_t         v3DDecayLengthDaugther1[10];   //[candSize]
   Float_t         v3DDecayLengthErrorDaugther1[10];   //[candSize]
   Float_t         v2DDecayLengthSignificanceDaugther1[10];   //[candSize]
   Float_t         zDCASignificanceDaugther2[10];   //[candSize]
   Float_t         xyDCASignificanceDaugther2[10];   //[candSize]
   Float_t         NHitD2[10];   //[candSize]
   Bool_t          HighPurityDaugther2[10];   //[candSize]
   Float_t         pTD2[10];   //[candSize]
   Float_t         EtaD2[10];   //[candSize]
   Float_t         PhiD2[10];   //[candSize]
   Float_t         pTerrD1[10];   //[candSize]
   Float_t         pTerrD2[10];   //[candSize]
   Float_t         dedxHarmonic2D2[10];   //[candSize]
   Float_t         zDCASignificanceGrandDaugther1[10];   //[candSize]
   Float_t         zDCASignificanceGrandDaugther2[10];   //[candSize]
   Float_t         xyDCASignificanceGrandDaugther1[10];   //[candSize]
   Float_t         xyDCASignificanceGrandDaugther2[10];   //[candSize]
   Float_t         NHitGrandD1[10];   //[candSize]
   Float_t         NHitGrandD2[10];   //[candSize]
   Bool_t          HighPurityGrandDaugther1[10];   //[candSize]
   Bool_t          HighPurityGrandDaugther2[10];   //[candSize]
   Float_t         pTGrandD1[10];   //[candSize]
   Float_t         pTGrandD2[10];   //[candSize]
   Float_t         pTerrGrandD1[10];   //[candSize]
   Float_t         pTerrGrandD2[10];   //[candSize]
   Float_t         EtaGrandD1[10];   //[candSize]
   Float_t         EtaGrandD2[10];   //[candSize]
   Float_t         dedxHarmonic2GrandD1[10];   //[candSize]
   Float_t         dedxHarmonic2GrandD2[10];   //[candSize]

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
   TBranch        *b_ephfpAngle;   //!
   TBranch        *b_ephfmAngle;   //!
   TBranch        *b_ephfpQ;   //!
   TBranch        *b_ephfmQ;   //!
   TBranch        *b_ephfpSumW;   //!
   TBranch        *b_ephfmSumW;   //!
   TBranch        *b_ephfAngle;   //!
   TBranch        *b_ephfAngleoff;   //!
   TBranch        *b_ephfmAngleoff;   //!
   TBranch        *b_ephfpAngleoff;   //!
   TBranch        *b_ephfAngleRaw;   //!
   TBranch        *b_ephfmAngleRaw;   //!
   TBranch        *b_ephfpAngleRaw;   //!
   TBranch        *b_ephfQ;   //!
   TBranch        *b_ephfSumW;   //!
   TBranch        *b_ephfmsumCosRaw;   //!
   TBranch        *b_ephfmsumSinRaw;   //!
   TBranch        *b_ephfmsumPtOrEt;   //!
   TBranch        *b_ephfpsumCosRaw;   //!
   TBranch        *b_ephfpsumSinRaw;   //!
   TBranch        *b_ephfpsumPtOrEt;   //!
   TBranch        *b_ephfsumCosRaw;   //!
   TBranch        *b_ephfsumSinRaw;   //!
   TBranch        *b_ephfsumSin;   //!
   TBranch        *b_ephfsumCos;   //!
   TBranch        *b_ephfsumPtOrEt;   //!
   TBranch        *b_pT;   //!
   TBranch        *b_y;   //!
   TBranch        *b_eta;   //!
   TBranch        *b_phi;   //!
   TBranch        *b_mass;   //!
   TBranch        *b_mva;   //!
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
   TBranch        *b_Trk3DDCA;   //!
   TBranch        *b_Trk3DDCAErr;   //!
   TBranch        *b_dca3D;   //!
   TBranch        *b_dca3DErr;   //!
   TBranch        *b_massDaugther1;   //!
   TBranch        *b_pTD1;   //!
   TBranch        *b_EtaD1;   //!
   TBranch        *b_PhiD1;   //!
   TBranch        *b_VtxProbDaugther1;   //!
   TBranch        *b_VtxChi2Daugther1;   //!
   TBranch        *b_VtxNDFDaugther1;   //!
   TBranch        *b_3DCosPointingAngleDaugther1;   //!
   TBranch        *b_3DPointingAngleDaugther1;   //!
   TBranch        *b_2DCosPointingAngleDaugther1;   //!
   TBranch        *b_2DPointingAngleDaugther1;   //!
   TBranch        *b_3DDecayLengthSignificanceDaugther1;   //!
   TBranch        *b_3DDecayLengthDaugther1;   //!
   TBranch        *b_3DDecayLengthErrorDaugther1;   //!
   TBranch        *b_2DDecayLengthSignificanceDaugther1;   //!
   TBranch        *b_zDCASignificanceDaugther2;   //!
   TBranch        *b_xyDCASignificanceDaugther2;   //!
   TBranch        *b_NHitD2;   //!
   TBranch        *b_HighPurityDaugther2;   //!
   TBranch        *b_pTD2;   //!
   TBranch        *b_EtaD2;   //!
   TBranch        *b_PhiD2;   //!
   TBranch        *b_pTerrD1;   //!
   TBranch        *b_pTerrD2;   //!
   TBranch        *b_dedxHarmonic2D2;   //!
   TBranch        *b_zDCASignificanceGrandDaugther1;   //!
   TBranch        *b_zDCASignificanceGrandDaugther2;   //!
   TBranch        *b_xyDCASignificanceGrandDaugther1;   //!
   TBranch        *b_xyDCASignificanceGrandDaugther2;   //!
   TBranch        *b_NHitGrandD1;   //!
   TBranch        *b_NHitGrandD2;   //!
   TBranch        *b_HighPurityGrandDaugther1;   //!
   TBranch        *b_HighPurityGrandDaugther2;   //!
   TBranch        *b_pTGrandD1;   //!
   TBranch        *b_pTGrandD2;   //!
   TBranch        *b_pTerrGrandD1;   //!
   TBranch        *b_pTerrGrandD2;   //!
   TBranch        *b_EtaGrandD1;   //!
   TBranch        *b_EtaGrandD2;   //!
   TBranch        *b_dedxHarmonic2GrandD1;   //!
   TBranch        *b_dedxHarmonic2GrandD2;   //!

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
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("output.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("output.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("output.root:/dStarana");
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
   fChain->SetBranchAddress("ephfpAngle", ephfpAngle, &b_ephfpAngle);
   fChain->SetBranchAddress("ephfmAngle", ephfmAngle, &b_ephfmAngle);
   fChain->SetBranchAddress("ephfpQ", ephfpQ, &b_ephfpQ);
   fChain->SetBranchAddress("ephfmQ", ephfmQ, &b_ephfmQ);
   fChain->SetBranchAddress("ephfpSumW", &ephfpSumW, &b_ephfpSumW);
   fChain->SetBranchAddress("ephfmSumW", &ephfmSumW, &b_ephfmSumW);
   fChain->SetBranchAddress("ephfAngle", ephfAngle, &b_ephfAngle);
   fChain->SetBranchAddress("ephfAngleoff", ephfAngleoff, &b_ephfAngleoff);
   fChain->SetBranchAddress("ephfmAngleoff", ephfmAngleoff, &b_ephfmAngleoff);
   fChain->SetBranchAddress("ephfpAngleoff", ephfpAngleoff, &b_ephfpAngleoff);
   fChain->SetBranchAddress("ephfAngleRaw", ephfAngleRaw, &b_ephfAngleRaw);
   fChain->SetBranchAddress("ephfmAngleRaw", ephfmAngleRaw, &b_ephfmAngleRaw);
   fChain->SetBranchAddress("ephfpAngleRaw", ephfpAngleRaw, &b_ephfpAngleRaw);
   fChain->SetBranchAddress("ephfQ", ephfQ, &b_ephfQ);
   fChain->SetBranchAddress("ephfSumW", &ephfSumW, &b_ephfSumW);
   fChain->SetBranchAddress("ephfmsumCosRaw", ephfmsumCosRaw, &b_ephfmsumCosRaw);
   fChain->SetBranchAddress("ephfmsumSinRaw", ephfmsumSinRaw, &b_ephfmsumSinRaw);
   fChain->SetBranchAddress("ephfmsumPtOrEt", ephfmsumPtOrEt, &b_ephfmsumPtOrEt);
   fChain->SetBranchAddress("ephfpsumCosRaw", ephfpsumCosRaw, &b_ephfpsumCosRaw);
   fChain->SetBranchAddress("ephfpsumSinRaw", ephfpsumSinRaw, &b_ephfpsumSinRaw);
   fChain->SetBranchAddress("ephfpsumPtOrEt", ephfpsumPtOrEt, &b_ephfpsumPtOrEt);
   fChain->SetBranchAddress("ephfsumCosRaw", ephfsumCosRaw, &b_ephfsumCosRaw);
   fChain->SetBranchAddress("ephfsumSinRaw", ephfsumSinRaw, &b_ephfsumSinRaw);
   fChain->SetBranchAddress("ephfsumSin", ephfsumSin, &b_ephfsumSin);
   fChain->SetBranchAddress("ephfsumCos", ephfsumCos, &b_ephfsumCos);
   fChain->SetBranchAddress("ephfsumPtOrEt", ephfsumPtOrEt, &b_ephfsumPtOrEt);
   fChain->SetBranchAddress("pT", pT, &b_pT);
   fChain->SetBranchAddress("y", y, &b_y);
   fChain->SetBranchAddress("eta", eta, &b_eta);
   fChain->SetBranchAddress("phi", phi, &b_phi);
   fChain->SetBranchAddress("mass", mass, &b_mass);
   fChain->SetBranchAddress("mva", mva, &b_mva);
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
   fChain->SetBranchAddress("Trk3DDCA", Trk3DDCA, &b_Trk3DDCA);
   fChain->SetBranchAddress("Trk3DDCAErr", Trk3DDCAErr, &b_Trk3DDCAErr);
   fChain->SetBranchAddress("dca3D", dca3D, &b_dca3D);
   fChain->SetBranchAddress("dca3DErr", dca3DErr, &b_dca3DErr);
   fChain->SetBranchAddress("massDaugther1", massDaugther1, &b_massDaugther1);
   fChain->SetBranchAddress("pTD1", pTD1, &b_pTD1);
   fChain->SetBranchAddress("EtaD1", EtaD1, &b_EtaD1);
   fChain->SetBranchAddress("PhiD1", PhiD1, &b_PhiD1);
   fChain->SetBranchAddress("VtxProbDaugther1", VtxProbDaugther1, &b_VtxProbDaugther1);
   fChain->SetBranchAddress("VtxChi2Daugther1", VtxChi2Daugther1, &b_VtxChi2Daugther1);
   fChain->SetBranchAddress("VtxNDFDaugther1", VtxNDFDaugther1, &b_VtxNDFDaugther1);
   fChain->SetBranchAddress("3DCosPointingAngleDaugther1", v3DCosPointingAngleDaugther1, &b_3DCosPointingAngleDaugther1);
   fChain->SetBranchAddress("3DPointingAngleDaugther1", v3DPointingAngleDaugther1, &b_3DPointingAngleDaugther1);
   fChain->SetBranchAddress("2DCosPointingAngleDaugther1", v2DCosPointingAngleDaugther1, &b_2DCosPointingAngleDaugther1);
   fChain->SetBranchAddress("2DPointingAngleDaugther1", v2DPointingAngleDaugther1, &b_2DPointingAngleDaugther1);
   fChain->SetBranchAddress("3DDecayLengthSignificanceDaugther1", v3DDecayLengthSignificanceDaugther1, &b_3DDecayLengthSignificanceDaugther1);
   fChain->SetBranchAddress("3DDecayLengthDaugther1", v3DDecayLengthDaugther1, &b_3DDecayLengthDaugther1);
   fChain->SetBranchAddress("3DDecayLengthErrorDaugther1", v3DDecayLengthErrorDaugther1, &b_3DDecayLengthErrorDaugther1);
   fChain->SetBranchAddress("2DDecayLengthSignificanceDaugther1", v2DDecayLengthSignificanceDaugther1, &b_2DDecayLengthSignificanceDaugther1);
   fChain->SetBranchAddress("zDCASignificanceDaugther2", zDCASignificanceDaugther2, &b_zDCASignificanceDaugther2);
   fChain->SetBranchAddress("xyDCASignificanceDaugther2", xyDCASignificanceDaugther2, &b_xyDCASignificanceDaugther2);
   fChain->SetBranchAddress("NHitD2", NHitD2, &b_NHitD2);
   fChain->SetBranchAddress("HighPurityDaugther2", HighPurityDaugther2, &b_HighPurityDaugther2);
   fChain->SetBranchAddress("pTD2", pTD2, &b_pTD2);
   fChain->SetBranchAddress("EtaD2", EtaD2, &b_EtaD2);
   fChain->SetBranchAddress("PhiD2", PhiD2, &b_PhiD2);
   fChain->SetBranchAddress("pTerrD1", pTerrD1, &b_pTerrD1);
   fChain->SetBranchAddress("pTerrD2", pTerrD2, &b_pTerrD2);
   fChain->SetBranchAddress("dedxHarmonic2D2", dedxHarmonic2D2, &b_dedxHarmonic2D2);
   fChain->SetBranchAddress("zDCASignificanceGrandDaugther1", zDCASignificanceGrandDaugther1, &b_zDCASignificanceGrandDaugther1);
   fChain->SetBranchAddress("zDCASignificanceGrandDaugther2", zDCASignificanceGrandDaugther2, &b_zDCASignificanceGrandDaugther2);
   fChain->SetBranchAddress("xyDCASignificanceGrandDaugther1", xyDCASignificanceGrandDaugther1, &b_xyDCASignificanceGrandDaugther1);
   fChain->SetBranchAddress("xyDCASignificanceGrandDaugther2", xyDCASignificanceGrandDaugther2, &b_xyDCASignificanceGrandDaugther2);
   fChain->SetBranchAddress("NHitGrandD1", NHitGrandD1, &b_NHitGrandD1);
   fChain->SetBranchAddress("NHitGrandD2", NHitGrandD2, &b_NHitGrandD2);
   fChain->SetBranchAddress("HighPurityGrandDaugther1", HighPurityGrandDaugther1, &b_HighPurityGrandDaugther1);
   fChain->SetBranchAddress("HighPurityGrandDaugther2", HighPurityGrandDaugther2, &b_HighPurityGrandDaugther2);
   fChain->SetBranchAddress("pTGrandD1", pTGrandD1, &b_pTGrandD1);
   fChain->SetBranchAddress("pTGrandD2", pTGrandD2, &b_pTGrandD2);
   fChain->SetBranchAddress("pTerrGrandD1", pTerrGrandD1, &b_pTerrGrandD1);
   fChain->SetBranchAddress("pTerrGrandD2", pTerrGrandD2, &b_pTerrGrandD2);
   fChain->SetBranchAddress("EtaGrandD1", EtaGrandD1, &b_EtaGrandD1);
   fChain->SetBranchAddress("EtaGrandD2", EtaGrandD2, &b_EtaGrandD2);
   fChain->SetBranchAddress("dedxHarmonic2GrandD1", dedxHarmonic2GrandD1, &b_dedxHarmonic2GrandD1);
   fChain->SetBranchAddress("dedxHarmonic2GrandD2", dedxHarmonic2GrandD2, &b_dedxHarmonic2GrandD2);
   Notify();
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
#endif // #ifdef PATCompositeNtuple_cxx
