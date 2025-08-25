#include "../DStarFitConfig.h"
#include "../MassFitterV2.h"
#include "../DataLoader.h"
#include "../PlotManager.h"
#include "../../Tools/ConfigManager.h"

/**
 * @brief Modern D* meson analysis using the new modular framework
 * 
 * This macro demonstrates how to use the improved MassFitterV2 with 
 * configurable kinematic bins and centrality support (dummy for now).
 */
void DStarAnalysisV2(bool doReFit = false, bool plotFit = true, bool useCUDA = true,
                     float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                     int centralityMin = 0, int centralityMax = 100) {
    
    std::cout << "=== D* Meson Analysis V2 ===" << std::endl;
    std::cout << "Using new modular framework with MassFitterV2" << std::endl;
    
    // Create main configuration
    DStarFitConfig config;
    
    // Configure file paths
    // config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_PbPb_mva0p99_PbPb_Aug25_v1.root");
    config.SetMCFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    config.SetDatasetName("datasetHX");
    config.SetOutputSubDir("/DStar_PbPb_Analysis_V2/");
    
    // Configure fit options
    config.SetFitMethod(FitMethod::BinnedNLL);  // Use extended ML fit
    config.SetUseCUDA(useCUDA);
    config.SetVerbose(false);
    config.SetDoRefit(doReFit);
    
    // Configure selection cuts
    config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
    config.SetMVACut(0.99);
    
    // Add kinematic bins
    config.AddPtBin(pTMin, pTMax);
    config.AddCosBin(cosMin, cosMax);
    config.AddCentralityBin(centralityMin, centralityMax);  // Dummy centrality for now
    
    // Get all kinematic combinations
    auto allBins = config.GetAllKinematicBins();
    std::cout << "Analysis will run on " << allBins.size() << " kinematic bin(s)" << std::endl;
    
    // Process each kinematic bin
    for (const auto& bin : allBins) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Processing bin: " << bin.GetBinName() << std::endl;
        std::cout << "  pT range: [" << bin.pTMin << ", " << bin.pTMax << "] GeV/c" << std::endl;
        std::cout << "  cos(θ*) range: [" << bin.cosMin << ", " << bin.cosMax << "]" << std::endl;
        std::cout << "  Centrality range: [" << bin.centralityMin << ", " << bin.centralityMax << "]% (dummy)" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        try {
            // Create optimized fitter for this bin
            auto fitter = CreateDStarFitter(bin, config);
            
            // Get bin-specific parameters
            auto binParams = config.GetParametersForBin(bin);
            auto fitOpt = config.CreateFitOpt(bin);
            
            std::cout << "Cut expression: " << fitOpt.cutExpr << std::endl;
            std::cout << "Expected signal ratio: " << binParams.nsig_ratio << std::endl;
            std::cout << "Expected background ratio: " << binParams.nbkg_ratio << std::endl;
            
            // Load data
            std::cout << "Loading data from: " << config.GetDataFilePath() << std::endl;
            DataLoader loader(config.GetDataFilePath());
            
            if (!loader.loadRooDataSet(config.GetDatasetName())) {
                std::cerr << "Failed to load dataset: " << config.GetDatasetName() << std::endl;
                continue;
            }
            
            auto dataset = loader.getDataSet();
            if (!dataset) {
                std::cerr << "Dataset is null!" << std::endl;
                continue;
            }
            
            std::cout << "Original dataset size: " << dataset->numEntries() << " events" << std::endl;
            
            // Perform the fit
            std::cout << "Performing fit..." << std::endl;
            
            bool fitSuccess = false;
            if (doReFit) {
                // Use the new templated fitting interface
                fitSuccess = fitter->PerformFit(fitOpt, dataset, 
                                               binParams.signalParams, 
                                               binParams.backgroundParams,
                                               bin.GetBinName());
            } else {
                // Check if results already exist
                std::cout << "Skipping fit (doReFit=false). Loading existing results..." << std::endl;
                // In practice, you would load existing results here
                fitSuccess = true;  // Assume success for demo
            }
            
            if (fitSuccess) {
                std::cout << "\n✓ Fit successful for bin: " << bin.GetBinName() << std::endl;
                
                // Print fit results
                fitter->PrintSummary(bin.GetBinName());
                
                // Get fit quality metrics
                double signalYield = fitter->GetSignalYield(bin.GetBinName());
                double signalError = fitter->GetSignalYieldError(bin.GetBinName());
                double backgroundYield = fitter->GetBackgroundYield(bin.GetBinName());
                double chiSquare = fitter->GetReducedChiSquare(bin.GetBinName());
                bool isGoodFit = fitter->IsGoodFit(bin.GetBinName());
                
                std::cout << "Fit Results Summary:" << std::endl;
                std::cout << "  Signal yield: " << signalYield << " ± " << signalError << std::endl;
                std::cout << "  Background yield: " << backgroundYield << std::endl;
                std::cout << "  Reduced χ²: " << chiSquare << std::endl;
                std::cout << "  Fit quality: " << (isGoodFit ? "GOOD" : "POOR") << std::endl;
                
                // Calculate significance
                double significance = fitter->CalculateSignificance(bin.GetBinName());
                double purity = fitter->CalculatePurity(bin.GetBinName());
                
                std::cout << "  Significance: " << significance << " σ" << std::endl;
                std::cout << "  Purity: " << purity * 100 << "%" << std::endl;
                
                // Create plots if requested
                if (plotFit) {
                    std::cout << "Creating plots..." << std::endl;
                    
                    PlotOptions plotOptions;
                    plotOptions.title = "D* Meson Fit - " + bin.GetBinName();
                    plotOptions.xAxisTitle = "Δm = m(Kππ) - m(Kπ) [GeV/c²]";
                    plotOptions.yAxisTitle = "Events / (0.4 MeV/c²)";
                    plotOptions.drawComponents = true;
                    plotOptions.drawResiduals = true;
                    plotOptions.nbins = 100;
                    
                    auto canvas = fitter->CreateCanvas(bin.GetBinName(), plotOptions);
                    if (canvas) {
                        std::string plotName = fitOpt.outputDir + "/" + fitOpt.outputFile + "_plot.pdf";
                        canvas->SaveAs(plotName.c_str());
                        std::cout << "Plot saved: " << plotName << std::endl;
                    }
                }
                
                // Save fit results
                std::cout << "Saving fit results..." << std::endl;
                fitter->SaveResult(bin.GetBinName(), fitOpt.outputDir, fitOpt.outputFile + ".root", true);
                
                // Export results to text/JSON for further analysis
                fitter->ExportResults("json", fitOpt.outputDir + "/" + fitOpt.outputFile + "_results.json");
                
            } else {
                std::cerr << "✗ Fit failed for bin: " << bin.GetBinName() << std::endl;
                
                // Run diagnostics to understand why fit failed
                std::cout << "Running fit diagnostics..." << std::endl;
                fitter->RunDiagnostics(fitOpt.outputDir + "/" + fitOpt.outputFile + "_diagnostics.txt");
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Exception occurred for bin " << bin.GetBinName() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "D* Analysis V2 completed!" << std::endl;
    std::cout << "Results saved in: results/" << config.GetOutputSubDir() << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

/**
 * @brief Example of multi-bin analysis similar to original condor script
 */
void DStarMultiBinAnalysis(bool doReFit = false, bool plotFit = true, bool useCUDA = true) {
    std::cout << "=== D* Multi-Bin Analysis ===" << std::endl;
    
    DStarFitConfig config;
    
    // Configure for PbPb analysis
    config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    config.SetMCFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    config.SetOutputSubDir("/DStar_PbPb_MultiBin_V2/");
    
    config.SetFitMethod(FitMethod::Extended);
    config.SetUseCUDA(useCUDA);
    config.SetDoRefit(doReFit);
    
    // Add multiple pT bins like in original script
    config.AddPtBin(5.0, 7.0);   // Low pT
    config.AddPtBin(7.0, 10.0);  // Medium pT
    config.AddPtBin(10.0, 100.0); // High pT
    
    // Add cos(theta) bins
    config.AddCosBin(-2.0, 2.0);  // Full range
    // config.AddCosBin(-1.0, 0.0);  // Backward
    // config.AddCosBin(0.0, 1.0);   // Forward
    
    // Add centrality bins (dummy for now)
    config.AddCentralityBin(0, 10);   // Most central
    config.AddCentralityBin(10, 30);  // Semi-central
    config.AddCentralityBin(30, 50);  // Semi-peripheral
    config.AddCentralityBin(50, 100); // Peripheral
    
    // Set bin-specific parameters (like in original script)
    auto allBins = config.GetAllKinematicBins();
    for (const auto& bin : allBins) {
        DStarBinParameters params = config.GetParametersForBin(bin);  // Start with defaults
        
        // Customize parameters based on bin characteristics
        if (bin.pTMin >= 5 && bin.pTMax <= 7) {
            // Low pT: expect more signal, adjust yields
            params.nsig_ratio = 0.3;
            params.nsig_min_ratio = 0.2;
            params.nsig_max_ratio = 0.9;
            params.nbkg_ratio = 0.2;
            params.nbkg_min_ratio = 0.2;
            params.nbkg_max_ratio = 0.7;
        } else if (bin.pTMin >= 7 && bin.pTMax <= 10) {
            // Medium pT
            params.nsig_ratio = 0.3;
            params.nsig_min_ratio = 0.2;
            params.nsig_max_ratio = 0.9;
            params.nbkg_ratio = 0.1;
            params.nbkg_min_ratio = 0.01;
            params.nbkg_max_ratio = 0.7;
        }
        // High pT bins use default parameters
        
        config.SetParametersForBin(bin, params);
    }
    
    // Run the batch analysis
    RunDStarFitAnalysis(config, plotFit);
}

/**
 * @brief Example function for testing different fit methods
 */
void CompareFitMethods(float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2) {
    std::cout << "=== Comparing Different Fit Methods ===" << std::endl;
    
    std::vector<FitMethod> methods = {FitMethod::NLL, FitMethod::BinnedNLL, FitMethod::Extended};
    std::vector<std::string> methodNames = {"Unbinned NLL", "Binned NLL", "Extended ML"};
    
    for (size_t i = 0; i < methods.size(); ++i) {
        std::cout << "\n--- Testing " << methodNames[i] << " ---" << std::endl;
        
        DStarFitConfig config;
        config.SetFitMethod(methods[i]);
        config.SetOutputSubDir("/DStar_FitMethod_Comparison/");
        config.AddPtBin(pTMin, pTMax);
        config.AddCosBin(cosMin, cosMax);
        
        // Set binning for binned fit
        if (methods[i] == FitMethod::BinnedNLL) {
            auto fitConfig = config.CreateFitConfig();
            fitConfig.histogramBins = 50;  // Fewer bins for binned fit
        }
        
        try {
            RunDStarFitAnalysis(config, true);
        } catch (const std::exception& e) {
            std::cerr << "Method " << methodNames[i] << " failed: " << e.what() << std::endl;
        }
    }
}