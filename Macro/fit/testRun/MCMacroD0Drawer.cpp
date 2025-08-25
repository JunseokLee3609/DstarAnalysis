#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager_test.h"
#include "../DataLoader.h"
// #include "../ParameterManager.h"

using ParamKey = std::pair<double, double>;
using D0ParamValue = std::pair<PDFParams::GaussianParams, PDFParams::PolynomialBkgParams>;
using D0MCParamValue = std::pair<PDFParams::DoubleGaussianParams, PDFParams::GaussianParams>;
using DStarParamValue1 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarParamValue2 = std::pair<PDFParams::DoubleDBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarMCParamValue = std::pair<PDFParams::DBCrystalBallParams, PDFParams::DBCrystalBallParams>;
using D0ParamMap = std::map<ParamKey, D0ParamValue>;
using D0MCParamMap = std::map<ParamKey, D0MCParamValue>;
using DStarParamMap1 = std::map<ParamKey, DStarParamValue1>;
using DStarParamMap2 = std::map<ParamKey, DStarParamValue2>;
using DStarMCParamMap = std::map<ParamKey, DStarMCParamValue>;
using D0MCBkgParamMap = std::map<ParamKey, PDFParams::PolynomialBkgParams>;

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


D0MCBkgParamMap D0BkgParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins, int order) {
    D0MCBkgParamMap params;
            
    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            PDFParams::PolynomialBkgParams value;
            double polyinit = 0.;
            double polymax = 0.5;
            double polymin = -0.5;
            for(int k =0; k < order; k++) {
                value.coefficients.push_back(polyinit);
                value.coef_max.push_back(polymax);
                value.coef_min.push_back(polymin);
            }
            params[key] = value;
        }
    }
            
    return params;
}

