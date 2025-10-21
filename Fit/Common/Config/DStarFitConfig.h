#ifndef DSTAR_FIT_CONFIG_H
#define DSTAR_FIT_CONFIG_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <sstream>
#if __cplusplus >= 201703L
#include <variant>
#endif
#include "../Core/MassFitterV2.h"
#include "DStarFitTypes.h"
#include "../Config/Opt.h"
#include "../Utils/FitStrategy.h"

inline void ConfigureDStarDataFitOpt(FitOpt& opt) {
    const double pTMin = opt.pTMin;
    const double pTMax = opt.pTMax;
    const double cosMin = opt.cosMin;
    const double cosMax = opt.cosMax;

    opt.name = "DStar";
    opt.fitMethod = FitMethod::BinnedNLL;
    opt.massVar = "massPion";
    opt.massMin = 0.140;
    opt.massMax = 0.155;
    opt.pTMin = pTMin <= 0.0 ? 10.0 : pTMin;
    opt.pTMax = pTMax <= 0.0 ? 100.0 : pTMax;
    if (cosMin == 0.0 && cosMax == 0.0) {
        opt.cosMin = -2.0;
        opt.cosMax = 2.0;
    } else {
        opt.cosMin = cosMin;
        opt.cosMax = cosMax;
    }
    opt.mvaMin = 0.99;
    opt.histogramBins = 50;
    opt.outputDir = "results/DStar_Data/";
    opt.plotName = "Plot_DStar_Data";
    opt.plotMCName = "Plot_DStar_Data_MC";
    opt.wsName = "workspace_DStar";
    opt.outputFile = "DStar_fit.root";
    opt.cutExpr.clear();
    opt.cutMCExpr.clear();
    opt.subDir.clear();
    opt.pdfName = "total_pdf";
    opt.GenerateLegends();
}

inline void ConfigureDStarMCFitOpt(FitOpt& opt) {
    const double pTMin = opt.pTMin;
    const double pTMax = opt.pTMax;
    const double cosMin = opt.cosMin;
    const double cosMax = opt.cosMax;

    ConfigureDStarDataFitOpt(opt);

    opt.pTMin = pTMin;
    opt.pTMax = pTMax;
    opt.cosMin = cosMin;
    opt.cosMax = cosMax;
    opt.fitMethod = FitMethod::Extended;
    opt.outputDir = "results/DStar_MC/";
    opt.plotName = "Plot_DStar_MC";
    opt.plotMCName = "Plot_DStar_MC";
    opt.wsName = "workspace_DStar";
    if (!opt.cutMCExpr.empty()) {
        opt.cutExpr = opt.cutMCExpr;
    }
    opt.GenerateLegends();
}

inline void ConfigureDStarMCAbsFitOpt(FitOpt& opt) {
    const double pTMin = opt.pTMin;
    const double pTMax = opt.pTMax;
    const double cosMin = opt.cosMin;
    const double cosMax = opt.cosMax;

    ConfigureDStarMCFitOpt(opt);

    opt.pTMin = pTMin;
    opt.pTMax = pTMax;
    opt.cosMin = cosMin;
    opt.cosMax = cosMax;

    opt.dcaVar = "dca3D";
    opt.dcaMin = 0.0;
    opt.dcaMax = 0.07;

    std::ostringstream cut;
    cut << "y<1 && y>-1"
        << " && abs(" << opt.cosVar << ") < " << cosMax
        << " && abs(" << opt.cosVar << ") >= " << cosMin
        << " && " << opt.ptVar << " < " << pTMax
        << " && " << opt.ptVar << " >= " << pTMin;

    opt.cutExpr = cut.str();
    opt.cutMCExpr = opt.cutExpr + " && matchGEN==1";
    opt.pdfName = "total_pdf";

    const std::string binLabel = opt.GetBinName();
    opt.plotName = "Plot_" + opt.name + "_" + binLabel + ".pdf";
    opt.plotMCName = "PlotMC_" + opt.name + "_" + binLabel + ".pdf";
    opt.outputFile = opt.name + "_" + binLabel + ".root";
    opt.outputMCFile = "MC_" + opt.name + "_" + binLabel + ".root";
    opt.outputMCSwap0File = "MC_Swap0_" + opt.name + "_" + binLabel + ".root";
    opt.outputMCSwap1File = "MC_Swap1_" + opt.name + "_" + binLabel + ".root";
    opt.outputDCAFile = "DCA_" + opt.name + "_" + binLabel + ".root";
    opt.centLegend = Form("%0.2f < |cos#theta_{HX}| < %0.2f", cosMin, cosMax);
    opt.GenerateLegends();
}

/**
 * @brief Main configuration class for D* meson fitting
 */
