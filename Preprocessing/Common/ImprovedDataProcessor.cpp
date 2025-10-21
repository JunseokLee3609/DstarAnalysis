#include <iostream>
#include <memory>
#include <chrono>
#include <ROOT/TThreadExecutor.hxx>
#include "../interface/simpleDMC.h"
#include "../Tools/DataProcessorBase.h"
#include "../Tools/FileManager.h"
#include "../Tools/ConfigManager.h"
#include "../Tools/CentralityUtils.h"
#include "../Tools/VariableHandler.h"

using namespace std;
using namespace DataFormat;

class ImprovedFlexibleFlattener : public DataProcessorBase {
private:
    unique_ptr<TChain> chainMC;
    unique_ptr<TChain> chainData;
    unique_ptr<TChain> chainEventInfoMC;
    unique_ptr<TChain> chainEventInfoData;
    unique_ptr<TChain> chainEventPlane;
    
    void* dinMCPtr = nullptr;
    void* doutMCPtr = nullptr;
    void* dinDataPtr = nullptr;
    
    TFile* outputFile = nullptr;
    TTree* outputTree = nullptr;
    TTree* outputGENTree = nullptr;
    
    // Centrality and event plane variables
    Short_t centralityMC = -99;
    Short_t centralityData = -99;
    Float_t ncoll = -99;
    Double_t trkQx = -99;
    Double_t trkQy = -99;
    Double_t Psi2Raw_Trk = -99;
    
public:
    ImprovedFlexibleFlattener(const ProcessingConfig& cfg) : DataProcessorBase(cfg) {}
    
    ~ImprovedFlexibleFlattener() {
        cleanup();
    }
    
    void process() override {
        if (!initialize()) {
            logError("Initialization failed");
            return;
        }
        
        switch (config.processingMode) {
            case ProcessingMode::MC_ONLY:
                processMCOnly();
                break;
            case ProcessingMode::DATA_ONLY:
                processDataOnly();
                break;
            case ProcessingMode::MIXED:
                processMixed();
                break;
        }
        
        finalize();
    }
    
private:
    bool initialize() override {
        if (!DataProcessorBase::initialize()) {
            return false;
        }
        
        logInfo("Setting up chains and data objects...");
        
        if (!setupChains()) {
            return false;
        }
        
        if (!setupDataObjects()) {
            return false;
        }
        
        if (!setupOutputFile()) {
            return false;
        }
        
        return true;
    }
    
    bool setupChains() {
        // Setup main chains based on processing mode
        if (config.processingMode == ProcessingMode::MC_ONLY || config.processingMode == ProcessingMode::MIXED) {
            chainMC = createChain(config.treeNameMC, config.inputPath);
            if (!chainMC) {
                logError("Failed to create MC chain");
                return false;
            }
            
            if (config.doCent) {
                chainEventInfoMC = createChain(config.eventInfoTreeName, config.inputPath);
                if (chainEventInfoMC && chainEventInfoMC->GetEntries() > 0) {
                    chainEventInfoMC->SetBranchAddress("centrality", &centralityMC);
                    logInfo("MC centrality branch linked");
                } else {
                    logWarning("MC event info tree not found or empty");
                }
            }
        }
        
        if (config.processingMode == ProcessingMode::DATA_ONLY || config.processingMode == ProcessingMode::MIXED) {
            chainData = createChain(config.treeNameData, config.inputPath);
            if (!chainData) {
                logError("Failed to create Data chain");
                return false;
            }
            
            if (config.doCent) {
                chainEventInfoData = createChain(config.eventInfoTreeName, config.inputPath);
                if (chainEventInfoData && chainEventInfoData->GetEntries() > 0) {
                    chainEventInfoData->SetBranchAddress("centrality", &centralityData);
                    logInfo("Data centrality branch linked");
                } else {
                    logWarning("Data event info tree not found or empty");
                }
            }
            
            if (config.doEvtPlane) {
                chainEventPlane = createChain(config.eventPlaneInfoTreeName, config.inputPath);
                if (chainEventPlane && chainEventPlane->GetEntries() > 0) {
                    chainEventPlane->SetBranchAddress("trkQx", &trkQx);
                    chainEventPlane->SetBranchAddress("trkQy", &trkQy);
                    logInfo("Event plane branches linked");
                } else {
                    logWarning("Event plane tree not found or empty");
                }
            }
        }
        
        return true;
    }
    
