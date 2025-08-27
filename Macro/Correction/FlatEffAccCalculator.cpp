#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.h"
#include "../interface/simpleAlgos.hxx"
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"
#include "../Tools/ConfigManager.h"



void FlatEffAccCalculator(
    // ANA::selBase selMVA,
    // ANA::selBase selDCA,
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Mar30NonSwap/d0ana_tree_nonswapsample_ppref_30Mar.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_0_06Apr25.root",
    //string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_1_07Apr25.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/flatSkimForBDT_DStar_MixDStar_MC_25Jul_0_.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/flatSkimForBDT_DStar_dstar_Mix_ppRef_MC_Jul28.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/0818/flatSkimForBDT_DStar_Mix_PPRef_MC_Aug18.root",// new ntupler, D0 Dau track cut 1 GeV , slow pion pT cut 0p3
    string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbMC/DStar/0822/flatSkimForBDT_DStar_dstar_PbPb_MC_wEvtplane_Aug22.root",// new ntupler, D0 Dau track cut 1 GeV , slow pion pT cut 0p3
    // string fileMCNP,
    string subDir = SelectionCuts::SUB_DIR,
    bool cutBased = false, 
    int mvaV = 0,
    double Direction_y = 1.0
){
    vector<double> cosBin = {-1,-0.8,-0.6,-0.4,-0.2,0,0.2,0.4,0.6,0.8,1};
	// TFile* fDz = TFile::Open("outVz.root");
	// TH1D* hDzRatio = (TH1D*) fDz->Get("hRatio");
    gStyle->SetOptStat(0);

   // double mvaCut = selMVA.low;
    double dcaCut = 0; // e.g. 0--0.01

    // std::map<MC, TH1D*> histoAgl;
    // histoAgl[MC::kPR] = new TH1D("agl_pr", "",nagl, &corrParams::avec()[0]);
    // histoAgl[MC::kNP] = new TH1D("agl_np", "",nagl, &corrParams::avec()[0]);
    std::map<MC, TH1D*> histoPt;
    histoPt[MC::kPR] = new TH1D("pt_pr", "",npt, &corrParams::pvec()[0]);
    histoPt[MC::kNP] = new TH1D("pt_np", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoY;
    histoY[MC::kPR] = new TH1D("y_pr", "",ny, &corrParams::yvec()[0]);
    histoY[MC::kNP] = new TH1D("y_np", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhi;
    histoPhi[MC::kPR] = new TH1D("phi_pr", "",nphi, phil, phih);
    histoPhi[MC::kNP] = new TH1D("phi_np", "",nphi, phil, phih);
    std::map<MC, TH1D*> histoCos;
    histoCos[MC::kPR] = new TH1D("cos_pr", "",ncos, coslo, coshi);
    histoCos[MC::kNP] = new TH1D("cos_np", "",ncos, coslo, coshi);
    std::map<MC, TH2D*> histoPtY;
    histoPtY[MC::kPR] = new TH2D("pt_y_pr", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    histoPtY[MC::kNP] = new TH2D("pt_y_np", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhi;
    histoYPhi[MC::kPR] = new TH2D("y_phi_pr", "",ny, yl, yh, nphi, phil, phih);
    histoYPhi[MC::kNP] = new TH2D("y_phi_np", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPt;
    histoPhiPt[MC::kPR] = new TH2D("phi_pt_pr", "",nphi, phil, phih, npt, pl, ph);
    histoPhiPt[MC::kNP] = new TH2D("phi_pt_np", "",nphi, phil, phih, npt, pl, ph);
    std::map<MC, TH2D*> histoYCos;
    histoYCos[MC::kPR] = new TH2D("y_cos_pr", "",ny, yl, yh, ncos, coslo, coshi);
    histoYCos[MC::kNP] = new TH2D("y_cos_np", "",ny, yl, yh, ncos, coslo, coshi);
    std::map<MC, TH2D*> histoPhiCos;
    histoPhiCos[MC::kPR] = new TH2D("phi_cos_pr", "", nphi, phil, phih, ncos, coslo, coshi );
    histoPhiCos[MC::kNP] = new TH2D("phi_cos_np", "", nphi, phil, phih, ncos, coslo, coshi );
    std::map<MC, TH2D*> histoPtCos;
    histoPtCos[MC::kPR] = new TH2D("pt_cos_pr", "", npt, &corrParams::pvec()[0], ncos, coslo, coshi);
    histoPtCos[MC::kNP] = new TH2D("pt_cos_np", "", npt, &corrParams::pvec()[0], ncos, coslo, coshi);

    // std::map<MC, TH1D*> histoAglGenPass;
    // histoAglGenPass[MC::kPR] = new TH1D("agl_pr_genpass", "",nagl, &corrParams::avec()[0]);
    // histoAglGenPass[MC::kNP] = new TH1D("agl_np_genpass", "",nagl, &corrParams::avec()[0]);
    std::map<MC, TH1D*> histoPtGenPass;
    histoPtGenPass[MC::kPR] = new TH1D("pt_pr_genpass", "",npt, &corrParams::pvec()[0]);
    histoPtGenPass[MC::kNP] = new TH1D("pt_np_genpass", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoYGenPass;
    histoYGenPass[MC::kPR] = new TH1D("y_pr_genpass", "",ny, &corrParams::yvec()[0]);
    histoYGenPass[MC::kNP] = new TH1D("y_np_genpass", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhiGenPass;
    histoPhiGenPass[MC::kPR] = new TH1D("phi_pr_genpass", "",nphi, phil, phih);
    histoPhiGenPass[MC::kNP] = new TH1D("phi_np_genpass", "",nphi, phil, phih);
    std::map<MC, TH2D*> histoPtYGenPass;
    histoPtYGenPass[MC::kPR] = new TH2D("pt_y_pr_genpass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    histoPtYGenPass[MC::kNP] = new TH2D("pt_y_np_genpass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhiGenPass;
    histoYPhiGenPass[MC::kPR] = new TH2D("y_phi_pr_genpass", "",ny, yl, yh, nphi, phil, phih);
    histoYPhiGenPass[MC::kNP] = new TH2D("y_phi_np_genpass", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPtGenPass;
    histoPhiPtGenPass[MC::kPR] = new TH2D("phi_pt_pr_genpass", "",nphi, phil, phih, npt, pl, ph);
    histoPhiPtGenPass[MC::kNP] = new TH2D("phi_pt_np_genpass", "",nphi, phil, phih, npt, pl, ph);

    // std::map<MC, TH1D*> histoAglPass;
    // histoAglPass[MC::kPR] = new TH1D("agl_pr_pass", "",nagl, &corrParams::avec()[0]);
    // histoAglPass[MC::kNP] = new TH1D("agl_np_pass", "",nagl, &corrParams::avec()[0]);
    std::map<MC, TH1D*> histoPtPass;
    histoPtPass[MC::kPR] = new TH1D("pt_pr_pass", "",npt, &corrParams::pvec()[0]);
    histoPtPass[MC::kNP] = new TH1D("pt_np_pass", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoYPass;
    histoYPass[MC::kPR] = new TH1D("y_pr_pass", "",ny, &corrParams::yvec()[0]);
    histoYPass[MC::kNP] = new TH1D("y_np_pass", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhiPass;
    histoPhiPass[MC::kPR] = new TH1D("phi_pr_pass", "",nphi, phil, phih);
    histoPhiPass[MC::kNP] = new TH1D("phi_np_pass", "",nphi, phil, phih);
    std::map<MC, TH1D*> histoCosPass;
    histoCosPass[MC::kPR] = new TH1D("cos_pr_pass", "",ncos, coslo, coshi );
    histoCosPass[MC::kNP] = new TH1D("cos_np_pass", "",ncos, coslo, coshi );
    std::map<MC, TH2D*> histoPtYPass;
    histoPtYPass[MC::kPR] = new TH2D("pt_y_pr_pass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    histoPtYPass[MC::kNP] = new TH2D("pt_y_np_pass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhiPass;
    histoYPhiPass[MC::kPR] = new TH2D("y_phi_pr_pass", "",ny, yl, yh, nphi, phil, phih);
    histoYPhiPass[MC::kNP] = new TH2D("y_phi_np_pass", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPtPass;
    histoPhiPtPass[MC::kPR] = new TH2D("phi_pt_pr_pass", "",nphi, phil, phih, npt, pl, ph);
    histoPhiPtPass[MC::kNP] = new TH2D("phi_pt_np_pass", "",nphi, phil, phih, npt, pl, ph);
    std::map<MC, TH2D*> histoYCosPass;
    histoYCosPass[MC::kPR] = new TH2D("y_cos_pr_pass", "",ny, yl, yh, ncos, coslo, coshi);
    histoYCosPass[MC::kNP] = new TH2D("y_cos_np_pass", "",ny, yl, yh, ncos, coslo, coshi);
    std::map<MC, TH2D*> histoPhiCosPass;
    histoPhiCosPass[MC::kPR] = new TH2D("phi_cos_pr_pass", "", nphi, phil, phih, ncos, coslo, coshi );
    histoPhiCosPass[MC::kNP] = new TH2D("phi_cos_np_pass", "", nphi, phil, phih, ncos, coslo, coshi );
    std::map<MC, TH2D*> histoPtCosPass;
    histoPtCosPass[MC::kPR] = new TH2D("pt_cos_pr_pass", "", npt, &corrParams::pvec()[0], ncos, coslo, coshi);
    histoPtCosPass[MC::kNP] = new TH2D("pt_cos_np_pass", "", npt, &corrParams::pvec()[0], ncos, coslo, coshi);

    std::map<MC, TH3D*> histo3D;
    histo3D[MC::kPR]  = new TH3D("pt_y_phi_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    histo3D[MC::kNP]  = new TH3D("pt_y_phi_np", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    std::map<MC, TH3D*> histo3DCos;
    histo3DCos[MC::kPR]  = new TH3D("pt_y_cos_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], cosBin.size()-1, &cosBin[0] );
    histo3DCos[MC::kNP]  = new TH3D("pt_y_cos_np", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], cosBin.size()-1, &cosBin[0] );

    std::map<MC, TH3D*> histo3DGenPass;
    histo3DGenPass[MC::kPR]  = new TH3D("pt_y_phi_pr_genpass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    histo3DGenPass[MC::kNP]  = new TH3D("pt_y_phi_np_genpass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    std::map<MC, TH3D*> histo3DPass;
    histo3DPass[MC::kPR]  = new TH3D("pt_y_phi_pr_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    histo3DPass[MC::kNP]  = new TH3D("pt_y_phi_np_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );
    std::map<MC, TH3D*> histo3DCosPass;
    histo3DCosPass[MC::kPR]  = new TH3D("pt_y_cos_pr_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], cosBin.size()-1, &cosBin[0] );
    histo3DCosPass[MC::kNP]  = new TH3D("pt_y_cos_np_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], cosBin.size()-1, &cosBin[0] );



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

    TFile* outputFile = new TFile(Form("output_%s.root",subDir.c_str()),"recreate");

    /* Histo Filling Algorithm, takes index and will run every loop that is declared after*/
    int countgen = 0;
    int count = 0;

    auto fillGen = [&](double dzW){
        if( fabs(evtGenFlat.gen_y) < SelectionCuts::Y_PARENT_MAX &&
            true){
            
            MC mcID = (evtGenFlat.gen_D0ancestorFlavor_ == 5) ? MC::kNP : MC::kPR;
            
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
            histoCos[mcID]->Fill(abs(vect.CosTheta()), dzW);
            histoPhi[mcID]->Fill(evtGenFlat.gen_phi, dzW);
            histoPtY[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, dzW);
            histoYCos[mcID]->Fill(Direction_y * evtGenFlat.gen_y, abs(vect.CosTheta()), dzW);
            histoPhiCos[mcID]->Fill(evtGenFlat.gen_phi, abs(vect.CosTheta()), dzW);
            histoPtCos[mcID]->Fill(evtGenFlat.gen_pT, abs(vect.CosTheta()), dzW);
            histoYPhi[mcID]->Fill(Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            histoPhiPt[mcID]->Fill(evtGenFlat.gen_phi, evtGenFlat.gen_pT, dzW);
            histo3D[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            histo3DCos[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, vect.CosTheta(), dzW);
            
            if (fabs(evtGenFlat.gen_D0y) < SelectionCuts::Y_PARENT_MAX &&
                fabs(evtGenFlat.gen_D1eta) < SelectionCuts::ETA_D2_MAX &&
                evtGenFlat.gen_D1pT > SelectionCuts::PT_D2_MIN &&
                fabs(evtGenFlat.gen_D0Dau1_eta) < SelectionCuts::ETA_GRANDD1_MAX &&
                fabs(evtGenFlat.gen_D0Dau2_eta) < SelectionCuts::ETA_GRANDD2_MAX &&
                evtGenFlat.gen_D0Dau1_pT > SelectionCuts::PT_GRANDD1_MIN &&
                evtGenFlat.gen_D0Dau2_pT > SelectionCuts::PT_GRANDD2_MIN &&
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
    auto fillReco = [&](double dzW){
        if (evtRecoFlat.matchGEN && !evtRecoFlat.isSwap &&
        true) {
            
            MC mcID = (evtRecoFlat.matchGen_D1ancestorFlavor_ == 5) ? MC::kNP : MC::kPR;
            auto D0y = algo::rapidity(evtRecoFlat.pTD1, evtRecoFlat.EtaD1, evtRecoFlat.PhiD1, evtRecoFlat.massDaugther1);
            
            // Apply selection criteria
            if (fabs(evtRecoFlat.y) < SelectionCuts::Y_PARENT_MAX &&
                evtRecoFlat.pT > SelectionCuts::PT_PARENT_MIN &&
                // fabs(D0y) < SelectionCuts::Y_PARENT_MAX &&
                fabs(evtRecoFlat.EtaD2) < SelectionCuts::ETA_D2_MAX &&
                evtRecoFlat.pTD2 > SelectionCuts::PT_D2_MIN &&
                fabs(evtRecoFlat.EtaGrandD1) < SelectionCuts::ETA_GRANDD1_MAX &&
                fabs(evtRecoFlat.EtaGrandD2) < SelectionCuts::ETA_GRANDD2_MAX &&
                evtRecoFlat.pTGrandD1 > SelectionCuts::PT_GRANDD1_MIN &&
                evtRecoFlat.pTGrandD2 > SelectionCuts::PT_GRANDD2_MIN &&
		evtRecoFlat.mva > 0.99 &&
                true ) {
                
                TLorentzVector Dstar, D0;
                Dstar.SetPtEtaPhiM(evtRecoFlat.pT, evtRecoFlat.eta, evtRecoFlat.phi, evtRecoFlat.mass);
                D0.SetPtEtaPhiM(evtRecoFlat.pTD1, evtRecoFlat.EtaD1, evtRecoFlat.PhiD1, evtRecoFlat.massDaugther1);
                TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);
                
                count++;
                
                histoPtPass[mcID]->Fill(evtRecoFlat.pT, dzW);
                histoYPass[mcID]->Fill(Direction_y * evtRecoFlat.y, dzW);
                histoPhiPass[mcID]->Fill(evtRecoFlat.phi, dzW);
                histoCosPass[mcID]->Fill(abs(vect.CosTheta()), dzW);
                histoPtYPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, dzW);
                histoYPhiPass[mcID]->Fill(Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                histoPhiPtPass[mcID]->Fill(evtRecoFlat.phi, evtRecoFlat.pT, dzW);
                histo3DPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                histoYCosPass[mcID]->Fill(Direction_y * evtRecoFlat.y, abs(vect.CosTheta()), dzW);
                histoPhiCosPass[mcID]->Fill(evtRecoFlat.phi, abs(vect.CosTheta()), dzW);
                histoPtCosPass[mcID]->Fill(evtRecoFlat.pT, abs(vect.CosTheta()), dzW);
                histo3DCosPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, vect.CosTheta(), dzW);
            }
        }
    };

    /* Actual filling loop*/

    for( auto idx : ROOT::TSeqU(numGenEntries)){
         if (idx % 100000 == 0) std::cout << " GEN Entry: " << idx << "/" << numGenEntries << std::endl;
        genTree->GetEntry(idx);
		// double dzW = hDzRatio->GetBinContent(hDzRatio->FindBin(evtPR.bestvtxZ));
        double dzW = 1.0;
            fillGen(dzW);
        // std::cout << std::endl;
    }
    for( auto idx : ROOT::TSeqU(numRecoEntries)){
         if (idx % 100000 == 0) std::cout << " RECO Entry: " << idx << "/" << numRecoEntries << std::endl;
        recoTree->GetEntry(idx);
        double dzW = 1.0;
            fillReco(dzW);
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
    auto ratio3DNP = (TH3D*) histo3DPass[MC::kNP]->Clone("pt_y_phi_np_ratio");
    auto ratio3DPRACC = (TH3D*) histo3DGenPass[MC::kPR]->Clone("pt_y_phi_pr_acc_ratio");
    auto ratio3DNPACC = (TH3D*) histo3DGenPass[MC::kNP]->Clone("pt_y_phi_np_acc_ratio");

    auto ratio3DPRRECO = (TH3D*) histo3DPass[MC::kPR]->Clone("pt_y_phi_pr_reco_ratio");
    auto ratio3DNPRECO = (TH3D*) histo3DPass[MC::kNP]->Clone("pt_y_phi_np_reco_ratio");
    ratio3DPR->SetName("pt_y_phi_pr_ratio");
    ratio3DNP->SetName("pt_y_phi_np_ratio");
    ratio3DPR->Divide(ratio3DPR, histo3D[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    ratio3DNP->Divide(ratio3DNP, histo3D[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");

    ratio3DPRACC->Divide(ratio3DPRACC, histo3D[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    ratio3DNPACC->Divide(ratio3DNPACC, histo3D[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");

    ratio3DPRRECO->Divide(ratio3DPRRECO, histo3DGenPass[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    ratio3DNPRECO->Divide(ratio3DNPRECO, histo3DGenPass[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");

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
       auto Get3DRatioPlot = [&](TH3D* num, TH3D* den){
        auto ratio3D =  (TH3D*) num->Clone(Form("%s_ratio",num->GetName()));
        ratio3D->Divide(den);
        // ratio3D->GetYaxis()->SetRangeUser(ratio3D->GetMinimum()*1.2, ratio3D->GetMaximum()*1.2);
        // ratio3D->GetXaxis()->SetRangeUser(ratio3D->GetMinimum()*1.2, ratio3D->GetMaximum()*1.2);
        return ratio3D;
    };
 

    // auto ratioAglPR = GetRatioPlot(histoAglPass[MC::kPR], histoAgl[MC::kPR]);
    // auto ratioAglNP = GetRatioPlot(histoAglPass[MC::kNP], histoAgl[MC::kNP]);
    auto ratioPtPR = GetRatioPlot(histoPtPass[MC::kPR], histoPt[MC::kPR]);
    auto ratioPtNP = GetRatioPlot(histoPtPass[MC::kNP], histoPt[MC::kNP]);
    auto ratioYPR = GetRatioPlot(histoYPass[MC::kPR], histoY[MC::kPR]);
    auto ratioYNP = GetRatioPlot(histoYPass[MC::kNP], histoY[MC::kNP]);
    auto ratioPhiPR = GetRatioPlot(histoPhiPass[MC::kPR], histoPhi[MC::kPR]);
    auto ratioPhiNP = GetRatioPlot(histoPhiPass[MC::kNP], histoPhi[MC::kNP]);
    auto ratioCosPR = GetRatioPlot(histoCosPass[MC::kPR], histoCos[MC::kPR]);
    auto ratioCosNP = GetRatioPlot(histoCosPass[MC::kNP], histoCos[MC::kNP]);
    auto ratioPtCosPR = Get2DRatioPlot(histoPtCosPass[MC::kPR], histoPtCos[MC::kPR]);
    auto ratioPtCosNP = Get2DRatioPlot(histoPtCosPass[MC::kNP], histoPtCos[MC::kNP]);
    auto ratioPtYPR = Get2DRatioPlot(histoPtYPass[MC::kPR], histoPtY[MC::kPR]);
    auto ratioPtYNP = Get2DRatioPlot(histoPtYPass[MC::kNP], histoPtY[MC::kNP]);
    auto ratioYPhiPR = Get2DRatioPlot(histoYPhiPass[MC::kPR], histoYPhi[MC::kPR]);
    auto ratioYPhiNP = Get2DRatioPlot(histoYPhiPass[MC::kNP], histoYPhi[MC::kNP]);
    auto ratioPhiPtPR = Get2DRatioPlot(histoPhiPtPass[MC::kPR], histoPhiPt[MC::kPR]);
    auto ratioPhiPtNP = Get2DRatioPlot(histoPhiPtPass[MC::kNP], histoPhiPt[MC::kNP]);
    auto ratioPtYCosPR = Get3DRatioPlot(histo3DCosPass[MC::kPR], histo3DCos[MC::kPR]);
    auto ratioPtYCosNP = Get3DRatioPlot(histo3DCosPass[MC::kNP], histo3DCos[MC::kNP]);

    // ratioAglPR->SetLineColor(kRed);
    // ratioAglNP->SetLineColor(kBlue);
    ratioPtPR->SetLineColor(kRed);
    ratioPtNP->SetLineColor(kBlue);
    ratioYPR->SetLineColor(kRed);
    ratioYNP->SetLineColor(kBlue);
    ratioPhiPR->SetLineColor(kRed);
    ratioPhiNP->SetLineColor(kBlue);
    ratioCosPR->SetLineColor(kRed);
    ratioCosNP->SetLineColor(kBlue);

    TCanvas* c1 = new TCanvas("c1", "c", 400*6, 400*4);
    c1->Divide(4,2);
    // c1->cd(1);
    // ratio3DPR->Draw("box2");
    c1->cd(1);
    ratioPtPR->Draw();
    ratioPtNP->Draw("same");
    c1->cd(2);
    ratioYPR->Draw();
    ratioYNP->Draw("same");
    c1->cd(3);
    ratioPhiPR->Draw();
    ratioPhiNP->Draw("same");
    c1->cd(4);
    ratioCosPR->Draw();
    ratioCosNP->Draw("same");
    // ratioCosPR->Draw();
    c1->cd(5);
    ratioPtYPR->Draw("colz");
    c1->cd(6);
    ratioYPhiPR->Draw("colz");
    c1->cd(7);
    ratioPhiPtPR->Draw("colz");
    c1->cd(8);
    ratioPtCosPR->Draw("colz");
    // ratioPtCosPR->Draw("colz");
    // ratioPhiNP->Draw("same");
    // c1->cd(5);
    // ratioAglPR->Draw();
    // ratioAglNP->Draw("same");

    // c1->SaveAs(Form("../../output/histogram/root/efficiency/%sdistribution_%s_%s.png",subDir.c_str(), selMVA.name.c_str(), selDCA.name.c_str()));
    c1->SaveAs(Form("distributions.pdf"));
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
    histo3DCos[MC::kPR]->Write();
    
    histoPt[MC::kNP]->Write();
    histoY[MC::kNP]->Write();
    histoPhi[MC::kNP]->Write();
    histoPtY[MC::kNP]->Write();
    histoYPhi[MC::kNP]->Write();
    histoPhiPt[MC::kNP]->Write();
    histoPtCos[MC::kNP]->Write();
    histoYCos[MC::kNP]->Write();
    histoPhiCos[MC::kNP]->Write();
    histo3D[MC::kNP] ->Write();
    histoPtGenPass[MC::kNP]->Write();
    histoCos[MC::kNP]->Write();
    histo3DCos[MC::kNP]->Write();
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
    histo3DCosPass[MC::kPR] ->Write();
    
    histoYGenPass[MC::kNP]->Write();
    histoPhiGenPass[MC::kNP]->Write();
    histoPtYGenPass[MC::kNP]->Write();
    histoYPhiGenPass[MC::kNP]->Write();
    histoPhiPtGenPass[MC::kNP]->Write();
    histoPtPass[MC::kNP]->Write();
    histoCosPass[MC::kNP]->Write();
    // histoAglPass[MC::kNP]->Write();
    histoYPass[MC::kNP]->Write();
    histoPhiPass[MC::kNP]->Write();
    histoPtYPass[MC::kNP]->Write();
    histoYPhiPass[MC::kNP]->Write();
    histoPhiPtPass[MC::kNP]->Write();
    histoYCosPass[MC::kNP]->Write();
    histoPhiCosPass[MC::kNP]->Write();
    histoPtCosPass[MC::kNP]->Write();
    histo3DGenPass[MC::kNP] ->Write();
    histo3DPass[MC::kNP] ->Write();
    histo3DCosPass[MC::kNP] ->Write();
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
    TH1D* projCos_firstPtBin = histoPtCosPass[MC::kPR]->ProjectionY("projCos_firstPtBin", 1, 3);

// 예시: 결과를 그리기
TCanvas* c_proj = new TCanvas("c_proj", "Projection of first pt bin", 600, 400);
projCos_firstPtBin->Draw();
c_proj->SaveAs("projCos_firstPtBin.png");
    ratio3DPR->Write();
    ratio3DNP->Write();
    ratio3DPRACC->Write();
    ratio3DNPACC->Write();
    ratio3DPRRECO->Write();
    ratio3DNPRECO->Write();
    ratioPtCosPR->Write();
    ratioPtCosNP->Write();
    ratioCosPR->Write();
    ratioCosNP->Write();
    ratioPtYCosPR->Write();
    ratioPtYCosNP->Write();
    outputFile->Close();

}
