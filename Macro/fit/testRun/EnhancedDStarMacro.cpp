#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
#include "../DCAFitter.h"
#include "../../Tools/ConfigManager.h"
#include <ctime>

// Forward declarations
std::string getCurrentTime();
void setupDStarParameters(std::pair<DBCrystalBallParams, PhenomenologicalParams>& paramPair,
                         float pTMin, float pTMax, float cosMin, float cosMax);
void performDataQualityChecks(RooDataSet* data, RooDataSet* mc, const FitOpt& opt);

void EnhancedDStarMacro(bool doReFit = false, bool plotFit = true, bool useCUDA = true, 
                       float pTMin = 4, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                       bool performDCAFit = true, bool performSPlot = false) {
    
    cout << "========== Enhanced D* Meson Analysis Framework ==========" << endl;
    cout << "pT range: [" << pTMin << ", " << pTMax << "] GeV" << endl;
    cout << "cos(θ) range: [" << cosMin << ", " << cosMax << "]" << endl;
    cout << "Features: Mass Fit, DCA Fit, Plotting, Data Quality Checks" << endl;
    cout << "=========================================================" << endl;

    // ============ Configuration Setup ============
    FitOpt opt;
    opt.useCUDA = useCUDA;
    opt.doFit = doReFit;
    
    // Data file paths
    std::string filepathMC = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_noPreselectionCut_ppRef_Jul28_v1.root";
    std::string filepathData = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_noPreselectionCut_ppRef_Aug01_0p5_v1.root";
    std::string datasetName = "datasetHX";
    
    // Output directory setup with timestamp
    std::string timestamp = getCurrentTime();
    std::string subDir = "/Enhanced_DStar_Analysis_" + timestamp + "/";
    
    // Define kinematic bins
    std::vector<std::pair<double,double>> ptBins = {{pTMin, pTMax}};
    std::vector<std::pair<double,double>> cosBins = {{cosMin, cosMax}};
    
    // Selection cuts using ConfigManager
    std::string slowPionCut = SelectionCuts::getSlowPionCuts();
    std::string grandDaughterCut = SelectionCuts::getGrandDaughterCuts();
    
    // ============ Parameter Configuration ============
    // Enhanced parameter setup with adaptive settings based on pT range
    double nsig_ratio, nsig_min_ratio, nsig_max_ratio;
    double nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio;
    
    // Adaptive parameter setup
    if (pTMin >= 4 && pTMax <= 7) {
        // Low pT: more conservative parameters
        nsig_ratio = 0.3; nsig_min_ratio = 0.2; nsig_max_ratio = 0.9;
        nbkg_ratio = 0.2; nbkg_min_ratio = 0.1; nbkg_max_ratio = 0.8;
    } else if (pTMin >= 7 && pTMax <= 15) {
        // Medium pT: balanced parameters
        nsig_ratio = 0.25; nsig_min_ratio = 0.15; nsig_max_ratio = 0.85;
        nbkg_ratio = 0.15; nbkg_min_ratio = 0.05; nbkg_max_ratio = 0.7;
    } else {
        // High pT: optimized for higher statistics
        nsig_ratio = 0.2; nsig_min_ratio = 0.1; nsig_max_ratio = 0.8;
        nbkg_ratio = 0.1; nbkg_min_ratio = 0.02; nbkg_max_ratio = 0.6;
    }
    
    // Get parameter maps with enhanced PDF configurations
    auto params = DStarParamMaker1({0}, {0});
    auto params2 = DStarParamMaker3({0}, {0});
    
    // Enhanced parameter tuning based on kinematic range
    setupDStarParameters(params[{0,0}], pTMin, pTMax, cosMin, cosMax);
    
    // ============ Data Loading ============
    cout << "Loading data files..." << endl;
    DataLoader MCloader(filepathMC);
    DataLoader Dataloader(filepathData);
    
    if (!MCloader.loadRooDataSet(datasetName) || !Dataloader.loadRooDataSet(datasetName)) {
        cerr << "Error: Failed to load datasets!" << endl;
        return;
    }
    
    cout << "MC entries: " << MCloader.getDataSet()->sumEntries() << endl;
    cout << "Data entries: " << Dataloader.getDataSet()->sumEntries() << endl;
    
    // ============ Analysis Loop ============
    for (auto ptbin : ptBins) {
        for (auto cosbin : cosBins) {
            cout << "\n========== Processing Bin: pT[" << ptbin.first << "," << ptbin.second 
                 << "], cos[" << cosbin.first << "," << cosbin.second << "] ==========" << endl;
            
            // Configure fit options for current bin
            opt.pTMin = ptbin.first;
            opt.pTMax = ptbin.second;
            opt.cosMin = cosbin.first;
            opt.cosMax = cosbin.second;
            
            opt.DStarMCAbsDefault();
            opt.outputDir = opt.outputDir + subDir;
            opt.outputMCDir = opt.outputMCDir + subDir;
            opt.outputPlotDir = opt.outputPlotDir + subDir;
            
            // Enhanced cut expressions
            opt.cutMCExpr = opt.cutMCExpr + "&&" + slowPionCut + "&&" + grandDaughterCut;
            opt.cutExpr = opt.cutExpr + "&&" + slowPionCut + "&&" + grandDaughterCut;
            
            BinInfo currentBin = createBinInfoFromFitOpt(opt);
            
            // ============ Mass Fitting ============
            if (doReFit) {
                cout << "Performing enhanced mass fitting..." << endl;
                MassFitter fitter(opt.name, opt.massVar, opt.massMin, opt.massMax,
                                nsig_ratio, nsig_min_ratio, nsig_max_ratio, 
                                nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio);
                
                try {
                    // Perform constraint fit with enhanced error handling
                    fitter.PerformConstraintFit(opt, Dataloader.getDataSet(), MCloader.getDataSet(), 
                                              true, "", "", params[{0,0}].first, params[{0,0}].second);
                    
                    RooFitResult* fitResult = fitter.GetFitResult();
                    if (fitResult) {
                        checkAndRecordFitStatus(fitResult, currentBin, "Mass", "Enhanced fit");
                        cout << "Mass fit successful! EDM = " << fitResult->edm() << ", Status = " << fitResult->status() << endl;
                    }
                } catch (const std::exception& e) {
                    cerr << "Mass fitting error: " << e.what() << endl;
                }
            }
            
            // ============ Plotting and Visualization ============
            cout << "Creating enhanced plots..." << endl;
            PlotManager plotManager(opt, opt.outputDir, opt.outputFile, 
                                  "plots/Enhanced_Data_DStar/" + subDir, true, true);
            PlotManager plotManagerMC(opt, opt.outputMCDir, opt.outputMCFile, 
                                    "plots/Enhanced_MC_DStar/" + subDir, true, true);
            
            if (!plotFit) {
                plotManagerMC.DrawRawDistribution();
                plotManager.DrawRawDistribution();
            } else {
                plotManagerMC.DrawFittedModel(true);
                plotManager.DrawFittedModel(true);
            }
            
            // ============ DCA Analysis ============
            if (performDCAFit) {
                cout << "Performing DCA significance analysis..." << endl;
                
                // Setup for D0 daughter mass fitting
                opt.massVar = "massDaugther1";
                opt.massMin = 1.75;
                opt.massMax = 2.00;
                
                DCAFitter DCAfitter(opt, "enhancedDCAFitter", opt.massVar, 
                                  opt.dcaMin, opt.dcaMax, 100);
                
                // Configure DCA fitter
                DCAfitter.setMCFile((opt.outputMCDir + "/" + opt.outputMCFile).c_str(), "reducedData");
                DCAfitter.setDataFile((opt.outputDir + "/" + opt.outputFile).c_str(), "reducedData");
                DCAfitter.setDCABranchName("dca3D");
                DCAfitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_");
                
                // Enhanced prompt/non-prompt categorization
                DCAfitter.setPromptPdgIds({4, 2});
                DCAfitter.setNonPromptPdgIds({5});
                DCAfitter.setOutputFile(opt.outputDir + "/" + opt.outputDCAFile);
                
                // Perform DCA analysis with enhanced error handling
                try {
                    if (DCAfitter.createTemplatesFromMC()) {
                        DCAfitter.plotRawDataDistribution(opt.outputPlotDir + opt.outputDCAFile + "_Enhanced_Templates");
                        
                        if (DCAfitter.loadData() && DCAfitter.buildModelwSideband()) {
                            RooFitResult* dcaFitResult = DCAfitter.performFit(true);
                            
                            if (dcaFitResult) {
                                DCAfitter.plotResults(dcaFitResult, opt.outputPlotDir + opt.outputDCAFile + "_Enhanced_Results");
                                DCAfitter.saveResults(dcaFitResult);
                                delete dcaFitResult;
                                cout << "DCA analysis completed successfully!" << endl;
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    cerr << "DCA fitting error: " << e.what() << endl;
                }
            }
            
            // ============ Data Quality Assessment ============
            cout << "Performing data quality checks..." << endl;
            performDataQualityChecks(Dataloader.getDataSet(), MCloader.getDataSet(), opt);
            
            cout << "Bin analysis completed!" << endl;
        }
    }
    
    // ============ Final Summary ============
    cout << "\n========== Analysis Summary ==========" << endl;
    printFailedFits();
    
    std::string analysisId = Form("Enhanced_pT_%.1f_%.1f_cos_%.3f_%.3f", pTMin, pTMax, cosMin, cosMax);
    std::string statusFileName = opt.outputDir + "/enhanced_fit_status_" + analysisId + ".csv";
    saveFitStatusToFile(statusFileName);
    
    cout << "Enhanced D* meson analysis completed successfully!" << endl;
    cout << "Results saved in: " << opt.outputDir << endl;
    cout << "Status summary: " << statusFileName << endl;
    cout << "=======================================" << endl;
}

// Enhanced helper function for parameter setup
void setupDStarParameters(std::pair<DBCrystalBallParams, PhenomenologicalParams>& paramPair,
                         float pTMin, float pTMax, float cosMin, float cosMax) {
    auto& sigParams = paramPair.first;
    auto& bkgParams = paramPair.second;
    
    // D* mass configuration with enhanced precision
    sigParams.mean = 0.1455;
    sigParams.mean_min = 0.1452;
    sigParams.mean_max = 0.1458;
    
    // Adaptive sigma based on pT range
    if (pTMin >= 4 && pTMax <= 7) {
        sigParams.sigma = 0.0006;
        sigParams.sigmaR = 0.0006;
        sigParams.sigmaL = 0.0006;
    } else if (pTMin >= 7 && pTMax <= 15) {
        sigParams.sigma = 0.0005;
        sigParams.sigmaR = 0.0005;
        sigParams.sigmaL = 0.0005;
    } else {
        sigParams.sigma = 0.0004;
        sigParams.sigmaR = 0.0004;
        sigParams.sigmaL = 0.0004;
    }
    
    // Common sigma ranges
    sigParams.sigma_min = 0.0001;
    sigParams.sigma_max = 0.01;
    sigParams.sigmaR_min = 0.00001;
    sigParams.sigmaR_max = 0.01;
    sigParams.sigmaL_min = 0.00001;
    sigParams.sigmaL_max = 0.01;
    
    // Crystal Ball tail parameters
    sigParams.alphaL = 1.1;
    sigParams.alphaL_min = 0.5;
    sigParams.alphaL_max = 2.0;
    sigParams.nL = 1.1;
    sigParams.nL_min = 1.0;
    sigParams.nL_max = 100.0;
    
    sigParams.alphaR = 1.0;
    sigParams.alphaR_min = 0.5;
    sigParams.alphaR_max = 2.0;
    sigParams.nR = 1.1;
    sigParams.nR_min = 1.0;
    sigParams.nR_max = 20.0;
    
    // Background parameters
    bkgParams.p0 = 0.01;
    bkgParams.p0_min = 0.001;
    bkgParams.p0_max = 1.0;
    bkgParams.p1 = 1.0;
    bkgParams.p1_min = -20.0;
    bkgParams.p1_max = 20.0;
    bkgParams.p2 = 1.0;
    bkgParams.p2_min = -20.0;
    bkgParams.p2_max = 20.0;
}

// Enhanced data quality check function
void performDataQualityChecks(RooDataSet* data, RooDataSet* mc, const FitOpt& opt) {
    cout << "Data Quality Assessment:" << endl;
    cout << "- Data entries: " << data->sumEntries() << endl;
    cout << "- MC entries: " << mc->sumEntries() << endl;
    
    // Basic statistics
    if (data->sumEntries() < 1000) {
        cout << "WARNING: Low data statistics (< 1000 events)" << endl;
    }
    if (mc->sumEntries() < 5000) {
        cout << "WARNING: Low MC statistics (< 5000 events)" << endl;
    }
    
    // Check for reasonable data/MC ratio
    double ratio = data->sumEntries() / mc->sumEntries();
    if (ratio < 0.01 || ratio > 100) {
        cout << "WARNING: Unusual Data/MC ratio: " << ratio << endl;
    }
    
    cout << "- Data/MC ratio: " << ratio << endl;
    cout << "Data quality check completed." << endl;
}

// Utility function to get current timestamp
std::string getCurrentTime() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", timeinfo);
    return std::string(buffer);
}