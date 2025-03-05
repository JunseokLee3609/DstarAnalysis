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
    
    // 변수명 정보
    std::string massVar;            // 질량 변수 이름
    std::string ptVar;              // pT 변수 이름
    std::string etaVar;             // eta 변수 이름
    std::string centVar;            // 중심도 변수 이름
    
    // 질량 범위 설정
    double massMin;                 // 질량 최소값
    double massMax;                 // 질량 최대값
    
    // 델타 질량 모드 설정 (D* 분석용)
    bool useDeltaMass;              // 델타 M 사용 여부
    double deltaMassMin;            // 델타 M 최소값
    double deltaMassMax;            // 델타 M 최대값
    std::string deltaMassVar;       // 델타 M 변수명
    
    // binning 관련
    std::vector<double> ptBins;
    std::vector<double> etaBins;
    std::vector<double> centBins;
    
    // 피팅 옵션
    bool useMinos;             
    bool useHesse;            
    bool verbose;             
    
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
        massMin(0.0),
        massMax(10.0),
        useDeltaMass(false),
        deltaMassMin(0.0),
        deltaMassMax(1.0),
        deltaMassVar("deltaMass"),
        useMinos(false),
        useHesse(true),
        verbose(true),
        savePlots(true),
        saveWorkspace(true)
    {}
    
    // 편의를 위한 D0 기본 설정 메서드
    void D0Default() {
        this->name = "D0";
        this->massMin = 1.81;
        this->massMax = 1.92;
        this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.5 && matchGEN==1", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str());
        this->pdfName = "total_pdf";
        // this->datasetName = "reducedData";
    }

};

#endif // OPT_H