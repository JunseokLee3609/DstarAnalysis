#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
// #include "../ParameterManager.h"

using ParamKey = std::pair<double, double>;
using D0ParamValue = std::pair<PDFParams::GaussianParams, PDFParams::PolynomialBkgParams>;
using DStarParamValue1 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarParamValue2 = std::pair<PDFParams::DoubleDBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarMCParamValue = std::pair<PDFParams::DBCrystalBallParams, PDFParams::DBCrystalBallParams>;
using D0ParamMap = std::map<ParamKey, D0ParamValue>;
using DStarParamMap1 = std::map<ParamKey, DStarParamValue1>;
using DStarParamMap2 = std::map<ParamKey, DStarParamValue2>;
using DStarMCParamMap = std::map<ParamKey, DStarMCParamValue>;

// KinematicParameterManager setupDStarParameters(std::vector<std::pair<double, double>> pTbins, std::vector<std::pair<double, double>> cosbins) {
    
//     // Create parameter manager for D* with DBCrystalBall signal and Phenomenological background
//     KinematicParameterManager<PDFParams::DoubleDBCrystalBallParams, PDFParams::PhenomenologicalParams> dstarManager;

//     PDFParams::DoubleDBCrystalBallParams defaultSignal;
    
//     PDFParams::PhenomenologicalParams defaultBkg;
//     dstarManager.setDefaultParameters(defaultSignal, defaultBkg);
    

//     dstarManager.setDefaultParameters(defaultSignal, defaultBkg);
    
//     // Set specific parameters for different pT ranges
//     for (pTbin : pTbins){
//         KinematicRange pT(pTbin.first, pTbin.second);
//         Binmap[{pT.pTMin,pT.pTMax,pT.cosMin,pT.cosMax}] = pT;
//         signalParamManager.setParameterValue(pT, "sigmaL", 0.0006);
        
//     }
//     for (cosbin : cosbins){
//         KinematicRange cos(cosMin=cosbin.first,cosMax=cosbin.second);
//         Binmap[{cos.pTMin,cos.pTMax,cos.cosMin,cos.cosMax}] = cos;
//         signalParamManager.setParameterValue(cos, "sigmaL", 0.0006);
//     }
    
    
//     signalParamManager.setParameterValue(midPt, "sigmaL", 0.0004);
//     signalParamManager.setParameterValue(highPt, "sigmaL", 0.0003);
// }


