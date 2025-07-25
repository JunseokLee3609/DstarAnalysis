#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <memory>
#include <chrono>
#include "TFile.h"
#include "TTree.h"
#include "TTreeIndex.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "../Tools/Transformations.h"
#include <algorithm>
#include <functional>

using namespace std;
using namespace RooFit;
TFile* createFileInDir(const std::string& dirPath, const std::string& filePath) {
    // 디렉토리 확인
    TSystemDirectory dir("dir", dirPath.c_str());
    void* dirCheck = gSystem->OpenDirectory(dirPath.c_str());

    if (!dirCheck) { // 폴더가 없으면 생성
        std::cout << "Directory does not exist. Creating: " << dirPath << std::endl;
        gSystem->MakeDirectory(dirPath.c_str());
    } else {
        std::cout << "Directory exists: " << dirPath << std::endl;
        gSystem->FreeDirectory(dirCheck);
    }

    // 새로운 TFile 생성
    TFile* file = TFile::Open((dirPath+filePath).c_str(), "RECREATE");
    if (file && file->IsOpen()) {
        std::cout << "Successfully created file: " << dirPath+filePath << std::endl;
        // file->Close();
    } else {
        std::cerr << "Failed to create file: " << dirPath+filePath << std::endl;
    }
    return file;
}

enum class VarType {
    FLOAT,    // 실수형 (RooRealVar)
    SHORT,
    INT,      // 정수형 (RooRealVar with integer range)
    BOOL,      // 불리언 (RooCategory with True/False)
    FORMULA
};

// 불리언 카테고리 상태를 정의하기 위한 구조체
struct CategoryState {
    string name;
    int value;
};

// 변수 정의를 위한 구조체
struct VarDef {
    string name;        // 변수 이름
    VarType type;       // 변수 타입
    double min;         // 최소값 (FLOAT/INT 타입에 사용)
    double max;         // 최대값 (FLOAT/INT 타입에 사용)
    vector<CategoryState> states; // 불리언 상태 (BOOL 타입에 사용)
    string formula;     // 수식 문자열 (FORMULA 타입에 사용)
    vector<string> dependVars; // 수식에 사용되는 변수 목록

    // FLOAT/INT 타입 변수 생성자
    VarDef(const string& n, VarType t, double mn, double mx) 
        : name(n), type(t), min(mn), max(mx) {}
    
    // BOOL 타입 변수 생성자 (자동으로 True/False 상태 생성)
    VarDef(const string& n, VarType t = VarType::BOOL) 
        : name(n), type(t), min(0), max(1) {
        if (t == VarType::BOOL) {
            states.push_back({"False", 0});
            states.push_back({"True", 1});
        }
    }
    VarDef(const string& n, const string& f, const vector<string>& deps)
    : name(n), type(VarType::FORMULA), min(0), max(0), formula(f), dependVars(deps) {}
};

