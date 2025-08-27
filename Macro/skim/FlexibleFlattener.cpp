#include <fstream>
#include <memory>
#include <stdexcept>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"

// 상수 정의
namespace FlattenerConstants {
    constexpr int NBINS = 200;
    constexpr int DEFAULT_SAMPLE_RATE = 100;
    constexpr int PROGRESS_REPORT_INTERVAL = 10000;
    constexpr int DATA_PROGRESS_REPORT_INTERVAL = 100000;
    constexpr Short_t DEFAULT_CENTRALITY = -99;
    constexpr Float_t DEFAULT_NCOLL = -99.0f;
    constexpr Double_t DEFAULT_PSI2RAW = -99.0;
}

float findNcoll(int hiBin) {
    const float Ncoll[FlattenerConstants::NBINS] = {1893.13, 1867.0, 1834.16, 1805.64, 1770.84, 1744.49, 1699.76, 1661.52, 1615.89, 1579.59, 1540.62, 1499.14, 1469.01, 1432.18, 1402.8, 1368.39, 1338.12, 1302.26, 1274.91, 1245.56, 1215.28, 1183.76, 1160.61, 1131.12, 1107.67, 1078.54, 1055.72, 1026.72, 1000.57, 980.728, 958.777, 936.515, 911.397, 889.182, 869.677, 853.33, 826.999, 808.145, 792.14, 769.639, 753.513, 732.883, 716.817, 697.168, 679.091, 668.056, 650.114, 631.024, 616.203, 597.835, 583.435, 571.454, 555.478, 543.589, 526.328, 511.657, 497.023, 489.255, 471.52, 461.133, 447.767, 436.993, 426.106, 412.626, 403.224, 389.71, 382.595, 371.48, 358.899, 349.179, 339.387, 330.523, 320.094, 313.254, 302.339, 292.421, 282.594, 274.834, 268.847, 259.463, 252.027, 244.561, 236.738, 229.574, 222.898, 215.138, 207.328, 200.879, 196.592, 190.921, 183.942, 176.685, 170.919, 166.96, 161.057, 154.421, 148.816, 144.84, 139.087, 134.448, 128.72, 124.905, 121.166, 116.648, 112.367, 109.012, 104.33, 100.736, 97.3484, 93.2283, 89.3299, 85.9068, 83.6446, 80.2019, 77.5299, 73.9647, 70.7606, 68.2284, 65.793, 63.4532, 60.4738, 58.2406, 55.063, 53.7287, 51.4638, 49.241, 47.0111, 45.5443, 43.1729, 41.5041, 39.5449, 37.9282, 36.8918, 34.9287, 33.1886, 31.9177, 30.756, 29.0803, 27.6721, 26.42, 25.2678, 24.2585, 23.1429, 22.0138, 21.0169, 19.8203, 19.1043, 18.1478, 17.1715, 16.3605, 15.4763, 14.7973, 14.1594, 13.3927, 12.795, 12.1059, 11.5921, 10.9751, 10.3213, 9.94434, 9.3518, 8.94274, 8.37618, 7.94437, 7.48868, 7.06923, 6.71137, 6.31856, 6.03184, 5.67048, 5.43369, 5.13727, 4.83292, 4.58846, 4.37208, 4.15225, 3.84385, 3.63752, 3.45214, 3.24892, 3.02845, 2.81715, 2.66395, 2.5053, 2.29512, 2.13703, 1.93591, 1.79771, 1.64165, 1.54375, 1.45878, 1.36718, 1.2942, 1.23934, 1.18423, 1.14467, 1.11826, 1.0863, 1.06149, 1.04497 };
    if (hiBin < 0 || hiBin >= FlattenerConstants::NBINS) {
        return FlattenerConstants::DEFAULT_NCOLL;
    }
    return Ncoll[hiBin];
}

void FillChain(TChain* chain, const std::vector<std::string>& files) {
    for (const auto& file : files) {
        chain->Add(file.c_str());
    }
}

