#include "MassFitter_copy.h"
#include <vector>
#include <iostream>

void FitDstar() {
    // Define pT bins
    std::vector<std::pair<double, double>> pTBins = {
        // {4.0, 5.0},
        // {5.0, 6.0},
        // {6.0, 7.0},
        {12.0, 50.0},
        // {8.0,9.0},
        // {9.0,10.0},
        // {10.0,12.0},
        // {12.0,16.0},
        // {16.0,20.0},
        // {20.0,30.0},
        // {30.0,50.0},
        // {50.0,100.0}
        // Add more bins as needed
    };

    // Create fitter instance
    MassFitter fitter_dstar("PbPb");
    // MassFitter fitter("PbPb");
    // MassFitter fitter_dstar_pp("pp","mass");
    // MassFitter fitter_pp("pp");

    // Set mass range for D* (in GeV)
    // fitter.SetMassRange(1.90, 2.1);  // Range around D* mass
    // fitter_pp.SetMassRange(1.90, 2.1);  // Range around D* mass

    // Load your data
    // fitter_dstar.LoadData("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_PbPb_250102_v1.root", "workspace", "dataset", true);
    // fitter_dstar.LoadData("/home/jun502s/DstarAna/DstarAna_real/DDbarpPb/data/DstarMC_Rds_PbPb_250110.root", "workspace", "dataset", true);
    // fitter_dstar.LoadData("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_PbPb_241226.root", "workspace", "dataset", true);
    // fitter_pp.LoadData("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_pp.root", "workspace", "dataset", false);
    // fitter_pp.setMassVariable("massPion");
    fitter_dstar.setMassVariable("massPion");
    // fitter_dstar_pp.LoadData("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_pp_241226.root", "workspace", "dataset", false);

    // Set up Double Gaussian parameters for D*
    // MassFitter::GaussianParams sigParams;
    // MassFitter::DoubleGaussianParams sigParams_pp;
    // Load the RooAbsPdf from a file

    // Core Gaussian
    // sigParams.mean = 2.01027;        // D* mass in GeV
    // sigParams.mean_min = 2.009;
    // sigParams.mean_max = 2.011;
    // sigParams.sigma = 0.015;        
    // sigParams.sigma_min = 0.001;
    // sigParams.sigma_max = 0.03;

    // sigParams_pp.mean1 = 2.01027;        // D* mass in GeV
    // sigParams_pp.mean1_min = 2.009;
    // sigParams_pp.mean1_max = 2.011;
    // sigParams_pp.sigma1 = 0.005;        
    // sigParams_pp.sigma1_min = 0.001;
    // sigParams_pp.sigma1_max = 0.03; // Wide Gaussian
    // sigParams_pp.mean2 = 2.01027;        // Same mean as core
    // sigParams_pp.mean2_min = 2.009;
    // sigParams_pp.mean2_max = 2.011;
    // sigParams_pp.sigma2 = 0.001;         
    // sigParams_pp.sigma2_min = 0.005;
    // sigParams_pp.sigma2_max = 0.05;

    // // Fraction of core Gaussian
    // sigParams_pp.fraction = 0.3;         
    // sigParams_pp.fraction_min = 0.2;
    // sigParams_pp.fraction_max = 0.9;

    // Create signal PDF
    // fitter.MakeGaussian(sigParams, "dstar_sig");
    // fitter_pp.MakeDoubleGaussian(sigParams_pp, "dstar_sig");
    double etaMin = -1;
    double etaMax = 1;
    double mvaMin = -99;
    double mvaMax = 1;  

    // // Set up exponential background parameters
    // MassFitter::BackgroundParams bkgParams;
    // bkgParams.coefficients = { 0.,0.,0. };
    // bkgParams.coef_min = { -1. };
    // bkgParams.coef_max = { 1.};

    // fitter.MakeExponential(bkgParams, "bkg");
    // fitter_pp.MakeExponential(bkgParams, "bkg");
    // fitter.MakePolynomial(bkgParams, "bkg");
    // fitter_pp.MakePolynomial(bkgParams, "bkg");


    // MassFitter::DoubleGaussianParams sigParams_pp;
    // sigParams_pp.mean1 = 0.1455;        // D* mass in GeV
    // sigParams_pp.mean1_min = 0.145;
    // sigParams_pp.mean1_max = 0.146;
    // sigParams_pp.sigma1 = 0.0005;        
    // sigParams_pp.sigma1_min = 0.0001;
    // sigParams_pp.sigma1_max = 0.001; // Wide Gaussian
    // // sigParams_pp.mean2 = 0.145;        // Same mean as core
    // // sigParams_pp.mean2_min = 0.144;
    // // sigParams_pp.mean2_max = 0.146;
    // //same mean
    // sigParams_pp.sigma2 = 0.00012;         
    // sigParams_pp.sigma2_min = 0.0005;
    // sigParams_pp.sigma2_max = 0.003;

    // sigParams_pp.fraction = 0.3;         
    // sigParams_pp.fraction_min = 0.2;
    // sigParams_pp.fraction_max = 0.9;
    // fitter_pp.MakeDoubleGaussian(sigParams_pp, "dstar_sig");
    
    MassFitter::DBCrystalBallParams sigParams_dstar;
    sigParams_dstar.mean = 0.1455;        // D* mass in GeV
    sigParams_dstar.mean_min = 0.145;
    sigParams_dstar.mean_max = 0.146;
    sigParams_dstar.sigmaR = 0.0005;
    sigParams_dstar.sigmaR_min = 0.0001;
    sigParams_dstar.sigmaR_max = 0.001;
    sigParams_dstar.sigmaL = 0.0005;
    sigParams_dstar.sigmaL_min = 0.0001;
    sigParams_dstar.sigmaL_max = 0.001;
    
    sigParams_dstar.alphaL = 2.;
    sigParams_dstar.alphaL_min = 0;
    sigParams_dstar.alphaL_max = 10;
    sigParams_dstar.nL = 1.5;
    sigParams_dstar.nL_min = 1;
    sigParams_dstar.nL_max = 10;

    sigParams_dstar.alphaR = 2.;
    sigParams_dstar.alphaR_min = 0.;
    sigParams_dstar.alphaR_max = 10.;
    sigParams_dstar.nR = 1.5;
    sigParams_dstar.nR_min = 1;
    sigParams_dstar.nR_max = 10;
    fitter_dstar.MakeDBCrystalBall(sigParams_dstar, "dstar_sig");
    

    MassFitter::PhenomenologicalParams bkgParams_dstar;
    bkgParams_dstar.p0 = 0.1;
    bkgParams_dstar.p0_min = 0.0;
    bkgParams_dstar.p0_max = 5.0;
    bkgParams_dstar.p1 = 2;
    bkgParams_dstar.p1_min = -10.0;
    bkgParams_dstar.p1_max = 10.0;
    bkgParams_dstar.p2 = -5;
    bkgParams_dstar.p2_min = -20.0;
    bkgParams_dstar.p2_max = 20.0;
    fitter_dstar.MakePhenomenological(bkgParams_dstar, "bkg");

    // Combine PDFs
     for (const auto& bin : pTBins) {
        
        double pTMin = bin.first;
        double pTMax = bin.second;

        // Define cuts for the current bin
        // TString CutPbPb = Form("pT > %.1f && pT < %.1f && eta < 1 && eta > -1 && mva> %.2f", pTMin, pTMax, etaMin, etaMax, mvaMin);
        // TString Cutpp = Form("pT > %f && pT < %f && eta < 1 && eta > -1", pTMin, pTMax);
        TString CutPbPb = Form("pT > %f && pT < %f", pTMin, pTMax);
        // TString Cutpp = Form("pT > %f && pT < %f ", pTMin, pTMax);

        // Reduce data with specific cuts
        fitter_dstar.ReduceData(CutPbPb.Data());
        // fitter_pp.ReduceData(Cutpp.Data());
        // fitter.CombinePDFs();
        // fitter_pp.CombinePDFs();
        // fitter_dstar.CombinePDFs();



        // Perform the fitting for the current bin
        // fitter_dstar.PerformFit(pTMin,pTMax,true);
        // fitter_dstar.saveFitModel(pTMin,pTMax);
        //fitter_pp.PerformFit();

        // Output the results for the current bin
        std::cout << "Results for pT bin [" << pTMin << ", " << pTMax << "]:" << std::endl;
        // fitter_pp.setMassVariable("mass");
        // fitter.PlotResult(pTMin,pTMax);
        // fitter_pp.PlotResult(pTMin,pTMax);
        // fitter.setMassVariable("massPion");
        // fitter_pp.setMassVariable("massPion");
        // fitter.PlotResult(pTMin,pTMax);
        // fitter_pp.PlotResult(pTMin,pTMax);
        // fitter.setMassVariable("massD0");
        // fitter_pp.setMassVariable("massD0");
        fitter_dstar.PlotResult(pTMin,pTMax);
        // fitter_pp.PlotResult(pTMin,pTMax);

        // break;
    }
}
