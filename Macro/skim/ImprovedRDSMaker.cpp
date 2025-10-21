#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include "TFile.h"
#include "TTree.h"
#include "TH3D.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooFormulaVar.h"
#include "../Tools/DataProcessorBase.h"
#include "../Tools/FileManager.h"
#include "../Tools/ConfigManager.h"
#include "../Tools/VariableHandler.h"
#include "../Tools/Transformations.h"

using namespace std;
using namespace RooFit;

class ImprovedRDSMaker : public DataProcessorBase {
private:
    vector<VarDef> variables;
    map<string, RooAbsArg*> varMap;
    map<string, float> floatBranchMap;
    map<string, short> shortBranchMap;
    map<string, int> intBranchMap;
    map<string, bool> boolBranchMap;
    
    unique_ptr<TFile> inputFile;
    TTree* inputTree = nullptr;
    TH3D* efficiencyMap = nullptr;
    
    // Output datasets
    unique_ptr<RooDataSet> baseDataset;
    unique_ptr<RooDataSet> csDataset;
    unique_ptr<RooDataSet> hxDataset;
    
    // RooFit variables for frame calculations
    unique_ptr<RooRealVar> cosThetaCS;
    unique_ptr<RooRealVar> cosThetaHX;
    unique_ptr<RooRealVar> weightVar;
    
    RooArgSet baseVarSet;
    RooArgSet csVarSet;
    RooArgSet hxVarSet;
    
public:
    ImprovedRDSMaker(const ProcessingConfig& cfg) : DataProcessorBase(cfg) {
        // Set RooDataSet storage optimizations
        RooDataSet::setDefaultStorageType(RooAbsData::Tree);
        TTree::SetMaxTreeSize(100LL * 1024 * 1024 * 1024);
    }
    
    ~ImprovedRDSMaker() {
        cleanup();
    }
    
    void process() override {
        if (!initialize()) {
            logError("Initialization failed");
            return;
        }
        
        processEvents();
        saveOutput();
        finalize();
    }
    
private:
    bool initialize() override {
        if (!DataProcessorBase::initialize()) {
            return false;
        }
        
        if (!loadInputFile()) return false;
        if (!loadEfficiencyMap()) return false;
        if (!setupVariables()) return false;
        if (!setupRooFitVariables()) return false;
        if (!setupDatasets()) return false;
        
        return true;
    }
    
    bool loadInputFile() {
        logInfo("Loading input file: " + config.inputPath);
        
        inputFile = make_unique<TFile>(TFile::Open(config.inputPath.c_str(), "READ"));
        if (!inputFile || inputFile->IsZombie()) {
            logError("Cannot open input file: " + config.inputPath);
            return false;
        }
        
        inputTree = (TTree*)inputFile->Get("skimTreeFlat");
        if (!inputTree) {
            logError("Cannot find tree 'skimTreeFlat' in input file");
            return false;
        }
        
        totalEvents = inputTree->GetEntries();
        logInfo("Input tree loaded with " + to_string(totalEvents) + " entries");
        
        return true;
    }
    
    bool loadEfficiencyMap() {
        if (config.effMapFile.empty()) {
            logInfo("No efficiency map provided - using weight 1.0");
            return true;
        }
        
        logInfo("Loading efficiency map: " + config.effMapFile);
        
        unique_ptr<TFile> effFile(TFile::Open(config.effMapFile.c_str(), "READ"));
        if (!effFile || effFile->IsZombie()) {
            logWarning("Cannot open efficiency file - using weight 1.0");
            return true;
        }
        
        efficiencyMap = (TH3D*)effFile->Get(config.effMapName.c_str());
        if (!efficiencyMap) {
            logWarning("Cannot find efficiency map '" + config.effMapName + "' - using weight 1.0");
            return true;
        }
        
        efficiencyMap->SetDirectory(0); // Detach from file
        logInfo("Efficiency map loaded successfully");
        
        return true;
    }
    
