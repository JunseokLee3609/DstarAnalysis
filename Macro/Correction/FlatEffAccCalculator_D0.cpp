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
    string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/D0/flatSkimForBDT_D0_PbPb_MC_ONNX_withCent_Apr23_0_.root",
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
    std::vector<double> mvaBin = { 0.990,0.991,0.992,0.993,0.994,0.995,0.996,0.997,0.998,0.999,1.0};

    // --- Define Histograms ---
    std::map<MC, TH1D*> histoPt;
    histoPt[MC::kPR] = new TH1D("pt_pr", "",npt, &corrParams::pvec()[0]);
    std::map<MC, TH1D*> histoY;
    histoY[MC::kPR] = new TH1D("y_pr", "",ny, &corrParams::yvec()[0]);
    std::map<MC, TH1D*> histoPhi;
    histoPhi[MC::kPR] = new TH1D("phi_pr", "",nphi, phil, phih);
    std::map<MC, TH1D*> histoCent;
    histoCent[MC::kPR] = new TH1D("cent_pr", "",ncent, &corrParams::centvec()[0]);
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
    histoCentPass[MC::kPR] = new TH1D("cent_pr_pass", "",ncent, &corrParams::centvec()[0]);
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

    // Add MVA histograms
    Int_t nmva = 20; // Number of bins for MVA
    Double_t mvalo = 0.9, mvahi = 1.0; // Range for MVA score
    std::map<MC, TH3D*> histoCuml3DPass;
    histoCuml3DPass[MC::kPR]  = new TH3D("pt_y_mva_pr_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );
    histoCuml3DPass[MC::kNP]  = new TH3D("pt_y_mva_np_pass", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );

    std::map<MC, TH3D*> histoCuml3D;
    histoCuml3D[MC::kPR]  = new TH3D("pt_y_mva_pr", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );
    histoCuml3D[MC::kNP]  = new TH3D("pt_y_mva_np", "", npt, &corrParams::pvec()[0], ny, &corrParams::yvec()[0], mvaBin.size()-1, mvaBin.data() );

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

    TFile* outputFile = new TFile(Form("output.root"),"recreate");

    /* Histo Filling Algorithm, takes index and will run every loop that is declared after*/
    int countgen = 0;
    int count = 0;

    auto fillGen = [&](double dzW, MC mcID){
        if( evtGenFlat.gen_pT > 0 && 
            fabs(evtGenFlat.gen_y) < 1 &&
            true){
            countgen++;
            histoPt[mcID]->Fill(evtGenFlat.gen_pT, dzW);
            histoY[mcID]->Fill(Direction_y * evtGenFlat.gen_y, dzW);
            histoCent[mcID]->Fill(evtGenFlat.centrality, dzW);
            histoPhi[mcID]->Fill(evtGenFlat.gen_phi, dzW);
            histoPtY[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, dzW);

            histoYPhi[mcID]->Fill(Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            histoPhiPt[mcID]->Fill(evtGenFlat.gen_phi, evtGenFlat.gen_pT, dzW);
            histo3D[mcID]->Fill(evtGenFlat.gen_pT, Direction_y * evtGenFlat.gen_y, evtGenFlat.gen_phi, dzW);
            for( auto mvas: mvaBin){
                histoMVA[mcID]->Fill(mvas, dzW);
                histoCuml3D[mcID] ->Fill(evtGenFlat.gen_pT, Direction_y* evtGenFlat.gen_y, mvas, dzW);
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
            // histoMVA[mcID]->Fill(evtRecoFlat.mva, dzW);

            
            // Apply selection criteria
            if (fabs(evtRecoFlat.y) < 1 &&
                evtRecoFlat.pT > 0 &&
                // fabs(D0y) < DSGLABSY_D0 &&
                // fabs(evtRecoFlat.EtaD2) < 2.4 &&
                // // evtRecoFlat.pTD2 > 1 &&
                // // evtRecoFlat.pTD1 > 1 &&
                // fabs(evtRecoFlat.EtaGrandD1) < DSGLABSETA_D0DAU1 &&
                // fabs(evtRecoFlat.EtaGrandD2) < DSGLABSETA_D0DAU2 &&
                // evtRecoFlat.pTGrandD1 > DSGLPT_D0DAU1 &&
                // evtRecoFlat.pTGrandD2 > DSGLPT_D0DAU2 &&
                true ) {
                
                
                
                // count++;
                
                // histoMVAPass[mcID]->Fill(evtRecoFlat.mva, dzW);

                histoPtPass[mcID]->Fill(evtRecoFlat.pT, dzW);
                histoYPass[mcID]->Fill(Direction_y * evtRecoFlat.y, dzW);
                histoPhiPass[mcID]->Fill(evtRecoFlat.phi, dzW);
                histoCentPass[mcID]->Fill(evtRecoFlat.centrality, dzW);
                histoPtYPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, dzW);
                histoYPhiPass[mcID]->Fill(Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                histoPhiPtPass[mcID]->Fill(evtRecoFlat.phi, evtRecoFlat.pT, dzW);
                histo3DPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, evtRecoFlat.phi, dzW);
                double dzW2 = dzW;
                // if(reweight){
                //     dzW2 *=  1.0/hist_pty[mcID]->GetBinContent(hist_pty[mcID]->FindBin(evtRecoFlatRecoFlat.pT, evtRecoFlat.y));
                // }
                for( auto mvaThres : mvaBin){
                    if(evtRecoFlat.mva > mvaThres) histoMVAPass[mcID]->Fill(mvaThres+0.00000001, dzW);
                    if(evtRecoFlat.mva > mvaThres) histoCuml3DPass[mcID]->Fill(evtRecoFlat.pT, Direction_y * evtRecoFlat.y, mvaThres+0.000000001, dzW2);
                }
                // if (((evtRecoFlat.mva-0.3)*DSGLMVASCALE +0.3 ) > 0.8){
                    // histoAglPass[mcID]->Fill(evtRecoFlatRecoFlat.v3DPointingAngle  , dzW);
                //     histoPtPass[mcID]->Fill(evtRecoFlat.pT  , dzW2);
                //     histoYPass[mcID]->Fill(Direction_y * evtRecoFlat.y  , dzW2);
                //     histoPhiPass[mcID]->Fill(evtRecoFlat.phi  , dzW2);
                //     histoPtYPass[mcID]->Fill(evtRecoFlat.pT, Direction_y *evtRecoFlat.y  , dzW2);
                //     histoYPhiPass[mcID]->Fill(Direction_y * evtRecoFlat.y, evtRecoFlat.phi  , dzW2);
                //     histoPhiPtPass[mcID]->Fill(evtRecoFlat.phi, evtRecoFlat.pT  , dzW2);
                //     histo3DPass[mcID] ->Fill(evtRecoFlat.pT, Direction_y *evtRecoFlat.y, evtRecoFlat.phi, dzW2);
                // count ++;
                // }
            }
        }
    };

    /* Actual filling loop*/

    std::cout << "Processing GEN entries..." << std::endl;
    for( auto idx : ROOT::TSeqU(numGenEntries)){
        if (idx % 100000 == 0) std::cout << " GEN Entry: " << idx << "/" << numGenEntries << std::endl;
        genTree->GetEntry(idx);
        double dzW = evtGenFlat.ncoll;
            fillGen(dzW, MC::kPR);
    }
    std::cout << "Processing RECO entries..." << std::endl;
    for( auto idx : ROOT::TSeqU(numRecoEntries)){
         if (idx % 100000 == 0) std::cout << " RECO Entry: " << idx << "/" << numRecoEntries << std::endl;
        recoTree->GetEntry(idx);
        // cout << evtRecoFlat.ncoll  << endl;
        double dzW = evtRecoFlat.ncoll;
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
    ratioMVAPR->SetRangeUser(ratioMVAPR->GetMinimum()*0.8, ratioMVAPR->GetMaimum()*1.2);
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
