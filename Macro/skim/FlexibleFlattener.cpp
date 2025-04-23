#include <fstream>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"

void loadRootFilesRecursively(TChain* chain, const std::string& folderPath) {
    
    if(TString(folderPath).EndsWith(".root")) {
                     std::cout << "Adding file directly: " << folderPath << std::endl;
                     chain->Add(folderPath.c_str());
                return; // Nothing more to do if it was a file path
    }
            // If it's a directory, proceed to list its contents below
            

        // Original logic for handling directories
    TSystemDirectory dir(folderPath.c_str(), folderPath.c_str());
    TList* files = dir.GetListOfFiles();

    if (!files) return;

    TIter next(files);
    TSystemFile* file;

    while ((file = (TSystemFile*)next())) {
        std::string fileName = file->GetName();

        // Skip "." and ".."
	if (fileName == "." || fileName == "..") continue;

	std::string fullPath = folderPath + "/" + fileName;

	if (file->IsDirectory()) {
		// Recursive call for subdirectories
		loadRootFilesRecursively(chain, fullPath);
	} else if (fileName.find(".root") != std::string::npos) {

		std::cout << "Adding: " << fullPath << std::endl;
		chain->Add(fullPath.c_str());
	}
    }
}
        
        

enum class ParticleType {
    D0,    // D0 입자
    DStar  // D* 입자
};

// 루트 파일 생성 함수
TFile* createOutputFile(const std::string& outputPath, const std::string& prefix, int jobIdx, const std::string& date) {
    // 출력 경로 확인 및 생성
    gSystem->mkdir(outputPath.c_str(), true);
    
    // 파일 경로 처리
    std::string path = outputPath;
    if (!path.empty() && path.back() != '/') path += '/';
    
    // 파일명 생성
    std::string fileName = Form("%s%s_%d_%s.root", path.c_str(), prefix.c_str(), jobIdx, date.c_str());
    TFile* fout = new TFile(fileName.c_str(), "RECREATE");
    std::cout << "Output file " << fileName << " has been created." << std::endl;
    return fout;
}

