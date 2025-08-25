#ifndef DSTAR_FIT_CONFIG_H
#define DSTAR_FIT_CONFIG_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "MassFitterV2.h"
#include "Params.h"
#include "Opt.h"
#include "FitStrategy.h"

/**
 * @brief Kinematic bin configuration for D* meson analysis
 */
struct KinematicBin {
    double pTMin;
    double pTMax;
    double cosMin;
    double cosMax;
    int centralityMin = 0;    // Dummy for future use
    int centralityMax = 100;  // Dummy for future use
    
    KinematicBin(double pt_min, double pt_max, double cos_min, double cos_max,
                 int cent_min = 0, int cent_max = 100)
        : pTMin(pt_min), pTMax(pt_max), cosMin(cos_min), cosMax(cos_max),
          centralityMin(cent_min), centralityMax(cent_max) {}
    
    std::string GetBinName() const {
        return "pT_" + std::to_string(pTMin) + "_" + std::to_string(pTMax) + 
               "_cos_" + std::to_string(cosMin) + "_" + std::to_string(cosMax) +
               "_cent_" + std::to_string(centralityMin) + "_" + std::to_string(centralityMax);
    }
    
    std::string GetCutString() const {
        std::string cut = "pT > " + std::to_string(pTMin) + " && pT < " + std::to_string(pTMax);
        if (cosMin > -2.0 || cosMax < 2.0) {
            cut += " && cosThetaHX > " + std::to_string(cosMin) + " && cosThetaHX < " + std::to_string(cosMax);
        }
        // Future: add centrality cut when needed
        // cut += " && centrality >= " + std::to_string(centralityMin) + " && centrality < " + std::to_string(centralityMax);
        return cut;
    }
};

/**
 * @brief Parameter configuration for different pT and cos(theta) bins
 */
struct DStarBinParameters {
    // Yield parameters
    double nsig_ratio = 0.01;
    double nsig_min_ratio = 0.0;
    double nsig_max_ratio = 1.0;
    double nbkg_ratio = 0.01;
    double nbkg_min_ratio = 0.0;
    double nbkg_max_ratio = 1.0;
    
    // Signal PDF parameters (DBCrystalBall)
    PDFParams::DBCrystalBallParams signalParams;
    
    // Background PDF parameters (Phenomenological)
    PDFParams::PhenomenologicalParams backgroundParams;
    
    // Constructor with defaults
    DStarBinParameters() {
        SetDefaultSignalParams();
        SetDefaultBackgroundParams();
    }
    
private:
    void SetDefaultSignalParams() {
        signalParams.mean = 0.1455;
        signalParams.mean_min = 0.1452;
        signalParams.mean_max = 0.1458;
        signalParams.sigma = 0.0005;
        signalParams.sigma_min = 0.0001;
        signalParams.sigma_max = 0.01;
        signalParams.alphaL = 1.1;
        signalParams.alphaL_min = 0.5;
        signalParams.alphaL_max = 2.0;
        signalParams.nL = 1.1;
        signalParams.nL_min = 1.0;
        signalParams.nL_max = 100.0;
        signalParams.alphaR = 1.0;
        signalParams.alphaR_min = 0.5;
        signalParams.alphaR_max = 2.0;
        signalParams.nR = 1.1;
        signalParams.nR_min = 1.0;
        signalParams.nR_max = 20.0;
    }
    
    void SetDefaultBackgroundParams() {
        backgroundParams.p0 = 0.01;
        backgroundParams.p0_min = 0.001;
        backgroundParams.p0_max = 1.0;
        backgroundParams.p1 = 1.0;
        backgroundParams.p1_min = -20.0;
        backgroundParams.p1_max = 20.0;
        backgroundParams.p2 = 1.0;
        backgroundParams.p2_min = -20.0;
        backgroundParams.p2_max = 20.0;
    }
};

/**
 * @brief Main configuration class for D* meson fitting
 */
