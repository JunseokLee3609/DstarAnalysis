#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <map>
#include <algorithm>
#include "TFile.h"
#include "TTree.h"
#include "TH3D.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TMath.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooFormulaVar.h"
#include "../Tools/Transformations.h"

using namespace std;
using namespace RooFit;

// ==================== Constants ====================
namespace Constants {
    constexpr Long64_t PROGRESS_INTERVAL = 100000;
    constexpr double MIN_EFFICIENCY = 1e-6;
    constexpr double MAX_WEIGHT = 10000.0;
    constexpr double DEFAULT_WEIGHT = 1.0;
    constexpr Long64_t MAX_TREE_SIZE = 100LL * 1024 * 1024 * 1024; // 100GB
}

// ==================== Enums ====================
enum class VarType {
    FLOAT,
    SHORT,
    INT,
    BOOL,
    FORMULA
};

// ==================== Structures ====================
struct VarDef {
    string name;
    VarType type;
    double min;
    double max;
    string formula;
    vector<string> dependVars;

    // FLOAT/INT/SHORT 타입 변수 생성자
    VarDef(const string& n, VarType t, double mn, double mx) 
        : name(n), type(t), min(mn), max(mx) {}
    
    // BOOL 타입 변수 생성자
    VarDef(const string& n, VarType t = VarType::BOOL) 
        : name(n), type(t), min(0), max(1) {}
    
    // FORMULA 타입 변수 생성자
    VarDef(const string& n, const string& f, const vector<string>& deps)
        : name(n), type(VarType::FORMULA), min(0), max(0), formula(f), dependVars(deps) {}
};

// ==================== Helper Functions ====================

// 디렉토리 생성 및 파일 열기
TFile* createFileInDir(const string& dirPath, const string& filePath) {
    void* dirCheck = gSystem->OpenDirectory(dirPath.c_str());
    
    if (!dirCheck) {
        cout << "Directory does not exist. Creating: " << dirPath << endl;
        if (gSystem->MakeDirectory(dirPath.c_str()) != 0) {
            cerr << "ERROR: Failed to create directory: " << dirPath << endl;
            return nullptr;
        }
    } else {
        gSystem->FreeDirectory(dirCheck);
    }
    
    TFile* file = TFile::Open((dirPath + filePath).c_str(), "RECREATE");
    if (file && file->IsOpen()) {
        cout << "Successfully created file: " << dirPath + filePath << endl;
    } else {
        cerr << "ERROR: Failed to create file: " << dirPath + filePath << endl;
    }
    return file;
}

// 효율성 맵 로드
unique_ptr<TH3D> loadEfficiencyMap(const string& effMapFile, const string& effMapName) {
    if (effMapFile.empty()) {
        cout << ">>> No efficiency map provided. Using weight 1.0" << endl;
        return nullptr;
    }
    
    unique_ptr<TFile> f_eff(TFile::Open(effMapFile.c_str()));
    if (!f_eff || f_eff->IsZombie()) {
        cerr << "!!! WARNING: Could not open efficiency file: " << effMapFile << endl;
        return nullptr;
    }
    
    TH3D* h_effMap = dynamic_cast<TH3D*>(f_eff->Get(effMapName.c_str()));
    if (!h_effMap) {
        cerr << "!!! WARNING: Efficiency map '" << effMapName << "' not found in file" << endl;
        return nullptr;
    }
    
    cout << ">>> Efficiency map '" << effMapName << "' loaded from " << effMapFile << endl;
    h_effMap->SetDirectory(0);  // Detach from file
    
    return unique_ptr<TH3D>(h_effMap);
}

// 필수 브랜치 확인
bool checkRequiredBranches(const vector<string>& requiredBranches, 
                          const vector<VarDef>& variables, 
                          TTree* tree) {
    for (const auto& reqBranch : requiredBranches) {
        auto it = find_if(variables.begin(), variables.end(), 
            [&reqBranch](const VarDef& v) { 
                return v.name == reqBranch && v.type == VarType::FLOAT; 
            });
        
        if (it == variables.end() && !tree->GetBranch(reqBranch.c_str())) {
            cerr << "ERROR: Required branch '" << reqBranch 
                 << "' not found in variables or tree" << endl;
            return false;
        }
    }
    return true;
}