void loadRootFilesRecursively(TChain* chain, const std::string& folderPath) {
    if(TString(folderPath).EndsWith(".root")) {
        std::cout << "Adding file directly: " << folderPath << std::endl;
        chain->Add(folderPath.c_str());
        return;
    }

    TSystemDirectory dir(folderPath.c_str(), folderPath.c_str());
    TList* files = dir.GetListOfFiles();

    if (!files) return;

    TIter next(files);
    TSystemFile* file;

    while ((file = (TSystemFile*)next())) {
        std::string fileName = file->GetName();

        if (fileName == "." || fileName == "..") continue;

        std::string fullPath = folderPath + "/" + fileName;

        if (file->IsDirectory()) {
            loadRootFilesRecursively(chain, fullPath);
        } else if (fileName.find(".root") != std::string::npos) {
            std::cout << "Adding: " << fullPath << std::endl;
            chain->Add(fullPath.c_str());
        }
    }
}

enum class ParticleType {
    D0,
    DStar
};

std::unique_ptr<TFile> createOutputFile(const std::string& outputPath, const std::string& prefix, int jobIdx, const std::string& date) {
    gSystem->mkdir(outputPath.c_str(), true);
    
    std::string path = outputPath;
    if (!path.empty() && path.back() != '/') path += '/';
    
    std::string fileName = Form("%s%s_%d_%s.root", path.c_str(), prefix.c_str(), jobIdx, date.c_str());
    auto fout = std::make_unique<TFile>(fileName.c_str(), "RECREATE");
    if (!fout || fout->IsZombie()) {
        throw std::runtime_error("Failed to create output file: " + fileName);
    }
    std::cout << "Output file " << fileName << " has been created." << std::endl;
    return fout;
}

// 템플릿 기반 프로세서 클래스
template<typename InputType, typename OutputType>
class ParticleProcessor {
private:
    std::unique_ptr<InputType> inputData;
    std::unique_ptr<OutputType> outputData;

public:
    ParticleProcessor() 
        : inputData(std::make_unique<InputType>())
        , outputData(std::make_unique<OutputType>()) {}

    InputType* getInput() { return inputData.get(); }
    OutputType* getOutput() { return outputData.get(); }

    void setTree(TChain* chain) {
        inputData->setTree<TChain>(chain);
    }

    void setGENTree(TChain* chain) {
        if constexpr (std::is_same_v<InputType, DataFormat::simpleDMCTreeevt> || 
                      std::is_same_v<InputType, DataFormat::simpleDStarMCTreeevt>) {
            inputData->setGENTree<TChain>(chain);
        }
    }

    void setOutputTree(TTree* tree) {
        outputData->setOutputTree(tree);
    }

    void setGENOutputTree(TTree* tree) {
        if constexpr (std::is_same_v<OutputType, DataFormat::simpleDMCTreeflat> || 
                      std::is_same_v<OutputType, DataFormat::simpleDStarMCTreeflat>) {
            outputData->setGENOutputTree(tree);
        }
    }
};

// 안전한 파일 목록 로더
std::vector<std::string> loadFileList(const std::string& filePath) {
    std::vector<std::string> files;
    std::ifstream inputFile(filePath);
    
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening the input file list: " + filePath);
    }

    std::string line;
    std::cout << "File Content: " << std::endl;
    while (getline(inputFile, line)) {
        std::cout << line << std::endl;
        files.push_back(line);
    }
    inputFile.close();
    
    if (files.empty()) {
        throw std::runtime_error("No files found in the file list: " + filePath);
    }
    
    return files;
}

