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
#include "MassFitterV2.h"
#include "Params.h"
#include "Opt.h"
#include "FitStrategy.h"

/**
 * @brief PDF types available for signal and background
 */
enum class PDFType {
    // Signal PDFs
    Gaussian = 0,
    DoubleGaussian = 1,
    CrystalBall = 2,
    DBCrystalBall = 3,
    DoubleDBCrystalBall = 4,
    Voigtian = 5,
    BreitWigner = 6,
    
    // Background PDFs
    Exponential = 10,
    Chebychev = 11,
    Phenomenological = 12,
    Polynomial = 13,
    ThresholdFunction = 14,
    ExpErf = 15,
    DstBkg = 16,
    DstD0  = 17,
    Phenomenological2 = 18
};

/**
 * @brief Parameter configuration for different pT and cos(theta) bins
 */
struct DStarBinParameters {
    // PDF type selections
    PDFType signalPdfType = PDFType::DBCrystalBall;
    PDFType backgroundPdfType = PDFType::Phenomenological;
    
    // Yield parameters
    double nsig_ratio = 0.01;
    double nsig_min_ratio = 0.0;
    double nsig_max_ratio = 1.0;
    double nbkg_ratio = 0.01;
    double nbkg_min_ratio = 0.0;
    double nbkg_max_ratio = 1.0;
    
    // Dynamic parameter storage for different PDF types
    std::map<std::string, double> signalParamValues;
    std::map<std::string, std::pair<double, double>> signalParamRanges;  // {min, max}
    std::map<std::string, double> backgroundParamValues;
    std::map<std::string, std::pair<double, double>> backgroundParamRanges;  // {min, max}
    
    // All possible signal PDF parameters
    PDFParams::GaussianParams gaussianParams;
    PDFParams::DoubleGaussianParams doubleGaussianParams;
    PDFParams::CrystalBallParams crystalBallParams;
    PDFParams::DBCrystalBallParams dbCrystalBallParams;
    PDFParams::DoubleDBCrystalBallParams doubleDBCrystalBallParams;
    PDFParams::VoigtianParams voigtianParams;
    PDFParams::BreitWignerParams breitWignerParams;
    
    // All possible background PDF parameters
    PDFParams::ExponentialBkgParams exponentialParams;
    PDFParams::ChebychevBkgParams chebychevParams;
    PDFParams::PhenomenologicalParams phenomenologicalParams;
    PDFParams::Phenomenological2Params phenomenological2Params;
    PDFParams::PolynomialBkgParams polynomialParams;
    PDFParams::ThresholdFuncParams thresholdFuncParams;
    PDFParams::ExpErfBkgParams expErfParams;
    PDFParams::DstBkgParams dstBkgParams;
    PDFParams::DstD0Params dstD0Params;
    
    // Legacy parameters for backward compatibility
    PDFParams::DBCrystalBallParams signalParams;    // Points to dbCrystalBallParams
    PDFParams::PhenomenologicalParams backgroundParams;  // Points to phenomenologicalParams
    
    // Constructor with defaults
    DStarBinParameters() {
        SetDefaultSignalParams();
        SetDefaultBackgroundParams();
    }
    
    // Get the correct signal parameters based on PDF type
    template<typename T>
    const T& GetSignalParams() const;
    
    // Get the correct background parameters based on PDF type
    template<typename T>
    const T& GetBackgroundParams() const;
    
    // Helper methods to apply a callable to the active parameter type (C++11/14 compatible)
    template <typename F>
    void ApplyToSignalParams(F&& f) const {
        switch (signalPdfType) {
            case PDFType::Gaussian:            f(gaussianParams); break;
            case PDFType::DoubleGaussian:      f(doubleGaussianParams); break;
            case PDFType::CrystalBall:         f(crystalBallParams); break;
            case PDFType::DBCrystalBall:       f(dbCrystalBallParams); break;
            case PDFType::DoubleDBCrystalBall: f(doubleDBCrystalBallParams); break;
            case PDFType::Voigtian:            f(voigtianParams); break;
            case PDFType::BreitWigner:         f(breitWignerParams); break;
            default:                            f(dbCrystalBallParams); break; // fallback
        }
    }

    template <typename F>
    void ApplyToBackgroundParams(F&& f) const {
        switch (backgroundPdfType) {
            case PDFType::Exponential:         f(exponentialParams); break;
            case PDFType::Chebychev:           f(chebychevParams); break;
            case PDFType::Phenomenological:    f(phenomenologicalParams); break;
            case PDFType::Phenomenological2:   f(phenomenological2Params); break;
            case PDFType::Polynomial:          f(polynomialParams); break;
            case PDFType::ThresholdFunction:   f(thresholdFuncParams); break;
            case PDFType::ExpErf:              f(expErfParams); break;
            case PDFType::DstBkg:              f(dstBkgParams); break;
            case PDFType::DstD0:               f(dstD0Params); break;
            default:                            f(phenomenologicalParams); break; // fallback
        }
    }

#if __cplusplus >= 201703L
    // Optional C++17 helpers returning std::variant for newer compilers
    std::variant<
        PDFParams::GaussianParams,
        PDFParams::DoubleGaussianParams,
        PDFParams::CrystalBallParams,
        PDFParams::DBCrystalBallParams,
        PDFParams::DoubleDBCrystalBallParams,
        PDFParams::VoigtianParams,
        PDFParams::BreitWignerParams
    > GetSignalParamsByType() const {
        switch(signalPdfType) {
            case PDFType::Gaussian: return gaussianParams;
            case PDFType::DoubleGaussian: return doubleGaussianParams;
            case PDFType::CrystalBall: return crystalBallParams;
            case PDFType::DBCrystalBall: return dbCrystalBallParams;
            case PDFType::DoubleDBCrystalBall: return doubleDBCrystalBallParams;
            case PDFType::Voigtian: return voigtianParams;
            case PDFType::BreitWigner: return breitWignerParams;
            default: return dbCrystalBallParams; // fallback
        }
    }
    
    std::variant<
        PDFParams::ExponentialBkgParams,
        PDFParams::ChebychevBkgParams,
        PDFParams::PhenomenologicalParams,
        PDFParams::Phenomenological2Params,
        PDFParams::PolynomialBkgParams,
        PDFParams::ThresholdFuncParams,
        PDFParams::ExpErfBkgParams,
        PDFParams::DstBkgParams
    > GetBackgroundParamsByType() const {
        switch(backgroundPdfType) {
            case PDFType::Exponential: return exponentialParams;
            case PDFType::Chebychev: return chebychevParams;
            case PDFType::Phenomenological: return phenomenologicalParams;
            case PDFType::Phenomenological2: return phenomenological2Params;
            case PDFType::Polynomial: return polynomialParams;
            case PDFType::ThresholdFunction: return thresholdFuncParams;
            case PDFType::ExpErf: return expErfParams;
            case PDFType::DstBkg: return dstBkgParams;
            default: return phenomenologicalParams; // fallback
        }
    }
#endif
    
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
        opt.datasetName = datasetName_;
        
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
        grandDaughterCut_ = "abs(EtaD2) < 1.2 && abs(EtaGrandD1) < 1.2 && abs(EtaGrandD2) < 1.2 && massDaugther1 > 1.82 && massDaugther1 < 1.92";
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