// 진행률 출력
void printProgress(Long64_t current, Long64_t total) {
    if (current % Constants::PROGRESS_INTERVAL == 0 || current == total - 1) {
        double progress = 100.0 * current / total;
        cout << "Processing: " << current << "/" << total 
             << " (" << fixed << setprecision(1) << progress << "%)" << endl;
    }
}

// 템플릿 함수로 변수 값 설정 및 범위 체크
template<typename T>
bool setVariableValue(RooRealVar* var, T value) {
    if (value < var->getMin() || value >= var->getMax()) {
        return false;  // Out of range
    }
    var->setVal(static_cast<double>(value));
    return true;
}

// 이벤트 가중치 계산
double calculateEventWeight(TH3D* h_effMap, 
                           const TLorentzVector& dstar,
                           const TLorentzVector& dau1,
                           float pT_val, 
                           float y_val) {
    if (!h_effMap) {
        return Constants::DEFAULT_WEIGHT;
    }
    
    TVector3 DstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
    float cos_val = abs(DstarDau1_HX.CosTheta());
    
    int bin = h_effMap->FindBin(pT_val, y_val, cos_val);
    double efficiency = h_effMap->GetBinContent(bin);
    
    if (efficiency < Constants::MIN_EFFICIENCY) {
        cout << "WARNING: Efficiency is zero for event with pT = " << pT_val 
             << ", y = " << y_val << ", cos = " << cos_val << endl;
        return 0.0;
    }
    
    double eventWeight = 1.0 / efficiency;
    
    if (eventWeight > Constants::MAX_WEIGHT) {
        cout << "WARNING: High weight " << eventWeight 
             << " for event with pT = " << pT_val 
             << ", y = " << y_val << ", cos = " << cos_val << endl;
    }
    
    return eventWeight;
}

// RooFit 변수 생성 및 브랜치 연결
class VariableManager {
public:
    RooArgSet baseVarSet;
    map<string, RooAbsArg*> varMap;
    map<string, float> floatBranchMap;
    map<string, short> shortBranchMap;
    map<string, int> intBranchMap;
    map<string, bool> boolBranchMap;
    vector<VarDef> formulaVars;
    
    ~VariableManager() {
        for (auto& pair : varMap) {
            delete pair.second;
        }
    }
    
    bool setupVariables(const vector<VarDef>& variables, TTree* tree) {
        for (const auto& varDef : variables) {
            if (varDef.type == VarType::FORMULA) {
                formulaVars.push_back(varDef);
                continue;
            }
            
            if (!setupBasicVariable(varDef, tree)) {
                return false;
            }
        }
        
        return setupFormulaVariables();
    }
    
