#ifndef DSTARRDSMAKER_H
#define DSTARRDSMAKER_H

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
#include "../../Tools/Transformations.h"

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
TFile* createFileInDir(const string& dirPath, const string& filePath);

// 효율성 맵 로드
unique_ptr<TH3D> loadEfficiencyMap(const string& effMapFile, const string& effMapName);

// 필수 브랜치 확인
bool checkRequiredBranches(const vector<string>& requiredBranches, 
                          const vector<VarDef>& variables, 
                          TTree* tree);

// 진행률 출력
void printProgress(Long64_t current, Long64_t total);

// 템플릿 함수로 변수 값 설정 및 범위 체크
template<typename T>
bool setVariableValue(RooRealVar* var, T value) {
    if (value < var->getMin() || value >= var->getMax()) {
        return false;
    }
    var->setVal(static_cast<double>(value));
    return true;
}

// 이벤트 가중치 계산
double calculateEventWeight(TH3D* h_effMap, 
                           const TLorentzVector& dstar,
                           const TLorentzVector& dau1,
                           float pT_val, 
                           float y_val);

// ==================== VariableManager Class ====================
class VariableManager {
public:
    RooArgSet baseVarSet;
    map<string, RooAbsArg*> varMap;
    map<string, float> floatBranchMap;
    map<string, short> shortBranchMap;
    map<string, int> intBranchMap;
    map<string, bool> boolBranchMap;
    vector<VarDef> formulaVars;
    
    ~VariableManager();
    
    bool setupVariables(const vector<VarDef>& variables, TTree* tree);
    bool processEvent(const vector<VarDef>& variables);
    
private:
    bool setupBasicVariable(const VarDef& varDef, TTree* tree);
    bool setupFormulaVariables();
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
);

// ==================== Main Function ====================
void DStarRDSMakerImproved(
    bool isMC = true, 
    bool isD0 = false, 
    bool isPP = true, 
    const string& inputPath = "", 
    const string& suffix = "",
    const string& effFile = "", 
    const string& effMapNameInFile = "",
    const string& datasetKey = "",
    const string& datasetCatalog = "Data/datasets.json"
);

// ==================== Options Struct ====================
struct DStarRDSOptions {
    bool isMC = true;
    bool isD0 = false;
    bool isPP = true;
    Long64_t maxEntries = -1;
    bool saveCS = false;
    bool saveHX = true;
    bool saveEP = true;
    string inputPath;
    string suffix;
    string effFile;
    string effMapName = "efficiency_map";
    string outputDirPP = "../Data/PP/";
    string outputDirPbPb = "../Data/PbPb/";
    string datasetKey;
    string datasetCatalog = "Data/datasets.json";
};

// ==================== Enhanced Interface ====================
void RunDStarRDSMaker(const DStarRDSOptions& options);

#endif // DSTARRDSMAKER_H
