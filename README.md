# D* Meson Analysis Framework

A tool for analyzing D*


## Analysis Workflow

The analysis is structured to follow a logical sequence of steps:

1.  **Skim Ntuples**: Start by running the skimming code on raw data and MC ntuples to select candidate events.
2.  **Perform Fit**: Run the fitting algorithm on the skimmed data to determine the raw signal yield.
3.  **Calculate Efficiencyε**: Use skimmed MC ntuples to calculate the acceptance-times-efficiency factor.
4.  **Apply Corrections**: Apply all necessary correction factors to the calculated yield and efficiency.

---


## Getting Started

### Prerequisites

* ROOT6.24

### Installation
1.  **Clone the repo**

```bash
git clone https://github.com/JunseokLee3609/DstarAnalysis.git
```

---
### Usage Example

```bash
# 1. Run the skimmer
cd Macro/skim/
root FlexibleFlattener.cpp\(0,-1,0,1, your_input_path\) #(start entry, end entry, idx, isMC, input_path)
# 2. make RooDataset
root DstarRDSMaker\(true,false,false,your_input_path \) #(isMC, isD0, isPP, input_path)
# 3. Run fitting Macro
cd Macro/fit/testRun
```
### Make your own macro (Example)
```cpp
#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
#include "../DCAFitter.h"
// #include "../ParameterManager.h"

using ParamKey = std::pair<double, double>;
using D0ParamValue = std::pair<PDFParams::DoubleGaussianParams, PDFParams::PolynomialBkgParams>;
using DStarParamValue1 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarParamValue2 = std::pair<PDFParams::DoubleDBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarMCParamValue = std::pair<PDFParams::DBCrystalBallParams, PDFParams::DBCrystalBallParams>;
using D0ParamMap = std::map<ParamKey, D0ParamValue>;
using DStarParamMap1 = std::map<ParamKey, DStarParamValue1>;
using DStarParamMap2 = std::map<ParamKey, DStarParamValue2>;
using DStarMCParamMap = std::map<ParamKey, DStarMCParamValue>;
using DStarDBGaussValue = std::pair<PDFParams::DoubleGaussianParams, PDFParams::PhenomenologicalParams>;



void MCMacroDstar_condor(bool doReFit= false, bool plotFit = true,bool useCUDA=true, float pTMin=4, float pTMax=100, float cosMin=-2, float cosMax=2){
    FitOpt D0opt;
    D0opt.useCUDA = useCUDA;
    D0opt.doFit = doReFit;
    cout << D0opt.mvaMin << endl; 
    std::string particleType = "DStar";
    std::vector<std::pair<double,double>> ptBins = {
        {pTMin,pTMax}
    };
    std::vector<std::pair<double,double>> cosBins = {
        {cosMin,cosMax},
    };
    std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_Mix_Jun17_v1.root";
    std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_Jun11_v1.root";
    std::string datasetName = "datasetHX";

    // Define subDir name
    std::string subDir = "/DStar_ppRef_absCos_Jul08_MCOnly_test_v2/";

    double nsig_ratio = 0.01;     
    double nsig_min_ratio = 0.0;
    double nsig_max_ratio = 1.0;  
    double nbkg_ratio = 0.01;     
    double nbkg_min_ratio = 0.0;
    double nbkg_max_ratio = 1.0;  
    auto params = DStarParamMaker1({0},{0});

    if(pTMin ==10 && pTMax == 20){
        nsig_ratio = 0.3;        
        nsig_min_ratio = 0.2;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1;       
        nbkg_min_ratio = 0.01;
        nbkg_max_ratio = 0.7;
    params[{0,0}].first.mean = 0.1455;        // D* mass in GeV
    params[{0,0}].first.mean_min = 0.1452;
    params[{0,0}].first.mean_max = 0.1458;
    params[{0,0}].first.sigma = 0.0005;
    params[{0,0}].first.sigma_min = 0.0001;
    params[{0,0}].first.sigma_max = 0.01;

    params[{0,0}].first.sigmaR = 0.0005;
    params[{0,0}].first.sigmaR_min = 0.00001;
    params[{0,0}].first.sigmaR_max = 0.01;
    params[{0,0}].first.sigmaL = 0.0005;
    params[{0,0}].first.sigmaL_min = 0.00001;
    params[{0,0}].first.sigmaL_max = 0.01;
    
    params[{0,0}].first.alphaL = 1.1;
    params[{0,0}].first.alphaL_min = 0.01;
    params[{0,0}].first.alphaL_max = 5;
    params[{0,0}].first.nL = 1.2;
    params[{0,0}].first.nL_min = 1.1;
    params[{0,0}].first.nL_max = 20;

    params[{0,0}].first.alphaR = 1.1;
    params[{0,0}].first.alphaR_min = 0.01;
    params[{0,0}].first.alphaR_max = 5.;
    params[{0,0}].first.nR = 1.85;
    params[{0,0}].first.nR_min = 1.1;
    params[{0,0}].first.nR_max = 20;
        params[{0,0}].second.p0 = 0.01;
        params[{0,0}].second.p0_min = 0.001;
        params[{0,0}].second.p0_max = 1;
        params[{0,0}].second.p1 = 1;
        params[{0,0}].second.p1_min = -10. ;
        params[{0,0}].second.p1_max = 10.;
        params[{0,0}].second.p2 = 1;
        params[{0,0}].second.p2_min = -10.0;
        params[{0,0}].second.p2_max = 10; 
    }
    if(pTMin ==20 && pTMax == 50){
        nsig_ratio = 0.3;        
        nsig_min_ratio = 0.2;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1;       
        nbkg_min_ratio = 0.01;
        nbkg_max_ratio = 0.7;
    params[{0,0}].first.mean = 0.1455;        // D* mass in GeV
    params[{0,0}].first.mean_min = 0.1452;
    params[{0,0}].first.mean_max = 0.1458;
    params[{0,0}].first.sigma = 0.0005;
    params[{0,0}].first.sigma_min = 0.0001;
    params[{0,0}].first.sigma_max = 0.01;

    params[{0,0}].first.sigmaR = 0.0005;
    params[{0,0}].first.sigmaR_min = 0.00001;
    params[{0,0}].first.sigmaR_max = 0.01;
    params[{0,0}].first.sigmaL = 0.0005;
    params[{0,0}].first.sigmaL_min = 0.00001;
    params[{0,0}].first.sigmaL_max = 0.01;
    
    params[{0,0}].first.alphaL = 1.1;
    params[{0,0}].first.alphaL_min = 0.01;
    params[{0,0}].first.alphaL_max = 5;
    params[{0,0}].first.nL = 1.2;
    params[{0,0}].first.nL_min = 1.1;
    params[{0,0}].first.nL_max = 20;

    params[{0,0}].first.alphaR = 1.1;
    params[{0,0}].first.alphaR_min = 0.01;
    params[{0,0}].first.alphaR_max = 5.;
    params[{0,0}].first.nR = 1.85;
    params[{0,0}].first.nR_min = 1.1;
    params[{0,0}].first.nR_max = 20;
 
        params[{0,0}].second.p0 = 0.01;
        params[{0,0}].second.p0_min = 0.001;
        params[{0,0}].second.p0_max = 1;
        params[{0,0}].second.p1 = 1;
        params[{0,0}].second.p1_min = -10. ;
        params[{0,0}].second.p1_max = 10.;
        params[{0,0}].second.p2 = 1;
        params[{0,0}].second.p2_min = -10.0;
        params[{0,0}].second.p2_max = 10; 
    }

    DataLoader MCloader(filepathMC);
    DataLoader Dataloader(filepathData);
    MCloader.loadRooDataSet(datasetName);
    Dataloader.loadRooDataSet(datasetName);    

    for(auto ptbin : ptBins){
        for(auto cosbin : cosBins){
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            D0opt.cosMin = cosbin.first;
            D0opt.cosMax = cosbin.second;
            
            D0opt.DStarMCAbsDefault();

            D0opt.setSubDir(subDir);
            if(doReFit){
                 MassFitter fitter(D0opt.name, D0opt.massVar, D0opt.massMin, D0opt.massMax,
                                  nsig_ratio, nsig_min_ratio, nsig_max_ratio, nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio);
                fitter.PerformConstraintFit(D0opt,Dataloader.getDataSet(),MCloader.getDataSet(), true, "","",params[{0,0}].first,params[{0,0}].second);
          }
          
        
    PlotManager plotManager(D0opt, D0opt.outputDir, D0opt.outputFile, "plots/Data_DStar_ppRef/" + subDir, true, true);
    PlotManager plotManagerMC1(D0opt, D0opt.outputMCDir, D0opt.outputMCFile, "plots/MC_DStar_ppRef/" + subDir, true, true);
    // PlotManager plotManagerMC2(D0opt,D0opt.outputMCDir,D0opt.outputMCSwap1File,"plots/MC/Swap1");
    if(!plotFit){
        plotManagerMC1.DrawRawDistribution();
        // plotManagerMC2.DrawRawDistribution();
        plotManager.DrawRawDistribution();
    }
    else{
    plotManagerMC1.DrawFittedModel(true);
    // plotManagerMC2.DrawFittedModel(true);
    plotManager.DrawFittedModel(true);
    }
        }
    }
}
```
### Define global options (example)
```bash
vi Macro/fit/Opt.h
```
```cpp
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
```
### Run Macro
```bash
# 1. Run fitting Macro
cd Macro/fit/testRun
# Make your own Macro (example)
root yourMacro.cpp
```


