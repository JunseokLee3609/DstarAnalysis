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

using namespace std;
using namespace RooFit;

enum class VarType {
    FLOAT,    // 실수형 (RooRealVar)
    INT,      // 정수형 (RooRealVar with integer range)
    BOOL      // 불리언 (RooCategory with True/False)
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
};

// Chunk 방식으로 RooDataSet 생성 함수
RooDataSet* ChunkedRDSMaker(
    const string& filename,            // 파일 경로
    const string& treename,            // 트리 이름
    const vector<VarDef>& variables,   // 변수 정의 목록
    Long64_t chunkSize = 100000,       // 청크 크기 (기본: 10만 이벤트)
    Long64_t maxEntries = -1)          // 최대 처리 이벤트 수 (기본: 전체)
{
    auto startTime = chrono::high_resolution_clock::now();
    
    // 파일 열기
    TFile* file = TFile::Open(filename.c_str(), "READ");
    if (!file || file->IsZombie()) {
        cerr << "Error: 파일을 열 수 없습니다: " << filename << endl;
        return nullptr;
    }
    
    // 트리 가져오기
    TTree* tree = (TTree*)file->Get(treename.c_str());
    if (!tree) {
        cerr << "Error: Can not find tree!: " << treename << endl;
        file->Close();
        return nullptr;
    }
    
    Long64_t nEntries = tree->GetEntries();
    cout << "File has been loaded: " << filename << endl;
    cout << "Tree has been loaded: " << treename << " (총 항목 수: " << nEntries << ")" << endl;
    
    // 처리할 최대 항목 수 설정
    if (maxEntries > 0 && maxEntries < nEntries) {
        nEntries = maxEntries;
        cout << "At maximum," << maxEntries << "entries will be processed." << endl;
    }
    
    // RooArgSet 생성 (변수 컨테이너)
    RooArgSet varSet;
    // 변수들을 보관할 맵 (이름 -> 변수 포인터)
    map<string, RooAbsArg*> varMap;
    map<string, float> floatBranchMap;   // float 브랜치 버퍼
    map<string, int> intBranchMap;       // int 브랜치 버퍼
    map<string, bool> boolBranchMap;       // int 브랜치 버퍼
    
    // 변수 생성 및 브랜치 바인딩
    for (const auto& varDef : variables) {
        switch (varDef.type) {
            case VarType::FLOAT: {
                RooRealVar* var = new RooRealVar(
                    varDef.name.c_str(), 
                    varDef.name.c_str(),
                    varDef.min,
                    varDef.max
                );
                varSet.add(*var);
                varMap[varDef.name] = var;
                
                floatBranchMap[varDef.name] = 0.0f;
                tree->SetBranchAddress(varDef.name.c_str(), &floatBranchMap[varDef.name]);
                
                cout << "Float var has been added: " << varDef.name << " (Range: " 
                     << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            
            case VarType::INT: {
                RooRealVar* var = new RooRealVar(
                    varDef.name.c_str(), 
                    varDef.name.c_str(),
                    varDef.min,
                    varDef.max,
                    "unit"
                );
                // var->setInteger(true);
                varSet.add(*var);
                varMap[varDef.name] = var;
                
                // TTree 브랜치에 연결할 버퍼 생성
                intBranchMap[varDef.name] = 0;
                tree->SetBranchAddress(varDef.name.c_str(), &intBranchMap[varDef.name]);
                
                cout << "Integer var has been added: " << varDef.name << " (Range: " 
                     << varDef.min << " - " << varDef.max << ")" << endl;
                break;
            }
            
            case VarType::BOOL: {
                RooCategory* var = new RooCategory(
                    varDef.name.c_str(),
                    varDef.name.c_str()
                );
                var->defineType("False", 0);
                var->defineType("True", 1);
                varSet.add(*var);
                varMap[varDef.name] = var;
                
                // TTree 브랜치에 연결할 버퍼 생성
                boolBranchMap[varDef.name] = 0;
                tree->SetBranchAddress(varDef.name.c_str(), &boolBranchMap[varDef.name]);
                
                cout << "Bool var has been added: " << varDef.name << endl;
                break;
            }
        }
    }
    
    // 전체 RooDataSet 생성
    TTree::SetMaxTreeSize(100LL * 1024 * 1024 * 1024);
    RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    RooDataSet* fullDataset = new RooDataSet("dataset", "Full Dataset", varSet);
    
    // 청크 단위 처리
    Long64_t nChunks = (nEntries + chunkSize - 1) / chunkSize; // 올림 나눗셈
    Long64_t totalProcessed = 0;
    
    cout << "\n (Total " << nChunks << " chunks)" << endl;
    
    for (Long64_t chunk = 0; chunk < nChunks; chunk++) {
        Long64_t startEntry = chunk * chunkSize;
        Long64_t endEntry = min(startEntry + chunkSize, nEntries);
        Long64_t chunkEntries = endEntry - startEntry;
        
        cout << " chunk " << (chunk + 1) << "/" << nChunks 
             << " is processing (Entry: " << startEntry << " - " << (endEntry - 1) 
             << ", Size : " << chunkEntries << ")" << endl;
        
        // 현재 청크에 대한 RooDataSet 생성
        RooDataSet* chunkDataset = new RooDataSet("chunkset", "Chunk Dataset", varSet);
        
        // 청크 내 각 엔트리 처리
        for (Long64_t entry = startEntry; entry < endEntry; entry++) {
            tree->GetEntry(entry);
            
            // 각 변수의 값 설정
            for (const auto& varDef : variables) {
                RooAbsArg* var = varMap[varDef.name];
                
                switch (varDef.type) {
                    case VarType::FLOAT: {
                        float value = floatBranchMap[varDef.name];
                        // 범위 제한 적용
                        RooRealVar* realVar = static_cast<RooRealVar*>(var);
                        if (value < realVar->getMin() || value > realVar->getMax()) {
                            goto nextEntry; // 범위를 벗어나면 다음 항목으로
                        }
                        realVar->setVal(value);
                        break;
                    }
                    
                    case VarType::INT: {
                        int value = intBranchMap[varDef.name];
                        // 범위 제한 적용
                        RooRealVar* realVar = static_cast<RooRealVar*>(var);
                        if (value < realVar->getMin() || value > realVar->getMax()) {
                            goto nextEntry; // 범위를 벗어나면 다음 항목으로
                        }
                        realVar->setVal(value);
                        break;
                    }
                    
                    case VarType::BOOL: {
                        bool value = boolBranchMap[varDef.name];
                        RooCategory* catVar = static_cast<RooCategory*>(var);
                        if (catVar->setIndex(value)) {
                            goto nextEntry; // 정의되지 않은 상태면 다음 항목으로
                        }
                        // catVar->setIndex(value);
                        break;
                    }
                }
            }
            
            // 모든 변수가 유효하면 현재 항목을 데이터셋에 추가
            chunkDataset->add(varSet);
            totalProcessed++;
            
            nextEntry:; // 다음 항목으로 이동
        }
        
        // 현재 청크 데이터셋을 전체 데이터셋에 추가
        fullDataset->append(*chunkDataset);
        
        // 청크 데이터셋 메모리 해제
        delete chunkDataset;
        
        // 진행 상황 출력
        cout << "  - " << totalProcessed << "/" << nEntries 
             << " has been processed (" 
             << (int)(100.0 * totalProcessed / nEntries) << "%)" << endl;
    }
    
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();

    
    // 파일 닫기
    file->Close();
    
    // 메모리에서 변수 포인터 제거 (dataset이 복사본을 가지고 있음)
    for (auto& pair : varMap) {
        delete pair.second;
    }
    
    return fullDataset;
}

// 메인 함수
void RDSMaker() {
    // 다양한 타입의 변수 정의
    vector<VarDef> variables = {
        // 실수형 변수들
        {"mass",   VarType::FLOAT, 1.7, 2.0},
        {"pT",     VarType::FLOAT, 0.0, 100.0},
        {"eta",    VarType::FLOAT, -2.5, 2.5},
        
        // 정수형 변수
        // {"charge", VarType::INT, -1, 1},
        // {"nHits",  VarType::INT, 0, 50},
        
        // 불리언 변수
        {"isSwap",VarType::BOOL},
        {"isMC", VarType::BOOL},
        {"matchGEN", VarType::BOOL},
        {"mva", VarType::FLOAT, -1, 1}
    };
    
    // 청크 단위로 RooDataSet 생성
    string inputfilename = "/home/jun502s/DstarAna/DStarAnalysis/Data/flatSkimForBDT_Mix_11_2500227.root";  // 실제 파일 경로로 변경
    string outputfilename = "RDS_D0DATAMVA.root";  // 출력 파일 이름
    string outputDirectory = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC";  // 출력 파일 이름
    string treename = "skimTreeFlat";        // 실제 트리 이름으로 변경
    Long64_t chunkSize = 5000000;      // 청크 크기 설정 (필요에 따라 조정)
    Long64_t maxEntries = -1;        // 전체 데이터 처리 (-1: 제한 없음)
    
    RooDataSet* data = ChunkedRDSMaker(inputfilename, treename, variables, chunkSize, maxEntries);

    
    // 데이터셋 사용 예시
    if (data) {
        // 출력 파일 생성
        TFile* outputFile = new TFile(Form("%s/%s",outputDirectory.c_str(),outputfilename.c_str()), "RECREATE");
        
        // 데이터셋을 파일에 저장
        data->Write("dataset");
        
        // 파일 닫기
        outputFile->Close();
        delete outputFile;
        
        // cout << "Dataset has been saved in " << Form("%s/RDS_D0MC.root",outputDirectory.c_str()) << "." << endl;
        cout << "Dataset has been saved in " << Form("%s/%s",outputDirectory.c_str(),outputfilename.c_str()) << "." << endl;

        
        // 메모리에서 데이터셋 삭제 (파일에는 이미 복사되었으므로 안전함)
        delete data;
        data = nullptr;
    }
}