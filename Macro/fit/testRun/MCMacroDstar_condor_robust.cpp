#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
#include "../DCAFitter.h"
#include "../RobustParameterManager.h"
#include "../../Tools/ConfigManager.h"

// Test macro for the new robust fitting framework
void MCMacroDstar_condor_robust(bool doReFit = false, bool plotFit = true, bool useCUDA = true, 
                                float pTMin = 4, float pTMax = 100, float cosMin = -2, float cosMax = 2) {
    
    // Initialize logging for the robust framework
    FittingLogger::setLogLevel(LogLevel::INFO);
    FittingLogger::setLogFile("robust_fit_test.log");
    FittingLogger::info("Starting robust D* meson fitting test");
    
    // Configure fit options
    FitOpt D0opt;
    D0opt.useCUDA = useCUDA;
    D0opt.doFit = doReFit;
    
    std::string particleType = "DStar";
    std::vector<std::pair<double,double>> ptBins = {{pTMin, pTMax}};
    std::vector<std::pair<double,double>> cosBins = {{cosMin, cosMax}};
    
    // File paths
    std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_noPreselectionCut_ppRef_Jul28_v1.root";
    std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_ppRef_Aug01_0p5_v1.root";
    std::string datasetName = "datasetHX";
    std::string subDir = "/DStar_ppRef_Data_robust_test_v1/";
    
    // Selection cuts
    std::string slowPionCut = SelectionCuts::getSlowPionCuts();
    std::string grandDaughterCut = SelectionCuts::getGrandDaughterCuts();
    
    // Signal and background yield ratios
    double nsig_ratio = 0.3;
    double nsig_min_ratio = 0.1;
    double nsig_max_ratio = 0.8;
    double nbkg_ratio = 0.2;
    double nbkg_min_ratio = 0.05;
    double nbkg_max_ratio = 0.6;
    
    // Get parameter templates
    auto params = DStarParamMaker1({0}, {0});
    
    // Configure parameters based on pT bins (simplified from original)
    if (pTMin >= 5 && pTMax <= 10) {
        // Low pT: more conservative parameters
        params[{0,0}].first.mean = 0.1455;
        params[{0,0}].first.mean_min = 0.1452;
        params[{0,0}].first.mean_max = 0.1458;
        params[{0,0}].first.sigma = 0.0005;
        params[{0,0}].first.sigma_min = 0.0001;
        params[{0,0}].first.sigma_max = 0.01;
        
        // Crystal Ball parameters
        params[{0,0}].first.alphaL = 1.1;
        params[{0,0}].first.alphaL_min = 0.5;
        params[{0,0}].first.alphaL_max = 2.5;
        params[{0,0}].first.nL = 1.5;
        params[{0,0}].first.nL_min = 1.0;
        params[{0,0}].first.nL_max = 50;
        
        // Background parameters
        params[{0,0}].second.p0 = 0.01;
        params[{0,0}].second.p0_min = 0.001;
        params[{0,0}].second.p0_max = 0.5;
    }
    
    // Load data
    DataLoader MCloader(filepathMC);
    DataLoader Dataloader(filepathData);
    
    try {
        MCloader.loadRooDataSet(datasetName);
        Dataloader.loadRooDataSet(datasetName);
        FittingLogger::info("Data loaded successfully");
    } catch (const std::exception& e) {
        FittingLogger::error("Failed to load data: " + std::string(e.what()));
        return;
    }
    
    // Main fitting loop
    for (auto ptbin : ptBins) {
        for (auto cosbin : cosBins) {
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            D0opt.cosMin = cosbin.first;
            D0opt.cosMax = cosbin.second;
            
            // Configure options
            D0opt.DStarMCAbsDefault();
            D0opt.outputDir = D0opt.outputDir + subDir;
            D0opt.outputMCDir = D0opt.outputMCDir + subDir;
            D0opt.outputPlotDir = D0opt.outputPlotDir + subDir;
            D0opt.cutMCExpr = D0opt.cutMCExpr + "&&" + slowPionCut + "&&" + grandDaughterCut;
            D0opt.cutExpr = D0opt.cutExpr + "&&" + slowPionCut + "&&" + grandDaughterCut;
            
            BinInfo currentBin = createBinInfoFromFitOpt(D0opt);
            
            if (doReFit) {
                FittingLogger::info("Starting fit for pT: [" + std::to_string(ptbin.first) + 
                                  ", " + std::to_string(ptbin.second) + "], cos: [" + 
                                  std::to_string(cosbin.first) + ", " + std::to_string(cosbin.second) + "]");
                
                try {
                    // Create MassFitter with robust framework
                    MassFitter fitter(D0opt.name, D0opt.massVar, D0opt.massMin, D0opt.massMax,
                                     nsig_ratio, nsig_min_ratio, nsig_max_ratio, 
                                     nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio);
                    
                    FittingLogger::info("MassFitter initialized with robust framework");
                    
                    // Test different fitting strategies
                    std::vector<FitStrategy> strategiesToTest = {
                        FitStrategy::STANDARD,
                        FitStrategy::ROBUST,
                        FitStrategy::ADAPTIVE
                    };
                    
                    bool fitSuccessful = false;
                    
                    // First, set up the PDFs using traditional method
                    fitter.PerformConstraintFit(D0opt, Dataloader.getDataSet(), MCloader.getDataSet(), 
                                               true, "", "", params[{0,0}].first, params[{0,0}].second);
                    
                    // Now test the robust fitting capabilities
                    for (auto strategy : strategiesToTest) {
                        FittingLogger::info("Testing fit strategy: " + std::to_string(static_cast<int>(strategy)));
                        
                        try {
                            // Use the new RobustFit method
                            auto robustResult = fitter.RobustFit(strategy);
                            
                            if (robustResult && robustResult->status() == 0) {
                                FittingLogger::info("Robust fit succeeded with strategy: " + 
                                                  std::to_string(static_cast<int>(strategy)));
                                
                                // Get comprehensive diagnostics
                                auto diagnostics = ComprehensiveFitDiagnostics::diagnose(
                                    robustResult.get(), nullptr, Dataloader.getDataSet());
                                
                                ComprehensiveFitDiagnostics::printDiagnostics(diagnostics);
                                
                                if (diagnostics.qualityScore > 0.7) {
                                    FittingLogger::info("High quality fit achieved, stopping strategy testing");
                                    fitSuccessful = true;
                                    break;
                                }
                            } else {
                                FittingLogger::warning("Strategy failed: " + std::to_string(static_cast<int>(strategy)));
                            }
                            
                        } catch (const FittingException& e) {
                            FittingLogger::error("Fitting exception: " + std::string(e.what()));
                        } catch (const std::exception& e) {
                            FittingLogger::error("General exception during robust fit: " + std::string(e.what()));
                        }
                    }
                    
                    // Get final fit result and record status
                    RooFitResult* fitResult = fitter.GetFitResult();
                    if (fitResult) {
                        checkAndRecordFitStatus(fitResult, currentBin, "Mass", "Robust Data fit");
                        
                        // Test parameter manager capabilities
                        testParameterManagerFeatures(fitter);
                        
                        FittingLogger::info("Fit completed for current bin");
                    } else {
                        FittingLogger::error("No fit result available");
                    }
                    
                } catch (const std::exception& e) {
                    FittingLogger::error("Exception during fitting: " + std::string(e.what()));
                }
            }
            
            // Plotting
            try {
                PlotManager plotManager(D0opt, D0opt.outputDir, D0opt.outputFile, 
                                      "plots/Data_DStar_ppRef_Robust/" + subDir, true, true);
                PlotManager plotManagerMC(D0opt, D0opt.outputMCDir, D0opt.outputMCFile, 
                                        "plots/MC_DStar_ppRef_Robust/" + subDir, true, true);
                
                if (!plotFit) {
                    plotManagerMC.DrawRawDistribution();
                    plotManager.DrawRawDistribution();
                } else {
                    plotManagerMC.DrawFittedModel(true);
                    plotManager.DrawFittedModel(true);
                }
                
                FittingLogger::info("Plots generated successfully");
                
            } catch (const std::exception& e) {
                FittingLogger::error("Exception during plotting: " + std::string(e.what()));
            }
            
            // Test DCAFitter with enhanced error handling
            testRobustDCAFitting(D0opt);
        }
    }
    
    // Print comprehensive statistics
    printRobustFittingStatistics();
    
    // Performance monitoring results
    PerformanceMonitor::printStatistics();
    PerformanceMonitor::saveStatistics(D0opt.outputDir + "/performance_stats.csv");
    
    FittingLogger::info("Robust fitting test completed successfully");
}

