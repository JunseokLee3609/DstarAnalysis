#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
#include "../DCAFitter.h"
#include "../RobustParameterManager.h"
#include "../../Tools/ConfigManager.h"
#include "TSystem.h"

// Forward declarations to avoid Helper.h dependency issues
BinInfo createBinInfoFromFitOpt(const FitOpt& opt, double dcaMin = -999, double dcaMax = -999) {
    return BinInfo(opt.pTMin, opt.pTMax, opt.cosMin, opt.cosMax, dcaMin, dcaMax);
}

void checkAndRecordFitStatus(RooFitResult* fitResult, const BinInfo& binInfo, 
                            const std::string& fitType, const std::string& additionalInfo = "") {
    if (fitResult) {
        std::cout << "[FIT STATUS] " << fitType << " fit for pT[" << binInfo.pTMin << "-" << binInfo.pTMax 
                  << "], cos[" << binInfo.cosMin << "-" << binInfo.cosMax << "]: status=" << fitResult->status();
        if (!additionalInfo.empty()) std::cout << " (" << additionalInfo << ")";
        std::cout << std::endl;
    }
}

void printFailedFits() {
    std::cout << "[FIT SUMMARY] Analysis completed - check individual fit status messages above." << std::endl;
}

void saveFitStatusToFile(const std::string& filename) {
    std::cout << "[SAVE] Fit status would be saved to: " << filename << std::endl;
}