// Chunk 방식으로 RooDataSet 생성 함수
vector<RooDataSet*> RDSMaker(
    const string& filename,            // 파일 경로
    const string& treename,            // 트리 이름
    const vector<VarDef>& variables,   // 변수 정의 목록
    // Long64_t chunkSize = 100000,       // 청크 크기 (기본: 10만 이벤트)
    Long64_t maxEntries = -1,          // 최대 처리 이벤트 수 (기본: 전체)
    bool saveCS = true,                // CS 프레임 데이터셋 저장 여부
    bool saveHX = true)                // HX 프레임 데이터셋 저장 여부
{
    auto startTime = chrono::high_resolution_clock::now();
    
    // 파일 열기
    TFile* file = TFile::Open(filename.c_str(), "READ");
    if (!file || file->IsZombie()) {
        cerr << "Error: 파일을 열 수 없습니다: " << filename << endl;
        return {};
    }
    
    // 트리 가져오기
    TTree* tree = (TTree*)file->Get(treename.c_str());
    if (!tree) {
        cerr << "Error: Can not find tree!: " << treename << endl;
        file->Close();
        return {};
    }
    
    Long64_t nEntries = tree->GetEntries();
    cout << "File has been loaded: " << filename << endl;
    cout << "Tree has been loaded: " << treename << " (총 항목 수: " << nEntries << ")" << endl;
    
    // 처리할 최대 항목 수 설정
    if (maxEntries > 0 && maxEntries < nEntries) {
        nEntries = maxEntries;
        cout << "At maximum, " << maxEntries << "entries will be processed." << endl;
    }
    vector<string> requiredBranches = {"pT", "eta", "phi", "mass", 
        "pTD1", "EtaD1", "PhiD1", "massDaugther1", 
        "pTD2", "EtaD2", "PhiD2"};
    auto checkRequiredBranches = [&](const vector<VarDef>& vars) -> bool {
        if (saveCS || saveHX) {
            for (const auto& reqBranch : requiredBranches) {
                auto it = find_if(vars.begin(), vars.end(), 
                    [&reqBranch](const VarDef& v) { 
                        return v.name == reqBranch && v.type == VarType::FLOAT; 
                    });
                if (it == vars.end()) {
                    if (!tree->GetBranch(reqBranch.c_str())) {
                        cerr << "Error: CS 또는 HX 프레임 계산에 필요한 Float 타입 브랜치 '" << reqBranch << "'가 variables에 정의되지 않았거나 트리에 없습니다." << endl;
                        return false;
                    }
                }
            }
        }
        return true;
    };
    cout << "Checking required branches..." << checkRequiredBranches(variables) <<  endl;
    if (!checkRequiredBranches(variables)) {
        file->Close();
        return {};
    }
    // RooArgSet 생성 (변수 컨테이너)
    RooArgSet baseVarSet;  // 기본 변수
    RooArgSet csVarSet;    // CS 프레임 변수
    RooArgSet hxVarSet;    // HX 프레임 변수
    map<string, RooAbsArg*> varMap;
    map<string, float> floatBranchMap;
    map<string, short> shortBranchMap;
    map<string, int> intBranchMap;
    map<string, bool> boolBranchMap;
    vector<VarDef> formulaVars;

    for (const auto& varDef : variables) {
        if (varDef.type == VarType::FORMULA) {
            formulaVars.push_back(varDef);
            continue;
        }
        switch (varDef.type) {
            case VarType::FLOAT: {
                RooRealVar* var = new RooRealVar(varDef.name.c_str(), varDef.name.c_str(), varDef.min, varDef.max);
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                floatBranchMap[varDef.name] = 0.0f;
                tree->SetBranchAddress(varDef.name.c_str(), &floatBranchMap[varDef.name]);
                cout << "Float var added: " << varDef.name << " (Range: " << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            case VarType::SHORT: {
                RooRealVar* var = new RooRealVar(varDef.name.c_str(), varDef.name.c_str(), varDef.min, varDef.max, "unit");
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                shortBranchMap[varDef.name] = 0;
                tree->SetBranchAddress(varDef.name.c_str(), &shortBranchMap[varDef.name]);
                cout << "Short var added: " << varDef.name << " (Range: " << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            case VarType::INT: {
                RooRealVar* var = new RooRealVar(varDef.name.c_str(), varDef.name.c_str(), varDef.min, varDef.max, "unit");
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                intBranchMap[varDef.name] = 0;
                tree->SetBranchAddress(varDef.name.c_str(), &intBranchMap[varDef.name]);
                cout << "Integer var added: " << varDef.name << " (Range: " << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            case VarType::BOOL: {
                RooCategory* var = new RooCategory(varDef.name.c_str(), varDef.name.c_str());
                var->defineType("False", 0);
                var->defineType("True", 1);
                baseVarSet.add(*var);
                varMap[varDef.name] = var;
                boolBranchMap[varDef.name] = 0;
                tree->SetBranchAddress(varDef.name.c_str(), &boolBranchMap[varDef.name]);
                cout << "Bool var added: " << varDef.name << endl;
                break;
            }
            default:
                break;
        }
    }

    // FORMULA 타입 변수 처리
    for (const auto& varDef : formulaVars) {
        RooArgList depVarList;
        bool allDepsFound = true;
        for (const auto& depName : varDef.dependVars) {
            if (varMap.find(depName) == varMap.end()) {
                cerr << "Error: 의존 변수 '" << depName << "'가 정의되지 않았습니다. 수식 '" 
                     << varDef.name << "' 생성 실패." << endl;
                allDepsFound = false;
                break;
            }
            RooAbsArg* depVar = varMap[depName];
            if (dynamic_cast<RooRealVar*>(depVar) || dynamic_cast<RooFormulaVar*>(depVar)) {
                depVarList.add(*depVar);
            } else {
                cerr << "Error: 의존 변수 '" << depName << "'가 RooRealVar 또는 RooFormulaVar 타입이 아닙니다." << endl;
                allDepsFound = false;
                break;
            }
        }
        if (allDepsFound) {
            RooFormulaVar* var = new RooFormulaVar(varDef.name.c_str(), varDef.formula.c_str(), depVarList);
            varMap[varDef.name] = var;
            cout << "Formula var added: " << varDef.name << " = " << varDef.formula << endl;
        }
    }
    
    RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    TTree::SetMaxTreeSize(100LL * 1024 * 1024 * 1024);

    RooRealVar cosThetaCS("cosThetaCS", "CosThetaCS", -1, 1);
    // RooRealVar phiCS("phiCS", "phiCS", -180, 180);
    // RooRealVar phiTildeCS("phiTildeCS", "phiTildeCS", -180, 180);
    RooRealVar cosThetaHX("cosThetaHX", "CosThetaHX", -1, 1);
    // RooRealVar phiHX("phiHX", "phiHX", -180, 180);
    // RooRealVar phiTildeHX("phiTildeHX", "phiTildeHX", -180, 180);

    csVarSet.add(baseVarSet);
    csVarSet.add(cosThetaCS);
    // csVarSet.add(phiCS);
    // csVarSet.add(phiTildeCS);

    hxVarSet.add(baseVarSet);
    hxVarSet.add(cosThetaHX);
    // hxVarSet.add(phiHX);
    // hxVarSet.add(phiTildeHX);

    // 데이터셋 생성
    // TTree::SetMaxTreeSize(100LL * 1024 * 1024 * 1024);
    // RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    RooDataSet* baseDataset = new RooDataSet("dataset", "Base Dataset", baseVarSet);
    RooDataSet* csDataset = saveCS ? new RooDataSet("datasetCS", "CS Frame Dataset", csVarSet) : nullptr;
    RooDataSet* hxDataset = saveHX ? new RooDataSet("datasetHX", "HX Frame Dataset", hxVarSet) : nullptr;

    Long64_t totalProcessed = 0;
    for (Long64_t entry = 0; entry < nEntries; entry++) {
        tree->GetEntry(entry);

        // 기본 변수 설정
        for (const auto& varDef : variables) {
            RooAbsArg* var = varMap[varDef.name];
            switch (varDef.type) {
                case VarType::FLOAT: {
                    float value = floatBranchMap[varDef.name];
                    RooRealVar* realVar = static_cast<RooRealVar*>(var);
                    // if (value < realVar->getMin() || value > realVar->getMax()) goto nextEntry;
                    realVar->setVal(value);
                    break;
                }
                case VarType::SHORT: {
                    short value = shortBranchMap[varDef.name];
                    RooRealVar* realVar = static_cast<RooRealVar*>(var);
                    // if (value < realVar->getMin() || value > realVar->getMax()) goto nextEntry;
                    realVar->setVal(value);
                    break;
                }
                case VarType::INT: {
                    int value = intBranchMap[varDef.name];
                    RooRealVar* realVar = static_cast<RooRealVar*>(var);
                    // if (value < realVar->getMin() || value > realVar->getMax()) goto nextEntry;
                    realVar->setVal(value);
                    break;
                }
                case VarType::BOOL: {
                    bool value = boolBranchMap[varDef.name];
                    RooCategory* catVar = static_cast<RooCategory*>(var);
                    catVar->setIndex(value);
                    break;
                }
                default: break;
            }
        }

        //  nextEntry:;

        // TLorentzVector 계산 (saveCS || saveHX가 true일 때만)
        // if (saveCS || saveHX) {
        if (saveHX) {
            TLorentzVector dstar, dau1;
            dstar.SetPtEtaPhiM(floatBranchMap["pT"], floatBranchMap["eta"], floatBranchMap["phi"], floatBranchMap["mass"]);
            dau1.SetPtEtaPhiM(floatBranchMap["pTD1"], floatBranchMap["EtaD1"], floatBranchMap["PhiD1"], floatBranchMap["massDaugther1"]);

            // if (saveCS) {
            //     TVector3 DstarDau1_CS = DstarDau1Vector_CollinsSoper(dstar, dau1);
            //     cosThetaCS.setVal(DstarDau1_CS.CosTheta());
            //     phiCS.setVal(DstarDau1_CS.Phi() * 180 / TMath::Pi());
            //     double phiCSVal = phiCS.getVal();
            //     if (cosThetaCS.getVal() < 0) {
            //         phiTildeCS.setVal((phiCSVal - 135 < -180) ? phiCSVal + 225 : phiCSVal - 135);
            //     } else {
            //         phiTildeCS.setVal((phiCSVal - 45 < -180) ? phiCSVal + 315 : phiCSVal - 45);
            //     }
            //     csDataset->add(csVarSet);
            // }
            // if (saveHX) {
                TVector3 DstarDau1_HX = DstarDau1Vector_Helicity(dstar, dau1);
                cosThetaHX.setVal(DstarDau1_HX.CosTheta());
                // phiHX.setVal(DstarDau1_HX.Phi() * 180 / TMath::Pi());
                // double phiHXVal = phiHX.getVal();
                // if (cosThetaHX.getVal() < 0) {
                //     phiTildeHX.setVal((phiHXVal - 135 < -180) ? phiHXVal + 225 : phiHXVal - 135);
                // } else {
                //     phiTildeHX.setVal((phiHXVal - 45 < -180) ? phiHXVal + 315 : phiHXVal - 45);
                // }
                hxDataset->add(hxVarSet);
            // }
        }

        baseDataset->add(baseVarSet);
        totalProcessed++;
    }

    cout << "  - " << totalProcessed << "/" << nEntries 
         << " has been processed (" << (int)(100.0 * totalProcessed / nEntries) << "%)" << endl;

    cout << "Adding formula variables using addColumn..." << endl;
    for (const auto& varDef : formulaVars) {
        if (varDef.type == VarType::FORMULA) {
            RooFormulaVar* formulaVar = static_cast<RooFormulaVar*>(varMap[varDef.name]);
            baseDataset->addColumn(*formulaVar);
            cout << "Added formula column: " << varDef.name << " to base dataset" << endl;
            if (saveCS && csDataset) csDataset->addColumn(*formulaVar);
            if (saveHX && hxDataset) hxDataset->addColumn(*formulaVar);
        }
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
    cout << "Processing completed in " << duration << " seconds." << endl;

    file->Close();
    for (auto& pair : varMap) delete pair.second;

    vector<RooDataSet*> result = {baseDataset};
    if (saveCS) result.push_back(csDataset);
    if (saveHX) result.push_back(hxDataset);
    return result;
}

// 메인 함수
void DStarRDSMaker(bool isMC = true, bool isD0=true, bool isPP=true, std::string inputPath = "", std::string suffix = "") {
    RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    TTree::SetMaxTreeSize(100LL * 1024 * 1024 * 1024);
    // 다양한 타입의 변수 정의
    vector<VarDef> variables = {
        {"mass", VarType::FLOAT, 1.7, 2.25},
        {"pT", VarType::FLOAT, 0.0, 100.0},
        {"eta", VarType::FLOAT, -2.5, 2.5},
       {"phi", VarType::FLOAT, -TMath::Pi(), TMath::Pi()},
	// {"Ncoll",VarType::FLOAT,0.0,3000},
        {"y", VarType::FLOAT, -1,1},
        {"dca3D", VarType::FLOAT, 0, 10}
        // {"pT_ratio", "pTD1 / pT", {"pTD1", "pT"}},
        // {"deltaEta", "eta - EtaD1", {"eta", "EtaD1"}}
        
    };
    if(!isD0){
        variables.push_back({"massDaugther1", VarType::FLOAT, 1.7, 2.1});
        variables.push_back({"massPion", "mass - massDaugther1", {"mass", "massDaugther1"}});
        variables.push_back({"pTD1", VarType::FLOAT, 0.0, 100.0});
        variables.push_back({"EtaD1", VarType::FLOAT, -2.5, 2.5});
        variables.push_back({"PhiD1", VarType::FLOAT, -5, 5});
        // variables.push_back({"3DDecayLength", VarType::FLOAT, 0.0, 50});
        // variables.push_back({"3DCosPointingAngle", VarType::FLOAT, -1, 1.0});
        // variables.push_back({"dcaDStar","3DDecayLength * 3DCosPointingAngle", {"3DDecayLength", "3DCosPointingAngle"}});
        // variables.push_back({"pTD2", VarType::FLOAT, 0.0, 100.0});
        // variables.push_back({"EtaD2", VarType::FLOAT, -2.5, 2.5});
        // variables.push_back({"PhiD2", VarType::FLOAT, -3.14, 3.14});
    }
    if (isMC) {
        variables.push_back({"isSwap", VarType::BOOL});
        variables.push_back({"isMC", VarType::BOOL});
        variables.push_back({"matchGEN", VarType::BOOL});
        variables.push_back({"matchGen_D1ancestorFlavor_", VarType::INT, 0, 5});
    }
    if(!isPP){
        variables.push_back({"centrality",VarType::SHORT, 0, 200}); // 중앙성 범위 설정
        variables.push_back({"Centrality","centrality/2",{"centrality"}});
        variables.push_back({"mva", VarType::FLOAT, 0.2, 1});
    }
    // 청크 단위로 RooDataSet 생성
    // string inputfilename = "/home/jun502s/DstarAna/DStarAnalysis/Data/flatSkimForBDT_DStarMC_ppRef_0_20250320.root"; //Data
    // string inputfilename = "/home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/ppMC/flatSkimForBDT_DStarMC_ppRef_NonSwap_Mar30_0_20250331.root"; //MC
    // string inputfilename = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/flatSkimForBDT_DStar_ppRef_NonSwapMC_0_07Apr25.root"; //MC
    string inputfilename = !inputPath.empty() ? inputPath : "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppData/flatSkimForBDT_DStar_ppRef_NonSwapData_PbPb_Data_ONNX_0_14Apr25.root"; //MC
    string outputfilename = !isMC ? "RDS_Physics_Data" : "RDS_Physics_MC";
    outputfilename = outputfilename + (isD0 ? "_D0" : "_DStar");
    outputfilename = outputfilename + (isPP ? "_ppRef" : "_PbPb");
    if(!suffix.empty()){
        outputfilename = outputfilename +"_" + suffix + ".root";
    }
    else {
        outputfilename = outputfilename + ".root";
    }
    string outputDirectory = !isMC ? "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/" : "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/";
    string treename = "skimTreeFlat";
    // Long64_t chunkSize = 5000000;
    Long64_t maxEntries = -1;
    bool saveCS = false;
    bool saveHX = true;
    if(isD0) saveCS = false;
    if(isD0) saveHX = false;
    

    
    // 데이터셋 사용 예시 (chunkSize, ChunkedRDSMaker → RDSMaker로 변경)
    vector<RooDataSet*> datasets = RDSMaker(inputfilename, treename, variables, maxEntries, saveCS, saveHX);

    if (!datasets.empty()) {
        // 출력 파일 생성
        TFile* outputFile = createFileInDir(outputDirectory.c_str(), outputfilename.c_str());

        // 데이터셋 저장
        datasets[0]->Write("dataset");  // 기본 데이터셋
        int idx = 1;
        if (saveCS && datasets.size() > idx) {
            datasets[idx]->Write("datasetCS"); idx++;
        }
        if (saveHX && datasets.size() > idx) {
            datasets[idx]->Write("datasetHX");
        }

        outputFile->Close();

        cout << "Dataset has been saved in " << Form("%s%s", outputDirectory.c_str(), outputfilename.c_str()) << "." << endl;

        // 메모리 해제
        for (auto* dataset : datasets) {
            delete dataset;
        }
    }
}