// 유연한 혼합 함수
void FlexibleMix(
    const std::string& mcPath,            // MC 파일 경로 패턴
    const std::string& dataPath,          // 데이터 파일 경로 패턴
    const std::string& treeName,          // 트리 이름
    const std::string& outputPath,        // 출력 파일 경로
    const std::string& outputPrefix,      // 출력 파일 접두사
    int start,                            // 시작 이벤트
    int end,                              // 종료 이벤트 (-1은 모든 이벤트)
    int jobIdx,                           // 작업 인덱스
    ParticleType particleType = ParticleType::D0, // 입자 타입 (D0 또는 DStar)
    int mcSampleRate = 100,               // MC 샘플링 비율 (matchGEN=false인 경우)
    int dataSampleRate = 100,             // 데이터 샘플링 비율
    const std::string& date = "20250320"  // 날짜 문자열
) {
    std::cout << "Starting FlexibleMix job #" << jobIdx << std::endl;
    std::cout << "Processing MC files: " << mcPath << std::endl;
    std::cout << "Processing Data files: " << dataPath << std::endl;
    std::cout << "Particle type: " << (particleType == ParticleType::D0 ? "D0" : "DStar") << std::endl;

    
    // MC와 데이터 체인 생성
    std::unique_ptr<TChain> chainMC(new TChain(treeName.c_str()));
    std::unique_ptr<TChain> chainData(new TChain(treeName.c_str()));
    
    // 파일 추가
    chainMC->Add(mcPath.c_str());
    chainData->Add(dataPath.c_str());
    
    using namespace DataFormat;
    
    // 입자 타입에 따라 적절한 클래스 인스턴스 생성
    void* dinMCPtr = nullptr;
    void* doutMCPtr = nullptr;
    void* dinDataPtr = nullptr;
    
    if (particleType == ParticleType::D0) {
        // D0 입자 처리를 위한 클래스 사용
        simpleDMCTreeevt* dinMC = new simpleDMCTreeevt();
        simpleDMCTreeflat* doutMC = new simpleDMCTreeflat();
        simpleDTreeevt* dinData = new simpleDTreeevt();
        
        dinMCPtr = dinMC;
        doutMCPtr = doutMC;
        dinDataPtr = dinData;
        
        // 체인 설정
        dinMC->setTree<TChain>(chainMC.get());
        dinData->setTree<TChain>(chainData.get());
    } else {
        // DStar 입자 처리를 위한 클래스 사용
        simpleDStarMCTreeevt* dinMC = new simpleDStarMCTreeevt();
        simpleDStarMCTreeflat* doutMC = new simpleDStarMCTreeflat();
        simpleDStarDataTreeevt* dinData = new simpleDStarDataTreeevt();
        
        dinMCPtr = dinMC;
        doutMCPtr = doutMC;
        dinDataPtr = dinData;
        
        // 체인 설정
        dinMC->setTree<TChain>(chainMC.get());
        dinData->setTree<TChain>(chainData.get());
    }
    
    // 로드된 이벤트 수 확인
    std::cout << "MC entries: " << chainMC->GetEntries() << ", Data entries: " << chainData->GetEntries() << std::endl;
    
    // 출력 파일 및 트리 설정
    TFile* fout = createOutputFile(outputPath, outputPrefix, jobIdx, date);
    TTree* tskim = new TTree("skimTreeFlat", "Flattened Skim Tree");
    
    // 출력 트리 설정
    if (particleType == ParticleType::D0) {
        ((simpleDMCTreeflat*)doutMCPtr)->setOutputTree(tskim);
    } else {
        ((simpleDStarMCTreeflat*)doutMCPtr)->setOutputTree(tskim);
    }
    
    // 처리할 이벤트 수 결정
    int totEvt = std::min(chainMC->GetEntries(), chainData->GetEntries());
    std::cout << "Total events available: " << totEvt << std::endl;
    
    // 종료 이벤트 조정
    end = (end == -1) ? totEvt : std::min(end, totEvt);
    std::cout << "Processing events from " << start << " to " << end << std::endl;
    
    int skip = 1;
    int idxMC = 0;
    int idxData = 0;
    
    // 메인 처리 루프
    for (auto iEvt : ROOT::TSeqU(start, end, skip)) {
        if ((iEvt % 10000) == 0) {
            std::cout << "Processing event " << iEvt << " / " << end << std::endl;
        }
        
        // MC 처리
        chainMC->GetEntry(iEvt);
        
        if (particleType == ParticleType::D0) {
            simpleDMCTreeevt* dinMC = (simpleDMCTreeevt*)dinMCPtr;
            simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                if (dinMC->matchGEN[iD1] == true) {
                    // GEN 매치된 항목은 모두 저장
                    doutMC->isMC = true;
                    doutMC->copyDn(*dinMC, iD1);
                    tskim->Fill();
                } else {
                    // 나머지는 샘플링 비율에 따라 저장
                    if (idxMC % mcSampleRate == 0) {
                        doutMC->isMC = true;
                        doutMC->copyDn(*dinMC, iD1);
                        tskim->Fill();
                    }
                    idxMC++;
                }
            }
        } else {
            simpleDStarMCTreeevt* dinMC = (simpleDStarMCTreeevt*)dinMCPtr;
            simpleDStarMCTreeflat* doutMC = (simpleDStarMCTreeflat*)doutMCPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                if (dinMC->matchGEN[iD1] == true) {
                    // GEN 매치된 항목은 모두 저장
                    doutMC->isMC = true;
                    doutMC->copyDn(*dinMC, iD1);
                    tskim->Fill();
                } else {
                    // 나머지는 샘플링 비율에 따라 저장
                    if (idxMC % mcSampleRate == 0) {
                        doutMC->isMC = true;
                        doutMC->copyDn(*dinMC, iD1);
                        tskim->Fill();
                    }
                    idxMC++;
                }
            }
        }
        
        // Data 처리
        chainData->GetEntry(iEvt);
        
        if (particleType == ParticleType::D0) {
            simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr;
            simpleDTreeevt* dinData = (simpleDTreeevt*)dinDataPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                // 샘플링 비율에 따라 저장
                if (idxData % dataSampleRate == 0) {
                    doutMC->isMC = false;
                    doutMC->isSwap = 0;
                    doutMC->matchGEN = 0;
                    doutMC->copyDn(*dinData, iD1);
                    tskim->Fill();
                }
                idxData++;
            }
        } else {
            simpleDStarMCTreeflat* doutMC = (simpleDStarMCTreeflat*)doutMCPtr;
            simpleDStarDataTreeevt* dinData = (simpleDStarDataTreeevt*)dinDataPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                // 샘플링 비율에 따라 저장
                if (idxData % dataSampleRate == 0) {
                    doutMC->isMC = false;
                    doutMC->isSwap = 0;
                    doutMC->matchGEN = 0;
                    doutMC->copyDn(*dinData, iD1);
                    tskim->Fill();
                }
                idxData++;
            }
        }
    }
    
    // 파일 저장 및 정리
    if (fout) {
        std::cout << "Writing output file with " << tskim->GetEntries() << " entries..." << std::endl;
        tskim->Write();
        fout->Write();
        fout->Close();
        delete fout;
    }
    
    // 메모리 정리
    if (particleType == ParticleType::D0) {
        delete (simpleDMCTreeevt*)dinMCPtr;
        delete (simpleDMCTreeflat*)doutMCPtr;
        delete (simpleDTreeevt*)dinDataPtr;
    } else {
        delete (simpleDStarMCTreeevt*)dinMCPtr;
        delete (simpleDStarMCTreeflat*)doutMCPtr;
        delete (simpleDStarDataTreeevt*)dinDataPtr;
    }
    
    std::cout << "FlexibleMix job #" << jobIdx << " completed successfully." << std::endl;
}

