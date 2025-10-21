#include <fstream>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"

// 루트 파일 생성 함수
TFile* createOutputFile(const std::string& date, int jobIdx) {
    // 배치 번호에 맞는 새로운 파일을 생성
    std::string fileName = Form("../../Data/flatSkim_BDT_%d_%s.root", 
                                 jobIdx, date.c_str());
    TFile* fout = new TFile(fileName.c_str(), "RECREATE");
    std::cout << "Output file " << fileName << " has been created." << std::endl;
    return fout;
}

void Flat(int start, int end, int jobIdx){


    const unsigned int workers = 1;
    string date = "2500328";

    

    // auto job = [&](int jobIdx){

        
        // TFile *fMC = TFile::Open(filePath1.c_str());
        // TTree* treeMC = (TTree*)fMC->Get(treeName.c_str());
        // TFile *fData = TFile::Open(filePath2.c_str());
        // TTree* treeData = (TTree*)fData->Get(treeName.c_str());
        // std::string filePath1 = "/home/jun502s/DstarAna/DStarAnalysis/Data/SkimStep1/d0ana_tree_115.root";
        // std::string filePath2 = "/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/d0ana_tree_1.root";
        // std::string filePath2 = "/home/CMS/Run3_2023/Data/SkimMVA/testD0.root";
        std::string treeName = "d0ana_newreduced/PATCompositeNtuple";
        std::string treeNameEvt = "eventinfoana/EventInfoNtuple";
        std::unique_ptr<TChain> chainData(new TChain(treeName.c_str()));

        std::unique_ptr<TChain> evt_chainData(new TChain(treeNameEvt.c_str()));
        chainData->Add("/home/CMS/Run3_2023/Data/SkimMVA/D0Ana_Data_Step2MVA_HIPhysicsRawPrime0_PromptRecov2_CMSSW_13_2_13_MVA_25Mar2025_v1/HIPhysicsRawPrime0/crab_D0Ana_Data_Step2MVA_HIPhysicsRawPrime0_PromptRecov2_CMSSW_13_2_13_MVA_25Mar2025_v1/250325_141120/0000/*.root");

        evt_chainData->Add("/home/CMS/Run3_2023/Data/SkimMVA/D0Ana_Data_Step2MVA_HIPhysicsRawPrime0_PromptRecov2_CMSSW_13_2_13_MVA_25Mar2025_v1/HIPhysicsRawPrime0/crab_D0Ana_Data_Step2MVA_HIPhysicsRawPrime0_PromptRecov2_CMSSW_13_2_13_MVA_25Mar2025_v1/250325_141120/0000/*.root");


        Short_t centrality1, centrality2, centrality3, centrality4, centrality5, centrality6, centrality7, centrality8;
        evt_chainData->SetBranchAddress("centrality", &centrality4);

    
        using namespace DataFormat;
        simpleDMCTreeflat* doutMC=new simpleDMCTreeflat();
        simpleDTreeevt* dinData =new simpleDTreeevt();
        // simpleDTreeflat* doutData =new simpleDTreeflat();
        // simpleDTreeflat doutData;
        // dinData->setTree<TTree>(treeData);
        dinData->setTree<TChain>(chainData.get());
        int batchSize = 100000;
        int batchIdx = 0;
        TFile* fout = createOutputFile(date, jobIdx);
        TTree* tskim = new TTree("skimTreeFlat", "");
		bool isPR = false;
		// tskim->Branch("isPrompt", &isPR);
        tskim->Branch("cBin", &centrality8);
		// long long nEvtPR = chainMC->GetEntries();
        int totEvt = chainData->GetEntries();
		cout << "totEvt : " << totEvt <<  endl;

        doutMC->setOutputTree(tskim);
        // int numFilesPerWorker = totEvt / workers;
        // int start = jobIdx*numFilesPerWorker;
        // int end = (jobIdx !=(workers-1)) ? (jobIdx+1)*numFilesPerWorker : totEvt;
        int skip = 1;

        short PS = 0;
        int counts = 0;
        cout << "loopstart" << endl;
        end = std::min(end, totEvt);
		end = std::max(end, 1);
		cout << "End : " << end << endl;
        int idxData = 0;
        for (auto iEvt : ROOT::TSeqU(start, end, skip))
        {
            // cout << "start" << start << " end" << end << " iEvt" << iEvt << endl;
            if ((iEvt % 100) == 0)
            {
                std::cout << "Processing event " << iEvt << std::endl;
            }
            chainData->GetEntry(iEvt);
            evt_chainData->GetEntry(iEvt);
            // cout << "Data" << endl;
            for (auto iD1 : ROOT::TSeqI(dinData->candSize))
            {
                // doutMC->isMC = false;
                // doutMC->isSwap = 0;   //[candSize]
                // doutMC->matchGEN = 0; //[candSize]
                doutMC->copyDn<simpleDTreeevt>(*dinData, iD1);
                centrality8 = centrality4;
                if( doutMC->mva > 0.9999){
                    tskim->Fill();
                }
                // cout << idxData << "fill data" << endl;
                idxData++;
            }
        }

        // 마지막 파일 저장
        if (fout) {
            tskim->Write();
            fout->Write();
            fout->Close();
        }

    // auto res = pool.Map(job, ROOT::TSeqI(workers));
    std::cout << "Done" << std::endl;

}
