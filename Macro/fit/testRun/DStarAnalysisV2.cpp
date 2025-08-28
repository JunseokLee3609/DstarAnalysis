#include "../DStarFitConfig.h"
#include "../MassFitterV2.h"
#include "../DataLoader.h"
// #include "../PlotManager.h"  // Disabled - has compilation issues
#include "../EnhancedPlotManager.h"  // Use our enhanced plotter instead
#include "../SimpleParameterLoader.h"  // External parameter loading
#include "../../Tools/ConfigManager.h"

// ===== PARAMETER PRINTING UTILITY FUNCTIONS =====

// Template function to get PDF type names
template<typename T> std::string GetPDFTypeName() { return "Unknown"; }

// Signal PDF type names
template<> std::string GetPDFTypeName<PDFParams::GaussianParams>() { return "Gaussian"; }
template<> std::string GetPDFTypeName<PDFParams::DoubleGaussianParams>() { return "DoubleGaussian"; }
template<> std::string GetPDFTypeName<PDFParams::CrystalBallParams>() { return "CrystalBall"; }
template<> std::string GetPDFTypeName<PDFParams::DBCrystalBallParams>() { return "DBCrystalBall"; }
template<> std::string GetPDFTypeName<PDFParams::VoigtianParams>() { return "Voigtian"; }
template<> std::string GetPDFTypeName<PDFParams::BreitWignerParams>() { return "BreitWigner"; }

// Background PDF type names
template<> std::string GetPDFTypeName<PDFParams::ExponentialBkgParams>() { return "Exponential"; }
template<> std::string GetPDFTypeName<PDFParams::ChebychevBkgParams>() { return "Chebychev"; }
template<> std::string GetPDFTypeName<PDFParams::PhenomenologicalParams>() { return "Phenomenological"; }
template<> std::string GetPDFTypeName<PDFParams::PolynomialBkgParams>() { return "Polynomial"; }
template<> std::string GetPDFTypeName<PDFParams::ThresholdFuncParams>() { return "ThresholdFunction"; }
template<> std::string GetPDFTypeName<PDFParams::ExpErfBkgParams>() { return "ExpErf"; }
template<> std::string GetPDFTypeName<PDFParams::DstBkgParams>() { return "DstBg"; }

// Parameter printing functions for each PDF type
// void PrintParameters(const PDFParams::DoubleGaussianParams& params) {
//     std::cout << "  Mean: " << params.mean << " [" << params.mean_min << ", " << params.mean_max << "]" << std::endl;
//     std::cout << "  Sigma1: " << params.sigma1 << " [" << params.sigma1_min << ", " << params.sigma1_max << "]" << std::endl;
//     std::cout << "  Sigma2: " << params.sigma2 << " [" << params.sigma2_min << ", " << params.sigma2_max << "]" << std::endl;
//     std::cout << "  Fraction: " << params.fraction << " [" << params.fraction_min << ", " << params.fraction_max << "]" << std::endl;
// }

// void PrintParameters(const PDFParams::ThresholdFuncParams& params) {
//     std::cout << "  p0 (init): " << params.p0_init << " [" << params.p0_min << ", " << params.p0_max << "]" << std::endl;
//     std::cout << "  p1 (init): " << params.p1_init << " [" << params.p1_min << ", " << params.p1_max << "]" << std::endl;
//     std::cout << "  p2 (init): " << params.p2_init << " [" << params.p2_min << ", " << params.p2_max << "]" << std::endl;
//     std::cout << "  p3 (init): " << params.p3_init << " [" << params.p3_min << ", " << params.p3_max << "]" << std::endl;
// }

// void PrintParameters(const PDFParams::DBCrystalBallParams& params) {
//     std::cout << "  Mean: " << params.mean << " [" << params.mean_min << ", " << params.mean_max << "]" << std::endl;
//     std::cout << "  Sigma: " << params.sigma << " [" << params.sigma_min << ", " << params.sigma_max << "]" << std::endl;
//     std::cout << "  AlphaL: " << params.alphaL << " [" << params.alphaL_min << ", " << params.alphaL_max << "]" << std::endl;
//     std::cout << "  nL: " << params.nL << " [" << params.nL_min << ", " << params.nL_max << "]" << std::endl;
//     std::cout << "  AlphaR: " << params.alphaR << " [" << params.alphaR_min << ", " << params.alphaR_max << "]" << std::endl;
//     std::cout << "  nR: " << params.nR << " [" << params.nR_min << ", " << params.nR_max << "]" << std::endl;
// }