void FlexibleMix(
    const std::string& mcPath,
    const std::string& dataPath,
    const std::string& treeNameMC,
    const std::string& treeNameData,
    const std::string& eventInfoTreeName,
    const std::string& outputPath,
    const std::string& outputPrefix,
    int start,
    int end,
    int jobIdx,
    ParticleType particleType = ParticleType::D0,
    int mcSampleRate = FlattenerConstants::DEFAULT_SAMPLE_RATE,
    int dataSampleRate = FlattenerConstants::DEFAULT_SAMPLE_RATE,
    bool doCent = true,
    const std::string& date = "20250320"
) {
    std::cout << "Starting FlexibleMix job #" << jobIdx << std::endl;
    std::cout << "Processing MC files: " << mcPath << std::endl;
    std::cout << "Processing Data files: " << dataPath << std::endl;
    std::cout << "Particle type: " << (particleType == ParticleType::D0 ? "D0" : "DStar") << std::endl;
    std::cout << "Setting doCent: " << (doCent ? "true" : "false") << std::endl;

    auto chainMC = std::make_unique<TChain>(treeNameMC.c_str());
    auto chainData = std::make_unique<TChain>(treeNameData.c_str());
    loadRootFilesRecursively(chainMC.get(), mcPath);
    loadRootFilesRecursively(chainData.get(), dataPath);

    std::unique_ptr<TChain> chainEventInfoMC = nullptr;
    std::unique_ptr<TChain> chainEventInfoData = nullptr;
    Short_t centralityMC = FlattenerConstants::DEFAULT_CENTRALITY;
    Short_t centralityData = FlattenerConstants::DEFAULT_CENTRALITY;
    Float_t ncoll = FlattenerConstants::DEFAULT_NCOLL;

    if (doCent) {
        chainEventInfoMC = std::make_unique<TChain>(eventInfoTreeName.c_str());
        loadRootFilesRecursively(chainEventInfoMC.get(), mcPath);
        if (chainEventInfoMC->GetEntries() > 0) {
            chainEventInfoMC->SetBranchAddress("centrality", &centralityMC);
            std::cout << "MC Centrality branch linked from " << eventInfoTreeName << std::endl;
        } else {
            std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty in MC files." << std::endl;
        }

        chainEventInfoData = std::make_unique<TChain>(eventInfoTreeName.c_str());
        loadRootFilesRecursively(chainEventInfoData.get(), dataPath);
        if (chainEventInfoData->GetEntries() > 0) {
            chainEventInfoData->SetBranchAddress("centrality", &centralityData);
            std::cout << "Data Centrality branch linked from " << eventInfoTreeName << std::endl;
        } else {
            std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty in Data files." << std::endl;
        }
    }

    using namespace DataFormat;

    std::unique_ptr<ParticleProcessor<simpleDMCTreeevt, simpleDMCTreeflat>> d0Processor;
    std::unique_ptr<ParticleProcessor<simpleDStarMCTreeevt, simpleDStarMCTreeflat>> dstarProcessor;
    std::unique_ptr<ParticleProcessor<simpleDTreeevt, simpleDMCTreeflat>> d0DataProcessor;
    std::unique_ptr<ParticleProcessor<simpleDStarDataTreeevt, simpleDStarMCTreeflat>> dstarDataProcessor;

    if (particleType == ParticleType::D0) {
        d0Processor = std::make_unique<ParticleProcessor<simpleDMCTreeevt, simpleDMCTreeflat>>();
        d0DataProcessor = std::make_unique<ParticleProcessor<simpleDTreeevt, simpleDMCTreeflat>>();
        d0Processor->setTree(chainMC.get());
        d0DataProcessor->setTree(chainData.get());
    } else {
        dstarProcessor = std::make_unique<ParticleProcessor<simpleDStarMCTreeevt, simpleDStarMCTreeflat>>();
        dstarDataProcessor = std::make_unique<ParticleProcessor<simpleDStarDataTreeevt, simpleDStarMCTreeflat>>();
        dstarProcessor->setTree(chainMC.get());
        dstarDataProcessor->setTree(chainData.get());
    }

    long long nEntriesMC = chainMC->GetEntries();
    long long nEntriesData = chainData->GetEntries();
    std::cout << "MC entries: " << nEntriesMC << ", Data entries: " << nEntriesData << std::endl;

    if (doCent) {
        if (chainEventInfoMC) {
            long long nEntriesEventInfoMC = chainEventInfoMC->GetEntries();
            std::cout << "MC Event Info entries: " << nEntriesEventInfoMC << std::endl;
            if (nEntriesMC != nEntriesEventInfoMC) {
                std::cerr << "Warning: Mismatch between MC main tree entries (" << nEntriesMC
                          << ") and MC event info entries (" << nEntriesEventInfoMC << ")." << std::endl;
            }
        }
        if (chainEventInfoData) {
            long long nEntriesEventInfoData = chainEventInfoData->GetEntries();
            std::cout << "Data Event Info entries: " << nEntriesEventInfoData << std::endl;
            if (nEntriesData != nEntriesEventInfoData) {
                std::cerr << "Warning: Mismatch between Data main tree entries (" << nEntriesData
                          << ") and Data event info entries (" << nEntriesEventInfoData << ")." << std::endl;
            }
        }
    }

    auto fout = createOutputFile(outputPath, outputPrefix, jobIdx, date);
    auto tskim = std::make_unique<TTree>("skimTreeFlat", "Flattened Mixed Skim Tree");

    if (particleType == ParticleType::D0) {
        d0Processor->setOutputTree(tskim.get());
    } else {
        dstarProcessor->setOutputTree(tskim.get());
    }

    Short_t output_centrality = FlattenerConstants::DEFAULT_CENTRALITY;
    Float_t output_ncoll = FlattenerConstants::DEFAULT_NCOLL;
    if (doCent) {
        tskim->Branch("centrality", &output_centrality, "centrality/S");
        tskim->Branch("Ncoll", &output_ncoll, "Ncoll/F");
        std::cout << "Added 'centrality' and 'Ncoll' branches to output tree." << std::endl;
    }

    long long totEvt = std::min(nEntriesMC, nEntriesData);
    std::cout << "Total events available for mixing: " << totEvt << std::endl;

    end = (end == -1) ? totEvt : std::min((long long)end, totEvt);
    std::cout << "Processing events from " << start << " to " << end << std::endl;

    int skip = 1;
    int idxMC = 0;
    int idxData = 0;

    for (auto iEvt : ROOT::TSeqU(start, end, skip)) {
        if ((iEvt % FlattenerConstants::PROGRESS_REPORT_INTERVAL) == 0) {
            std::cout << "Processing event " << iEvt << " / " << end << std::endl;
        }

        centralityMC = FlattenerConstants::DEFAULT_CENTRALITY;
        centralityData = FlattenerConstants::DEFAULT_CENTRALITY;
        ncoll = FlattenerConstants::DEFAULT_NCOLL;
        
        if (doCent) {
            if (chainEventInfoMC && iEvt < chainEventInfoMC->GetEntries()) {
                chainEventInfoMC->GetEntry(iEvt);
                ncoll = findNcoll(centralityMC);
            }
            if (chainEventInfoData && iEvt < chainEventInfoData->GetEntries()) {
                chainEventInfoData->GetEntry(iEvt);
            }
        }

        // MC 처리
        if (iEvt < nEntriesMC) {
            chainMC->GetEntry(iEvt);
            output_centrality = centralityMC;
            output_ncoll = ncoll;

            if (particleType == ParticleType::D0) {
                auto* dinMC = d0Processor->getInput();
                auto* doutMC = d0Processor->getOutput();

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
                auto* dinMC = dstarProcessor->getInput();
                auto* doutMC = dstarProcessor->getOutput();

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
            output_centrality = centralityData;
            output_ncoll = FlattenerConstants::DEFAULT_NCOLL;

            if (particleType == ParticleType::D0) {
                auto* doutMC = d0Processor->getOutput();
                auto* dinData = d0DataProcessor->getInput();

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
                auto* doutMC = dstarProcessor->getOutput();
                auto* dinData = dstarDataProcessor->getInput();

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

    if (fout) {
        std::cout << "Writing output file with " << tskim->GetEntries() << " entries..." << std::endl;
        fout->cd();
        tskim->Write();
        fout->Close();
        std::cout << "Output file " << fout->GetName() << " has been created." << std::endl;
    }

    std::cout << "FlexibleMix job #" << jobIdx << " completed successfully." << std::endl;
}

void FlexibleData(
    const std::string& dataPath,
    const std::string& treeName,
    const std::string& eventInfoTreeName,
    const std::string& eventPlaneInfoTreeName,
    const std::string& outputPath,
    const std::string& outputPrefix,
    int start,
    int end,
    int jobIdx,
    ParticleType particleType = ParticleType::D0,
    bool doCent = true,
    bool doEvtPlane = true,
    const std::string& date = "20250320"
) {
    std::cout << "Starting FlexibleData job #" << jobIdx << std::endl;
    std::cout << "Processing Data files: " << dataPath << std::endl;
    std::cout << "Particle type: " << (particleType == ParticleType::D0 ? "D0" : "DStar") << std::endl;
    std::cout << "Setting doCent: " << (doCent ? "true" : "false") << std::endl;

    auto chainData = std::make_unique<TChain>(treeName.c_str());
    
    try {
        auto files = loadFileList(dataPath);
        FillChain(chainData.get(), files);

        std::unique_ptr<TChain> chainEventInfo = nullptr;
        Short_t centrality = FlattenerConstants::DEFAULT_CENTRALITY;
        
        if (doCent) {
            chainEventInfo = std::make_unique<TChain>(eventInfoTreeName.c_str());
            FillChain(chainEventInfo.get(), files);
            if (chainEventInfo->GetEntries() > 0) {
                chainEventInfo->SetBranchAddress("centrality", &centrality);
                std::cout << "Centrality branch linked from " << eventInfoTreeName << std::endl;
            } else {
                std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty." << std::endl;
                doCent = false;
            }
        }

        std::unique_ptr<TChain> chainEventPlane = nullptr;
        Double_t trkQx = FlattenerConstants::DEFAULT_PSI2RAW;
        Double_t trkQy = FlattenerConstants::DEFAULT_PSI2RAW;
        Double_t Psi2Raw_Trk = FlattenerConstants::DEFAULT_PSI2RAW;
        
        if (doEvtPlane) {
            chainEventPlane = std::make_unique<TChain>(eventPlaneInfoTreeName.c_str());
            FillChain(chainEventPlane.get(), files);
            if (chainEventPlane->GetEntries() > 0) {
                chainEventPlane->SetBranchAddress("trkQx", &trkQx);
                chainEventPlane->SetBranchAddress("trkQy", &trkQy);
                std::cout << "Event plane branches linked from " << eventPlaneInfoTreeName << std::endl;
            } else {
                std::cerr << "Warning: Event plane tree '" << eventPlaneInfoTreeName << "' not found or empty." << std::endl;
                doEvtPlane = false;
            }
        }

        using namespace DataFormat;

        std::unique_ptr<ParticleProcessor<simpleDTreeevt, simpleDMCTreeflat>> d0Processor;
        std::unique_ptr<ParticleProcessor<simpleDStarDataTreeevt, simpleDStarMCTreeflat>> dstarProcessor;

        if (particleType == ParticleType::D0) {
            d0Processor = std::make_unique<ParticleProcessor<simpleDTreeevt, simpleDMCTreeflat>>();
            d0Processor->setTree(chainData.get());
        } else {
            dstarProcessor = std::make_unique<ParticleProcessor<simpleDStarDataTreeevt, simpleDStarMCTreeflat>>();
            dstarProcessor->setTree(chainData.get());
        }

        long long nEntriesData = chainData->GetEntries();
        std::cout << "Data entries: " << nEntriesData << std::endl;
        
        if (doCent && chainEventInfo) {
            long long nEntriesEventInfo = chainEventInfo->GetEntries();
            std::cout << "Event Info entries: " << nEntriesEventInfo << std::endl;
            if (nEntriesData != nEntriesEventInfo) {
                std::cerr << "Warning: Mismatch between main data tree entries (" << nEntriesData
                          << ") and event info entries (" << nEntriesEventInfo << ")." << std::endl;
            }
        }
        
        if (doEvtPlane && chainEventPlane) {
            long long nEntriesEventPlane = chainEventPlane->GetEntries();
            std::cout << "Event Plane entries: " << nEntriesEventPlane << std::endl;
            if (nEntriesData != nEntriesEventPlane) {
                std::cerr << "Warning: Mismatch between main data tree entries (" << nEntriesData
                          << ") and event plane entries (" << nEntriesEventPlane << ")." << std::endl;
            }
        }

        auto fout = createOutputFile(outputPath, outputPrefix, jobIdx, date);
        auto tskim = std::make_unique<TTree>("skimTreeFlat", "Flattened Data Skim Tree");

        if (particleType == ParticleType::D0) {
            d0Processor->setOutputTree(tskim.get());
        } else {
            dstarProcessor->setOutputTree(tskim.get());
        }

        if (doCent) {
            tskim->Branch("centrality", &centrality, "centrality/S");
            std::cout << "Added 'centrality' branch to output tree." << std::endl;
        }
        
        if (doEvtPlane) {
            tskim->Branch("Psi2Raw_Trk", &Psi2Raw_Trk, "Psi2Raw_Trk/D");
            std::cout << "Added 'Psi2Raw_Trk' branch to output tree." << std::endl;
        }

        int totEvt = static_cast<int>(nEntriesData);
        std::cout << "Total events available: " << totEvt << std::endl;

        end = (end == -1) ? totEvt : std::min(end, totEvt);
        std::cout << "Processing events from " << start << " to " << end << std::endl;

        int skip = 1;

        for (auto iEvt : ROOT::TSeqU(start, end, skip)) {
            if ((iEvt % FlattenerConstants::DATA_PROGRESS_REPORT_INTERVAL) == 0) {
                std::cout << "Processing event " << iEvt << " / " << end << std::endl;
            }

            if (doCent && chainEventInfo) {
                chainEventInfo->GetEntry(iEvt);
            } else {
                centrality = FlattenerConstants::DEFAULT_CENTRALITY;
            }
            
            if (doEvtPlane && chainEventPlane) {
                chainEventPlane->GetEntry(iEvt);
                Psi2Raw_Trk = atan2(trkQy, trkQx);
            } else {
                Psi2Raw_Trk = FlattenerConstants::DEFAULT_PSI2RAW;
            }

            chainData->GetEntry(iEvt);

            if (particleType == ParticleType::D0) {
                auto* doutData = d0Processor->getOutput();
                auto* dinData = d0Processor->getInput();

                for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                    doutData->isMC = false;
                    doutData->isSwap = 0;
                    doutData->matchGEN = 0;
                    doutData->copyDn<simpleDTreeevt>(*dinData, iD1);
                    tskim->Fill();
                }
            } else {
                auto* doutData = dstarProcessor->getOutput();
                auto* dinData = dstarProcessor->getInput();

                for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                    doutData->isMC = false;
                    doutData->isSwap = 0;
                    doutData->matchGEN = 0;
                    doutData->copyDn(*dinData, iD1);
                    tskim->Fill();
                }
            }
        }

        if (fout) {
            std::cout << "Writing output file with " << tskim->GetEntries() << " entries..." << std::endl;
            fout->cd();
            tskim->Write();
            fout->Close();
            std::cout << "Output file " << fout->GetName() << " has been created." << std::endl;
        }

        std::cout << "FlexibleData job #" << jobIdx << " completed successfully." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in FlexibleData: " << e.what() << std::endl;
        throw;
    }
}

void FlexibleMC(
    const std::string& mcPath,
    const std::string& treeName,
    const std::string& eventInfoTreeName,
    const std::string& outputPath,
    const std::string& outputPrefix,
    int start,
    int end,
    int jobIdx,
    ParticleType particleType = ParticleType::D0,
    bool setGEN = true,
    bool doCent = true,
    const std::string& date = "20250331"
) {
    std::cout << "Starting FlexibleMC job #" << jobIdx << std::endl;
    std::cout << "Processing MC files: " << mcPath << std::endl;
    std::cout << "Particle type: " << (particleType == ParticleType::D0 ? "D0" : "DStar") << std::endl;
    std::cout << "Setting GEN tree: " << (setGEN ? "true" : "false") << std::endl;
    std::cout << "Setting doCent: " << (doCent ? "true" : "false") << std::endl;

    try {
        auto chainMC = std::make_unique<TChain>(treeName.c_str());
        auto files = loadFileList(mcPath);
        FillChain(chainMC.get(), files);

        std::unique_ptr<TChain> chainEventInfo = nullptr;
        Short_t centrality = FlattenerConstants::DEFAULT_CENTRALITY;
        Float_t ncoll = FlattenerConstants::DEFAULT_NCOLL;
        
        if (doCent) {
            chainEventInfo = std::make_unique<TChain>(eventInfoTreeName.c_str());
            FillChain(chainEventInfo.get(), files);
            if (chainEventInfo->GetEntries() > 0) {
                chainEventInfo->SetBranchAddress("centrality", &centrality);
                std::cout << "Centrality branch linked from " << eventInfoTreeName << std::endl;
            } else {
                std::cerr << "Warning: Event info tree '" << eventInfoTreeName << "' not found or empty." << std::endl;
                doCent = false;
            }
        }

        using namespace DataFormat;

        std::unique_ptr<ParticleProcessor<simpleDMCTreeevt, simpleDMCTreeflat>> d0Processor;
        std::unique_ptr<ParticleProcessor<simpleDStarMCTreeevt, simpleDStarMCTreeflat>> dstarProcessor;

        if (particleType == ParticleType::D0) {
            d0Processor = std::make_unique<ParticleProcessor<simpleDMCTreeevt, simpleDMCTreeflat>>();
            d0Processor->setTree(chainMC.get());
            if (setGEN) d0Processor->setGENTree(chainMC.get());
        } else {
            dstarProcessor = std::make_unique<ParticleProcessor<simpleDStarMCTreeevt, simpleDStarMCTreeflat>>();
            dstarProcessor->setTree(chainMC.get());
            if (setGEN) dstarProcessor->setGENTree(chainMC.get());
        }

        long long nEntriesMC = chainMC->GetEntries();
        std::cout << "MC entries: " << nEntriesMC << std::endl;
        
        if (doCent && chainEventInfo) {
            long long nEntriesEventInfo = chainEventInfo->GetEntries();
            std::cout << "Event Info entries: " << nEntriesEventInfo << std::endl;
            if (nEntriesMC != nEntriesEventInfo) {
                std::cerr << "Warning: Mismatch between main tree entries (" << nEntriesMC
                          << ") and event info entries (" << nEntriesEventInfo << ")." << std::endl;
            }
        }

        auto fout = createOutputFile(outputPath, outputPrefix, jobIdx, date);
        auto tskim = std::make_unique<TTree>("skimTreeFlat", "Flattened MC Skim Tree");
        std::unique_ptr<TTree> tskimGEN = nullptr;
        
        if (setGEN) {
            tskimGEN = std::make_unique<TTree>("skimGENTreeFlat", "Flattened MC Skim GEN Tree");
        }

        if (particleType == ParticleType::D0) {
            d0Processor->setOutputTree(tskim.get());
            if (setGEN && tskimGEN) d0Processor->setGENOutputTree(tskimGEN.get());
        } else {
            dstarProcessor->setOutputTree(tskim.get());
            if (setGEN && tskimGEN) dstarProcessor->setGENOutputTree(tskimGEN.get());
        }

        if (doCent) {
            tskim->Branch("centrality", &centrality, "centrality/S");
            tskim->Branch("Ncoll", &ncoll, "Ncoll/F");
            if (setGEN && tskimGEN) {
                tskimGEN->Branch("centrality", &centrality, "centrality/S");
                tskimGEN->Branch("Ncoll", &ncoll, "Ncoll/F");
            }
            std::cout << "Added 'centrality' branch to output tree(s)." << std::endl;
        }

        int totEvt = static_cast<int>(nEntriesMC);
        std::cout << "Total events available: " << totEvt << std::endl;

        end = (end == -1) ? totEvt : std::min(end, totEvt);
        std::cout << "Processing events from " << start << " to " << end << std::endl;

        int skip = 1;

        for (auto iEvt : ROOT::TSeqU(start, end, skip)) {
            if ((iEvt % FlattenerConstants::PROGRESS_REPORT_INTERVAL) == 0) {
                std::cout << "Processing event " << iEvt << " / " << end << std::endl;
            }

            if (doCent && chainEventInfo) {
                chainEventInfo->GetEntry(iEvt);
                ncoll = findNcoll(centrality);
            } else {
                centrality = FlattenerConstants::DEFAULT_CENTRALITY;
            }

            chainMC->GetEntry(iEvt);

            if (particleType == ParticleType::D0) {
                auto* dinMC = d0Processor->getInput();
                auto* doutMC = d0Processor->getOutput();

                for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                    doutMC->isMC = true;
                    doutMC->copyDn(*dinMC, iD1);
                    tskim->Fill();
                }
                
                if (setGEN && tskimGEN) {
                    for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen)) {
                        doutMC->isMC = true;
                        doutMC->copyGENDn(*dinMC, iD1);
                        tskimGEN->Fill();
                    }
                }
            } else {
                auto* dinMC = dstarProcessor->getInput();
                auto* doutMC = dstarProcessor->getOutput();

                for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                    doutMC->isMC = true;
                    doutMC->copyDn(*dinMC, iD1);
                    tskim->Fill();
                }
                
                if (setGEN && tskimGEN) {
                    for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen)) {
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
        }

        std::cout << "FlexibleMC job #" << jobIdx << " completed successfully." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in FlexibleMC: " << e.what() << std::endl;
        throw;
    }
}