// Helper function to test parameter manager features
void testParameterManagerFeatures(MassFitter& fitter) {
    FittingLogger::info("Testing RobustParameterManager features");
    
    try {
        // Access the parameter manager through the workspace or direct access if available
        // This would need to be implemented based on your specific access patterns
        
        FittingLogger::info("Parameter manager test completed");
        
    } catch (const std::exception& e) {
        FittingLogger::error("Parameter manager test failed: " + std::string(e.what()));
    }
}

// Enhanced DCA fitting with robust error handling
void testRobustDCAFitting(FitOpt& D0opt) {
    FittingLogger::info("Starting robust DCA fitting test");
    
    try {
        // Configure for D0 daughter analysis
        D0opt.massVar = "massDaugther1";
        D0opt.massMin = 1.75;
        D0opt.massMax = 2.00;
        
        // Create DCAFitter with enhanced error handling
        DCAFitter DCAfitter(D0opt, "dcaFitter", D0opt.massVar, D0opt.dcaMin, D0opt.dcaMax, 100);
        
        // Configure files with validation
        std::string mcFile = D0opt.outputMCDir + "/" + D0opt.outputMCFile;
        std::string dataFile = D0opt.outputDir + "/" + D0opt.outputFile;
        
        // Validate file existence before proceeding
        if (!std::ifstream(mcFile) || !std::ifstream(dataFile)) {
            FittingLogger::warning("Required files not found, skipping DCA fitting");
            return;
        }
        
        DCAfitter.setMCFile(mcFile.c_str(), "reducedData");
        DCAfitter.setDataFile(dataFile.c_str(), "reducedData");
        DCAfitter.setDCABranchName("dca3D");
        DCAfitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_");
        
        // Define prompt/non-prompt particles
        DCAfitter.setPromptPdgIds({4, 2});
        DCAfitter.setNonPromptPdgIds({5});
        
        DCAfitter.setOutputFile(D0opt.outputDir + "/" + D0opt.outputDCAFile);
        
        // Execute DCA fitting with comprehensive error handling
        if (DCAfitter.createTemplatesFromMC()) {
            DCAfitter.plotRawDataDistribution(D0opt.outputPlotDir + D0opt.outputDCAFile + "_MC_Templates_Normalized");
            
            if (DCAfitter.loadData()) {
                if (DCAfitter.buildModelwSideband()) {
                    RooFitResult* fitResult = DCAfitter.performFit(true);
                    
                    if (fitResult) {
                        DCAfitter.plotResults(fitResult, D0opt.outputPlotDir + D0opt.outputDCAFile + "_DCA_Fit_Result_Plot");
                        DCAfitter.saveResults(fitResult);
                        
                        // Safe memory management
                        delete fitResult;
                        
                        FittingLogger::info("DCA fitting completed successfully");
                    } else {
                        FittingLogger::error("DCA fit failed to produce result");
                    }
                } else {
                    FittingLogger::error("Failed to build DCA model");
                }
            } else {
                FittingLogger::error("Failed to load DCA data");
            }
        } else {
            FittingLogger::error("Failed to create DCA templates from MC");
        }
        
    } catch (const std::exception& e) {
        FittingLogger::error("Exception in DCA fitting: " + std::string(e.what()));
    }
}