    bool setupVariables() {
        logInfo("Setting up variables...");
        
        // Create variables based on particle type and analysis mode
        bool isMC = (config.processingMode == ProcessingMode::MC_ONLY);
        bool isD0 = (config.particleType == ParticleType::D0);
        bool isPP = (config.collisionSystem == CollisionSystem::PP);
        
        variables = VariableHandler::createStandardVariables(isMC, isD0, isPP);
        
        // Validate required branches for frame calculations
        if (!VariableHandler::validateRequiredBranches(inputTree, variables, config.saveCS, config.saveHX)) {
            logError("Required branches validation failed");
            return false;
        }
        
        // Setup branch addresses
        VariableHandler::setupBranches(inputTree, variables, floatBranchMap, 
                                     shortBranchMap, intBranchMap, boolBranchMap);
        
        logInfo("Variables setup completed with " + to_string(variables.size()) + " variables");
        return true;
    }
    
    bool setupRooFitVariables() {
        logInfo("Setting up RooFit variables...");
        
        // Create base variable set
        baseVarSet = VariableHandler::createRooVarSet(variables, varMap);
        
        // Create frame-specific variables
        if (config.saveCS || config.saveHX) {
            weightVar = make_unique<RooRealVar>("weight", "Event Weight", 0.0, 10000.0);
            baseVarSet.add(*weightVar);
        }
        
        if (config.saveCS) {
            cosThetaCS = make_unique<RooRealVar>("cosThetaCS", "CosThetaCS", -1, 1);
            csVarSet.add(baseVarSet);
            csVarSet.add(*cosThetaCS);
        }
        
        if (config.saveHX) {
            cosThetaHX = make_unique<RooRealVar>("cosThetaHX", "CosThetaHX", -1, 1);
            hxVarSet.add(baseVarSet);
            hxVarSet.add(*cosThetaHX);
        }
        
        // Setup formula variables
        setupFormulaVariables();
        
        logInfo("RooFit variables setup completed");
        return true;
    }
    
    void setupFormulaVariables() {
        vector<VarDef> formulaVars;
        for (const auto& varDef : variables) {
            if (varDef.type == VarType::FORMULA) {
                formulaVars.push_back(varDef);
            }
        }
        
        for (const auto& varDef : formulaVars) {
            RooArgList depVarList;
            bool allDepsFound = true;
            
            for (const auto& depName : varDef.dependVars) {
                auto it = varMap.find(depName);
                if (it == varMap.end()) {
                    logError("Dependency variable '" + depName + "' not found for formula '" + varDef.name + "'");
                    allDepsFound = false;
                    break;
                }
                
                RooAbsArg* depVar = it->second;
                if (dynamic_cast<RooRealVar*>(depVar) || dynamic_cast<RooFormulaVar*>(depVar)) {
                    depVarList.add(*depVar);
                } else {
                    logError("Dependency variable '" + depName + "' is not RooRealVar or RooFormulaVar type");
                    allDepsFound = false;
                    break;
                }
            }
            
            if (allDepsFound) {
                RooFormulaVar* formulaVar = new RooFormulaVar(varDef.name.c_str(), 
                                                            varDef.formula.c_str(), depVarList);
                varMap[varDef.name] = formulaVar;
                logInfo("Formula variable added: " + varDef.name + " = " + varDef.formula);
            }
        }
    }
    
    bool setupDatasets() {
        logInfo("Setting up output datasets...");
        
        // Create base dataset
        baseDataset = make_unique<RooDataSet>("dataset", "Base Dataset", baseVarSet);
        
        // Create frame-specific datasets
        if (config.saveCS) {
            csDataset = make_unique<RooDataSet>("datasetCS", "CS Frame Dataset", 
                                              csVarSet, WeightVar(*weightVar));
        }
        
        if (config.saveHX) {
            hxDataset = make_unique<RooDataSet>("datasetHX", "HX Frame Dataset", 
                                              hxVarSet, WeightVar(*weightVar));
        }
        
        logInfo("Datasets setup completed");
        return true;
    }
    