// Data만 처리하는 함수
void FlexibleData(
    const std::string& dataPath,          // 데이터 파일 경로 패턴
    const std::string& treeName,          // 트리 이름
    const std::string& eventInfoTreeName, // Event Info 트리 이름 (추가)
    const std::string& outputPath,        // 출력 파일 경로
    const std::string& outputPrefix,      // 출력 파일 접두사
    int start,                            // 시작 이벤트
    int end,                              // 종료 이벤트 (-1은 모든 이벤트)
    int jobIdx,                           // 작업 인덱스
    ParticleType particleType = ParticleType::D0, // 입자 타입 (D0 또는 DStar)
    bool doCent = true,                 // 중앙값 설정 여부 (추가)
    const std::string& date = "20250320"  // 날짜 문자열
) {
    std::cout << "Starting FlexibleData job #" << jobIdx << std::endl;
    std::cout << "Processing Data files: " << dataPath << std::endl;
    std::cout << "Particle type: " << (particleType == ParticleType::D0 ? "D0" : "DStar") << std::endl;
    std::cout << "Setting doCent: " << (doCent ? "true" : "false") << std::endl; // 추가된 로그

    // 데이터 메인 트리 체인 생성
    std::unique_ptr<TChain> chainData(new TChain(treeName.c_str()));
    loadRootFilesRecursively(chainData.get(), dataPath);

    // Event Info 체인 (doCent가 true일 경우에만 생성 및 사용)
    std::unique_ptr<TChain> chainEventInfo = nullptr;
    Short_t centrality = -99; // 기본값 초기화
    if (doCent) {
        chainEventInfo.reset(new TChain(eventInfoTreeName.c_str()));
        loadRootFilesRecursively(chainEventInfo.get(), dataPath); // 데이터 경로에서 Event Info 로드
        if (chainEventInfo->GetEntries() > 0) {
            chainEventInfo->SetBranchAddress("centrality", &centrality);
            std::cout << "Centrality branch linked from " << eventInfoTreeName << std::endl;
        } else {
            std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty in data files. Centrality will not be saved." << std::endl;
            doCent = false; // Centrality 처리 비활성화
        }
    }

    using namespace DataFormat;

    // 입자 타입에 따라 적절한 클래스 인스턴스 생성
    void* dinDataPtr = nullptr;
    void* doutDataPtr = nullptr;

    if (particleType == ParticleType::D0) {
        simpleDTreeevt* dinData = new simpleDTreeevt();
        simpleDMCTreeflat* doutData = new simpleDMCTreeflat(); // MC 타입 출력 객체 사용 (공통 구조)
        dinDataPtr = dinData;
        doutDataPtr = doutData;
        dinData->setTree<TChain>(chainData.get());
    } else {
        simpleDStarDataTreeevt* dinData = new simpleDStarDataTreeevt();
        simpleDStarMCTreeflat* doutData = new simpleDStarMCTreeflat(); // MC 타입 출력 객체 사용 (공통 구조)
        dinDataPtr = dinData;
        doutDataPtr = doutData;
        dinData->setTree<TChain>(chainData.get());
    }

    // 로드된 이벤트 수 확인
    long long nEntriesData = chainData->GetEntries();
    std::cout << "Data entries: " << nEntriesData << std::endl;
    if (doCent && chainEventInfo) {
        long long nEntriesEventInfo = chainEventInfo->GetEntries();
        std::cout << "Event Info entries: " << nEntriesEventInfo << std::endl;
        if (nEntriesData != nEntriesEventInfo) {
            std::cerr << "Warning: Mismatch between main data tree entries (" << nEntriesData
                      << ") and event info entries (" << nEntriesEventInfo << "). Centrality might be incorrect." << std::endl;
        }
    }

    // 출력 파일 및 트리 설정
    TFile* fout = createOutputFile(outputPath, outputPrefix, jobIdx, date);
    TTree* tskim = new TTree("skimTreeFlat", "Flattened Data Skim Tree");

    // 출력 트리 설정
    if (particleType == ParticleType::D0) {
        ((simpleDMCTreeflat*)doutDataPtr)->setOutputTree(tskim);
    } else {
        ((simpleDStarMCTreeflat*)doutDataPtr)->setOutputTree(tskim);
    }

    // 출력 트리에 centrality 브랜치 추가 (doCent가 true일 경우)
    if (doCent) {
        tskim->Branch("centrality", &centrality, "centrality/S");
        std::cout << "Added 'centrality' branch to output tree." << std::endl;
    }

    // 처리할 이벤트 수 결정
    int totEvt = nEntriesData;
    std::cout << "Total events available: " << totEvt << std::endl;

    // 종료 이벤트 조정
    end = (end == -1) ? totEvt : std::min(end, totEvt);
    std::cout << "Processing events from " << start << " to " << end << std::endl;

    int skip = 1;
    int idxData = 0;

    // 메인 처리 루프
    for (auto iEvt : ROOT::TSeqU(start, end, skip)) {
        if ((iEvt % 100000) == 0) {
            std::cout << "Processing event " << iEvt << " / " << end << std::endl;
        }

        // Event Info 트리에서 현재 이벤트의 centrality 가져오기 (doCent가 true일 경우)
        if (doCent && chainEventInfo) {
            chainEventInfo->GetEntry(iEvt);
        } else {
            centrality = -1.0f; // centrality를 사용하지 않으면 기본값 유지
        }

        // Data 메인 트리 처리
        chainData->GetEntry(iEvt);

        if (particleType == ParticleType::D0) {
            simpleDMCTreeflat* doutData = (simpleDMCTreeflat*)doutDataPtr;
            simpleDTreeevt* dinData = (simpleDTreeevt*)dinDataPtr;

            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                    if(dinData[iD1].pT > 100 || abs(dinData[iD1].y) >1 || dinData[iD1].pTD1 <1 || dinData[iD1].pTD2 <1 || dinData[iD1].EtaD1 >2.4 || dinData[iD1].EtaD2 >2.4) continue;
                    doutData->isMC = false;
                    doutData->isSwap = 0;
                    doutData->matchGEN = 0;
                    doutData->copyDn<simpleDTreeevt>(*dinData, iD1);
                    tskim->Fill();
            }
        } else {
            simpleDStarMCTreeflat* doutData = (simpleDStarMCTreeflat*)doutDataPtr;
            simpleDStarDataTreeevt* dinData = (simpleDStarDataTreeevt*)dinDataPtr;

            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                    doutData->isMC = false;
                    doutData->isSwap = 0;
                    doutData->matchGEN = 0;
                    doutData->copyDn(*dinData, iD1);
                    tskim->Fill();
            }
        }
    }

    // 파일 저장 및 정리
    if (fout) {
        std::cout << "Writing output file with " << tskim->GetEntries() << " entries..." << std::endl;
        fout->cd(); // 파일 컨텍스트 활성화
        tskim->Write();
        // fout->Write(); // Write()는 모든 객체를 다시 쓰므로 필요 없음
        fout->Close();
        std::cout << "Output file " << fout->GetName() << " has been created." << std::endl;
        delete fout;
    }

    // 메모리 정리
    if (particleType == ParticleType::D0) {
        delete (simpleDTreeevt*)dinDataPtr;
        delete (simpleDMCTreeflat*)doutDataPtr;
    } else {
        delete (simpleDStarDataTreeevt*)dinDataPtr;
        delete (simpleDStarMCTreeflat*)doutDataPtr;
    }

    std::cout << "FlexibleData job #" << jobIdx << " completed successfully." << std::endl;
}