    bool setupDataObjects() {
        // Create appropriate data objects based on particle type
        if (config.particleType == ParticleType::D0) {
            if (chainMC) {
                simpleDMCTreeevt* dinMC = new simpleDMCTreeevt();
                simpleDMCTreeflat* doutMC = new simpleDMCTreeflat();
                dinMC->setTree<TChain>(chainMC.get());
                if (config.setGEN) dinMC->setGENTree<TChain>(chainMC.get());
                dinMCPtr = dinMC;
                doutMCPtr = doutMC;
            }
            
            if (chainData) {
                simpleDTreeevt* dinData = new simpleDTreeevt();
                dinData->setTree<TChain>(chainData.get());
                dinDataPtr = dinData;
                if (!doutMCPtr) {
                    doutMCPtr = new simpleDMCTreeflat(); // Shared output object
                }
            }
        } else { // DStar
            if (chainMC) {
                simpleDStarMCTreeevt* dinMC = new simpleDStarMCTreeevt();
                simpleDStarMCTreeflat* doutMC = new simpleDStarMCTreeflat();
                dinMC->setTree<TChain>(chainMC.get());
                if (config.setGEN) dinMC->setGENTree<TChain>(chainMC.get());
                dinMCPtr = dinMC;
                doutMCPtr = doutMC;
            }
            
            if (chainData) {
                simpleDStarDataTreeevt* dinData = new simpleDStarDataTreeevt();
                dinData->setTree<TChain>(chainData.get());
                dinDataPtr = dinData;
                if (!doutMCPtr) {
                    doutMCPtr = new simpleDStarMCTreeflat(); // Shared output object
                }
            }
        }
        
        return true;
    }
    
    bool setupOutputFile() {
        outputFile = createOutputFile();
        if (!outputFile) {
            logError("Failed to create output file");
            return false;
        }
        
        // Create output trees
        outputTree = new TTree("skimTreeFlat", "Improved Flattened Skim Tree");
        
        if (config.setGEN && config.processingMode == ProcessingMode::MC_ONLY) {
            outputGENTree = new TTree("skimGENTreeFlat", "Improved Flattened MC GEN Tree");
        }
        
        // Setup output tree branches
        if (config.particleType == ParticleType::D0) {
            ((simpleDMCTreeflat*)doutMCPtr)->setOutputTree(outputTree);
            if (outputGENTree) {
                ((simpleDMCTreeflat*)doutMCPtr)->setGENOutputTree(outputGENTree);
            }
        } else {
            ((simpleDStarMCTreeflat*)doutMCPtr)->setOutputTree(outputTree);
            if (outputGENTree) {
                ((simpleDStarMCTreeflat*)doutMCPtr)->setGENOutputTree(outputGENTree);
            }
        }
        
        // Add centrality and Ncoll branches if needed
        if (config.doCent) {
            Short_t output_centrality = -99;
            Float_t output_ncoll = -99;
            outputTree->Branch("centrality", &output_centrality, "centrality/S");
            outputTree->Branch("Ncoll", &output_ncoll, "Ncoll/F");
            
            if (outputGENTree) {
                outputGENTree->Branch("centrality", &output_centrality, "centrality/S");
                outputGENTree->Branch("Ncoll", &output_ncoll, "Ncoll/F");
            }
            
            logInfo("Added centrality and Ncoll branches");
        }
        
        // Add event plane branch if needed
        if (config.doEvtPlane) {
            outputTree->Branch("Psi2Raw_Trk", &Psi2Raw_Trk, "Psi2Raw_Trk/D");
            logInfo("Added event plane branch");
        }
        
        return true;
    }
    
    void processMCOnly() {
        if (!chainMC) {
            logError("MC chain not available for MC-only processing");
            return;
        }
        
        long long nEntriesMC = chainMC->GetEntries();
        long long endEvent = determineEventRange(nEntriesMC);
        
        logInfo("Processing MC events from " + to_string(config.startEvent) + " to " + to_string(endEvent));
        
        for (long long iEvt = config.startEvent; iEvt < endEvent; ++iEvt) {
            printProgress(iEvt, endEvent);
            
            // Get centrality info if needed
            if (config.doCent && chainEventInfoMC && iEvt < chainEventInfoMC->GetEntries()) {
                chainEventInfoMC->GetEntry(iEvt);
                ncoll = CentralityUtils::findNcoll(centralityMC);
            }
            
            // Process MC event
            chainMC->GetEntry(iEvt);
            processMCEvent(centralityMC, ncoll);
            
            totalProcessed++;
        }
        
        logInfo("MC processing completed: " + to_string(totalProcessed) + " events processed");
    }
    