class DStarFitOpt {
public:
    DStarFitOpt() {
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

    void ClearCentralityBins() {
        centralityBins_.clear();
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
        binParameters_[MakeBinKey(bin)] = params;
    }
    
    // Set fixed flags for specific bins
    void SetFixedFlagsForBin(const KinematicBin& bin, const std::map<std::string, bool>& fixedFlags) {
        binFixedFlags_[MakeBinKey(bin)] = fixedFlags;
    }
    
    // Get fixed flags for a specific bin
    std::map<std::string, bool> GetFixedFlagsForBin(const KinematicBin& bin) const {
        auto it = binFixedFlags_.find(MakeBinKey(bin));
        if (it != binFixedFlags_.end()) {
            return it->second;
        }
        return std::map<std::string, bool>();  // Return empty map if not found
    }
    
    // Get parameters for a specific bin
    DStarBinParameters GetParametersForBin(const KinematicBin& bin) const {
        auto it = binParameters_.find(MakeBinKey(bin));
        if (it != binParameters_.end()) {
            return it->second;
        }
        return GetDefaultParameters(bin);  // Return optimized defaults
    }
    
    // Configuration for file paths
    void SetDataFilePath(const std::string& path) { dataFilePath_ = path; }
    void SetMCFilePath(const std::string& path) { mcFilePath_ = path; }
    void SetDatasetName(const std::string& name) { datasetName_ = name; }
    void SetMCdatasetName(const std::string& name) { MCdatasetName_ = name; }
    void SetOutputSubDir(const std::string& subdir) { outputSubDir_ = subdir; }
    
    std::string GetDataFilePath() const { return dataFilePath_; }
    std::string GetMCFilePath() const { return mcFilePath_; }
    std::string GetDatasetName() const { return datasetName_; }
    std::string GetMCdatasetName() const { return MCdatasetName_; }
    std::string GetOutputSubDir() const { return outputSubDir_; }
    
    // Fit configuration
    void SetFitMethod(FitMethod method) { fitMethod_ = method; }
    void SetUseCUDA(bool use) { useCUDA_ = use; }
    void SetVerbose(bool verbose) { verbose_ = verbose; }
    void SetDoRefit(bool refit) { doRefit_ = refit; }
    void SetUseAbsCosCuts(bool use) { useAbsCosCuts_ = use; }
    // Auto-tune behavior when changing yield mode (e.g., switch to NLL and enable fsig coefficient model)
    void SetYieldModeAutoTuning(bool v) { yieldModeAutoTuning_ = v; }
    bool GetYieldModeAutoTuning() const { return yieldModeAutoTuning_; }
    
    FitMethod GetFitMethod() const { return fitMethod_; }
    bool GetUseCUDA() const { return useCUDA_; }
    bool GetVerbose() const { return verbose_; }
    bool GetDoRefit() const { return doRefit_; }
    bool GetUseAbsCosCuts() const { return useAbsCosCuts_; }
    
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
        ConfigureDStarDataFitOpt(opt);
        opt.ApplyKinematicBin(bin);

        // Ensure dataset names are set before building cuts
        opt.datasetName = datasetName_;
        opt.MCdatasetName = MCdatasetName_;

        auto combineCuts = [](const std::string& lhs, const std::string& rhs) {
            if (lhs.empty()) return rhs;
            if (rhs.empty()) return lhs;
            return lhs + " && " + rhs;
        };

        const std::string baseCut = GetFullCutString();
        const std::string kinCut = opt.BuildKinematicCut(bin, useAbsCosCuts_);
        opt.cutExpr = combineCuts(baseCut, kinCut);

        opt.cutMCExpr = opt.cutExpr.empty() ? "matchGEN==1" : opt.cutExpr + " && matchGEN==1";

        // Override with configuration-specific settings
        opt.useCUDA = useCUDA_;
        opt.doFit = doRefit_;
        opt.verbose = verbose_;
        opt.useMinos = false;  // Usually not needed for D* fits
        opt.useHesse = true;
        opt.fitMethod = fitMethod_;  // Use configured fit method
        
        // Set output configuration
        opt.outputDir = "results/";
        opt.outputFile = "DStar_fit_" + BuildBinName(bin);
        opt.subDir = outputSubDir_;
        
        // Generate proper legends for this bin
        opt.GenerateLegends();
        
        return opt;
    }
    
    // Create FitOpt for modern fitting framework
    FitOpt CreateFitOpt() const {
        FitOpt tempOpt;
        ConfigureDStarDataFitOpt(tempOpt);

        tempOpt.fitMethod = fitMethod_;
        tempOpt.useCUDA = useCUDA_;
        tempOpt.verbose = verbose_;
        tempOpt.useMinos = false;
        tempOpt.useHesse = true;
        tempOpt.numCPU = 24;

        tempOpt.datasetName = datasetName_;
        tempOpt.subDir = outputSubDir_;

        return tempOpt;
    }
    
