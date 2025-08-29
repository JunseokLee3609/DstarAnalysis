#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
// #include "../ParameterManager.h"

// using ParamKey = std::pair<double, double>;
// using D0ParamValue = std::pair<PDFParams::GaussianParams, PDFParams::PolynomialBkgParams>;
// using DStarParamValue1 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>;
// using DStarParamValue2 = std::pair<PDFParams::DoubleDBCrystalBallParams, PDFParams::PhenomenologicalParams>;
// using DStarMCParamValue = std::pair<PDFParams::DBCrystalBallParams, PDFParams::DBCrystalBallParams>;
// using D0ParamMap = std::map<ParamKey, D0ParamValue>;
// using DStarParamMap1 = std::map<ParamKey, DStarParamValue1>;
// using DStarParamMap2 = std::map<ParamKey, DStarParamValue2>;
// using DStarMCParamMap = std::map<ParamKey, DStarMCParamValue>;

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


void MCMacro_D0(bool doFit = false,bool useCUDA=true, float pTMin=4, float pTMax=100, float cosMin=-1, float cosMax=1){
    FitOpt D0opt;
    D0opt.useCUDA = useCUDA;
    D0opt.doFit = doFit;
    cout << D0opt.mvaMin << endl; 
    std::string particleType = "DStar";
    std::vector<double> ptBins = {0.0, 1.0, 2.0, 3.0};
    std::vector<double> etaBins = {0.0, 0.5, 1.0};
    // std::vector<double> mvabin = {-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
    // std::string filepath = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_D0DATAMVA.root";
    // std::string filepathMC = "/home/jun502s/DstarAna/DstarAna_real/DDbarpPb/data/DstarMC_Rds_PbPb_250110.root";
    // std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_MC_pp.root";
    // std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/DstarMC_Rds_pp.root";
    std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_D0_ppRef_pORun_Jul01_v1.root";
    std::string datasetName = "dataset";
    auto params = D0ParamMaker(ptBins,etaBins,3);
    // DataLoader MCloader(filepathMC);
    DataLoader Dataloader(filepathData);
    // MCloader.loadRooDataSet(datasetName,"workspace");
    Dataloader.loadRooDataSet(datasetName);
    // RooDataSet* dataset = (RooDataSet*)TFile::Open(filepath.c_str())->Get(datasetName.c_str());
    // fitter.SetData(dataset);
    // fitter.ApplyCut(D0opt.cutExpr);
    // for(auto mva : mvabin){
    // D0opt.mvaMin = mva;

    D0opt.D0DataDefault2();
    D0opt.pTMax= pTMax;
    D0opt.pTMin = pTMin;
    // D0opt.cosMin = cosMin;
    // D0opt.cosMax = cosMax;

    // DBCrystalBallParams value1;
    // value1.mean = 0.1455;        // D* mass in GeV
    // value1.mean_min = 0.1452;
    // value1.mean_max = 0.1458;
    // value1.sigmaR = 0.0005;
    // value1.sigmaR_min = 0.0001;
    // value1.sigmaR_max = 0.01;
    // value1.sigmaL = 0.0005;
    // value1.sigmaL_min = 0.0001;
    // value1.sigmaL_max = 0.01;
    
    // value1.alphaL = 2.;
    // value1.alphaL_min = 0.0001;
    // value1.alphaL_max = 10;
    // value1.nL = 1.5;
    // value1.nL_min = 1;
    // value1.nL_max = 10;

    // value1.alphaR = 2.;
    // value1.alphaR_min = 0.0001;
    // value1.alphaR_max = 10.;
    // value1.nR = 1.5;
    // value1.nR_min = 1;
    // value1.nR_max = 10;
    // DBCrystalBallParams value2;
    // value2.mean = 0.1455;        // D* mass in GeV
    // value2.mean_min = 0.1453;
    // value2.mean_max = 0.1457;
    // value2.sigmaR = 0.001;
    // value2.sigmaR_min = 0.0002;
    // value2.sigmaR_max = 0.02;
    // value2.sigmaL = 0.001;
    // value2.sigmaL_min = 0.0002;
    // value2.sigmaL_max = 0.02;
    
    // value2.alphaL = 0.002;
    // value2.alphaL_min = 0.0005;
    // value2.alphaL_max = 1;
    // value2.nL = 1.5;
    // value2.nL_min = 1;
    // value2.nL_max = 200;

    // value2.alphaR = 0.002;
    // value2.alphaR_min = 0.0005;
    // value2.alphaR_max = 1.;
    // value2.nR = 1.5;
    // value2.nR_min = 1;
    // value2.nR_max = 100;

    // DoubleGaussianParams param1;
    // param1.mean1 = 0.1455;
    // param1.mean1_min = 0.1453;
    // param1.mean1_max = 0.1457;
    // param1.sigma1 = 0.001;
    // param1.sigma1_min = 0.0002;
    // param1.sigma1_max = 0.02;
    // param1.mean2 = 0.1455;
    // param1.mean2_min = 0.1453;
    // param1.mean2_max = 0.1457;
    // param1.sigma2 = 0.001;
    // param1.sigma2_min = 0.0002;
    // param1.sigma2_max = 0.02;
    // GaussianParams param2;
    // param2.mean = 0.1455;
    // param2.mean_min = 0.1453;
    // param2.mean_max = 0.1457;
    // param2.sigma = 0.001;
    // param2.sigma_min = 0.0002;
    // param2.sigma_max = 0.02;
    
    MassFitter fitter(D0opt.name,D0opt.massVar,D0opt.massMin,D0opt.massMax);
    // fitter.PerformMCFit(D0opt,MCloader.getDataSet(),true,"","",value1,param2);
    fitter.PerformFit(D0opt,Dataloader.getDataSet(), true, "","", params[{0,0}].first,params[{0,0}].second);
    // fitter.PlotResult(true,D0opt.outputFile);
    PlotManager plotManager(D0opt,D0opt.outputDir,D0opt.outputFile,"plots/Data",true,false);
    // PlotManager plotManagerMC1(D0opt,D0opt.outputMCDir,D0opt.outputMCSwap0File,"plots/MC/Swap0");
    // PlotManager plotManagerMC2(D0opt,D0opt.outputMCDir,D0opt.outputMCSwap1File,"plots/MC/Swap1");
    if(!doFit){
        // plotManagerMC1.DrawRawDistribution();
        // plotManagerMC2.DrawRawDistribution();
        plotManager.DrawRawDistribution();
    }
    else{
    // plotManagerMC1.DrawFittedModel(true);
    // plotManagerMC2.DrawFittedModel(true);
    plotManager.DrawFittedModel(true);
    }
}
    

    





