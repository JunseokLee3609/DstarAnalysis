#include <fstream>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"

// 루트 파일 생성 함수
TFile* createOutputFile(const std::string& date, int jobIdx) {
    // 배치 번호에 맞는 새로운 파일을 생성
    std::string fileName = Form("../../Data/flatSkimForBDT_Mix_100to1_%d_%s.root", 
                                 jobIdx, date.c_str());
    TFile* fout = new TFile(fileName.c_str(), "RECREATE");
    std::cout << "Output file " << fileName << " has been created." << std::endl;
    return fout;
}

void FlatMix(int start, int end, int jobIdx){


    const unsigned int workers = 1;
    string date = "2500227";

    

    // auto job = [&](int jobIdx){

        
        // TFile *fMC = TFile::Open(filePath1.c_str());
        // TTree* treeMC = (TTree*)fMC->Get(treeName.c_str());
        // TFile *fData = TFile::Open(filePath2.c_str());
        // TTree* treeData = (TTree*)fData->Get(treeName.c_str());
        std::string filePath1 = "/home/jun502s/DstarAna/DStarAnalysis/Data/SkimStep1/d0ana_tree_115.root";
        // std::string filePath2 = "/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/d0ana_tree_1.root";
        std::string filePath2 = "/home/CMS/Run3_2023/Data/SkimMVA/testD0.root";
        std::string treeName = "d0ana_newreduced/PATCompositeNtuple";
        std::unique_ptr<TChain> chainMC(new TChain(treeName.c_str()));
        std::unique_ptr<TChain> chainData(new TChain(treeName.c_str()));
        chainMC->Add("/home/jun502s/DstarAna/DStarAnalysis/Data/0000/*.root");
        chainData->Add("/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/*.root");
        // chainData->Add(filePath2.c_str());
    
        using namespace DataFormat;
        simpleDMCTreeevt* dinMC=new simpleDMCTreeevt();
        simpleDMCTreeflat* doutMC=new simpleDMCTreeflat();
        simpleDTreeevt* dinData =new simpleDTreeevt();
        // simpleDTreeflat* doutData =new simpleDTreeflat();
        // simpleDTreeflat doutData;
        // cout << treeMC->GetEntries() << " " << treeData->GetEntries() << endl;
        cout << chainMC->GetEntries() << " " << chainData->GetEntries() << endl;
        // dinMC->setTree<TTree>(treeMC);
        // dinData->setTree<TTree>(treeData);
        dinMC->setTree<TChain>(chainMC.get());
        dinData->setTree<TChain>(chainData.get());
        int batchSize = 100000;
        int batchIdx = 0;
        TFile* fout = createOutputFile(date, jobIdx);
        TTree* tskim = new TTree("skimTreeFlat", "");
        int totEvt = std::min(chainMC->GetEntries(), chainData->GetEntries());
        // totEvt = 1000000;
        cout << "Total events: " << totEvt << endl;
        // totEvt = chain->GetEntries();
        // doutData->setOutputTree(tskim);
        doutMC->setOutputTree(tskim);

        // int numFilesPerWorker = totEvt / workers;
        // int start = jobIdx*numFilesPerWorker;
        // int end = (jobIdx !=(workers-1)) ? (jobIdx+1)*numFilesPerWorker : totEvt;
        int skip = 1;

        short PS = 0;
        int counts = 0;
        cout << "loopstart" << endl;
        end = std::min(end, totEvt);
        int idxMC = 0;
        int idxData = 0;
        for (auto iEvt : ROOT::TSeqU(start, end, skip))
        {
            // cout << "start" << start << " end" << end << " iEvt" << iEvt << endl;
            if ((iEvt % 10000) == 0)
            {
                std::cout << "Processing event " << iEvt << std::endl;
            }
            // try {

            // if (status == 0 || status2 == 0) {
            //     std::cout << "Event " << iEvt << " skipped due to error." << std::endl;
            //     continue;
            // }

            // std::cout << "Processing event " << iEvt << std::endl;

            chainMC->GetEntry(iEvt);
            for (auto iD1 : ROOT::TSeqI(dinMC->candSize))
            {
                if (dinMC->matchGEN[iD1] == true)
                {
                    doutMC->isMC = true;
                    doutMC->copyDn(*dinMC, iD1);
                    tskim->Fill();
                }
                else
                {
                    if (idxMC % 100 == 0)
                    {
                        doutMC->isMC = true;
                        doutMC->copyDn(*dinMC, iD1);
                        tskim->Fill();
                    // cout << idxMC << "fill mc" << endl;
                        
                    }
                        idxMC++;
                }
            }
            chainData->GetEntry(iEvt);
            // cout << "Data" << endl;
            for (auto iD1 : ROOT::TSeqI(dinData->candSize))
            {
                //   if ((iD1 % 10000) ==0 )std::cout << "Processing cand " << iD1<< std::endl;

                //   doutMC->idmom_reco=-999;   //[candSize]
                //   doutMC->idBAnc_reco=-999;   //[candSize]
                //   doutMC->matchGen3DPointingAngle=-999;   //[candSize]
                //   doutMC->matchGen2DPointingAngle=-999;   //[candSize]
                //   doutMC->matchGen3DDecayLength=-999;   //[candSize]
                //   doutMC->matchGen2DDecayLength=-999;   //[candSize]
                //   doutMC->matchGen_D0pT=-999;   //[candSize]
                //   doutMC->matchGen_D0eta=-999;   //[candSize]
                //   doutMC->matchGen_D0phi=-999;   //[candSize]
                //   doutMC->matchGen_D0mass=-999;   //[candSize]
                //   doutMC->matchGen_D0y=-999;   //[candSize]
                //   doutMC->matchGen_D0charge=-999;   //[candSize]
                //   doutMC->matchGen_D0pdgId=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau1_pT=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau1_eta=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau1_phi=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau1_mass=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau1_y=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau1_charge=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau1_pdgId=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau2_pT=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau2_eta=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau2_phi=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau2_mass=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau2_y=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau2_charge=-999;   //[candSize]
                //   doutMC->matchGen_D0Dau2_pdgId=-999;   //[candSize]
                //   doutMC->matchGen_D1ancestorId_=-999;   //[candSize]
                //   doutMC->matchGen_D1ancestorFlavor_=-999;   //[candSize]
                if (idxData % 100 == 0)
                {
                    doutMC->isMC = false;
                    doutMC->isSwap = 0;   //[candSize]
                    doutMC->matchGEN = 0; //[candSize]
                    doutMC->copyDn<simpleDTreeevt>(*dinData, iD1);
                    tskim->Fill();
                    // cout << idxData << "fill data" << endl;
                }
                idxData++;
            }
        }
        // break;
        // if ((iEvt + 1) % batchSize == 0) {
        //         // 기존 파일 저장 후 새로운 파일을 열기
        //         tskim->Write();
        //         fout->Write();
        //         fout->Close();

        //         // 배치 번호 증가 및 새로운 파일 생성
        //         batchIdx++;
        //         fout = createOutputFile(date, jobIdx, batchIdx);
        //         tskim = new TTree("skimTreeFlat", "");
        //         doutMC->setOutputTree(tskim);
        //     }
        // break;
        // }
        

        // 마지막 파일 저장
        if (fout) {
            tskim->Write();
            fout->Write();
            fout->Close();
        }
                    // }
        //                         catch (const std::exception& e) {
        //     std::cout << "Exception occurred while processing event " << iEvt << ": " << e.what() << std::endl;
        //     continue;
        

            
                

            
        
        // tskim->Write();
        // fout->Write();
        // fout->Close();
        
        
    
        // delete dinMC;
        // delete dinData;
        // return counts;
        
    // };
        

    // fMC->Close();
    // fData->Close();

    // auto res = pool.Map(job, ROOT::TSeqI(workers));
    std::cout << "Done" << std::endl;

}