void MCMacroD0Drawer(bool doReFit= false, bool plotFit = true, bool useCUDA=true, float pTMin=2, float pTMax=100, float centMin=0, float centMax=100){
    // std::string subDir = "/D0_May03_inclusive/";
    std::string subDir = "/D0_May14_MC_pol1_v1/";
    // std::string subDir = "/D0_Apr30/";
    FitOpt D0opt;
    D0opt.useCUDA = useCUDA;
    D0opt.doFit = doReFit;
    cout << D0opt.mvaMin << endl; 
    std::string particleType = "D0";
    
    std::vector<std::pair<double,double>> ptBins = {
         {2,4},
         {4,9},
         {9,50}
        //  {0,50},
        // {10,12.5},
        // {12.5,15},
        // {15,20},
        // {20,25},
        // {25,30},
        // {30,40},
        // {40,100}
        //{2,4},
        //{4,9},
        //{9,50}
    };
    std::vector<std::pair<double,double>> centBins = {
        {0,10},
        {10,40},
        {40,90}

    
     
        // {50,100}
        // {0,90}
    };
    std::vector<std::pair<double,double>> iptBins = {
        {2,50},
        // {4,9},
        // {9,50}
       //  {0,50},
       // {10,12.5},
       // {12.5,15},
       // {15,20},
       // {20,25},
       // {25,30},
       // {30,40},
       // {40,100}
       //{2,4},
       //{4,9},
       //{9,50}
   };
   std::vector<std::pair<double,double>> icentBins = {
       {0,90},
    //    {10,40},
    //    {40,90}
       // {50,100}
       // {0,90}
   };
     std::vector<double> mvaBin = {0.8,0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    //  std::vector<double> mvaBin = {0.999};
    //  std::vector<double> mvaBin = {0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    //  std::vector<double> mvaBin = {0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98};
    //  std::vector<double> pTBin1 = {2,4,9,50};
     std::vector<double> pTBin1;
     for(auto ptbin : ptBins){
         pTBin1.push_back(ptbin.first);
         if(ptbin==ptBins.back()){
             pTBin1.push_back(ptbin.second);
         }
     }
     std::vector<double> centBin1;
     for(auto centbin : centBins){
         centBin1.push_back(centbin.first);
         if(centbin==centBins.back()){
             centBin1.push_back(centbin.second);
         }
     }

     std::vector<double> ipTBin1;
     for(auto ptbin : iptBins){
         ipTBin1.push_back(ptbin.first);
         if(ptbin==ptBins.back()){
             ipTBin1.push_back(ptbin.second);
         }
     }
     std::vector<double> icentBin1;
     for(auto centbin : icentBins){
         icentBin1.push_back(centbin.first);
         if(centbin==icentBins.back()){
             icentBin1.push_back(centbin.second);
         }
     }

    //  std::vector<double> mvaBin1 = {0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999,1.000};
     std::vector<double> mvaBin1 = {0.8,0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999,1.000};
    // std::vector<double> mvaBin = {0.990};
    // std::string filepath = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_D0DATAMVA.root";
    // std::string filepathMC = "/home/jun502s/DstarAna/DstarAna_real/DDbarpPb/data/DstarMC_Rds_PbPb_250110.root";
    //std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_PbPb_ONNX_withCent.root";
    std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_PbPb_ONNX_withCent_wNcoll_May02.root";
    // std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_PbPb_ONNX_withCent.root";
    //std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_PbPb_ONNX_withCent_Apr24.root";
    std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_PbPb_ONNX_withCent_wNcoll_May02.root";
    // std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_PbPb_ONNX_withCent_Apr30.root";
    //std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_PbPb_ONNX_withCent.root";
    std::string datasetName = "dataset";
    auto params = D0MCParamMaker({0},{0});
    auto params2 = D0BkgParamMaker({0},{0},1);
    DataLoader MCloader(filepathMC);
    DataLoader Dataloader(filepathData);
    MCloader.loadRooDataSet(datasetName);
    Dataloader.loadRooDataSet(datasetName);
    // RooDataSet* dataset = (RooDataSet*)TFile::Open(filepath.c_str())->Get(datasetName.c_str());
    // fitter.SetData(dataset);
    // fitter.ApplyCut(D0opt.cutExpr);
    // for(auto mva : mvabin){
    // for(auto mva : mvabin){
TH1D* h1D = new TH1D("h1D","D^{0} Yield; MVA Score",
    mvaBin1.size()-1, &mvaBin1[0]);
TH1D* h1D_SB = new TH1D("h1D_SB","D^{0} Signal/Background; MVA Score; S/B",
    mvaBin1.size()-1, &mvaBin1[0]); 
TH2D* h2D_cent = new TH2D("h2D_cent","D^{0} Yield; MVA Score", mvaBin1.size()-1, &mvaBin1[0],centBin1.size()-1, &centBin1[0]
);
TH2D* h2D_cent_SB = new TH2D("h2D_cent_SB","D^{0} Signal/Background; MVA Score; S/B", 
    mvaBin1.size()-1, &mvaBin1[0], centBin1.size()-1, &centBin1[0]); 

TH2D* h2D = new TH2D("h2D","D^{0} Yield; MVA Score", mvaBin1.size()-1, &mvaBin1[0],icentBin1.size()-1, &icentBin1[0]
);
TH2D* h2D_SB = new TH2D("h2D_SB","D^{0} Signal/Background; MVA Score; S/B", 
    mvaBin1.size()-1, &mvaBin1[0], icentBin1.size()-1, &icentBin1[0]); 

TH2D* h2D_pt = new TH2D("h2D_pt","D^{0} Yield; MVA Score", mvaBin1.size()-1, &mvaBin1[0],pTBin1.size()-1, &pTBin1[0]
);
TH2D* h2D_pt_SB = new TH2D("h2D_pt_SB","D^{0} Signal/Background; MVA Score; S/B", 
    mvaBin1.size()-1, &mvaBin1[0], pTBin1.size()-1, &pTBin1[0]); 


int imva=0;
for(auto mva : mvaBin){
int ipT=0;
    for(auto ptbin : ptBins){
        for(auto centbin : centBins){
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            D0opt.centMin = 0;
            D0opt.centMax = 90;
            D0opt.mvaMin = mva;
            D0opt.D0MCDefault();
            D0opt.outputDir = D0opt.outputDir + subDir;
            D0opt.outputMCDir = D0opt.outputMCDir + subDir;
	   if(doReFit){
             MassFitter fitter(D0opt.name,D0opt.massVar,D0opt.massMin,D0opt.massMax);
             fitter.PerformConstraintFit(D0opt,Dataloader.getDataSet(),MCloader.getDataSet(), true, "","", params[{0,0}].first,params[{0,0}].second, params2[{0,0}] );
       }

             //fitter.PlotResult(true,D0opt.outputFile);
             //PlotManager plotManagerMC1(D0opt,D0opt.outputMCDir,D0opt.outputMCFile,"plots/MC/DBCrystalBall");
            std::string fullPath = D0opt.outputDir;

            if (!fullPath.empty() && fullPath.back() != '/')
                fullPath += '/';

            fullPath += D0opt.outputFile;
            // cout << "Full path: " << fullPath << endl;

            TFile *file = TFile::Open(fullPath.c_str());
            if (file && !file->IsZombie())
            {
                // Get signal yield from TParameter (assuming it's saved this way)
                TParameter<double> *pureYieldValParam = dynamic_cast<TParameter<double> *>(file->Get("nSigPure_Value"));
                TParameter<double> *pureYieldErrParam = dynamic_cast<TParameter<double> *>(file->Get("nSigPure_Error"));

                // Get RooFitResult to extract background yield
                // *** Adjust the name "fitResult" if MassFitter saves it with a different name ***
                RooFitResult *fitResult = dynamic_cast<RooFitResult *>(file->Get("fitResult"));

                double sigVal = 0.0, sigErr = 0.0;
                double bkgVal = 0.0, bkgErr = 0.0;
                bool sigOk = false, bkgOk = false;

                if (pureYieldValParam && pureYieldErrParam) {
                    sigVal = pureYieldValParam->GetVal();
                    sigErr = pureYieldErrParam->GetVal();
                    h2D_pt->SetBinContent(imva + 1, ipT +1, sigVal);
                    h2D_pt->SetBinError(imva + 1, ipT+1, sigErr);
                    sigOk = true;
                } else {
                     std::cout << "Warning: Could not retrieve signal yield TParameters from file: " << fullPath << std::endl;
                     h2D_pt->SetBinContent(imva + 1, ipT +1, 0);
                     h2D_pt->SetBinError(imva + 1, ipT+1, 0);
                }

                if (fitResult) {
                    // Find the background yield parameter (nBkg) in the fit result
                    // *** Adjust the name "nBkg" if the parameter has a different name in your fit model ***
                    RooRealVar *nBkgVar = dynamic_cast<RooRealVar *>(fitResult->floatParsFinal().find("nbkg"));
                    if (nBkgVar) {
                        bkgVal = nBkgVar->getVal();
                        bkgErr = nBkgVar->getError();
                        bkgOk = true;
                    } else {
                        std::cout << "Warning: Could not find 'nBkg' RooRealVar in RooFitResult from file: " << fullPath << std::endl;
                    }
                } else {
                     std::cout << "Warning: Could not retrieve RooFitResult object ('fitResult') from file: " << fullPath << std::endl;
                }

                // Calculate and fill S/B histogram if both signal and background were retrieved
                if (sigOk && bkgOk) {
                     if (bkgVal > 0) { // Avoid division by zero
                        double sbRatio = sigVal / bkgVal;
                        // Error propagation
                        double sbErr = 0.0;
                        if (sigVal != 0 && bkgVal != 0) { // Avoid division by zero in error calculation
                             sbErr = sbRatio * std::sqrt(std::pow(sigErr / sigVal, 2) + std::pow(bkgErr / bkgVal, 2));
                        }
                        h2D_pt_SB->SetBinContent(imva + 1, ipT+1, sbRatio);
                        h2D_pt_SB->SetBinError(imva + 1, ipT+1, sbErr);
                    } else {
                        h2D_pt_SB->SetBinContent(imva + 1,ipT+1, 0); // Set S/B to 0 if background is zero
                        h2D_pt_SB->SetBinError(imva + 1,ipT+1, 0);
                    }
                } else {
                    // If either signal or background failed, set S/B to 0
                    h2D_pt_SB->SetBinContent(imva + 1,ipT+1, 0);
                    h2D_pt_SB->SetBinError(imva + 1,ipT+1, 0);
                }



                file->Close();
                delete file; // file owns fitResult, no need to delete fitResult separately
            }
            else
            {
                std::cout << "Could not open file: " << fullPath << std::endl;
            }
            // PlotManager plotManagerMCSwap0(D0opt, D0opt.outputMCDir, D0opt.outputMCSwap0File, "plots/MC_D0_PbPb/Swap0"+subDir);
            // PlotManager plotManagerMCSwap1(D0opt, D0opt.outputMCDir, D0opt.outputMCSwap1File, "plots/MC_D0_PbPb/Swap1"+subDir);
            // PlotManager plotManager(D0opt, D0opt.outputDir, D0opt.outputFile, "plots/Data_D0_PbPb/"+subDir);
            // if (!plotFit)
            // {
            //     // plotManagerMC1.DrawRawDistribution();
            //     // plotManagerMC2.DrawRawDistribution();
            //     plotManagerMCSwap0.DrawRawDistribution();
            //     plotManagerMCSwap1.DrawRawDistribution();
            //     plotManager.DrawRawDistribution();
            // }
            // else
            // {
                
            //     // plotManagerMC1.DrawFittedModel(true);
            //     // plotManagerMC2.DrawFittedModel(true);
            //     plotManagerMCSwap0.DrawFittedModel(true);
            //     plotManagerMCSwap1.DrawFittedModel(true);
            //     plotManager.DrawFittedModel(true);
            // }
        }
        ipT++;
    }
    imva++;
}
imva=0;
for(auto mva : mvaBin){
int icent=0;
    for(auto ptbin : ptBins){
        for(auto centbin : centBins){
            D0opt.pTMin = 2;
            D0opt.pTMax = 50;
            D0opt.centMin = centbin.first;
            D0opt.centMax = centbin.second;
            D0opt.mvaMin = mva;
            D0opt.D0MCDefault();
            D0opt.outputDir = D0opt.outputDir + subDir;
            D0opt.outputMCDir = D0opt.outputMCDir + subDir;
	   if(doReFit){
             MassFitter fitter(D0opt.name,D0opt.massVar,D0opt.massMin,D0opt.massMax);
             fitter.PerformConstraintFit(D0opt,Dataloader.getDataSet(),MCloader.getDataSet(), true, "","", params[{0,0}].first,params[{0,0}].second, params2[{0,0}] );
       }

             //fitter.PlotResult(true,D0opt.outputFile);
             //PlotManager plotManagerMC1(D0opt,D0opt.outputMCDir,D0opt.outputMCFile,"plots/MC/DBCrystalBall");
            std::string fullPath = D0opt.outputDir;

            if (!fullPath.empty() && fullPath.back() != '/')
                fullPath += '/';

            fullPath += D0opt.outputFile;
            // cout << "Full path: " << fullPath << endl;

            TFile *file = TFile::Open(fullPath.c_str());
            if (file && !file->IsZombie())
            {
                // Get signal yield from TParameter (assuming it's saved this way)
                TParameter<double> *pureYieldValParam = dynamic_cast<TParameter<double> *>(file->Get("nSigPure_Value"));
                TParameter<double> *pureYieldErrParam = dynamic_cast<TParameter<double> *>(file->Get("nSigPure_Error"));

                // Get RooFitResult to extract background yield
                // *** Adjust the name "fitResult" if MassFitter saves it with a different name ***
                RooFitResult *fitResult = dynamic_cast<RooFitResult *>(file->Get("fitResult"));

                double sigVal = 0.0, sigErr = 0.0;
                double bkgVal = 0.0, bkgErr = 0.0;
                bool sigOk = false, bkgOk = false;

                if (pureYieldValParam && pureYieldErrParam) {
                    sigVal = pureYieldValParam->GetVal();
                    sigErr = pureYieldErrParam->GetVal();
                    h2D_cent->SetBinContent(imva + 1, icent +1, sigVal);
                    h2D_cent->SetBinError(imva + 1, icent+1, sigErr);
                    sigOk = true;
                } else {
                     std::cout << "Warning: Could not retrieve signal yield TParameters from file: " << fullPath << std::endl;
                     h2D_cent->SetBinContent(imva + 1, icent +1, 0);
                     h2D_cent->SetBinError(imva + 1, icent+1, 0);
                }

                if (fitResult) {
                    // Find the background yield parameter (nBkg) in the fit result
                    // *** Adjust the name "nBkg" if the parameter has a different name in your fit model ***
                    RooRealVar *nBkgVar = dynamic_cast<RooRealVar *>(fitResult->floatParsFinal().find("nbkg"));
                    if (nBkgVar) {
                        bkgVal = nBkgVar->getVal();
                        bkgErr = nBkgVar->getError();
                        bkgOk = true;
                    } else {
                        std::cout << "Warning: Could not find 'nBkg' RooRealVar in RooFitResult from file: " << fullPath << std::endl;
                    }
                } else {
                     std::cout << "Warning: Could not retrieve RooFitResult object ('fitResult') from file: " << fullPath << std::endl;
                }

                // Calculate and fill S/B histogram if both signal and background were retrieved
                if (sigOk && bkgOk) {
                     if (bkgVal > 0) { // Avoid division by zero
                        double sbRatio = sigVal / bkgVal;
                        // Error propagation
                        double sbErr = 0.0;
                        if (sigVal != 0 && bkgVal != 0) { // Avoid division by zero in error calculation
                             sbErr = sbRatio * std::sqrt(std::pow(sigErr / sigVal, 2) + std::pow(bkgErr / bkgVal, 2));
                        }
                        h2D_cent_SB->SetBinContent(imva + 1, icent+1, sbRatio);
                        h2D_cent_SB->SetBinError(imva + 1, icent+1, sbErr);
                    } else {
                        h2D_cent_SB->SetBinContent(imva + 1,icent+1, 0); // Set S/B to 0 if background is zero
                        h2D_cent_SB->SetBinError(imva + 1,icent+1, 0);
                    }
                } else {
                    // If either signal or background failed, set S/B to 0
                    h2D_cent_SB->SetBinContent(imva + 1,icent+1, 0);
                    h2D_cent_SB->SetBinError(imva + 1,icent+1, 0);
                }



                file->Close();
                delete file; // file owns fitResult, no need to delete fitResult separately
            }
            else
            {
                std::cout << "Could not open file: " << fullPath << std::endl;
            }
            // PlotManager plotManagerMCSwap0(D0opt, D0opt.outputMCDir, D0opt.outputMCSwap0File, "plots/MC_D0_PbPb/Swap0"+subDir);
            // PlotManager plotManagerMCSwap1(D0opt, D0opt.outputMCDir, D0opt.outputMCSwap1File, "plots/MC_D0_PbPb/Swap1"+subDir);
            // PlotManager plotManager(D0opt, D0opt.outputDir, D0opt.outputFile, "plots/Data_D0_PbPb/"+subDir);
            // if (!plotFit)
            // {
            //     // plotManagerMC1.DrawRawDistribution();
            //     // plotManagerMC2.DrawRawDistribution();
            //     plotManagerMCSwap0.DrawRawDistribution();
            //     plotManagerMCSwap1.DrawRawDistribution();
            //     plotManager.DrawRawDistribution();
            // }
            // else
            // {
                
            //     // plotManagerMC1.DrawFittedModel(true);
            //     // plotManagerMC2.DrawFittedModel(true);
            //     plotManagerMCSwap0.DrawFittedModel(true);
            //     plotManagerMCSwap1.DrawFittedModel(true);
            //     plotManager.DrawFittedModel(true);
            // }
        icent++;
        }
    }
    imva++;
}
imva=0;
for(auto mva : mvaBin){
int icent=0;
    for(auto ptbin : iptBins){
        for(auto centbin : icentBins){
            D0opt.pTMin = 2;
            D0opt.pTMax = 50;
            D0opt.centMin = centbin.first;
            D0opt.centMax = centbin.second;
            D0opt.mvaMin = mva;
            D0opt.D0MCDefault();
            D0opt.outputDir = D0opt.outputDir + subDir;
            D0opt.outputMCDir = D0opt.outputMCDir + subDir;
	   if(doReFit){
             MassFitter fitter(D0opt.name,D0opt.massVar,D0opt.massMin,D0opt.massMax);
             fitter.PerformConstraintFit(D0opt,Dataloader.getDataSet(),MCloader.getDataSet(), true, "","", params[{0,0}].first,params[{0,0}].second, params2[{0,0}] );
       }

             //fitter.PlotResult(true,D0opt.outputFile);
             //PlotManager plotManagerMC1(D0opt,D0opt.outputMCDir,D0opt.outputMCFile,"plots/MC/DBCrystalBall");
            std::string fullPath = D0opt.outputDir;

            if (!fullPath.empty() && fullPath.back() != '/')
                fullPath += '/';

            fullPath += D0opt.outputFile;
            // cout << "Full path: " << fullPath << endl;

            TFile *file = TFile::Open(fullPath.c_str());
            if (file && !file->IsZombie())
            {
                // Get signal yield from TParameter (assuming it's saved this way)
                TParameter<double> *pureYieldValParam = dynamic_cast<TParameter<double> *>(file->Get("nSigPure_Value"));
                TParameter<double> *pureYieldErrParam = dynamic_cast<TParameter<double> *>(file->Get("nSigPure_Error"));

                // Get RooFitResult to extract background yield
                // *** Adjust the name "fitResult" if MassFitter saves it with a different name ***
                RooFitResult *fitResult = dynamic_cast<RooFitResult *>(file->Get("fitResult"));

                double sigVal = 0.0, sigErr = 0.0;
                double bkgVal = 0.0, bkgErr = 0.0;
                bool sigOk = false, bkgOk = false;

                if (pureYieldValParam && pureYieldErrParam) {
                    sigVal = pureYieldValParam->GetVal();
                    sigErr = pureYieldErrParam->GetVal();
                    h2D->SetBinContent(imva + 1, icent +1, sigVal);
                    h2D->SetBinError(imva + 1, icent+1, sigErr);
                    sigOk = true;
                } else {
                     std::cout << "Warning: Could not retrieve signal yield TParameters from file: " << fullPath << std::endl;
                     h2D->SetBinContent(imva + 1, icent +1, 0);
                     h2D->SetBinError(imva + 1, icent+1, 0);
                }

                if (fitResult) {
                    // Find the background yield parameter (nBkg) in the fit result
                    // *** Adjust the name "nBkg" if the parameter has a different name in your fit model ***
                    RooRealVar *nBkgVar = dynamic_cast<RooRealVar *>(fitResult->floatParsFinal().find("nbkg"));
                    if (nBkgVar) {
                        bkgVal = nBkgVar->getVal();
                        bkgErr = nBkgVar->getError();
                        bkgOk = true;
                    } else {
                        std::cout << "Warning: Could not find 'nBkg' RooRealVar in RooFitResult from file: " << fullPath << std::endl;
                    }
                } else {
                     std::cout << "Warning: Could not retrieve RooFitResult object ('fitResult') from file: " << fullPath << std::endl;
                }

                // Calculate and fill S/B histogram if both signal and background were retrieved
                if (sigOk && bkgOk) {
                     if (bkgVal > 0) { // Avoid division by zero
                        double sbRatio = sigVal / bkgVal;
                        // Error propagation
                        double sbErr = 0.0;
                        if (sigVal != 0 && bkgVal != 0) { // Avoid division by zero in error calculation
                             sbErr = sbRatio * std::sqrt(std::pow(sigErr / sigVal, 2) + std::pow(bkgErr / bkgVal, 2));
                        }
                        h2D_SB->SetBinContent(imva + 1, icent+1, sbRatio);
                        h2D_SB->SetBinError(imva + 1, icent+1, sbErr);
                    } else {
                        h2D_SB->SetBinContent(imva + 1,icent+1, 0); // Set S/B to 0 if background is zero
                        h2D_SB->SetBinError(imva + 1,icent+1, 0);
                    }
                } else {
                    // If either signal or background failed, set S/B to 0
                    h2D_SB->SetBinContent(imva + 1,icent+1, 0);
                    h2D_SB->SetBinError(imva + 1,icent+1, 0);
                }



                file->Close();
                delete file; // file owns fitResult, no need to delete fitResult separately
            }
            else
            {
                std::cout << "Could not open file: " << fullPath << std::endl;
            }
            // PlotManager plotManagerMCSwap0(D0opt, D0opt.outputMCDir, D0opt.outputMCSwap0File, "plots/MC_D0_PbPb/Swap0"+subDir);
            // PlotManager plotManagerMCSwap1(D0opt, D0opt.outputMCDir, D0opt.outputMCSwap1File, "plots/MC_D0_PbPb/Swap1"+subDir);
            // PlotManager plotManager(D0opt, D0opt.outputDir, D0opt.outputFile, "plots/Data_D0_PbPb/"+subDir);
            // if (!plotFit)
            // {
            //     // plotManagerMC1.DrawRawDistribution();
            //     // plotManagerMC2.DrawRawDistribution();
            //     plotManagerMCSwap0.DrawRawDistribution();
            //     plotManagerMCSwap1.DrawRawDistribution();
            //     plotManager.DrawRawDistribution();
            // }
            // else
            // {
                
            //     // plotManagerMC1.DrawFittedModel(true);
            //     // plotManagerMC2.DrawFittedModel(true);
            //     plotManagerMCSwap0.DrawFittedModel(true);
            //     plotManagerMCSwap1.DrawFittedModel(true);
            //     plotManager.DrawFittedModel(true);
            // }
        icent++;
        }
    }
    imva++;
}
TCanvas* c1 = new TCanvas("c1", "D0 Yields", 800, 600);
gPad->SetRightMargin(0.15);
h2D_cent->SetStats(0);
h2D_cent->Draw("colztext");
c1->SaveAs(Form("%sD0_Yield_mva_cent.pdf",D0opt.outputDir.c_str()));

// S/B 플롯 그리기 및 저장
TCanvas* c2 = new TCanvas("c2", "D0 S/B Ratio", 800, 600);
gPad->SetRightMargin(0.15);
h2D_cent_SB->SetStats(0);
h2D_cent_SB->Draw("colztext");
// h2D_cent_SB->SaveAs(Form("%sD0_SB_Ratio_mva_cent.pdf",D0opt.outputDir.c_str()));
// c2->SaveAs(Form("%sD0_SB_Ratio_mva_cent.png",D0opt.outputDir.c_str()));

TCanvas* c3 = new TCanvas("c4", "D0 Yields", 800, 600);
gPad->SetRightMargin(0.15);
h2D_pt->SetStats(0);
h2D_pt->Draw("colztext");
c3->SaveAs(Form("%sD0_Yield_mva_pt.pdf",D0opt.outputDir.c_str()));

// S/B 플롯 그리기 및 저장
TCanvas* c4 = new TCanvas("c4", "D0 S/B Ratio", 800, 600);
gPad->SetRightMargin(0.15);
h2D_pt_SB->SetStats(0);
h2D_pt_SB->Draw("colztext");
// h2D_pt_SB->SaveAs(Form("%sD0_SB_Ratio_mva_pt.pdf",D0opt.outputDir.c_str()));
// c4->SaveAs(Form("%sD0_SB_Ratio_mva_pt.png",D0opt.outputDir.c_str()));

TCanvas* c5 = new TCanvas("c5", "D0 Yields", 800, 600);
gPad->SetRightMargin(0.15);
h2D->SetStats(0);
h2D->Draw("colztext");
c5->SaveAs(Form("%sD0_Yield_mva.pdf",D0opt.outputDir.c_str()));

TFile *fout = new TFile(Form("%sD0_Yield_mva.root",D0opt.outputDir.c_str()), "RECREATE");
fout->cd();
h2D_cent->Write("D0_Yield_mva_cent");
h2D_cent_SB->Write("D0_SB_Ratio_mva_cent"); // S/B 히스토그램 저장
h2D_pt->Write("D0_Yield_mva_pt");
h2D_pt_SB->Write("D0_SB_Ratio_mva_pt"); // S/B 히스토그램 저장
h2D->Write("D0_Yield_mva");
h2D_SB->Write("D0_SB_Ratio_mva"); // S/B 히스토그램 저장
fout->Close();
std::cout << "Results saved." << std::endl;
}