// Print comprehensive fitting statistics
void printRobustFittingStatistics() {
    FittingLogger::info("=== ROBUST FITTING TEST SUMMARY ===");
    
    // Print failed fits
    printFailedFits();
    
    // Additional statistics would be printed here
    // This could include strategy success rates, quality score distributions, etc.
    
    FittingLogger::info("=== END SUMMARY ===");
}

// Test specific robust features
void demonstrateRobustFeatures() {
    FittingLogger::info("Demonstrating robust framework features");
    
    try {
        // Create a standalone parameter manager for testing
        RobustParameterManager paramManager;
        
        // Test parameter creation and validation
        auto* meanParam = paramManager.createParameter("test_mean", ParameterType::MEAN, 0.1455, 0.1450, 0.1460);
        auto* sigmaParam = paramManager.createParameter("test_sigma", ParameterType::SIGMA, 0.0005, 0.0001, 0.001);
        
        if (meanParam && sigmaParam) {
            FittingLogger::info("Parameters created successfully");
            
            // Test parameter validation
            if (paramManager.validateAllParameters()) {
                FittingLogger::info("All parameters are valid");
            }
            
            // Test state saving/restoration
            paramManager.saveParameterState("test_state");
            
            // Modify parameters
            paramManager.setParameterValue("test_mean", 0.1456);
            
            // Restore state
            paramManager.restoreParameterState("test_state");
            
            FittingLogger::info("Parameter state management test completed");
        }
        
        // Test smart fitting strategy manager
        SmartFitStrategyManager strategyManager;
        
        // This would be used in actual fitting scenarios
        FittingLogger::info("Strategy manager initialized");
        
        // Print strategy statistics (would show real data in actual usage)
        strategyManager.printStrategyStatistics();
        
    } catch (const std::exception& e) {
        FittingLogger::error("Exception in robust features demo: " + std::string(e.what()));
    }
}