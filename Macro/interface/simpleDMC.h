#ifndef __DATAFORMAT_D_MC__
#define __DATAFORMAT_D_MC__
// #include "src/common/interface/commonHeader.hxx"
#include <TTree.h>
#include <TBranch.h>
#include <iostream>
const int __MAXCAND_MC__ = 30000;
//const int __MAXCAND_MC__ = 1000;

namespace DataFormat{
    struct simpleDMCTreeevtMVA{
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
   Int_t         matchGen_D0charge[MAXCAND];   //[candSize]
   Int_t           matchGen_D0pdgId[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_pT[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_eta[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_phi[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_mass[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_y[MAXCAND];   //[candSize]
   Int_t           matchGen_D0Dau1_charge[MAXCAND];   //[candSize]
   Int_t           matchGen_D0Dau1_pdgId[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_pT[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_eta[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_phi[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_mass[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_y[MAXCAND];   //[candSize]
   Int_t           matchGen_D0Dau2_charge[MAXCAND];   //[candSize]
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
   Float_t         dca3D[MAXCAND];   //[candSize]

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
   simpleDMCTreeevtMVA* getEventHandle(){ return this;}
    template <typename T>
        void setTree(T *fChain){
   fChain->SetBranchAddress("Ntrkoffline", &Ntrkoffline, &b_Ntrkoffline);
   fChain->SetBranchAddress("Npixel", &Npixel, &b_Npixel);
   fChain->SetBranchAddress("HFsumETPlus", &HFsumETPlus, &b_HFsumETPlus);
   fChain->SetBranchAddress("HFsumETMinus", &HFsumETMinus, &b_HFsumETMinus);
   fChain->SetBranchAddress("candSize", &candSize, &b_candSize);
   fChain->SetBranchAddress("pT", pT, &b_pT);
   fChain->SetBranchAddress("y", y, &b_y);
   fChain->SetBranchAddress("eta", eta, &b_eta);
   fChain->SetBranchAddress("phi", phi, &b_phi);
   fChain->SetBranchAddress("mass", mass, &b_mass);
   fChain->SetBranchAddress("VtxProb", VtxProb, &b_VtxProb);
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
   fChain->SetBranchAddress("Trk3DDCA", Trk3DDCA, &b_Trk3DDCA);
    fChain->SetBranchAddress("Trk3DDCAErr", Trk3DDCAErr, &b_Trk3DDCAErr);
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
   Int_t         matchGen_D0charge[MAXCAND];   //[candSize]
   Int_t           matchGen_D0pdgId[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_pT[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_eta[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_phi[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_mass[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau1_y[MAXCAND];   //[candSize]
   Int_t           matchGen_D0Dau1_charge[MAXCAND];   //[candSize]
   Int_t           matchGen_D0Dau1_pdgId[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_pT[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_eta[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_phi[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_mass[MAXCAND];   //[candSize]
   Float_t         matchGen_D0Dau2_y[MAXCAND];   //[candSize]
   Int_t           matchGen_D0Dau2_charge[MAXCAND];   //[candSize]
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
   Float_t         dca3D[MAXCAND];   //[candSize]

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
    TBranch        *b_dca3D;   //!
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
    fChain->SetBranchAddress("dca3D", dca3D, &b_dca3D);
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
   Int_t         matchGen_D0charge;   //[candSize]
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
   Int_t         matchGen_D0Dau2_charge;   //[candSize]
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
    Float_t        dca3D;   //[candSize]
    Short_t centrality;
    Float_t ncoll;
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
    fChain->SetBranchAddress("dca3D",&dca3D);
    fChain->SetBranchAddress("centrality",&centrality);
    fChain->SetBranchAddress("Ncoll",&ncoll);
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
   fChain->SetBranchAddress("centrality",&centrality);
   fChain->SetBranchAddress("Ncoll",&ncoll);
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
    fChain->Branch("dca3D",&dca3D);
   
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
void copyDn( simpleDMCTreeevtMVA& evt, int idx){
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
dca3D =evt.dca3D[idx];   //[candSize]

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
    Float_t       dca3D[MAXCAND];   //[candSize]

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
    TBranch       *b_dca3D;   //!
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
    fChain->SetBranchAddress("dca3D", dca3D, &b_dca3D);
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
     Float_t       dca3D;   //[candSize]

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
   fChain->Branch("isMC",&isMC);
    fChain->SetBranchAddress("dca3D",&dca3D);
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
    fChain->Branch("dca3D",&dca3D);
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
dca3D=evt.dca3D[idx];
// isMC=isMC;
        };
    };
    // simpleDStarDataTreeevt 구조체에 event plane 변수들 추가
    struct simpleDStarDataTreeevt{
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
   Float_t         massDaugther1[MAXCAND];   //[candSize]
   Float_t         pTD1[MAXCAND];   //[candSize]
   Float_t         EtaD1[MAXCAND];   //[candSize]
   Float_t         PhiD1[MAXCAND];   //[candSize]
   Float_t         VtxProbDaugther1[MAXCAND];   //[candSize]
   Float_t         VtxChi2Daugther1[MAXCAND];   //[candSize]
   Float_t         VtxNDFDaugther1[MAXCAND];   //[candSize]
   Float_t         _3DCosPointingAngleDaugther1[MAXCAND];   //[candSize]
   Float_t         _3DPointingAngleDaugther1[MAXCAND];   //[candSize]
   Float_t         _2DCosPointingAngleDaugther1[MAXCAND];   //[candSize]
   Float_t         _2DPointingAngleDaugther1[MAXCAND];   //[candSize]
   Float_t         _3DDecayLengthSignificanceDaugther1[MAXCAND];   //[candSize]
   Float_t         _3DDecayLengthDaugther1[MAXCAND];   //[candSize]
   Float_t         _3DDecayLengthErrorDaugther1[MAXCAND];   //[candSize]
   Float_t         _2DDecayLengthSignificanceDaugther1[MAXCAND];   //[candSize]
   Float_t         zDCASignificanceDaugther2[MAXCAND];   //[candSize]
   Float_t         xyDCASignificanceDaugther2[MAXCAND];   //[candSize]
   Float_t         NHitD2[MAXCAND];   //[candSize]
   Bool_t          HighPurityDaugther2[MAXCAND];   //[candSize]
   Float_t         pTD2[MAXCAND];   //[candSize]
   Float_t         EtaD2[MAXCAND];   //[candSize]
   Float_t         PhiD2[MAXCAND];   //[candSize]
   Float_t         pTerrD1[MAXCAND];   //[candSize]
   Float_t         pTerrD2[MAXCAND];   //[candSize]
   Float_t         dedxHarmonic2D2[MAXCAND];   //[candSize]
   Float_t         zDCASignificanceGrandDaugther1[MAXCAND];   //[candSize]
   Float_t         zDCASignificanceGrandDaugther2[MAXCAND];   //[candSize]
   Float_t         xyDCASignificanceGrandDaugther1[MAXCAND];   //[candSize]
   Float_t         xyDCASignificanceGrandDaugther2[MAXCAND];   //[candSize]
   Float_t         NHitGrandD1[MAXCAND];   //[candSize]
   Float_t         NHitGrandD2[MAXCAND];   //[candSize]
   Bool_t          HighPurityGrandDaugther1[MAXCAND];   //[candSize]
   Bool_t          HighPurityGrandDaugther2[MAXCAND];   //[candSize]
   Float_t         pTGrandD1[MAXCAND];   //[candSize]
   Float_t         pTGrandD2[MAXCAND];   //[candSize]
   Float_t         pTerrGrandD1[MAXCAND];   //[candSize]
   Float_t         pTerrGrandD2[MAXCAND];   //[candSize]
   Float_t         EtaGrandD1[MAXCAND];   //[candSize]
   Float_t         EtaGrandD2[MAXCAND];   //[candSize]
   Float_t         dedxHarmonic2GrandD1[MAXCAND];   //[candSize]
   Float_t         dedxHarmonic2GrandD2[MAXCAND];   //[candSize]
   Float_t         Trk3DDCA[MAXCAND];   //[candSize]
   Float_t         Trk3DDCAErr[MAXCAND];   //[candSize]
   Float_t         mva[MAXCAND];   //[candSize]
   Float_t        dca3D[MAXCAND];   //[candSize]

    // Event plane 관련 변수들 추가
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
    Float_t         dca3DErr[MAXCAND];   //[candSize]

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
    TBranch        *b_Trk3DDCA;   //!
    TBranch        *b_Trk3DDCAErr;   //!
    TBranch        *b_mva;   //!
    TBranch        *b_dca3D;   //!

    // 대응하는 TBranch 포인터들 추가
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
    TBranch        *b_dca3DErr;   //!

    simpleDStarDataTreeevt* getEventHandle(){ return this;}
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
   fChain->SetBranchAddress("massDaugther1", massDaugther1, &b_massDaugther1);
   fChain->SetBranchAddress("pTD1", pTD1, &b_pTD1);
   fChain->SetBranchAddress("EtaD1", EtaD1, &b_EtaD1);
   fChain->SetBranchAddress("PhiD1", PhiD1, &b_PhiD1);
   fChain->SetBranchAddress("VtxProbDaugther1", VtxProbDaugther1, &b_VtxProbDaugther1);
   fChain->SetBranchAddress("VtxChi2Daugther1", VtxChi2Daugther1, &b_VtxChi2Daugther1);
   fChain->SetBranchAddress("VtxNDFDaugther1", VtxNDFDaugther1, &b_VtxNDFDaugther1);
   fChain->SetBranchAddress("3DCosPointingAngleDaugther1", _3DCosPointingAngleDaugther1, &b_3DCosPointingAngleDaugther1);
   fChain->SetBranchAddress("3DPointingAngleDaugther1", _3DPointingAngleDaugther1, &b_3DPointingAngleDaugther1);
   fChain->SetBranchAddress("2DCosPointingAngleDaugther1", _2DCosPointingAngleDaugther1, &b_2DCosPointingAngleDaugther1);
   fChain->SetBranchAddress("2DPointingAngleDaugther1", _2DPointingAngleDaugther1, &b_2DPointingAngleDaugther1);
   fChain->SetBranchAddress("3DDecayLengthSignificanceDaugther1", _3DDecayLengthSignificanceDaugther1, &b_3DDecayLengthSignificanceDaugther1);
   fChain->SetBranchAddress("3DDecayLengthDaugther1", _3DDecayLengthDaugther1, &b_3DDecayLengthDaugther1);
   fChain->SetBranchAddress("3DDecayLengthErrorDaugther1", _3DDecayLengthErrorDaugther1, &b_3DDecayLengthErrorDaugther1);
   fChain->SetBranchAddress("2DDecayLengthSignificanceDaugther1", _2DDecayLengthSignificanceDaugther1, &b_2DDecayLengthSignificanceDaugther1);
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
   fChain->SetBranchAddress("Trk3DDCA", Trk3DDCA, &b_Trk3DDCA);
    fChain->SetBranchAddress("Trk3DDCAErr", Trk3DDCAErr, &b_Trk3DDCAErr);
    fChain->SetBranchAddress("mva", mva, &b_mva);
    fChain->SetBranchAddress("dca3D", dca3D, &b_dca3D);

        // Event plane 관련 branch 추가
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
        fChain->SetBranchAddress("dca3DErr", dca3DErr, &b_dca3DErr);
        };

    // simpleDStarDataTreeevt* getEventHandle(){ return this;}
//     template <typename T>
//         void setTree(T *fChain){
//    fChain->SetBranchAddress("Ntrkoffline", &Ntrkoffline, &b_Ntrkoffline);
//    fChain->SetBranchAddress("Npixel", &Npixel, &b_Npixel);
//    fChain->SetBranchAddress("HFsumETPlus", &HFsumETPlus, &b_HFsumETPlus);
//    fChain->SetBranchAddress("HFsumETMinus", &HFsumETMinus, &b_HFsumETMinus);
//    fChain->SetBranchAddress("ZDCPlus", &ZDCPlus, &b_ZDCPlus);
//    fChain->SetBranchAddress("ZDCMinus", &ZDCMinus, &b_ZDCMinus);
//    fChain->SetBranchAddress("bestvtxX", &bestvtxX, &b_bestvtxX);
//    fChain->SetBranchAddress("bestvtxY", &bestvtxY, &b_bestvtxY);
//    fChain->SetBranchAddress("bestvtxZ", &bestvtxZ, &b_bestvtxZ);
//    fChain->SetBranchAddress("candSize", &candSize, &b_candSize);
//    fChain->SetBranchAddress("pT", pT, &b_pT);
//    fChain->SetBranchAddress("y", y, &b_y);
//    fChain->SetBranchAddress("eta", eta, &b_eta);
//    fChain->SetBranchAddress("phi", phi, &b_phi);
//    fChain->SetBranchAddress("mass", mass, &b_mass);
//    fChain->SetBranchAddress("flavor", flavor, &b_flavor);
//    fChain->SetBranchAddress("VtxProb", VtxProb, &b_VtxProb);
//    fChain->SetBranchAddress("VtxChi2", VtxChi2, &b_VtxChi2);
//    fChain->SetBranchAddress("VtxNDF", VtxNDF, &b_VtxNDF);
//    fChain->SetBranchAddress("3DCosPointingAngle", _3DCosPointingAngle, &b_3DCosPointingAngle);
//    fChain->SetBranchAddress("3DPointingAngle", _3DPointingAngle, &b_3DPointingAngle);
//    fChain->SetBranchAddress("2DCosPointingAngle", _2DCosPointingAngle, &b_2DCosPointingAngle);
//    fChain->SetBranchAddress("2DPointingAngle", _2DPointingAngle, &b_2DPointingAngle);
//    fChain->SetBranchAddress("3DDecayLengthSignificance", _3DDecayLengthSignificance, &b_3DDecayLengthSignificance);
//    fChain->SetBranchAddress("3DDecayLength", _3DDecayLength, &b_3DDecayLength);
//    fChain->SetBranchAddress("2DDecayLengthSignificance", _2DDecayLengthSignificance, &b_2DDecayLengthSignificance);
//    fChain->SetBranchAddress("2DDecayLength", _2DDecayLength, &b_2DDecayLength);
//    fChain->SetBranchAddress("massDaugther1", massDaugther1, &b_massDaugther1);
//    fChain->SetBranchAddress("pTD1", pTD1, &b_pTD1);
//    fChain->SetBranchAddress("EtaD1", EtaD1, &b_EtaD1);
//    fChain->SetBranchAddress("PhiD1", PhiD1, &b_PhiD1);
//    fChain->SetBranchAddress("VtxProbDaugther1", VtxProbDaugther1, &b_VtxProbDaugther1);
//    fChain->SetBranchAddress("VtxChi2Daugther1", VtxChi2Daugther1, &b_VtxChi2Daugther1);
//    fChain->SetBranchAddress("VtxNDFDaugther1", VtxNDFDaugther1, &b_VtxNDFDaugther1);
//    fChain->SetBranchAddress("3DCosPointingAngleDaugther1", _3DCosPointingAngleDaugther1, &b_3DCosPointingAngleDaugther1);
//    fChain->SetBranchAddress("3DPointingAngleDaugther1", _3DPointingAngleDaugther1, &b_3DPointingAngleDaugther1);
//    fChain->SetBranchAddress("2DCosPointingAngleDaugther1", _2DCosPointingAngleDaugther1, &b_2DCosPointingAngleDaugther1);
//    fChain->SetBranchAddress("2DPointingAngleDaugther1", _2DPointingAngleDaugther1, &b_2DPointingAngleDaugther1);
//    fChain->SetBranchAddress("3DDecayLengthSignificanceDaugther1", _3DDecayLengthSignificanceDaugther1, &b_3DDecayLengthSignificanceDaugther1);
//    fChain->SetBranchAddress("3DDecayLengthDaugther1", _3DDecayLengthDaugther1, &b_3DDecayLengthDaugther1);
//    fChain->SetBranchAddress("3DDecayLengthErrorDaugther1", _3DDecayLengthErrorDaugther1, &b_3DDecayLengthErrorDaugther1);
//    fChain->SetBranchAddress("2DDecayLengthSignificanceDaugther1", _2DDecayLengthSignificanceDaugther1, &b_2DDecayLengthSignificanceDaugther1);
//    fChain->SetBranchAddress("zDCASignificanceDaugther2", zDCASignificanceDaugther2, &b_zDCASignificanceDaugther2);
//    fChain->SetBranchAddress("xyDCASignificanceDaugther2", xyDCASignificanceDaugther2, &b_xyDCASignificanceDaugther2);
//    fChain->SetBranchAddress("NHitD2", NHitD2, &b_NHitD2);
//    fChain->SetBranchAddress("HighPurityDaugther2", HighPurityDaugther2, &b_HighPurityDaugther2);
//    fChain->SetBranchAddress("pTD2", pTD2, &b_pTD2);
//    fChain->SetBranchAddress("EtaD2", EtaD2, &b_EtaD2);
//    fChain->SetBranchAddress("PhiD2", PhiD2, &b_PhiD2);
//    fChain->SetBranchAddress("pTerrD1", pTerrD1, &b_pTerrD1);
//    fChain->SetBranchAddress("pTerrD2", pTerrD2, &b_pTerrD2);
//    fChain->SetBranchAddress("dedxHarmonic2D2", dedxHarmonic2D2, &b_dedxHarmonic2D2);
//    fChain->SetBranchAddress("zDCASignificanceGrandDaugther1", zDCASignificanceGrandDaugther1, &b_zDCASignificanceGrandDaugther1);
//    fChain->SetBranchAddress("zDCASignificanceGrandDaugther2", zDCASignificanceGrandDaugther2, &b_zDCASignificanceGrandDaugther2);
//    fChain->SetBranchAddress("xyDCASignificanceGrandDaugther1", xyDCASignificanceGrandDaugther1, &b_xyDCASignificanceGrandDaugther1);
//    fChain->SetBranchAddress("xyDCASignificanceGrandDaugther2", xyDCASignificanceGrandDaugther2, &b_xyDCASignificanceGrandDaugther2);
//    fChain->SetBranchAddress("NHitGrandD1", NHitGrandD1, &b_NHitGrandD1);
//    fChain->SetBranchAddress("NHitGrandD2", NHitGrandD2, &b_NHitGrandD2);
//    fChain->SetBranchAddress("HighPurityGrandDaugther1", HighPurityGrandDaugther1, &b_HighPurityGrandDaugther1);
//    fChain->SetBranchAddress("HighPurityGrandDaugther2", HighPurityGrandDaugther2, &b_HighPurityGrandDaugther2);
//    fChain->SetBranchAddress("pTGrandD1", pTGrandD1, &b_pTGrandD1);
//    fChain->SetBranchAddress("pTGrandD2", pTGrandD2, &b_pTGrandD2);
//    fChain->SetBranchAddress("pTerrGrandD1", pTerrGrandD1, &b_pTerrGrandD1);
//    fChain->SetBranchAddress("pTerrGrandD2", pTerrGrandD2, &b_pTerrGrandD2);
//    fChain->SetBranchAddress("EtaGrandD1", EtaGrandD1, &b_EtaGrandD1);
//    fChain->SetBranchAddress("EtaGrandD2", EtaGrandD2, &b_EtaGrandD2);
//    fChain->SetBranchAddress("dedxHarmonic2GrandD1", dedxHarmonic2GrandD1, &b_dedxHarmonic2GrandD1);
//    fChain->SetBranchAddress("dedxHarmonic2GrandD2", dedxHarmonic2GrandD2, &b_dedxHarmonic2GrandD2);
//    fChain->SetBranchAddress("Trk3DDCA", Trk3DDCA, &b_Trk3DDCA);
//     fChain->SetBranchAddress("Trk3DDCAErr", Trk3DDCAErr, &b_Trk3DDCAErr);
//     fChain->SetBranchAddress("mva", mva, &b_mva);
//     fChain->SetBranchAddress("dca3D", dca3D, &b_dca3D);
//         };
    };
    struct simpleDStarMCTreeevt{
        protected:
            static const int MAXCAND = __MAXCAND_MC__;
        public:
            int   Ntrkoffline;
            int Npixel;
            float HFsumETPlus;
            float HFsumETMinus;
            float ZDCPlus;
            float ZDCMinus;
            float bestvtxX;
            float bestvtxY;
            float bestvtxZ;
            int candSize;
            float pT[MAXCAND];
            float y[MAXCAND];
            float eta[MAXCAND];
            float phi[MAXCAND];
            float mass[MAXCAND];
            float flavor[MAXCAND];
            float VtxProb[MAXCAND];
            float VtxChi2[MAXCAND];
            float VtxNDF[MAXCAND];
            float _3DCosPointingAngle[MAXCAND];
            float _3DPointingAngle[MAXCAND];
            float _2DCosPointingAngle[MAXCAND];
            float _2DPointingAngle[MAXCAND];
            float _3DDecayLengthSignificance[MAXCAND];
            float _3DDecayLength[MAXCAND];
            float _2DDecayLengthSignificance[MAXCAND];
            float _2DDecayLength[MAXCAND];
            bool isSwap[MAXCAND];
            int idmom_reco[MAXCAND];
            int idBAnc_reco[MAXCAND];
            bool matchGEN[MAXCAND];
            float matchGen3DPointingAngle[MAXCAND];
            float matchGen2DPointingAngle[MAXCAND];
            float matchGen3DDecayLength[MAXCAND];
            float matchGen2DDecayLength[MAXCAND];
            float matchgen_D0pT[MAXCAND];
            float matchgen_D0eta[MAXCAND];
            float matchgen_D0phi[MAXCAND];
            float matchgen_D0mass[MAXCAND];
            float matchgen_D0y[MAXCAND];
            int matchgen_D0charge[MAXCAND];
            float matchgen_D0pdgId[MAXCAND];
            float matchGen_D0pT[MAXCAND];
            float matchGen_D0eta[MAXCAND];
            float matchGen_D0phi[MAXCAND];
            float matchGen_D0mass[MAXCAND];
            float matchGen_D0y[MAXCAND];
            int matchGen_D0charge[MAXCAND];
            float matchGen_D0pdgId[MAXCAND];
            float matchGen_D0Dau1_pT[MAXCAND];
            float matchGen_D0Dau1_eta[MAXCAND];
            float matchGen_D0Dau1_phi[MAXCAND];
            float matchGen_D0Dau1_mass[MAXCAND];
            float matchGen_D0Dau1_y[MAXCAND];
            float matchGen_D0Dau1_charge[MAXCAND];
            float matchGen_D0Dau1_pdgId[MAXCAND];
            float matchGen_D0Dau2_pT[MAXCAND];
            float matchGen_D0Dau2_eta[MAXCAND];
            float matchGen_D0Dau2_phi[MAXCAND];
            float matchGen_D0Dau2_mass[MAXCAND];
            float matchGen_D0Dau2_y[MAXCAND];
            float matchGen_D0Dau2_charge[MAXCAND];
            float matchGen_D0Dau2_pdgId[MAXCAND];
            float matchGen_D1pT[MAXCAND];
            float matchGen_D1eta[MAXCAND];
            float matchGen_D1phi[MAXCAND];
            float matchGen_D1mass[MAXCAND];
            float matchGen_D1y[MAXCAND];
            float matchGen_D1charge[MAXCAND];
            float matchGen_D1pdgId[MAXCAND];
            float matchGen_D1decayLength2D_[MAXCAND];
            float matchGen_D1decayLength3D_[MAXCAND];
            float matchGen_D1angle2D_[MAXCAND];
            float matchGen_D1angle3D_[MAXCAND];
            Int_t matchGen_D1ancestorId_[MAXCAND];
            Int_t matchGen_D1ancestorFlavor_[MAXCAND];
            float massDaugther1[MAXCAND];
            float pTD1[MAXCAND];
            float EtaD1[MAXCAND];
            float PhiD1[MAXCAND];
            float VtxProbDaugther1[MAXCAND];
            float VtxChi2Daugther1[MAXCAND];
            float VtxNDFDaugther1[MAXCAND];
            float _3DCosPointingAngleDaugther1[MAXCAND];
            float _3DPointingAngleDaugther1[MAXCAND];
            float _2DCosPointingAngleDaugther1[MAXCAND];
            float _2DPointingAngleDaugther1[MAXCAND];
            float _3DDecayLengthSignificanceDaugther1[MAXCAND];
            float _3DDecayLengthDaugther1[MAXCAND];
            float _3DDecayLengthErrorDaugther1[MAXCAND];
            float _2DDecayLengthSignificanceDaugther1[MAXCAND];
            float zDCASignificanceDaugther2[MAXCAND];
            float xyDCASignificanceDaugther2[MAXCAND];
            float NHitD2[MAXCAND];
            bool HighPurityDaugther2[MAXCAND];
            float pTD2[MAXCAND];
            float EtaD2[MAXCAND];
            float PhiD2[MAXCAND];
            float pTerrD1[MAXCAND];
            float pTerrD2[MAXCAND];
            float dedxHarmonic2D2[MAXCAND];
            float zDCASignificanceGrandDaugther1[MAXCAND];
            float zDCASignificanceGrandDaugther2[MAXCAND];
            float xyDCASignificanceGrandDaugther1[MAXCAND];
            float xyDCASignificanceGrandDaugther2[MAXCAND];
            float NHitGrandD1[MAXCAND];
            float NHitGrandD2[MAXCAND];
            bool HighPurityGrandDaugther1[MAXCAND];
            bool HighPurityGrandDaugther2[MAXCAND];
            float pTGrandD1[MAXCAND];
            float pTGrandD2[MAXCAND];
            float pTerrGrandD1[MAXCAND];
            float pTerrGrandD2[MAXCAND];
            float EtaGrandD1[MAXCAND];
            float EtaGrandD2[MAXCAND];
            float dedxHarmonic2GrandD1[MAXCAND];
            float dedxHarmonic2GrandD2[MAXCAND];
	    float mva[MAXCAND];
            float Trk3DDCA[MAXCAND];
            float Trk3DDCAErr[MAXCAND];
            float dca3D[MAXCAND];
            Int_t           candSize_gen;
   Float_t         gen_mass[MAXCAND];   //[candSize_gen]
   Float_t         gen_pT[MAXCAND];   //[candSize_gen]
   Float_t         gen_eta[MAXCAND];   //[candSize_gen]
   Float_t         gen_phi[MAXCAND];   //[candSize_gen]
   Float_t         gen_y[MAXCAND];   //[candSize_gen]
   Int_t           gen_status[MAXCAND];   //[candSize_gen]
   Int_t           gen_MotherID[MAXCAND];   //[candSize_gen]
   Int_t           gen_DauID1[MAXCAND];   //[candSize_gen]
   Int_t           gen_DauID2[MAXCAND];   //[candSize_gen]
   Int_t           gen_DauID3[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0pT[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0eta[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0phi[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0mass[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0y[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0charge[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0pdgId[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0ancestorId_[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0ancestorFlavor_[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau1_pT[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau1_eta[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau1_phi[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau1_mass[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau1_y[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0Dau1_charge[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0Dau1_pdgId[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau2_pT[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau2_eta[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau2_phi[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau2_mass[MAXCAND];   //[candSize_gen]
   Float_t         gen_D0Dau2_y[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0Dau2_charge[MAXCAND];   //[candSize_gen]
   Int_t         gen_D0Dau2_pdgId[MAXCAND];   //[candSize_gen]
   Float_t         gen_D1pT[MAXCAND];   //[candSize_gen]
   Float_t         gen_D1eta[MAXCAND];   //[candSize_gen]
   Float_t         gen_D1phi[MAXCAND];   //[candSize_gen]
   Float_t         gen_D1mass[MAXCAND];   //[candSize_gen]
   Float_t         gen_D1y[MAXCAND];   //[candSize_gen]
   Int_t         gen_D1charge[MAXCAND];   //[candSize_gen]
   Int_t         gen_D1pdgId[MAXCAND]; 
  TBranch        *b_candSize_gen;   //!
   TBranch        *b_gen_mass;   //!
   TBranch        *b_gen_pT;   //!
   TBranch        *b_gen_eta;   //!
   TBranch        *b_gen_phi;   //!
   TBranch        *b_gen_y;   //!
   TBranch        *b_gen_status;   //!
   TBranch        *b_gen_MotherID;   //!
   TBranch        *b_gen_DauID1;   //!
   TBranch        *b_gen_DauID2;   //!
   TBranch        *b_gen_DauID3;   //!
   TBranch        *b_gen_D0pT;   //!
   TBranch        *b_gen_D0eta;   //!
   TBranch        *b_gen_D0phi;   //!
   TBranch        *b_gen_D0mass;   //!
   TBranch        *b_gen_D0y;   //!
   TBranch        *b_gen_D0charge;   //!
   TBranch        *b_gen_D0pdgId;   //!
   TBranch        *b_gen_D0ancestorId;   //!
   TBranch        *b_gen_D0ancestorFlavor;   //!
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
   TBranch        *b_gen_D1pT;   //!
   TBranch        *b_gen_D1eta;   //!
   TBranch        *b_gen_D1phi;   //!
   TBranch        *b_gen_D1mass;   //!
   TBranch        *b_gen_D1y;   //!
   TBranch        *b_gen_D1charge;   //!
   TBranch        *b_gen_D1pdgId;   //!

            //bool isData;
            // int candSize_gen;
            // float gen_pT[MAXCAND];
            // float gen_eat[MAXCAND];
            // float gen_y[MAXCAND];
            // int gen_status[MAXCAND];
            // int gen_MotherID[MAXCAND];
            // int gen_DauID1[MAXCAND];
            // int gen_DauID2[MAXCAND];
            // int gen_DauID3[MAXCAND];
            // float gen_D0pT[MAXCAND];
            // float gen_D0eta[MAXCAND];
            // float gen_D0phi[MAXCAND];
            // float gen_D0mass[MAXCAND];
            // float gen_D0y[MAXCAND];
            // float gen_D0charge[MAXCAND];
            // float gen_D0pdgId[MAXCAND];
            // float gen_D0Dau1_pT[MAXCAND];
            // float gen_D0Dau1_eta[MAXCAND];
            // float gen_D0Dau1_phi[MAXCAND];
            // float gen_D0Dau1_mass[MAXCAND];
            // float gen_D0Dau1_y[MAXCAND];
            // float gen_D0Dau1_charge[MAXCAND];
            // float gen_D0Dau1_pdgId[MAXCAND];
            // float gen_D0Dau2_pT[MAXCAND];
            // float gen_D0Dau2_eta[MAXCAND];
            // float gen_D0Dau2_phi[MAXCAND];
            // float gen_D0Dau2_mass[MAXCAND];
            // float gen_D0Dau2_y[MAXCAND];
            // float gen_D0Dau2_charge[MAXCAND];
            // float gen_D0Dau2_pdgId[MAXCAND];
            // float gen_D1pT[MAXCAND];
            // float gen_D1eta[MAXCAND];
            // float gen_D1phi[MAXCAND];
            // float gen_D1mass[MAXCAND];
            // float gen_D1y[MAXCAND]22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222;
            // float gen_D1charge[MAXCAND];
            // float gen_D1pdgI[MAXCAND];
            simpleDStarMCTreeevt* getEventHandle(){ return this;}
            template <typename T>
            void setTree(T *t){
            t->SetBranchAddress("Ntrkoffline",&Ntrkoffline);
            t->SetBranchAddress("Npixel",&Npixel);
            t->SetBranchAddress("HFsumETPlus",&HFsumETPlus);
            t->SetBranchAddress("HFsumETMinus",&HFsumETMinus);
            t->SetBranchAddress("ZDCPlus",&ZDCPlus);
            t->SetBranchAddress("ZDCMinus",&ZDCMinus);
            t->SetBranchAddress("bestvtxX",&bestvtxX);
            t->SetBranchAddress("bestvtxY",&bestvtxY);
            t->SetBranchAddress("bestvtxZ",&bestvtxZ);
            t->SetBranchAddress("candSize",&candSize);
            t->SetBranchAddress("pT",&pT);
            t->SetBranchAddress("y",&y);
            t->SetBranchAddress("eta",&eta);
            t->SetBranchAddress("phi",&phi);
            t->SetBranchAddress("mass",&mass);
            t->SetBranchAddress("flavor",&flavor);
            t->SetBranchAddress("VtxProb",&VtxProb);
            t->SetBranchAddress("VtxChi2",&VtxChi2);
            t->SetBranchAddress("VtxNDF",&VtxNDF);
            t->SetBranchAddress("3DCosPointingAngle",&_3DCosPointingAngle);
            t->SetBranchAddress("3DPointingAngle",&_3DPointingAngle);
            t->SetBranchAddress("2DCosPointingAngle",&_2DCosPointingAngle);
            t->SetBranchAddress("2DPointingAngle",&_2DPointingAngle);
            t->SetBranchAddress("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
            t->SetBranchAddress("3DDecayLength",&_3DDecayLength);
            t->SetBranchAddress("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
            t->SetBranchAddress("2DDecayLength",&_2DDecayLength);
            t->SetBranchAddress("isSwap",&isSwap);
            t->SetBranchAddress("idmom_reco",&idmom_reco);
            t->SetBranchAddress("idBAnc_reco",&idBAnc_reco);
            t->SetBranchAddress("matchGEN",&matchGEN);
            t->SetBranchAddress("matchGen3DPointingAngle",&matchGen3DPointingAngle);
            t->SetBranchAddress("matchGen2DPointingAngle",&matchGen2DPointingAngle);
            t->SetBranchAddress("matchGen3DDecayLength",&matchGen3DDecayLength);
            t->SetBranchAddress("matchGen2DDecayLength",&matchGen2DDecayLength);
            t->SetBranchAddress("matchgen_D0pT",&matchgen_D0pT);
            t->SetBranchAddress("matchgen_D0eta",&matchgen_D0eta);
            t->SetBranchAddress("matchgen_D0phi",&matchgen_D0phi);
            t->SetBranchAddress("matchgen_D0mass",&matchgen_D0mass);
            t->SetBranchAddress("matchgen_D0y",&matchgen_D0y);
            t->SetBranchAddress("matchgen_D0charge",&matchgen_D0charge);
            t->SetBranchAddress("matchgen_D0pdgId",&matchgen_D0pdgId);
            t->SetBranchAddress("matchGen_D0pT",&matchGen_D0pT);
            t->SetBranchAddress("matchGen_D0eta",&matchGen_D0eta);
            t->SetBranchAddress("matchGen_D0phi",&matchGen_D0phi);
            t->SetBranchAddress("matchGen_D0mass",&matchGen_D0mass);
            t->SetBranchAddress("matchGen_D0y",&matchGen_D0y);
            t->SetBranchAddress("matchGen_D0charge",&matchGen_D0charge);
            t->SetBranchAddress("matchGen_D0pdgId",&matchGen_D0pdgId);
            t->SetBranchAddress("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT);
            t->SetBranchAddress("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta);
            t->SetBranchAddress("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi);
            t->SetBranchAddress("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass);
            t->SetBranchAddress("matchGen_D0Dau1_y",&matchGen_D0Dau1_y);
            t->SetBranchAddress("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge);
            t->SetBranchAddress("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId);
            t->SetBranchAddress("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT);
            t->SetBranchAddress("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta);
            t->SetBranchAddress("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi);
            t->SetBranchAddress("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass);
            t->SetBranchAddress("matchGen_D0Dau2_y",&matchGen_D0Dau2_y);
            t->SetBranchAddress("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge);
            t->SetBranchAddress("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId);
            t->SetBranchAddress("matchGen_D1pT",&matchGen_D1pT);
            t->SetBranchAddress("matchGen_D1eta",&matchGen_D1eta);
            t->SetBranchAddress("matchGen_D1phi",&matchGen_D1phi);
            t->SetBranchAddress("matchGen_D1mass",&matchGen_D1mass);
            t->SetBranchAddress("matchGen_D1y",&matchGen_D1y);
            t->SetBranchAddress("matchGen_D1charge",&matchGen_D1charge);
            t->SetBranchAddress("matchGen_D1pdgId",&matchGen_D1pdgId);
            t->SetBranchAddress("matchGen_D1decayLength2D_",&matchGen_D1decayLength2D_);
            t->SetBranchAddress("matchGen_D1decayLength3D_",&matchGen_D1decayLength3D_);
            t->SetBranchAddress("matchGen_D1angle2D_",&matchGen_D1angle2D_);
            t->SetBranchAddress("matchGen_D1angle3D_",&matchGen_D1angle3D_);
            t->SetBranchAddress("matchGen_D1ancestorId_",&matchGen_D1ancestorId_);
            t->SetBranchAddress("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_);
            t->SetBranchAddress("massDaugther1",&massDaugther1);
            t->SetBranchAddress("pTD1",&pTD1);
            t->SetBranchAddress("EtaD1",&EtaD1);
            t->SetBranchAddress("PhiD1",&PhiD1);
            t->SetBranchAddress("VtxProbDaugther1",&VtxProbDaugther1);
            t->SetBranchAddress("VtxChi2Daugther1",&VtxChi2Daugther1);
            t->SetBranchAddress("VtxNDFDaugther1",&VtxNDFDaugther1);
            t->SetBranchAddress("3DCosPointingAngleDaugther1",&_3DCosPointingAngleDaugther1);
            t->SetBranchAddress("3DPointingAngleDaugther1",&_3DPointingAngleDaugther1);
            t->SetBranchAddress("2DCosPointingAngleDaugther1",&_2DCosPointingAngleDaugther1);
            t->SetBranchAddress("2DPointingAngleDaugther1",&_2DPointingAngleDaugther1);
            t->SetBranchAddress("3DDecayLengthSignificanceDaugther1",&_3DDecayLengthSignificanceDaugther1);
            t->SetBranchAddress("3DDecayLengthDaugther1",&_3DDecayLengthDaugther1);
            t->SetBranchAddress("3DDecayLengthErrorDaugther1",&_3DDecayLengthErrorDaugther1);
            t->SetBranchAddress("2DDecayLengthSignificanceDaugther1",&_2DDecayLengthSignificanceDaugther1);
            t->SetBranchAddress("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
            t->SetBranchAddress("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
            t->SetBranchAddress("NHitD2",&NHitD2);
            t->SetBranchAddress("HighPurityDaugther2",&HighPurityDaugther2);
            t->SetBranchAddress("pTD2",&pTD2);
            t->SetBranchAddress("EtaD2",&EtaD2);
            t->SetBranchAddress("PhiD2",&PhiD2);
            t->SetBranchAddress("pTerrD1",&pTerrD1);
            t->SetBranchAddress("pTerrD2",&pTerrD2);
            t->SetBranchAddress("dedxHarmonic2D2",&dedxHarmonic2D2);
            t->SetBranchAddress("zDCASignificanceGrandDaugther1",&zDCASignificanceGrandDaugther1);
            t->SetBranchAddress("zDCASignificanceGrandDaugther2",&zDCASignificanceGrandDaugther2);
            t->SetBranchAddress("xyDCASignificanceGrandDaugther1",&xyDCASignificanceGrandDaugther1);
            t->SetBranchAddress("xyDCASignificanceGrandDaugther2",&xyDCASignificanceGrandDaugther2);
            t->SetBranchAddress("NHitGrandD1",&NHitGrandD1);
            t->SetBranchAddress("NHitGrandD2",&NHitGrandD2);
            t->SetBranchAddress("HighPurityGrandDaugther1",&HighPurityGrandDaugther1);
            t->SetBranchAddress("HighPurityGrandDaugther2",&HighPurityGrandDaugther2);
            t->SetBranchAddress("pTGrandD1",&pTGrandD1);
            t->SetBranchAddress("pTGrandD2",&pTGrandD2);
            t->SetBranchAddress("pTerrGrandD1",&pTerrGrandD1);
            t->SetBranchAddress("pTerrGrandD2",&pTerrGrandD2);
            t->SetBranchAddress("EtaGrandD1",&EtaGrandD1);
            t->SetBranchAddress("EtaGrandD2",&EtaGrandD2);
            t->SetBranchAddress("dedxHarmonic2GrandD1",&dedxHarmonic2GrandD1);
            t->SetBranchAddress("dedxHarmonic2GrandD2",&dedxHarmonic2GrandD2);
            t->SetBranchAddress("Trk3DDCA",&Trk3DDCA);
            t->SetBranchAddress("Trk3DDCAErr",&Trk3DDCAErr);
            t->SetBranchAddress("dca3D",&dca3D);
	    t->SetBranchAddress("mva",&mva);
            };
            template <typename T>
            void setGENTree(T *t){
            t->SetBranchAddress("candSize_gen", &candSize_gen, &b_candSize_gen);
            t->SetBranchAddress("gen_mass", gen_mass, &b_gen_mass);
            t->SetBranchAddress("gen_pT", gen_pT, &b_gen_pT);
            t->SetBranchAddress("gen_eta", gen_eta, &b_gen_eta);
            t->SetBranchAddress("gen_phi", gen_phi, &b_gen_phi);
            t->SetBranchAddress("gen_y", gen_y, &b_gen_y);
            t->SetBranchAddress("gen_status", gen_status, &b_gen_status);
            t->SetBranchAddress("gen_MotherID", gen_MotherID, &b_gen_MotherID);
            t->SetBranchAddress("gen_DauID1", gen_DauID1, &b_gen_DauID1);
            t->SetBranchAddress("gen_DauID2", gen_DauID2, &b_gen_DauID2);
            t->SetBranchAddress("gen_DauID3", gen_DauID3, &b_gen_DauID3);
            t->SetBranchAddress("gen_D0pT", gen_D0pT, &b_gen_D0pT);
            t->SetBranchAddress("gen_D0eta", gen_D0eta, &b_gen_D0eta);
            t->SetBranchAddress("gen_D0phi", gen_D0phi, &b_gen_D0phi);
            t->SetBranchAddress("gen_D0mass", gen_D0mass, &b_gen_D0mass);
            t->SetBranchAddress("gen_D0y", gen_D0y, &b_gen_D0y);
            t->SetBranchAddress("gen_D0charge", gen_D0charge, &b_gen_D0charge);
            t->SetBranchAddress("gen_D0pdgId", gen_D0pdgId, &b_gen_D0pdgId);
            t->SetBranchAddress("gen_D0ancestorId_", gen_D0ancestorId_, &b_gen_D0ancestorId);
            t->SetBranchAddress("gen_D0ancestorFlavor_", gen_D0ancestorFlavor_, &b_gen_D0ancestorFlavor);
            t->SetBranchAddress("gen_D0Dau1_pT", gen_D0Dau1_pT, &b_gen_D0Dau1_pT);
            t->SetBranchAddress("gen_D0Dau1_eta", gen_D0Dau1_eta, &b_gen_D0Dau1_eta);
            t->SetBranchAddress("gen_D0Dau1_phi", gen_D0Dau1_phi, &b_gen_D0Dau1_phi);
            t->SetBranchAddress("gen_D0Dau1_mass", gen_D0Dau1_mass, &b_gen_D0Dau1_mass);
            t->SetBranchAddress("gen_D0Dau1_y", gen_D0Dau1_y, &b_gen_D0Dau1_y);
            t->SetBranchAddress("gen_D0Dau1_charge", gen_D0Dau1_charge, &b_gen_D0Dau1_charge);
            t->SetBranchAddress("gen_D0Dau1_pdgId", gen_D0Dau1_pdgId, &b_gen_D0Dau1_pdgId);
            t->SetBranchAddress("gen_D0Dau2_pT", gen_D0Dau2_pT, &b_gen_D0Dau2_pT);
            t->SetBranchAddress("gen_D0Dau2_eta", gen_D0Dau2_eta, &b_gen_D0Dau2_eta);
            t->SetBranchAddress("gen_D0Dau2_phi", gen_D0Dau2_phi, &b_gen_D0Dau2_phi);
            t->SetBranchAddress("gen_D0Dau2_mass", gen_D0Dau2_mass, &b_gen_D0Dau2_mass);
            t->SetBranchAddress("gen_D0Dau2_y", gen_D0Dau2_y, &b_gen_D0Dau2_y);
            t->SetBranchAddress("gen_D0Dau2_charge", gen_D0Dau2_charge, &b_gen_D0Dau2_charge);
            t->SetBranchAddress("gen_D0Dau2_pdgId", gen_D0Dau2_pdgId, &b_gen_D0Dau2_pdgId);
            t->SetBranchAddress("gen_D1pT", gen_D1pT, &b_gen_D1pT);
            t->SetBranchAddress("gen_D1eta", gen_D1eta, &b_gen_D1eta);
            t->SetBranchAddress("gen_D1phi", gen_D1phi, &b_gen_D1phi);
            t->SetBranchAddress("gen_D1mass", gen_D1mass, &b_gen_D1mass);
            t->SetBranchAddress("gen_D1y", gen_D1y, &b_gen_D1y);
            t->SetBranchAddress("gen_D1charge", gen_D1charge, &b_gen_D1charge);
            t->SetBranchAddress("gen_D1pdgId", gen_D1pdgId, &b_gen_D1pdgId);
            // t->SetBranchAddress("candSize_gen",&candSize_gen);
            // t->SetBranchAddress("gen_pT",&gen_pT);
            // t->SetBranchAddress("gen_eat",&gen_eat);
            // t->SetBranchAddress("gen_y",&gen_y);
            // t->SetBranchAddress("gen_status",&gen_status);
            // t->SetBranchAddress("gen_MotherID",&gen_MotherID);
            // t->SetBranchAddress("gen_DauID1",&gen_DauID1);
            // t->SetBranchAddress("gen_DauID2",&gen_DauID2);
            // t->SetBranchAddress("gen_DauID3",&gen_DauID3);
            // t->SetBranchAddress("gen_D0pT",&gen_D0pT);
            // t->SetBranchAddress("gen_D0eta",&gen_D0eta);
            // t->SetBranchAddress("gen_D0phi",&gen_D0phi);
            // t->SetBranchAddress("gen_D0mass",&gen_D0mass);
            // t->SetBranchAddress("gen_D0y",&gen_D0y);
            // t->SetBranchAddress("gen_D0charge",&gen_D0charge);
            // t->SetBranchAddress("gen_D0pdgId",&gen_D0pdgId);
            // t->SetBranchAddress("gen_D0Dau1_pT",&gen_D0Dau1_pT);
            // t->SetBranchAddress("gen_D0Dau1_eta",&gen_D0Dau1_eta);
            // t->SetBranchAddress("gen_D0Dau1_phi",&gen_D0Dau1_phi);
            // t->SetBranchAddress("gen_D0Dau1_mass",&gen_D0Dau1_mass);
            // t->SetBranchAddress("gen_D0Dau1_y",&gen_D0Dau1_y);
            // t->SetBranchAddress("gen_D0Dau1_charge",&gen_D0Dau1_charge);
            // t->SetBranchAddress("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId);
            // t->SetBranchAddress("gen_D0Dau2_pT",&gen_D0Dau2_pT);
            // t->SetBranchAddress("gen_D0Dau2_eta",&gen_D0Dau2_eta);
            // t->SetBranchAddress("gen_D0Dau2_phi",&gen_D0Dau2_phi);
            // t->SetBranchAddress("gen_D0Dau2_mass",&gen_D0Dau2_mass);
            // t->SetBranchAddress("gen_D0Dau2_y",&gen_D0Dau2_y);
            // t->SetBranchAddress("gen_D0Dau2_charge",&gen_D0Dau2_charge);
            // t->SetBranchAddress("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId);
            // t->SetBranchAddress("gen_D1pT",&gen_D1pT);
            // t->SetBranchAddress("gen_D1eta",&gen_D1eta);
            // t->SetBranchAddress("gen_D1phi",&gen_D1phi);
            // t->SetBranchAddress("gen_D1mass",&gen_D1mass);
            // t->SetBranchAddress("gen_D1y",&gen_D1y);
            // t->SetBranchAddress("gen_D1charge",&gen_D1charge);
            // t->SetBranchAddress("gen_D1pdgId",&gen_D1pdgId);
            };
        };
    struct simpleDStarMCTreeflat{
        public:
            int   Ntrkoffline;
            int Npixel;
            float HFsumETPlus;
            float HFsumETMinus;
            float ZDCPlus;
            float ZDCMinus;
            float bestvtxX;
            float bestvtxY;
            float bestvtxZ;
            int candSize;
            float pT;
            float y;
            float eta;
            float phi;
            float mass;
            float flavor;
            float VtxProb;
            float VtxChi2;
            float VtxNDF;
            float _3DCosPointingAngle;
            float _3DPointingAngle;
            float _2DCosPointingAngle;
            float _2DPointingAngle;
            float _3DDecayLengthSignificance;
            float _3DDecayLength;
            float _2DDecayLengthSignificance;
            float _2DDecayLength;
            bool isSwap;
            int idmom_reco;
            int idBAnc_reco;
            bool matchGEN;
            float matchGen3DPointingAngle;
            float matchGen2DPointingAngle;
            float matchGen3DDecayLength;
            float matchGen2DDecayLength;
            float matchgen_D0pT;
            float matchgen_D0eta;
            float matchgen_D0phi;
            float matchgen_D0mass;
            float matchgen_D0y;
            int matchgen_D0charge;
            float matchgen_D0pdgId;
            float matchGen_D0pT;
            float matchGen_D0eta;
            float matchGen_D0phi;
            float matchGen_D0mass;
            float matchGen_D0y;
            int matchGen_D0charge;
            float matchGen_D0pdgId;
            float matchGen_D0Dau1_pT;
            float matchGen_D0Dau1_eta;
            float matchGen_D0Dau1_phi;
            float matchGen_D0Dau1_mass;
            float matchGen_D0Dau1_y;
            int matchGen_D0Dau1_charge;
            float matchGen_D0Dau1_pdgId;
            float matchGen_D0Dau2_pT;
            float matchGen_D0Dau2_eta;
            float matchGen_D0Dau2_phi;
            float matchGen_D0Dau2_mass;
            float matchGen_D0Dau2_y;
            int matchGen_D0Dau2_charge;
            float matchGen_D0Dau2_pdgId;
            float matchGen_D1pT;
            float matchGen_D1eta;
            float matchGen_D1phi;
            float matchGen_D1mass;
            float matchGen_D1y;
            float matchGen_D1charge;
            float matchGen_D1pdgId;
            float matchGen_D1decayLength2D_;
            float matchGen_D1decayLength3D_;
            float matchGen_D1angle2D_;
            float matchGen_D1angle3D_;
            Int_t matchGen_D1ancestorId_;
            Int_t matchGen_D1ancestorFlavor_;
            float massDaugther1;
            float pTD1;
            float EtaD1;
            float PhiD1;
            float VtxProbDaugther1;
            float VtxChi2Daugther1;
            float VtxNDFDaugther1;
            float _3DCosPointingAngleDaugther1;
            float _3DPointingAngleDaugther1;
            float _2DCosPointingAngleDaugther1;
            float _2DPointingAngleDaugther1;
            float _3DDecayLengthSignificanceDaugther1;
            float _3DDecayLengthDaugther1;
            float _3DDecayLengthErrorDaugther1;
            float _2DDecayLengthSignificanceDaugther1;
            float zDCASignificanceDaugther2;
            float xyDCASignificanceDaugther2;
            float NHitD2;
            bool HighPurityDaugther2;
            float pTD2;
            float EtaD2;
            float PhiD2;
            float pTerrD1;
            float pTerrD2;
            float dedxHarmonic2D2;
            float zDCASignificanceGrandDaugther1;
            float zDCASignificanceGrandDaugther2;
            float xyDCASignificanceGrandDaugther1;
            float xyDCASignificanceGrandDaugther2;
            float NHitGrandD1;
            float NHitGrandD2;
            bool HighPurityGrandDaugther1;
            bool HighPurityGrandDaugther2;
            float pTGrandD1;
            float pTGrandD2;
            float pTerrGrandD1;
            float pTerrGrandD2;
            float EtaGrandD1;
            float EtaGrandD2;
            float dedxHarmonic2GrandD1;
            float dedxHarmonic2GrandD2;
            float Trk3DDCA;
            float Trk3DDCAErr;
            float dca3D;
            bool isMC;
	   float mva;

                  Int_t           candSize_gen;
   Float_t         gen_mass;   //[candSize_gen]
   Float_t         gen_pT;   //[candSize_gen]
   Float_t         gen_eta;   //[candSize_gen]
   Float_t         gen_phi;   //[candSize_gen]
   Float_t         gen_y;   //[candSize_gen]
   Int_t           gen_status;   //[candSize_gen]
   Int_t           gen_MotherID;   //[candSize_gen]
   Int_t           gen_DauID1;   //[candSize_gen]
   Int_t           gen_DauID2;   //[candSize_gen]
   Int_t           gen_DauID3;   //[candSize_gen]
   Float_t         gen_D0pT;   //[candSize_gen]
   Float_t         gen_D0eta;   //[candSize_gen]
   Float_t         gen_D0phi;   //[candSize_gen]
   Float_t         gen_D0mass;   //[candSize_gen]
   Float_t         gen_D0y;   //[candSize_gen]
   Int_t         gen_D0charge;   //[candSize_gen]
   Int_t         gen_D0pdgId;   //[candSize_gen]
   Int_t        gen_D0ancestorId_;   //[candSize_gen]
    Int_t        gen_D0ancestorFlavor_;   //[candSize_gen]
   Float_t         gen_D0Dau1_pT;   //[candSize_gen]
   Float_t         gen_D0Dau1_eta;   //[candSize_gen]
   Float_t         gen_D0Dau1_phi;   //[candSize_gen]
   Float_t         gen_D0Dau1_mass;   //[candSize_gen]
   Float_t         gen_D0Dau1_y;   //[candSize_gen]
   Float_t         gen_D0Dau1_charge;   //[candSize_gen]
   Int_t         gen_D0Dau1_pdgId;   //[candSize_gen]
   Float_t         gen_D0Dau2_pT;   //[candSize_gen]
   Float_t         gen_D0Dau2_eta;   //[candSize_gen]
   Float_t         gen_D0Dau2_phi;   //[candSize_gen]
   Float_t         gen_D0Dau2_mass;   //[candSize_gen]
   Float_t         gen_D0Dau2_y;   //[candSize_gen]
   Int_t         gen_D0Dau2_charge;   //[candSize_gen]
   Int_t         gen_D0Dau2_pdgId;   //[candSize_gen]
   Float_t         gen_D1pT;   //[candSize_gen]
   Float_t         gen_D1eta;   //[candSize_gen]
   Float_t         gen_D1phi;   //[candSize_gen]
   Float_t         gen_D1mass;   //[candSize_gen]
   Float_t         gen_D1y;   //[candSize_gen]
   Float_t         gen_D1charge;   //[candSize_gen]
   Int_t         gen_D1pdgId; 

            //bool isData;

            // int candSize_gen;
            // float gen_pT;
            // float gen_eat;
            // float gen_y;
            // int gen_status;
            // int gen_MotherID;
            // int gen_DauID1;
            // int gen_DauID2;
            // int gen_DauID3;
            // float gen_D0pT;
            // float gen_D0eta;
            // float gen_D0phi;
            // float gen_D0mass;
            // float gen_D0y;
            // float gen_D0charge;
            // float gen_D0pdgId;
            // float gen_D0Dau1_pT;
            // float gen_D0Dau1_eta;
            // float gen_D0Dau1_phi;
            // float gen_D0Dau1_mass;
            // float gen_D0Dau1_y;
            // float gen_D0Dau1_charge;
            // float gen_D0Dau1_pdgId;
            // float gen_D0Dau2_pT;
            // float gen_D0Dau2_eta;
            // float gen_D0Dau2_phi;
            // float gen_D0Dau2_mass;
            // float gen_D0Dau2_y;
            // float gen_D0Dau2_charge;
            // float gen_D0Dau2_pdgId;
            // float gen_D1pT;
            // float gen_D1eta;
            // float gen_D1phi;
            // float gen_D1mass;
            // float gen_D1y;
            // float gen_D1charge;
            // float gen_D1pdgI;
            simpleDStarMCTreeflat* getEventHandle(){ return this;}
            template <typename T>
        void setGENTree(T *t){
                // t->Branch("isData",&isData);
                t->SetBranchAddress("candSize_gen",&candSize_gen);
                t->SetBranchAddress("gen_mass",&gen_mass);  
                t->SetBranchAddress("gen_pT",&gen_pT);
                t->SetBranchAddress("gen_eta",&gen_eta);
                t->SetBranchAddress("gen_phi",&gen_phi);
                t->SetBranchAddress("gen_y",&gen_y);
                t->SetBranchAddress("gen_status",&gen_status);
                t->SetBranchAddress("gen_MotherID",&gen_MotherID);
                t->SetBranchAddress("gen_DauID1",&gen_DauID1);
                t->SetBranchAddress("gen_DauID2",&gen_DauID2);
                t->SetBranchAddress("gen_DauID3",&gen_DauID3);
                t->SetBranchAddress("gen_D0pT",&gen_D0pT);
                t->SetBranchAddress("gen_D0eta",&gen_D0eta);
                t->SetBranchAddress("gen_D0phi",&gen_D0phi);
                t->SetBranchAddress("gen_D0mass",&gen_D0mass);
                t->SetBranchAddress("gen_D0y",&gen_D0y);
                t->SetBranchAddress("gen_D0charge",&gen_D0charge);
                t->SetBranchAddress("gen_D0pdgId",&gen_D0pdgId);
                t->SetBranchAddress("gen_D0ancestorId_",&gen_D0ancestorId_);
                t->SetBranchAddress("gen_D0ancestorFlavor_",&gen_D0ancestorFlavor_);
                t->SetBranchAddress("gen_D0Dau1_pT",&gen_D0Dau1_pT);
                t->SetBranchAddress("gen_D0Dau1_eta",&gen_D0Dau1_eta);
                t->SetBranchAddress("gen_D0Dau1_phi",&gen_D0Dau1_phi);
                t->SetBranchAddress("gen_D0Dau1_mass",&gen_D0Dau1_mass);
                t->SetBranchAddress("gen_D0Dau1_y",&gen_D0Dau1_y);
                t->SetBranchAddress("gen_D0Dau1_charge",&gen_D0Dau1_charge);
                t->SetBranchAddress("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId);
                t->SetBranchAddress("gen_D0Dau2_pT",&gen_D0Dau2_pT);
                t->SetBranchAddress("gen_D0Dau2_eta",&gen_D0Dau2_eta);
                t->SetBranchAddress("gen_D0Dau2_phi",&gen_D0Dau2_phi);
                t->SetBranchAddress("gen_D0Dau2_mass",&gen_D0Dau2_mass);
                t->SetBranchAddress("gen_D0Dau2_y",&gen_D0Dau2_y);
                t->SetBranchAddress("gen_D0Dau2_charge",&gen_D0Dau2_charge);
                t->SetBranchAddress("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId);
                t->SetBranchAddress("gen_D1pT",&gen_D1pT);
                t->SetBranchAddress("gen_D1eta",&gen_D1eta);
                t->SetBranchAddress("gen_D1phi",&gen_D1phi);
                t->SetBranchAddress("gen_D1mass",&gen_D1mass);
                t->SetBranchAddress("gen_D1y",&gen_D1y);
                t->SetBranchAddress("gen_D1charge",&gen_D1charge);
                t->SetBranchAddress("gen_D1pdgId",&gen_D1pdgId);
        }; 
        template <typename T>
        void setTree(T *t){
            t->SetBranchAddress("Ntrkoffline",&Ntrkoffline);
            t->SetBranchAddress("Npixel",&Npixel);
            t->SetBranchAddress("HFsumETPlus",&HFsumETPlus);
            t->SetBranchAddress("HFsumETMinus",&HFsumETMinus);
            t->SetBranchAddress("ZDCPlus",&ZDCPlus);
            t->SetBranchAddress("ZDCMinus",&ZDCMinus);
            t->SetBranchAddress("bestvtxX",&bestvtxX);
            t->SetBranchAddress("bestvtxY",&bestvtxY);
            t->SetBranchAddress("bestvtxZ",&bestvtxZ);
            t->SetBranchAddress("candSize",&candSize);
            t->SetBranchAddress("pT",&pT);
            t->SetBranchAddress("y",&y);
            t->SetBranchAddress("eta",&eta);
            t->SetBranchAddress("phi",&phi);
            t->SetBranchAddress("mass",&mass);
            t->SetBranchAddress("flavor",&flavor);
            t->SetBranchAddress("VtxProb",&VtxProb);
            t->SetBranchAddress("VtxChi2",&VtxChi2);
            t->SetBranchAddress("VtxNDF",&VtxNDF);
            t->SetBranchAddress("3DCosPointingAngle",&_3DCosPointingAngle);
            t->SetBranchAddress("3DPointingAngle",&_3DPointingAngle);
            t->SetBranchAddress("2DCosPointingAngle",&_2DCosPointingAngle);
            t->SetBranchAddress("2DPointingAngle",&_2DPointingAngle);
            t->SetBranchAddress("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
            t->SetBranchAddress("3DDecayLength",&_3DDecayLength);
            t->SetBranchAddress("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
            t->SetBranchAddress("2DDecayLength",&_2DDecayLength);
            t->SetBranchAddress("isSwap",&isSwap);
            t->SetBranchAddress("idmom_reco",&idmom_reco);
            t->SetBranchAddress("idBAnc_reco",&idBAnc_reco);
            t->SetBranchAddress("matchGEN",&matchGEN);
            t->SetBranchAddress("matchGen3DPointingAngle",&matchGen3DPointingAngle);
            t->SetBranchAddress("matchGen2DPointingAngle",&matchGen2DPointingAngle);
            t->SetBranchAddress("matchGen3DDecayLength",&matchGen3DDecayLength);
            t->SetBranchAddress("matchGen2DDecayLength",&matchGen2DDecayLength);
            t->SetBranchAddress("matchgen_D0pT",&matchgen_D0pT);
            t->SetBranchAddress("matchgen_D0eta",&matchgen_D0eta);
            t->SetBranchAddress("matchgen_D0phi",&matchgen_D0phi);
            t->SetBranchAddress("matchgen_D0mass",&matchgen_D0mass);
            t->SetBranchAddress("matchgen_D0y",&matchgen_D0y);
            t->SetBranchAddress("matchgen_D0charge",&matchgen_D0charge);
            t->SetBranchAddress("matchgen_D0pdgId",&matchgen_D0pdgId);
            t->SetBranchAddress("matchGen_D0pT",&matchGen_D0pT);
            t->SetBranchAddress("matchGen_D0eta",&matchGen_D0eta);
            t->SetBranchAddress("matchGen_D0phi",&matchGen_D0phi);
            t->SetBranchAddress("matchGen_D0mass",&matchGen_D0mass);
            t->SetBranchAddress("matchGen_D0y",&matchGen_D0y);
            t->SetBranchAddress("matchGen_D0charge",&matchGen_D0charge);
            t->SetBranchAddress("matchGen_D0pdgId",&matchGen_D0pdgId);
            t->SetBranchAddress("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT);
            t->SetBranchAddress("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta);
            t->SetBranchAddress("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi);
            t->SetBranchAddress("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass);
            t->SetBranchAddress("matchGen_D0Dau1_y",&matchGen_D0Dau1_y);
            t->SetBranchAddress("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge);
            t->SetBranchAddress("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId);
            t->SetBranchAddress("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT);
            t->SetBranchAddress("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta);
            t->SetBranchAddress("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi);
            t->SetBranchAddress("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass);
            t->SetBranchAddress("matchGen_D0Dau2_y",&matchGen_D0Dau2_y);
            t->SetBranchAddress("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge);
            t->SetBranchAddress("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId);
            t->SetBranchAddress("matchGen_D1pT",&matchGen_D1pT);
            t->SetBranchAddress("matchGen_D1eta",&matchGen_D1eta);
            t->SetBranchAddress("matchGen_D1phi",&matchGen_D1phi);
            t->SetBranchAddress("matchGen_D1mass",&matchGen_D1mass);
            t->SetBranchAddress("matchGen_D1y",&matchGen_D1y);
            t->SetBranchAddress("matchGen_D1charge",&matchGen_D1charge);
            t->SetBranchAddress("matchGen_D1pdgId",&matchGen_D1pdgId);
            t->SetBranchAddress("matchGen_D1decayLength2D_",&matchGen_D1decayLength2D_);
            t->SetBranchAddress("matchGen_D1decayLength3D_",&matchGen_D1decayLength3D_);
            t->SetBranchAddress("matchGen_D1angle2D_",&matchGen_D1angle2D_);
            t->SetBranchAddress("matchGen_D1angle3D_",&matchGen_D1angle3D_);
            t->SetBranchAddress("matchGen_D1ancestorId_",&matchGen_D1ancestorId_);
            t->SetBranchAddress("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_);
            t->SetBranchAddress("massDaugther1",&massDaugther1);
            t->SetBranchAddress("pTD1",&pTD1);
            t->SetBranchAddress("EtaD1",&EtaD1);
            t->SetBranchAddress("PhiD1",&PhiD1);
            t->SetBranchAddress("VtxProbDaugther1",&VtxProbDaugther1);
            t->SetBranchAddress("VtxChi2Daugther1",&VtxChi2Daugther1);
            t->SetBranchAddress("VtxNDFDaugther1",&VtxNDFDaugther1);
            t->SetBranchAddress("3DCosPointingAngleDaugther1",&_3DCosPointingAngleDaugther1);
            t->SetBranchAddress("3DPointingAngleDaugther1",&_3DPointingAngleDaugther1);
            t->SetBranchAddress("2DCosPointingAngleDaugther1",&_2DCosPointingAngleDaugther1);
            t->SetBranchAddress("2DPointingAngleDaugther1",&_2DPointingAngleDaugther1);
            t->SetBranchAddress("3DDecayLengthSignificanceDaugther1",&_3DDecayLengthSignificanceDaugther1);
            t->SetBranchAddress("3DDecayLengthDaugther1",&_3DDecayLengthDaugther1);
            t->SetBranchAddress("3DDecayLengthErrorDaugther1",&_3DDecayLengthErrorDaugther1);
            t->SetBranchAddress("2DDecayLengthSignificanceDaugther1",&_2DDecayLengthSignificanceDaugther1);
            t->SetBranchAddress("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
            t->SetBranchAddress("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
            t->SetBranchAddress("NHitD2",&NHitD2);
            t->SetBranchAddress("HighPurityDaugther2",&HighPurityDaugther2);
            t->SetBranchAddress("pTD2",&pTD2);
            t->SetBranchAddress("EtaD2",&EtaD2);
            t->SetBranchAddress("PhiD2",&PhiD2);
            t->SetBranchAddress("pTerrD1",&pTerrD1);
            t->SetBranchAddress("pTerrD2",&pTerrD2);
            t->SetBranchAddress("dedxHarmonic2D2",&dedxHarmonic2D2);
            t->SetBranchAddress("zDCASignificanceGrandDaugther1",&zDCASignificanceGrandDaugther1);
            t->SetBranchAddress("zDCASignificanceGrandDaugther2",&zDCASignificanceGrandDaugther2);
            t->SetBranchAddress("xyDCASignificanceGrandDaugther1",&xyDCASignificanceGrandDaugther1);
            t->SetBranchAddress("xyDCASignificanceGrandDaugther2",&xyDCASignificanceGrandDaugther2);
            t->SetBranchAddress("NHitGrandD1",&NHitGrandD1);
            t->SetBranchAddress("NHitGrandD2",&NHitGrandD2);
            t->SetBranchAddress("HighPurityGrandDaugther1",&HighPurityGrandDaugther1);
            t->SetBranchAddress("HighPurityGrandDaugther2",&HighPurityGrandDaugther2);
            t->SetBranchAddress("pTGrandD1",&pTGrandD1);
            t->SetBranchAddress("pTGrandD2",&pTGrandD2);
            t->SetBranchAddress("pTerrGrandD1",&pTerrGrandD1);
            t->SetBranchAddress("pTerrGrandD2",&pTerrGrandD2);
            t->SetBranchAddress("EtaGrandD1",&EtaGrandD1);
            t->SetBranchAddress("EtaGrandD2",&EtaGrandD2);
            t->SetBranchAddress("dedxHarmonic2GrandD1",&dedxHarmonic2GrandD1);
            t->SetBranchAddress("dedxHarmonic2GrandD2",&dedxHarmonic2GrandD2);
            t->SetBranchAddress("Trk3DDCA",&Trk3DDCA);
            t->SetBranchAddress("Trk3DDCAErr",&Trk3DDCAErr);
            t->SetBranchAddress("dca3D",&dca3D);
	    t->SetBranchAddress("mva",&mva);};
            template <typename T>
        void setOutputTree(T *t){
            t->Branch("Ntrkoffline",&Ntrkoffline);
            t->Branch("Npixel",&Npixel);
            t->Branch("HFsumETPlus",&HFsumETPlus);
            t->Branch("HFsumETMinus",&HFsumETMinus);
            t->Branch("ZDCPlus",&ZDCPlus);
            t->Branch("ZDCMinus",&ZDCMinus);
            t->Branch("bestvtxX",&bestvtxX);
            t->Branch("bestvtxY",&bestvtxY);
            t->Branch("bestvtxZ",&bestvtxZ);
            t->Branch("candSize",&candSize);
            t->Branch("pT",&pT);
            t->Branch("y",&y);
            t->Branch("eta",&eta);
            t->Branch("phi",&phi);
            t->Branch("mass",&mass);
            t->Branch("flavor",&flavor);
            t->Branch("VtxProb",&VtxProb);
            t->Branch("VtxChi2",&VtxChi2);
            t->Branch("VtxNDF",&VtxNDF);
            t->Branch("3DCosPointingAngle",&_3DCosPointingAngle);
            t->Branch("3DPointingAngle",&_3DPointingAngle);
            t->Branch("2DCosPointingAngle",&_2DCosPointingAngle);
            t->Branch("2DPointingAngle",&_2DPointingAngle);
            t->Branch("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
            t->Branch("3DDecayLength",&_3DDecayLength);
            t->Branch("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
            t->Branch("2DDecayLength",&_2DDecayLength);
            t->Branch("isSwap",&isSwap);
            t->Branch("idmom_reco",&idmom_reco);
            t->Branch("idBAnc_reco",&idBAnc_reco);
            t->Branch("matchGEN",&matchGEN);
            t->Branch("matchGen3DPointingAngle",&matchGen3DPointingAngle);
            t->Branch("matchGen2DPointingAngle",&matchGen2DPointingAngle);
            t->Branch("matchGen3DDecayLength",&matchGen3DDecayLength);
            t->Branch("matchGen2DDecayLength",&matchGen2DDecayLength);
            t->Branch("matchgen_D0pT",&matchgen_D0pT);
            t->Branch("matchgen_D0eta",&matchgen_D0eta);
            t->Branch("matchgen_D0phi",&matchgen_D0phi);
            t->Branch("matchgen_D0mass",&matchgen_D0mass);
            t->Branch("matchgen_D0y",&matchgen_D0y);
            t->Branch("matchgen_D0charge",&matchgen_D0charge);
            t->Branch("matchgen_D0pdgId",&matchgen_D0pdgId);
            t->Branch("matchGen_D0pT",&matchGen_D0pT);
            t->Branch("matchGen_D0eta",&matchGen_D0eta);
            t->Branch("matchGen_D0phi",&matchGen_D0phi);
            t->Branch("matchGen_D0mass",&matchGen_D0mass);
            t->Branch("matchGen_D0y",&matchGen_D0y);
            t->Branch("matchGen_D0charge",&matchGen_D0charge);
            t->Branch("matchGen_D0pdgId",&matchGen_D0pdgId);
            t->Branch("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT);
            t->Branch("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta);
            t->Branch("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi);
            t->Branch("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass);
            t->Branch("matchGen_D0Dau1_y",&matchGen_D0Dau1_y);
            t->Branch("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge);
            t->Branch("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId);
            t->Branch("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT);
            t->Branch("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta);
            t->Branch("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi);
            t->Branch("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass);
            t->Branch("matchGen_D0Dau2_y",&matchGen_D0Dau2_y);
            t->Branch("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge);
            t->Branch("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId);
            t->Branch("matchGen_D1pT",&matchGen_D1pT);
            t->Branch("matchGen_D1eta",&matchGen_D1eta);
            t->Branch("matchGen_D1phi",&matchGen_D1phi);
            t->Branch("matchGen_D1mass",&matchGen_D1mass);
            t->Branch("matchGen_D1y",&matchGen_D1y);
            t->Branch("matchGen_D1charge",&matchGen_D1charge);
            t->Branch("matchGen_D1pdgId",&matchGen_D1pdgId);
            t->Branch("matchGen_D1decayLength2D_",&matchGen_D1decayLength2D_);
            t->Branch("matchGen_D1decayLength3D_",&matchGen_D1decayLength3D_);
            t->Branch("matchGen_D1angle2D_",&matchGen_D1angle2D_);
            t->Branch("matchGen_D1angle3D_",&matchGen_D1angle3D_);
            t->Branch("matchGen_D1ancestorId_",&matchGen_D1ancestorId_);
            t->Branch("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_);
            t->Branch("massDaugther1",&massDaugther1);
            t->Branch("pTD1",&pTD1);
            t->Branch("EtaD1",&EtaD1);
            t->Branch("PhiD1",&PhiD1);
            t->Branch("VtxProbDaugther1",&VtxProbDaugther1);
            t->Branch("VtxChi2Daugther1",&VtxChi2Daugther1);
            t->Branch("VtxNDFDaugther1",&VtxNDFDaugther1);
            t->Branch("3DCosPointingAngleDaugther1",&_3DCosPointingAngleDaugther1);
            t->Branch("3DPointingAngleDaugther1",&_3DPointingAngleDaugther1);
            t->Branch("2DCosPointingAngleDaugther1",&_2DCosPointingAngleDaugther1);
            t->Branch("2DPointingAngleDaugther1",&_2DPointingAngleDaugther1);
            t->Branch("3DDecayLengthSignificanceDaugther1",&_3DDecayLengthSignificanceDaugther1);
            t->Branch("3DDecayLengthDaugther1",&_3DDecayLengthDaugther1);
            t->Branch("3DDecayLengthErrorDaugther1",&_3DDecayLengthErrorDaugther1);
            t->Branch("2DDecayLengthSignificanceDaugther1",&_2DDecayLengthSignificanceDaugther1);
            t->Branch("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
            t->Branch("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
            t->Branch("NHitD2",&NHitD2);
            t->Branch("HighPurityDaugther2",&HighPurityDaugther2);
            t->Branch("pTD2",&pTD2);
            t->Branch("EtaD2",&EtaD2);
            t->Branch("PhiD2",&PhiD2);
            t->Branch("pTerrD1",&pTerrD1);
            t->Branch("pTerrD2",&pTerrD2);
            t->Branch("dedxHarmonic2D2",&dedxHarmonic2D2);
            t->Branch("zDCASignificanceGrandDaugther1",&zDCASignificanceGrandDaugther1);
            t->Branch("zDCASignificanceGrandDaugther2",&zDCASignificanceGrandDaugther2);
            t->Branch("xyDCASignificanceGrandDaugther1",&xyDCASignificanceGrandDaugther1);
            t->Branch("xyDCASignificanceGrandDaugther2",&xyDCASignificanceGrandDaugther2);
            t->Branch("NHitGrandD1",&NHitGrandD1);
            t->Branch("NHitGrandD2",&NHitGrandD2);
            t->Branch("HighPurityGrandDaugther1",&HighPurityGrandDaugther1);
            t->Branch("HighPurityGrandDaugther2",&HighPurityGrandDaugther2);
            t->Branch("pTGrandD1",&pTGrandD1);
            t->Branch("pTGrandD2",&pTGrandD2);
            t->Branch("pTerrGrandD1",&pTerrGrandD1);
            t->Branch("pTerrGrandD2",&pTerrGrandD2);
            t->Branch("EtaGrandD1",&EtaGrandD1);
            t->Branch("EtaGrandD2",&EtaGrandD2);
            t->Branch("dedxHarmonic2GrandD1",&dedxHarmonic2GrandD1);
            t->Branch("dedxHarmonic2GrandD2",&dedxHarmonic2GrandD2);
            t->Branch("Trk3DDCA",&Trk3DDCA);
            t->Branch("Trk3DDCAErr",&Trk3DDCAErr);
            t->Branch("dca3D",&dca3D);
            t->Branch("isMC",&isMC);
	    t->Branch("mva",&mva);
        };
        template <typename T>
        void setGENOutputTree(T *t){
            // t->Branch("isData",&isData);
            t->Branch("candSize_gen",&candSize_gen);
            t->Branch("gen_mass",&gen_mass);  
            t->Branch("gen_pT",&gen_pT);
            t->Branch("gen_eta",&gen_eta);
            t->Branch("gen_phi",&gen_phi);
            t->Branch("gen_y",&gen_y);
            t->Branch("gen_status",&gen_status);
            t->Branch("gen_MotherID",&gen_MotherID);
            t->Branch("gen_DauID1",&gen_DauID1);
            t->Branch("gen_DauID2",&gen_DauID2);
            t->Branch("gen_DauID3",&gen_DauID3);
            t->Branch("gen_D0pT",&gen_D0pT);
            t->Branch("gen_D0eta",&gen_D0eta);
            t->Branch("gen_D0phi",&gen_D0phi);
            t->Branch("gen_D0mass",&gen_D0mass);
            t->Branch("gen_D0y",&gen_D0y);
            t->Branch("gen_D0charge",&gen_D0charge);
            t->Branch("gen_D0pdgId",&gen_D0pdgId);
            t->Branch("gen_D0ancestorId",&gen_D0ancestorId_);
            t->Branch("gen_D0ancestorFlavor",&gen_D0ancestorFlavor_);
            t->Branch("gen_D0Dau1_pT",&gen_D0Dau1_pT);
            t->Branch("gen_D0Dau1_eta",&gen_D0Dau1_eta);
            t->Branch("gen_D0Dau1_phi",&gen_D0Dau1_phi);
            t->Branch("gen_D0Dau1_mass",&gen_D0Dau1_mass);
            t->Branch("gen_D0Dau1_y",&gen_D0Dau1_y);
            t->Branch("gen_D0Dau1_charge",&gen_D0Dau1_charge);
            t->Branch("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId);
            t->Branch("gen_D0Dau2_pT",&gen_D0Dau2_pT);
            t->Branch("gen_D0Dau2_eta",&gen_D0Dau2_eta);
            t->Branch("gen_D0Dau2_phi",&gen_D0Dau2_phi);
            t->Branch("gen_D0Dau2_mass",&gen_D0Dau2_mass);
            t->Branch("gen_D0Dau2_y",&gen_D0Dau2_y);
            t->Branch("gen_D0Dau2_charge",&gen_D0Dau2_charge);
            t->Branch("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId);
            t->Branch("gen_D1pT",&gen_D1pT);
            t->Branch("gen_D1eta",&gen_D1eta);
            t->Branch("gen_D1phi",&gen_D1phi);
            t->Branch("gen_D1mass",&gen_D1mass);
            t->Branch("gen_D1y",&gen_D1y);
            t->Branch("gen_D1charge",&gen_D1charge);
            t->Branch("gen_D1pdgId",&gen_D1pdgId);
    };
        void copyDn( simpleDStarMCTreeevt& evt, int idx){
            Ntrkoffline=evt.Ntrkoffline;
            Npixel=evt.Npixel;
            HFsumETPlus=evt.HFsumETPlus;
            HFsumETMinus=evt.HFsumETMinus;
            ZDCPlus=evt.ZDCPlus;
            ZDCMinus=evt.ZDCMinus;
            bestvtxX=evt.bestvtxX;
            bestvtxY=evt.bestvtxY;
            bestvtxZ=evt.bestvtxZ;
            candSize=evt.candSize;
            pT=evt.pT[idx];
            y=evt.y[idx];
            eta=evt.eta[idx];
            phi=evt.phi[idx];
            mass=evt.mass[idx];
            flavor=evt.flavor[idx];
            VtxProb=evt.VtxProb[idx];
            VtxChi2=evt.VtxChi2[idx];
            VtxNDF=evt.VtxNDF[idx];
            _3DCosPointingAngle=evt._3DCosPointingAngle[idx];
            _3DPointingAngle=evt._3DPointingAngle[idx];
            _2DCosPointingAngle=evt._2DCosPointingAngle[idx];
            _2DPointingAngle=evt._2DPointingAngle[idx];
            _3DDecayLengthSignificance=evt._3DDecayLengthSignificance[idx];
            _3DDecayLength=evt._3DDecayLength[idx];
            _2DDecayLengthSignificance=evt._2DDecayLengthSignificance[idx];
            _2DDecayLength=evt._2DDecayLength[idx];
            isSwap=evt.isSwap[idx];
            idmom_reco=evt.idmom_reco[idx];
            idBAnc_reco=evt.idBAnc_reco[idx];
            matchGEN=evt.matchGEN[idx];
            matchGen3DPointingAngle=evt.matchGen3DPointingAngle[idx];
            matchGen2DPointingAngle=evt.matchGen2DPointingAngle[idx];
            matchGen3DDecayLength=evt.matchGen3DDecayLength[idx];
            matchGen2DDecayLength=evt.matchGen2DDecayLength[idx];
            matchgen_D0pT=evt.matchgen_D0pT[idx];
            matchgen_D0eta=evt.matchgen_D0eta[idx];
            matchgen_D0phi=evt.matchgen_D0phi[idx];
            matchgen_D0mass=evt.matchgen_D0mass[idx];
            matchgen_D0y=evt.matchgen_D0y[idx];
            matchgen_D0charge=evt.matchgen_D0charge[idx];
            matchgen_D0pdgId=evt.matchgen_D0pdgId[idx];
            matchGen_D0pT=evt.matchGen_D0pT[idx];
            matchGen_D0eta=evt.matchGen_D0eta[idx];
            matchGen_D0phi=evt.matchGen_D0phi[idx];
            matchGen_D0mass=evt.matchGen_D0mass[idx];
            matchGen_D0y=evt.matchGen_D0y[idx];
            matchGen_D0charge=evt.matchGen_D0charge[idx];
            matchGen_D0pdgId=evt.matchGen_D0pdgId[idx];
            matchGen_D0Dau1_pT=evt.matchGen_D0Dau1_pT[idx];
            matchGen_D0Dau1_eta=evt.matchGen_D0Dau1_eta[idx];
            matchGen_D0Dau1_phi=evt.matchGen_D0Dau1_phi[idx];
            matchGen_D0Dau1_mass=evt.matchGen_D0Dau1_mass[idx];
            matchGen_D0Dau1_y=evt.matchGen_D0Dau1_y[idx];
            matchGen_D0Dau1_charge=evt.matchGen_D0Dau1_charge[idx];
            matchGen_D0Dau1_pdgId=evt.matchGen_D0Dau1_pdgId[idx];
            matchGen_D0Dau2_pT=evt.matchGen_D0Dau2_pT[idx];
            matchGen_D0Dau2_eta=evt.matchGen_D0Dau2_eta[idx];
            matchGen_D0Dau2_phi=evt.matchGen_D0Dau2_phi[idx];
            matchGen_D0Dau2_mass=evt.matchGen_D0Dau2_mass[idx];
            matchGen_D0Dau2_y=evt.matchGen_D0Dau2_y[idx];
            matchGen_D0Dau2_charge=evt.matchGen_D0Dau2_charge[idx];
            matchGen_D0Dau2_pdgId=evt.matchGen_D0Dau2_pdgId[idx];
            matchGen_D1pT=evt.matchGen_D1pT[idx];
            matchGen_D1eta=evt.matchGen_D1eta[idx];
            matchGen_D1phi=evt.matchGen_D1phi[idx];
            matchGen_D1mass=evt.matchGen_D1mass[idx];
            matchGen_D1y=evt.matchGen_D1y[idx];
            matchGen_D1charge=evt.matchGen_D1charge[idx];
            matchGen_D1pdgId=evt.matchGen_D1pdgId[idx];
            matchGen_D1decayLength2D_=evt.matchGen_D1decayLength2D_[idx];
            matchGen_D1decayLength3D_=evt.matchGen_D1decayLength3D_[idx];
            matchGen_D1angle2D_=evt.matchGen_D1angle2D_[idx];
            matchGen_D1angle3D_=evt.matchGen_D1angle3D_[idx];
            matchGen_D1ancestorId_=evt.matchGen_D1ancestorId_[idx];
            matchGen_D1ancestorFlavor_=evt.matchGen_D1ancestorFlavor_[idx];
            massDaugther1=evt.massDaugther1[idx];
            pTD1=evt.pTD1[idx];
            EtaD1=evt.EtaD1[idx];
            PhiD1=evt.PhiD1[idx];
            VtxProbDaugther1=evt.VtxProbDaugther1[idx];
            VtxChi2Daugther1=evt.VtxChi2Daugther1[idx];
            VtxNDFDaugther1=evt.VtxNDFDaugther1[idx];
            _3DCosPointingAngleDaugther1=evt._3DCosPointingAngleDaugther1[idx];
            _3DPointingAngleDaugther1=evt._3DPointingAngleDaugther1[idx];
            _2DCosPointingAngleDaugther1=evt._2DCosPointingAngleDaugther1[idx];
            _2DPointingAngleDaugther1=evt._2DPointingAngleDaugther1[idx];
            _3DDecayLengthSignificanceDaugther1=evt._3DDecayLengthSignificanceDaugther1[idx];
            _3DDecayLengthDaugther1=evt._3DDecayLengthDaugther1[idx];
            _3DDecayLengthErrorDaugther1=evt._3DDecayLengthErrorDaugther1[idx];
            _2DDecayLengthSignificanceDaugther1=evt._2DDecayLengthSignificanceDaugther1[idx];
            zDCASignificanceDaugther2=evt.zDCASignificanceDaugther2[idx];
            xyDCASignificanceDaugther2=evt.xyDCASignificanceDaugther2[idx];
            NHitD2=evt.NHitD2[idx];
            HighPurityDaugther2=evt.HighPurityDaugther2[idx];
            pTD2=evt.pTD2[idx];
            EtaD2=evt.EtaD2[idx];
            PhiD2=evt.PhiD2[idx];
            pTerrD1=evt.pTerrD1[idx];
            pTerrD2=evt.pTerrD2[idx];
            dedxHarmonic2D2=evt.dedxHarmonic2D2[idx];
            zDCASignificanceGrandDaugther1=evt.zDCASignificanceGrandDaugther1[idx];
            zDCASignificanceGrandDaugther2=evt.zDCASignificanceGrandDaugther2[idx];
            xyDCASignificanceGrandDaugther1=evt.xyDCASignificanceGrandDaugther1[idx];
            xyDCASignificanceGrandDaugther2=evt.xyDCASignificanceGrandDaugther2[idx];
            NHitGrandD1=evt.NHitGrandD1[idx];
            NHitGrandD2=evt.NHitGrandD2[idx];
            HighPurityGrandDaugther1=evt.HighPurityGrandDaugther1[idx];
            HighPurityGrandDaugther2=evt.HighPurityGrandDaugther2[idx];
            pTGrandD1=evt.pTGrandD1[idx];
            pTGrandD2=evt.pTGrandD2[idx];
            pTerrGrandD1=evt.pTerrGrandD1[idx];
            pTerrGrandD2=evt.pTerrGrandD2[idx];
            EtaGrandD1=evt.EtaGrandD1[idx];
            EtaGrandD2=evt.EtaGrandD2[idx];
            dedxHarmonic2GrandD1=evt.dedxHarmonic2GrandD1[idx];
            dedxHarmonic2GrandD2=evt.dedxHarmonic2GrandD2[idx];
            Trk3DDCA=evt.Trk3DDCA[idx];
            Trk3DDCAErr=evt.Trk3DDCAErr[idx];
            dca3D=evt.dca3D[idx];
	    mva = evt.mva[idx];
            //isData=evt.isData;
            };
        void copyDn( simpleDStarDataTreeevt& evt, int idx){
            Ntrkoffline=evt.Ntrkoffline;
            Npixel=evt.Npixel;
            HFsumETPlus=evt.HFsumETPlus;
            HFsumETMinus=evt.HFsumETMinus;
            ZDCPlus=evt.ZDCPlus;
            ZDCMinus=evt.ZDCMinus;
            bestvtxX=evt.bestvtxX;
            bestvtxY=evt.bestvtxY;
            bestvtxZ=evt.bestvtxZ;
            candSize=evt.candSize;
            pT=evt.pT[idx];
            y=evt.y[idx];
            eta=evt.eta[idx];
            phi=evt.phi[idx];
            mass=evt.mass[idx];
            flavor=evt.flavor[idx];
            VtxProb=evt.VtxProb[idx];
            VtxChi2=evt.VtxChi2[idx];
            VtxNDF=evt.VtxNDF[idx];
            _3DCosPointingAngle=evt._3DCosPointingAngle[idx];
            _3DPointingAngle=evt._3DPointingAngle[idx];
            _2DCosPointingAngle=evt._2DCosPointingAngle[idx];
            _2DPointingAngle=evt._2DPointingAngle[idx];
            _3DDecayLengthSignificance=evt._3DDecayLengthSignificance[idx];
            _3DDecayLength=evt._3DDecayLength[idx];
            _2DDecayLengthSignificance=evt._2DDecayLengthSignificance[idx];
            _2DDecayLength=evt._2DDecayLength[idx];
            massDaugther1=evt.massDaugther1[idx];
            pTD1=evt.pTD1[idx];
            EtaD1=evt.EtaD1[idx];
            PhiD1=evt.PhiD1[idx];
            VtxProbDaugther1=evt.VtxProbDaugther1[idx];
            VtxChi2Daugther1=evt.VtxChi2Daugther1[idx];
            VtxNDFDaugther1=evt.VtxNDFDaugther1[idx];
            _3DCosPointingAngleDaugther1=evt._3DCosPointingAngleDaugther1[idx];
            _3DPointingAngleDaugther1=evt._3DPointingAngleDaugther1[idx];
            _2DCosPointingAngleDaugther1=evt._2DCosPointingAngleDaugther1[idx];
            _2DPointingAngleDaugther1=evt._2DPointingAngleDaugther1[idx];
            _3DDecayLengthSignificanceDaugther1=evt._3DDecayLengthSignificanceDaugther1[idx];
            _3DDecayLengthDaugther1=evt._3DDecayLengthDaugther1[idx];
            _3DDecayLengthErrorDaugther1=evt._3DDecayLengthErrorDaugther1[idx];
            _2DDecayLengthSignificanceDaugther1=evt._2DDecayLengthSignificanceDaugther1[idx];
            zDCASignificanceDaugther2=evt.zDCASignificanceDaugther2[idx];
            xyDCASignificanceDaugther2=evt.xyDCASignificanceDaugther2[idx];
            NHitD2=evt.NHitD2[idx];
            HighPurityDaugther2=evt.HighPurityDaugther2[idx];
            pTD2=evt.pTD2[idx];
            EtaD2=evt.EtaD2[idx];
            PhiD2=evt.PhiD2[idx];
            pTerrD1=evt.pTerrD1[idx];
            pTerrD2=evt.pTerrD2[idx];
            dedxHarmonic2D2=evt.dedxHarmonic2D2[idx];
            zDCASignificanceGrandDaugther1=evt.zDCASignificanceGrandDaugther1[idx];
            zDCASignificanceGrandDaugther2=evt.zDCASignificanceGrandDaugther2[idx];
            xyDCASignificanceGrandDaugther1=evt.xyDCASignificanceGrandDaugther1[idx];
            xyDCASignificanceGrandDaugther2=evt.xyDCASignificanceGrandDaugther2[idx];
            NHitGrandD1=evt.NHitGrandD1[idx];
            NHitGrandD2=evt.NHitGrandD2[idx];
            HighPurityGrandDaugther1=evt.HighPurityGrandDaugther1[idx];
            HighPurityGrandDaugther2=evt.HighPurityGrandDaugther2[idx];
            pTGrandD1=evt.pTGrandD1[idx];
            pTGrandD2=evt.pTGrandD2[idx];
            pTerrGrandD1=evt.pTerrGrandD1[idx];
            pTerrGrandD2=evt.pTerrGrandD2[idx];
            EtaGrandD1=evt.EtaGrandD1[idx];
            EtaGrandD2=evt.EtaGrandD2[idx];
            dedxHarmonic2GrandD1=evt.dedxHarmonic2GrandD1[idx];
            dedxHarmonic2GrandD2=evt.dedxHarmonic2GrandD2[idx];
            Trk3DDCA=evt.Trk3DDCA[idx];
            Trk3DDCAErr=evt.Trk3DDCAErr[idx];
            dca3D=evt.dca3D[idx];
            mva = evt.mva[idx];
            //isData=evt.isData;
            };
        void copyGENDn( simpleDStarMCTreeevt& evt, int idx){
            gen_mass=evt.gen_mass[idx];
            gen_pT=evt.gen_pT[idx];
            gen_eta=evt.gen_eta[idx];
            gen_phi=evt.gen_phi[idx];
            gen_y=evt.gen_y[idx];
            gen_mass=evt.gen_mass[idx];  
            gen_pT=evt.gen_pT[idx];
            gen_eta=evt.gen_eta[idx];
            gen_y=evt.gen_y[idx];
            gen_status=evt.gen_status[idx];
            gen_MotherID=evt.gen_MotherID[idx];
            gen_DauID1=evt.gen_DauID1[idx];
            gen_DauID2=evt.gen_DauID2[idx];
            gen_DauID3=evt.gen_DauID3[idx];
            gen_D0pT=evt.gen_D0pT[idx];
            gen_D0eta=evt.gen_D0eta[idx];
            gen_D0phi=evt.gen_D0phi[idx];
            gen_D0mass=evt.gen_D0mass[idx];
            gen_D0y=evt.gen_D0y[idx];
            gen_D0charge=evt.gen_D0charge[idx];
            gen_D0pdgId=evt.gen_D0pdgId[idx];
            gen_D0ancestorFlavor_=evt.gen_D0ancestorFlavor_[idx];
            gen_D0ancestorId_=evt.gen_D0ancestorId_[idx];
            gen_D0Dau1_pT=evt.gen_D0Dau1_pT[idx];
            gen_D0Dau1_eta=evt.gen_D0Dau1_eta[idx];
            gen_D0Dau1_phi=evt.gen_D0Dau1_phi[idx];
            gen_D0Dau1_mass=evt.gen_D0Dau1_mass[idx];
            gen_D0Dau1_y=evt.gen_D0Dau1_y[idx];
            gen_D0Dau1_charge=evt.gen_D0Dau1_charge[idx];
            gen_D0Dau1_pdgId=evt.gen_D0Dau1_pdgId[idx];
            gen_D0Dau2_pT=evt.gen_D0Dau2_pT[idx];
            gen_D0Dau2_eta=evt.gen_D0Dau2_eta[idx];
            gen_D0Dau2_phi=evt.gen_D0Dau2_phi[idx];
            gen_D0Dau2_mass=evt.gen_D0Dau2_mass[idx];
            gen_D0Dau2_y=evt.gen_D0Dau2_y[idx];
            gen_D0Dau2_charge=evt.gen_D0Dau2_charge[idx];
            gen_D0Dau2_pdgId=evt.gen_D0Dau2_pdgId[idx];
            gen_D1pT=evt.gen_D1pT[idx];
            gen_D1eta=evt.gen_D1eta[idx];
            gen_D1phi=evt.gen_D1phi[idx];
            gen_D1mass=evt.gen_D1mass[idx];
            gen_D1y=evt.gen_D1y[idx];
            gen_D1charge=evt.gen_D1charge[idx];
            gen_D1pdgId=evt.gen_D1pdgId[idx];
        };
        void copyDn( simpleDStarMCTreeflat& evt){
            Ntrkoffline=evt.Ntrkoffline;
            Npixel=evt.Npixel;
            HFsumETPlus=evt.HFsumETPlus;
            HFsumETMinus=evt.HFsumETMinus;
            ZDCPlus=evt.ZDCPlus;
            ZDCMinus=evt.ZDCMinus;
            bestvtxX=evt.bestvtxX;
            bestvtxY=evt.bestvtxY;
            bestvtxZ=evt.bestvtxZ;
            candSize=evt.candSize;
            pT=evt.pT;
            y=evt.y;
            eta=evt.eta;
            phi=evt.phi;
            mass=evt.mass;
            flavor=evt.flavor;
            VtxProb=evt.VtxProb;
            VtxChi2=evt.VtxChi2;
            VtxNDF=evt.VtxNDF;
            _3DCosPointingAngle=evt._3DCosPointingAngle;
            _3DPointingAngle=evt._3DPointingAngle;
            _2DCosPointingAngle=evt._2DCosPointingAngle;
            _2DPointingAngle=evt._2DPointingAngle;
            _3DDecayLengthSignificance=evt._3DDecayLengthSignificance;
            _3DDecayLength=evt._3DDecayLength;
            _2DDecayLengthSignificance=evt._2DDecayLengthSignificance;
            _2DDecayLength=evt._2DDecayLength;
            isSwap=evt.isSwap;
            idmom_reco=evt.idmom_reco;
            idBAnc_reco=evt.idBAnc_reco;
            matchGEN=evt.matchGEN;
            matchGen3DPointingAngle=evt.matchGen3DPointingAngle;
            matchGen2DPointingAngle=evt.matchGen2DPointingAngle;
            matchGen3DDecayLength=evt.matchGen3DDecayLength;
            matchGen2DDecayLength=evt.matchGen2DDecayLength;
            matchgen_D0pT=evt.matchgen_D0pT;
            matchgen_D0eta=evt.matchgen_D0eta;
            matchgen_D0phi=evt.matchgen_D0phi;
            matchgen_D0mass=evt.matchgen_D0mass;
            matchgen_D0y=evt.matchgen_D0y;
            matchgen_D0charge=evt.matchgen_D0charge;
            matchgen_D0pdgId=evt.matchgen_D0pdgId;
            matchGen_D0pT=evt.matchGen_D0pT;
            matchGen_D0eta=evt.matchGen_D0eta;
            matchGen_D0phi=evt.matchGen_D0phi;
            matchGen_D0mass=evt.matchGen_D0mass;
            matchGen_D0y=evt.matchGen_D0y;
            matchGen_D0charge=evt.matchGen_D0charge;
            matchGen_D0pdgId=evt.matchGen_D0pdgId;
            matchGen_D0Dau1_pT=evt.matchGen_D0Dau1_pT;
            matchGen_D0Dau1_eta=evt.matchGen_D0Dau1_eta;
            matchGen_D0Dau1_phi=evt.matchGen_D0Dau1_phi;
            matchGen_D0Dau1_mass=evt.matchGen_D0Dau1_mass;
            matchGen_D0Dau1_y=evt.matchGen_D0Dau1_y;
            matchGen_D0Dau1_charge=evt.matchGen_D0Dau1_charge;
            matchGen_D0Dau1_pdgId=evt.matchGen_D0Dau1_pdgId;
            matchGen_D0Dau2_pT=evt.matchGen_D0Dau2_pT;
            matchGen_D0Dau2_eta=evt.matchGen_D0Dau2_eta;
            matchGen_D0Dau2_phi=evt.matchGen_D0Dau2_phi;
            matchGen_D0Dau2_mass=evt.matchGen_D0Dau2_mass;
            matchGen_D0Dau2_y=evt.matchGen_D0Dau2_y;
            matchGen_D0Dau2_charge=evt.matchGen_D0Dau2_charge;
            matchGen_D0Dau2_pdgId=evt.matchGen_D0Dau2_pdgId;
            matchGen_D1pT=evt.matchGen_D1pT;
            matchGen_D1eta=evt.matchGen_D1eta;
            matchGen_D1phi=evt.matchGen_D1phi;
            matchGen_D1mass=evt.matchGen_D1mass;
            matchGen_D1y=evt.matchGen_D1y;
            matchGen_D1charge=evt.matchGen_D1charge;
            matchGen_D1pdgId=evt.matchGen_D1pdgId;
            matchGen_D1decayLength2D_=evt.matchGen_D1decayLength2D_;
            matchGen_D1decayLength3D_=evt.matchGen_D1decayLength3D_;
            matchGen_D1angle2D_=evt.matchGen_D1angle2D_;
            matchGen_D1angle3D_=evt.matchGen_D1angle3D_;
            matchGen_D1ancestorId_=evt.matchGen_D1ancestorId_;
            matchGen_D1ancestorFlavor_=evt.matchGen_D1ancestorFlavor_;
            massDaugther1=evt.massDaugther1;
            pTD1=evt.pTD1;
            EtaD1=evt.EtaD1;
            PhiD1=evt.PhiD1;
            VtxProbDaugther1=evt.VtxProbDaugther1;
            VtxChi2Daugther1=evt.VtxChi2Daugther1;
            VtxNDFDaugther1=evt.VtxNDFDaugther1;
            _3DCosPointingAngleDaugther1=evt._3DCosPointingAngleDaugther1;
            _3DPointingAngleDaugther1=evt._3DPointingAngleDaugther1;
            _2DCosPointingAngleDaugther1=evt._2DCosPointingAngleDaugther1;
            _2DPointingAngleDaugther1=evt._2DPointingAngleDaugther1;
            _3DDecayLengthSignificanceDaugther1=evt._3DDecayLengthSignificanceDaugther1;
            _3DDecayLengthDaugther1=evt._3DDecayLengthDaugther1;
            _3DDecayLengthErrorDaugther1=evt._3DDecayLengthErrorDaugther1;
            _2DDecayLengthSignificanceDaugther1=evt._2DDecayLengthSignificanceDaugther1;
            zDCASignificanceDaugther2=evt.zDCASignificanceDaugther2;
            xyDCASignificanceDaugther2=evt.xyDCASignificanceDaugther2;
            NHitD2=evt.NHitD2;
            HighPurityDaugther2=evt.HighPurityDaugther2;
            pTD2=evt.pTD2;
            EtaD2=evt.EtaD2;
            PhiD2=evt.PhiD2;
            pTerrD1=evt.pTerrD1;
            pTerrD2=evt.pTerrD2;
            dedxHarmonic2D2=evt.dedxHarmonic2D2;
            zDCASignificanceGrandDaugther1=evt.zDCASignificanceGrandDaugther1;
            zDCASignificanceGrandDaugther2=evt.zDCASignificanceGrandDaugther2;
            xyDCASignificanceGrandDaugther1=evt.xyDCASignificanceGrandDaugther1;
            xyDCASignificanceGrandDaugther2=evt.xyDCASignificanceGrandDaugther2;
            NHitGrandD1=evt.NHitGrandD1;
            NHitGrandD2=evt.NHitGrandD2;
            HighPurityGrandDaugther1=evt.HighPurityGrandDaugther1;
            HighPurityGrandDaugther2=evt.HighPurityGrandDaugther2;
            pTGrandD1=evt.pTGrandD1;
            pTGrandD2=evt.pTGrandD2;
            pTerrGrandD1=evt.pTerrGrandD1;
            pTerrGrandD2=evt.pTerrGrandD2;
            EtaGrandD1=evt.EtaGrandD1;
            EtaGrandD2=evt.EtaGrandD2;
            dedxHarmonic2GrandD1=evt.dedxHarmonic2GrandD1;
            dedxHarmonic2GrandD2=evt.dedxHarmonic2GrandD2;
	    mva = evt.mva;
            //isData=evt.isData;
            };
};    struct simpleDStarDataTreeflat{
        public:
            int   Ntrkoffline;
            int Npixel;
            float HFsumETPlus;
            float HFsumETMinus;
            float ZDCPlus;
            float ZDCMinus;
            float bestvtxX;
            float bestvtxY;
            float bestvtxZ;
            int candSize;
            float pT;
            float y;
            float eta;
            float phi;
            float mass;
            float flavor;
            float VtxProb;
            float VtxChi2;
            float VtxNDF;
            float _3DCosPointingAngle;
            float _3DPointingAngle;
            float _2DCosPointingAngle;
            float _2DPointingAngle;
            float _3DDecayLengthSignificance;
            float _3DDecayLength;
            float _2DDecayLengthSignificance;
            float _2DDecayLength;
            float massDaugther1;
            float pTD1;
            float EtaD1;
            float PhiD1;
            float VtxProbDaugther1;
            float VtxChi2Daugther1;
            float VtxNDFDaugther1;
            float _3DCosPointingAngleDaugther1;
            float _3DPointingAngleDaugther1;
            float _2DCosPointingAngleDaugther1;
            float _2DPointingAngleDaugther1;
            float _3DDecayLengthSignificanceDaugther1;
            float _3DDecayLengthDaugther1;
            float _3DDecayLengthErrorDaugther1;
            float _2DDecayLengthSignificanceDaugther1;
            float zDCASignificanceDaugther2;
            float xyDCASignificanceDaugther2;
            float NHitD2;
            bool HighPurityDaugther2;
            float pTD2;
            float EtaD2;
            float PhiD2;
            float pTerrD1;
            float pTerrD2;
            float dedxHarmonic2D2;
            float zDCASignificanceGrandDaugther1;
            float zDCASignificanceGrandDaugther2;
            float xyDCASignificanceGrandDaugther1;
            float xyDCASignificanceGrandDaugther2;
            float NHitGrandD1;
            float NHitGrandD2;
            bool HighPurityGrandDaugther1;
            bool HighPurityGrandDaugther2;
            float pTGrandD1;
            float pTGrandD2;
            float pTerrGrandD1;
            float pTerrGrandD2;
            float EtaGrandD1;
            float EtaGrandD2;
            float dedxHarmonic2GrandD1;
            float dedxHarmonic2GrandD2;
            float Trk3DDCA;
            float Trk3DDCAErr;
            float dca3D;
            float mva;
            bool isMC;
            // int candSize_gen;
            // float gen_pT;
            // float gen_eat;
            // float gen_y;
            // int gen_status;
            // int gen_MotherID;
            // int gen_DauID1;
            // int gen_DauID2;
            // int gen_DauID3;
            // float gen_D0pT;
            // float gen_D0eta;
            // float gen_D0phi;
            // float gen_D0mass;
            // float gen_D0y;
            // float gen_D0charge;
            // float gen_D0pdgId;
            // float gen_D0Dau1_pT;
            // float gen_D0Dau1_eta;
            // float gen_D0Dau1_phi;
            // float gen_D0Dau1_mass;
            // float gen_D0Dau1_y;
            // float gen_D0Dau1_charge;
            // float gen_D0Dau1_pdgId;
            // float gen_D0Dau2_pT;
            // float gen_D0Dau2_eta;
            // float gen_D0Dau2_phi;
            // float gen_D0Dau2_mass;
            // float gen_D0Dau2_y;
            // float gen_D0Dau2_charge;
            // float gen_D0Dau2_pdgId;
            // float gen_D1pT;
            // float gen_D1eta;
            // float gen_D1phi;
            // float gen_D1mass;
            // float gen_D1y;
            // float gen_D1charge;
            // float gen_D1pdgI;
            simpleDStarDataTreeflat* getEventHandle(){ return this;}
            template<typename T>
              void setTree(T *t){
            t->SetBranchAddress("Ntrkoffline",&Ntrkoffline);
            t->SetBranchAddress("Npixel",&Npixel);
            t->SetBranchAddress("HFsumETPlus",&HFsumETPlus);
            t->SetBranchAddress("HFsumETMinus",&HFsumETMinus);
            t->SetBranchAddress("ZDCPlus",&ZDCPlus);
            t->SetBranchAddress("ZDCMinus",&ZDCMinus);
            t->SetBranchAddress("bestvtxX",&bestvtxX);
            t->SetBranchAddress("bestvtxY",&bestvtxY);
            t->SetBranchAddress("bestvtxZ",&bestvtxZ);
            t->SetBranchAddress("candSize",&candSize);
            t->SetBranchAddress("pT",&pT);
            t->SetBranchAddress("y",&y);
            t->SetBranchAddress("eta",&eta);
            t->SetBranchAddress("phi",&phi);
            t->SetBranchAddress("mass",&mass);
            t->SetBranchAddress("flavor",&flavor);
            t->SetBranchAddress("VtxProb",&VtxProb);
            t->SetBranchAddress("VtxChi2",&VtxChi2);
            t->SetBranchAddress("VtxNDF",&VtxNDF);
            t->SetBranchAddress("3DCosPointingAngle",&_3DCosPointingAngle);
            t->SetBranchAddress("3DPointingAngle",&_3DPointingAngle);
            t->SetBranchAddress("2DCosPointingAngle",&_2DCosPointingAngle);
            t->SetBranchAddress("2DPointingAngle",&_2DPointingAngle);
            t->SetBranchAddress("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
            t->SetBranchAddress("3DDecayLength",&_3DDecayLength);
            t->SetBranchAddress("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
            t->SetBranchAddress("2DDecayLength",&_2DDecayLength);
            // t->SetBranchAddress("isSwap",&isSwap);
            // t->SetBranchAddress("idmom_reco",&idmom_reco);
            // t->SetBranchAddress("idBAnc_reco",&idBAnc_reco);
            // t->SetBranchAddress("matchGEN",&matchGEN);
            // t->SetBranchAddress("matchGen3DPointingAngle",&matchGen3DPointingAngle);
            // t->SetBranchAddress("matchGen2DPointingAngle",&matchGen2DPointingAngle);
            // t->SetBranchAddress("matchGen3DDecayLength",&matchGen3DDecayLength);
            // t->SetBranchAddress("matchGen2DDecayLength",&matchGen2DDecayLength);
            // t->SetBranchAddress("matchgen_D0pT",&matchgen_D0pT);
            // t->SetBranchAddress("matchgen_D0eta",&matchgen_D0eta);
            // t->SetBranchAddress("matchgen_D0phi",&matchgen_D0phi);
            // t->SetBranchAddress("matchgen_D0mass",&matchgen_D0mass);
            // t->SetBranchAddress("matchgen_D0y",&matchgen_D0y);
            // t->SetBranchAddress("matchgen_D0charge",&matchgen_D0charge);
            // t->SetBranchAddress("matchgen_D0pdgId",&matchgen_D0pdgId);
            // t->SetBranchAddress("matchGen_D0pT",&matchGen_D0pT);
            // t->SetBranchAddress("matchGen_D0eta",&matchGen_D0eta);
            // t->SetBranchAddress("matchGen_D0phi",&matchGen_D0phi);
            // t->SetBranchAddress("matchGen_D0mass",&matchGen_D0mass);
            // t->SetBranchAddress("matchGen_D0y",&matchGen_D0y);
            // t->SetBranchAddress("matchGen_D0charge",&matchGen_D0charge);
            // t->SetBranchAddress("matchGen_D0pdgId",&matchGen_D0pdgId);
            // t->SetBranchAddress("matchGen_D0Dau1_pT",&matchGen_D0Dau1_pT);
            // t->SetBranchAddress("matchGen_D0Dau1_eta",&matchGen_D0Dau1_eta);
            // t->SetBranchAddress("matchGen_D0Dau1_phi",&matchGen_D0Dau1_phi);
            // t->SetBranchAddress("matchGen_D0Dau1_mass",&matchGen_D0Dau1_mass);
            // t->SetBranchAddress("matchGen_D0Dau1_y",&matchGen_D0Dau1_y);
            // t->SetBranchAddress("matchGen_D0Dau1_charge",&matchGen_D0Dau1_charge);
            // t->SetBranchAddress("matchGen_D0Dau1_pdgId",&matchGen_D0Dau1_pdgId);
            // t->SetBranchAddress("matchGen_D0Dau2_pT",&matchGen_D0Dau2_pT);
            // t->SetBranchAddress("matchGen_D0Dau2_eta",&matchGen_D0Dau2_eta);
            // t->SetBranchAddress("matchGen_D0Dau2_phi",&matchGen_D0Dau2_phi);
            // t->SetBranchAddress("matchGen_D0Dau2_mass",&matchGen_D0Dau2_mass);
            // t->SetBranchAddress("matchGen_D0Dau2_y",&matchGen_D0Dau2_y);
            // t->SetBranchAddress("matchGen_D0Dau2_charge",&matchGen_D0Dau2_charge);
            // t->SetBranchAddress("matchGen_D0Dau2_pdgId",&matchGen_D0Dau2_pdgId);
            // t->SetBranchAddress("matchGen_D1pT",&matchGen_D1pT);
            // t->SetBranchAddress("matchGen_D1eta",&matchGen_D1eta);
            // t->SetBranchAddress("matchGen_D1phi",&matchGen_D1phi);
            // t->SetBranchAddress("matchGen_D1mass",&matchGen_D1mass);
            // t->SetBranchAddress("matchGen_D1y",&matchGen_D1y);
            // t->SetBranchAddress("matchGen_D1charge",&matchGen_D1charge);
            // t->SetBranchAddress("matchGen_D1pdgId",&matchGen_D1pdgId);
            // t->SetBranchAddress("matchGen_D1decayLength2D_",&matchGen_D1decayLength2D_);
            // t->SetBranchAddress("matchGen_D1decayLength3D_",&matchGen_D1decayLength3D_);
            // t->SetBranchAddress("matchGen_D1angle2D_",&matchGen_D1angle2D_);
            // t->SetBranchAddress("matchGen_D1angle3D_",&matchGen_D1angle3D_);
            // t->SetBranchAddress("matchGen_D1ancestorId_",&matchGen_D1ancestorId_);
            // t->SetBranchAddress("matchGen_D1ancestorFlavor_",&matchGen_D1ancestorFlavor_);
            t->SetBranchAddress("massDaugther1",&massDaugther1);
            t->SetBranchAddress("pTD1",&pTD1);
            t->SetBranchAddress("EtaD1",&EtaD1);
            t->SetBranchAddress("PhiD1",&PhiD1);
            t->SetBranchAddress("VtxProbDaugther1",&VtxProbDaugther1);
            t->SetBranchAddress("VtxChi2Daugther1",&VtxChi2Daugther1);
            t->SetBranchAddress("VtxNDFDaugther1",&VtxNDFDaugther1);
            t->SetBranchAddress("3DCosPointingAngleDaugther1",&_3DCosPointingAngleDaugther1);
            t->SetBranchAddress("3DPointingAngleDaugther1",&_3DPointingAngleDaugther1);
            t->SetBranchAddress("2DCosPointingAngleDaugther1",&_2DCosPointingAngleDaugther1);
            t->SetBranchAddress("2DPointingAngleDaugther1",&_2DPointingAngleDaugther1);
            t->SetBranchAddress("3DDecayLengthSignificanceDaugther1",&_3DDecayLengthSignificanceDaugther1);
            t->SetBranchAddress("3DDecayLengthDaugther1",&_3DDecayLengthDaugther1);
            t->SetBranchAddress("3DDecayLengthErrorDaugther1",&_3DDecayLengthErrorDaugther1);
            t->SetBranchAddress("2DDecayLengthSignificanceDaugther1",&_2DDecayLengthSignificanceDaugther1);
            t->SetBranchAddress("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
            t->SetBranchAddress("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
            t->SetBranchAddress("NHitD2",&NHitD2);
            t->SetBranchAddress("HighPurityDaugther2",&HighPurityDaugther2);
            t->SetBranchAddress("pTD2",&pTD2);
            t->SetBranchAddress("EtaD2",&EtaD2);
            t->SetBranchAddress("PhiD2",&PhiD2);
            t->SetBranchAddress("pTerrD1",&pTerrD1);
            t->SetBranchAddress("pTerrD2",&pTerrD2);
            t->SetBranchAddress("dedxHarmonic2D2",&dedxHarmonic2D2);
            t->SetBranchAddress("zDCASignificanceGrandDaugther1",&zDCASignificanceGrandDaugther1);
            t->SetBranchAddress("zDCASignificanceGrandDaugther2",&zDCASignificanceGrandDaugther2);
            t->SetBranchAddress("xyDCASignificanceGrandDaugther1",&xyDCASignificanceGrandDaugther1);
            t->SetBranchAddress("xyDCASignificanceGrandDaugther2",&xyDCASignificanceGrandDaugther2);
            t->SetBranchAddress("NHitGrandD1",&NHitGrandD1);
            t->SetBranchAddress("NHitGrandD2",&NHitGrandD2);
            t->SetBranchAddress("HighPurityGrandDaugther1",&HighPurityGrandDaugther1);
            t->SetBranchAddress("HighPurityGrandDaugther2",&HighPurityGrandDaugther2);
            t->SetBranchAddress("pTGrandD1",&pTGrandD1);
            t->SetBranchAddress("pTGrandD2",&pTGrandD2);
            t->SetBranchAddress("pTerrGrandD1",&pTerrGrandD1);
            t->SetBranchAddress("pTerrGrandD2",&pTerrGrandD2);
            t->SetBranchAddress("EtaGrandD1",&EtaGrandD1);
            t->SetBranchAddress("EtaGrandD2",&EtaGrandD2);
            t->SetBranchAddress("dedxHarmonic2GrandD1",&dedxHarmonic2GrandD1);
            t->SetBranchAddress("dedxHarmonic2GrandD2",&dedxHarmonic2GrandD2);
            t->SetBranchAddress("Trk3DDCA",&Trk3DDCA);
            t->SetBranchAddress("Trk3DDCAErr",&Trk3DDCAErr);
            t->SetBranchAddress("dca3D",&dca3D);
	    t->SetBranchAddress("mva",&mva);};
            template <typename T>
        void setOutputTree(T *t){
            t->Branch("Ntrkoffline",&Ntrkoffline);
            t->Branch("Npixel",&Npixel);
            t->Branch("HFsumETPlus",&HFsumETPlus);
            t->Branch("HFsumETMinus",&HFsumETMinus);
            t->Branch("ZDCPlus",&ZDCPlus);
            t->Branch("ZDCMinus",&ZDCMinus);
            t->Branch("bestvtxX",&bestvtxX);
            t->Branch("bestvtxY",&bestvtxY);
            t->Branch("bestvtxZ",&bestvtxZ);
            t->Branch("candSize",&candSize);
            t->Branch("pT",&pT);
            t->Branch("y",&y);
            t->Branch("eta",&eta);
            t->Branch("phi",&phi);
            t->Branch("mass",&mass);
            t->Branch("flavor",&flavor);
            t->Branch("VtxProb",&VtxProb);
            t->Branch("VtxChi2",&VtxChi2);
            t->Branch("VtxNDF",&VtxNDF);
            t->Branch("3DCosPointingAngle",&_3DCosPointingAngle);
            t->Branch("3DPointingAngle",&_3DPointingAngle);
            t->Branch("2DCosPointingAngle",&_2DCosPointingAngle);
            t->Branch("2DPointingAngle",&_2DPointingAngle);
            t->Branch("3DDecayLengthSignificance",&_3DDecayLengthSignificance);
            t->Branch("3DDecayLength",&_3DDecayLength);
            t->Branch("2DDecayLengthSignificance",&_2DDecayLengthSignificance);
            t->Branch("2DDecayLength",&_2DDecayLength);
            t->Branch("massDaugther1",&massDaugther1);
            t->Branch("pTD1",&pTD1);
            t->Branch("EtaD1",&EtaD1);
            t->Branch("PhiD1",&PhiD1);
            t->Branch("VtxProbDaugther1",&VtxProbDaugther1);
            t->Branch("VtxChi2Daugther1",&VtxChi2Daugther1);
            t->Branch("VtxNDFDaugther1",&VtxNDFDaugther1);
            t->Branch("3DCosPointingAngleDaugther1",&_3DCosPointingAngleDaugther1);
            t->Branch("3DPointingAngleDaugther1",&_3DPointingAngleDaugther1);
            t->Branch("2DCosPointingAngleDaugther1",&_2DCosPointingAngleDaugther1);
            t->Branch("2DPointingAngleDaugther1",&_2DPointingAngleDaugther1);
            t->Branch("3DDecayLengthSignificanceDaugther1",&_3DDecayLengthSignificanceDaugther1);
            t->Branch("3DDecayLengthDaugther1",&_3DDecayLengthDaugther1);
            t->Branch("3DDecayLengthErrorDaugther1",&_3DDecayLengthErrorDaugther1);
            t->Branch("2DDecayLengthSignificanceDaugther1",&_2DDecayLengthSignificanceDaugther1);
            t->Branch("zDCASignificanceDaugther2",&zDCASignificanceDaugther2);
            t->Branch("xyDCASignificanceDaugther2",&xyDCASignificanceDaugther2);
            t->Branch("NHitD2",&NHitD2);
            t->Branch("HighPurityDaugther2",&HighPurityDaugther2);
            t->Branch("pTD2",&pTD2);
            t->Branch("EtaD2",&EtaD2);
            t->Branch("PhiD2",&PhiD2);
            t->Branch("pTerrD1",&pTerrD1);
            t->Branch("pTerrD2",&pTerrD2);
            t->Branch("dedxHarmonic2D2",&dedxHarmonic2D2);
            t->Branch("zDCASignificanceGrandDaugther1",&zDCASignificanceGrandDaugther1);
            t->Branch("zDCASignificanceGrandDaugther2",&zDCASignificanceGrandDaugther2);
            t->Branch("xyDCASignificanceGrandDaugther1",&xyDCASignificanceGrandDaugther1);
            t->Branch("xyDCASignificanceGrandDaugther2",&xyDCASignificanceGrandDaugther2);
            t->Branch("NHitGrandD1",&NHitGrandD1);
            t->Branch("NHitGrandD2",&NHitGrandD2);
            t->Branch("HighPurityGrandDaugther1",&HighPurityGrandDaugther1);
            t->Branch("HighPurityGrandDaugther2",&HighPurityGrandDaugther2);
            t->Branch("pTGrandD1",&pTGrandD1);
            t->Branch("pTGrandD2",&pTGrandD2);
            t->Branch("pTerrGrandD1",&pTerrGrandD1);
            t->Branch("pTerrGrandD2",&pTerrGrandD2);
            t->Branch("EtaGrandD1",&EtaGrandD1);
            t->Branch("EtaGrandD2",&EtaGrandD2);
            t->Branch("dedxHarmonic2GrandD1",&dedxHarmonic2GrandD1);
            t->Branch("dedxHarmonic2GrandD2",&dedxHarmonic2GrandD2);
            t->Branch("Trk3DDCA",&Trk3DDCA);
            t->Branch("Trk3DDCAErr",&Trk3DDCAErr);
            t->Branch("dca3D",&dca3D);
            t->Branch("mva",&mva);
            t->Branch("isMC",&isMC);
            // t->SetBranchAddress("candSize_gen",&candSize_gen);
            // t->SetBranchAddress("gen_pT",&gen_pT);
            // t->SetBranchAddress("gen_eat",&gen_eat);
            // t->SetBranchAddress("gen_y",&gen_y);
            // t->SetBranchAddress("gen_status",&gen_status);
            // t->SetBranchAddress("gen_MotherID",&gen_MotherID);
            // t->SetBranchAddress("gen_DauID1",&gen_DauID1);
            // t->SetBranchAddress("gen_DauID2",&gen_DauID2);
            // t->SetBranchAddress("gen_DauID3",&gen_DauID3);
            // t->SetBranchAddress("gen_D0pT",&gen_D0pT);
            // t->SetBranchAddress("gen_D0eta",&gen_D0eta);
            // t->SetBranchAddress("gen_D0phi",&gen_D0phi);
            // t->SetBranchAddress("gen_D0mass",&gen_D0mass);
            // t->SetBranchAddress("gen_D0y",&gen_D0y);
            // t->SetBranchAddress("gen_D0charge",&gen_D0charge);
            // t->SetBranchAddress("gen_D0pdgId",&gen_D0pdgId);
            // t->SetBranchAddress("gen_D0Dau1_pT",&gen_D0Dau1_pT);
            // t->SetBranchAddress("gen_D0Dau1_eta",&gen_D0Dau1_eta);
            // t->SetBranchAddress("gen_D0Dau1_phi",&gen_D0Dau1_phi);
            // t->SetBranchAddress("gen_D0Dau1_mass",&gen_D0Dau1_mass);
            // t->SetBranchAddress("gen_D0Dau1_y",&gen_D0Dau1_y);
            // t->SetBranchAddress("gen_D0Dau1_charge",&gen_D0Dau1_charge);
            // t->SetBranchAddress("gen_D0Dau1_pdgId",&gen_D0Dau1_pdgId);
            // t->SetBranchAddress("gen_D0Dau2_pT",&gen_D0Dau2_pT);
            // t->SetBranchAddress("gen_D0Dau2_eta",&gen_D0Dau2_eta);
            // t->SetBranchAddress("gen_D0Dau2_phi",&gen_D0Dau2_phi);
            // t->SetBranchAddress("gen_D0Dau2_mass",&gen_D0Dau2_mass);
            // t->SetBranchAddress("gen_D0Dau2_y",&gen_D0Dau2_y);
            // t->SetBranchAddress("gen_D0Dau2_charge",&gen_D0Dau2_charge);
            // t->SetBranchAddress("gen_D0Dau2_pdgId",&gen_D0Dau2_pdgId);
            // t->SetBranchAddress("gen_D1pT",&gen_D1pT);
            // t->SetBranchAddress("gen_D1eta",&gen_D1eta);
            // t->SetBranchAddress("gen_D1phi",&gen_D1phi);
            // t->SetBranchAddress("gen_D1mass",&gen_D1mass);
            // t->SetBranchAddress("gen_D1y",&gen_D1y);
            // t->SetBranchAddress("gen_D1charge",&gen_D1charge);
            // t->SetBranchAddress("gen_D1pdgId",&gen_D1pdgId);
    };
        void copyDn( simpleDStarDataTreeevt& evt, int idx){
            Ntrkoffline=evt.Ntrkoffline;
            Npixel=evt.Npixel;
            HFsumETPlus=evt.HFsumETPlus;
            HFsumETMinus=evt.HFsumETMinus;
            ZDCPlus=evt.ZDCPlus;
            ZDCMinus=evt.ZDCMinus;
            bestvtxX=evt.bestvtxX;
            bestvtxY=evt.bestvtxY;
            bestvtxZ=evt.bestvtxZ;
            candSize=evt.candSize;
            pT=evt.pT[idx];
            y=evt.y[idx];
            eta=evt.eta[idx];
            phi=evt.phi[idx];
            mass=evt.mass[idx];
            flavor=evt.flavor[idx];
            VtxProb=evt.VtxProb[idx];
            VtxChi2=evt.VtxChi2[idx];
            VtxNDF=evt.VtxNDF[idx];
            _3DCosPointingAngle=evt._3DCosPointingAngle[idx];
            _3DPointingAngle=evt._3DPointingAngle[idx];
            _2DCosPointingAngle=evt._2DCosPointingAngle[idx];
            _2DPointingAngle=evt._2DPointingAngle[idx];
            _3DDecayLengthSignificance=evt._3DDecayLengthSignificance[idx];
            _3DDecayLength=evt._3DDecayLength[idx];
            _2DDecayLengthSignificance=evt._2DDecayLengthSignificance[idx];
            _2DDecayLength=evt._2DDecayLength[idx];
            massDaugther1=evt.massDaugther1[idx];
            pTD1=evt.pTD1[idx];
            EtaD1=evt.EtaD1[idx];
            PhiD1=evt.PhiD1[idx];
            VtxProbDaugther1=evt.VtxProbDaugther1[idx];
            VtxChi2Daugther1=evt.VtxChi2Daugther1[idx];
            VtxNDFDaugther1=evt.VtxNDFDaugther1[idx];
            _3DCosPointingAngleDaugther1=evt._3DCosPointingAngleDaugther1[idx];
            _3DPointingAngleDaugther1=evt._3DPointingAngleDaugther1[idx];
            _2DCosPointingAngleDaugther1=evt._2DCosPointingAngleDaugther1[idx];
            _2DPointingAngleDaugther1=evt._2DPointingAngleDaugther1[idx];
            _3DDecayLengthSignificanceDaugther1=evt._3DDecayLengthSignificanceDaugther1[idx];
            _3DDecayLengthDaugther1=evt._3DDecayLengthDaugther1[idx];
            _3DDecayLengthErrorDaugther1=evt._3DDecayLengthErrorDaugther1[idx];
            _2DDecayLengthSignificanceDaugther1=evt._2DDecayLengthSignificanceDaugther1[idx];
            zDCASignificanceDaugther2=evt.zDCASignificanceDaugther2[idx];
            xyDCASignificanceDaugther2=evt.xyDCASignificanceDaugther2[idx];
            NHitD2=evt.NHitD2[idx];
            HighPurityDaugther2=evt.HighPurityDaugther2[idx];
            pTD2=evt.pTD2[idx];
            EtaD2=evt.EtaD2[idx];
            PhiD2=evt.PhiD2[idx];
            pTerrD1=evt.pTerrD1[idx];
            pTerrD2=evt.pTerrD2[idx];
            dedxHarmonic2D2=evt.dedxHarmonic2D2[idx];
            zDCASignificanceGrandDaugther1=evt.zDCASignificanceGrandDaugther1[idx];
            zDCASignificanceGrandDaugther2=evt.zDCASignificanceGrandDaugther2[idx];
            xyDCASignificanceGrandDaugther1=evt.xyDCASignificanceGrandDaugther1[idx];
            xyDCASignificanceGrandDaugther2=evt.xyDCASignificanceGrandDaugther2[idx];
            NHitGrandD1=evt.NHitGrandD1[idx];
            NHitGrandD2=evt.NHitGrandD2[idx];
            HighPurityGrandDaugther1=evt.HighPurityGrandDaugther1[idx];
            HighPurityGrandDaugther2=evt.HighPurityGrandDaugther2[idx];
            pTGrandD1=evt.pTGrandD1[idx];
            pTGrandD2=evt.pTGrandD2[idx];
            pTerrGrandD1=evt.pTerrGrandD1[idx];
            pTerrGrandD2=evt.pTerrGrandD2[idx];
            EtaGrandD1=evt.EtaGrandD1[idx];
            EtaGrandD2=evt.EtaGrandD2[idx];
            dedxHarmonic2GrandD1=evt.dedxHarmonic2GrandD1[idx];
            dedxHarmonic2GrandD2=evt.dedxHarmonic2GrandD2[idx];
            Trk3DDCA=evt.Trk3DDCA[idx];
            Trk3DDCAErr=evt.Trk3DDCAErr[idx];
            dca3D=evt.dca3D[idx];
            mva=evt.mva[idx];
            };
    };
 
}
#endif