void MCMacro(bool doFit = false,bool useCUDA=true, float pTMin=4, float pTMax=100, float cosMin=-2, float cosMax=2){
    FitOpt D0opt;
    D0opt.useCUDA = useCUDA;
    D0opt.doFit = doFit;
    cout << D0opt.mvaMin << endl; 
    std::string particleType = "DStar";
    std::vector<std::pair<double,double>> ptBins = {
        {5,7},
        {7,10},
        {10,15},
        {15,20},
        {20,30}
    };
    std::vector<std::pair<double,double>> cosBins = {
        {-1,-0.75},
        {-0.75,-0.5},
        {-0.5,-0.25},
        {-0.25,0},
        {0,0.25},
        {0.25,0.5},
        {0.5,0.75},
        {0.75,1}
    };
    // std::vector<double> mvabin = {-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
    // std::string filepath = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_D0DATAMVA.root";
    // std::string filepathMC = "/home/jun502s/DstarAna/DstarAna_real/DDbarpPb/data/DstarMC_Rds_PbPb_250110.root";
    std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_Jun11_v1.root";
    std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_Jun11_v1.root";
    std::string datasetName = "datasetHX";
    auto params = DStarParamMaker1({0},{0});
    DataLoader MCloader(filepathMC);
    DataLoader Dataloader(filepathData);
    MCloader.loadRooDataSet(datasetName);
    Dataloader.loadRooDataSet(datasetName);

    

    DBCrystalBallParams value1;
    value1.mean = 0.1455;        // D* mass in GeV
    value1.mean_min = 0.1452;
    value1.mean_max = 0.1458;
    value1.sigmaR = 0.0005;
    value1.sigmaR_min = 0.0001;
    value1.sigmaR_max = 0.01;
    value1.sigmaL = 0.0005;
    value1.sigmaL_min = 0.0001;
    value1.sigmaL_max = 0.01;
    
    value1.alphaL = 2.;
    value1.alphaL_min = 0.0001;
    value1.alphaL_max = 10;
    value1.nL = 1.5;
    value1.nL_min = 1;
    value1.nL_max = 10;

    value1.alphaR = 2.;
    value1.alphaR_min = 0.0001;
    value1.alphaR_max = 10.;
    value1.nR = 1.5;
    value1.nR_min = 1;
    value1.nR_max = 10;
    DBCrystalBallParams value2;
    value2.mean = 0.1455;        // D* mass in GeV
    value2.mean_min = 0.1453;
    value2.mean_max = 0.1457;
    value2.sigmaR = 0.001;
    value2.sigmaR_min = 0.0002;
    value2.sigmaR_max = 0.02;
    value2.sigmaL = 0.001;
    value2.sigmaL_min = 0.0002;
    value2.sigmaL_max = 0.02;
    
    value2.alphaL = 0.002;
    value2.alphaL_min = 0.0005;
    value2.alphaL_max = 1;
    value2.nL = 1.5;
    value2.nL_min = 1;
    value2.nL_max = 200;

    value2.alphaR = 0.002;
    value2.alphaR_min = 0.0005;
    value2.alphaR_max = 1.;
    value2.nR = 1.5;
    value2.nR_min = 1;
    value2.nR_max = 100;
    DoubleDBCrystalBallParams value;
    value.mean1 = 0.1455;        // D* mass in GeV
    value.mean2 = 0.1455;        // D* mass in GeV
    value.mean_min = 0.1452;
    value.mean_max = 0.1458;
    value.sigmaR1 = 0.0005;
    value.sigmaR2 = 0.0005;
    value.sigmaR_min = 0.0001;
    value.sigmaR_max = 0.01;
    value.sigmaL1 = 0.0005;
    value.sigmaL2 = 0.0005;
    value.sigmaL_min = 0.0001;
    value.sigmaL_max = 0.01;
    
    value.alphaL1 = 2.;
    value.alphaL2 = 2.;
    value.alphaL_min = 0.005;
    value.alphaL_max = 10;
    value.nL1 = 1.5;
    value.nL2 = 1.5;
    value.nL_min = 1;
    value.nL_max = 10;

    value.alphaR1 = 2.;
    value.alphaR2 = 2.;
    value.alphaR_min = 0.005;
    value.alphaR_max = 20.;
    value.nR1 = 1.5;
    value.nR2 = 1.5;
    value.nR_min = 1;
    value.nR_max = 20;
    for(auto ptbin : ptBins){
        for(auto cosbin : cosBins){
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            D0opt.cosMin = cosbin.first;
            D0opt.cosMax = cosbin.second;
            D0opt.DStarMCDefault();
            
    
    MassFitter fitter(D0opt.name,D0opt.massVar,D0opt.massMin,D0opt.massMax);
    // fitter.PerformMCFit(D0opt,MCloader.getDataSet(),true,"","",value1,value2);
    fitter.PerformConstraintFit(D0opt,Dataloader.getDataSet(),MCloader.getDataSet(), true, "","",params[{0,0}].first,params[{0,0}].second);
    // fitter.PlotResult(true,D0opt.outputFile);
    PlotManager plotManager(D0opt,D0opt.outputDir,D0opt.outputFile,"plots/Data/DBCrystalBall",true,true);
    PlotManager plotManagerMC1(D0opt,D0opt.outputMCDir,D0opt.outputMCFile,"plots/MC/DBCrystalBall",true,true);
    // PlotManager plotManagerMC2(D0opt,D0opt.outputMCDir,D0opt.outputMCSwap1File,"plots/MC/Swap1");
    if(!doFit){
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

    





