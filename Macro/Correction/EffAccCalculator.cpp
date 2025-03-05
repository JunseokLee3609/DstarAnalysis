#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.hxx"
#include "../interface/simpleAlgos.hxx"



void EffAccCalculator(
    // ANA::selBase selMVA,
    // ANA::selBase selDCA,
    string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root",
    // string fileMCNP,
    string subDir = "",
    bool cutBased = false, 
    int mvaV = 0,
    double Direction_y = 1.0
){
	// TFile* fDz = TFile::Open("outVz.root");
	// TH1D* hDzRatio = (TH1D*) fDz->Get("hRatio");

    // double mvaCut = selMVA.low;
    double dcaCut = 0; // e.g. 0--0.01

    // std::map<MC, TH1D*> histoAgl;
    // histoAgl[MC::kPR] = new TH1D("agl_pr", "",nagl, &corrParams::avec()[0]);
    // histoAgl[MC::kNP] = new TH1D("agl_np", "",nagl, &corrParams::avec()[0]);
    std::map<MC, TH1D*> histoPt;
    histoPt[MC::kPR] = new TH1D("pt_pr", "",npt, &corrParams::pvec()[0]);
    // histoPt[MC::kNP] = new TH1D("pt_np", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoY;
    histoY[MC::kPR] = new TH1D("y_pr", "",ny, &corrParams::yvec()[0]);
    // histoY[MC::kNP] = new TH1D("y_np", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhi;
    histoPhi[MC::kPR] = new TH1D("phi_pr", "",nphi, phil, phih);
    // histoPhi[MC::kNP] = new TH1D("phi_np", "",nphi, phil, phih);
    std::map<MC, TH2D*> histoPtY;
    histoPtY[MC::kPR] = new TH2D("pt_y_pr", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    // histoPtY[MC::kNP] = new TH2D("pt_y_np", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhi;
    histoYPhi[MC::kPR] = new TH2D("y_phi_pr", "",ny, yl, yh, nphi, phil, phih);
    // histoYPhi[MC::kNP] = new TH2D("y_phi_np", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPt;
    histoPhiPt[MC::kPR] = new TH2D("phi_pt_pr", "",nphi, phil, phih, npt, pl, ph);
    // histoPhiPt[MC::kNP] = new TH2D("phi_pt_np", "",nphi, phil, phih, npt, pl, ph);

    // std::map<MC, TH1D*> histoAglGenPass;
    // histoAglGenPass[MC::kPR] = new TH1D("agl_pr_genpass", "",nagl, &corrParams::avec()[0]);
    // histoAglGenPass[MC::kNP] = new TH1D("agl_np_genpass", "",nagl, &corrParams::avec()[0]);
    std::map<MC, TH1D*> histoPtGenPass;
    histoPtGenPass[MC::kPR] = new TH1D("pt_pr_genpass", "",npt, &corrParams::pvec()[0]);
    // histoPtGenPass[MC::kNP] = new TH1D("pt_np_genpass", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoYGenPass;
    histoYGenPass[MC::kPR] = new TH1D("y_pr_genpass", "",ny, &corrParams::yvec()[0]);
    // histoYGenPass[MC::kNP] = new TH1D("y_np_genpass", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhiGenPass;
    histoPhiGenPass[MC::kPR] = new TH1D("phi_pr_genpass", "",nphi, phil, phih);
    // histoPhiGenPass[MC::kNP] = new TH1D("phi_np_genpass", "",nphi, phil, phih);
    std::map<MC, TH2D*> histoPtYGenPass;
    histoPtYGenPass[MC::kPR] = new TH2D("pt_y_pr_genpass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    // histoPtYGenPass[MC::kNP] = new TH2D("pt_y_np_genpass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhiGenPass;
    histoYPhiGenPass[MC::kPR] = new TH2D("y_phi_pr_genpass", "",ny, yl, yh, nphi, phil, phih);
    // histoYPhiGenPass[MC::kNP] = new TH2D("y_phi_np_genpass", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPtGenPass;
    histoPhiPtGenPass[MC::kPR] = new TH2D("phi_pt_pr_genpass", "",nphi, phil, phih, npt, pl, ph);
    // histoPhiPtGenPass[MC::kNP] = new TH2D("phi_pt_np_genpass", "",nphi, phil, phih, npt, pl, ph);

    // std::map<MC, TH1D*> histoAglPass;
    // histoAglPass[MC::kPR] = new TH1D("agl_pr_pass", "",nagl, &corrParams::avec()[0]);
    // histoAglPass[MC::kNP] = new TH1D("agl_np_pass", "",nagl, &corrParams::avec()[0]);
    std::map<MC, TH1D*> histoPtPass;
    histoPtPass[MC::kPR] = new TH1D("pt_pr_pass", "",npt, &corrParams::pvec()[0]);
    // histoPtPass[MC::kNP] = new TH1D("pt_np_pass", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoYPass;
    histoYPass[MC::kPR] = new TH1D("y_pr_pass", "",ny, &corrParams::yvec()[0]);
    // histoYPass[MC::kNP] = new TH1D("y_np_pass", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhiPass;
    histoPhiPass[MC::kPR] = new TH1D("phi_pr_pass", "",nphi, phil, phih);
    // histoPhiPass[MC::kNP] = new TH1D("phi_np_pass", "",nphi, phil, phih);
    std::map<MC, TH2D*> histoPtYPass;
    histoPtYPass[MC::kPR] = new TH2D("pt_y_pr_pass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    // histoPtYPass[MC::kNP] = new TH2D("pt_y_np_pass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhiPass;
    histoYPhiPass[MC::kPR] = new TH2D("y_phi_pr_pass", "",ny, yl, yh, nphi, phil, phih);
    // histoYPhiPass[MC::kNP] = new TH2D("y_phi_np_pass", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPtPass;
    histoPhiPtPass[MC::kPR] = new TH2D("phi_pt_pr_pass", "",nphi, phil, phih, npt, pl, ph);
    // histoPhiPtPass[MC::kNP] = new TH2D("phi_pt_np_pass", "",nphi, phil, phih, npt, pl, ph);

    std::map<MC, TH3D*> histo3D;
    histo3D[MC::kPR]  = new TH3D("pt_y_phi_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    // histo3D[MC::kNP]  = new TH3D("pt_y_phi_np", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    std::map<MC, TH3D*> histo3DGenPass;
    histo3DGenPass[MC::kPR]  = new TH3D("pt_y_phi_pr_genpass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    // histo3DGenPass[MC::kNP]  = new TH3D("pt_y_phi_np_genpass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    std::map<MC, TH3D*> histo3DPass;
    histo3DPass[MC::kPR]  = new TH3D("pt_y_phi_pr_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    // histo3DPass[MC::kNP]  = new TH3D("pt_y_phi_np_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );


    DataFormat::simpleDStarMCTreeevt evtPR;
    DataFormat::simpleDStarDataTreeflat evtData;

    string name_tree = "dStarana_mc/VertexCompositeNtuple";
    int count = 0;
    int countgen = 0;


    /* Get file*/
    TFile* fMCPR =  TFile::Open(fileMCPR.c_str());
    // TFile* fMCNP =  TFile::Open(fileMCNP.c_str());

    /* Get tree*/
    auto tMCPR = (TTree*) fMCPR->Get(name_tree.c_str());
    // auto tMCNP = (TTree*) fMCNP->Get(name_tree.c_str());

    /* Set tree*/
    evtPR.setTree(tMCPR);
    evtPR.setGENTree(tMCPR);
    // evtNP.setTree(tMCNP);

    long long numEvtPR = tMCPR->GetEntries();
    // numEvtPR = 100000;
    // long long numEvtNP = tMCNP->GetEntries();

    TFile* outputFile = new TFile(Form("output.root"),"recreate");

    /* Histo Filling Algorithm, takes index and will run every loop that is declared after*/

    auto fillGen = [&](int igen, double dzW, MC mcID){
        if((
            // abs(evtPR.gen_D1ancestorId[igen]) < 500 && 
            // std::min(abs(evtPR.DauID1_gen[igen]), abs(evtPR.DauID2_gen[igen])) ==PDG_DAU1 && 
            // std::max(abs(evtPR.DauID1_gen[igen]), abs(evtPR.DauID2_gen[igen])) ==PDG_DAU2 && 
            evtPR.gen_pT[igen] > DSGLPTLO && 
            // evtPR.gen_pT[igen] < DSGLPTHI &&
            // evtPR.gen_D0pT[igen] > DSGLPT_DAU1 &&
            // evtPR.gen_D1pT[igen] > DSGLPT_DAU2 && 
            // fabs(evtPR.gen_D0eta[igen]) < DSGLABSETA_DAU1 &&
            // fabs(evtPR.gen_D1eta[igen]) < DSGLABSETA_DAU2 && 
            fabs(evtPR.gen_y[igen]) < DSGLABSY && 
            // evtPR.angle2D_gen[igen] < 1.0 && 
            // evtPR.angle3D_gen[igen] < 1.0 && 
            true
        )) {
            countgen++;
            histoPt[mcID]->Fill(evtPR.gen_pT[igen] , dzW);
            // histoAgl[mcID]->Fill(evtPR.angle3D_gen[igen] , dzW);
            histoY[mcID]->Fill(Direction_y* evtPR.gen_y[igen] , dzW);
            histoPhi[mcID]->Fill(evtPR.gen_phi[igen] , dzW);
            histoPtY[mcID]->Fill(evtPR.gen_pT[igen], Direction_y * evtPR.gen_y[igen] , dzW);
            histoYPhi[mcID]->Fill(Direction_y* evtPR.gen_y[igen], evtPR.gen_phi[igen] , dzW);
            histoPhiPt[mcID]->Fill(evtPR.gen_phi[igen], evtPR.gen_pT[igen] , dzW);
            histo3D[mcID] ->Fill(evtPR.gen_pT[igen], Direction_y* evtPR.gen_y[igen], evtPR.gen_phi[igen], dzW);
            if( 
                // std::min(abs(evtPR.DauID1_gen[igen]), abs(evtPR.DauID2_gen[igen])) ==PDG_DAU1 && 
                // std::max(abs(evtPR.DauID1_gen[igen]), abs(evtPR.DauID2_gen[igen])) ==PDG_DAU2 && 
                fabs(evtPR.gen_D0y[igen]) < DSGLABSY_D0 &&
                fabs(evtPR.gen_D1eta[igen]) < DSGLABSETA_D1 && 
                evtPR.gen_D1pT[igen] > DSGLPT_D1 && 
                fabs(evtPR.gen_D0Dau1_eta[igen]) < DSGLABSETA_D0DAU1 &&
                fabs(evtPR.gen_D0Dau2_eta[igen]) < DSGLABSETA_D0DAU2 &&
                evtPR.gen_D0Dau1_pT[igen] > DSGLPT_D0DAU1 &&
                evtPR.gen_D0Dau2_pT[igen] > DSGLPT_D0DAU2 &&
                true
            ){
                histoPtGenPass[mcID]->Fill(evtPR.gen_pT[igen]  , dzW);
                // histoAglGenPass[mcID]->Fill(evtPR.angle3D_gen[igen]  , dzW);
                histoYGenPass[mcID]->Fill(Direction_y * evtPR.gen_y[igen]  , dzW);
                histoPhiGenPass[mcID]->Fill(evtPR.gen_phi[igen]  , dzW);
                histoPtYGenPass[mcID]->Fill(evtPR.gen_pT[igen], Direction_y * evtPR.gen_y[igen]  , dzW);
                histoYPhiGenPass[mcID]->Fill(Direction_y * evtPR.gen_y[igen], evtPR.gen_phi[igen]  , dzW);
                histoPhiPtGenPass[mcID]->Fill(evtPR.gen_phi[igen], evtPR.gen_pT[igen]  , dzW);
                histo3DGenPass[mcID] ->Fill(evtPR.gen_pT[igen], Direction_y * evtPR.gen_y[igen], evtPR.gen_phi[igen], dzW);
            }
        }
    };
    auto fillReco = [&](int ireco, double dzW, MC mcID){
        if( evtPR.matchGEN[ireco]  && !evtPR.isSwap[ireco] ){ 
            auto D0y= algo::rapidity(evtPR.pTD1[ireco], evtPR.EtaD1[ireco], evtPR.phi[ireco], evtPR.massDaugther1[ireco]);
            
            bool cutPassed = false;
            if(cutBased){
                if(mvaV<4) cutPassed = ( (mvaV ==1  || evtPR._3DDecayLengthSignificance[ireco] > 3.5) && (mvaV == 2 || evtPR._3DPointingAngle[ireco] < 0.13) && ( mvaV == 3 || evtPR.VtxProb[ireco] > 0.1) );
                else{
                    if(mvaV == 4) cutPassed = evtPR._3DDecayLengthSignificance[ireco] > 3.5 ;
                    if(mvaV == 5) cutPassed = evtPR._3DPointingAngle[ireco] < 0.13 ;
                    if(mvaV == 6) cutPassed = evtPR.VtxProb[ireco] > 0.1 ;
                }                  
            }
            if ((
                // ( 
                //     (fabs(evtPR.pTD1[ireco] -evtPR.gen_D1pTD1[ireco])/evtPR.gen_D1pTD1[ireco] < 0.3 && 
                //     fabs(evtPR.pTD2[ireco] -evtPR.gen_D1pTD2[ireco])/evtPR.gen_D1pTD2[ireco] < 0.3 ) || 
                //     (fabs(evtPR.pTD2[ireco] -evtPR.gen_D1pTD1[ireco])/evtPR.gen_D1pTD1[ireco] < 0.3 && 
                //     fabs(evtPR.pTD1[ireco] -evtPR.gen_D1pTD2[ireco])/evtPR.gen_D1pTD2[ireco] < 0.3) 
                // ) && 
                // ( dR(evtPR.EtaD1[ireco] -evtPR.gen_D1etaD1[ireco],(evtPR.PhiD1[ireco] -evtPR.gen_D1phiD1[ireco] )) < 0.03) &&
                // ( dR(evtPR.EtaD2[ireco] -evtPR.gen_D1etaD2[ireco],(evtPR.PhiD2[ireco] -evtPR.gen_D1phiD2[ireco] )) < 0.03) &&
                // evtPR.HighPuritydaughter1[ireco] && evtPR.HighPuritydaughter2[ireco]
                // Candiate
                // && (evtPR.NHitD1[ireco] > DSGLNHIT_DAU1 && evtPR.NHitD2[ireco] > DSGLNHIT_DAU2 )
                // true
                // (evtPR.pTD1[ireco] > DSGLPT_D0DAU1 && evtPR.pTD2[ireco] > DSGLPT_D0DAU2)
                // && (evtPR.pTerrD1[ireco] < DSGLPTERR_DAU1 && evtPR.pTerrD2[ireco] < DSGLPTERR_DAU2)
                // && abs(evtPR.EtaD1[ireco] -evtPR.EtaD2[ireco]) < DSGLETADIFF_DAU
                // && abs(evtPR.EtaD1[ireco]) < DSGLABSETA_DAU1 
                // && abs(evtPR.EtaD2[ireco]) < DSGLABSETA_DAU2
                // && (evtPR.pTD1[ireco] + evtPR.pTD2[ireco]) > DSGLPTSUM_DAU
                // // && ((evtPR.mva[ireco]-0.3)*DSGLMVASCALE +0.3 ) > mvaCut
                // // && evtPR.mva[ireco] > mvaCut
                // // && evtPR.mva[ireco] < selMVA.high
                // && evtPR._3DPointingAngle[ireco] < DSGL3DPA 
                // && evtPR._2DPointingAngle[ireco] < DSGL2DPA  
                // // && (TMath::Sin(evtPR._3DPointingAngle[ireco]) * evtPR._3DDecayLength[ireco]) < dcaCut
                // && (evtPR.pT[ireco] > DSGLPTLO && evtPR.pT[ireco]< DSGLPTHI)
                // && ( fabs(evtPR.y[ireco]) < DSGLABSY) 
                fabs(evtPR.EtaD2[ireco]) < DSGLABSETA_D1 && 
                evtPR.pTD2[ireco] > DSGLPT_D1 && 
                fabs(D0y) < DSGLABSY_D0 &&
                

                fabs(evtPR.EtaGrandD1[ireco]) < DSGLABSETA_D0DAU1 &&
                fabs(evtPR.EtaGrandD2[ireco]) < DSGLABSETA_D0DAU2 &&
                evtPR.pTGrandD1[ireco] > DSGLPT_D0DAU1 &&
                evtPR.pTGrandD2[ireco] > DSGLPT_D0DAU2 &&
                true
                // && ( !cutBased || cutPassed) // False or (True/False)
            )){
                count ++;
                    // histoAglPass[mcID]->Fill(evtPR.v3DPointingAngle[ireco]  , dzW);
                    histoPtPass[mcID]->Fill(evtPR.pT[ireco]  , dzW);
                    histoYPass[mcID]->Fill(Direction_y * evtPR.y[ireco]  , dzW);
                    histoPhiPass[mcID]->Fill(evtPR.phi[ireco]  , dzW);
                    histoPtYPass[mcID]->Fill(evtPR.pT[ireco], Direction_y *evtPR.y[ireco]  , dzW);
                    histoYPhiPass[mcID]->Fill(Direction_y * evtPR.y[ireco], evtPR.phi[ireco]  , dzW);
                    histoPhiPtPass[mcID]->Fill(evtPR.phi[ireco], evtPR.pT[ireco]  , dzW);
                    histo3DPass[mcID] ->Fill(evtPR.pT[ireco], Direction_y *evtPR.y[ireco], evtPR.phi[ireco], dzW);
            }
        }
    };

    /* Actual filling loop*/

    for( auto idx : ROOT::TSeqU(numEvtPR)){
        tMCPR->GetEntry(idx);
		// double dzW = hDzRatio->GetBinContent(hDzRatio->FindBin(evtPR.bestvtxZ));
        double dzW = 1.0;
        for( auto igen : ROOT::TSeqI(evtPR.candSize_gen)){
            fillGen(igen, dzW, MC::kPR);
        }
        for( auto ireco : ROOT::TSeqI(evtPR.candSize)){
            fillReco(ireco, dzW, MC::kPR);
        }
        // std::cout << std::endl;
    }
    // for( auto idx : ROOT::TSeqU(numEvtPR)){
    // for( auto idx : ROOT::TSeqU(100)){
    //     tMCPR->GetEntry(idx);
	// 	// double dzW = hDzRatio->GetBinContent(hDzRatio->FindBin(evtPR.bestvtxZ));
    //     double dzW = 1.0;
    //     for( auto igen : ROOT::TSeqI(evtPR.candSize_gen)){
    //         fillGen(igen, dzW, MC::kNP);
    //     }
    //     for( auto ireco : ROOT::TSeqI(evtPR.candSize)){
    //         fillReco(ireco, dzW, MC::kNP);
    //     }
    //     // std::cout << std::endl;
    // }

    auto ratio3DPR = (TH3D*) histo3DPass[MC::kPR]->Clone("pt_y_phi_pr_ratio");
    // auto ratio3DNP = (TH3D*) histo3DPass[MC::kNP]->Clone("pt_y_phi_np_ratio");
    auto ratio3DPRACC = (TH3D*) histo3DGenPass[MC::kPR]->Clone("pt_y_phi_pr_acc_ratio");
    // auto ratio3DNPACC = (TH3D*) histo3DGenPass[MC::kNP]->Clone("pt_y_phi_np_acc_ratio");

    auto ratio3DPRRECO = (TH3D*) histo3DPass[MC::kPR]->Clone("pt_y_phi_pr_reco_ratio");
    // auto ratio3DNPRECO = (TH3D*) histo3DPass[MC::kNP]->Clone("pt_y_phi_np_reco_ratio");
    ratio3DPR->SetName("pt_y_phi_pr_ratio");
    // ratio3DNP->SetName("pt_y_phi_np_ratio");
    ratio3DPR->Divide(ratio3DPR, histo3D[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    // ratio3DNP->Divide(ratio3DNP, histo3D[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");

    ratio3DPRACC->Divide(ratio3DPRACC, histo3D[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    // ratio3DNPACC->Divide(ratio3DNPACC, histo3D[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");

    ratio3DPRRECO->Divide(ratio3DPRRECO, histo3DGenPass[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    // ratio3DNPRECO->Divide(ratio3DNPRECO, histo3DGenPass[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");

    auto GetRatioPlot = [&](TH1D* num, TH1D* den){
        auto ratio1D =  (TH1D*) num->Clone(Form("%s_ratio",num->GetName()));
        ratio1D->Divide(den);
        ratio1D->GetYaxis()->SetRangeUser(0.0, ratio1D->GetMaximum()*1.2);
        return ratio1D;
    };

    // auto ratioAglPR = GetRatioPlot(histoAglPass[MC::kPR], histoAgl[MC::kPR]);
    // auto ratioAglNP = GetRatioPlot(histoAglPass[MC::kNP], histoAgl[MC::kNP]);
    auto ratioPtPR = GetRatioPlot(histoPtPass[MC::kPR], histoPt[MC::kPR]);
    // auto ratioPtNP = GetRatioPlot(histoPtPass[MC::kNP], histoPt[MC::kNP]);
    auto ratioYPR = GetRatioPlot(histoYPass[MC::kPR], histoY[MC::kPR]);
    // auto ratioYNP = GetRatioPlot(histoYPass[MC::kNP], histoY[MC::kNP]);
    auto ratioPhiPR = GetRatioPlot(histoPhiPass[MC::kPR], histoPhi[MC::kPR]);
    // auto ratioPhiNP = GetRatioPlot(histoPhiPass[MC::kNP], histoPhi[MC::kNP]);
    // ratioAglPR->SetLineColor(kRed);
    // ratioAglNP->SetLineColor(kBlue);
    ratioPtPR->SetLineColor(kRed);
    // ratioPtNP->SetLineColor(kBlue);
    ratioYPR->SetLineColor(kRed);
    // ratioYNP->SetLineColor(kBlue);
    ratioPhiPR->SetLineColor(kRed);
    // ratioPhiNP->SetLineColor(kBlue);

    TCanvas* c1 = new TCanvas("c1", "c", 400*3, 400*2);
    c1->Divide(3,2);
    c1->cd(1);
    ratio3DPR->Draw("box2");
    c1->cd(2);
    ratioPtPR->Draw();
    // ratioPtNP->Draw("same");
    c1->cd(3);
    ratioYPR->Draw();
    // ratioYNP->Draw("same");
    c1->cd(4);
    ratioPhiPR->Draw();
    // ratioPhiNP->Draw("same");
    // c1->cd(5);
    // ratioAglPR->Draw();
    // ratioAglNP->Draw("same");

    // c1->SaveAs(Form("../../output/histogram/root/efficiency/%sdistribution_%s_%s.png",subDir.c_str(), selMVA.name.c_str(), selDCA.name.c_str()));
    c1->SaveAs(Form("distributions.png"));

    std::cout << "Passing Gen  total : " << countgen << std::endl;
    std::cout << "Passing Reco total : " << count << std::endl;
    outputFile->cd();

    // histoAgl[MC::kPR]->Write();
    histoPt[MC::kPR]->Write();
    histoY[MC::kPR]->Write();
    histoPhi[MC::kPR]->Write();
    histoPtY[MC::kPR]->Write();
    histoYPhi[MC::kPR]->Write();
    histoPhiPt[MC::kPR]->Write();
    histo3D[MC::kPR] ->Write();
    histoPtGenPass[MC::kPR]->Write();
    // histoAglGenPass[MC::kPR]->Write();
    histoYGenPass[MC::kPR]->Write();
    histoPhiGenPass[MC::kPR]->Write();
    histoPtYGenPass[MC::kPR]->Write();
    histoYPhiGenPass[MC::kPR]->Write();
    histoPhiPtGenPass[MC::kPR]->Write();
    histoPtPass[MC::kPR]->Write();
    // histoAglPass[MC::kPR]->Write();
    histoYPass[MC::kPR]->Write();
    histoPhiPass[MC::kPR]->Write();
    histoPtYPass[MC::kPR]->Write();
    histoYPhiPass[MC::kPR]->Write();
    histoPhiPtPass[MC::kPR]->Write();
    histo3DGenPass[MC::kPR] ->Write();
    histo3DPass[MC::kPR] ->Write();
    // histoPt[MC::kNP]->Write();
    // histoY[MC::kNP]->Write();
    // histoPhi[MC::kNP]->Write();
    // histoPtY[MC::kNP]->Write();
    // histoYPhi[MC::kNP]->Write();
    // histoPhiPt[MC::kNP]->Write();
    // histo3D[MC::kPR] ->Write();
    // histo3D[MC::kNP] ->Write();
    // histoPtGenPass[MC::kNP]->Write();
    // // histoAglGenPass[MC::kNP]->Write();
    // histoYGenPass[MC::kNP]->Write();
    // histoPhiGenPass[MC::kNP]->Write();
    // histoPtYGenPass[MC::kNP]->Write();
    // histoYPhiGenPass[MC::kNP]->Write();
    // histoPhiPtGenPass[MC::kNP]->Write();
    // histoPtPass[MC::kNP]->Write();
    // // histoAglPass[MC::kNP]->Write();
    // histoYPass[MC::kNP]->Write();
    // histoPhiPass[MC::kNP]->Write();
    // histoPtYPass[MC::kNP]->Write();
    // histoYPhiPass[MC::kNP]->Write();
    // histoPhiPtPass[MC::kNP]->Write();
    // histo3DGenPass[MC::kNP] ->Write();
    // histo3DPass[MC::kNP] ->Write();
    ratio3DPR->Write();
    // ratio3DNP->Write();
    ratio3DPRACC->Write();
    // ratio3DNPACC->Write();
    ratio3DPRRECO->Write();
    // ratio3DNPRECO->Write();
    outputFile->Close();

}