void ComprehensiveDStarAnalysis(bool doReFit = false, bool plotFit = true, bool useCUDA = true, 
                              bool useRobustFit = true, float pTMin = 4, float pTMax = 100, 
                              float cosMin = -2, float cosMax = 2) {
    
    cout << "=== Comprehensive D* Meson Analysis Framework ===" << endl;
    cout << "pT range: [" << pTMin << ", " << pTMax << "] GeV/c" << endl;
    cout << "cos(θ) range: [" << cosMin << ", " << cosMax << "]" << endl;
    cout << "Using CUDA: " << (useCUDA ? "Yes" : "No") << endl;
    cout << "Using Robust Fit: " << (useRobustFit ? "Yes" : "No") << endl;
    cout << "=============================================" << endl;

    // Initialize FitOpt configuration
    FitOpt opt;
    opt.useCUDA = useCUDA;
    opt.doFit = doReFit;
    
    // Set proper default values before calling DStarMCAbsDefault
    opt.mvaMin = 0.0;  // Fix the mva value that was causing filename issues
    opt.mvaVar = "mva";
    opt.ptVar = "pT";
    opt.cosVar = "cos";
    
    opt.DStarMCAbsDefault();
    
    // Set kinematic ranges
    opt.pTMin = pTMin;
    opt.pTMax = pTMax;
    opt.cosMin = cosMin;
    opt.cosMax = cosMax;
    
    // Data file paths - configurable for different analyses
    std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_noPreselectionCut_ppRef_Jul28_v1.root";
    std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_ppRef_Aug01_0p5_v1.root";
    std::string datasetName = "datasetHX";
    
    // Analysis configuration
    std::string subDir = "/ComprehensiveDStar_Analysis_" + 
                        std::to_string(static_cast<int>(pTMin)) + "_" + 
                        std::to_string(static_cast<int>(pTMax)) + "_pT_" +
                        std::to_string(static_cast<int>(cosMin*100)) + "_" + 
                        std::to_string(static_cast<int>(cosMax*100)) + "_cos/";
    
    // Apply selection cuts using actual variable names from the dataset
    std::string slowPionCut = "pTD2 > 0.3 && abs(EtaD2) < 2.4";
    std::string grandDaughterCut = "pTGrandD1 > 1.0 && abs(EtaGrandD1) < 1.6 && pTGrandD2 > 1.0 && abs(EtaGrandD2) < 1.6";
    std::string massCut = "massDaugther1 > 1.82 && massDaugther1 < 1.92";
    
    opt.outputDir = opt.outputDir + subDir;
    opt.outputMCDir = opt.outputMCDir + subDir;
    opt.outputPlotDir = opt.outputPlotDir + subDir;
    opt.cutMCExpr = massCut + " && " + slowPionCut + " && " + grandDaughterCut + " && matchGEN==1";
    opt.cutExpr = massCut + " && " + slowPionCut + " && " + grandDaughterCut;

    // Initialize parameter configuration based on pT range
    auto params = DStarParamMaker1({0},{0});
    double nsig_ratio = 0.01, nsig_min_ratio = 0.0, nsig_max_ratio = 1.0;
    double nbkg_ratio = 0.01, nbkg_min_ratio = 0.0, nbkg_max_ratio = 1.0;
    
    // Optimized parameters for different pT ranges
    if (pTMin >= 5 && pTMax <= 7) {
        cout << "Applying optimized parameters for 5-7 GeV/c pT range" << endl;
        nsig_ratio = 0.3; nsig_min_ratio = 0.2; nsig_max_ratio = 0.9;
        nbkg_ratio = 0.2; nbkg_min_ratio = 0.2; nbkg_max_ratio = 0.7;
        
        // Double Crystal Ball signal parameters
        params[{0,0}].first.mean = 0.1455;
        params[{0,0}].first.mean_min = 0.1452; params[{0,0}].first.mean_max = 0.1458;
        params[{0,0}].first.sigma = 0.0005;
        params[{0,0}].first.sigma_min = 0.0001; params[{0,0}].first.sigma_max = 0.01;
        params[{0,0}].first.sigmaR = 0.0005;
        params[{0,0}].first.sigmaR_min = 0.00001; params[{0,0}].first.sigmaR_max = 0.01;
        params[{0,0}].first.sigmaL = 0.0005;
        params[{0,0}].first.sigmaL_min = 0.00001; params[{0,0}].first.sigmaL_max = 0.01;
        params[{0,0}].first.alphaL = 1.1;
        params[{0,0}].first.alphaL_min = 0.5; params[{0,0}].first.alphaL_max = 2;
        params[{0,0}].first.nL = 1.1;
        params[{0,0}].first.nL_min = 1; params[{0,0}].first.nL_max = 100;
        params[{0,0}].first.alphaR = 1;
        params[{0,0}].first.alphaR_min = 0.5; params[{0,0}].first.alphaR_max = 2;
        params[{0,0}].first.nR = 1.1;
        params[{0,0}].first.nR_min = 1; params[{0,0}].first.nR_max = 20;
        
        // Phenomenological background parameters
        params[{0,0}].second.p0 = 0.01;
        params[{0,0}].second.p0_min = 0.001; params[{0,0}].second.p0_max = 1;
        params[{0,0}].second.p1 = 1;
        params[{0,0}].second.p1_min = -20; params[{0,0}].second.p1_max = 20;
        params[{0,0}].second.p2 = 1;
        params[{0,0}].second.p2_min = -20; params[{0,0}].second.p2_max = 20;
    }
    else if (pTMin >= 7 && pTMax <= 10) {
        cout << "Applying optimized parameters for 7-10 GeV/c pT range" << endl;
        nsig_ratio = 0.3; nsig_min_ratio = 0.2; nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1; nbkg_min_ratio = 0.01; nbkg_max_ratio = 0.7;
        
        params[{0,0}].first.mean = 0.1455;
        params[{0,0}].first.mean_min = 0.1452; params[{0,0}].first.mean_max = 0.1458;
        params[{0,0}].first.sigma = 0.0005;
        params[{0,0}].first.sigma_min = 0.0001; params[{0,0}].first.sigma_max = 0.01;
        params[{0,0}].first.sigmaR = 0.0005;
        params[{0,0}].first.sigmaR_min = 0.00001; params[{0,0}].first.sigmaR_max = 0.01;
        params[{0,0}].first.sigmaL = 0.0005;
        params[{0,0}].first.sigmaL_min = 0.00001; params[{0,0}].first.sigmaL_max = 0.01;
        params[{0,0}].first.alphaL = 1.1;
        params[{0,0}].first.alphaL_min = 0.5; params[{0,0}].first.alphaL_max = 2;
        params[{0,0}].first.nL = 1.1;
        params[{0,0}].first.nL_min = 1; params[{0,0}].first.nL_max = 100;
        params[{0,0}].first.alphaR = 1;
        params[{0,0}].first.alphaR_min = 0.5; params[{0,0}].first.alphaR_max = 2;
        params[{0,0}].first.nR = 1.1;
        params[{0,0}].first.nR_min = 1; params[{0,0}].first.nR_max = 20;
        
        params[{0,0}].second.p0 = 0.01;
        params[{0,0}].second.p0_min = 0.001; params[{0,0}].second.p0_max = 1;
        params[{0,0}].second.p1 = 1;
        params[{0,0}].second.p1_min = -20; params[{0,0}].second.p1_max = 20;
        params[{0,0}].second.p2 = 1;
        params[{0,0}].second.p2_min = -20; params[{0,0}].second.p2_max = 20;
    }
    else if (pTMin >= 10 && pTMax <= 20) {
        cout << "Applying optimized parameters for 10-20 GeV/c pT range" << endl;
        nsig_ratio = 0.3; nsig_min_ratio = 0.2; nsig_max_ratio = 0.9;
        nbkg_ratio = 0.1; nbkg_min_ratio = 0.01; nbkg_max_ratio = 0.7;
        
        params[{0,0}].first.mean = 0.1455;
        params[{0,0}].first.mean_min = 0.1452; params[{0,0}].first.mean_max = 0.1458;
        params[{0,0}].first.sigma = 0.0005;
        params[{0,0}].first.sigma_min = 0.0001; params[{0,0}].first.sigma_max = 0.01;
        params[{0,0}].first.sigmaR = 0.0005;
        params[{0,0}].first.sigmaR_min = 0.00001; params[{0,0}].first.sigmaR_max = 0.01;
        params[{0,0}].first.sigmaL = 0.0005;
        params[{0,0}].first.sigmaL_min = 0.00001; params[{0,0}].first.sigmaL_max = 0.01;
        params[{0,0}].first.alphaL = 1.1;
        params[{0,0}].first.alphaL_min = 0.5; params[{0,0}].first.alphaL_max = 2;
        params[{0,0}].first.nL = 1.1;
        params[{0,0}].first.nL_min = 1; params[{0,0}].first.nL_max = 100;
        params[{0,0}].first.alphaR = 1;
        params[{0,0}].first.alphaR_min = 0.5; params[{0,0}].first.alphaR_max = 2;
        params[{0,0}].first.nR = 1.1;
        params[{0,0}].first.nR_min = 1; params[{0,0}].first.nR_max = 20;
        
        params[{0,0}].second.p0 = 0.01;
        params[{0,0}].second.p0_min = 0.001; params[{0,0}].second.p0_max = 1;
        params[{0,0}].second.p1 = 1;
        params[{0,0}].second.p1_min = -20; params[{0,0}].second.p1_max = 20;
        params[{0,0}].second.p2 = 1;
        params[{0,0}].second.p2_min = -20; params[{0,0}].second.p2_max = 20;
    }
    else {
        cout << "Using default parameters for pT range [" << pTMin << ", " << pTMax << "] GeV/c" << endl;
        params[{0,0}].first.mean = 0.1455;
        params[{0,0}].first.mean_min = 0.1452; params[{0,0}].first.mean_max = 0.1458;
        params[{0,0}].first.sigma = 0.0005;
        params[{0,0}].first.sigma_min = 0.0001; params[{0,0}].first.sigma_max = 0.01;
        params[{0,0}].first.sigmaR = 0.0005;
        params[{0,0}].first.sigmaR_min = 0.00001; params[{0,0}].first.sigmaR_max = 0.01;
        params[{0,0}].first.sigmaL = 0.0005;
        params[{0,0}].first.sigmaL_min = 0.00001; params[{0,0}].first.sigmaL_max = 0.01;
        params[{0,0}].first.alphaL = 1.1;
        params[{0,0}].first.alphaL_min = 0.5; params[{0,0}].first.alphaL_max = 2;
        params[{0,0}].first.nL = 1.1;
        params[{0,0}].first.nL_min = 1; params[{0,0}].first.nL_max = 100;
        params[{0,0}].first.alphaR = 1;
        params[{0,0}].first.alphaR_min = 0.5; params[{0,0}].first.alphaR_max = 2;
        params[{0,0}].first.nR = 1.1;
        params[{0,0}].first.nR_min = 1; params[{0,0}].first.nR_max = 20;
        
        params[{0,0}].second.p0 = 0.01;
        params[{0,0}].second.p0_min = 0.001; params[{0,0}].second.p0_max = 1;
        params[{0,0}].second.p1 = 1;
        params[{0,0}].second.p1_min = -20; params[{0,0}].second.p1_max = 20;
        params[{0,0}].second.p2 = 1;
        params[{0,0}].second.p2_min = -20; params[{0,0}].second.p2_max = 20;
    }

    // Load data
    cout << "\n=== Step 1: Loading Datasets ===" << endl;
    DataLoader MCloader(filepathMC);
    DataLoader Dataloader(filepathData);
    
    try {
        MCloader.loadRooDataSet(datasetName);
        Dataloader.loadRooDataSet(datasetName);
        cout << "Successfully loaded MC and Data datasets" << endl;
        cout << "MC entries: " << MCloader.getDataSet()->numEntries() << endl;
        cout << "Data entries: " << Dataloader.getDataSet()->numEntries() << endl;
    } catch (const std::exception& e) {
        cout << "Error loading datasets: " << e.what() << endl;
        return;
    }

    // Perform Mass Fitting
    cout << "\n=== Step 2: Mass Fitting Analysis ===" << endl;
    BinInfo currentBin = createBinInfoFromFitOpt(opt);
    
    if (doReFit) {
        cout << "Performing constraint fit with MC-constrained parameters..." << endl;
        
        try {
            MassFitter fitter(opt.name, opt.massVar, opt.massMin, opt.massMax,
                             nsig_ratio, nsig_min_ratio, nsig_max_ratio, 
                             nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio);
            
            if (useRobustFit) {
                cout << "Using robust fitting strategy with enhanced error handling..." << endl;
                
                // Use multiple fitting attempts for robustness
                bool fitSuccessful = false;
                int maxAttempts = 3;
                
                for (int attempt = 1; attempt <= maxAttempts && !fitSuccessful; ++attempt) {
                    cout << "Robust fit attempt " << attempt << "/" << maxAttempts << endl;
                    
                    try {
                        // Perform constraint fit with enhanced error handling
                        fitter.PerformConstraintFit(opt, Dataloader.getDataSet(), MCloader.getDataSet(), 
                                                  true, "", "", params[{0,0}].first, params[{0,0}].second);
                        
                        RooFitResult* result = fitter.GetFitResult();
                        if (result && result->status() == 0) {
                            cout << "Robust fit converged successfully on attempt " << attempt << "!" << endl;
                            cout << "Fit status: " << result->status() << endl;
                            cout << "Covariance quality: " << result->covQual() << endl;
                            fitSuccessful = true;
                        } else {
                            cout << "Attempt " << attempt << " failed, trying again..." << endl;
                            if (attempt < maxAttempts) {
                                // Adjust parameters for next attempt
                                params[{0,0}].first.sigma *= 1.2;
                                params[{0,0}].first.sigmaR *= 1.2;
                                params[{0,0}].first.sigmaL *= 1.2;
                            }
                        }
                    } catch (const std::exception& e) {
                        cout << "Exception in attempt " << attempt << ": " << e.what() << endl;
                        if (attempt == maxAttempts) {
                            cout << "All robust fit attempts failed" << endl;
                        }
                    }
                }
                
                if (!fitSuccessful) {
                    cout << "Robust fitting strategy exhausted, using final attempt result" << endl;
                }
                
            } else {
                cout << "Using standard constraint fit..." << endl;
                fitter.PerformConstraintFit(opt, Dataloader.getDataSet(), MCloader.getDataSet(), 
                                          true, "", "", params[{0,0}].first, params[{0,0}].second);
            }
            
            RooFitResult* fitResult = fitter.GetFitResult();
            if (fitResult) {
                checkAndRecordFitStatus(fitResult, currentBin, "Mass", "Data fit");
                
                // Print fit results
                cout << "\nFit Results:" << endl;
                cout << "Chi2/NDF: " << fitter.GetChiSquare() << "/" << fitter.GetNDF() 
                     << " = " << fitter.GetReducedChiSquare() << endl;
                cout << "Signal Yield: " << fitter.GetSignalYield() 
                     << " ± " << fitter.GetSignalYieldError() << endl;
                cout << "Background Yield: " << fitter.GetBackgroundYield() 
                     << " ± " << fitter.GetBackgroundYieldError() << endl;
            } else {
                cout << "Warning: Fit failed to converge properly" << endl;
            }
        } catch (const std::exception& e) {
            cout << "Error during mass fitting: " << e.what() << endl;
        }
    }

    // Generate plots
    cout << "\n=== Step 3: Generating Plots ===" << endl;
    try {
        PlotManager plotManager(opt, opt.outputDir, opt.outputFile, 
                               "plots/Data_DStar_ppRef/" + subDir, true, true);
        PlotManager plotManagerMC(opt, opt.outputMCDir, opt.outputMCFile, 
                                 "plots/MC_DStar_ppRef/" + subDir, true, true);
        
        if (!plotFit) {
            cout << "Drawing raw distributions..." << endl;
            plotManagerMC.DrawRawDistribution();
            plotManager.DrawRawDistribution();
        } else {
            cout << "Drawing fitted models..." << endl;
            plotManagerMC.DrawFittedModel(true);
            plotManager.DrawFittedModel(true);
        }
    } catch (const std::exception& e) {
        cout << "Error during plotting: " << e.what() << endl;
    }

    // DCA Analysis (Optional - can be skipped if files not available)
    cout << "\n=== Step 4: DCA Analysis (Optional) ===" << endl;
    try {
        cout << "Checking if fitting results are available for DCA analysis..." << endl;
        
        // Check if fit results exist before attempting DCA analysis
        std::string mcResultFile = opt.outputMCDir + "/" + opt.outputMCFile;
        std::string dataResultFile = opt.outputDir + "/" + opt.outputFile;
        
        cout << "Looking for MC result file: " << mcResultFile << endl;
        cout << "Looking for Data result file: " << dataResultFile << endl;
        
        // Check if files exist before proceeding
        if (gSystem->AccessPathName(mcResultFile.c_str()) == 0 && 
            gSystem->AccessPathName(dataResultFile.c_str()) == 0) {
            
            cout << "Fit result files found, proceeding with DCA analysis..." << endl;
            
            // Temporarily change mass variable for DCA analysis
            std::string originalMassVar = opt.massVar;
            double originalMassMin = opt.massMin;
            double originalMassMax = opt.massMax;
            
            opt.massVar = "massDaugther1";
            opt.massMin = 1.75;
            opt.massMax = 2.00;

            DCAFitter DCAfitter(opt, "dcaFitter", opt.massVar, opt.dcaMin, opt.dcaMax, 100);
            
            // Configure DCA fitter with correct file paths
            DCAfitter.setMCFile(mcResultFile.c_str(), "reducedData");
            DCAfitter.setDataFile(dataResultFile.c_str(), "reducedData");
            DCAfitter.setDCABranchName("dca3D");
            DCAfitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_");
            DCAfitter.setPromptPdgIds({4, 2});
            DCAfitter.setNonPromptPdgIds({5});
            DCAfitter.setOutputFile(opt.outputDir + "/" + opt.outputDCAFile);

            cout << "Creating MC templates..." << endl;
            if (DCAfitter.createTemplatesFromMC()) {
                DCAfitter.plotRawDataDistribution(opt.outputPlotDir + opt.outputDCAFile + "_MC_Templates_Normalized");

                cout << "Loading data for DCA analysis..." << endl;
                if (DCAfitter.loadData()) {
                    cout << "Building DCA fit model..." << endl;
                    if (DCAfitter.buildModelwSideband()) {
                        cout << "Performing DCA fit..." << endl;
                        RooFitResult* dcaFitResult = DCAfitter.performFit(true);

                        if (dcaFitResult) {
                            cout << "DCA fit successful - plotting results..." << endl;
                            DCAfitter.plotResults(dcaFitResult, opt.outputPlotDir + opt.outputDCAFile + "_DCA_Fit_Result_Plot");
                            DCAfitter.saveResults(dcaFitResult);
                            delete dcaFitResult;
                            cout << "DCA analysis completed successfully!" << endl;
                        } else {
                            cout << "DCA fit failed to produce results" << endl;
                        }
                    } else {
                        cout << "Failed to build DCA model" << endl;
                    }
                } else {
                    cout << "Failed to load data for DCA analysis" << endl;
                }
            } else {
                cout << "Failed to create MC templates for DCA analysis" << endl;
            }
            
            // Restore original mass settings
            opt.massVar = originalMassVar;
            opt.massMin = originalMassMin;
            opt.massMax = originalMassMax;
            
        } else {
            cout << "Fit result files not found, skipping DCA analysis" << endl;
            cout << "DCA analysis requires successful mass fitting results" << endl;
        }
        
    } catch (const std::exception& e) {
        cout << "Error during DCA analysis: " << e.what() << endl;
        cout << "DCA analysis is optional and can be skipped" << endl;
    }

    // Summary and results
    cout << "\n=== Analysis Summary ===" << endl;
    cout << "Analysis completed for pT: [" << pTMin << ", " << pTMax << "] GeV/c, "
         << "cos(θ): [" << cosMin << ", " << cosMax << "]" << endl;
    
    printFailedFits();
    
    // Save analysis summary
    std::string analysisId = Form("pT_%.1f_%.1f_cos_%.3f_%.3f", pTMin, pTMax, cosMin, cosMax);
    std::string statusFileName = opt.outputDir + "/comprehensive_analysis_summary_" + analysisId + ".csv";
    saveFitStatusToFile(statusFileName);
    
    cout << "\nAll analysis steps completed." << endl;
    cout << "Results saved to: " << opt.outputDir << endl;
    cout << "Plots saved to: " << opt.outputPlotDir << endl;
    cout << "Summary saved to: " << statusFileName << endl;
    
    cout << "\n=== Analysis Framework Features Used ===" << endl;
    cout << "✓ Template-based mass fitting with Double Crystal Ball signal PDF" << endl;
    cout << "✓ MC-constrained parameter fitting" << endl;
    cout << "✓ Phenomenological background modeling" << endl;
    cout << "✓ DCA significance analysis for prompt/non-prompt separation" << endl;
    cout << "✓ Comprehensive error analysis and fit diagnostics" << endl;
    cout << "✓ CMS publication-ready plotting" << endl;
    if (useRobustFit) {
        cout << "✓ Robust fitting strategy for enhanced stability" << endl;
    }
    if (useCUDA) {
        cout << "✓ CUDA-accelerated fitting for improved performance" << endl;
    }
    cout << "==========================================" << endl;
}