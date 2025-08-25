#include "../MassFitter_backup.h"
#include "../Opt_backup.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
#include "../DCAFitter.h"
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
// using DStarDBGaussValue = std::pair<PDFParams::DoubleGaussianParams, PDFParams::PhenomenologicalParams>;

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


void MCMacroDstarOO_condor(bool doReFit= false, bool plotFit = true,bool useCUDA=true, float pTMin=4, float pTMax=50){
    FitOpt D0opt;
    D0opt.useCUDA = useCUDA;
    D0opt.doFit = doReFit;
    cout << D0opt.mvaMin << endl; 
    std::string particleType = "DStar";
    std::vector<std::pair<double,double>> ptBins = {
        {pTMin,pTMax}
    };
    // std::vector<std::pair<double,double>> cosBins = {
    //     {cosMin,cosMax},
    // };
    // std::vector<double> mvabin = {-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
    // std::string filepath = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_D0DATAMVA.root";
    // std::string filepathMC = "/home/jun502s/DstarAna/DstarAna_real/DDbarpPb/data/DstarMC_Rds_PbPb_250110.root";
    // std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_Jun11_v1.root";
    std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_noPreselectionCut_ppRef_Jul25_v1.root";
    //std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_Mix_Jun17_v1.root";
    // std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_Jun11_v1.root";

    // std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_noPreselectionCut_ppRef_Jul25_v1.root";
    std::string filepathDataoo = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_OO_Jul25_v1.root";
    // std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_noReweight_OO_Jul25_v1.root";
    std::string filepathDatapp = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_ppRef_Jul25_v1.root";
    std::string datasetName = "datasetHX";

    // 결과 저장을 위한 하위 디렉토리 이름 설정 (D0 코드 스타일)
    // std::string EtaCut = "abs(EtaD2)< 1.2 && abs(EtaGrandD1) < 1.2 && abs(EtaGrandD2) < 1.2 && massDaugther1 > 1.82 && massDaugther1 < 1.92";
    std::string EtaCut = "abs(EtaD2)< 1.6 && abs(EtaGrandD1) < 1.6 && abs(EtaGrandD2) < 1.6 && pTD2 >0.4 && pTGrandD1 >0.8 && pTGrandD2 >0.8 ";
    // std::string EtaCut = "abs(y)<1";
    auto params2 = DStarParamMaker3({0},{0});
    auto params = DStarParamMaker3({0},{0});
    DataLoader MCloader(filepathMC);
    DataLoader Dataloaderoo(filepathDataoo);
    DataLoader Dataloaderpp(filepathDatapp);
    MCloader.loadRooDataSet(datasetName);
    Dataloaderoo.loadRooDataSet(datasetName);    
    Dataloaderpp.loadRooDataSet(datasetName);    
    double nsig_ratio = 0.01;     
    double nsig_min_ratio = 0.0;
    double nsig_max_ratio = 1.0;  
    double nbkg_ratio = 0.01;     
    double nbkg_min_ratio = 0.0;
    double nbkg_max_ratio = 1.0;  
          params2[{0,0}].first.sigma1 = 0.0005;
        params2[{0,0}].first.sigma1_min = 0.00001;
        params2[{0,0}].first.sigma1_max = 0.005;
        params2[{0,0}].first.sigma2 = 0.0005;
        params2[{0,0}].first.sigma2_min = 0.00001;
        params2[{0,0}].first.sigma2_max = 0.001;
        params2[{0,0}].first.fraction = 0.2;
        params2[{0,0}].first.fraction_min = 0.1;
        params2[{0,0}].first.fraction_max = 1.0;
         params2[{0,0}].first.sigma1 = 0.0005;
    // params[{0,0}].first.nL = 1.1;
    // params[{0,0}].first.nL_min = 1;
    // params[{0,0}].first.nL_max = 200;
    //     params[{0,0}].first.nR = 1.1;
    // params[{0,0}].first.nR_min = 1;
    // params[{0,0}].first.nR_max = 20;
    // // if(pTMax<=5){ 
    //     nsig_ratio = 0.3;        
    //     nsig_min_ratio = 0.2;
    //     nsig_max_ratio = 0.9;
    //     nbkg_ratio = 0.;       
    //     nbkg_min_ratio = 0.01;
    //     nbkg_max_ratio = 1.0;
    // params[{0,0}].first.mean = 0.1455;        // D* mass in GeV
    // params[{0,0}].first.mean_min = 0.1452;
    // params[{0,0}].first.mean_max = 0.1458;
    // params[{0,0}].first.sigma = 0.0005;
    // params[{0,0}].first.sigma_min = 0.0001;
    // params[{0,0}].first.sigma_max = 0.01;

    // params[{0,0}].first.sigmaR = 0.0005;
    // params[{0,0}].first.sigmaR_min = 0.00001;
    // params[{0,0}].first.sigmaR_max = 0.01;
    // params[{0,0}].first.sigmaL = 0.0005;
    // params[{0,0}].first.sigmaL_min = 0.00001;
    // params[{0,0}].first.sigmaL_max = 0.01;
    
    // params[{0,0}].first.alphaL = 1.1;
    // params[{0,0}].first.alphaL_min = 0.5;
    // params[{0,0}].first.alphaL_max = 2;
    // params[{0,0}].first.nL = 1.1;
    // params[{0,0}].first.nL_min = 1;
    // params[{0,0}].first.nL_max = 200;

    // params[{0,0}].first.alphaR = 1;
    // params[{0,0}].first.alphaR_min = 0.5;
    // params[{0,0}].first.alphaR_max = 2;
    // params[{0,0}].first.nR = 1.1;
    // params[{0,0}].first.nR_min = 1;
    // params[{0,0}].first.nR_max = 20;
    //     params[{0,0}].second.p0 = 0.01;
    //     params[{0,0}].second.p0_min = 0.001;
    //     params[{0,0}].second.p0_max = 1;
    //     params[{0,0}].second.p1 = 20;
    //     params[{0,0}].second.p1_min = -100 ;
    //     params[{0,0}].second.p1_max = 100.;
    //     params[{0,0}].second.p2 = 0;
    //     params[{0,0}].second.p2_min = -20;
    //     params[{0,0}].second.p2_max = 20; 
        // params2[{0,0}].first = params[{0,0}].first;
// }

    // if(pTMin ==5 && pTMax == 6){
    //     nsig_ratio = 0.3;        
    //     nsig_min_ratio = 0.2;
    //     nsig_max_ratio = 0.9;
    //     nbkg_ratio = 0.2;       
    //     nbkg_min_ratio = 0.2;
    //     nbkg_max_ratio = 0.9;
    // params[{0,0}].first.mean = 0.1455;        // D* mass in GeV
    // params[{0,0}].first.mean_min = 0.1452;
    // params[{0,0}].first.mean_max = 0.1458;
    // params[{0,0}].first.sigma = 0.0005;
    // params[{0,0}].first.sigma_min = 0.0001;
    // params[{0,0}].first.sigma_max = 0.01;

    // params[{0,0}].first.sigmaR = 0.0005;
    // params[{0,0}].first.sigmaR_min = 0.00001;
    // params[{0,0}].first.sigmaR_max = 0.01;
    // params[{0,0}].first.sigmaL = 0.0005;
    // params[{0,0}].first.sigmaL_min = 0.00001;
    // params[{0,0}].first.sigmaL_max = 0.01;
    
    // params[{0,0}].first.alphaL = 1.1;
    // params[{0,0}].first.alphaL_min = 0.5;
    // params[{0,0}].first.alphaL_max = 2;
    // params[{0,0}].first.nL = 1.1;
    // params[{0,0}].first.nL_min = 1;
    // params[{0,0}].first.nL_max = 200;

    // params[{0,0}].first.alphaR = 1;
    // params[{0,0}].first.alphaR_min = 0.5;
    // params[{0,0}].first.alphaR_max = 2;
    // params[{0,0}].first.nR = 1.1;
    // params[{0,0}].first.nR_min = 1;
    // params[{0,0}].first.nR_max = 20;
     //     params[{0,0}].second.p0 = 0.01;
    //     params[{0,0}].second.p0_min = 0.001;
    //     params[{0,0}].second.p0_max = 1;
    //     params[{0,0}].second.p1 = 1;
    //     params[{0,0}].second.p1_min = -20 ;
    //     params[{0,0}].second.p1_max = 20.;
    //     params[{0,0}].second.p2 = 1;
    //     params[{0,0}].second.p2_min = -20;
    //     params[{0,0}].second.p2_max = 20; 
    // }

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
    // DoubleDBCrystalBallParams value;
    // value.mean1 = 0.1455;        // D* mass in GeV
    // value.mean2 = 0.1455;        // D* mass in GeV
    // value.mean_min = 0.1452;
    // value.mean_max = 0.1458;
    // value.sigmaR1 = 0.0005;
    // value.sigmaR2 = 0.0005;
    // value.sigmaR_min = 0.0001;
    // value.sigmaR_max = 0.01;
    // value.sigmaL1 = 0.0005;
    // value.sigmaL2 = 0.0005;
    // value.sigmaL_min = 0.0001;
    // value.sigmaL_max = 0.01;
    
    // value.alphaL1 = 2.;
    // value.alphaL2 = 2.;
    // value.alphaL_min = 0.005;
    // value.alphaL_max = 10;
    // value.nL1 = 1.5;
    // value.nL2 = 1.5;
    // value.nL_min = 1;
    // value.nL_max = 10;

    // value.alphaR1 = 2.;
    // value.alphaR2 = 2.;
    // value.alphaR_min = 0.005;
    // value.alphaR_max = 20.;
    // value.nR1 = 1.5;
    // value.nR2 = 1.5;
    // value.nR_min = 1;
    // value.nR_max = 20;

    // std::string subDir = "/DStar_ppRef_Jun12_v1/";
/// fitting parameter tunning ///
if(pTMax<=8){
        nsig_ratio = 0.1;        
        nsig_min_ratio = 0.01;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.5;       
        nbkg_min_ratio = 0.01;
        nbkg_max_ratio = 1.0;
       params2[{0,0}].first.sigma1 = 0.0005;
        params2[{0,0}].first.sigma1_min = 0.00001;
        params2[{0,0}].first.sigma1_max = 0.005;
        params2[{0,0}].first.sigma2 = 0.0005;
        params2[{0,0}].first.sigma2_min = 0.00001;
        params2[{0,0}].first.sigma2_max = 0.001;
        params2[{0,0}].first.fraction = 0.2;
        params2[{0,0}].first.fraction_min = 0.1;
        params2[{0,0}].first.fraction_max = 1.0;
         params2[{0,0}].first.sigma1 = 0.0005;
//         params2[{0,0}].first.sigma1_min = 0.00001;
//         params2[{0,0}].first.sigma1_max = 0.005;
//         params2[{0,0}].first.sigma2 = 0.001;
//         params2[{0,0}].first.sigma2_min = 0.00001;
//         params2[{0,0}].first.sigma2_max = 0.01;
//         params2[{0,0}].first.fraction = 0.4;
//         params2[{0,0}].first.fraction_min = 0.2;
//         params2[{0,0}].first.fraction_max = 1.0;
}
if(pTMax==17 && pTMin ==15){
        nsig_ratio = 0.7;        
        nsig_min_ratio = 0.01;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1;       
        nbkg_min_ratio = 0.01;
        nbkg_max_ratio = 0.5;
}if(pTMax==13 && pTMin ==11){
        nsig_ratio = 0.7;        
        nsig_min_ratio = 0.01;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1;       
        nbkg_min_ratio = 0.01;
        nbkg_max_ratio = 0.5;
}
if(pTMax==11 && pTMin ==9){
        nsig_ratio = 0.7;        
        nsig_min_ratio = 0.01;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1;       
        nbkg_min_ratio = 0.01;
        nbkg_max_ratio = 0.5;
}if(pTMax==20 && pTMin ==17){
        nsig_ratio = 0.7;        
        nsig_min_ratio = 0.01;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1;       
        nbkg_min_ratio = 0.01;
        nbkg_max_ratio = 0.5;
}if(pTMax==23 && pTMin ==20){
        nsig_ratio = 0.7;        
        nsig_min_ratio = 0.1;
        nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1;       
        nbkg_min_ratio = 0.05;
        nbkg_max_ratio = 0.3;
}
/// ### DStar OO Fit Macro ### ///
    for(auto ptbin : ptBins){
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            // D0opt.cosMin = cosbin.first;
            // D0opt.cosMax = cosbin.second;
            std::string subDir = "/DStar_OO_RAA_Jul24_v1/";
            
            D0opt.DStarOODefault();
            D0opt.setSubDir(subDir);
            D0opt.cutMCExpr = D0opt.cutMCExpr +"&&"+ EtaCut;
            D0opt.cutExpr = D0opt.cutExpr + "&&"+EtaCut;
            
            if(doReFit){
                 MassFitter fitter(D0opt.name, D0opt.massVar, D0opt.massMin, D0opt.massMax,nsig_ratio, nsig_min_ratio, nsig_max_ratio, nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio);
                D0opt.pTMax <= 8 ? fitter.PerformConstraintFit(D0opt,Dataloaderoo.getDataSet(),MCloader.getDataSet(), true, "","",params2[{0,0}].first,params2[{0,0}].second) : fitter.PerformConstraintFit(D0opt,Dataloaderoo.getDataSet(),MCloader.getDataSet(), true, "","",params[{0,0}].first,params[{0,0}].second);
                // fitter.PerformFit(D0opt,Dataloader.getDataSet(), true, "","",params[{0,0}].first,params[{0,0}].second);
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
//// ## DStar ppRef Macro ## ////
    for(auto ptbin : ptBins){
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            // D0opt.cosMin = cosbin.first;
            // D0opt.cosMax = cosbin.second;
            std::string subDir = "/DStar_ppRef_RAA_Jul24_v1/";
            
            D0opt.DStarPPDefault();
            D0opt.setSubDir(subDir);
            D0opt.cutMCExpr = D0opt.cutMCExpr +"&&"+ EtaCut;
            D0opt.cutExpr = D0opt.cutExpr + "&&"+EtaCut;
            
            if(doReFit){
                 MassFitter fitter(D0opt.name, D0opt.massVar, D0opt.massMin, D0opt.massMax,nsig_ratio, nsig_min_ratio, nsig_max_ratio, nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio);
                D0opt.pTMax <= 100 ? fitter.PerformConstraintFit(D0opt,Dataloaderpp.getDataSet(),MCloader.getDataSet(), true, "","",params2[{0,0}].first,params2[{0,0}].second) : fitter.PerformConstraintFit(D0opt,Dataloaderpp.getDataSet(),MCloader.getDataSet(), true, "","",params[{0,0}].first,params[{0,0}].second);
                // fitter.PerformFit(D0opt,Dataloader.getDataSet(), true, "","",params[{0,0}].first,params[{0,0}].second);
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

    // D0opt.massVar = "massDaugther1"; // MassFitter에 전달할 질량 변수 이름 설정
    // D0opt.massMin = 1.75;
    // D0opt.massMax = 2.00;
    // DCAFitter DCAfitter(D0opt,"dcaFitter",D0opt.massVar, D0opt.dcaMin, D0opt.dcaMax, 100);

    // DCAfitter.setMCFile((D0opt.outputMCDir+"/"+D0opt.outputMCFile).c_str(), "reducedData");
    // DCAfitter.setDataFile((D0opt.outputDir+"/"+D0opt.outputFile).c_str(), "reducedData"); // 데이터 파일과 트리 이름 설정
    // DCAfitter.setDCABranchName(D0opt.dcaVar);
    // DCAfitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_"); // MC에서만 사용됨
    // DCAfitter.setPromptPdgIds({4,2});
    // DCAfitter.setNonPromptPdgIds({5});

    // std::string commonCuts = "abs(y) < 1.0";
    // DCAfitter.setMCCuts(commonCuts);
    // DCAfitter.setDataCuts(commonCuts);
    



    // --- 결과 저장 파일 설정 ---
    // DCAfitter.setOutputFile(D0opt.outputDir+"DCAFit_Results.root");
    // std::cout << "Step 1: Creating templates from MC..." << std::endl;
    // if (DCAfitter.createTemplatesFromMC()) {
    //     DCAfitter.plotRawDataDistribution(D0opt.outputPlotDir+D0opt.outputDCAFile+"_MC_Templates_Normalized");

    //     std::cout << "\nStep 2: Loading data..." << std::endl;
    //     if (DCAfitter.loadData()) {

    //         std::cout << "\nStep 3: Building the fit model..." << std::endl;
    //         if (DCAfitter.buildModelwSideband()) {

    //             std::cout << "\nStep 4: Performing the fit..." << std::endl;
    //             // fitTo()가 RooFitResult 포인터를 반환하므로, 사용 후 메모리를 해제해야 합니다.
    //             RooFitResult* fitResult = DCAfitter.performFit(true); // true: MINOS 에러 계산 사용

    //             if (fitResult) {
    //                 std::cout << "\nStep 5: Plotting and saving results..." << std::endl;
    //                 // 피팅 결과를 시각화
    //                 DCAfitter.plotResults(fitResult,D0opt.outputPlotDir+D0opt.outputDCAFile+"_DCA_Fit_Result_Plot");

    //                 // 워크스페이스, 피팅 결과, 그림 등을 ROOT 파일에 저장
    //                 DCAfitter.saveResults(fitResult);

    //                 // performFit()에서 동적으로 할당된 RooFitResult 객체의 메모리 해제
    //                 delete fitResult;
                    
    //                 std::cout << "\nAnalysis finished successfully!" << std::endl;
    //             } else {
    //                 std::cerr << "Fit failed to produce a result." << std::endl;
    //             }
    //         } else {
    //             std::cerr << "Failed to build the model." << std::endl;
    //         }
    //     } else {
    //         std::cerr << "Failed to load data." << std::endl;
    //     }
    // } else {
    //     std::cerr << "Failed to create templates from MC." << std::endl;
    // }
    // }
    
    }
    // printAllFitResultsInDir(D0opt.outputDir);
    // }