    void processEvents() {
        logInfo("Processing events...");
        
        long long endEvent = determineEventRange(totalEvents);
        
        for (long long entry = config.startEvent; entry < endEvent; ++entry) {
            printProgress(entry, endEvent, 50000);
            
            inputTree->GetEntry(entry);
            
            // Set variable values and check for cuts
            bool skipEntry;
            VariableHandler::setVariableValues(variables, varMap, floatBranchMap,
                                             shortBranchMap, intBranchMap, boolBranchMap, skipEntry);
            
            if (skipEntry) continue;
            
            // Calculate event weight
            double eventWeight = calculateEventWeight(entry);
            
            // Add to base dataset
            baseDataset->add(baseVarSet);
            
            // Process frame calculations if needed
            if (config.saveHX && processHelicityFrame(eventWeight)) {
                hxDataset->add(hxVarSet, eventWeight);
            }
            
            if (config.saveCS && processCollinsSoperFrame(eventWeight)) {
                csDataset->add(csVarSet, eventWeight);
            }
            
            totalProcessed++;
        }
        
        // Add formula variables to datasets
        addFormulaColumns();
        
        logInfo("Event processing completed: " + to_string(totalProcessed) + " events processed");
    }
    
    double calculateEventWeight(long long entry) {
        double weight = 1.0;
        
        if (efficiencyMap && config.saveHX) {
            float pT_val = floatBranchMap["pT"];
            float y_val = floatBranchMap["y"];
            float cos_val = cosThetaHX->getVal();
            
            int bin = efficiencyMap->FindBin(pT_val, y_val, cos_val);
            double efficiency = efficiencyMap->GetBinContent(bin);
            
            if (efficiency > 1e-6) {
                weight = 1.0 / efficiency;
                if (weight > 10000) {
                    logWarning("Very high weight (" + to_string(weight) + 
                              ") for pT=" + to_string(pT_val) + 
                              ", y=" + to_string(y_val) + 
                              ", cos=" + to_string(cos_val));
                }
            } else {
                logWarning("Zero efficiency for pT=" + to_string(pT_val) + 
                          ", y=" + to_string(y_val) + 
                          ", cos=" + to_string(cos_val));
                weight = 0.0;
            }
        }
        
        if (weightVar) {
            weightVar->setVal(weight);
        }
        
        return weight;
    }
    
    bool processHelicityFrame(double weight) {
        try {
            TLorentzVector dstar, dau1;
            dstar.SetPtEtaPhiM(floatBranchMap["pT"], floatBranchMap["eta"], 
                              floatBranchMap["phi"], floatBranchMap["mass"]);
            dau1.SetPtEtaPhiM(floatBranchMap["pTD1"], floatBranchMap["EtaD1"], 
                             floatBranchMap["PhiD1"], floatBranchMap["massDaugther1"]);
            
            TVector3 dstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
            float cosTheta = dstarDau1_HX.CosTheta();
            
            cosThetaHX->setVal(cosTheta);
            return true;
        } catch (const exception& e) {
            logWarning("Error in helicity frame calculation: " + string(e.what()));
            return false;
        }
    }
    
    bool processCollinsSoperFrame(double weight) {
        try {
            TLorentzVector dstar, dau1;
            dstar.SetPtEtaPhiM(floatBranchMap["pT"], floatBranchMap["eta"], 
                              floatBranchMap["phi"], floatBranchMap["mass"]);
            dau1.SetPtEtaPhiM(floatBranchMap["pTD1"], floatBranchMap["EtaD1"], 
                             floatBranchMap["PhiD1"], floatBranchMap["massDaugther1"]);
            
            TVector3 dstarDau1_CS = DstarDau1Vector_CollinsSoper(dstar, dau1);
            float cosTheta = dstarDau1_CS.CosTheta();
            
            cosThetaCS->setVal(cosTheta);
            return true;
        } catch (const exception& e) {
            logWarning("Error in Collins-Soper frame calculation: " + string(e.what()));
            return false;
        }
    }
    
    void addFormulaColumns() {
        logInfo("Adding formula variables to datasets...");
        
        for (const auto& varDef : variables) {
            if (varDef.type == VarType::FORMULA) {
                auto it = varMap.find(varDef.name);
                if (it != varMap.end()) {
                    RooFormulaVar* formulaVar = static_cast<RooFormulaVar*>(it->second);
                    
                    baseDataset->addColumn(*formulaVar);
                    if (csDataset) csDataset->addColumn(*formulaVar);
                    if (hxDataset) hxDataset->addColumn(*formulaVar);
                    
                    logInfo("Added formula column: " + varDef.name);
                }
            }
        }
    }
    