// MC만 처리하는 함수
void FlexibleMC(
    const std::string& mcPath,            // MC 파일 경로 패턴
    const std::string& treeName,          // 트리 이름
    const std::string& eventInfoTreeName, // Event Info 트리 이름 (예: "eventinfo/tree")
    const std::string& outputPath,        // 출력 파일 경로
    const std::string& outputPrefix,      // 출력 파일 접두사
    int start,                            // 시작 이벤트
    int end,                              // 종료 이벤트 (-1은 모든 이벤트)
    int jobIdx,                           // 작업 인덱스
    ParticleType particleType = ParticleType::D0, // 입자 타입 (D0 또는 DStar)
    bool setGEN = true,                  // GEN 트리 설정 여부
    bool doCent = true,                 // 중앙값 설정 여부
    const std::string& date = "20250331"  // 날짜 문자열
) {
    std::cout << "Starting FlexibleMC job #" << jobIdx << std::endl;
    std::cout << "Processing MC files: " << mcPath << std::endl;
    std::cout << "Particle type: " << (particleType == ParticleType::D0 ? "D0" : "DStar") << std::endl;
    std::cout << "Setting GEN tree: " << (setGEN ? "true" : "false") << std::endl;
    std::cout << "Setting doCent: " << (doCent ? "true" : "false") << std::endl;

    // 메인 트리 체인
    std::unique_ptr<TChain> chainMC(new TChain(treeName.c_str()));
    loadRootFilesRecursively(chainMC.get(), mcPath);

    // Event Info 체인 (doCent가 true일 경우에만 생성 및 사용)
    std::unique_ptr<TChain> chainEventInfo = nullptr;
    Short_t centrality = -99; // 기본값 초기화
    if (doCent) {
        chainEventInfo.reset(new TChain(eventInfoTreeName.c_str()));
        loadRootFilesRecursively(chainEventInfo.get(), mcPath);
        if (chainEventInfo->GetEntries() > 0) {
            chainEventInfo->SetBranchAddress("centrality", &centrality);
            std::cout << "Centrality branch linked from " << eventInfoTreeName << std::endl;
        } else {
            std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty. Centrality will not be saved." << std::endl;
            doCent = false; // Centrality 처리 비활성화
        }
    }

    using namespace DataFormat;

    void* dinMCPtr = nullptr;
    void* doutMCPtr = nullptr;

    if (particleType == ParticleType::D0) {
        simpleDMCTreeevt* dinMC = new simpleDMCTreeevt();
        simpleDMCTreeflat* doutMC = new simpleDMCTreeflat();
        dinMCPtr = dinMC;
        doutMCPtr = doutMC;
        dinMC->setTree<TChain>(chainMC.get());
        if(setGEN) dinMC->setGENTree<TChain>(chainMC.get());
    } else {
        simpleDStarMCTreeevt* dinMC = new simpleDStarMCTreeevt();
        simpleDStarMCTreeflat* doutMC = new simpleDStarMCTreeflat();
        dinMCPtr = dinMC;
        doutMCPtr = doutMC;
        dinMC->setTree<TChain>(chainMC.get());
        if(setGEN) dinMC->setGENTree<TChain>(chainMC.get());
    }

    long long nEntriesMC = chainMC->GetEntries();
    std::cout << "MC entries: " << nEntriesMC << std::endl;
    if (doCent && chainEventInfo) {
        long long nEntriesEventInfo = chainEventInfo->GetEntries();
        std::cout << "Event Info entries: " << nEntriesEventInfo << std::endl;
        if (nEntriesMC != nEntriesEventInfo) {
            std::cerr << "Warning: Mismatch between main tree entries (" << nEntriesMC
                      << ") and event info entries (" << nEntriesEventInfo << "). Centrality might be incorrect." << std::endl;
        }
    }

    TFile* fout = createOutputFile(outputPath, outputPrefix, jobIdx, date);
    TTree* tskim = new TTree("skimTreeFlat", "Flattened MC Skim Tree");
    TTree* tskimGEN = nullptr;
    if(setGEN) tskimGEN = new TTree("skimGENTreeFlat", "Flattened MC Skim GEN Tree");

    if (particleType == ParticleType::D0) {
        ((simpleDMCTreeflat*)doutMCPtr)->setOutputTree(tskim);
        if(setGEN) ((simpleDMCTreeflat*)doutMCPtr)->setGENOutputTree(tskimGEN);
    } else {
        ((simpleDStarMCTreeflat*)doutMCPtr)->setOutputTree(tskim);
        if(setGEN) ((simpleDStarMCTreeflat*)doutMCPtr)->setGENOutputTree(tskimGEN);
    }

    if (doCent) {
        tskim->Branch("centrality", &centrality, "centrality/S");
        std::cout << "Added 'centrality' branch to output tree(s)." << std::endl;
    }

    int totEvt = nEntriesMC;
    std::cout << "Total events available: " << totEvt << std::endl;

    end = (end == -1) ? totEvt : std::min(end, totEvt);
    std::cout << "Processing events from " << start << " to " << end << std::endl;

    int skip = 1;
    int idxMC = 0;

    for (auto iEvt : ROOT::TSeqU(start, end, skip)) {
        if ((iEvt % 10000) == 0) {
            std::cout << "Processing event " << iEvt << " / " << end << std::endl;
        }

        if (doCent && chainEventInfo) {
            chainEventInfo->GetEntry(iEvt);
        } else {
            centrality = -1.0f;
        }

        chainMC->GetEntry(iEvt);

        if (particleType == ParticleType::D0) {
            simpleDMCTreeevt* dinMC = (simpleDMCTreeevt*)dinMCPtr;
            simpleDMCTreeflat *doutMC = (simpleDMCTreeflat *)doutMCPtr;

            for (auto iD1 : ROOT::TSeqI(dinMC->candSize))
            {
                if(dinData[iD1].pT > 100 || abs(dinData[iD1].y) >1 || dinData[iD1].pTD1 <1 || dinData[iD1].pTD2 <1 || dinData[iD1].EtaD1 >2.4 || dinData[iD1].EtaD2 >2.4) continue;
                doutMC->isMC = true;
                doutMC->copyDn(*dinMC, iD1);
                tskim->Fill();
            }
            if (setGEN && tskimGEN)
            {
                for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen))
                {
                    doutMC->isMC = true;
                    doutMC->copyGENDn(*dinMC, iD1);
                    tskimGEN->Fill();
                }
            }
        }
        else
        {
            simpleDStarMCTreeevt *dinMC = (simpleDStarMCTreeevt *)dinMCPtr;
            simpleDStarMCTreeflat *doutMC = (simpleDStarMCTreeflat *)doutMCPtr;

            for (auto iD1 : ROOT::TSeqI(dinMC->candSize))
            {
                doutMC->isMC = true;
                doutMC->copyDn(*dinMC, iD1);
                tskim->Fill();
            }
            if (setGEN && tskimGEN)
            {
                for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen))
                {
                    doutMC->isMC = true;
                    doutMC->copyGENDn(*dinMC, iD1);
                    tskimGEN->Fill();
                }
            }
        }
    }

    if (fout) {
        std::cout << "Writing output file with " << tskim->GetEntries() << " entries..." << std::endl;
        fout->cd();
        tskim->Write();
        if (tskimGEN) {
            tskimGEN->Write();
        }
        fout->Close();
        std::cout << "Output file " << fout->GetName() << " has been created." << std::endl;
        delete fout;
    }

    if (particleType == ParticleType::D0) {
        delete (simpleDMCTreeevt*)dinMCPtr;
        delete (simpleDMCTreeflat*)doutMCPtr;
    } else {
        delete (simpleDStarMCTreeevt*)dinMCPtr;
        delete (simpleDStarMCTreeflat*)doutMCPtr;
    }
    
    std::cout << "FlexibleMC job #" << jobIdx << " completed successfully." << std::endl;
}

