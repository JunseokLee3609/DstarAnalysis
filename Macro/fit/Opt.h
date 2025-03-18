#ifndef OPT_H
#define OPT_H
#include <string>
#include <memory>
#include <vector>
#include "Params.h" 

struct FitOpt {

    // 기본 정보
    std::string name;               // 인스턴스 이름
    
    // 파일 및 데이터셋 관련
    std::string filePath;           // 데이터 파일 경로
    std::string datasetName;        // 데이터셋 이름
    std::string reducedDataName;
    std::string treeName;           // 트리 이름 (필요시)
    std::string cutExpr;            // 추가 컷 문자열
    std::string outputFile;
    std::string pdfName;
    std::string fitResultName;
    std::string wsName;
    std::string plotName;
    
    // 변수명 정보
    std::string massVar;            // 질량 변수 이름
    std::string ptVar;              // pT 변수 이름
    std::string etaVar;             // eta 변수 이름
    std::string centVar;            // 중심도 변수 이름
    std::string mvaVar;            // 중심도 변수 이름
    
    // 질량 범위 설정
    double massMin;                 // 질량 최소값
    double massMax;                 // 질량 최대값
    
    // 델타 질량 모드 설정 (D* 분석용)
    bool useDeltaMass;              // 델타 M 사용 여부
    double deltaMassMin;            // 델타 M 최소값
    double deltaMassMax;            // 델타 M 최대값
    std::string deltaMassVar;       // 델타 M 변수명
    double mvaMin;
    
    // binning 관련
    std::vector<double> ptBins;
    std::vector<double> etaBins;
    std::vector<double> centBins;
    std::vector<double> mvaBins;
    
    // 피팅 옵션
    bool useMinos;             
    bool useHesse;            
    bool verbose;             
    bool useCUDA;
    bool doFit;
    
    // 출력 관련
    std::string outputDir;    
    std::string outputPrefix; 
    bool savePlots;           
    bool saveWorkspace;       
    
    // 생성자 - 기본값 설정
    FitOpt() : 
        name("default"),
        datasetName("dataset"),
        reducedDataName("reducedData"),
        outputFile("outputtest.root"),
        fitResultName("fitResult"),
        massVar("mass"),
        ptVar("pT"),
        etaVar("eta"),
        centVar("cent"),
        mvaVar("mva"),
        massMin(0.0),
        massMax(10.0),
        useDeltaMass(false),
        deltaMassMin(0.0),
        deltaMassMax(1.0),
        deltaMassVar("deltaMass"),
        mvaMin(0.0),
        useMinos(false),
        useHesse(true),
        verbose(false),
        useCUDA(true),
        savePlots(true),
        saveWorkspace(true)
    {}
    
    // 편의를 위한 D0 기본 설정 메서드
    void D0MCDefault() {
        this->name = "D0";
        this->massMin = 1.74;
        this->massMax = 1.98;
        this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.5 && matchGEN==1 && isSwap==1", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str());
        this->pdfName = "total_pdf";
        this->wsName = "ws_D0";
        // this->datasetName = "reducedData";
    }
    void D0DataDefault() {
        this->name = "D0";
        this->massMin = 1.74;
        this->massMax = 1.98;
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        this->cutExpr = Form("%s > %f", this->mvaVar.c_str(), this->mvaMin);
        this->pdfName = "total_pdf";
        this->wsName = "ws_D0";
        this->plotName = Form("Plot%s_%s_%s_%s_%s_%s",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputFile = Form("%s_%s_%s_%s_%s_%s",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->datasetName = "reducedData";
    }
    void DStarDataDefault() {
        this->name = "DStar";
        this->massVar ="massPion";
        this->massMin = 0.14;
        this->massMax = 0.155;
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        // this->cutExpr = Form("%s > %f", this->mvaVar.c_str(), this->mvaMin);
        this->cutExpr = Form("eta<1 && eta>-1 && pT>4");
        this->pdfName = "total_pdf";
        this->wsName = "ws_DStar";
        this->plotName = Form("Plot%s_%s_%s_%s_%s_%s",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputFile = Form("%s_%s_%s_%s_%s_%s",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->datasetName = "reducedData";
    }
    std::string convertDotToP(double value) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << value;
        std::string str = oss.str();
        std::replace(str.begin(), str.end(), '.', 'p');
        std::replace(str.begin(), str.end(), '-', 'm');
        return str;
    }

};

#endif // OPT_H