#include <fstream>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"

// 루트 파일 생성 함수
TFile* createOutputFile(const std::string& date, int jobIdx, int batchIdx) {
    // 배치 번호에 맞는 새로운 파일을 생성
    std::string fileName = Form("../../Data/flatSkimForBDT_Mix_%d_%s_batch_%d.root", 
                                 jobIdx, date.c_str(), batchIdx);
    TFile* fout = new TFile(fileName.c_str(), "RECREATE");
    return fout;
}

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


void FlatMix_threadsafe(){

    // using namespace ANA;
    // constexpr bool isMC = false;
    const unsigned int workers = 2;
    ROOT::TThreadExecutor pool(workers);
    string date = "2500227";
    std::mutex tree_mutex;

    // std::string filePath1 = "/home/jun502s/DstarAna/DStarAnalysis/Data/SkimStep1/d0ana_tree_115.root";
    // std::string filePath2 = "/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/d0ana_tree_1.root";
    // std::string treeName = "d0ana_newreduced/PATCompositeNtuple";
    // TChain* chainMC = new TChain(treeName.c_str());
    // loadRootFilesToTChain(chainMC, "/home/jun502s/DstarAna/DStarAnalysis/Data/0000");
    // TChain* chainData = new TChain(treeName.c_str());
    // loadRootFilesToTChain(chainData, "/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/");
    

    

    auto job = [&](int jobIdx){
        std::string filePath1 = "/home/jun502s/DstarAna/DStarAnalysis/Data/SkimStep1/d0ana_tree_115.root";
        std::string filePath2 = "/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/d0ana_tree_1.root";
        std::string treeName = "d0ana_newreduced/PATCompositeNtuple";
        std::unique_ptr<TChain> chainMC(new TChain(treeName.c_str()));
        std::unique_ptr<TChain> chainData(new TChain(treeName.c_str()));
        loadRootFilesToTChain(chainMC.get(), "/home/jun502s/DstarAna/DStarAnalysis/Data/0000");
        loadRootFilesToTChain(chainData.get(), "/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/");
        TTreeReader readerMC(localChainMC.get());
        TTreeReader readerData(localChainData.get());
    

        // TFile *fMC = TFile::Open(filePath1.c_str());
        // TTree* treeMC = (TTree*)fMC->Get(treeName.c_str());
        // TFile *fData = TFile::Open(filePath2.c_str());
        // TTree* treeData = (TTree*)fData->Get(treeName.c_str());
    
        using namespace DataFormat;
        std::unique_ptr<simpleDMCTreeevt> dinMC(new simpleDMCTreeevt());
        std::unique_ptr<simpleDTreeevt> dinData(new simpleDTreeevt());
        std::unique_ptr<simpleDMCTreeflat> doutMC(new simpleDMCTreeflat());

        // simpleDMCTreeevt* dinMC=new simpleDMCTreeevt();
        // simpleDMCTreeflat* doutMC=new simpleDMCTreeflat();
        // uniquedinData =new simpleDTreeevt();
        // simpleDTreeflat* doutData =new simpleDTreeflat();
        // simpleDTreeflat doutData;
        // cout << treeMC->GetEntries() << " " << treeData->GetEntries() << endl;
        cout << chainMC->GetEntries() << " " << chainData->GetEntries() << endl;
        // dinMC->setTree<TTree>(treeMC);
        // dinData->setTree<TTree>(treeData);
        dinMC->setTree<TChain>(chainMC.get());
        dinData->setTree<TChain>(chainData.get());
        int batchsize = 100;
        std::unique_ptr<TFile> fout(new TFile(Form("../../Data/test/flatSkimForBDT_%d_%s.root", jobIdx,date.c_str()),"recreate"));
        fout->cd();
        std::unique_ptr<TTree> tskim(new TTree("skimTreeFlat", ""));
        int totEvt = std::min(chainMC->GetEntries(), chainData->GetEntries());
        totEvt = 100;
        cout << "Total events: " << totEvt << endl;
        // totEvt = chain->GetEntries();
        // doutData->setOutputTree(tskim);
        doutMC->setOutputTree(tskim.get());

        int numFilesPerWorker = totEvt / workers;
        int start = jobIdx*numFilesPerWorker;
        int end = (jobIdx !=(workers-1)) ? (jobIdx+1)*numFilesPerWorker : totEvt;
        int skip = 1;

        short PS = 0;
        int counts = 0;
        int batchIdx = 0;
        cout << "loopstart" << endl;
        for( auto iEvt : ROOT::TSeqU(start, end, skip)){
            if ((iEvt % 100) ==0 )std::cout << "Processing event " << iEvt<< std::endl;
                    // try {
            chainMC->GetEntry(iEvt);

            // if (status == 0 || status2 == 0) {
                // std::cout << "Event " << iEvt << " skipped due to error." << std::endl;
                // continue;
            // }

            // std::cout << "Processing event " << iEvt << std::endl;

        
            
            for( auto iD1 : ROOT::TSeqI(dinMC->candSize)){
            
                doutMC->isMC = true;
                doutMC->copyDn(*dinMC, iD1);
                
                tskim->Fill();
            }
            chainData->GetEntry(iEvt);
            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                    doutMC->isMC = false;
          doutMC->isSwap=-999;   //[candSize]
          doutMC->idmom_reco=-999;   //[candSize]
          doutMC->idBAnc_reco=-999;   //[candSize]
          doutMC->matchGEN=0;   //[candSize]
          doutMC->matchGen3DPointingAngle=-999;   //[candSize]
          doutMC->matchGen2DPointingAngle=-999;   //[candSize]
          doutMC->matchGen3DDecayLength=-999;   //[candSize]
          doutMC->matchGen2DDecayLength=-999;   //[candSize]
          doutMC->matchGen_D0pT=-999;   //[candSize]
          doutMC->matchGen_D0eta=-999;   //[candSize]
          doutMC->matchGen_D0phi=-999;   //[candSize]
          doutMC->matchGen_D0mass=-999;   //[candSize]
          doutMC->matchGen_D0y=-999;   //[candSize]
          doutMC->matchGen_D0charge=-999;   //[candSize]
          doutMC->matchGen_D0pdgId=-999;   //[candSize]
          doutMC->matchGen_D0Dau1_pT=-999;   //[candSize]
          doutMC->matchGen_D0Dau1_eta=-999;   //[candSize]
          doutMC->matchGen_D0Dau1_phi=-999;   //[candSize]
          doutMC->matchGen_D0Dau1_mass=-999;   //[candSize]
          doutMC->matchGen_D0Dau1_y=-999;   //[candSize]
          doutMC->matchGen_D0Dau1_charge=-999;   //[candSize]
          doutMC->matchGen_D0Dau1_pdgId=-999;   //[candSize]
          doutMC->matchGen_D0Dau2_pT=-999;   //[candSize]
          doutMC->matchGen_D0Dau2_eta=-999;   //[candSize]
          doutMC->matchGen_D0Dau2_phi=-999;   //[candSize]
          doutMC->matchGen_D0Dau2_mass=-999;   //[candSize]
          doutMC->matchGen_D0Dau2_y=-999;   //[candSize]
          doutMC->matchGen_D0Dau2_charge=-999;   //[candSize]
          doutMC->matchGen_D0Dau2_pdgId=-999;   //[candSize]
          doutMC->matchGen_D1ancestorId_=-999;   //[candSize]
          doutMC->matchGen_D1ancestorFlavor_=-999;   //[candSize]
                    doutMC->copyDn<simpleDTreeevt>(*dinData, iD1);
                    tskim->Fill();
                }
                    // }
        //                         catch (const std::exception& e) {
        //     std::cout << "Exception occurred while processing event " << iEvt << ": " << e.what() << std::endl;
        //     continue;
        // }

            
        }
                

            
        
        tskim->Write();
        fout->Write();
        fout->Close();
        
        
    
        // delete dinMC;
        // delete dinData;
        
        return counts;
        
    };
        

    // fMC->Close();
    // fData->Close();

    auto res = pool.Map(job, ROOT::TSeqI(workers));
    std::cout << "Done" << std::endl;

}
