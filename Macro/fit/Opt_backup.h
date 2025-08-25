#ifndef OPT_H
#define OPT_H
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "Params.h"
#include "../Tools/GlobalCuts.h" 

struct FitOpt {

    std::string name;               // 인스턴스 이름
    
    

    std::string datasetName;        // 데이터셋 이름
    std::string reducedDataName;
    std::string treeName;           // 트리 이름 (필요시)
    std::string cutMCExpr;            // 추가 컷 문자열
    std::string cutExpr;            // 추가 컷 문자열
    std::string outputFile;
    std::string outputMCFile;
    std::string outputMCSwap0File;
    std::string outputMCSwap1File;
    std::string outputDCAFile;
    std::string pdfName;
    std::string fitResultName;
    std::string wsName;
    std::string plotName;
    std::string plotMCName;
    std::string pTLegend;
    std::string yLegend;
    std::string centLegend;
    std::string cosLegend;
    std::string dcaLegend;
    std::string ELabel;
    std::string subDir;

    
    std::string massVar;            // 질량 변수 이름
    std::string ptVar;              // pT 변수 이름
    std::string etaVar;             // eta 변수 이름
    std::string yVar;
    std::string centVar;            // 중심도 변수 이름
    std::string mvaVar;            // 중심도 변수 이름
    std::string cosVar;            // 중심도 변수 이름
    std::string dcaVar;
    std::vector<std::string> constraintParameters;
    
    double massMin;                 // 질량 최소값
    double massMax;                 // 질량 최대값
    
    bool useDeltaMass;              // 델타 M 사용 여부
    double deltaMassMin;            // 델타 M 최소값
    double deltaMassMax;            // 델타 M 최대값
    std::string deltaMassVar;       // 델타 M 변수명
    double mvaMin;
    double pTMin;
    double pTMax;
    double etaMin;
    double etaMax;
    double cosMin;
    double cosMax;
    double centMin;
    double centMax;
    double dcaMin;                  // DCA 최소값
    double dcaMax;                  // DCA 최대값
    
    std::vector<double> ptBins;
    std::vector<double> etaBins;
    std::vector<double> centBins;
    std::vector<double> mvaBins;
    std::vector<double> dcaBins =  {0,0.0012,0.0023,0.0039,0.0059,0.0085,0.0160,0.0281,0.0476,0.07};
    
    bool useMinos;             
    bool useHesse;            
    bool verbose;             
    bool useCUDA;
    bool doFit;
    
    std::string outputDir;    
    std::string outputPlotDir;
    std::string outputMCDir;    
    std::string outputPrefix; 
    bool savePlots;           
    bool saveWorkspace;       

private:
    std::string baseOutputDir;
    std::string baseOutputMCDir;
    std::string baseOutputPlotDir;
    
public:
    
    FitOpt() : 
        name("default"),
        datasetName("datasetHX"),
        reducedDataName("reducedData"),
        outputFile("outputtest.root"),
        fitResultName("fitResult"),
        massVar("mass"),
        ptVar("pT"),
        etaVar("eta"),
        yVar("y"),
        centVar("cent"),
        mvaVar("mva"),
        cosVar("cos"),
        ELabel(""),
        massMin(0.0),
        massMax(10.0),
        centMin(0),
        centMax(100),
        useDeltaMass(false),
        deltaMassMin(0.0),
        deltaMassMax(1.0),
        deltaMassVar("deltaMass"),
        subDir(""),
        mvaMin(0.0),
        // pTMin(4.0),
        // pTMax(100.0),
        // etaMin(-1.5),
        // etaMax(1.5),
        // cosMin(-1.0),
        // cosMax(1.0),
        useMinos(false),
        useHesse(true),
        verbose(false),
        useCUDA(true),
        outputDir("roots/Data/"),
        outputMCDir("roots/MC/"),
        baseOutputDir("roots/Data/"),
        baseOutputMCDir("roots/MC/"),
        baseOutputPlotDir("plots/"),
        savePlots(true),
        saveWorkspace(true)
    {}
    
    void setSubDir(const std::string& newSubDir) {
        subDir = newSubDir;
        updateDirectories();
    }
    