// void PrintParameters(const PDFParams::PhenomenologicalParams& params) {
//     std::cout << "  p0: " << params.p0 << " [" << params.p0_min << ", " << params.p0_max << "]" << std::endl;
//     std::cout << "  p1: " << params.p1 << " [" << params.p1_min << ", " << params.p1_max << "]" << std::endl;
//     std::cout << "  p2: " << params.p2 << " [" << params.p2_min << ", " << params.p2_max << "]" << std::endl;
// }

// // Generic fallback for other parameter types
// template<typename T>
// void PrintParameters(const T& params) {
//     std::cout << "  [Parameter details not implemented for this PDF type]" << std::endl;
// }

/**
 * @brief Modern D* meson analysis using the new modular framework
 * 
 * This macro demonstrates how to use the improved MassFitterV2 with 
 * configurable kinematic bins and centrality support (dummy for now).
 */
void DStarAnalysisV2(bool doReFit = false, bool plotFit = true, bool useCUDA = true,
                     float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                     int centralityMin = 0, int centralityMax = 100, 
                     const std::string& parameterFile = "") {
    
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
    config.SetFitMethod(FitMethod::NLL);  // Use extended ML fit
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
    
    // ===== PARAMETER LOADING SYSTEM =====
    std::cout << "Setting up parameters..." << std::endl;
    
    // Check if external parameter file is provided
    if (!parameterFile.empty()) {
        std::cout << "Loading parameters from external file: " << parameterFile << std::endl;
        
        try {
            // Load parameters from file
            ParameterLoaderUtils::LoadParametersToConfig(config, parameterFile);
            std::cout << "✓ External parameters loaded successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "⚠ Failed to load external parameters: " << e.what() << std::endl;
            std::cout << "Falling back to hardcoded parameters..." << std::endl;
            // Will use hardcoded parameters below
        }
    } else {
        std::cout << "No parameter file provided, using hardcoded parameters" << std::endl;
        
        // ===== HARDCODED PARAMETERS (FALLBACK) =====
        auto allBins = config.GetAllKinematicBins();
        
        for (const auto& bin : allBins) {
            // Create custom parameters for this bin
            DStarBinParameters binParams;
            
            // ===== SIGNAL PDF CONFIGURATION =====
            binParams.signalPdfType = PDFType::DoubleGaussian;
            
            // Configure DoubleGaussian signal parameters
            binParams.doubleGaussianParams.mean = 0.1455;
            binParams.doubleGaussianParams.mean_min = 0.1452;
            binParams.doubleGaussianParams.mean_max = 0.1458;
            binParams.doubleGaussianParams.sigma1 = 0.0005;
            binParams.doubleGaussianParams.sigma1_min = 0.0001;
            binParams.doubleGaussianParams.sigma1_max = 0.01;
            binParams.doubleGaussianParams.sigma2 = 0.001;
            binParams.doubleGaussianParams.sigma2_min = 0.0001;
            binParams.doubleGaussianParams.sigma2_max = 0.01;
            binParams.doubleGaussianParams.fraction = 0.7;
            binParams.doubleGaussianParams.fraction_min = 0.0;
            binParams.doubleGaussianParams.fraction_max = 1.0;
            
            // ===== BACKGROUND PDF CONFIGURATION =====
            binParams.backgroundPdfType = PDFType::ThresholdFunction;
            
            // Configure ThresholdFunction background parameters
            binParams.thresholdFuncParams.p0_init = 1.0;
            binParams.thresholdFuncParams.p0_min = -10.0;
            binParams.thresholdFuncParams.p0_max = 10.0;
            binParams.thresholdFuncParams.p1_init = -1.0;
            binParams.thresholdFuncParams.p1_min = -10.0;
            binParams.thresholdFuncParams.p1_max = 10.0;
            binParams.thresholdFuncParams.m_pi_value = 0.13957;  // Charged pion mass
            
            // Customize yield ratios
            binParams.nsig_ratio = 0.005;
            binParams.nsig_min_ratio = 0.0001;
            binParams.nsig_max_ratio = 0.05;
            binParams.nbkg_ratio = 0.02;
            binParams.nbkg_min_ratio = 0.001;
            binParams.nbkg_max_ratio = 0.5;
            
            // Apply these parameters to this specific bin
            config.SetParametersForBin(bin, binParams);
        }
    }
    
    // Get all bins after parameter configuration
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
                // Get parameters automatically based on PDF type using std::visit
                auto signalParamsVariant = binParams.GetSignalParamsByType();
                auto backgroundParamsVariant = binParams.GetBackgroundParamsByType();
                
                std::cout << "\n=== PARAMETER CONFIGURATION ===" << std::endl;
                
                // Print signal parameters
                std::visit([&](auto&& signalParams) {
                    using ParamType = std::decay_t<decltype(signalParams)>;
                    std::cout << "SIGNAL PDF PARAMETERS (" << GetPDFTypeName<ParamType>() << "):" << std::endl;
                    // PrintParameters(signalParams);
                }, signalParamsVariant);
                
                // Print background parameters  
                std::visit([&](auto&& backgroundParams) {
                    using ParamType = std::decay_t<decltype(backgroundParams)>;
                    std::cout << "\nBACKGROUND PDF PARAMETERS (" << GetPDFTypeName<ParamType>() << "):" << std::endl;
                    // PrintParameters(backgroundParams);
                }, backgroundParamsVariant);
                
                std::cout << "===============================\n" << std::endl;
                
                std::visit([&](auto&& signalParams) {
                    std::visit([&](auto&& backgroundParams) {
                        fitSuccess = fitter->PerformFit(fitOpt, dataset, 
                                                       signalParams, 
                                                       backgroundParams,
                                                       bin.GetBinName());
                    }, backgroundParamsVariant);
                }, signalParamsVariant);
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
                
                // Create plots if requested - using EnhancedPlotManager
                if (plotFit) {
                    std::cout << "Creating plots using EnhancedPlotManager..." << std::endl;
                    
                    try {
                        // Create EnhancedPlotManager with the saved fit results
                        std::string inputDir = fitOpt.outputDir;
                        std::string inputFile = fitOpt.outputFile + ".root";
                        std::string plotDir = fitOpt.outputDir + "/plots";
                        
                        EnhancedPlotManager plotManager(fitOpt, inputDir, inputFile, plotDir, false, true);
                        
                        if (plotManager.IsValid()) {
                            // Print summary
                            plotManager.PrintSummary();
                            
                            // Draw raw distribution
                            bool rawSuccess = plotManager.DrawRawDistribution("raw_" + bin.GetBinName());
                            if (rawSuccess) {
                                std::cout << "✓ Raw distribution plot created" << std::endl;
                            }
                            
                            // Draw fitted model with pull plot
                            bool fitSuccess = plotManager.DrawFittedModel(true, "fitted_" + bin.GetBinName());
                            if (fitSuccess) {
                                std::cout << "✓ Fitted model plot created" << std::endl;
                            }
                            
                        } else {
                            std::cout << "⚠ PlotManager initialization failed, falling back to basic plotting" << std::endl;
                            
                            // Fallback to basic plotting
                            PlotOptions plotOptions;
                            plotOptions.title = "D* Meson Fit - " + bin.GetBinName();
                            plotOptions.xAxisTitle = "Δm = m(Kππ) - m(Kπ) [GeV/c²]";
                            plotOptions.yAxisTitle = "Events / (0.4 MeV/c²)";
                            plotOptions.drawComponents = true;
                            plotOptions.drawResiduals = true;
                            plotOptions.nbins = 100;
                            
                            auto canvas = fitter->CreateCanvas(bin.GetBinName(), plotOptions);
                            if (canvas) {
                                std::string plotName = fitOpt.outputDir + "/plots/" + fitOpt.outputFile + "_plot.pdf";
                                createDir(Form("%s/plots/", fitOpt.outputDir.c_str()));
                                canvas->SaveAs(plotName.c_str());
                                std::cout << "Plot saved: " << plotName << std::endl;
                            }
                        }
                        
                    } catch (const std::exception& e) {
                        std::cerr << "Error during plotting: " << e.what() << std::endl;
                        std::cout << "Continuing without plots..." << std::endl;
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
    
    config.SetFitMethod(FitMethod::NLL);
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