    void processDataOnly() {
        if (!chainData) {
            logError("Data chain not available for data-only processing");
            return;
        }
        
        long long nEntriesData = chainData->GetEntries();
        long long endEvent = determineEventRange(nEntriesData);
        
        logInfo("Processing Data events from " + to_string(config.startEvent) + " to " + to_string(endEvent));
        
        for (long long iEvt = config.startEvent; iEvt < endEvent; ++iEvt) {
            printProgress(iEvt, endEvent, 100000); // Less frequent updates for data
            
            // Get centrality info if needed
            if (config.doCent && chainEventInfoData && iEvt < chainEventInfoData->GetEntries()) {
                chainEventInfoData->GetEntry(iEvt);
            }
            
            // Get event plane info if needed
            if (config.doEvtPlane && chainEventPlane && iEvt < chainEventPlane->GetEntries()) {
                chainEventPlane->GetEntry(iEvt);
                Psi2Raw_Trk = atan2(trkQy, trkQx);
            }
            
            // Process data event
            chainData->GetEntry(iEvt);
            processDataEvent(centralityData);
            
            totalProcessed++;
        }
        
        logInfo("Data processing completed: " + to_string(totalProcessed) + " events processed");
    }
    
    void processMixed() {
        if (!chainMC || !chainData) {
            logError("Both MC and Data chains required for mixed processing");
            return;
        }
        
        long long nEntriesMC = chainMC->GetEntries();
        long long nEntriesData = chainData->GetEntries();
        long long nEntriesMin = min(nEntriesMC, nEntriesData);
        long long endEvent = determineEventRange(nEntriesMin);
        
        logInfo("Processing Mixed events from " + to_string(config.startEvent) + " to " + to_string(endEvent));
        logInfo("MC entries: " + to_string(nEntriesMC) + ", Data entries: " + to_string(nEntriesData));
        
        int idxMC = 0, idxData = 0;
        
        for (long long iEvt = config.startEvent; iEvt < endEvent; ++iEvt) {
            printProgress(iEvt, endEvent);
            
            // Get centrality info
            centralityMC = -99;
            centralityData = -99;
            ncoll = -99;
            
            if (config.doCent) {
                if (chainEventInfoMC && iEvt < chainEventInfoMC->GetEntries()) {
                    chainEventInfoMC->GetEntry(iEvt);
                    ncoll = CentralityUtils::findNcoll(centralityMC);
                }
                if (chainEventInfoData && iEvt < chainEventInfoData->GetEntries()) {
                    chainEventInfoData->GetEntry(iEvt);
                }
            }
            
            // Process MC part
            if (iEvt < nEntriesMC) {
                chainMC->GetEntry(iEvt);
                processMCEventMixed(centralityMC, ncoll, idxMC);
            }
            
            // Process Data part
            if (iEvt < nEntriesData) {
                chainData->GetEntry(iEvt);
                processDataEventMixed(centralityData, idxData);
            }
            
            totalProcessed++;
        }
        
        logInfo("Mixed processing completed: " + to_string(totalProcessed) + " events processed");
    }
    