    bool processEvent(const vector<VarDef>& variables) {
        for (const auto& varDef : variables) {
            if (varDef.type == VarType::FORMULA) continue;
            
            auto it = varMap.find(varDef.name);
            if (it == varMap.end()) continue;
            
            bool inRange = true;
            
            switch (varDef.type) {
                case VarType::FLOAT: {
                    RooRealVar* realVar = dynamic_cast<RooRealVar*>(it->second);
                    if (realVar) {
                        inRange = setVariableValue(realVar, floatBranchMap[varDef.name]);
                    }
                    break;
                }
                case VarType::SHORT: {
                    RooRealVar* realVar = dynamic_cast<RooRealVar*>(it->second);
                    if (realVar) {
                        inRange = setVariableValue(realVar, shortBranchMap[varDef.name]);
                    }
                    break;
                }
                case VarType::INT: {
                    RooRealVar* realVar = dynamic_cast<RooRealVar*>(it->second);
                    if (realVar) {
                        inRange = setVariableValue(realVar, intBranchMap[varDef.name]);
                    }
                    break;
                }
                case VarType::BOOL: {
                    RooCategory* catVar = dynamic_cast<RooCategory*>(it->second);
                    if (catVar) {
                        catVar->setIndex(boolBranchMap[varDef.name] ? 1 : 0);
                    }
                    break;
                }
                default:
                    break;
            }
            
            if (!inRange) {
                return false;  // Skip this event
            }
        }
        return true;  // Event is valid
    }
    
private:
    bool setupBasicVariable(const VarDef& varDef, TTree* tree) {
        switch (varDef.type) {
            case VarType::FLOAT: {
                RooRealVar* var = new RooRealVar(varDef.name.c_str(), varDef.name.c_str(), 
                                                 varDef.min, varDef.max);
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                floatBranchMap[varDef.name] = 0.0f;
                
                if (tree->SetBranchAddress(varDef.name.c_str(), &floatBranchMap[varDef.name]) < 0) {
                    cerr << "WARNING: Failed to set branch address for: " << varDef.name << endl;
                }
                cout << "Float var added: " << varDef.name 
                     << " (Range: " << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            case VarType::SHORT: {
                RooRealVar* var = new RooRealVar(varDef.name.c_str(), varDef.name.c_str(), 
                                                 varDef.min, varDef.max);
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                shortBranchMap[varDef.name] = 0;
                
                if (tree->SetBranchAddress(varDef.name.c_str(), &shortBranchMap[varDef.name]) < 0) {
                    cerr << "WARNING: Failed to set branch address for: " << varDef.name << endl;
                }
                cout << "Short var added: " << varDef.name 
                     << " (Range: " << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            case VarType::INT: {
                RooRealVar* var = new RooRealVar(varDef.name.c_str(), varDef.name.c_str(), 
                                                 varDef.min, varDef.max);
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                intBranchMap[varDef.name] = 0;
                
                if (tree->SetBranchAddress(varDef.name.c_str(), &intBranchMap[varDef.name]) < 0) {
                    cerr << "WARNING: Failed to set branch address for: " << varDef.name << endl;
                }
                cout << "Integer var added: " << varDef.name 
                     << " (Range: " << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            case VarType::BOOL: {
                RooCategory* var = new RooCategory(varDef.name.c_str(), varDef.name.c_str());
                var->defineType("False", 0);
                var->defineType("True", 1);
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                boolBranchMap[varDef.name] = false;
                
                if (tree->SetBranchAddress(varDef.name.c_str(), &boolBranchMap[varDef.name]) < 0) {
                    cerr << "WARNING: Failed to set branch address for: " << varDef.name << endl;
                }
                cout << "Bool var added: " << varDef.name << endl;
                break;
            }
            default:
                break;
        }
        return true;
    }
    
    bool setupFormulaVariables() {
        for (const auto& varDef : formulaVars) {
            RooArgList depVarList;
            
            for (const auto& depName : varDef.dependVars) {
                auto it = varMap.find(depName);
                if (it == varMap.end()) {
                    cerr << "ERROR: Dependency '" << depName 
                         << "' not found for formula '" << varDef.name << "'" << endl;
                    return false;
                }
                
                RooAbsArg* depVar = it->second;
                if (!dynamic_cast<RooRealVar*>(depVar) && !dynamic_cast<RooFormulaVar*>(depVar)) {
                    cerr << "ERROR: Dependency '" << depName 
                         << "' is not RooRealVar or RooFormulaVar" << endl;
                    return false;
                }
                depVarList.add(*depVar);
            }
            
            RooFormulaVar* var = new RooFormulaVar(varDef.name.c_str(), 
                                                    varDef.formula.c_str(), 
                                                    depVarList);
            varMap[varDef.name] = var;
            cout << "Formula var added: " << varDef.name << " = " << varDef.formula << endl;
        }
        return true;
    }
};

// ==================== Main RDS Maker Function ====================
vector<RooDataSet*> RDSMaker(
    const string& filename,
    const string& treename,
    const vector<VarDef>& variables,
    Long64_t maxEntries = -1,
    bool saveCS = false,
    bool saveHX = true,
    bool saveEP = true,
    const string& effMapFile = "",
    const string& effMapName = "efficiency_map"
)
{
    auto startTime = chrono::high_resolution_clock::now();
    
    // 파일 열기
    unique_ptr<TFile> file(TFile::Open(filename.c_str(), "READ"));
    if (!file || file->IsZombie()) {
        cerr << "ERROR: Cannot open file: " << filename << endl;
        return {};
    }
    
    // 트리 가져오기
    TTree* tree = dynamic_cast<TTree*>(file->Get(treename.c_str()));
    if (!tree) {
        cerr << "ERROR: Cannot find tree: " << treename << endl;
        return {};
    }
    
    Long64_t nEntries = tree->GetEntries();
    cout << "File loaded: " << filename << endl;
    cout << "Tree loaded: " << treename << " (Total entries: " << nEntries << ")" << endl;
    
    if (maxEntries > 0 && maxEntries < nEntries) {
        nEntries = maxEntries;
        cout << "Processing maximum " << maxEntries << " entries" << endl;
    }
    
    // 효율성 맵 로드
    unique_ptr<TH3D> h_effMap = loadEfficiencyMap(effMapFile, effMapName);
    
    // 필수 브랜치 확인
    if (saveCS || saveHX) {
        vector<string> requiredBranches = {
            "pT", "eta", "phi", "mass", 
            "pTD1", "EtaD1", "PhiD1", "massDaugther1"
        };
        if (!checkRequiredBranches(requiredBranches, variables, tree)) {
            return {};
        }
    }
    
    // 변수 설정
    VariableManager varMgr;
    if (!varMgr.setupVariables(variables, tree)) {
        cerr << "ERROR: Failed to setup variables" << endl;
        return {};
    }
    
    // Event plane angle (조건부 로드)
    Double_t Psi2Raw_Trk = -99.0;
    if (saveEP && tree->GetBranch("Psi2Raw_Trk")) {
        tree->SetBranchAddress("Psi2Raw_Trk", &Psi2Raw_Trk);
    }
    
    // RooFit 설정
    RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    TTree::SetMaxTreeSize(Constants::MAX_TREE_SIZE);
    
    // 가중치 변수
    RooRealVar weightVar("weight", "Event Weight", 0.0, Constants::MAX_WEIGHT);
    varMgr.baseVarSet.add(weightVar);
    
    // 각도 변수
    RooRealVar cosThetaCS("cosThetaCS", "CosThetaCS", -1, 1);
    RooRealVar cosThetaHX("cosThetaHX", "CosThetaHX", -1, 1);
    RooRealVar cosThetaEP("cosThetaEP", "CosThetaEP", -1, 1);
    
    // 프레임별 변수 세트
    RooArgSet csVarSet(varMgr.baseVarSet);
    csVarSet.add(cosThetaCS);
    
    RooArgSet hxVarSet(varMgr.baseVarSet);
    hxVarSet.add(cosThetaHX);
    
    RooArgSet epVarSet(varMgr.baseVarSet);
    epVarSet.add(cosThetaEP);
    
    // 데이터셋 생성
    RooDataSet* baseDataset = new RooDataSet("dataset", "Base Dataset", varMgr.baseVarSet);
    
    RooDataSet* csDataset = nullptr;
    if (saveCS) {
        csDataset = new RooDataSet("datasetCS", "CS Frame Dataset", csVarSet, WeightVar(weightVar));
    }
    
    RooDataSet* hxDataset = nullptr;
    if (saveHX) {
        hxDataset = new RooDataSet("datasetHX", "HX Frame Dataset", hxVarSet, WeightVar(weightVar));
    }
    
    RooDataSet* epDataset = nullptr;
    if (saveEP) {
        epDataset = new RooDataSet("datasetEP", "Event Plane Frame Dataset", epVarSet, WeightVar(weightVar));
    }
    
    // 이벤트 루프
    Long64_t totalProcessed = 0;
    Long64_t totalSkipped = 0;
    
    for (Long64_t entry = 0; entry < nEntries; entry++) {
        tree->GetEntry(entry);
        printProgress(entry, nEntries);
        
        // 변수 값 설정 및 범위 체크
        if (!varMgr.processEvent(variables)) {
            totalSkipped++;
            continue;
        }
        
        double eventWeight = Constants::DEFAULT_WEIGHT;
        
        // 물리 프레임 계산
        if (saveHX || saveCS || saveEP) {
            // TLorentzVector 생성 (한 번만)
            TLorentzVector dstar, dau1;
            dstar.SetPtEtaPhiM(
                varMgr.floatBranchMap["pT"], 
                varMgr.floatBranchMap["eta"], 
                varMgr.floatBranchMap["phi"], 
                varMgr.floatBranchMap["mass"]
            );
            dau1.SetPtEtaPhiM(
                varMgr.floatBranchMap["pTD1"], 
                varMgr.floatBranchMap["EtaD1"], 
                varMgr.floatBranchMap["PhiD1"], 
                varMgr.floatBranchMap["massDaugther1"]
            );
            
            // Helicity 프레임
            if (saveHX) {
                TVector3 DstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
                cosThetaHX.setVal(DstarDau1_HX.CosTheta());
                
                // 효율성 가중치 계산 (DstarDau1_HX 재사용)
                if (h_effMap) {
                    eventWeight = calculateEventWeight(
                        h_effMap.get(), 
                        dstar, 
                        dau1,
                        varMgr.floatBranchMap["pT"],
                        varMgr.floatBranchMap["y"]
                    );
                    
                    if (eventWeight == 0.0) {
                        totalSkipped++;
                        continue;
                    }
                }
                
                weightVar.setVal(eventWeight);
                hxDataset->add(hxVarSet, eventWeight);
            }
            
            // Collins-Soper 프레임
            if (saveCS) {
                TVector3 DstarDau1_CS = DstarDau1Vector_CollinsSoper(dstar, dau1);
                cosThetaCS.setVal(DstarDau1_CS.CosTheta());
                csDataset->add(csVarSet, eventWeight);
            }
            
            // Event Plane 프레임
            if (saveEP) {
                TVector3 DstarDau1_EP = DstarDau1Vector_EventPlane(dstar, dau1, Psi2Raw_Trk);
                cosThetaEP.setVal(DstarDau1_EP.CosTheta());
                epDataset->add(epVarSet, eventWeight);
            }
        }
        
        baseDataset->add(varMgr.baseVarSet);
        totalProcessed++;
    }
    
    cout << "\n=== Processing Summary ===" << endl;
    cout << "Total processed: " << totalProcessed << "/" << nEntries 
         << " (" << (100.0 * totalProcessed / nEntries) << "%)" << endl;
    cout << "Total skipped: " << totalSkipped << endl;
    
    // Formula 변수 추가
    if (!varMgr.formulaVars.empty()) {
        cout << "\nAdding formula variables..." << endl;
        for (const auto& varDef : varMgr.formulaVars) {
            auto it = varMgr.varMap.find(varDef.name);
            if (it != varMgr.varMap.end()) {
                RooFormulaVar* formulaVar = dynamic_cast<RooFormulaVar*>(it->second);
                if (formulaVar) {
                    baseDataset->addColumn(*formulaVar);
                    if (saveCS && csDataset) csDataset->addColumn(*formulaVar);
                    if (saveHX && hxDataset) hxDataset->addColumn(*formulaVar);
                    if (saveEP && epDataset) epDataset->addColumn(*formulaVar);
                    cout << "  Added: " << varDef.name << endl;
                }
            }
        }
    }
    
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
    cout << "\nProcessing completed in " << duration << " seconds." << endl;
    
    // 결과 반환
    vector<RooDataSet*> result = {baseDataset};
    if (saveCS && csDataset) result.push_back(csDataset);
    if (saveHX && hxDataset) result.push_back(hxDataset);
    if (saveEP && epDataset) result.push_back(epDataset);
    
    return result;
}

// ==================== Main Function ====================
void DStarRDSMakerImproved(
    bool isMC = true, 
    bool isD0 = false, 
    bool isPP = true, 
    const string& inputPath = "", 
    const string& suffix = "",
    const string& effFile = "", 
    const string& effMapNameInFile = ""
) {
    cout << "\n========================================" << endl;
    cout << "  D* RDS Maker (Improved Version)" << endl;
    cout << "========================================" << endl;
    cout << "Configuration:" << endl;
    cout << "  - MC: " << (isMC ? "Yes" : "No") << endl;
    cout << "  - Particle: " << (isD0 ? "D0" : "D*") << endl;
    cout << "  - Collision: " << (isPP ? "pp" : "PbPb") << endl;
    cout << "========================================\n" << endl;
    
    // 변수 정의
    vector<VarDef> variables = {
        {"mass", VarType::FLOAT, 1.7, 2.25},
        {"pT", VarType::FLOAT, 5.0, 50.0},
        {"eta", VarType::FLOAT, -2.5, 2.5},
        {"phi", VarType::FLOAT, -TMath::Pi(), TMath::Pi()},
        {"y", VarType::FLOAT, -1, 1},
        {"dca3D", VarType::FLOAT, 0, 10}
    };
    
    // D* 특화 변수
    if (!isD0) {
        variables.push_back({"massDaugther1", VarType::FLOAT, 1.7, 2.1});
        variables.push_back({"massPion", "mass - massDaugther1", {"mass", "massDaugther1"}});
        variables.push_back({"pTD1", VarType::FLOAT, 0.0, 50.0});
        variables.push_back({"EtaD1", VarType::FLOAT, -2.5, 2.5});
        variables.push_back({"PhiD1", VarType::FLOAT, -TMath::Pi(), TMath::Pi()});
        variables.push_back({"pTD2", VarType::FLOAT, 0.0, 50.0});
        variables.push_back({"EtaD2", VarType::FLOAT, -2.5, 2.5});
        variables.push_back({"pTGrandD1", VarType::FLOAT, 0.0, 50.0});
        variables.push_back({"EtaGrandD1", VarType::FLOAT, -2.5, 2.5});
        variables.push_back({"pTGrandD2", VarType::FLOAT, 0.0, 50.0});
        variables.push_back({"EtaGrandD2", VarType::FLOAT, -2.5, 2.5});
    }
    
    // MC 특화 변수
    if (isMC) {
        variables.push_back({"isSwap", VarType::BOOL});
        variables.push_back({"isMC", VarType::BOOL});
        variables.push_back({"matchGEN", VarType::BOOL});
        variables.push_back({"matchGen_D1ancestorFlavor_", VarType::INT, 0, 10});
    }
    
    // PbPb 특화 변수
    if (!isPP) {
        variables.push_back({"centrality", VarType::SHORT, 0, 200});
        variables.push_back({"Centrality", "centrality/2", {"centrality"}});
        variables.push_back({"mva", VarType::FLOAT, 0.9, 1.0});
    }
    
    // 파일 경로 설정
    string inputfilename = inputPath.empty() 
        ? "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppData/flatSkimForBDT_DStar_ppRef_NonSwapData_PbPb_Data_ONNX_0_14Apr25.root"
        : inputPath;
    
    // 출력 파일명 생성
    string outputfilename = isMC ? "RDS_Physics_MC" : "RDS_Physics_Data";
    outputfilename += isD0 ? "_D0" : "_DStar";
    outputfilename += isPP ? "_ppRef" : "_PbPb";
    if (!suffix.empty()) {
        outputfilename += "_" + suffix;
    }
    outputfilename += ".root";
    
    string outputDirectory = isMC 
        ? "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/"
        : "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/";
    
    string treename = "skimTreeFlat";
    
    // 프레임 저장 설정
    bool saveCS = false;
    bool saveHX = !isD0;  // D0는 HX 프레임 저장 안함
    bool saveEP = !isD0;  // D0는 EP 프레임 저장 안함
    
    // RooDataSet 생성
    vector<RooDataSet*> datasets = RDSMaker(
        inputfilename, 
        treename, 
        variables, 
        -1,  // maxEntries (-1 = all)
        saveCS, 
        saveHX, 
        saveEP, 
        effFile, 
        effMapNameInFile
    );
    
    if (datasets.empty()) {
        cerr << "ERROR: No datasets created!" << endl;
        return;
    }
    
    // 출력 파일 저장
    TFile* outputFile = createFileInDir(outputDirectory, outputfilename);
    if (!outputFile || !outputFile->IsOpen()) {
        cerr << "ERROR: Failed to create output file" << endl;
        for (auto* dataset : datasets) delete dataset;
        return;
    }
    
    // 데이터셋 저장
    cout << "\nSaving datasets..." << endl;
    datasets[0]->Write("dataset");
    cout << "  - Base dataset saved" << endl;
    
    int idx = 1;
    if (saveCS && datasets.size() > idx) {
        datasets[idx]->Write("datasetCS");
        cout << "  - CS frame dataset saved" << endl;
        idx++;
    }
    if (saveHX && datasets.size() > idx) {
        datasets[idx]->Write("datasetHX");
        cout << "  - HX frame dataset saved" << endl;
        idx++;
    }
    if (saveEP && datasets.size() > idx) {
        datasets[idx]->Write("datasetEP");
        cout << "  - EP frame dataset saved" << endl;
    }
    
    outputFile->Close();
    delete outputFile;
    
    cout << "\n========================================" << endl;
    cout << "Dataset saved successfully!" << endl;
    cout << "Output: " << outputDirectory << outputfilename << endl;
    cout << "========================================" << endl;
    
    // 메모리 해제
    for (auto* dataset : datasets) {
        delete dataset;
    }
}

