#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.h"
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"



void FlatEffAccCalculator_D0(
    // ANA::selBase selMVA,
    // ANA::selBase selDCA,
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/output_pbpb_mc_wgeninfo.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Mar30NonSwap/d0ana_tree_nonswapsample_ppref_30Mar.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_0_06Apr25.root",
    //string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Apr07NonSwap_y1p2_pT2/flatSkimForBDT_DStar_ppRef_NonSwapMC_1_07Apr25.root",
    //string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/D0/flatSkimForBDT_D0_PbPb_MC_ONNX_withOfficialCent_DpT018_30Apr25.root",
    string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/D0/flatSkimForBDT_D0_PbPb_MC_ONNX_withOfficialCent_DpT018_17May25.root",
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/D0/flatSkimForBDT_D0_Prompt_withOfficialCent_pT018_0_14May25.root",
    // string fileMCNP,
//    string subDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppData/D0/flatSkimForBDT_D0_PbPb_Data_ONNX_withCent_16Apr.root",
    bool cutBased = false, 
    int mvaV = 0,
    double Direction_y = 1.0
){
	// TFile* fDz = TFile::Open("outVz.root");
	// TH1D* hDzRatio = (TH1D*) fDz->Get("hRatio");

    // double mvaCut = selMVA.low;
    double dcaCut = 0; // e.g. 0--0.01
    std::vector<double> mvaBin = {0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98, 0.990,0.991,0.992,0.993,0.994,0.995,0.996,0.997,0.998,0.999,1.0};
    std::vector<double> centralityBin = {0,5,10,15,20,25,30,35,40,45,50,60,70,80,90};

    // --- Define Histograms ---
    std::map<MC, TH1D*> histoPt;
    histoPt[MC::kPR] = new TH1D("pt_pr", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoY;
    histoY[MC::kPR] = new TH1D("y_pr", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhi;
    histoPhi[MC::kPR] = new TH1D("phi_pr", "",nphi, phil, phih);
    std::map<MC, TH1D*> histoCent;
    histoCent[MC::kPR] = new TH1D("cent_pr", "",centralityBin.size()-1, centralityBin.data());
    std::map<MC,TH1D*> histoMVA;
    histoMVA[MC::kPR] = new TH1D("mva_pr", "", mvaBin.size()-1, mvaBin.data());
    std::map<MC, TH2D*> histoPtY;
    histoPtY[MC::kPR] = new TH2D("pt_y_pr", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhi;
    histoYPhi[MC::kPR] = new TH2D("y_phi_pr", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPt;
    histoPhiPt[MC::kPR] = new TH2D("phi_pt_pr", "",nphi, phil, phih, npt, pl, ph);
    std::map<MC, TH3D*> histo3D;
    histo3D[MC::kPR]  = new TH3D("pt_y_phi_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );

    std::map<MC, TH1D*> histoPtGenPass;
    histoPtGenPass[MC::kPR] = new TH1D("pt_pr_genpass", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoYGenPass;
    histoYGenPass[MC::kPR] = new TH1D("y_pr_genpass", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhiGenPass;
    histoPhiGenPass[MC::kPR] = new TH1D("phi_pr_genpass", "",nphi, phil, phih);
    std::map<MC, TH2D*> histoPtYGenPass;
    histoPtYGenPass[MC::kPR] = new TH2D("pt_y_pr_genpass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhiGenPass;
    histoYPhiGenPass[MC::kPR] = new TH2D("y_phi_pr_genpass", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPtGenPass;
    histoPhiPtGenPass[MC::kPR] = new TH2D("phi_pt_pr_genpass", "",nphi, phil, phih, npt, pl, ph);
    std::map<MC, TH3D*> histo3DGenPass;
    histo3DGenPass[MC::kPR]  = new TH3D("pt_y_phi_pr_genpass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );

    std::map<MC, TH1D*> histoPtPass;
    histoPtPass[MC::kPR] = new TH1D("pt_pr_pass", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoYPass;
    histoYPass[MC::kPR] = new TH1D("y_pr_pass", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhiPass;
    histoPhiPass[MC::kPR] = new TH1D("phi_pr_pass", "",nphi, phil, phih);
    std::map<MC, TH1D*> histoCentPass;
    histoCentPass[MC::kPR] = new TH1D("cent_pr_pass", "",centralityBin.size()-1, centralityBin.data());
    std::map<MC, TH1D*> histoMVAPass;
    histoMVAPass[MC::kPR] = new TH1D("mva_pr_pass", "" ,mvaBin.size()-1, mvaBin.data());
    std::map<MC, TH2D*> histoPtYPass;
    histoPtYPass[MC::kPR] = new TH2D("pt_y_pr_pass", "",npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0]);
    std::map<MC, TH2D*> histoYPhiPass;
    histoYPhiPass[MC::kPR] = new TH2D("y_phi_pr_pass", "",ny, yl, yh, nphi, phil, phih);
    std::map<MC, TH2D*> histoPhiPtPass;
    histoPhiPtPass[MC::kPR] = new TH2D("phi_pt_pr_pass", "",nphi, phil, phih, npt, pl, ph);
    std::map<MC, TH3D*> histo3DPass;
    histo3DPass[MC::kPR]  = new TH3D("pt_y_phi_pr_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0],  nphi, &algo::getEqualRangeV<double>(phil, phih, nphi)[0] );

    // Int_t bins[4] = {npt, ny, mvaBin.size()-1, centralityBin.size()-1};
    // Double_t low[4] = {corrParams::pvec()[0], corrParams::yvec()[0], mvaBin[0], centralityBin[0]};
    // Double_t high[4] = {corrParams::pvec()[npt], corrParams::yvec()[ny], mvaBin[mvaBin.size()-1], centralityBin[centralityBin.size()-1]};

    // Add MVA histograms
    Int_t nmva = 20; // Number of bins for MVA
    Double_t mvalo = 0.9, mvahi = 1.0; // Range for MVA score
    std::map<MC, TH3D*> histoCuml3DPass;
    histoCuml3DPass[MC::kPR]  = new TH3D("pt_y_mva_pr_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );
    histoCuml3DPass[MC::kNP]  = new TH3D("pt_y_mva_np_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );
    // std::map<MC, THn*> histoCuml4DPass;
    // histoCuml4DPass[MC::kPR] = new THn("pt_y_mva_cent_pr_pass", "", 4, npt, low, high, bins, low, high);
    

    std::map<MC, TH3D*> histoCuml3D;
    histoCuml3D[MC::kPR]  = new TH3D("pt_y_mva_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );
    histoCuml3D[MC::kNP]  = new TH3D("pt_y_mva_np", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );
    // std::map<MC, THn*> histoCuml4D;
    // histoCuml4D[MC::kPR] = new THn("pt_y_mva_cent_pr", "", 4, npt, low, high, bins, low, high);
    // histoCuml4D[MC::kPR]  = new THn("pt_y_mva_cent_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data(), centralityBin.size()-1, centralityBin.data() );
    // histoCuml4D[MC::kNP]  = new THn("pt_y_mva_cent_np", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data(),centralityBin.size()-1, centralityBin.data() );

    // --- New histograms for pT, y, centrality efficiency maps per MVA cut ---
    std::map<MC, TH3D*> histoGen_PtYCent;
    histoGen_PtYCent[MC::kPR] = new TH3D("pt_y_cent_pr_gen_total", "Gen pT vs y vs centrality (PR);pT;y;centrality", 
                                         npt, &corrParams::pvec()[0], 
                                         ny, &corrParams::yvec()[0], 
                                         centralityBin.size()-1, centralityBin.data());
    // Add for MC::kNP if needed and if input for NP is processed
    // histoGen_PtYCent[MC::kNP] = new TH3D("pt_y_cent_np_gen_total", "Gen pT vs y vs centrality (NP);pT;y;centrality", 
    //                                      npt, &corrParams::pvec()[0], 
    //                                      ny, &corrParams::yvec()[0], 
    //                                      centralityBin.size()-1, centralityBin.data());

    std::map<MC, std::vector<TH3D*>> histoReco_PtYCent_ForMVACut;
    histoReco_PtYCent_ForMVACut[MC::kPR].resize(mvaBin.size());
    // histoReco_PtYCent_ForMVACut[MC::kNP].resize(mvaBin.size()); // If MC::kNP is processed

    for (size_t i_mva = 0; i_mva < mvaBin.size(); ++i_mva) {
        cout << i_mva << " " << mvaBin[i_mva] << endl;
        histoReco_PtYCent_ForMVACut[MC::kPR][i_mva] = 
            new TH3D(Form("pt_y_cent_pr_reco_mvaCut_gt_%.4f", mvaBin[i_mva]), 
                     Form("Reco pT vs y vs centrality (PR), MVA > %.4f;pT;y;centrality", mvaBin[i_mva]),
                     npt, &corrParams::pvec()[0], 
                     ny, &corrParams::yvec()[0], 
                     centralityBin.size()-1, centralityBin.data());
        // Add for MC::kNP if needed
        // histoReco_PtYCent_ForMVACut[MC::kNP][i_mva] = 
        //     new TH3D(Form("pt_y_cent_np_reco_mvaCut_gt_%.4f", mvaBin[i_mva]), 
        //              Form("Reco pT vs y vs centrality (NP), MVA > %.4f;pT;y;centrality", mvaBin[i_mva]),
        //              npt, &corrParams::pvec()[0], 
        //              ny, &corrParams::yvec()[0], 
        //              centralityBin.size()-1, centralityBin.data());
    }
    // --- End of new histogram definitions ---

    DataFormat::simpleDMCTreeflat evtGenFlat;  // For Gen tree
    DataFormat::simpleDMCTreeflat evtRecoFlat; // For Reco tree
    
    /* Get files and trees */
    TFile* fMCPR = TFile::Open(fileMCPR.c_str());
    if (!fMCPR || fMCPR->IsZombie()) {
        std::cerr << "Error opening file: " << fileMCPR << std::endl;
        return;
    }
    
    // Get the flat trees
    TTree* genTree = (TTree*)fMCPR->Get("skimGENTreeFlat");
    TTree* recoTree = (TTree*)fMCPR->Get("skimTreeFlat");
    if (!genTree || !recoTree) {
        std::cerr << "Error: Could not find required trees in the file" << std::endl;
        fMCPR->Close(); // Close the file if trees are not found
        return;
    }

    /* Set tree*/
    evtGenFlat.setGENTree(genTree);
    evtRecoFlat.setTree(recoTree);
    long long numGenEntries = genTree->GetEntries();
    long long numRecoEntries = recoTree->GetEntries();


    /* Histo Filling Algorithm, takes index and will run every loop that is declared after*/
    int countgen = 0;
    int count = 0;

    auto fillGen = [&](double dzW, MC mcID){
        if( evtGenFlat.gen_pT > 0 && 
            fabs(evtGenFlat.gen_y) < 1 && // Basic kinematic acceptance for generated particles
            true){
            countgen++;
            histoPt[mcID]->Fill(evtGenFlat.gen_pT, dzW);
            histoY[mcID]->Fill(Direction_y * evtGenFlat.gen_y, dzW);
            histoCent[mcID]->Fill(evtGenFlat.centrality/2, dzW);
            histoPhi[mcID]->Fill(evtGenFlat.gen_phi, dzW);
            histoPtY[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, dzW);

            histoYPhi[mcID]->Fill(Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            histoPhiPt[mcID]->Fill(evtGenFlat.gen_phi, evtGenFlat.gen_pT, dzW);
            histo3D[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            
            // Fill the new generator histogram for pT, y, centrality
            histoGen_PtYCent[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, evtGenFlat.centrality/2, dzW);

            for( auto mvas: mvaBin){ // This loop is for existing cumulative MVA histograms
                histoMVA[mcID]->Fill(mvas, dzW); // This seems to fill all mva thresholds for each event, check logic
                histoCuml3D[mcID] ->Fill(evtGenFlat.gen_pT, Direction_y* evtGenFlat.gen_y, mvas, dzW);
                // histoCuml4D[mcID] ->Fill(evtGenFlat.gen_pT, Direction_y* evtGenFlat.gen_y, mvas, evtGenFlat.centrality/2, dzW); // centrality/2 might need review
            }
            
            if (fabs(evtGenFlat.gen_y) < DSGLABSY_D0 &&
                // fabs(evtGenFlat.gen_D1) < DSGLABSETA_D1 &&
                // evtGenFlat.gen_D1pT > DSGLPT_D1 &&
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
        if (evtRecoFlat.matchGEN && !evtRecoFlat.isSwap) {
            // histoMVA[mcID]->Fill(evtRecoFlat.mva, dzW); // This was for all reco MVA values, not binned/cut

            
            // Apply selection criteria
            if (fabs(evtRecoFlat.y) < 1 &&
                evtRecoFlat.pT > 0 &&
                evtRecoFlat.pT < 50 &&
                // evtRecoFlat.centrality/2 > 0 &&
                // evtRecoFlat.centrality/2 < 90 &&
                // ... other reco selections ...
                true ) {
                
                // ... existing histo fills for pass ...
                histoPtPass[mcID]->Fill(evtRecoFlat.pT, dzW);
                histoYPass[mcID]->Fill(Direction_y * evtRecoFlat.y, dzW);
                histoPhiPass[mcID]->Fill(evtRecoFlat.phi, dzW);
                histoCentPass[mcID]->Fill(evtRecoFlat.centrality/2, dzW);
                histoPtYPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, dzW);
                histoYPhiPass[mcID]->Fill(Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                histoPhiPtPass[mcID]->Fill(evtRecoFlat.phi, evtRecoFlat.pT, dzW);
                histo3DPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                
                double dzW2 = dzW; // Potentially reweighted

                // Fill new reco histograms for pT, y, centrality, for each MVA cut
                if (histoReco_PtYCent_ForMVACut.count(mcID)) {
                    for (size_t i_mva = 0; i_mva < mvaBin.size(); ++i_mva) {
                        if (evtRecoFlat.mva > mvaBin[i_mva]) {
                            histoReco_PtYCent_ForMVACut[mcID][i_mva]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, evtRecoFlat.centrality/2, dzW2);

                        }
                    }
                }

                // This loop is for existing cumulative MVA histograms
                for( auto mvaThres : mvaBin){
                    if(evtRecoFlat.mva > mvaThres) histoMVAPass[mcID]->Fill(mvaThres+0.00000001, dzW); // Fills at the threshold value
                    if(evtRecoFlat.mva > mvaThres) histoCuml3DPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, mvaThres+0.000000001, dzW2);
                    // if(evtRecoFlat.mva > mvaThres) histoCuml4DPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, mvaThres+0.000000001, evtRecoFlat.centrality/2, dzW2);
                }
                count ++;
            }
        }
    };

    /* Actual filling loop*/

    std::cout << "Processing GEN entries..." << std::endl;
    for( auto idx : ROOT::TSeqU(numGenEntries)){
        if (idx % 100000 == 0) std::cout << " GEN Entry: " << idx << "/" << numGenEntries << std::endl;
        genTree->GetEntry(idx);
        double dzW = evtGenFlat.ncoll;
        // double dzW = 1;
            fillGen(dzW, MC::kPR);
    }
    std::cout << "Processing RECO entries..." << std::endl;
    for( auto idx : ROOT::TSeqU(numRecoEntries)){
         if (idx % 100000 == 0) std::cout << " RECO Entry: " << idx << "/" << numRecoEntries << std::endl;
        recoTree->GetEntry(idx);
        // cout << evtRecoFlat.ncoll  << endl;
        double dzW = evtRecoFlat.ncoll;
        // double dzW = 1;
            fillReco(dzW, MC::kPR);
    }

    auto ratio3DPR = (TH3D*) histo3DPass[MC::kPR]->Clone("pt_y_phi_pr_ratio");
    auto ratio3DPRACC = (TH3D*) histo3DGenPass[MC::kPR]->Clone("pt_y_phi_pr_acc_ratio");
    auto ratio3DPRRECO = (TH3D*) histo3DPass[MC::kPR]->Clone("pt_y_phi_pr_reco_ratio");
    ratio3DPR->SetName("pt_y_phi_pr_ratio");
    ratio3DPR->Divide(ratio3DPR, histo3D[MC::kPR], 1, 1, "B");
    ratio3DPRACC->Divide(ratio3DPRACC, histo3D[MC::kPR], 1, 1, "B");
    ratio3DPRRECO->Divide(ratio3DPRRECO, histo3DGenPass[MC::kPR], 1, 1, "B");

    auto GetRatioPlot = [&](TH1D* num, TH1D* den){
        auto ratio1D =  (TH1D*) num->Clone(Form("%s_ratio",num->GetName()));
        ratio1D->Divide(num, den, 1, 1, "B");
        ratio1D->GetYaxis()->SetTitle("Efficiency");
        ratio1D->GetYaxis()->SetRangeUser(0.0, 1.05);
        return ratio1D;
    };

    auto Get2DRatioPlot = [&](TH2D* num, TH2D* den){
        auto ratio2D =  (TH2D*) num->Clone(Form("%s_ratio",num->GetName()));
        ratio2D->Divide(num, den, 1, 1, "B");
        ratio2D->GetZaxis()->SetRangeUser(0.0, 1.0);
        return ratio2D;
    };

    // auto ratioCuml3DPR = (TH3D*) histoCuml3DPass[MC::kPR]->Clone("pt_y_mva_pr_ratio");
    // auto ratioCuml3DNP = (TH3D*) histoCuml3DPass[MC::kNP]->Clone("pt_y_mva_np_ratio");
    // ratioCuml3DPR->Divide(ratioCuml3DPR, histoCuml3D[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    // ratioCuml3DNP->Divide(ratioCuml3DNP, histoCuml3D[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");
    // auto ratioMVAPR = GetRatioPlot(histoMVAPass[MC::kPR], histoMVA[MC::kPR]);
    // ratioMVAPR->SetLineColor(kMagenta);
    auto ratioCuml3DPR = (TH3D*) histoCuml3DPass[MC::kPR]->Clone("pt_y_mva_pr_ratio");
    // auto ratioCuml3DNP = (TH3D*) histoCuml3DPass[MC::kNP]->Clone("pt_y_mva_np_ratio");
    ratioCuml3DPR->Divide(ratioCuml3DPR, histoCuml3D[MC::kPR], 1, 1, "cl=0.683 b(1,1) mode");
    // ratioCuml3DNP->Divide(ratioCuml3DNP, histoCuml3D[MC::kNP], 1, 1, "cl=0.683 b(1,1) mode");

    auto ratioPtPR = GetRatioPlot(histoPtPass[MC::kPR], histoPt[MC::kPR]);
    auto ratioYPR = GetRatioPlot(histoYPass[MC::kPR], histoY[MC::kPR]);
    auto ratioMVAPR = GetRatioPlot(histoMVAPass[MC::kPR], histoMVA[MC::kPR]);
    auto ratioPhiPR = GetRatioPlot(histoPhiPass[MC::kPR], histoPhi[MC::kPR]);
    auto ratioCentPR = GetRatioPlot(histoCentPass[MC::kPR], histoCent[MC::kPR]);
    auto ratioPtYPR = Get2DRatioPlot(histoPtYPass[MC::kPR], histoPtY[MC::kPR]);
    // auto ratioMVA = GetRatioPlot(histoMVAPass[MC::kPR], histoMVA[MC::kPR]);

    ratioPtPR->SetLineColor(kRed);
    ratioYPR->SetLineColor(kRed);
    ratioPhiPR->SetLineColor(kRed);

    TCanvas* c1 = new TCanvas("c1", "c", 400*4, 400*2);
    c1->Divide(4,2);
    c1->cd(1);
    ratioPtYPR->Draw("colz");
    c1->cd(2);
    ratioMVAPR->GetYaxis()->SetRangeUser(ratioMVAPR->GetMinimum()*0.8, ratioMVAPR->GetMaximum()*1.2);
    ratioMVAPR->Draw();
    c1->cd(3);
    ratioPtPR->Draw();
    c1->cd(4);
    ratioYPR->Draw();
    c1->cd(5);
    ratioPhiPR->Draw();
    c1->cd(6);
    ratioCentPR->Draw();

    c1->SaveAs(Form("distributions.png"));

    TFile* outputFile = new TFile(Form("output_MC_May17_ncoll_v1.root"),"recreate");

    std::cout << "Passing Gen total : " << countgen << std::endl;
    std::cout << "Passing Reco total : " << count << std::endl;
    outputFile->cd();

    histoPt[MC::kPR]->Write();
    histoY[MC::kPR]->Write();
    histoPhi[MC::kPR]->Write();
    histoCent[MC::kPR]->Write();
    histoPtY[MC::kPR]->Write();
    histoYPhi[MC::kPR]->Write();
    histoPhiPt[MC::kPR]->Write();
    histo3D[MC::kPR] ->Write();

    histoPtGenPass[MC::kPR]->Write();
    histoYGenPass[MC::kPR]->Write();
    histoPhiGenPass[MC::kPR]->Write();
    histoPtYGenPass[MC::kPR]->Write();
    histoYPhiGenPass[MC::kPR]->Write();
    histoPhiPtGenPass[MC::kPR]->Write();
    histo3DGenPass[MC::kPR] ->Write();

    histoPtPass[MC::kPR]->Write();
    histoYPass[MC::kPR]->Write();
    histoPhiPass[MC::kPR]->Write();
    histoCentPass[MC::kPR]->Write();
    histoPtYPass[MC::kPR]->Write();
    histoYPhiPass[MC::kPR]->Write();
    histoPhiPtPass[MC::kPR]->Write();
    histo3DPass[MC::kPR] ->Write();
    histoCuml3DPass[MC::kPR]->Write();
    // histoCuml3DPass[MC::kNP]->Write();
    histoCuml3D[MC::kPR]->Write();
    // histoCuml3D[MC::kNP]->Write();

    histoMVA[MC::kPR]->Write();
    histoMVAPass[MC::kPR]->Write();
    ratioMVAPR->Write();

    // --- Write new histograms and efficiency maps ---
    if (histoGen_PtYCent.count(MC::kPR)) {
        histoGen_PtYCent[MC::kPR]->Write();
    }
    // Add for MC::kNP if processed
    // if (histoGen_PtYCent.count(MC::kNP)) {
    //     histoGen_PtYCent[MC::kNP]->Write();
    // }

    if (histoReco_PtYCent_ForMVACut.count(MC::kPR)) {
        for (size_t i_mva = 0; i_mva < mvaBin.size(); ++i_mva) {
            TH3D* num_hist = histoReco_PtYCent_ForMVACut[MC::kPR][i_mva];
            TH3D* den_hist = histoGen_PtYCent[MC::kPR];

            num_hist->Write(); // Write the numerator histogram for this MVA cut

            if (den_hist && den_hist->GetEntries() > 0) { // Ensure denominator is valid
                TH3D* eff_hist = (TH3D*)num_hist->Clone(Form("eff_pt_y_cent_pr_mvaCut_gt_%.4f", mvaBin[i_mva]));
                eff_hist->SetTitle(Form("Efficiency (pT,y,cent) PR for MVA > %.4f;pT (GeV/c);y;Centrality (%%)", mvaBin[i_mva]));
                eff_hist->Divide(eff_hist, den_hist, 1, 1, "B"); // "B" for binomial errors
                eff_hist->Write();
            }
        }
    }
    // Add similar loop for MC::kNP if processed
    // if (histoReco_PtYCent_ForMVACut.count(MC::kNP) && histoGen_PtYCent.count(MC::kNP)) {
    //     for (size_t i_mva = 0; i_mva < mvaBin.size(); ++i_mva) {
    //         TH3D* num_hist = histoReco_PtYCent_ForMVACut[MC::kNP][i_mva];
    //         TH3D* den_hist = histoGen_PtYCent[MC::kNP];
    //         num_hist->Write();
    //         if (den_hist && den_hist->GetEntries() > 0) {
    //             TH3D* eff_hist = (TH3D*)num_hist->Clone(Form("eff_pt_y_cent_np_mvaCut_gt_%.4f", mvaBin[i_mva]));
    //             eff_hist->SetTitle(Form("Efficiency (pT,y,cent) NP for MVA > %.4f;pT (GeV/c);y;Centrality (%%)", mvaBin[i_mva]));
    //             eff_hist->Divide(eff_hist, den_hist, 1, 1, "B");
    //             eff_hist->Write();
    //         }
    //     }
    // }
    // --- End of writing new histograms ---


    ratio3DPR->Write();
    ratio3DPRACC->Write();
    ratio3DPRRECO->Write();
    ratioCentPR->Write();
    ratioPtPR->Write();
    ratioPtYPR->Write();
    ratioYPR->Write();
    ratioCuml3DPR->Write();
    // ratioCuml3DNP->Write();
    ratioPhiPR->Write();

    outputFile->Close();
    fMCPR->Close();
}