    void processMCEvent(Short_t centrality, Float_t ncoll) {
        if (config.particleType == ParticleType::D0) {
            simpleDMCTreeevt* dinMC = (simpleDMCTreeevt*)dinMCPtr;
            simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                doutMC->isMC = true;
                doutMC->copyDn(*dinMC, iD1);
                outputTree->Fill();
            }
            
            if (config.setGEN && outputGENTree) {
                for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen)) {
                    doutMC->isMC = true;
                    doutMC->copyGENDn(*dinMC, iD1);
                    outputGENTree->Fill();
                }
            }
        } else {
            simpleDStarMCTreeevt* dinMC = (simpleDStarMCTreeevt*)dinMCPtr;
            simpleDStarMCTreeflat* doutMC = (simpleDStarMCTreeflat*)doutMCPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                doutMC->isMC = true;
                doutMC->copyDn(*dinMC, iD1);
                outputTree->Fill();
            }
            
            if (config.setGEN && outputGENTree) {
                for (auto iD1 : ROOT::TSeqI(dinMC->candSize_gen)) {
                    doutMC->isMC = true;
                    doutMC->copyGENDn(*dinMC, iD1);
                    outputGENTree->Fill();
                }
            }
        }
    }
    
    void processDataEvent(Short_t centrality) {
        if (config.particleType == ParticleType::D0) {
            simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr;
            simpleDTreeevt* dinData = (simpleDTreeevt*)dinDataPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                doutMC->isMC = false;
                doutMC->isSwap = 0;
                doutMC->matchGEN = 0;
                doutMC->copyDn<simpleDTreeevt>(*dinData, iD1);
                outputTree->Fill();
            }
        } else {
            simpleDStarMCTreeflat* doutMC = (simpleDStarMCTreeflat*)doutMCPtr;
            simpleDStarDataTreeevt* dinData = (simpleDStarDataTreeevt*)dinDataPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                doutMC->isMC = false;
                doutMC->isSwap = 0;
                doutMC->matchGEN = 0;
                doutMC->copyDn(*dinData, iD1);
                outputTree->Fill();
            }
        }
    }
    
    void processMCEventMixed(Short_t centrality, Float_t ncoll, int& idxMC) {
        if (config.particleType == ParticleType::D0) {
            simpleDMCTreeevt* dinMC = (simpleDMCTreeevt*)dinMCPtr;
            simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinMC->candSize)) {
                if (dinMC->matchGEN[iD1] == true) {
                    doutMC->isMC = true;
                    doutMC->copyDn(*dinMC, iD1);
                    outputTree->Fill();
                } else {
                    if (idxMC % config.mcSampleRate == 0) {
                        doutMC->isMC = true;
                        doutMC->copyDn(*dinMC, iD1);
                        outputTree->Fill();
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
                    outputTree->Fill();
                } else {
                    if (idxMC % config.mcSampleRate == 0) {
                        doutMC->isMC = true;
                        doutMC->copyDn(*dinMC, iD1);
                        outputTree->Fill();
                    }
                    idxMC++;
                }
            }
        }
    }
    
    void processDataEventMixed(Short_t centrality, int& idxData) {
        if (config.particleType == ParticleType::D0) {
            simpleDMCTreeflat* doutMC = (simpleDMCTreeflat*)doutMCPtr;
            simpleDTreeevt* dinData = (simpleDTreeevt*)dinDataPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                if (idxData % config.dataSampleRate == 0) {
                    doutMC->isMC = false;
                    doutMC->isSwap = 0;
                    doutMC->matchGEN = 0;
                    doutMC->copyDn<simpleDTreeevt>(*dinData, iD1);
                    outputTree->Fill();
                }
                idxData++;
            }
        } else {
            simpleDStarMCTreeflat* doutMC = (simpleDStarMCTreeflat*)doutMCPtr;
            simpleDStarDataTreeevt* dinData = (simpleDStarDataTreeevt*)dinDataPtr;
            
            for (auto iD1 : ROOT::TSeqI(dinData->candSize)) {
                if (idxData % config.dataSampleRate == 0) {
                    doutMC->isMC = false;
                    doutMC->isSwap = 0;
                    doutMC->matchGEN = 0;
                    doutMC->copyDn(*dinData, iD1);
                    outputTree->Fill();
                }
                idxData++;
            }
        }
    }
    
    void finalize() override {
        if (outputFile) {
            logInfo("Writing output file with " + to_string(outputTree->GetEntries()) + " entries...");
            outputFile->cd();
            outputTree->Write();
            if (outputGENTree) {
                outputGENTree->Write();
                logInfo("GEN tree written with " + to_string(outputGENTree->GetEntries()) + " entries");
            }
            outputFile->Close();
            logInfo("Output file saved: " + string(outputFile->GetName()));
        }
        
        cleanup();
        DataProcessorBase::finalize();
    }
    
    void cleanup() {
        // Clean up data objects
        if (config.particleType == ParticleType::D0) {
            delete (simpleDMCTreeevt*)dinMCPtr;
            delete (simpleDMCTreeflat*)doutMCPtr;
            delete (simpleDTreeevt*)dinDataPtr;
        } else {
            delete (simpleDStarMCTreeevt*)dinMCPtr;
            delete (simpleDStarMCTreeflat*)doutMCPtr;
            delete (simpleDStarDataTreeevt*)dinDataPtr;
        }
        
        dinMCPtr = nullptr;
        doutMCPtr = nullptr;
        dinDataPtr = nullptr;
        
        if (outputFile) {
            delete outputFile;
            outputFile = nullptr;
        }
    }
};

