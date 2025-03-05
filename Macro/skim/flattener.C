#include "src/common/interface/commonHeader.hxx"
#include "src/common/interface/simpleAlgos.hxx"

#include <fstream>
#include <type_traits>
#include <ROOT/TThreadExecutor.hxx>

#include "src/dataformat/interface/simpleD.hxx"
#include "src/dataformat/interface/simpleDMC.hxx"


void flattener(bool isMC){
	string date ="1109";

    using namespace ANA;
    // constexpr bool isMC = false;
    const unsigned int workers = 1;
    ROOT::TThreadExecutor pool(workers);

    auto job = [&](int jobIdx){
        //TFile* finMC = TFile::Open("~/DstarAna/DStarAna/data/output_dstarMC_BPHCut_241106.root");
        TFile* finMC = TFile::Open("~/DstarAna/DStarAna/data/output_BPHcut_MC_241108.root");
        //TFile* finMC = TFile::Open("/home/jun502s/DstarAna/DstarAna_real/dstarMC_output.root");
        TFile* finData = TFile::Open("/home/jun502s/DstarAna/MVAStudy/MVA/data/d0ana_tree_data.root");
        TTree* tfinderMC = (TTree*) finMC->Get("dStarana_mc/VertexCompositeNtuple");
        TTree* tfinderData = (TTree*) finData->Get("dStarana/VertexCompositeNtuple");
        // TFile* fin = TFile::Open("/home/jun502s/DstarAna/MVAStudy/MVA/data/d0ana_tree_mc.root");
        // TTree* tfinder = (TTree*) fin->Get("dStarana_mc/VertexCompositeNtuple");
        using namespace DataFormat;
        simpleDStarMCTreeevt dinMC;
        simpleDStarMCTreeflat doutMC;
        simpleDStarMCTreeevt dinData;
        // simpleDStarMCTreeflat dout;
        std::cout << tfinderMC->GetEntries() << std::endl;
        dinMC.setTree<TTree>(tfinderMC);
        dinData.setTree<TTree>(tfinderData);
        TFile* fout = new TFile(Form("../../data/flatSkimForBDT_isMC%d_%d_%s.root", (int) isMC, jobIdx,date.c_str()),"recreate");
        fout->cd();
        TTree* tskim = new TTree("skimTreeFlat", "");
        int iEvent;
        tskim->Branch("evtNo", &iEvent);
        doutMC.setOutputTree(tskim);
        int totEvt;

        if(isMC==true){
        totEvt = tfinderMC->GetEntries();}
        else{
        totEvt = tfinderData->GetEntries();}
        std::cout << totEvt << std::endl;

        int numFilesPerWorker = totEvt / workers;
        int start = jobIdx*numFilesPerWorker;
        // int start = 170000;
        int end = (jobIdx !=(workers-1)) ? (jobIdx+1)*numFilesPerWorker : totEvt;
        int skip = 1;

        short PS = 0;
        int counts = 0;
        for( auto iEvt : ROOT::TSeqU(start, end, skip)){
            if ((iEvt % 10000) ==0 )std::cout << iEvt<< std::endl;
            if(isMC==true){
                // cout << iEvt << endl;
            if (iEvt==170975) {
            std::cerr << "Warning: Skipping inaccessible or missing entry " << iEvt << std::endl;
            continue;  // Skip this entry
            }
            tfinderMC->GetEntry(iEvt);

            for( auto iD1 : ROOT::TSeqI(dinMC.candSize)){
                dinMC.isData=false;
                // if(dinMC.matchGEN[iD1]==true){cout<<dinMC.matchGEN[iD1]<<endl;}
                doutMC.copyDn(dinMC, iD1);
                // if(dinMC.matchGEN[iD1]==doutMC.matchGEN && dinMC.matchGEN[iD1]==true){cout << dinMC.matchGEN[iD1] << " : "  << doutMC.matchGEN  << endl;}
                // if(doutMC.matchGEN ==true){cout<<doutMC.matchGEN<<endl;}
                tskim->Fill();
            }
            }
            else{
                // cout << "asdasd"<<endl;
            tfinderData->GetEntry(iEvt);
            for( auto iD1 : ROOT::TSeqI(dinData.candSize)){
            dinData.isSwap[iD1]=false;
            dinData.idmom_reco[iD1]=-999;
            dinData.idBAnc_reco[iD1]=-999;
            dinData.matchGEN[iD1]=false;
            dinData.matchGen3DPointingAngle[iD1]=-999;
            dinData.matchGen2DPointingAngle[iD1]=-999;
            dinData.matchGen3DDecayLength[iD1]=-999;
            dinData.matchGen2DDecayLength[iD1]=-999;
            dinData.matchgen_D0pT[iD1]=-999;
            dinData.matchgen_D0eta[iD1]=-999;
            dinData.matchgen_D0phi[iD1]=-999;
            dinData.matchgen_D0mass[iD1]=-999;
            dinData.matchgen_D0y[iD1]=-999;
            dinData.matchgen_D0charge[iD1]=-999;
            dinData.matchgen_D0pdgId[iD1]=-999;
            dinData.matchGen_D0pT[iD1]=-999;
            dinData.matchGen_D0eta[iD1]=-999;
            dinData.matchGen_D0phi[iD1]=-999;
            dinData.matchGen_D0mass[iD1]=-999;
            dinData.matchGen_D0y[iD1]=-999;
            dinData.matchGen_D0charge[iD1]=-999;
            dinData.matchGen_D0pdgId[iD1]=-999;
            dinData.matchGen_D0Dau1_pT[iD1]=-999;
            dinData.matchGen_D0Dau1_eta[iD1]=-999;
            dinData.matchGen_D0Dau1_phi[iD1]=-999;
            dinData.matchGen_D0Dau1_mass[iD1]=-999;
            dinData.matchGen_D0Dau1_y[iD1]=-999;
            dinData.matchGen_D0Dau1_charge[iD1]=-999;
            dinData.matchGen_D0Dau1_pdgId[iD1]=-999;
            dinData.matchGen_D0Dau2_pT[iD1]=-999;
            dinData.matchGen_D0Dau2_eta[iD1]=-999;
            dinData.matchGen_D0Dau2_phi[iD1]=-999;
            dinData.matchGen_D0Dau2_mass[iD1]=-999;
            dinData.matchGen_D0Dau2_y[iD1]=-999;
            dinData.matchGen_D0Dau2_charge[iD1]=-999;
            dinData.matchGen_D0Dau2_pdgId[iD1]=-999;
            dinData.matchGen_D1pT[iD1]=-999;
            dinData.matchGen_D1eta[iD1]=-999;
            dinData.matchGen_D1phi[iD1]=-999;
            dinData.matchGen_D1mass[iD1]=-999;
            dinData.matchGen_D1y[iD1]=-999;
            dinData.matchGen_D1charge[iD1]=-999;
            dinData.matchGen_D1pdgId[iD1]=-999;
            dinData.matchGen_D1decayLength2D_[iD1]=-999;
            dinData.matchGen_D1decayLength3D_[iD1]=-999;
            dinData.matchGen_D1angle2D_[iD1]=-999;
            dinData.matchGen_D1angle3D_[iD1]=-999;
            dinData.matchGen_D1ancestorId_[iD1]=-999;
            dinData.matchGen_D1ancestorFlavor_[iD1]=-999;
            dinData.isData=true;
                
                doutMC.copyDn(dinData, iD1);
                tskim->Fill();
            }
        }
        }
        tskim->Write();
        fout->Write();
        fout->Close();
        return counts;
    };

    auto res = pool.Map(job, ROOT::TSeqI(workers));
    std::cout << "Done" << std::endl;

}
