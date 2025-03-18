#ifndef __DATAFORMAT_D_MC__
#define __DATAFORMAT_D_MC__
// #include "src/common/interface/commonHeader.hxx"
#include <TTree.h>
#include <TBranch.h>
#include <iostream>
const int __MAXCAND_MC__ = 35000;

namespace DataFormat{
    struct simpleDMCTreeevt{
        protected:
            static const int MAXCAND = __MAXCAND_MC__;
        public :
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
   Float_t         _3DCosPointingAngle[MAXCAND];   //[candSize]
   Float_t         _3DPointingAngle[MAXCAND];   //[candSize]
   Float_t         _2DCosPointingAngle[MAXCAND];   //[candSize]
   Float_t         _2DPointingAngle[MAXCAND];   //[candSize]
   Float_t         _3DDecayLengthSignificance[MAXCAND];   //[candSize]
   Float_t         _3DDecayLength[MAXCAND];   //[candSize]
   Float_t         _2DDecayLengthSignificance[MAXCAND];   //[candSize]
   Float_t         _2DDecayLength[MAXCAND];   //[candSize]
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
   Float_t         Trk3DDCA[MAXCAND];   //[candSize]
   Float_t         Trk3DDCAErr[MAXCAND];   //[candSize]
   Float_t         mva[MAXCAND];   //[candSize]

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
    TBranch        *b_Trk3DDCA;   //!
    TBranch        *b_Trk3DDCAErr;   //!
    TBranch        *b_mva;   //!
   simpleDMCTreeevt* getEventHandle(){ return this;}
    template <typename T>
        void setTree(T *fChain){
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
   fChain->SetBranchAddress("3DCosPointingAngle", _3DCosPointingAngle, &b_3DCosPointingAngle);
   fChain->SetBranchAddress("3DPointingAngle", _3DPointingAngle, &b_3DPointingAngle);
   fChain->SetBranchAddress("2DCosPointingAngle", _2DCosPointingAngle, &b_2DCosPointingAngle);
   fChain->SetBranchAddress("2DPointingAngle", _2DPointingAngle, &b_2DPointingAngle);
   fChain->SetBranchAddress("3DDecayLengthSignificance", _3DDecayLengthSignificance, &b_3DDecayLengthSignificance);
   fChain->SetBranchAddress("3DDecayLength", _3DDecayLength, &b_3DDecayLength);
   fChain->SetBranchAddress("2DDecayLengthSignificance", _2DDecayLengthSignificance, &b_2DDecayLengthSignificance);
   fChain->SetBranchAddress("2DDecayLength", _2DDecayLength, &b_2DDecayLength);
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
   fChain->SetBranchAddress("Trk3DDCA", Trk3DDCA, &b_Trk3DDCA);
    fChain->SetBranchAddress("Trk3DDCAErr", Trk3DDCAErr, &b_Trk3DDCAErr);
    fChain->SetBranchAddress("mva", mva, &b_mva);
        };
    template <typename T>
        void setGENTree(T *fChain){ 
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
        };
    };
    struct simpleDMCTreeflat{
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
   Float_t        Trk3DDCA;   //[candSize]
    Float_t        Trk3DDCAErr;   //[candSize]
    Float_t        mva;   //[candSize]
   bool isMC;
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
    fChain->SetBranchAddress("Trk3DDCA",&Trk3DDCA);
    fChain->SetBranchAddress("Trk3DDCAErr",&Trk3DDCAErr);
    fChain->SetBranchAddress("mva",&mva);
    };
    template <typename T>
    void setGENTree(T *fChain){
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
    };
        template <typename T>
    void setOutputTree(T *fChain){
   fChain->Branch("Ntrkoffline", &Ntrkoffline);
   fChain->Branch("Npixel", &Npixel);
   fChain->Branch("HFsumETPlus", &HFsumETPlus);
   fChain->Branch("HFsumETMinus", &HFsumETMinus);
   fChain->Branch("ZDCPlus", &ZDCPlus);
   fChain->Branch("ZDCMinus", &ZDCMinus);
   fChain->Branch("bestvtxX", &bestvtxX);
   fChain->Branch("bestvtxY", &bestvtxY);
   fChain->Branch("bestvtxZ", &bestvtxZ);
   fChain->Branch("candSize", &candSize);
   fChain->Branch("pT",&pT);
   fChain->Branch("y",&y);
   fChain->Branch("eta",&eta);
   fChain->Branch("phi",&phi);
   fChain->Branch("mass",&mass);
   fChain->Branch("flavor",&flavor);
   fChain->Branch("VtxProb",&VtxProb);
   fChain->Branch("VtxChi2",&VtxChi2);
   fChain->Branch("VtxNDF",&VtxNDF);
   fChain->Branch("3DCosPointingAngle",&_3DCosPointingAngle);
   fChain->Branch("3DPointingAngle",&_3DPointingAngle);
   fChain->Branch("2DCosPointingAngle",&_2DCosPointingAngle);
   fChain->Branch("2DPointingAngle",&_2DPointingAngle);
   fChain->Branch("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
   fChain->Branch("3DDecayLength",&_3DDecayLength);
   fChain->Branch("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
   fChain->Branch("2DDecayLength",&_2DDecayLength);
   fChain->Branch("isSwap",&isSwap);
   fChain->Branch("idmom_reco",&idmom_reco);
   fChain->Branch("idBAnc_reco",&idBAnc_reco);
   fChain->Branch("matchGEN",&matchGEN);
   fChain->Branch("matchGen3DPointingAngle",&matchGen3DPointingAngle);
   fChain->Branch("matchGen2DPointingAngle",&matchGen2DPointingAngle);
   fChain->Branch("matchGen3DDecayLength",&matchGen3DDecayLength);
   fChain->Branch("matchGen2DDecayLength",&matchGen2DDecayLength);
   fChain->Branch("matchGen_D0pT",&matchGen_D0pT);
   fChain->Branch("matchGen_D0eta",&matchGen_D0eta);
   fChain->Branch("matchGen_D0phi",&matchGen_D0phi);
   fChain->Branch("matchGen_D0mass",&matchGen_D0mass);
   fChain->Branch("matchGen_D0y",&matchGen_D0y);
   fChain->Branch("matchGen_D0charge",&matchGen_D0charge);
   fChain->Branch("matchGen_D0pdgId",&matchGen_D0pdgId);
   fChain->Branch("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT);
   fChain->Branch("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta);
   fChain->Branch("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi);
   fChain->Branch("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass);
   fChain->Branch("matchGen_D0Dau1_y",&matchGen_D0Dau1_y);
   fChain->Branch("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge);
   fChain->Branch("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId);
   fChain->Branch("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT);
   fChain->Branch("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta);
   fChain->Branch("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi);
   fChain->Branch("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass);
   fChain->Branch("matchGen_D0Dau2_y",&matchGen_D0Dau2_y);
   fChain->Branch("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge);
   fChain->Branch("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId);
   fChain->Branch("matchGen_D1ancestorId_",&matchGen_D1ancestorId_);
   fChain->Branch("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_);
   fChain->Branch("zDCASignificanceDaugther1",&zDCASignificanceDaugther1);
   fChain->Branch("xyDCASignificanceDaugther1",&xyDCASignificanceDaugther1);
   fChain->Branch("NHitD1",&NHitD1);
   fChain->Branch("HighPurityDaugther1",&HighPurityDaugther1);
   fChain->Branch("pTD1",&pTD1);
   fChain->Branch("pTerrD1",&pTerrD1);
   fChain->Branch("EtaD1",&EtaD1);
   fChain->Branch("PhiD1",&PhiD1);
   fChain->Branch("dedxHarmonic2D1",&dedxHarmonic2D1);
   fChain->Branch("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
   fChain->Branch("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
   fChain->Branch("NHitD2",&NHitD2);
   fChain->Branch("HighPurityDaugther2",&HighPurityDaugther2);
   fChain->Branch("pTD2",&pTD2);
   fChain->Branch("pTerrD2",&pTerrD2);
   fChain->Branch("EtaD2",&EtaD2);
   fChain->Branch("PhiD2",&PhiD2);
   fChain->Branch("dedxHarmonic2D2",&dedxHarmonic2D2);
   fChain->Branch("isMC",&isMC);
    fChain->Branch("Trk3DDCA",&Trk3DDCA);
    fChain->Branch("Trk3DDCAErr",&Trk3DDCAErr);
    fChain->Branch("mva",&mva);
   
    };
    template <typename T>
    void setGENOutputTree(T *fChain){
   fChain->Branch("candSize_gen",&candSize_gen);
   fChain->Branch("gen_mass",&gen_mass);
   fChain->Branch("gen_pT",&gen_pT);
   fChain->Branch("gen_eta",&gen_eta);
   fChain->Branch("gen_phi",&gen_phi);
   fChain->Branch("gen_y",&gen_y);
   fChain->Branch("gen_status",&gen_status);
   fChain->Branch("gen_pdgId",&gen_pdgId);
   fChain->Branch("gen_charge",&gen_charge);
   fChain->Branch("gen_MotherID",&gen_MotherID);
   fChain->Branch("gen_DauID1",&gen_DauID1);
   fChain->Branch("gen_DauID2",&gen_DauID2);
   fChain->Branch("gen_DauID3",&gen_DauID3);
   fChain->Branch("gen_D0ancestorId_",&gen_D0ancestorId_);
   fChain->Branch("gen_D0ancestorFlavor_",&gen_D0ancestorFlavor_);
   fChain->Branch("gen_D0Dau1_pT",&gen_D0Dau1_pT);
   fChain->Branch("gen_D0Dau1_eta",&gen_D0Dau1_eta);
   fChain->Branch("gen_D0Dau1_phi",&gen_D0Dau1_phi);
   fChain->Branch("gen_D0Dau1_mass",&gen_D0Dau1_mass);
   fChain->Branch("gen_D0Dau1_y",&gen_D0Dau1_y);
   fChain->Branch("gen_D0Dau1_charge",&gen_D0Dau1_charge);
   fChain->Branch("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId);
   fChain->Branch("gen_D0Dau2_pT",&gen_D0Dau2_pT);
   fChain->Branch("gen_D0Dau2_eta",&gen_D0Dau2_eta);
   fChain->Branch("gen_D0Dau2_phi",&gen_D0Dau2_phi);
   fChain->Branch("gen_D0Dau2_mass",&gen_D0Dau2_mass);
   fChain->Branch("gen_D0Dau2_y",&gen_D0Dau2_y);
   fChain->Branch("gen_D0Dau2_charge",&gen_D0Dau2_charge);
   fChain->Branch("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId);
    };
void copyDn( simpleDMCTreeevt& evt, int idx){
Npixel =evt.Npixel;
HFsumETPlus =evt.HFsumETPlus;
HFsumETMinus =evt.HFsumETMinus;
ZDCPlus =evt.ZDCPlus;
ZDCMinus =evt.ZDCMinus;
bestvtxX =evt.bestvtxX;
bestvtxY =evt.bestvtxY;
bestvtxZ =evt.bestvtxZ;
candSize =evt.candSize;
pT =evt.pT[idx];   //[candSize]
y =evt.y[idx];   //[candSize]
eta =evt.eta[idx];   //[candSize]
phi =evt.phi[idx];   //[candSize]
mass =evt.mass[idx];   //[candSize]
flavor =evt.flavor[idx];   //[candSize]
VtxProb =evt.VtxProb[idx];   //[candSize]
VtxChi2 =evt.VtxChi2[idx];   //[candSize]
VtxNDF =evt.VtxNDF[idx];   //[candSize]
_3DCosPointingAngle =evt._3DCosPointingAngle[idx];   //[candSize]
_3DPointingAngle =evt._3DPointingAngle[idx];   //[candSize]
_2DCosPointingAngle =evt._2DCosPointingAngle[idx];   //[candSize]
_2DPointingAngle =evt._2DPointingAngle[idx];   //[candSize]
_3DDecayLengthSignificance =evt._3DDecayLengthSignificance[idx];   //[candSize]
_3DDecayLength =evt._3DDecayLength[idx];   //[candSize]
_2DDecayLengthSignificance =evt._2DDecayLengthSignificance[idx];   //[candSize]
_2DDecayLength =evt._2DDecayLength[idx];   //[candSize]
isSwap =evt.isSwap[idx];   //[candSize]
idmom_reco =evt.idmom_reco[idx];   //[candSize]
idBAnc_reco =evt.idBAnc_reco[idx];   //[candSize]
matchGEN =evt.matchGEN[idx];   //[candSize]
matchGen3DPointingAngle =evt.matchGen3DPointingAngle[idx];   //[candSize]
matchGen2DPointingAngle =evt.matchGen2DPointingAngle[idx];   //[candSize]
matchGen3DDecayLength =evt.matchGen3DDecayLength[idx];   //[candSize]
matchGen2DDecayLength =evt.matchGen2DDecayLength[idx];   //[candSize]
matchGen_D0pT =evt.matchGen_D0pT[idx];   //[candSize]
matchGen_D0eta =evt.matchGen_D0eta[idx];   //[candSize]
matchGen_D0phi =evt.matchGen_D0phi[idx];   //[candSize]
matchGen_D0mass =evt.matchGen_D0mass[idx];   //[candSize]
matchGen_D0y =evt.matchGen_D0y[idx];   //[candSize]
matchGen_D0charge =evt.matchGen_D0charge[idx];   //[candSize]
matchGen_D0pdgId =evt.matchGen_D0pdgId[idx];   //[candSize]
matchGen_D0Dau1_pT =evt.matchGen_D0Dau1_pT[idx];   //[candSize]
matchGen_D0Dau1_eta =evt.matchGen_D0Dau1_eta[idx];   //[candSize]
matchGen_D0Dau1_phi =evt.matchGen_D0Dau1_phi[idx];   //[candSize]
matchGen_D0Dau1_mass =evt.matchGen_D0Dau1_mass[idx];   //[candSize]
matchGen_D0Dau1_y =evt.matchGen_D0Dau1_y[idx];   //[candSize]
matchGen_D0Dau1_charge =evt.matchGen_D0Dau1_charge[idx];   //[candSize]
matchGen_D0Dau1_pdgId =evt.matchGen_D0Dau1_pdgId[idx];   //[candSize]
matchGen_D0Dau2_pT =evt.matchGen_D0Dau2_pT[idx];   //[candSize]
matchGen_D0Dau2_eta =evt.matchGen_D0Dau2_eta[idx];   //[candSize]
matchGen_D0Dau2_phi =evt.matchGen_D0Dau2_phi[idx];   //[candSize]
matchGen_D0Dau2_mass =evt.matchGen_D0Dau2_mass[idx];   //[candSize]
matchGen_D0Dau2_y =evt.matchGen_D0Dau2_y[idx];   //[candSize]
matchGen_D0Dau2_charge =evt.matchGen_D0Dau2_charge[idx];   //[candSize]
matchGen_D0Dau2_pdgId =evt.matchGen_D0Dau2_pdgId[idx];   //[candSize]
matchGen_D1ancestorId_ =evt.matchGen_D1ancestorId_[idx];   //[candSize]
matchGen_D1ancestorFlavor_ =evt.matchGen_D1ancestorFlavor_[idx];   //[candSize]
zDCASignificanceDaugther1 =evt.zDCASignificanceDaugther1[idx];   //[candSize]
xyDCASignificanceDaugther1 =evt.xyDCASignificanceDaugther1[idx];   //[candSize]
NHitD1 =evt.NHitD1[idx];   //[candSize]
HighPurityDaugther1 =evt.HighPurityDaugther1[idx];   //[candSize]
pTD1 =evt.pTD1[idx];   //[candSize]
pTerrD1 =evt.pTerrD1[idx];   //[candSize]
EtaD1 =evt.EtaD1[idx];   //[candSize]
PhiD1 =evt.PhiD1[idx];   //[candSize]
dedxHarmonic2D1 =evt.dedxHarmonic2D1[idx];   //[candSize]
zDCASignificanceDaugther2 =evt.zDCASignificanceDaugther2[idx];   //[candSize]
xyDCASignificanceDaugther2 =evt.xyDCASignificanceDaugther2[idx];   //[candSize]
NHitD2 =evt.NHitD2[idx];   //[candSize]
HighPurityDaugther2 =evt.HighPurityDaugther2[idx];   //[candSize]
pTD2 =evt.pTD2[idx];   //[candSize]
pTerrD2 =evt.pTerrD2[idx];   //[candSize]
EtaD2 =evt.EtaD2[idx];   //[candSize]
PhiD2 =evt.PhiD2[idx];   //[candSize]
dedxHarmonic2D2 =evt.dedxHarmonic2D2[idx];   //[candSize]
Trk3DDCA =evt.Trk3DDCA[idx];   //[candSize]
Trk3DDCAErr =evt.Trk3DDCAErr[idx];   //[candSize]
mva =evt.mva[idx];   //[candSize]

        };
template <typename T>
void copyDn( T& evt, int idx){
Npixel =evt.Npixel;
HFsumETPlus =evt.HFsumETPlus;
HFsumETMinus =evt.HFsumETMinus;
ZDCPlus =evt.ZDCPlus;
ZDCMinus =evt.ZDCMinus;
bestvtxX =evt.bestvtxX;
bestvtxY =evt.bestvtxY;
bestvtxZ =evt.bestvtxZ;
candSize =evt.candSize;
pT =evt.pT[idx];   //[candSize]
y =evt.y[idx];   //[candSize]
eta =evt.eta[idx];   //[candSize]
phi =evt.phi[idx];   //[candSize]
mass =evt.mass[idx];   //[candSize]
flavor =evt.flavor[idx];   //[candSize]
VtxProb =evt.VtxProb[idx];   //[candSize]
VtxChi2 =evt.VtxChi2[idx];   //[candSize]
VtxNDF =evt.VtxNDF[idx];   //[candSize]
_3DCosPointingAngle =evt._3DCosPointingAngle[idx];   //[candSize]
_3DPointingAngle =evt._3DPointingAngle[idx];   //[candSize]
_2DCosPointingAngle =evt._2DCosPointingAngle[idx];   //[candSize]
_2DPointingAngle =evt._2DPointingAngle[idx];   //[candSize]
_3DDecayLengthSignificance =evt._3DDecayLengthSignificance[idx];   //[candSize]
_3DDecayLength =evt._3DDecayLength[idx];   //[candSize]
_2DDecayLengthSignificance =evt._2DDecayLengthSignificance[idx];   //[candSize]
_2DDecayLength =evt._2DDecayLength[idx];   //[candSize]
zDCASignificanceDaugther1 =evt.zDCASignificanceDaugther1[idx];   //[candSize]
xyDCASignificanceDaugther1 =evt.xyDCASignificanceDaugther1[idx];   //[candSize]
NHitD1 =evt.NHitD1[idx];   //[candSize]
HighPurityDaugther1 =evt.HighPurityDaugther1[idx];   //[candSize]
pTD1 =evt.pTD1[idx];   //[candSize]
pTerrD1 =evt.pTerrD1[idx];   //[candSize]
EtaD1 =evt.EtaD1[idx];   //[candSize]
PhiD1 =evt.PhiD1[idx];   //[candSize]
dedxHarmonic2D1 =evt.dedxHarmonic2D1[idx];   //[candSize]
zDCASignificanceDaugther2 =evt.zDCASignificanceDaugther2[idx];   //[candSize]
xyDCASignificanceDaugther2 =evt.xyDCASignificanceDaugther2[idx];   //[candSize]
NHitD2 =evt.NHitD2[idx];   //[candSize]
HighPurityDaugther2 =evt.HighPurityDaugther2[idx];   //[candSize]
pTD2 =evt.pTD2[idx];   //[candSize]
pTerrD2 =evt.pTerrD2[idx];   //[candSize]
EtaD2 =evt.EtaD2[idx];   //[candSize]
PhiD2 =evt.PhiD2[idx];   //[candSize]
dedxHarmonic2D2 =evt.dedxHarmonic2D2[idx];   //[candSize]
Trk3DDCA =evt.Trk3DDCA[idx];   //[candSize]
Trk3DDCAErr =evt.Trk3DDCAErr[idx];   //[candSize]
mva =evt.mva[idx];   //[candSize]

// isMC=isMC;
        };
void copyGENDn( simpleDMCTreeevt& evt, int idx){
candSize_gen =evt.candSize_gen;
gen_mass =evt.gen_mass[idx];   //[candSize_gen]
gen_pT =evt.gen_pT[idx];   //[candSize_gen]
gen_eta =evt.gen_eta[idx];   //[candSize_gen]
gen_phi =evt.gen_phi[idx];   //[candSize_gen]
gen_y =evt.gen_y[idx];   //[candSize_gen]
gen_status =evt.gen_status[idx];   //[candSize_gen]
gen_pdgId =evt.gen_pdgId[idx];   //[candSize_gen]
gen_charge =evt.gen_charge[idx];   //[candSize_gen]
gen_MotherID =evt.gen_MotherID[idx];   //[candSize_gen]
gen_DauID1 =evt.gen_DauID1[idx];   //[candSize_gen]
gen_DauID2 =evt.gen_DauID2[idx];   //[candSize_gen]
gen_DauID3 =evt.gen_DauID3[idx];   //[candSize_gen]
gen_D0ancestorId_ =evt.gen_D0ancestorId_[idx];   //[candSize_gen]
gen_D0ancestorFlavor_ =evt.gen_D0ancestorFlavor_[idx];   //[candSize_gen]
gen_D0Dau1_pT =evt.gen_D0Dau1_pT[idx];   //[candSize_gen]
gen_D0Dau1_eta =evt.gen_D0Dau1_eta[idx];   //[candSize_gen]
gen_D0Dau1_phi =evt.gen_D0Dau1_phi[idx];   //[candSize_gen]
gen_D0Dau1_mass =evt.gen_D0Dau1_mass[idx];   //[candSize_gen]
gen_D0Dau1_y =evt.gen_D0Dau1_y[idx];   //[candSize_gen]
gen_D0Dau1_charge =evt.gen_D0Dau1_charge[idx];   //[candSize_gen]
gen_D0Dau1_pdgId =evt.gen_D0Dau1_pdgId[idx];   //[candSize_gen]
gen_D0Dau2_pT =evt.gen_D0Dau2_pT[idx];   //[candSize_gen]
gen_D0Dau2_eta =evt.gen_D0Dau2_eta[idx];   //[candSize_gen]
gen_D0Dau2_phi =evt.gen_D0Dau2_phi[idx];   //[candSize_gen]
gen_D0Dau2_mass =evt.gen_D0Dau2_mass[idx];   //[candSize_gen]
gen_D0Dau2_y =evt.gen_D0Dau2_y[idx];   //[candSize_gen]
gen_D0Dau2_charge =evt.gen_D0Dau2_charge[idx];   //[candSize_gen]
gen_D0Dau2_pdgId =evt.gen_D0Dau2_pdgId[idx];   //[candSize_gen]
    };
};
    struct simpleDTreeevt{
        protected:
            static const int MAXCAND = __MAXCAND_MC__;
        public :
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
   Float_t         _3DCosPointingAngle[MAXCAND];   //[candSize]
   Float_t         _3DPointingAngle[MAXCAND];   //[candSize]
   Float_t         _2DCosPointingAngle[MAXCAND];   //[candSize]
   Float_t         _2DPointingAngle[MAXCAND];   //[candSize]
   Float_t         _3DDecayLengthSignificance[MAXCAND];   //[candSize]
   Float_t         _3DDecayLength[MAXCAND];   //[candSize]
   Float_t         _2DDecayLengthSignificance[MAXCAND];   //[candSize]
   Float_t         _2DDecayLength[MAXCAND];   //[candSize]
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
   Float_t        Trk3DDCA[MAXCAND];   //[candSize]
    Float_t        Trk3DDCAErr[MAXCAND];   //[candSize]
    Float_t        mva[MAXCAND];   //[candSize]

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
   TBranch       *b_Trk3DDCA;   //!
    TBranch       *b_Trk3DDCAErr;   //!
    TBranch       *b_mva;   //!
   simpleDTreeevt* getEventHandle(){ return this;};
    template <typename T>
    void setTree(T *fChain){
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
   fChain->SetBranchAddress("3DCosPointingAngle", _3DCosPointingAngle, &b_3DCosPointingAngle);
   fChain->SetBranchAddress("3DPointingAngle", _3DPointingAngle, &b_3DPointingAngle);
   fChain->SetBranchAddress("2DCosPointingAngle", _2DCosPointingAngle, &b_2DCosPointingAngle);
   fChain->SetBranchAddress("2DPointingAngle", _2DPointingAngle, &b_2DPointingAngle);
   fChain->SetBranchAddress("3DDecayLengthSignificance", _3DDecayLengthSignificance, &b_3DDecayLengthSignificance);
   fChain->SetBranchAddress("3DDecayLength", _3DDecayLength, &b_3DDecayLength);
   fChain->SetBranchAddress("2DDecayLengthSignificance", _2DDecayLengthSignificance, &b_2DDecayLengthSignificance);
   fChain->SetBranchAddress("2DDecayLength", _2DDecayLength, &b_2DDecayLength);
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
    fChain->SetBranchAddress("Trk3DDCA", Trk3DDCA, &b_Trk3DDCA);
    fChain->SetBranchAddress("Trk3DDCAErr", Trk3DDCAErr, &b_Trk3DDCAErr);
    fChain->SetBranchAddress("mva", mva, &b_mva);
        };
    
    };
    struct simpleDTreeflat{
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
    Float_t        Trk3DDCA;   //[candSize]
     Float_t        Trk3DDCAErr;   //[candSize]
     Float_t        mva;   //[candSize]

   bool isMC;
    simpleDTreeflat* getEventHandle(){ return this;}
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
    fChain->SetBranchAddress("Trk3DDCA",&Trk3DDCA);
    fChain->SetBranchAddress("Trk3DDCAErr",&Trk3DDCAErr);
    fChain->SetBranchAddress("mva",&mva);
    };
        template <typename T>
    void setOutputTree(T *fChain){
   fChain->Branch("Ntrkoffline", &Ntrkoffline);
   fChain->Branch("Npixel", &Npixel);
   fChain->Branch("HFsumETPlus", &HFsumETPlus);
   fChain->Branch("HFsumETMinus", &HFsumETMinus);
   fChain->Branch("ZDCPlus", &ZDCPlus);
   fChain->Branch("ZDCMinus", &ZDCMinus);
   fChain->Branch("bestvtxX", &bestvtxX);
   fChain->Branch("bestvtxY", &bestvtxY);
   fChain->Branch("bestvtxZ", &bestvtxZ);
   fChain->Branch("candSize", &candSize);
   fChain->Branch("pT",&pT);
   fChain->Branch("y",&y);
   fChain->Branch("eta",&eta);
   fChain->Branch("phi",&phi);
   fChain->Branch("mass",&mass);
   fChain->Branch("flavor",&flavor);
   fChain->Branch("VtxProb",&VtxProb);
   fChain->Branch("VtxChi2",&VtxChi2);
   fChain->Branch("VtxNDF",&VtxNDF);
   fChain->Branch("3DCosPointingAngle",&_3DCosPointingAngle);
   fChain->Branch("3DPointingAngle",&_3DPointingAngle);
   fChain->Branch("2DCosPointingAngle",&_2DCosPointingAngle);
   fChain->Branch("2DPointingAngle",&_2DPointingAngle);
   fChain->Branch("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
   fChain->Branch("3DDecayLength",&_3DDecayLength);
   fChain->Branch("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
   fChain->Branch("2DDecayLength",&_2DDecayLength);
   fChain->Branch("zDCASignificanceDaugther1",&zDCASignificanceDaugther1);
   fChain->Branch("xyDCASignificanceDaugther1",&xyDCASignificanceDaugther1);
   fChain->Branch("NHitD1",&NHitD1);
   fChain->Branch("HighPurityDaugther1",&HighPurityDaugther1);
   fChain->Branch("pTD1",&pTD1);
   fChain->Branch("pTerrD1",&pTerrD1);
   fChain->Branch("EtaD1",&EtaD1);
   fChain->Branch("PhiD1",&PhiD1);
   fChain->Branch("dedxHarmonic2D1",&dedxHarmonic2D1);
   fChain->Branch("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
   fChain->Branch("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
   fChain->Branch("NHitD2",&NHitD2);
   fChain->Branch("HighPurityDaugther2",&HighPurityDaugther2);
   fChain->Branch("pTD2",&pTD2);
   fChain->Branch("pTerrD2",&pTerrD2);
   fChain->Branch("EtaD2",&EtaD2);
   fChain->Branch("PhiD2",&PhiD2);
   fChain->Branch("dedxHarmonic2D2",&dedxHarmonic2D2);
   fChain->Branch("isMC",&isMC);
   fChain->Branch("Trk3DDCA",&Trk3DDCA);
    fChain->Branch("Trk3DDCAErr",&Trk3DDCAErr);
    fChain->Branch("mva",&mva);
    };
void copyDn( simpleDTreeevt& evt, int idx){
Npixel =evt.Npixel;
HFsumETPlus =evt.HFsumETPlus;
HFsumETMinus =evt.HFsumETMinus;
ZDCPlus =evt.ZDCPlus;
ZDCMinus =evt.ZDCMinus;
bestvtxX =evt.bestvtxX;
bestvtxY =evt.bestvtxY;
bestvtxZ =evt.bestvtxZ;
candSize =evt.candSize;
pT =evt.pT[idx];   //[candSize]
y =evt.y[idx];   //[candSize]
eta =evt.eta[idx];   //[candSize]
phi =evt.phi[idx];   //[candSize]
mass =evt.mass[idx];   //[candSize]
flavor =evt.flavor[idx];   //[candSize]
VtxProb =evt.VtxProb[idx];   //[candSize]
VtxChi2 =evt.VtxChi2[idx];   //[candSize]
VtxNDF =evt.VtxNDF[idx];   //[candSize]
_3DCosPointingAngle =evt._3DCosPointingAngle[idx];   //[candSize]
_3DPointingAngle =evt._3DPointingAngle[idx];   //[candSize]
_2DCosPointingAngle =evt._2DCosPointingAngle[idx];   //[candSize]
_2DPointingAngle =evt._2DPointingAngle[idx];   //[candSize]
_3DDecayLengthSignificance =evt._3DDecayLengthSignificance[idx];   //[candSize]
_3DDecayLength =evt._3DDecayLength[idx];   //[candSize]
_2DDecayLengthSignificance =evt._2DDecayLengthSignificance[idx];   //[candSize]
_2DDecayLength =evt._2DDecayLength[idx];   //[candSize]
zDCASignificanceDaugther1 =evt.zDCASignificanceDaugther1[idx];   //[candSize]
xyDCASignificanceDaugther1 =evt.xyDCASignificanceDaugther1[idx];   //[candSize]
NHitD1 =evt.NHitD1[idx];   //[candSize]
HighPurityDaugther1 =evt.HighPurityDaugther1[idx];   //[candSize]
pTD1 =evt.pTD1[idx];   //[candSize]
pTerrD1 =evt.pTerrD1[idx];   //[candSize]
EtaD1 =evt.EtaD1[idx];   //[candSize]
PhiD1 =evt.PhiD1[idx];   //[candSize]
dedxHarmonic2D1 =evt.dedxHarmonic2D1[idx];   //[candSize]
zDCASignificanceDaugther2 =evt.zDCASignificanceDaugther2[idx];   //[candSize]
xyDCASignificanceDaugther2 =evt.xyDCASignificanceDaugther2[idx];   //[candSize]
NHitD2 =evt.NHitD2[idx];   //[candSize]
HighPurityDaugther2 =evt.HighPurityDaugther2[idx];   //[candSize]
pTD2 =evt.pTD2[idx];   //[candSize]
pTerrD2 =evt.pTerrD2[idx];   //[candSize]
EtaD2 =evt.EtaD2[idx];   //[candSize]
PhiD2 =evt.PhiD2[idx];   //[candSize]
dedxHarmonic2D2 =evt.dedxHarmonic2D2[idx];   //[candSize]
Trk3DDCA =evt.Trk3DDCA[idx];   //[candSize]
Trk3DDCAErr =evt.Trk3DDCAErr[idx];   //[candSize]
mva=evt.mva[idx];
// isMC=isMC;
        };
    };
}
#endif