class DStarFitConfig {
public:
    DStarFitConfig() {
        SetupDefaultConfiguration();
    }
    
    // Add kinematic bins
    void AddPtBin(double pTMin, double pTMax) {
        ptBins_.emplace_back(pTMin, pTMax);
    }
    
    void AddCosBin(double cosMin, double cosMax) {
        cosBins_.emplace_back(cosMin, cosMax);
    }
    
    void AddCentralityBin(int centMin, int centMax) {
        centralityBins_.emplace_back(centMin, centMax);
    }
    
    // Get all kinematic combinations
    std::vector<KinematicBin> GetAllKinematicBins() const {
        std::vector<KinematicBin> allBins;
        
        for (const auto& ptBin : ptBins_) {
            for (const auto& cosBin : cosBins_) {
                for (const auto& centBin : centralityBins_) {
                    allBins.emplace_back(ptBin.first, ptBin.second,
                                        cosBin.first, cosBin.second,
                                        centBin.first, centBin.second);
                }
            }
        }
        
        return allBins;
    }
    
    // Set parameters for specific bins
    void SetParametersForBin(const KinematicBin& bin, const DStarBinParameters& params) {
        binParameters_[bin.GetBinName()] = params;
    }
    
    // Get parameters for a specific bin
    DStarBinParameters GetParametersForBin(const KinematicBin& bin) const {
        auto it = binParameters_.find(bin.GetBinName());
        if (it != binParameters_.end()) {
            return it->second;
        }
        return GetDefaultParameters(bin);  // Return optimized defaults
    }
    
    // Configuration for file paths
    void SetDataFilePath(const std::string& path) { dataFilePath_ = path; }
    void SetMCFilePath(const std::string& path) { mcFilePath_ = path; }
    void SetDatasetName(const std::string& name) { datasetName_ = name; }
    void SetOutputSubDir(const std::string& subdir) { outputSubDir_ = subdir; }
    
    std::string GetDataFilePath() const { return dataFilePath_; }
    std::string GetMCFilePath() const { return mcFilePath_; }
    std::string GetDatasetName() const { return datasetName_; }
    std::string GetOutputSubDir() const { return outputSubDir_; }
    
    // Fit configuration
    void SetFitMethod(FitMethod method) { fitMethod_ = method; }
    void SetUseCUDA(bool use) { useCUDA_ = use; }
    void SetVerbose(bool verbose) { verbose_ = verbose; }
    void SetDoRefit(bool refit) { doRefit_ = refit; }
    
    FitMethod GetFitMethod() const { return fitMethod_; }
    bool GetUseCUDA() const { return useCUDA_; }
    bool GetVerbose() const { return verbose_; }
    bool GetDoRefit() const { return doRefit_; }
    
    // Selection cuts
    void SetSlowPionCut(const std::string& cut) { slowPionCut_ = cut; }
    void SetGrandDaughterCut(const std::string& cut) { grandDaughterCut_ = cut; }
    void SetMVACut(double mvaMin) { mvaCut_ = "mva > " + std::to_string(mvaMin); }
    
    std::string GetSlowPionCut() const { return slowPionCut_; }
    std::string GetGrandDaughterCut() const { return grandDaughterCut_; }
    std::string GetMVACut() const { return mvaCut_; }
    std::string GetFullCutString() const {
        std::string fullCut = slowPionCut_;
        if (!grandDaughterCut_.empty()) {
            fullCut += " && " + grandDaughterCut_;
        }
        if (!mvaCut_.empty()) {
            fullCut += " && " + mvaCut_;
        }
        return fullCut;
    }
    