    // Yield-mode preference (default: Fraction)
    void SetUseIndependentYields(bool v) {
        useIndependentYields_ = v;
        if (yieldModeAutoTuning_) {
            if (!v) {
                // Fraction mode: prefer non-extended fits and coefficient model
                fitMethod_ = FitMethod::NLL;
                preferCoeffModelForFraction_ = true;
            } else {
                // Independent yields: default back to Extended unless user overrides later
                fitMethod_ = FitMethod::Extended;
                preferCoeffModelForFraction_ = false;
            }
        }
    }
    bool GetUseIndependentYields() const { return useIndependentYields_; }
    bool GetPreferCoeffModelForFraction() const { return preferCoeffModelForFraction_; }
    
private:
    // Kinematic bins
    std::vector<std::pair<double, double>> ptBins_;
    std::vector<std::pair<double, double>> cosBins_;
    std::vector<std::pair<int, int>> centralityBins_;
    
    // Parameters for each bin
    std::map<std::string, DStarBinParameters> binParameters_;
    
    // Fixed flags for each bin's parameters
    std::map<std::string, std::map<std::string, bool>> binFixedFlags_;
    
    // File paths
    std::string dataFilePath_;
    std::string mcFilePath_;
    std::string datasetName_ = "datasetHX";
    std::string MCdatasetName_ = "datasetHX";  // MC dataset name for variable selection
    std::string outputSubDir_;
    
    // Fit configuration
    FitMethod fitMethod_ = FitMethod::Extended;
    bool useCUDA_ = true;
    bool verbose_ = false;
    bool doRefit_ = false;
    bool useAbsCosCuts_ = true;  // Whether to use abs(cosThetaHX) in cuts
    bool useIndependentYields_ = true; // Whether to use independent nsig/nbkg yields
    bool yieldModeAutoTuning_ = false;  // When true, auto-adjust fit method and model by yield mode
    bool preferCoeffModelForFraction_ = false; // Enable fsig coefficient model in fraction mode
    
    // Selection cuts
    std::string slowPionCut_;
    std::string grandDaughterCut_;
    std::string mvaCut_;

    std::string MakeBinKey(const KinematicBin& bin) const {
        return BuildBinName(bin);
    }
    
    void SetupDefaultConfiguration() {
        // Default centrality bin (dummy for now)
        centralityBins_.emplace_back(0, 100);
        
        // Default cuts (can be overridden)
        slowPionCut_ = "pTD2 > 0.3 && abs(EtaD2) < 2.4";
        grandDaughterCut_ = "abs(EtaD2) < 1.2 && abs(EtaGrandD1) < 1.2 && abs(EtaGrandD2) < 1.2";
        mvaCut_ = "";
        
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

using DStarFitConfig = DStarFitOpt;

/**
 * @brief Factory function to create a configured MassFitterV2 for D* analysis
 */
inline std::unique_ptr<MassFitterV2> CreateDStarFitter(const KinematicBin& bin, const DStarFitOpt& config) {
    auto fitOpt = config.CreateFitOpt(bin);
    auto binParams = config.GetParametersForBin(bin);
    
    // Create fitter with optimized yield parameters using name from opt.h
    auto fitter = std::make_unique<MassFitterV2>(
        fitOpt.name,  // Use the name from opt.name instead of hardcoded "DStar_" + bin name
        fitOpt.massVar,
        fitOpt.massMin,
        fitOpt.massMax,
        binParams.nsig_ratio,
        binParams.nsig_min_ratio,
        binParams.nsig_max_ratio,
        binParams.nbkg_ratio,
        binParams.nbkg_min_ratio,
        binParams.nbkg_max_ratio,
        config.GetUseIndependentYields() ? MassFitterV2::YieldMode::Independent : MassFitterV2::YieldMode::Fraction
    );
    
    // Configure for delta mass
    // fitter->UseDeltaMass(true, 1.82, 1.92);
    
    // Configuration will be handled by FitOpt.ToFitConfig() in PerformFit()
    // No need to set configuration here - it's redundant
    
    // Apply bin-specific PDF types and parameters
    const std::string binLabel = BuildBinName(bin);
    std::cout << "[Config] Applying custom PDF configuration for bin: " << binLabel << std::endl;
    std::cout << "[Config] Signal PDF: " << static_cast<int>(binParams.signalPdfType) 
              << ", Background PDF: " << static_cast<int>(binParams.backgroundPdfType) << std::endl;
    
    // Auto-enable coefficient-based fraction model when requested by config
    if (!config.GetUseIndependentYields() && config.GetPreferCoeffModelForFraction()) {
        std::cout << "[Config] Enabling coefficient-based fraction model (fsig) automatically" << std::endl;
        fitter->UseFractionCoefficientModel(true);
    }

    // The fitter will use the binParams.signalPdfType and backgroundPdfType
    // when SetSignalPDF() and SetBackgroundPDF() are called in PerformFit()
    
    return fitter;
}

/**
 * @brief Batch fitting function for multiple kinematic bins
 */


#endif // DSTAR_FIT_CONFIG_H
