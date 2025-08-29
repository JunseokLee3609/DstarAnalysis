#include <fstream>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"

// ë£¨í¸ íì¼ ìì± í¨ì
TFile* createOutputFile(const std::string& date, int jobIdx) {
    // ë°°ì¹ ë²í¸ì ë§ë ìë¡ì´ íì¼ì ìì±
    std::string fileName = Form("Data/flatSkimForBDT_Mix_100to1_OnlyNonPrompt_%d_%s.root", 
                                 jobIdx, date.c_str());
    TFile* fout = new TFile(fileName.c_str(), "RECREATE");
    std::cout << "Output file " << fileName << " has been created." << std::endl;
    return fout;
}

void FlatMixWithPromptNP(int start, int end, int jobIdx){


    const unsigned int workers = 1;
    string date = "250426";

    

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
        std::unique_ptr<TChain> chainMC(new TChain(treeName.c_str()));
        std::unique_ptr<TChain> chainMCHighPT(new TChain(treeName.c_str()));
        std::unique_ptr<TChain> chainMCNP(new TChain(treeName.c_str()));
        std::unique_ptr<TChain> chainData(new TChain(treeName.c_str()));

        std::unique_ptr<TChain> evt_chainMC(new TChain(treeNameEvt.c_str()));
        std::unique_ptr<TChain> evt_chainMCHighPT(new TChain(treeNameEvt.c_str()));
        std::unique_ptr<TChain> evt_chainMCNP(new TChain(treeNameEvt.c_str()));
        std::unique_ptr<TChain> evt_chainData(new TChain(treeNameEvt.c_str()));
        //chainMCNP->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCNonPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/nonpromptD0ToKPi_PT-0_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCNonPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/250425_164900/0000/*.root");
        //chainMCNP->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCNonPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/nonpromptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCNonPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131600/0000/*.root");
        //chainMCHighPT->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCNonPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/nonpromptD0ToKPi_PT-8_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCNonPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131624/0000/*.root");
        chainMCHighPT->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/promptD0ToKPi_PT-8_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131450/0000/*.root");
        chainMC->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/promptD0ToKPi_PT-0_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/250425_164744/0000/*.root");
        chainMC->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131426/0000/*.root");
        chainData->Add("/u/user/jun502s/SE_UserHome/Run3_2023/Data/SkimMVA/D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/HIPhysicsRawPrime2/crab_D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/250424_165928/0000/*.root");
        //evt_chainMCNP->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCNonPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/nonpromptD0ToKPi_PT-0_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCNonPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/250425_164900/0000/*.root");
        //evt_chainMCNP->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCNonPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/nonpromptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCNonPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131600/0000/*.root");
        //evt_chainMCHighPT->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCNonPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/nonpromptD0ToKPi_PT-8_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCNonPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131624/0000/*.root");
        evt_chainMCHighPT->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/promptD0ToKPi_PT-8_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT8_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131450/0000/*.root");
        evt_chainMC->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/promptD0ToKPi_PT-0_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT0_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr24_v2/250425_164744/0000/*.root");
        evt_chainMC->Add("/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131426/0000/*.root");
        evt_chainData->Add("/u/user/jun502s/SE_UserHome/Run3_2023/Data/SkimMVA/D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/HIPhysicsRawPrime2/crab_D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/250424_165928/0000/*.root");

        //evt_chainMC->Add("/home/CMS/Run3_2023/MC/SkimMVA/D0Ana_MC_Step1_PromptD0DPt1_CMSSW_13_2_13_MVA_17Mar2025_v1/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step1_PromptD0DPt1_CMSSW_13_2_13_MVA_17Mar2025_v1/250317_130407/0000/*.root");
        //evt_chainMC->Add("/home/CMS/Run3_2023/MC/SkimMVA/D0Ana_MC_Step1_Prompt_DPt0_CMSSW_13_2_13_MVA_17Mar2025_v1/promptD0ToKPi_PT-0_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step1_Prompt_DPt0_CMSSW_13_2_13_MVA_17Mar2025_v1/250317_132646/0000/*.root");
        //evt_chainMCHighPT->Add("/home/CMS/Run3_2023/MC/SkimMVA/D0Ana_MC_Step1_Prompt_DPt8_CMSSW_13_2_13_MVA_17Mar2025_v1/promptD0ToKPi_PT-8_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step1_Prompt_DPt8_CMSSW_13_2_13_MVA_17Mar2025_v1/250317_132712/0000/*.root");
        //evt_chainMCNP->Add("/home/CMS/Run3_2023/MC/SkimMVA/D0Ana_MC_Step1_NonPrompt_DPt0_CMSSW_13_2_13_MVA_17Mar2025_v1/nonpromptD0ToKPi_PT-0_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step1_NonPrompt_DPt0_CMSSW_13_2_13_MVA_17Mar2025_v1/250317_132838/0000/*.root");
        //evt_chainMCHighPT->Add("/home/CMS/Run3_2023/MC/SkimMVA/D0Ana_MC_Step1_NonPrompt_DPt8_CMSSW_13_2_13_MVA_17Mar2025_v1/nonpromptD0ToKPi_PT-8_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step1_NonPrompt_DPt8_CMSSW_13_2_13_MVA_17Mar2025_v1/250317_133232/0000/*.root");
        //evt_chainMCNP->Add("/home/CMS/Run3_2023/MC/SkimMVA/D0Ana_MC_Step1_NonPrompt_DPt1_CMSSW_13_2_13_MVA_17Mar2025_v1/nonpromptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step1_NonPrompt_DPt1_CMSSW_13_2_13_MVA_17Mar2025_v1/250317_132733/0000/*.root");
        //evt_chainData->Add("/home/CMS/Run3_2023/Data/SkimMVA/D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_17Mar2025_v1/HIPhysicsRawPrime0/crab_D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_17Mar2025_v1/250317_130532/0000/*.root");


        Short_t centrality1, centrality2, centrality3, centrality4, centrality5, centrality6, centrality7, centrality8;
        evt_chainMC->SetBranchAddress("centrality", &centrality1);
        evt_chainMCHighPT->SetBranchAddress("centrality", &centrality2);
        evt_chainMCNP->SetBranchAddress("centrality", &centrality3);
        evt_chainData->SetBranchAddress("centrality", &centrality4);

    
        using namespace DataFormat;
        simpleDMCTreeevtMVA* dinMC=new simpleDMCTreeevtMVA();
        simpleDMCTreeevtMVA* dinMCHighPT=new simpleDMCTreeevtMVA();
        simpleDMCTreeevtMVA* dinMCNP=new simpleDMCTreeevtMVA();
        simpleDMCTreeflat* doutMC=new simpleDMCTreeflat();
        simpleDTreeevt* dinData =new simpleDTreeevt();
        // simpleDTreeflat* doutData =new simpleDTreeflat();
        // simpleDTreeflat doutData;
        // cout << treeMC->GetEntries() << " " << treeData->GetEntries() << endl;
        cout << chainMC->GetEntries() << " " << chainData->GetEntries() << endl;
        // dinMC->setTree<TTree>(treeMC);
        // dinData->setTree<TTree>(treeData);
        dinMC->setTree<TChain>(chainMC.get());
        dinMCHighPT->setTree<TChain>(chainMCHighPT.get());
        dinMCNP->setTree<TChain>(chainMCNP.get());
        dinData->setTree<TChain>(chainData.get());
        int batchSize = 100000;
        int batchIdx = 0;
        TFile* fout = createOutputFile(date, jobIdx);
        TTree* tskim = new TTree("skimTreeFlat", "");
		bool isPR = false;
		tskim->Branch("isPrompt", &isPR);
        tskim->Branch("cBin", &centrality8);
		long long nEvtPR = chainMC->GetEntries();
        int totEvt = std::min({nEvtPR, chainData->GetEntries()});
		cout << "totEvt : " << totEvt << "(" << nEvtPR << ", " << chainData->GetEntries() << ")" << endl;
        // totEvt = 1000000;
        cout << "Total events: " << totEvt << endl;
        // totEvt = chain->GetEntries();
        // doutData->setOutputTree(tskim);
        doutMC->setOutputTree(tskim);

		long long nEvtNP = chainMCNP->GetEntries();
		long long nEvtHP = chainMCHighPT->GetEntries();
		int skipHP = nEvtPR/nEvtHP;
		if(skipHP == 0) skipHP =1;
        // int numFilesPerWorker = totEvt / workers;
        // int start = jobIdx*numFilesPerWorker;
        // int end = (jobIdx !=(workers-1)) ? (jobIdx+1)*numFilesPerWorker : totEvt;
        int skip = 1;

        short PS = 0;
        int counts = 0;
        cout << "loopstart" << endl;
        end = std::min(end, totEvt);
	if(end<0) end = totEvt;
		//end = std::max(end, 1);
		cout << "End : " << end << endl;
        int idxMC = 0;
        int idxMCNP = 0;
        int idxMCHP = 0;
        int idxData = 0;
        for (auto iEvt : ROOT::TSeqU(start, end, skip))
        {
            // cout << "start" << start << " end" << end << " iEvt" << iEvt << endl;
            if ((iEvt % 100) == 0)
            {
                std::cout << "Processing event " << iEvt << std::endl;
            }
            // try {

            // if (status == 0 || status2 == 0) {
            //     std::cout << "Event " << iEvt << " skipped due to error." << std::endl;
            //     continue;
            // }

            // std::cout << "Processing event " << iEvt << std::endl;

            if( iEvt < nEvtPR){
				chainMC->GetEntry(iEvt);
				evt_chainMC->GetEntry(iEvt);
            	for (auto iD1 : ROOT::TSeqI(dinMC->candSize))
            	{
            	    if (dinMC->matchGEN[iD1] == true)
            	    {
            	        doutMC->isMC = true;
            	        doutMC->copyDn(*dinMC, iD1);
						isPR = doutMC->matchGen_D1ancestorFlavor_ !=5 ? true : false;
                        centrality8 = centrality1;
            	        tskim->Fill();
            	    }
            	    else
            	    {
            	        if (idxMC % 100 == 0)
            	        {
            	            doutMC->isMC = true;
            	            doutMC->copyDn(*dinMC, iD1);
							isPR= true;
                            centrality8 = centrality1;
            	            tskim->Fill();
            	        // cout << idxMC << "fill mc" << endl;
            	            
            	        }
            	            idxMC++;
            	    }
            	}
			}
			if(iEvt%skipHP == 0 && iEvt < nEvtHP){
            	chainMCHighPT->GetEntry(iEvt);
            	evt_chainMCHighPT->GetEntry(iEvt);
            	for (auto iD1 : ROOT::TSeqI(dinMCHighPT->candSize))
            	{
            	    if (dinMCHighPT->matchGEN[iD1] == true)
            	    {
            	        doutMC->isMC = true;
            	        doutMC->copyDn(*dinMCHighPT, iD1);
						isPR = doutMC->matchGen_D1ancestorFlavor_ !=5 ? true : false;
                        centrality8 = centrality2;
            	        tskim->Fill();
            	    }
            	    else
            	    {
            	        if (idxMCHP % 100 == 0)
            	        {
            	            doutMC->isMC = true;
            	            doutMC->copyDn(*dinMCHighPT, iD1);
							isPR= true;
                            centrality8 = centrality2;
            	            tskim->Fill();
            	        // cout << idxMCNP << "fill mc" << endl;
            	            
            	        }
            	            idxMCHP++;
            	    }
            	}
			}
	//	if(iEvt < nEvtNP){
        //    	chainMCNP->GetEntry(iEvt);
        //    	evt_chainMCNP->GetEntry(iEvt);
        //    	for (auto iD1 : ROOT::TSeqI(dinMCNP->candSize))
        //    	{
        //    	    if (dinMCNP->matchGEN[iD1] == true)
        //    	    {
        //    	        doutMC->isMC = true;
        //    	        doutMC->copyDn(*dinMCNP, iD1);
	//					isPR = doutMC->matchGen_D1ancestorFlavor_ !=5 ? true : false;
        //                centrality8 = centrality3;
        //    	        tskim->Fill();
        //    	    }
        //    	    else
        //    	    {
        //    	        if (idxMCNP % 100 == 0)
        //    	        {
        //    	            doutMC->isMC = true;
        //    	            doutMC->copyDn(*dinMCNP, iD1);
	//						isPR= false;
        //                    centrality8 = centrality3;
        //    	            tskim->Fill();
        //    	        // cout << idxMCNP << "fill mc" << endl;
        //    	            
        //    	        }
        //    	            idxMCNP++;
        //    	    }
        //    	}
	//		}
            chainData->GetEntry(iEvt);
            evt_chainData->GetEntry(iEvt);
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
                    centrality8 = centrality4;
                    tskim->Fill();
                    // cout << idxData << "fill data" << endl;
                }
                idxData++;
            }
        }
        // break;
        // if ((iEvt + 1) % batchSize == 0) {
        //         // ê¸°ì¡´ íì¼ ì ì¥ í ìë¡ì´ íì¼ì ì´ê¸°
        //         tskim->Write();
        //         fout->Write();
        //         fout->Close();

        //         // ë°°ì¹ ë²í¸ ì¦ê° ë° ìë¡ì´ íì¼ ìì±
        //         batchIdx++;
        //         fout = createOutputFile(date, jobIdx, batchIdx);
        //         tskim = new TTree("skimTreeFlat", "");
        //         doutMC->setOutputTree(tskim);
        //     }
        // break;
        // }
        

        // ë§ì§ë§ íì¼ ì ì¥
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
