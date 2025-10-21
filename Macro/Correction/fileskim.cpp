#include "EffHead.h"
#include "commonSelectionVar.h"
#include "../interface/simpleDMC.hxx"
#include "../interface/simpleAlgos.hxx"
#include "../Tools/Transformations.h"
#include "../Tools/BasicHeaders.h"
#include "../Tools/Parameters/AnalysisParameters.h"
#include "../Tools/Parameters/PhaseSpace.h"

void fileskim(
    string fileName="/home/jun502s/DstarAna/DStarAna/data/output_onlyGENMC_PbPb.root"
){
    TFile* f = TFile::Open(fileName.c_str());
    string name_tree = "dStarana_mc/VertexCompositeNtuple";
    /* Get tree*/
    auto t = (TTree*) f->Get(name_tree.c_str());
    DataFormat::simpleDStarMCTreeevt evtMC;
    evtMC.setTree(t);
    evtMC.setGENTree(t);
    int nEvts;
    nEvts = t->GetEntries();
    
    // 출력 파일 생성
    TFile* outFile = new TFile("dstar_helicity_skimmed.root", "RECREATE");
    
    // D* 관련 변수
    float Dstar_pT, Dstar_eta, Dstar_phi, Dstar_y, Dstar_mass, Dstar_cosTheta;
    
    // D0 관련 변수 (daughter 1)
    float D0_pT, D0_eta, D0_phi, D0_y, D0_mass;
    
    // Soft pion 관련 변수 (daughter 2)
    float softPion_pT, softPion_eta, softPion_phi, softPion_y, softPion_mass;
    
    // D0 decay product 1 (granddaughter 1)
    float D0Dau1_pT, D0Dau1_eta, D0Dau1_phi, D0Dau1_mass;
    
    // D0 decay product 2 (granddaughter 2)
    float D0Dau2_pT, D0Dau2_eta, D0Dau2_phi, D0Dau2_mass;
    
    // 새 TTree 생성
    TTree* outTree = new TTree("dstar_helicity", "D* variables in helicity frame");
    
    // D* 변수 등록
    outTree->Branch("Dstar_pT", &Dstar_pT, "Dstar_pT/F");
    outTree->Branch("Dstar_eta", &Dstar_eta, "Dstar_eta/F");
    outTree->Branch("Dstar_phi", &Dstar_phi, "Dstar_phi/F");
    outTree->Branch("Dstar_y", &Dstar_y, "Dstar_y/F");
    outTree->Branch("Dstar_mass", &Dstar_mass, "Dstar_mass/F");
    outTree->Branch("Dstar_cosTheta", &Dstar_cosTheta, "Dstar_cosTheta/F");
    
    // D0 변수 등록
    outTree->Branch("D0_pT", &D0_pT, "D0_pT/F");
    outTree->Branch("D0_eta", &D0_eta, "D0_eta/F");
    outTree->Branch("D0_phi", &D0_phi, "D0_phi/F");
    outTree->Branch("D0_y", &D0_y, "D0_y/F");
    outTree->Branch("D0_mass", &D0_mass, "D0_mass/F");
    
    // Soft pion 변수 등록
    outTree->Branch("softPion_pT", &softPion_pT, "softPion_pT/F");
    outTree->Branch("softPion_eta", &softPion_eta, "softPion_eta/F");
    outTree->Branch("softPion_phi", &softPion_phi, "softPion_phi/F");
    outTree->Branch("softPion_y", &softPion_y, "softPion_y/F");
    outTree->Branch("softPion_mass", &softPion_mass, "softPion_mass/F");
    
    // D0 decay products 변수 등록
    outTree->Branch("D0Dau1_pT", &D0Dau1_pT, "D0Dau1_pT/F");
    outTree->Branch("D0Dau1_eta", &D0Dau1_eta, "D0Dau1_eta/F");
    outTree->Branch("D0Dau1_phi", &D0Dau1_phi, "D0Dau1_phi/F");
    outTree->Branch("D0Dau1_mass", &D0Dau1_mass, "D0Dau1_mass/F");
    
    outTree->Branch("D0Dau2_pT", &D0Dau2_pT, "D0Dau2_pT/F");
    outTree->Branch("D0Dau2_eta", &D0Dau2_eta, "D0Dau2_eta/F");
    outTree->Branch("D0Dau2_phi", &D0Dau2_phi, "D0Dau2_phi/F");
    outTree->Branch("D0Dau2_mass", &D0Dau2_mass, "D0Dau2_mass/F");
    
    cout << "Processing " << nEvts << " events..." << endl;
    
    // 이벤트 루프
    for(int evt = 0; evt < nEvts; evt++) {
        if(evt % 10000 == 0) cout << "Processing event " << evt << "/" << nEvts << endl;
        t->GetEntry(evt);
        
        for(int i = 0; i < evtMC.candSize_gen; i++) {
            TLorentzVector Dstar, D0, softPion, D0Dau1, D0Dau2;
            
            // D* 입자 설정
            Dstar.SetPtEtaPhiM(evtMC.gen_pT[i], evtMC.gen_eta[i], evtMC.gen_phi[i], evtMC.gen_mass[i]);
            
            // D0 입자 설정
            D0.SetPtEtaPhiM(evtMC.gen_D0pT[i], evtMC.gen_D0eta[i], evtMC.gen_D0phi[i], evtMC.gen_D0mass[i]);
            
            // Soft pion 설정 - 소프트 파이온 정보를 직접 가져오거나 D* - D0로 계산
            if(evtMC.candSize_gen > 0) {
                // 실제 소프트 파이온 데이터가 있는 경우
                softPion.SetPtEtaPhiM(evtMC.gen_D1pT[i], evtMC.gen_D1eta[i], evtMC.gen_D1phi[i], evtMC.gen_D1mass[i]);
            } else {
                // 소프트 파이온 데이터가 없는 경우 D* - D0로 계산
                softPion = Dstar - D0;
            }
            
            // D0 decay products 설정
            D0Dau1.SetPtEtaPhiM(evtMC.gen_D0Dau1_pT[i], evtMC.gen_D0Dau1_eta[i], 
                               evtMC.gen_D0Dau1_phi[i], evtMC.gen_D0Dau1_mass[i]);
            D0Dau2.SetPtEtaPhiM(evtMC.gen_D0Dau2_pT[i], evtMC.gen_D0Dau2_eta[i], 
                               evtMC.gen_D0Dau2_phi[i], evtMC.gen_D0Dau2_mass[i]);
            
            // 선택 조건 적용 (현재는 모든 이벤트 저장)
            if( 
                evtMC.gen_pT[i] > DSGLPTLO && 
                fabs(evtMC.gen_y[i]) < DSGLABSY && 
                DstarDauSimpleAcc(D0,softPion) &&
                D0DauAcc(D0Dau1,D0Dau2) &&
                true) {
                // Helicity frame 계산
                TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);
                
                // D* 변수 할당
                Dstar_pT = Dstar.Pt();
                Dstar_eta = Dstar.Eta();
                Dstar_phi = Dstar.Phi();
                Dstar_y = Dstar.Rapidity();
                Dstar_mass = Dstar.M();
                Dstar_cosTheta = vect.CosTheta();
                
                // D0 변수 할당
                D0_pT = D0.Pt();
                D0_eta = D0.Eta();
                D0_phi = D0.Phi();
                D0_y = D0.Rapidity();
                D0_mass = D0.M();
                
                // Soft pion 변수 할당
                softPion_pT = softPion.Pt();
                softPion_eta = softPion.Eta();
                softPion_phi = softPion.Phi();
                softPion_y = softPion.Rapidity();
                softPion_mass = softPion.M();
                
                // D0 decay products 변수 할당
                D0Dau1_pT = D0Dau1.Pt();
                D0Dau1_eta = D0Dau1.Eta();
                D0Dau1_phi = D0Dau1.Phi();
                D0Dau1_mass = D0Dau1.M();
                
                D0Dau2_pT = D0Dau2.Pt();
                D0Dau2_eta = D0Dau2.Eta();
                D0Dau2_phi = D0Dau2.Phi();
                D0Dau2_mass = D0Dau2.M();
                
                // TTree에 데이터 채우기
                outTree->Fill();
            }
        }
    }
    
    // 출력 쓰기 및 종료
    outFile->cd();
    outTree->Write();
    outFile->Close();
    
    cout << "Skimmed data saved to dstar_helicity_skimmed.root" << endl;
}