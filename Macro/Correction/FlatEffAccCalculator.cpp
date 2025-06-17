#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.h"
#include "../interface/simpleAlgos.hxx"
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"



void FlatEffAccCalculator(
    // ANA::selBase selMVA,
    // ANA::selBase selDCA,
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Mar30NonSwap/d0ana_tree_nonswapsample_ppref_30Mar.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_0_06Apr25.root",
    //string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_1_07Apr25.root",
    string fileMCPR="",
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
    std::map<MC, TH1D*> histoCos;
    histoCos[MC::kPR] = new TH1D("cos_pr", "",ncos, coslo, coshi);
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
    std::map<MC, TH2D*> histoYCos;
    histoYCos[MC::kPR] = new TH2D("y_cos_pr", "",ny, yl, yh, ncos, coslo, coshi);
    // histoPtY[MC::kNP] = new TH2D("pt_y_np", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoPhiCos;
    histoPhiCos[MC::kPR] = new TH2D("phi_cos_pr", "", nphi, phil, phih, ncos, coslo, coshi );
    // histoYPhi[MC::kNP] = new TH2D("y_phi_np", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPtCos;
    histoPtCos[MC::kPR] = new TH2D("pt_cos_pr", "", npt, pl, ph, ncos, coslo, coshi);
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
    std::map<MC, TH1D*> histoCosPass;
    histoCosPass[MC::kPR] = new TH1D("cos_pr_pass", "",ncos, coslo, coshi );
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
    std::map<MC, TH2D*> histoYCosPass;
    histoYCosPass[MC::kPR] = new TH2D("y_cos_pr_pass", "",ny, yl, yh, ncos, coslo, coshi);
    // histoPtY[MC::kNP] = new TH2D("pt_y_np", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoPhiCosPass;
    histoPhiCosPass[MC::kPR] = new TH2D("phi_cos_pr_pass", "", nphi, phil, phih, ncos, coslo, coshi );
    // histoYPhi[MC::kNP] = new TH2D("y_phi_np", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPtCosPass;
    histoPtCosPass[MC::kPR] = new TH2D("pt_cos_pr_pass", "", npt, pl, ph, ncos, coslo, coshi);
    // histoPhiPt[MC::kNP] = new TH2D("phi_pt_np", "",nphi, phil, phih, npt, pl, ph);

    std::map<MC, TH3D*> histo3D;
    histo3D[MC::kPR]  = new TH3D("pt_y_phi_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    // histo3D[MC::kNP]  = new TH3D("pt_y_phi_np", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    std::map<MC, TH3D*> histo3DGenPass;
    histo3DGenPass[MC::kPR]  = new TH3D("pt_y_phi_pr_genpass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    // histo3DGenPass[MC::kNP]  = new TH3D("pt_y_phi_np_genpass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    std::map<MC, TH3D*> histo3DPass;
    histo3DPass[MC::kPR]  = new TH3D("pt_y_phi_pr_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    // histo3DPass[MC::kNP]  = new TH3D("pt_y_phi_np_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );



    DataFormat::simpleDStarMCTreeflat evtGenFlat;  // For Gen tree
    DataFormat::simpleDStarMCTreeflat evtRecoFlat; // For Reco tree
    
    /* Get files and trees */
    TFile* fMCPR = TFile::Open(fileMCPR.c_str());
    if (!fMCPR || fMCPR->IsZombie()) {
        std::cerr << "Error opening file: " << fileMCPR << std::endl;
        return;
    }
    
    // Get the flat trees
    TTree* genTree = (TTree*)fMCPR->Get("skimGENTreeFlat");
    TTree* recoTree = (TTree*)fMCPR->Get("skimTreeFlat");
    // auto tMCNP = (TTree*) fMCNP->Get(name_tree.c_str());
    if (!genTree || !recoTree) {
        std::cerr << "Error: Could not find required trees in the file" << std::endl;
        return;
    }

    /* Set tree*/
    evtGenFlat.setGENTree(genTree);
    evtRecoFlat.setTree(recoTree);
    long long numGenEntries = genTree->GetEntries();
    long long numRecoEntries = recoTree->GetEntries();
    // evtNP.setTree(tMCNP);

    // long long numEvtPR = tMCPR->GetEntries();
    // numEvtPR = 100000;
    // long long numEvtNP = tMCNP->GetEntries();

    TFile* outputFile = new TFile(Form("output.root"),"recreate");

    /* Histo Filling Algorithm, takes index and will run every loop that is declared after*/
    int countgen = 0;
    int count = 0;

    auto fillGen = [&](double dzW, MC mcID){
        if( evtGenFlat.gen_pT > DSGLPTLO && 
            fabs(evtGenFlat.gen_y) < DSGLABSY &&
            true){
            TLorentzVector Dstar,D0,D1,D0Dau1,D0Dau2;
            Dstar.SetPtEtaPhiM(evtGenFlat.gen_pT, evtGenFlat.gen_eta, evtGenFlat.gen_phi, evtGenFlat.gen_mass);
            D0.SetPtEtaPhiM(evtGenFlat.gen_D0pT, evtGenFlat.gen_D0eta, evtGenFlat.gen_D0phi, evtGenFlat.gen_D0mass);
            D1.SetPtEtaPhiM(evtGenFlat.gen_D1pT, evtGenFlat.gen_D1eta, evtGenFlat.gen_D1phi, evtGenFlat.gen_D1mass);
            D0Dau1.SetPtEtaPhiM(evtGenFlat.gen_D0Dau1_pT, evtGenFlat.gen_D0Dau1_eta, evtGenFlat.gen_D0Dau1_phi, evtGenFlat.gen_D0Dau1_mass);
            D0Dau2.SetPtEtaPhiM(evtGenFlat.gen_D0Dau2_pT, evtGenFlat.gen_D0Dau2_eta, evtGenFlat.gen_D0Dau2_phi, evtGenFlat.gen_D0Dau2_mass);
            TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);
            countgen++;
            histoPt[mcID]->Fill(evtGenFlat.gen_pT, dzW);
            histoY[mcID]->Fill(Direction_y * evtGenFlat.gen_y, dzW);
            histoCos[mcID]->Fill(vect.CosTheta(), dzW);
            histoPhi[mcID]->Fill(evtGenFlat.gen_phi, dzW);
            histoPtY[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, dzW);
            histoYCos[mcID]->Fill(Direction_y * evtGenFlat.gen_y, vect.CosTheta(), dzW);
            histoPhiCos[mcID]->Fill(evtGenFlat.gen_phi, vect.CosTheta(), dzW);
            histoPtCos[mcID]->Fill(evtGenFlat.gen_pT, vect.CosTheta(), dzW);
            histoYPhi[mcID]->Fill(Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            histoPhiPt[mcID]->Fill(evtGenFlat.gen_phi, evtGenFlat.gen_pT, dzW);
            histo3D[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            
            if (fabs(evtGenFlat.gen_D0y) < DSGLABSY_D0 &&
                fabs(evtGenFlat.gen_D1eta) < DSGLABSETA_D1 &&
                evtGenFlat.gen_D1pT > DSGLPT_D1 &&
                fabs(evtGenFlat.gen_D0Dau1_eta) < DSGLABSETA_D0DAU1 &&
                fabs(evtGenFlat.gen_D0Dau2_eta) < DSGLABSETA_D0DAU2 &&
                evtGenFlat.gen_D0Dau1_pT > DSGLPT_D0DAU1 &&
                evtGenFlat.gen_D0Dau2_pT > DSGLPT_D0DAU2 &&
                true) {
                
                histoPtGenPass[mcID]->Fill(evtGenFlat.gen_pT, dzW);
                histoYGenPass[mcID]->Fill(Direction_y * evtGenFlat.gen_y, dzW);
                histoPhiGenPass[mcID]->Fill(evtGenFlat.gen_phi, dzW);
                histoPtYGenPass[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, dzW);
                histoYPhiGenPass[mcID]->Fill(Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
                histoPhiPtGenPass[mcID]->Fill(evtGenFlat.gen_phi, evtGenFlat.gen_pT, dzW);
                histo3DGenPass[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            }
        }
    };
    auto fillReco = [&](double dzW, MC mcID){
        if (evtRecoFlat.matchGEN) {
            auto D0y = algo::rapidity(evtRecoFlat.pTD1, evtRecoFlat.EtaD1, evtRecoFlat.PhiD1, evtRecoFlat.massDaugther1);
            
            // Apply selection criteria
            if (fabs(evtRecoFlat.y) < DSGLABSY &&
                evtRecoFlat.pT > DSGLPTLO &&
                fabs(D0y) < DSGLABSY_D0 &&
                fabs(evtRecoFlat.EtaD2) < DSGLABSETA_D1 &&
                evtRecoFlat.pTD2 > DSGLPT_D1 &&
                fabs(evtRecoFlat.EtaGrandD1) < DSGLABSETA_D0DAU1 &&
                fabs(evtRecoFlat.EtaGrandD2) < DSGLABSETA_D0DAU2 &&
                evtRecoFlat.pTGrandD1 > DSGLPT_D0DAU1 &&
                evtRecoFlat.pTGrandD2 > DSGLPT_D0DAU2 &&
                true ) {
                
                TLorentzVector Dstar, D0;
                Dstar.SetPtEtaPhiM(evtRecoFlat.pT, evtRecoFlat.eta, evtRecoFlat.phi, evtRecoFlat.mass);
                D0.SetPtEtaPhiM(evtRecoFlat.pTD1, evtRecoFlat.EtaD1, evtRecoFlat.PhiD1, evtRecoFlat.massDaugther1);
                TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);
                
                count++;
                
                histoPtPass[mcID]->Fill(evtRecoFlat.pT, dzW);
                histoYPass[mcID]->Fill(Direction_y * evtRecoFlat.y, dzW);
                histoPhiPass[mcID]->Fill(evtRecoFlat.phi, dzW);
                histoCosPass[mcID]->Fill(vect.CosTheta(), dzW);
                histoPtYPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, dzW);
                histoYPhiPass[mcID]->Fill(Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                histoPhiPtPass[mcID]->Fill(evtRecoFlat.phi, evtRecoFlat.pT, dzW);
                histo3DPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                histoYCosPass[mcID]->Fill(Direction_y * evtRecoFlat.y, vect.CosTheta(), dzW);
                histoPhiCosPass[mcID]->Fill(evtRecoFlat.phi, vect.CosTheta(), dzW);
                histoPtCosPass[mcID]->Fill(evtRecoFlat.pT, vect.CosTheta(), dzW);
            }
        }
    };

    /* Actual filling loop*/

    for( auto idx : ROOT::TSeqU(numGenEntries)){
        genTree->GetEntry(idx);
		// double dzW = hDzRatio->GetBinContent(hDzRatio->FindBin(evtPR.bestvtxZ));
        double dzW = 1.0;
            fillGen(dzW, MC::kPR);
        // std::cout << std::endl;
    }
    for( auto idx : ROOT::TSeqU(numRecoEntries)){
        recoTree->GetEntry(idx);
        double dzW = 1.0;
		// double dzW = hDzRatio->GetBinContent(hDzRatio->FindBin(evtPR.bestvtxZ));
            fillReco(dzW, MC::kPR);
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

    auto Get2DRatioPlot = [&](TH2D* num, TH2D* den){
        auto ratio2D =  (TH2D*) num->Clone(Form("%s_ratio",num->GetName()));
        ratio2D->Divide(den);
        // ratio2D->GetYaxis()->SetRangeUser(ratio2D->GetMinimum()*1.2, ratio2D->GetMaximum()*1.2);
        // ratio2D->GetXaxis()->SetRangeUser(ratio2D->GetMinimum()*1.2, ratio2D->GetMaximum()*1.2);
        return ratio2D;
    };

    // auto ratioAglPR = GetRatioPlot(histoAglPass[MC::kPR], histoAgl[MC::kPR]);
    // auto ratioAglNP = GetRatioPlot(histoAglPass[MC::kNP], histoAgl[MC::kNP]);
    auto ratioPtPR = GetRatioPlot(histoPtPass[MC::kPR], histoPt[MC::kPR]);
    // auto ratioPtNP = GetRatioPlot(histoPtPass[MC::kNP], histoPt[MC::kNP]);
    auto ratioYPR = GetRatioPlot(histoYPass[MC::kPR], histoY[MC::kPR]);
    // auto ratioYNP = GetRatioPlot(histoYPass[MC::kNP], histoY[MC::kNP]);
    auto ratioPhiPR = GetRatioPlot(histoPhiPass[MC::kPR], histoPhi[MC::kPR]);
    auto ratioCosPR = GetRatioPlot(histoCosPass[MC::kPR], histoCos[MC::kPR]);
    auto ratioPtCosPR = Get2DRatioPlot(histoPtCosPass[MC::kPR], histoPtCos[MC::kPR]);
    // auto ratioPhiNP = GetRatioPlot(histoPhiPass[MC::kNP], histoPhi[MC::kNP]);
    // ratioAglPR->SetLineColor(kRed);
    // ratioAglNP->SetLineColor(kBlue);
    ratioPtPR->SetLineColor(kRed);
    // ratioPtNP->SetLineColor(kBlue);
    ratioYPR->SetLineColor(kRed);
    // ratioYNP->SetLineColor(kBlue);
    ratioPhiPR->SetLineColor(kRed);
    // ratioPhiNP->SetLineColor(kBlue);
    ratioCosPR->SetLineColor(kRed);

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
    c1->cd(5);
    ratioCosPR->Draw();
    c1->cd(6);
    ratioPtCosPR->Draw("colz");
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
    histoPtCos[MC::kPR]->Write();
    histoYCos[MC::kPR]->Write();
    histoPhiCos[MC::kPR]->Write();
    histo3D[MC::kPR] ->Write();
    histoPtGenPass[MC::kPR]->Write();
    histoCos[MC::kPR]->Write();
    // histoAglGenPass[MC::kPR]->Write();
    histoYGenPass[MC::kPR]->Write();
    histoPhiGenPass[MC::kPR]->Write();
    histoPtYGenPass[MC::kPR]->Write();
    histoYPhiGenPass[MC::kPR]->Write();
    histoPhiPtGenPass[MC::kPR]->Write();
    histoPtPass[MC::kPR]->Write();
    histoCosPass[MC::kPR]->Write();
    // histoAglPass[MC::kPR]->Write();
    histoYPass[MC::kPR]->Write();
    histoPhiPass[MC::kPR]->Write();
    histoPtYPass[MC::kPR]->Write();
    histoYPhiPass[MC::kPR]->Write();
    histoPhiPtPass[MC::kPR]->Write();
    histoYCosPass[MC::kPR]->Write();
    histoPhiCosPass[MC::kPR]->Write();
    histoPtCosPass[MC::kPR]->Write();
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
    ratioPtCosPR->Write();
    ratioCosPR->Write();
    outputFile->Close();

}
