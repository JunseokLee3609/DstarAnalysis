#include <fstream>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"

float findNcoll(int hiBin) {
    const int nbins = 200;
    const float Ncoll[200] = {1893.13, 1867.0, 1834.16, 1805.64, 1770.84, 1744.49, 1699.76, 1661.52, 1615.89, 1579.59, 1540.62, 1499.14, 1469.01, 1432.18, 1402.8, 1368.39, 1338.12, 1302.26, 1274.91, 1245.56, 1215.28, 1183.76, 1160.61, 1131.12, 1107.67, 1078.54, 1055.72, 1026.72, 1000.57, 980.728, 958.777, 936.515, 911.397, 889.182, 869.677, 853.33, 826.999, 808.145, 792.14, 769.639, 753.513, 732.883, 716.817, 697.168, 679.091, 668.056, 650.114, 631.024, 616.203, 597.835, 583.435, 571.454, 555.478, 543.589, 526.328, 511.657, 497.023, 489.255, 471.52, 461.133, 447.767, 436.993, 426.106, 412.626, 403.224, 389.71, 382.595, 371.48, 358.899, 349.179, 339.387, 330.523, 320.094, 313.254, 302.339, 292.421, 282.594, 274.834, 268.847, 259.463, 252.027, 244.561, 236.738, 229.574, 222.898, 215.138, 207.328, 200.879, 196.592, 190.921, 183.942, 176.685, 170.919, 166.96, 161.057, 154.421, 148.816, 144.84, 139.087, 134.448, 128.72, 124.905, 121.166, 116.648, 112.367, 109.012, 104.33, 100.736, 97.3484, 93.2283, 89.3299, 85.9068, 83.6446, 80.2019, 77.5299, 73.9647, 70.7606, 68.2284, 65.793, 63.4532, 60.4738, 58.2406, 55.063, 53.7287, 51.4638, 49.241, 47.0111, 45.5443, 43.1729, 41.5041, 39.5449, 37.9282, 36.8918, 34.9287, 33.1886, 31.9177, 30.756, 29.0803, 27.6721, 26.42, 25.2678, 24.2585, 23.1429, 22.0138, 21.0169, 19.8203, 19.1043, 18.1478, 17.1715, 16.3605, 15.4763, 14.7973, 14.1594, 13.3927, 12.795, 12.1059, 11.5921, 10.9751, 10.3213, 9.94434, 9.3518, 8.94274, 8.37618, 7.94437, 7.48868, 7.06923, 6.71137, 6.31856, 6.03184, 5.67048, 5.43369, 5.13727, 4.83292, 4.58846, 4.37208, 4.15225, 3.84385, 3.63752, 3.45214, 3.24892, 3.02845, 2.81715, 2.66395, 2.5053, 2.29512, 2.13703, 1.93591, 1.79771, 1.64165, 1.54375, 1.45878, 1.36718, 1.2942, 1.23934, 1.18423, 1.14467, 1.11826, 1.0863, 1.06149, 1.04497 };
    return Ncoll[hiBin];
 }

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
    const std::string& treeNameMC,          // 트리 이름
    const std::string& treeNameData,          // 트리 이름
    const std::string& eventInfoTreeName, // Event Info 트리 이름 (추가)
    const std::string& outputPath,        // 출력 파일 경로
    const std::string& outputPrefix,      // 출력 파일 접두사
    int start,                            // 시작 이벤트
    int end,                              // 종료 이벤트 (-1은 모든 이벤트)
    int jobIdx,                           // 작업 인덱스
    ParticleType particleType = ParticleType::D0, // 입자 타입 (D0 또는 DStar)
    int mcSampleRate = 100,               // MC 샘플링 비율 (matchGEN=false인 경우)
    int dataSampleRate = 100,             // 데이터 샘플링 비율
    bool doCent = true,                   // 중앙값 설정 여부 (추가)
    bool doEvtPlane = false,            // 이벤트 평면 설정 여부 (추가)
    const std::string& date = "20250320"  // 날짜 문자열
) {
    std::cout << "Starting FlexibleMix job #" << jobIdx << std::endl;
    std::cout << "Processing MC files: " << mcPath << std::endl;
    std::cout << "Processing Data files: " << dataPath << std::endl;
    std::cout << "Particle type: " << (particleType == ParticleType::D0 ? "D0" : "DStar") << std::endl;
    std::cout << "Setting doCent: " << (doCent ? "true" : "false") << std::endl;

    // MC와 데이터 메인 트리 체인 생성
    std::unique_ptr<TChain> chainMC(new TChain(treeNameMC.c_str()));
    std::unique_ptr<TChain> chainData(new TChain(treeNameData.c_str()));
    loadRootFilesRecursively(chainMC.get(), mcPath);
    loadRootFilesRecursively(chainData.get(), dataPath);

    // Event Info 체인 (doCent가 true일 경우에만 생성 및 사용)
    std::unique_ptr<TChain> chainEventInfoMC = nullptr;
    std::unique_ptr<TChain> chainEventInfoData = nullptr;
    Short_t centralityMC = -99;
    Short_t centralityData = -99;
    Float_t ncoll = -99; // Ncoll은 MC에만 해당

    if (doCent) {
        // MC Event Info
        chainEventInfoMC.reset(new TChain(eventInfoTreeName.c_str()));
        loadRootFilesRecursively(chainEventInfoMC.get(), mcPath);
        if (chainEventInfoMC->GetEntries() > 0) {
            chainEventInfoMC->SetBranchAddress("centrality", &centralityMC);
            std::cout << "MC Centrality branch linked from " << eventInfoTreeName << std::endl;
        } else {
            std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty in MC files. Centrality will not be saved for MC." << std::endl;
            // MC Centrality 처리를 비활성화하지 않고, 해당 이벤트에서 기본값을 사용하도록 유지
        }

        // Data Event Info
        chainEventInfoData.reset(new TChain(eventInfoTreeName.c_str()));
        loadRootFilesRecursively(chainEventInfoData.get(), dataPath);
        if (chainEventInfoData->GetEntries() > 0) {
            chainEventInfoData->SetBranchAddress("centrality", &centralityData);
            std::cout << "Data Centrality branch linked from " << eventInfoTreeName << std::endl;
        } else {
            std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty in Data files. Centrality will not be saved for Data." << std::endl;
            // Data Centrality 처리를 비활성화하지 않고, 해당 이벤트에서 기본값을 사용하도록 유지
        }
    }

    using namespace DataFormat;

    // 입자 타입에 따라 적절한 클래스 인스턴스 생성
    void* dinMCPtr = nullptr;
    void* doutMCPtr = nullptr; // MC와 Data 공통 출력 객체 사용
    void* dinDataPtr = nullptr;

    if (particleType == ParticleType::D0) {
        simpleDMCTreeevt* dinMC = new simpleDMCTreeevt();
        simpleDMCTreeflat* doutMC = new simpleDMCTreeflat(); // 공통 출력 객체
        simpleDTreeevt* dinData = new simpleDTreeevt();

        dinMCPtr = dinMC;
        doutMCPtr = doutMC;
        dinDataPtr = dinData;

        dinMC->setTree<TChain>(chainMC.get());
        dinData->setTree<TChain>(chainData.get());
    } else {
        simpleDStarMCTreeevt* dinMC = new simpleDStarMCTreeevt();
        simpleDStarMCTreeflat* doutMC = new simpleDStarMCTreeflat(); // 공통 출력 객체
        simpleDStarDataTreeevt* dinData = new simpleDStarDataTreeevt();

        dinMCPtr = dinMC;
        doutMCPtr = doutMC;
        dinDataPtr = dinData;

        dinMC->setTree<TChain>(chainMC.get());
        dinData->setTree<TChain>(chainData.get());
    }

    // 로드된 이벤트 수 확인
    long long nEntriesMC = chainMC->GetEntries();
    long long nEntriesData = chainData->GetEntries();
    std::cout << "MC entries: " << nEntriesMC << ", Data entries: " << nEntriesData << std::endl;

    if (doCent) {
        if (chainEventInfoMC) {
            long long nEntriesEventInfoMC = chainEventInfoMC->GetEntries();
            std::cout << "MC Event Info entries: " << nEntriesEventInfoMC << std::endl;
            if (nEntriesMC != nEntriesEventInfoMC) {
                std::cerr << "Warning: Mismatch between MC main tree entries (" << nEntriesMC
                          << ") and MC event info entries (" << nEntriesEventInfoMC << "). Centrality might be incorrect for MC." << std::endl;
            }
        }
        if (chainEventInfoData) {
            long long nEntriesEventInfoData = chainEventInfoData->GetEntries();
            std::cout << "Data Event Info entries: " << nEntriesEventInfoData << std::endl;
            if (nEntriesData != nEntriesEventInfoData) {
                std::cerr << "Warning: Mismatch between Data main tree entries (" << nEntriesData
                          << ") and Data event info entries (" << nEntriesEventInfoData << "). Centrality might be incorrect for Data." << std::endl;
            }
        }
    }


    // 출력 파일 및 트리 설정
    TFile* fout = createOutputFile(outputPath, outputPrefix, jobIdx, date);
    TTree* tskim = new TTree("skimTreeFlat", "Flattened Mixed Skim Tree");

    // 출력 트리 설정 (공통 객체 사용)
    if (particleType == ParticleType::D0) {
        ((simpleDMCTreeflat*)doutMCPtr)->setOutputTree(tskim);
    } else {
        ((simpleDStarMCTreeflat*)doutMCPtr)->setOutputTree(tskim);
    }

    // 출력 트리에 centrality 및 Ncoll 브랜치 추가 (doCent가 true일 경우)
    Short_t output_centrality = -99; // 출력용 centrality 변수
    Float_t output_ncoll = -99;      // 출력용 Ncoll 변수
    if (doCent) {
        tskim->Branch("centrality", &output_centrality, "centrality/S");
        tskim->Branch("Ncoll", &output_ncoll, "Ncoll/F"); // Ncoll은 MC일 때만 의미있는 값, Data는 -99
        std::cout << "Added 'centrality' and 'Ncoll' branches to output tree." << std::endl;
    }

    // 처리할 이벤트 수 결정 (MC와 Data 중 작은 쪽 기준)
    long long totEvt = std::min(nEntriesMC, nEntriesData);
    std::cout << "Total events available for mixing: " << totEvt << std::endl;

    // 종료 이벤트 조정
    end = (end == -1) ? totEvt : std::min((long long)end, totEvt);
    std::cout << "Processing events from " << start << " to " << end << std::endl;

    int skip = 1;
    int idxMC = 0;
    int idxData = 0;

    // 메인 처리 루프
    for (auto iEvt : ROOT::TSeqU(start, end, skip)) {
        if ((iEvt % 10000) == 0) {
            std::cout << "Processing event " << iEvt << " / " << end << std::endl;
        }

        // Event Info 트리에서 현재 이벤트의 centrality 가져오기 (doCent가 true일 경우)
        centralityMC = -99; // 루프마다 초기화
        centralityData = -99;
        ncoll = -99;
        if (doCent) {
            if (chainEventInfoMC && iEvt < chainEventInfoMC->GetEntries()) {
                chainEventInfoMC->GetEntry(iEvt);
                ncoll = findNcoll(centralityMC); // MC의 Ncoll 계산
            }
            if (chainEventInfoData && iEvt < chainEventInfoData->GetEntries()) {
                chainEventInfoData->GetEntry(iEvt);
            }
        }

        // MC 처리
        if (iEvt < nEntriesMC) {
            chainMC->GetEntry(iEvt);
            output_centrality = centralityMC; // MC centrality 설정
            output_ncoll = ncoll;             // MC Ncoll 설정

            if (particleType == ParticleType::D0) {
                simpleDMCTreeevt* dinMC = (simpleDMCTreeevt*)dinMCPtr;
                simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr;

                for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                    if (dinMC->matchGEN[iD1] == true) {
                        doutMC->isMC = true;
                        doutMC->copyDn(*dinMC, iD1);
                        tskim->Fill();
                    } else {
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
                        doutMC->isMC = true;
                        doutMC->copyDn(*dinMC, iD1);
                        tskim->Fill();
                    } else {
                        if (idxMC % mcSampleRate == 0) {
                            doutMC->isMC = true;
                            doutMC->copyDn(*dinMC, iD1);
                            tskim->Fill();
                        }
                        idxMC++;
                    }
                }
            }
        }

        // Data 처리
        if (iEvt < nEntriesData) {
            chainData->GetEntry(iEvt);
            output_centrality = centralityData; // Data centrality 설정
            output_ncoll = -99;                 // Data는 Ncoll 없음

            if (particleType == ParticleType::D0) {
                simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr; // 공통 출력 객체 사용
                simpleDTreeevt* dinData = (simpleDTreeevt*)dinDataPtr;

                for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
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
                simpleDStarMCTreeflat* doutMC = (simpleDStarMCTreeflat*)doutMCPtr; // 공통 출력 객체 사용
                simpleDStarDataTreeevt* dinData = (simpleDStarDataTreeevt*)dinDataPtr;

                for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
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
    }

    // 파일 저장 및 정리
    if (fout) {
        std::cout << "Writing output file with " << tskim->GetEntries() << " entries..." << std::endl;
        fout->cd();
        tskim->Write();
        fout->Close();
        std::cout << "Output file " << fout->GetName() << " has been created." << std::endl;
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
    const std::string& eventPlaneInfoTreeName,
    const std::string& outputPath,        // 출력 파일 경로
    const std::string& outputPrefix,      // 출력 파일 접두사
    int start,                            // 시작 이벤트
    int end,                              // 종료 이벤트 (-1은 모든 이벤트)
    int jobIdx,                           // 작업 인덱스
    ParticleType particleType = ParticleType::D0, // 입자 타입 (D0 또는 DStar)
    bool doCent = true,                 // 중앙값 설정 여부 (추가)
    bool doEvtPlane = true,            // 이벤트 평면 설정 여부 (추가)
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
    std::unique_ptr<TChain> chainEventPlane = nullptr;
    Double_t trkQx = -99; // 기본값 초기화
    Double_t trkQy = -99; // 기본값 초기화
    Double_t Psi2Raw_Trk = -99; // 기본값 초기화
    if (doEvtPlane) {
        chainEventPlane.reset(new TChain(eventPlaneInfoTreeName.c_str()));
        loadRootFilesRecursively(chainEventPlane.get(), dataPath); // 데이터 경로에서 Event Info 로드
        if (chainEventPlane->GetEntries() > 0) {
            chainEventPlane->SetBranchAddress("trkQx", &trkQx);
            chainEventPlane->SetBranchAddress("trkQy", &trkQy);
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
    if (doEvtPlane && chainEventPlane) {
        long long nEntriesEventPlane = chainEventPlane->GetEntries();
        std::cout << "Event Plane entries: " << nEntriesEventPlane << std::endl;
        if (nEntriesData != nEntriesEventPlane) {
            std::cerr << "Warning: Mismatch between main data tree entries (" << nEntriesData
                      << ") and event plane entries (" << nEntriesEventPlane << "). Event plane might be incorrect." << std::endl;
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
    if (doEvtPlane){
        tskim->Branch("Psi2Raw_Trk", &Psi2Raw_Trk, "Psi2Raw_Trk/D");
        std::cout << "Added 'Psi2Raw_Trk' branch to output tree." << std::endl;
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
        if (doEvtPlane && chainEventPlane) {
            chainEventPlane->GetEntry(iEvt);
            Psi2Raw_Trk = atan2(trkQy, trkQx);
        } else {
            Psi2Raw_Trk = -1.0f; // Psi2Raw_Trk를 사용하지 않으면 기본값 유지
        }

        // Data 메인 트리 처리
        chainData->GetEntry(iEvt);

        if (particleType == ParticleType::D0) {
            simpleDMCTreeflat* doutData = (simpleDMCTreeflat*)doutDataPtr;
            simpleDTreeevt* dinData = (simpleDTreeevt*)dinDataPtr;

            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                if(dinData->pT[iD1] > 100 || abs(dinData->y[iD1]) >1 || dinData->pTD1[iD1] <1 || dinData->pTD2[iD1] <1 || dinData->EtaD1[iD1] >2.4 || dinData->EtaD2[iD1] >2.4) continue;
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
                if(dinData->pT[iD1] > 100 || abs(dinData->y[iD1]) >1 || dinData->pT[iD1] < 4  || dinData->EtaD1[iD1] >2.4 || dinData->EtaD2[iD1] >2.4 || dinData->pTGrandD1[iD1]< 1 || dinData->pTGrandD2[iD1] <1 || dinData->EtaGrandD1[iD1] > 2.4 || dinData->EtaGrandD2[iD1] >2.4) continue;
                    doutData->isMC = false;
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
    Float_t ncoll = -99;
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
        tskim->Branch("Ncoll", &ncoll, "Ncoll/F");
	    tskimGEN->Branch("centrality",&centrality,"centrality/S");
        tskimGEN->Branch("Ncoll", &ncoll, "Ncoll/F");

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
            ncoll = findNcoll(centrality);
            // cout << ncoll << endl;
        } else {
            centrality = -1.0f;
        }

        chainMC->GetEntry(iEvt);

        if (particleType == ParticleType::D0) {
            simpleDMCTreeevt* dinMC = (simpleDMCTreeevt*)dinMCPtr;
            simpleDMCTreeflat *doutMC = (simpleDMCTreeflat *)doutMCPtr;

            for (auto iD1 : ROOT::TSeqI(dinMC->candSize))
            {
                if(dinMC->pT[iD1] > 100 || abs(dinMC->y[iD1]) >1 || dinMC->pTD1[iD1] <1 || dinMC->pTD2[iD1] <1 || dinMC->EtaD1[iD1] >2.4 || dinMC->EtaD2[iD1] >2.4) continue;
                doutMC->isMC = true;
                doutMC->copyDn(*dinMC, iD1);
                tskim->Fill();
            }
            if (setGEN && tskimGEN)
            {
                for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen))
                {
                if(dinMC->gen_pT[iD1] > 100 || abs(dinMC->gen_y[iD1]) >1 || dinMC->gen_D0Dau1_pT[iD1] <1 || dinMC->gen_D0Dau2_pT[iD1] <1 || dinMC->gen_D0Dau1_eta[iD1] >2.4 || dinMC->gen_D0Dau2_eta[iD1] >2.4) continue;
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
                if(dinMC->pT[iD1] > 100 || abs(dinMC->y[iD1]) >1 || dinMC->pT[iD1] < 4  || dinMC->EtaD1[iD1] >2.4 || dinMC->EtaD2[iD1] >2.4 || dinMC->pTGrandD1[iD1]< 1 || dinMC->pTGrandD2[iD1] <1 || dinMC->EtaGrandD1[iD1] > 2.4 || dinMC->EtaGrandD2[iD1] >2.4) continue;
                doutMC->isMC = true;
                doutMC->copyDn(*dinMC, iD1);
                tskim->Fill();
            }
            if (setGEN && tskimGEN)
            {
                for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen))
                {
                // if(dinMC->gen_pT[iD1] > 100 || abs(dinMC->gen_y[iD1]) >1 || dinMC->gen_pT[iD1] < 4  || dinMC->EtaD1[iD1] >2.4 || dinMC->EtaD2[iD1] >2.4 || dinMC->pTGrandD1[iD1]< 1 || dinMC->pTGrandD2[iD1] <1 || dinMC->EtaGrandD1[iD1] > 2.4 || dinMC->EtaGrandD2[iD1] >2.4) continue;
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


int FlexibleFlattener(int start=0, int end=-1, int idx=0, int type=0, std::string path = "", std::string suffix="") {

    int start_ = start;
    int end_ = end;
    int jobIdx_ = idx;
    bool setGEN = true;
    ParticleType particleType = ParticleType::DStar;
    bool doCent = true;
    bool doEvtPlane = particleType == ParticleType::DStar ? true : false;

    std::string eventInfoTreeName = "eventinfoana/EventInfoNtuple";
    std::string eventPlaneInfoTreeName = "eventplane/EventPlane"; // Event Plane 트리 이름 (추가)

    //std::string mcPath = "/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MC_Step2MVA_D0Kpi_DpT_NonSwap_CMSSW_13_2_13_NoMVACut_04Apr2025_v1/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MC_Step2MVA_D0Kpi_DpT_NonSwap_CMSSW_13_2_13_NoMVACut_04Apr2025_v1/250404_044553/0000/*.root";
    //std::string dataPath = "/u/user/jun502s/SE_UserHome/DStarMC/junseok/20250401_v1/DStarAnalysis_PPRef2024_DstarToKpipi_CMSSW_14_1_7_PPRef10_141X_dataRun3_Express_v3_20250401_v1/PPRefZeroBiasPlusForward10/DStarAnalysis_PPRef2024_DstarToKpipi_CMSSW_14_1_7_PPRef10_141X_dataRun3_Express_v3_20250401_v1/250401_130954/0000/*.root";
    std::string mcPath = "/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131426/0000";
    std::string dataPath = "/u/user/jun502s/SE_UserHome/Run3_2023/Data/SkimMVA/D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/HIPhysicsRawPrime2/crab_D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/250424_165928/";


    std::string date = "25Apr25";
    int dataSampleRate =100;
    int mcSampleRate = 100; 

    std::string outputPath;
    std::string outputPrefix;
    std::string treeNameMC = particleType==ParticleType::D0 ? "d0ana_mc/PATCompositeNtuple": "dStarana_mc/PATCompositeNtuple";
    std::string treeNameData = particleType==ParticleType::D0 ? "d0ana_newreduced/PATCompositeNtuple": "dStarana/PATCompositeNtuple";

    if (type == 1) {
	    outputPath = particleType==ParticleType::D0 ? "./Data/FlatSample/ppMC/D0" : "./Data/FlatSample/ppMC/DStar";
	    outputPrefix = particleType==ParticleType::D0 ? "flatSkimForBDT_D0" : "flatSkimForBDT_DStar";
 	    if(!path.empty()) mcPath = path;
	    if(!suffix.empty()) outputPrefix += "_"+ suffix;
		    FlexibleMC(mcPath, treeNameMC, eventInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_,particleType, setGEN, doCent, date);

    } else if (type == 0) {
	    outputPath = particleType==ParticleType::D0 ? "./Data/FlatSample/ppData/D0" : "./Data/FlatSample/ppData/DStar";
	    outputPrefix = particleType==ParticleType::D0 ? "flatSkimForBDT_D0" : "flatSkimForBDT_DStar";

	    if(!path.empty()) dataPath = path;
	    if(!suffix.empty()) outputPrefix += "_"+suffix;

	    FlexibleData(dataPath, treeNameData, eventInfoTreeName, eventPlaneInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_, particleType, doCent, doEvtPlane, date);

    } else if (type == 2) {
        outputPath = "./Data/FlatSample/ppMix/";
        outputPrefix = "flatSkimForBDT_DStar_ppRef_NonSwapMix";
	if(!suffix.empty()) outputPrefix += "_"+suffix;
        // FlexibleMix 호출 업데이트 필요 - mcPath, dataPath, treeName, eventInfoTreeName, doCent 등 설정
        FlexibleMix(mcPath, dataPath, treeNameMC,treeNameData, eventInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_, particleType, mcSampleRate, dataSampleRate, doCent, date);
        std::cout << "FlexibleMix (type 2) is currently commented out. Update parameters and uncomment to run." << std::endl;

    } else {
	    std::cerr << "Invalid type value: " << type << ". Must be 0 (MC), 1 (Data), or 2 (Mix)." << std::endl;
	    return 1;
    }

    return 0;
}
