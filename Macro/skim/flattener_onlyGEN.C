// #include "../interface/commonHeader.hxx"
// #include "src/common/interface/simpleAlgos.hxx"

#include <fstream>
#include <type_traits>
#include <ROOT/TThreadExecutor.hxx>

// #include "src/dataformat/interface/simpleD.hxx"
#include "../interface/simpleDMC.hxx"
void loadRootFilesToTChain(TChain* chain, const std::string& folderPath) {

    TSystemDirectory dir(folderPath.c_str(), folderPath.c_str());
    TList* files = dir.GetListOfFiles();

    if (files) {
        TSystemFile* file;
        TIter next(files);

        while ((file = (TSystemFile*)next())) {
            std::string fileName = file->GetName();
            if (!(file->IsZombie()) &&fileName.find(".root") != std::string::npos) {
                std::string filePath = folderPath + "/" + fileName;
                chain->Add(filePath.c_str()); 
            }
        }
    }

    if (chain->GetEntries() > 0) {
        std::cout << "TChain에 " << chain->GetEntries() << "개의 엔트리가 로드되었습니다." << std::endl;
    } else {
        std::cout << "TChain에 파일이 추가되지 않았습니다. 폴더나 트리 이름을 확인하세요." << std::endl;
    }

}


void flattener_onlyGEN(bool isMC){

    // using namespace ANA;
    // constexpr bool isMC = false;
    const unsigned int workers = 1;
    ROOT::TThreadExecutor pool(workers);
    string date = "250108";
    

    auto job = [&](int jobIdx){

        std::string filePath1 = "/home/jun502s/DstarAna/DStarAnalysis/Data/MC/d0ana_tree_240.root";
        //std::string filePath2 = "/eos/cms/store/group/phys_heavyions/junseok/DStarAna/MC/DStarAna_MCDstarKpipi_selectionstudy_CMSSW_13_2_11_241109_v2/DStarKpipiPU/crab_DStarAna_MCDstarKpipi_selectionstudy_CMSSW_13_2_11_241109_v2/241109_171602/0001";
        //std::string filePath3 = "/eos/cms/store/group/phys_heavyions/junseok/DStarAna/MC/DStarAna_MCDstarKpipi_selectionstudy_CMSSW_13_2_11_241109_v2/DStarKpipiPU/crab_DStarAna_MCDstarKpipi_selectionstudy_CMSSW_13_2_11_241109_v2/241109_171602/0002";
        std::string treeName = "dStarana_mc/PATCompositeNtuple";
        TChain* chain = new TChain(treeName.c_str());  
        chain->Add(filePath1.c_str());
        // loadRootFilesToTChain(chain,filePath1);
     //   loadRootFilesToTChain(chain,filePath2);
      //  loadRootFilesToTChain(chain,filePath3);
        using namespace DataFormat;
        simpleDStarMCTreeevt dinMC;
        simpleDStarMCTreeflat doutMC;
        simpleDStarDataTreeevt dinData;
        simpleDStarDataTreeflat doutData;
        //std::cout << tfinderMC->GetEntries() << std::endl;
        if(isMC==true) dinMC.setGENTree<TChain>(chain);
        else dinData.setTree<TChain>(chain);
        //dinData.setTree<TChain>(chain);
        TFile* fout = new TFile(Form("/home/jun502s/DstarAna/DStarAnalysis/Data/flatSkimForBDT_isMC%d_OnlyGEN_%d_%s.root", (int) isMC, jobIdx,date.c_str()),"recreate");
        fout->cd();
        TTree* tskim = new TTree("skimTreeFlat", "");
        TTree* tGENskim = new TTree("skimGENTreeFlat", "");
        int iEvent;
        tskim->Branch("evtNo", &iEvent);
        int totEvt;
        if(isMC==true){
        totEvt = chain->GetEntries();
        doutMC.setOutputTree(tskim);
        doutMC.setGENOutputTree(tGENskim);
        }
        else{
        totEvt = chain->GetEntries();
        doutData.setOutputTree(tskim);}
        std::cout << totEvt << std::endl;
        // totEvt = 80000;

        int numFilesPerWorker = totEvt / workers;
        int start = jobIdx*numFilesPerWorker;
        //int start = 171000;
        int end = (jobIdx !=(workers-1)) ? (jobIdx+1)*numFilesPerWorker : totEvt;
       // int end = 1000;
        int skip = 1;

        short PS = 0;
        int counts = 0;
        for( auto iEvt : ROOT::TSeqU(start, end, skip)){
            if ((iEvt % 10000) ==0 )std::cout << iEvt<< std::endl;
            if(isMC==true){
        //    if (iEvt==171449) {
        //    std::cerr << "Warning: Skipping inaccessible or missing entry " << iEvt << std::endl;
        //    continue;  // Skip this entry
        //    }
            chain->GetEntry(iEvt);

        //     for( auto iD1 : ROOT::TSeqI(dinMC.candSize)){
        //     //    dinMC.isData=false;
        //         // if(dinMC.matchGEN[iD1]==true){cout<<dinMC.matchGEN[iD1]<<endl;}
        //         doutMC.copyDn(dinMC, iD1);
        //         // if(dinMC.matchGEN[iD1]==doutMC.matchGEN && dinMC.matchGEN[iD1]==true){cout << dinMC.matchGEN[iD1] << " : "  << doutMC.matchGEN  << endl;}
        // //        if(doutMC.matchGEN ==true){cout<<doutMC.matchGEN<<endl;}
        //         tskim->Fill();
        //     }
            for( auto iD1 : ROOT::TSeqI(dinMC.candSize_gen)){
            //    dinMC.isData=false;
                // if(dinMC.matchGEN[iD1]==true){cout<<dinMC.matchGEN[iD1]<<endl;}
                doutMC.copyGENDn(dinMC, iD1);
                // cout << doutMC.gen_pT << endl;
                // if(dinMC.gen_pt[iD1]==doutMC.matchGEN && dinMC.matchGEN[iD1]==true){cout << dinMC.matchGEN[iD1] << " : "  << doutMC.matchGEN  << endl;}
        //        if(doutMC.matchGEN ==true){cout<<doutMC.matchGEN<<endl;}
                tGENskim->Fill();
            }
            }
            
            else{
            chain->GetEntry(iEvt);
            for( auto iD1 : ROOT::TSeqI(dinData.candSize)){
                    doutData.copyDn(dinData,iD1);
                    // cout << doutData.eta << endl;
                    tskim->Fill();

            }
        }
    }
        // tskim->Write();
        tGENskim->Write();
        fout->Write();
        fout->Close();
        return counts;
        
    };


    auto res = pool.Map(job, ROOT::TSeqI(workers));
    std::cout << "Done" << std::endl;

}