    void saveOutput() {
        logInfo("Saving output datasets...");
        
        // Generate output filename
        string outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/";
        if (config.processingMode == ProcessingMode::MC_ONLY) {
            outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/";
        }
        
        string outputFileName = generateOutputFileName();
        
        unique_ptr<TFile> outputFile(FileManager::createFileInDir(outputDir, outputFileName));
        if (!outputFile) {
            logError("Failed to create output file");
            return;
        }
        
        // Save datasets
        baseDataset->Write("dataset");
        logInfo("Base dataset saved with " + to_string(baseDataset->numEntries()) + " entries");
        
        if (csDataset) {
            csDataset->Write("datasetCS");
            logInfo("CS dataset saved with " + to_string(csDataset->numEntries()) + " entries");
        }
        
        if (hxDataset) {
            hxDataset->Write("datasetHX");
            logInfo("HX dataset saved with " + to_string(hxDataset->numEntries()) + " entries");
        }
        
        outputFile->Close();
        logInfo("Output saved to: " + outputDir + outputFileName);
    }
    
    string generateOutputFileName() {
        string fileName = "RDS_Physics_";
        if (config.processingMode == ProcessingMode::MC_ONLY) {
            fileName = "RDS_MC_";
        }
        
        fileName += (config.particleType == ParticleType::D0) ? "D0" : "DStar";
        fileName += (config.collisionSystem == CollisionSystem::PP) ? "_ppRef" : "_PbPb";
        
        if (!config.suffix.empty()) {
            fileName += "_" + config.suffix;
        }
        
        fileName += ".root";
        return fileName;
    }
    
    void cleanup() {
        // Clean up RooFit objects
        for (auto& pair : varMap) {
            delete pair.second;
        }
        varMap.clear();
        
        if (efficiencyMap) {
            delete efficiencyMap;
            efficiencyMap = nullptr;
        }
    }
    
    void finalize() override {
        cleanup();
        DataProcessorBase::finalize();
    }
};

// Convenience wrapper function matching original interface
void ImprovedDStarRDSMaker(bool isMC = true, bool isD0 = false, bool isPP = true, 
                          string inputPath = "", string suffix = "", 
                          string effFile = "", string effMapName = "") {
    
    // Create configuration
    ProcessingMode mode = isMC ? ProcessingMode::MC_ONLY : ProcessingMode::DATA_ONLY;
    ParticleType particle = isD0 ? ParticleType::D0 : ParticleType::DStar;
    CollisionSystem system = isPP ? CollisionSystem::PP : CollisionSystem::PBPB;
    
    ProcessingConfig config = ConfigManager::getDefaultConfig(particle, mode, system);
    
    // Override with provided parameters
    if (!inputPath.empty()) {
        config.inputPath = inputPath;
    } else {
        // Set default path if not provided
        config.inputPath = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppData/flatSkimForBDT_DStar_ppRef_NonSwapData_PbPb_Data_ONNX_0_14Apr25.root";
    }
    
    if (!suffix.empty()) {
        config.suffix = suffix;
    }
    
    if (!effFile.empty()) {
        config.effMapFile = effFile;
        config.effMapName = effMapName.empty() ? "efficiency_map" : effMapName;
    }
    
    // Frame-specific settings
    if (isD0) {
        config.saveCS = false;
        config.saveHX = false;
    } else {
        config.saveCS = false;
        config.saveHX = true;
    }
    
    ImprovedRDSMaker processor(config);
    processor.process();
}

// Main entry point for compatibility
int ImprovedRDSMaker_Main(bool isMC = true, bool isD0 = false, bool isPP = true, 
                         string inputPath = "", string suffix = "", 
                         string effFile = "", string effMapName = "") {
    try {
        ImprovedDStarRDSMaker(isMC, isD0, isPP, inputPath, suffix, effFile, effMapName);
        return 0;
    } catch (const exception& e) {
        cerr << "Error in ImprovedRDSMaker: " << e.what() << endl;
        return 1;
    }
}