// Convenience wrapper functions for easy usage
void ImprovedFlexibleMix(const string& mcPath, const string& dataPath, int jobIdx = 0, 
                        ParticleType particle = ParticleType::DStar, const string& suffix = "") {
    ProcessingConfig config = ConfigManager::getDefaultConfig(particle, ProcessingMode::MIXED, CollisionSystem::PP);
    config.inputPath = mcPath; // Note: Mixed mode needs both paths - this is simplified
    config.jobIdx = jobIdx;
    if (!suffix.empty()) {
        config.outputPrefix += "_" + suffix;
    }
    
    ImprovedFlexibleFlattener processor(config);
    processor.process();
}

void ImprovedFlexibleData(const string& dataPath, int jobIdx = 0, 
                         ParticleType particle = ParticleType::DStar, const string& suffix = "") {
    ProcessingConfig config = ConfigManager::getDefaultConfig(particle, ProcessingMode::DATA_ONLY, CollisionSystem::PP);
    config.inputPath = dataPath;
    config.jobIdx = jobIdx;
    if (!suffix.empty()) {
        config.outputPrefix += "_" + suffix;
    }
    
    ImprovedFlexibleFlattener processor(config);
    processor.process();
}

void ImprovedFlexibleMC(const string& mcPath, int jobIdx = 0, 
                       ParticleType particle = ParticleType::DStar, const string& suffix = "") {
    ProcessingConfig config = ConfigManager::getDefaultConfig(particle, ProcessingMode::MC_ONLY, CollisionSystem::PP);
    config.inputPath = mcPath;
    config.jobIdx = jobIdx;
    if (!suffix.empty()) {
        config.outputPrefix += "_" + suffix;
    }
    
    ImprovedFlexibleFlattener processor(config);
    processor.process();
}

// Main entry point with same interface as original
int ImprovedFlexibleFlattener_Main(int start = 0, int end = -1, int idx = 0, int type = 0, 
                                  string path = "", string suffix = "") {
    ParticleType particleType = ParticleType::DStar;
    ProcessingMode mode;
    
    switch (type) {
        case 0: mode = ProcessingMode::DATA_ONLY; break;
        case 1: mode = ProcessingMode::MC_ONLY; break;
        case 2: mode = ProcessingMode::MIXED; break;
        default:
            cerr << "Invalid type value: " << type << ". Must be 0 (Data), 1 (MC), or 2 (Mix)." << endl;
            return 1;
    }
    
    ProcessingConfig config = ConfigManager::getDefaultConfig(particleType, mode, CollisionSystem::PP);
    config.startEvent = start;
    config.endEvent = end;
    config.jobIdx = idx;
    
    if (!path.empty()) {
        config.inputPath = path;
    }
    if (!suffix.empty()) {
        config.outputPrefix += "_" + suffix;
    }
    
    // Set default paths if not provided
    if (config.inputPath.empty()) {
        if (mode == ProcessingMode::MC_ONLY) {
            config.inputPath = "/u/user/jun502s/SE_UserHome/DStarMC/D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/promptD0ToKPi_PT-1_TuneCP5_5p36TeV_pythia8-evtgen/crab_D0Ana_MCPromptD0Kpi_DpT1_CentralityTable_HFtowers200_HydjetDrum5F_CMSSW_13_2_11_25Apr23_v2/250423_131426/0000";
        } else {
            config.inputPath = "/u/user/jun502s/SE_UserHome/Run3_2023/Data/SkimMVA/D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/HIPhysicsRawPrime2/crab_D0Ana_Data_Step1_Run375513_HIPhysicsRawPrime0_wOffCentTable_CMSSW_13_2_13_MVA_25Apr2025_v1/250424_165928/";
        }
    }
    
    ImprovedFlexibleFlattener processor(config);
    processor.process();
    
    return 0;
}