int FlexibleFlattener(int start=0, int end=-1, int idx=0, int type=0, std::string path = "", std::string suffix="") {
    try {
        int start_ = start;
        int end_ = end;
        int jobIdx_ = idx;
        bool setGEN = true;
        ParticleType particleType = ParticleType::DStar;
        bool doCent = false;
        bool doEvtPlane = particleType == ParticleType::DStar ? true : false;
        doEvtPlane = false;

        std::string eventInfoTreeName = "eventinfoana/EventInfoNtuple";
        std::string eventPlaneInfoTreeName = "eventplane/EventPlane";

        std::string mcPath = "/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131426/0000";
        std::string dataPath = "/u/user/jun502s/SE_UserHome/Run3_2023/Data/SkimMVA/D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/HIPhysicsRawPrime2/crab_D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/250424_165928/";

        std::string date = "";
        int dataSampleRate = FlattenerConstants::DEFAULT_SAMPLE_RATE;
        int mcSampleRate = FlattenerConstants::DEFAULT_SAMPLE_RATE;

        std::string outputPath;
        std::string outputPrefix;
        std::string treeNameMC = particleType == ParticleType::D0 ? "d0ana_mc/PATCompositeNtuple": "dStarana_mc/PATCompositeNtuple";
        std::string treeNameData = particleType == ParticleType::D0 ? "d0ana_newreduced/PATCompositeNtuple": "dStarana/PATCompositeNtuple";

        if (type == 1) {
            outputPath = particleType == ParticleType::D0 ? "./Data/FlatSample/ppMC/D0" : "./Data/FlatSample/ppMC/DStar";
            outputPrefix = particleType == ParticleType::D0 ? "flatSkimForBDT_D0" : "flatSkimForBDT_DStar";
            if (!path.empty()) mcPath = path;
            if (!suffix.empty()) outputPrefix += "_" + suffix;
            FlexibleMC(mcPath, treeNameMC, eventInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_, particleType, setGEN, doCent, date);

        } else if (type == 0) {
            outputPath = particleType == ParticleType::D0 ? "./Data/FlatSample/ppData/D0" : "./Data/FlatSample/ppData/DStar";
            outputPrefix = particleType == ParticleType::D0 ? "flatSkimForBDT_D0" : "flatSkimForBDT_DStar";

            if (!path.empty()) dataPath = path;
            if (!suffix.empty()) outputPrefix += "_" + suffix;

            FlexibleData(dataPath, treeNameData, eventInfoTreeName, eventPlaneInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_, particleType, doCent, doEvtPlane, date);

        } else if (type == 2) {
            outputPath = "./Data/FlatSample/ppMix/";
            outputPrefix = "flatSkimForBDT_DStar_ppRef_NonSwapMix";
            if (!suffix.empty()) outputPrefix += "_" + suffix;
            FlexibleMix(mcPath, dataPath, treeNameMC, treeNameData, eventInfoTreeName, outputPath, outputPrefix, start_, end_, jobIdx_, particleType, mcSampleRate, dataSampleRate, doCent, date);

        } else {
            std::cerr << "Invalid type value: " << type << ". Must be 0 (Data), 1 (MC), or 2 (Mix)." << std::endl;
            return 1;
        }

        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in FlexibleFlattener: " << e.what() << std::endl;
        return 1;
    }
}