    // Create FitOpt for a specific bin
    FitOpt CreateFitOpt(const KinematicBin& bin) const {
        FitOpt opt;
        
        // Set kinematic range
        opt.pTMin = bin.pTMin;
        opt.pTMax = bin.pTMax;
        opt.cosMin = bin.cosMin;
        opt.cosMax = bin.cosMax;
        
        // Set cuts
        opt.cutExpr = GetFullCutString() + " && " + bin.GetCutString();
        opt.cutMCExpr = opt.cutExpr;  // Same cuts for MC
        
        // Set fit configuration
        opt.useCUDA = useCUDA_;
        opt.doFit = doRefit_;
        opt.verbose = verbose_;
        opt.useMinos = false;  // Usually not needed for D* fits
        opt.useHesse = true;
        
        // Set output configuration
        opt.outputDir = "results";
        opt.outputFile = "DStar_fit_" + bin.GetBinName();
        opt.subDir = outputSubDir_;
        
        // Set mass range for D* (delta mass)
        opt.massVar = "massDifference";
        opt.massMin = 0.139;
        opt.massMax = 0.180;
        
        return opt;
    }
    
    // Create FitConfig for modern fitting framework
    FitConfig CreateFitConfig() const {
        FitConfig config;
        config.fitMethod = fitMethod_;
        config.useCUDA = useCUDA_;
        config.verbose = verbose_;
        config.useMinos = false;
        config.useHesse = true;
        config.numCPU = 24;
        config.maxRetries = 3;
        config.histogramBins = 100;
        return config;
    }
    
private:
    // Kinematic bins
    std::vector<std::pair<double, double>> ptBins_;
    std::vector<std::pair<double, double>> cosBins_;
    std::vector<std::pair<int, int>> centralityBins_;
    
    // Parameters for each bin
    std::map<std::string, DStarBinParameters> binParameters_;
    
    // File paths
    std::string dataFilePath_;
    std::string mcFilePath_;
    std::string datasetName_ = "datasetHX";
    std::string outputSubDir_;
    
    // Fit configuration
    FitMethod fitMethod_ = FitMethod::Extended;
    bool useCUDA_ = true;
    bool verbose_ = false;
    bool doRefit_ = false;
    
    // Selection cuts
    std::string slowPionCut_;
    std::string grandDaughterCut_;
    std::string mvaCut_;
    
    void SetupDefaultConfiguration() {
        // Default centrality bin (dummy for now)
        centralityBins_.emplace_back(0, 100);
        
        // Default cuts (can be overridden)
        slowPionCut_ = "pTD2 > 0.3 && abs(EtaD2) < 2.4";
        grandDaughterCut_ = "abs(EtaD2) < 1.2 && abs(EtaGrandD1) < 1.2 && abs(EtaGrandD2) < 1.2 && massDaugther1 > 1.82 && massDaugther1 < 1.92";
        mvaCut_ = "mva > 0.99";
        
        // Default output subdirectory
        outputSubDir_ = "/DStar_PbPb_Analysis/";
    }
    
    // Get optimized default parameters based on pT bin
    DStarBinParameters GetDefaultParameters(const KinematicBin& bin) const {
        DStarBinParameters params;
        
        // Optimize parameters based on pT range
        if (bin.pTMin >= 5 && bin.pTMax <= 7) {
            // Low pT bin - more signal expected
            params.nsig_ratio = 0.3;
            params.nsig_min_ratio = 0.2;
            params.nsig_max_ratio = 0.9;
            params.nbkg_ratio = 0.2;
            params.nbkg_min_ratio = 0.2;
            params.nbkg_max_ratio = 0.7;
        }
        else if (bin.pTMin >= 7 && bin.pTMax <= 10) {
            // Medium pT bin
            params.nsig_ratio = 0.3;
            params.nsig_min_ratio = 0.2;
            params.nsig_max_ratio = 0.9;
            params.nbkg_ratio = 0.1;
            params.nbkg_min_ratio = 0.01;
            params.nbkg_max_ratio = 0.7;
        }
        else if (bin.pTMin >= 10) {
            // High pT bin - less background
            params.nsig_ratio = 0.01;
            params.nsig_min_ratio = 0.0;
            params.nsig_max_ratio = 1.0;
            params.nbkg_ratio = 0.01;
            params.nbkg_min_ratio = 0.0;
            params.nbkg_max_ratio = 1.0;
        }
        
        return params;
    }
};