// 간단한 래퍼 함수 - 기본 경로로 호출
void FlexibleMixDefault(int start, int end, int jobIdx, ParticleType particleType = ParticleType::D0) {
    FlexibleMix(
        "/home/jun502s/DstarAna/DStarAnalysis/Data/0000/*.root",  // MC 경로
        "/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/*.root",  // 데이터 경로
        "d0ana_newreduced/PATCompositeNtuple",    // 트리 이름
        "../../Data",                              // 출력 경로
        "flatSkimForBDT_Mix_100to1",              // 출력 접두사
        start,
        end,
        jobIdx,
        particleType
    );
}

// main 함수 예시
int FlexibleFlattener(int start=0, int end=-1, int idx=0, int type=0, std::string path = "", std::string suffix="") {

    int start_ = start;
    int end_ = end;
    int jobIdx_ = idx;
    bool setGEN = true;
    ParticleType particleType = ParticleType::D0;
    bool doCent = true;
    std::string eventInfoTreeName = "eventinfoana/EventInfoNtuple";

    std::string mcPath = "/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MC_Step2MVA_D0Kpi_DpT_NonSwap_CMSSW_13_2_13_NoMVACut_04Apr2025_v1/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step2MVA_D0Kpi_DpT_NonSwap_CMSSW_13_2_13_NoMVACut_04Apr2025_v1/250404_044553/0000/*.root";
    std::string dataPath = "/u/user/jun502s/SE_UserHome/DStarMC/junseok/20250401_v1/DStarAnalysis_PPRef2024_DstarToKpipi_CMSSW_14_1_7_PPRef10_141X_dataRun3_Express_v3_20250401_v1/PPRefZeroBiasPlusForward10/DStarAnalysis_PPRef2024_DstarToKpipi_CMSSW_14_1_7_PPRef10_141X_dataRun3_Express_v3_20250401_v1/250401_130954/0000/*.root";


    std::string date = "";

    std::string outputPath;
    std::string outputPrefix;
    std::string treeNameMC = particleType==ParticleType::D0 ? "d0ana_mc/PATCompositeNtuple": "dStarana_mc/PATCompositeNtuple";
    std::string treeNameData = particleType==ParticleType::D0 ? "d0ana_newreduced/PATCompositeNtuple": "dStarana_mc/PATCompositeNtuple";

    if (type == 0) {
	    outputPath = particleType==ParticleType::D0 ? "./Data/FlatSample/ppMC/D0" : "./Data/FlatSample/ppMC/DStar";
	    outputPrefix = particleType==ParticleType::D0 ? "flatSkimForBDT_D0" : "flatSkimForBDT_DStar";
 	    if(!path.empty()) mcPath = path;
	    if(!suffix.empty()) outputPrefix += "_"+ suffix;
		    FlexibleMC(mcPath, treeNameMC, eventInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_,particleType, setGEN, doCent, date);

    } else if (type == 1) {
	    outputPath = particleType==ParticleType::D0 ? "./Data/FlatSample/ppData/D0" : "./Data/FlatSample/ppData/DStar";
	    outputPrefix = particleType==ParticleType::D0 ? "flatSkimForBDT_D0" : "flatSkimForBDT_DStar";

	    if(!path.empty()) dataPath = path;
	    if(!suffix.empty()) outputPrefix += "_"+suffix;

	    FlexibleData(dataPath, treeNameData, eventInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_, particleType, doCent, date);

    } else if (type == 2) {
	    outputPath = "./Data/FlatSample/ppMix/";
	    outputPrefix = "flatSkimForBDT_DStar_ppRef_NonSwapMix";
	    //FlexibleMix(dataPath, mcPath, treeName, outputPath, outputPrefix, start_, end_, jobIdx_, cut, ParticleType::DStar, setGEN, date);

    } else {
	    std::cerr << "Invalid type value: " << type << ". Must be 0 (MC), 1 (Data), or 2 (Mix)." << std::endl;
	    return 1;
    }

    return 0;
}