    void setBaseDirectories(const std::string& dataDir, const std::string& mcDir, const std::string& plotDir = "") {
        baseOutputDir = dataDir;
        baseOutputMCDir = mcDir;
        if (!plotDir.empty()) {
            baseOutputPlotDir = plotDir;
        }
        updateDirectories();
    }
    
    void updateDirectories() {
        if (subDir.empty()) {
            outputDir = baseOutputDir;
            outputMCDir = baseOutputMCDir;
            outputPlotDir = baseOutputPlotDir;
        } else {
            outputDir = addSubDirToPath(baseOutputDir, subDir);
            outputMCDir = addSubDirToPath(baseOutputMCDir, subDir);
            outputPlotDir = addSubDirToPath(baseOutputPlotDir, subDir);
        }
    }

private:
    std::string addSubDirToPath(const std::string& basePath, const std::string& sub) const {
        std::string path = basePath;
        if (!path.empty() && path.back() != '/') path += "/";
        return path + sub + "/";
    }

public:
    
    // 편의를 위한 D0 기본 설정 메서드
    // void D0MCDefault() {
    //     this->name = "D0";
    //     this->massMin = 1.74;
    //     this->massMax = 1.98;
    //     this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.5 && matchGEN==1 && isSwap==0", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str());
    //     this->pdfName = "total_pdf";
    //     this->wsName = "ws_D0";
    //     // this->datasetName = "reducedData";
    // }
    void D0DataDefault() {
        this->name = "D0";
        this->massMin = 1.74;
        this->massMax = 1.97;
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        this->cutExpr = Form("%s > %f", this->ptVar.c_str(), this->pTMin);
        this->pdfName = "total_pdf";
        this->wsName = "ws_D0";
        this->plotName = Form("Plot%s_%s_%s_%s_%s_%s",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputFile = Form("%s_%s_%s_%s_%s_%s",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/Data/", "roots/MC/", "plots/Data/");
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
        this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputFile = Form("%s_%s_%s_%s_%s_%s.root",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/MC/", "roots/MC/", "plots/MC/");
        // this->datasetName = "reducedData";
    }
        void D0DataDefault2() {
        this->name = "D0";
        this->massVar ="mass";
        this->massMin = 1.75;
        this->massMax = 1.97;
        this->datasetName = "dataset";
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        // this->cutExpr = Form("%s > %f", this->mvaVar.c_str(), this->mvaMin);
        this->cutExpr = Form("eta<1 && eta>-1 && pT>4");
        this->pdfName = "total_pdf";
        this->wsName = "ws_D0";
        this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputFile = Form("%s_%s_%s_%s_%s_%s.root",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/Data/", "roots/MC/", "plots/Data/");
        // this->datasetName = "reducedData";
    }
    void DStarMCAbsDefault() {
        this->name = "DStar";
        this->massVar ="massPion";
        this->massMin = 0.139;
        this->massMax = 0.155;
        this->dcaVar = "dca3D";
        this->dcaMin = 0.0;
        this->dcaMax = 0.07;
        // this->dcaBins ={0,0.001,0.0023,0.0039,0.0059,0.0085,0.0160,0.0281,0.0476,0.1};
        this->dcaBins ={0,0.001,0.0023,0.0039,0.0059,0.0085,0.0118,0.0160,0.0214,0.0281,0.0367,0.0476,0.07};
        this->ELabel= "ppRef #sqrt{s_{NN}} = 5.36 TeV";
        this->pTLegend = Form("%0.1f < p_{T} < %0.1f", this->pTMin, this->pTMax);
        this->yLegend = Form("|y| < 1");
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        // this->cutExpr = Form("%s > %f", this->mvaVar.c_str(), this->mvaMin);
        // this->cutMCExpr = Form("eta<1 && eta>-1 && pT>4 && matchGEN==1");
        this->cutMCExpr = Form("y<1 && y>-1 && abs(cosThetaHX)<%0.2f && abs(cosThetaHX) >=%0.2f&&  pT<%0.2f && pT>=%0.2f  && matchGEN==1", this->cosMax, this->cosMin, this->pTMax, this->pTMin);
        this->cutExpr = Form("y<1 && y>-1 && abs(cosThetaHX)<%0.2f && abs(cosThetaHX) >=%0.2f&&  pT<%0.2f && pT>=%0.2f", this->cosMax, this->cosMin, this->pTMax, this->pTMin);
        // this->cutExpr = Form("eta<1 && eta>-1 && pT<%f && pT>%f", this->pTMax, this->pTMin);
        this->pdfName = "total_pdf";
        this->wsName = Form("ws_%s",this->name.c_str());
        this->plotMCName = Form("PlotMC%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->plotName = Form("Plot%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->cosVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap0File = Form("MC_Swap0_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap1File = Form("MC_Swap1_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCFile = Form("MC_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputDCAFile = Form("DCA_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->constraintParameters= {};
        // this->constraintParameters= {"mean"};
        // this->constraintParameters= {"mean","alpha","n"};
        this->centLegend = Form("%0.2f < |cos#theta_{HX}| < %0.2f", this->cosMin, this->cosMax);
        
        this->outputFile = Form("Data_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/Data_DStar_ppRef/", "roots/MC_DStar_ppRef/", "plots/Data_DStar_ppRef/");
        // this->datasetName = "reducedData";
    }
        void DStarOODefault() {
        this->name = "DStar";
        this->massVar ="massPion";
        this->massMin = 0.139;
        this->massMax = 0.154;
        this->dcaVar = "dca3D";
        this->dcaMin = 0.0;
        this->dcaMax = 0.07;
        this->dcaBins ={0,0.001,0.0023,0.0039,0.0059,0.0085,0.0118,0.0160,0.0214,0.0281,0.0367,0.0476,0.07};
        this->cutMCExpr = Form("abs(y)<1 &&  pT<%0.2f && pT>=%0.2f  && matchGEN==1",  this->pTMax, this->pTMin);
        this->cutExpr = Form("abs(y)<1 &&  pT<%0.2f && pT>=%0.2f", this->pTMax, this->pTMin);
        this->ELabel= "OO #sqrt{s_{NN}} = 5.36 TeV";
        this->pTLegend = Form("%0.1f < p_{T} < %0.1f", this->pTMin, this->pTMax);
        this->yLegend = Form("|y| < 1");
        this->centLegend = "";
        // this->cutExpr = Form("eta<1 && eta>-1 && pT<%f && pT>%f", this->pTMax, this->pTMin);
        this->pdfName = "total_pdf";
        this->wsName = Form("ws_%s",this->name.c_str());
        this->plotMCName = Form("PlotMC%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->plotName = Form("Plot%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->cosVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap0File = Form("MC_Swap0_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap1File = Form("MC_Swap1_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCFile = Form("MC_%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputDCAFile = Form("DCA_%s_%s%sto%s_%s%s",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->constraintParameters= {};
        // this->constraintParameters= {"mean"};
        // this->constraintParameters= {"mean","alpha","n"};
        
        this->outputFile = Form("Data_%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/Data_DStar_ppRef/", "roots/MC_DStar_ppRef/", "plots/Data_DStar_ppRef/");
        // this->datasetName = "reducedData";
    }

    void DStarPPDefault() {
        this->name = "DStar";
        this->massVar ="massPion";
        this->massMin = 0.139;
        this->massMax = 0.154;
        this->dcaVar = "dca3D";
        this->dcaMin = 0.0;
        this->dcaMax = 0.07;
        this->dcaBins ={0,0.001,0.0023,0.0039,0.0059,0.0085,0.0118,0.0160,0.0214,0.0281,0.0367,0.0476,0.07};
        this->cutMCExpr = Form("abs(y)<1 &&  pT<%0.2f && pT>=%0.2f  && matchGEN==1",  this->pTMax, this->pTMin);
        this->cutExpr = Form("abs(y)<1 &&  pT<%0.2f && pT>=%0.2f", this->pTMax, this->pTMin);
        this->ELabel= "ppRef #sqrt{s_{NN}} = 5.36 TeV";
        this->pTLegend = Form("%0.1f < p_{T} < %0.1f", this->pTMin, this->pTMax);
        this->yLegend = Form("|y| < 1");
        this->centLegend = "";
        // this->cutExpr = Form("eta<1 && eta>-1 && pT<%f && pT>%f", this->pTMax, this->pTMin);
        this->pdfName = "total_pdf";
        this->wsName = Form("ws_%s",this->name.c_str());
        this->plotMCName = Form("PlotMC%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->plotName = Form("Plot%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->cosVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap0File = Form("MC_Swap0_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap1File = Form("MC_Swap1_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCFile = Form("MC_%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputDCAFile = Form("DCA_%s_%s%sto%s_%s%s",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->constraintParameters= {};
        // this->constraintParameters= {"mean"};
        // this->constraintParameters= {"mean","alpha","n"};
        
        this->outputFile = Form("Data_%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/Data_DStar_ppRef/", "roots/MC_DStar_ppRef/", "plots/Data_DStar_ppRef/");
        // this->datasetName = "reducedData";
    }
     void DStarMCAbsDefault2() {
        this->name = "DStar";
        this->massVar ="mass";
        this->massMin = 1.9;
        this->massMax = 2.1;
        this->dcaVar = "dca2D";
        this->dcaMin = 0.0;
        this->dcaMax = 0.07;
        // this->dcaBins ={0,0.001,0.0023,0.0039,0.0059,0.0085,0.0160,0.0281,0.0476,0.1};
        this->dcaBins ={0,0.001,0.0023,0.0039,0.0059,0.0085,0.0118,0.0160,0.0214,0.0281,0.0367,0.0476,0.07};
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        // this->cutExpr = Form("%s > %f", this->mvaVar.c_str(), this->mvaMin);
        // this->cutMCExpr = Form("eta<1 && eta>-1 && pT>4 && matchGEN==1");
        this->cutMCExpr = Form("y<1 && y>-1 && abs(cosThetaHX)<%0.2f && abs(cosThetaHX) >=%0.2f&&  pT<%0.2f && pT>=%0.2f  && matchGEN==1", this->cosMax, this->cosMin, this->pTMax, this->pTMin);
        this->cutExpr = Form("y<1 && y>-1 && abs(cosThetaHX)<%0.2f && abs(cosThetaHX) >=%0.2f&&  pT<%0.2f && pT>=%0.2f", this->cosMax, this->cosMin, this->pTMax, this->pTMin);
        // this->cutExpr = Form("eta<1 && eta>-1 && pT<%f && pT>%f", this->pTMax, this->pTMin);
        this->pdfName = "total_pdf";
        this->wsName = Form("ws_%s",this->name.c_str());
        this->plotMCName = Form("PlotMC%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->plotName = Form("Plot%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->cosVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap0File = Form("MC_Swap0_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap1File = Form("MC_Swap1_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCFile = Form("MC_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputDCAFile = Form("DCA_%s_%s%sto%s_%s%sto%s_%s%s",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->constraintParameters= {};
        // this->constraintParameters= {"mean"};
        // this->constraintParameters= {"mean","alpha","n"};
        this->centLegend = Form("%0.2f < |cos#theta_{HX}| < %0.2f", this->cosMin, this->cosMax);
        
        this->outputFile = Form("Data_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/Data_DStar_ppRef/", "roots/MC_DStar_ppRef/", "plots/Data_DStar_ppRef/");
        // this->datasetName = "reducedData";
    }
    void DStarMCDefault() {
        this->name = "DStar";
        this->massVar ="massPion";
        this->massMin = 0.140;
        this->massMax = 0.155;
        this->dcaVar = "dca3D";
        this->dcaMin = 0.0;
        this->dcaMax = 0.07;
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        // this->cutExpr = Form("%s > %f", this->mvaVar.c_str(), this->mvaMin);
        // this->cutMCExpr = Form("eta<1 && eta>-1 && pT>4 && matchGEN==1");
        this->cutMCExpr = Form("y<1 && y>-1 && cosThetaHX<%0.2f && cosThetaHX >%0.2f&&  pT<%0.2f && pT>%0.2f  && matchGEN==1", this->cosMax, this->cosMin, this->pTMax, this->pTMin);
        this->cutExpr = Form("y<1 && y>-1 && cosThetaHX<%0.2f && cosThetaHX >%0.2f&&  pT<%0.2f && pT>%0.2f", this->cosMax, this->cosMin, this->pTMax, this->pTMin);
        // this->cutExpr = Form("eta<1 && eta>-1 && pT<%f && pT>%f", this->pTMax, this->pTMin);
        this->pdfName = "total_pdf";
        this->wsName = Form("ws_%s",this->name.c_str());
        this->plotMCName = Form("PlotMC%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->plotName = Form("Plot%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->cosVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap0File = Form("MC_Swap0_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap1File = Form("MC_Swap1_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCFile = Form("MC_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputDCAFile = Form("DCA_%s_%s%sto%s_%s%sto%s_%s%s",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->constraintParameters= {"alpha","mean"};
        
        this->outputFile = Form("Data_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->cosVar.c_str(),convertDotToP(this->cosMin).c_str(),convertDotToP(this->cosMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->setBaseDirectories("roots/Data_DStar_ppRef/", "roots/MC_DStar_ppRef/", "plots/Data_DStar_ppRef/");
        // this->datasetName = "reducedData";
    }
    void D0MCDefault() {
        this->name = "D0";
        this->massVar ="mass";
        this->massMin = 1.75;
        this->massMax = 2.00;
        // this->cutExpr = Form("%s > 2.0 && %s < 100.0 && abs(%s) < 1.6 && %s > %f", this->ptVar.c_str(),this->ptVar.c_str(), this->etaVar.c_str(), this->mvaVar.c_str(), this->mvaMin);
        // this->cutExpr = Form("%s > %f", this->mvaVar.c_str(), this->mvaMin);
        // this->cutMCExpr = Form("eta<1 && eta>-1 && pT>4 && matchGEN==1");
        this->cutMCExpr = Form("y<1 && y>-1 && Centrality <%0.2f && Centrality >= %0.2f&&  pT<%0.2f && pT>=%0.2f && mva >= %0.9f  && matchGEN==1", this->centMax, this->centMin, this->pTMax, this->pTMin, this->mvaMin);
        this->cutExpr = Form("y<1 && y>-1 && Centrality <%0.2f && Centrality >= %0.2f&&  pT<%0.2f && pT>=%0.2f && mva >= %0.9f", this->centMax, this->centMin, this->pTMax, this->pTMin, this->mvaMin);
        // this->cutExpr = Form("y<1 && y>-1 && pT<%f && pT>%f", this->pTMax, this->pTMin);
        this->pdfName = "total_pdf";
        this->wsName = Form("ws_%s",this->name.c_str());
        this->plotMCName = Form("PlotMC%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->centVar.c_str(),convertDotToP(this->centMin).c_str(),convertDotToP(this->centMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->plotName = Form("Plot%s_%s%sto%s_%s%sto%s_%s%s.pdf",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->centVar.c_str(),convertDotToP(this->centMin).c_str(),convertDotToP(this->centMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        // this->plotName = Form("Plot%s_%s_%s_%s_%s_%s.pdf",this->name.c_str(), this->ptVar.c_str(), this->etaVar.c_str(),this->centVar.c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap0File = Form("MC_Swap0_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->centVar.c_str(),convertDotToP(this->centMin).c_str(),convertDotToP(this->centMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCSwap1File = Form("MC_Swap1_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->centVar.c_str(),convertDotToP(this->centMin).c_str(),convertDotToP(this->centMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputMCFile = Form("MC_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->centVar.c_str(),convertDotToP(this->centMin).c_str(),convertDotToP(this->centMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputFile = Form("Data_%s_%s%sto%s_%s%sto%s_%s%s.root",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->centVar.c_str(),convertDotToP(this->centMin).c_str(),convertDotToP(this->centMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->outputDCAFile = Form("DCA_%s_%s%sto%s_%s%sto%s_%s%s",this->name.c_str(), this->ptVar.c_str(), convertDotToP(this->pTMin).c_str(),convertDotToP(this->pTMax).c_str(),this->centVar.c_str(),convertDotToP(this->centMin).c_str(),convertDotToP(this->centMax).c_str(), this->mvaVar.c_str(), convertDotToP(this->mvaMin).c_str());
        this->constraintParameters= {};
        this->constraintParameters= {"sigma_Swap1"};
        this->setBaseDirectories("roots/Data_D0_PbPb/", "roots/MC_D0_PbPb/", "plots/Data_D0_PbPb/");
        // this->outputMCDir="roots/MC/";
        // this->outputDir="roots/Data/";
        this->datasetName = "dataset";
    }
    std::string convertDotToP(double value) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << value;
        std::string str = oss.str();
        std::replace(str.begin(), str.end(), '.', 'p');
        std::replace(str.begin(), str.end(), '-', 'm');
        return str;
    }

};

#endif // OPT_H