/**
 * @brief Factory function to create a configured MassFitterV2 for D* analysis
 */
inline std::unique_ptr<MassFitterV2> CreateDStarFitter(const KinematicBin& bin, const DStarFitConfig& config) {
    auto fitOpt = config.CreateFitOpt(bin);
    auto binParams = config.GetParametersForBin(bin);
    
    // Create fitter with optimized yield parameters
    auto fitter = std::make_unique<MassFitterV2>(
        "DStar_" + bin.GetBinName(),
        fitOpt.massVar,
        fitOpt.massMin,
        fitOpt.massMax,
        binParams.nsig_ratio,
        binParams.nsig_min_ratio,
        binParams.nsig_max_ratio,
        binParams.nbkg_ratio,
        binParams.nbkg_min_ratio,
        binParams.nbkg_max_ratio
    );
    
    // Configure for delta mass
    fitter->UseDeltaMass(true, 1.82, 1.92);
    
    // Load configuration
    fitter->SetConfiguration(config.CreateFitConfig());
    
    return fitter;
}

/**
 * @brief Batch fitting function for multiple kinematic bins
 */
inline void RunDStarFitAnalysis(const DStarFitConfig& config, bool plotResults = true) {
    auto allBins = config.GetAllKinematicBins();
    
    std::cout << "Starting D* fit analysis for " << allBins.size() << " kinematic bins..." << std::endl;
    
    for (const auto& bin : allBins) {
        std::cout << "\nFitting bin: " << bin.GetBinName() << std::endl;
        std::cout << "pT range: [" << bin.pTMin << ", " << bin.pTMax << "]" << std::endl;
        std::cout << "cos(θ) range: [" << bin.cosMin << ", " << bin.cosMax << "]" << std::endl;
        std::cout << "Centrality range: [" << bin.centralityMin << ", " << bin.centralityMax << "] (dummy)" << std::endl;
        
        try {
            // Create fitter for this bin
            auto fitter = CreateDStarFitter(bin, config);
            auto fitOpt = config.CreateFitOpt(bin);
            auto binParams = config.GetParametersForBin(bin);
            
            // Load data
            // Note: In practice, you would load the actual dataset here
            std::cout << "Loading data from: " << config.GetDataFilePath() << std::endl;
            
            // Perform fit (placeholder - would need actual data loading)
            /*
            DataLoader loader(config.GetDataFilePath());
            loader.loadRooDataSet(config.GetDatasetName());
            auto dataset = loader.getDataSet();
            
            if (dataset) {
                bool success = fitter->PerformFit(fitOpt, dataset, 
                                                  binParams.signalParams, 
                                                  binParams.backgroundParams,
                                                  bin.GetBinName());
                
                if (success) {
                    std::cout << "Fit successful for bin " << bin.GetBinName() << std::endl;
                    fitter->PrintSummary(bin.GetBinName());
                    
                    if (plotResults) {
                        PlotOptions plotOpt;
                        plotOpt.title = "D* Fit - " + bin.GetBinName();
                        fitter->CreateCanvas(bin.GetBinName(), plotOpt);
                    }
                    
                    // Save results
                    fitter->SaveResult(bin.GetBinName(), fitOpt.outputDir, fitOpt.outputFile + ".root");
                } else {
                    std::cerr << "Fit failed for bin " << bin.GetBinName() << std::endl;
                }
            }
            */
            
        } catch (const std::exception& e) {
            std::cerr << "Error fitting bin " << bin.GetBinName() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "\nD* fit analysis completed." << std::endl;
}

#endif // DSTAR_FIT_CONFIG_H