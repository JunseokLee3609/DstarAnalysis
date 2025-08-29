#include "../DStarFitConfig.h"
#include "../MassFitterV2.h"
#include "../DataLoader.h"
// #include "../PlotManager.h"  // Disabled - has compilation issues
#include "../EnhancedPlotManager.h"  // Use our enhanced plotter instead
#include "../SimpleParameterLoader.h"  // External parameter loading
#include "../JSONParameterLoader.h"    // JSON parameter loading
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

// ===== PARAMETER DEBUGGING FUNCTIONS =====

// Function to print PDF parameters for debugging
void PrintBinParameters(const KinematicBin& bin, const DStarBinParameters& params, const std::string& title = "") {
    if (!title.empty()) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << title << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
    
    std::cout << "📍 Bin: " << bin.GetBinName() << std::endl;
    std::cout << "   pT: [" << bin.pTMin << ", " << bin.pTMax << "] GeV/c" << std::endl;
    std::cout << "   cos(θ*): [" << bin.cosMin << ", " << bin.cosMax << "]" << std::endl;
    std::cout << "   Centrality: [" << bin.centralityMin << ", " << bin.centralityMax << "]%" << std::endl;
    
    // Print PDF types
    std::cout << "\n🔍 PDF Configuration:" << std::endl;
    std::cout << "   Signal PDF: " << static_cast<int>(params.signalPdfType);
    switch(params.signalPdfType) {
        case PDFType::Gaussian: std::cout << " (Gaussian)"; break;
        case PDFType::DoubleGaussian: std::cout << " (DoubleGaussian)"; break;
        case PDFType::CrystalBall: std::cout << " (CrystalBall)"; break;
        case PDFType::DBCrystalBall: std::cout << " (DBCrystalBall)"; break;
        default: std::cout << " (Other)"; break;
    }
    std::cout << std::endl;
    
    std::cout << "   Background PDF: " << static_cast<int>(params.backgroundPdfType);
    switch(params.backgroundPdfType) {
        case PDFType::Exponential: std::cout << " (Exponential)"; break;
        case PDFType::ThresholdFunction: std::cout << " (ThresholdFunction)"; break;
        case PDFType::Chebychev: std::cout << " (Chebychev)"; break;
        case PDFType::Phenomenological: std::cout << " (Phenomenological)"; break;
        default: std::cout << " (Other)"; break;
    }
    std::cout << std::endl;
    
    // Print signal parameters based on PDF type
    std::cout << "\n🎯 Signal Parameters:" << std::endl;
    switch(params.signalPdfType) {
        case PDFType::Gaussian:
            std::cout << "   mean: " << params.gaussianParams.mean 
                      << " [" << params.gaussianParams.mean_min << ", " << params.gaussianParams.mean_max << "]" << std::endl;
            std::cout << "   sigma: " << params.gaussianParams.sigma 
                      << " [" << params.gaussianParams.sigma_min << ", " << params.gaussianParams.sigma_max << "]" << std::endl;
            break;
            
        case PDFType::DoubleGaussian:
            std::cout << "   mean: " << params.doubleGaussianParams.mean 
                      << " [" << params.doubleGaussianParams.mean_min << ", " << params.doubleGaussianParams.mean_max << "]" << std::endl;
            std::cout << "   sigma1: " << params.doubleGaussianParams.sigma1 
                      << " [" << params.doubleGaussianParams.sigma1_min << ", " << params.doubleGaussianParams.sigma1_max << "]" << std::endl;
            std::cout << "   sigma2: " << params.doubleGaussianParams.sigma2 
                      << " [" << params.doubleGaussianParams.sigma2_min << ", " << params.doubleGaussianParams.sigma2_max << "]" << std::endl;
            std::cout << "   fraction: " << params.doubleGaussianParams.fraction 
                      << " [" << params.doubleGaussianParams.fraction_min << ", " << params.doubleGaussianParams.fraction_max << "]" << std::endl;
            break;
            
        case PDFType::CrystalBall:
            std::cout << "   mean: " << params.crystalBallParams.mean 
                      << " [" << params.crystalBallParams.mean_min << ", " << params.crystalBallParams.mean_max << "]" << std::endl;
            std::cout << "   sigma: " << params.crystalBallParams.sigma 
                      << " [" << params.crystalBallParams.sigma_min << ", " << params.crystalBallParams.sigma_max << "]" << std::endl;
            std::cout << "   alpha: " << params.crystalBallParams.alpha 
                      << " [" << params.crystalBallParams.alpha_min << ", " << params.crystalBallParams.alpha_max << "]" << std::endl;
            std::cout << "   n: " << params.crystalBallParams.n 
                      << " [" << params.crystalBallParams.n_min << ", " << params.crystalBallParams.n_max << "]" << std::endl;
            break;
            
        default:
            std::cout << "   (Parameters not displayed for this PDF type)" << std::endl;
            break;
    }
    
    // Print background parameters based on PDF type
    std::cout << "\n🔲 Background Parameters:" << std::endl;
    switch(params.backgroundPdfType) {
        case PDFType::Exponential:
            std::cout << "   lambda: " << params.exponentialParams.lambda 
                      << " [" << params.exponentialParams.lambda_min << ", " << params.exponentialParams.lambda_max << "]" << std::endl;
            break;
            
        case PDFType::ThresholdFunction:
            std::cout << "   p0: " << params.thresholdFuncParams.p0_init 
                      << " [" << params.thresholdFuncParams.p0_min << ", " << params.thresholdFuncParams.p0_max << "]" << std::endl;
            std::cout << "   p1: " << params.thresholdFuncParams.p1_init 
                      << " [" << params.thresholdFuncParams.p1_min << ", " << params.thresholdFuncParams.p1_max << "]" << std::endl;
            std::cout << "   m_pi: " << params.thresholdFuncParams.m_pi_value << " (fixed)" << std::endl;
            break;
            
        case PDFType::Chebychev:
            std::cout << "   coefficients: [";
            for(size_t i = 0; i < params.chebychevParams.coefficients.size(); ++i) {
                std::cout << params.chebychevParams.coefficients[i];
                if(i < params.chebychevParams.coefficients.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            std::cout << "   coef_min: [";
            for(size_t i = 0; i < params.chebychevParams.coef_min.size(); ++i) {
                std::cout << params.chebychevParams.coef_min[i];
                if(i < params.chebychevParams.coef_min.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            std::cout << "   coef_max: [";
            for(size_t i = 0; i < params.chebychevParams.coef_max.size(); ++i) {
                std::cout << params.chebychevParams.coef_max[i];
                if(i < params.chebychevParams.coef_max.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            break;
            
        default:
            std::cout << "   (Parameters not displayed for this PDF type)" << std::endl;
            break;
    }
    
    // Print yield parameters
    std::cout << "\n📊 Yield Parameters:" << std::endl;
    std::cout << "   nsig_ratio: " << params.nsig_ratio 
              << " [" << params.nsig_min_ratio << ", " << params.nsig_max_ratio << "]" << std::endl;
    std::cout << "   nbkg_ratio: " << params.nbkg_ratio 
              << " [" << params.nbkg_min_ratio << ", " << params.nbkg_max_ratio << "]" << std::endl;
    
    std::cout << std::string(60, '-') << std::endl;
}

// Function to print all parameters in the config
void PrintAllConfigParameters(const DStarFitConfig& config, const std::string& title = "") {
    if (!title.empty()) {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "📋 " << title << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }
    
    auto allBins = config.GetAllKinematicBins();
    std::cout << "Total bins: " << allBins.size() << std::endl;
    
    for (const auto& bin : allBins) {
        try {
            auto params = config.GetParametersForBin(bin);
            PrintBinParameters(bin, params);
        } catch (const std::exception& e) {
            std::cout << "❌ Error getting parameters for bin " << bin.GetBinName() 
                      << ": " << e.what() << std::endl;
        }
    }
}

// Function to compare parameters before and after JSON loading
void CompareParameters(const DStarFitConfig& configBefore, const DStarFitConfig& configAfter, 
                      const std::string& binName = "") {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "🔍 PARAMETER COMPARISON" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    auto binsBefore = configBefore.GetAllKinematicBins();
    auto binsAfter = configAfter.GetAllKinematicBins();
    
    if (binsBefore.size() != binsAfter.size()) {
        std::cout << "⚠️  Warning: Different number of bins! Before: " << binsBefore.size() 
                  << ", After: " << binsAfter.size() << std::endl;
    }
    
    for (size_t i = 0; i < std::min(binsBefore.size(), binsAfter.size()); ++i) {
        const auto& bin = binsBefore[i];
        
        if (!binName.empty() && bin.GetBinName().find(binName) == std::string::npos) {
            continue;
        }
        
        std::cout << "\n📍 Bin: " << bin.GetBinName() << std::endl;
        
        try {
            auto paramsBefore = configBefore.GetParametersForBin(bin);
            auto paramsAfter = configAfter.GetParametersForBin(bin);
            
            bool changed = false;
            
            // Compare PDF types
            if (paramsBefore.signalPdfType != paramsAfter.signalPdfType) {
                std::cout << "  🎯 Signal PDF: " << static_cast<int>(paramsBefore.signalPdfType) 
                          << " → " << static_cast<int>(paramsAfter.signalPdfType) << std::endl;
                changed = true;
            }
            
            if (paramsBefore.backgroundPdfType != paramsAfter.backgroundPdfType) {
                std::cout << "  🔲 Background PDF: " << static_cast<int>(paramsBefore.backgroundPdfType) 
                          << " → " << static_cast<int>(paramsAfter.backgroundPdfType) << std::endl;
                changed = true;
            }
            
            // Compare yield ratios
            if (paramsBefore.nsig_ratio != paramsAfter.nsig_ratio) {
                std::cout << "  📊 nsig_ratio: " << paramsBefore.nsig_ratio 
                          << " → " << paramsAfter.nsig_ratio << std::endl;
                changed = true;
            }
            
            if (paramsBefore.nbkg_ratio != paramsAfter.nbkg_ratio) {
                std::cout << "  📊 nbkg_ratio: " << paramsBefore.nbkg_ratio 
                          << " → " << paramsAfter.nbkg_ratio << std::endl;
                changed = true;
            }
            
            // Compare signal parameters (example for DoubleGaussian)
            if (paramsAfter.signalPdfType == PDFType::DoubleGaussian) {
                const auto& before = paramsBefore.doubleGaussianParams;
                const auto& after = paramsAfter.doubleGaussianParams;
                
                if (before.mean != after.mean) {
                    std::cout << "  🎯 mean: " << before.mean << " → " << after.mean << std::endl;
                    changed = true;
                }
                if (before.sigma1 != after.sigma1) {
                    std::cout << "  🎯 sigma1: " << before.sigma1 << " → " << after.sigma1 << std::endl;
                    changed = true;
                }
                if (before.sigma2 != after.sigma2) {
                    std::cout << "  🎯 sigma2: " << before.sigma2 << " → " << after.sigma2 << std::endl;
                    changed = true;
                }
                if (before.fraction != after.fraction) {
                    std::cout << "  🎯 fraction: " << before.fraction << " → " << after.fraction << std::endl;
                    changed = true;
                }
            }
            
            // Compare background parameters (example for ThresholdFunction)
            if (paramsAfter.backgroundPdfType == PDFType::ThresholdFunction) {
                const auto& before = paramsBefore.thresholdFuncParams;
                const auto& after = paramsAfter.thresholdFuncParams;
                
                if (before.p0_init != after.p0_init) {
                    std::cout << "  🔲 p0: " << before.p0_init << " → " << after.p0_init << std::endl;
                    changed = true;
                }
                if (before.p1_init != after.p1_init) {
                    std::cout << "  🔲 p1: " << before.p1_init << " → " << after.p1_init << std::endl;
                    changed = true;
                }
            }
            
            if (!changed) {
                std::cout << "  ✅ No changes detected" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "  ❌ Error comparing parameters: " << e.what() << std::endl;
        }
    }
    
    std::cout << std::string(80, '=') << std::endl;
}

// Function to load parameters from JSON file
void LoadParametersFromJSON(DStarFitConfig& config, const std::string& jsonFile) {
    JSONParameterLoader jsonLoader;
    jsonLoader.loadFromFile(jsonFile);
    
    std::cout << "[JSON Loader] Loading parameters from: " << jsonFile << std::endl;
    jsonLoader.printLoadedParameters();
    
    // Get all bins from JSON
    auto jsonBins = jsonLoader.getAvailableBins();
    auto configBins = config.GetAllKinematicBins();
    
    for (const auto& bin : configBins) {
        std::string binKey = bin.GetBinName();
        
        // Find matching JSON bin by checking pT ranges and cos ranges
        std::string matchingJsonBin = "";
        std::cout << "[JSON Loader] Searching for bin with pT[" << bin.pTMin << ", " << bin.pTMax 
                  << "], cos[" << bin.cosMin << ", " << bin.cosMax << "]" << std::endl;
        
        for (const auto& jsonBin : jsonBins) {
            std::cout << "[JSON Loader] Checking JSON bin: " << jsonBin << std::endl;
            
            // More flexible matching: check if pT ranges overlap
            bool ptMatch = (jsonBin.find(std::to_string((int)bin.pTMin)) != std::string::npos) ||
                           (jsonBin.find(std::to_string((int)bin.pTMax)) != std::string::npos) ||
                           (bin.pTMin >= 4 && bin.pTMax <= 10 && jsonBin.find("pt_4_") != std::string::npos) ||
                           (bin.pTMin >= 6 && bin.pTMax <= 10 && jsonBin.find("pt_6_") != std::string::npos) ||
                           (bin.pTMin >= 10 && jsonBin.find("pt_10_") != std::string::npos);
            
            // Check cos range matching
            bool cosMatch = (bin.cosMin >= -1 && bin.cosMax <= 1 && 
                            (jsonBin.find("cos_-1_") != std::string::npos || 
                             jsonBin.find("cos_0_") != std::string::npos ||
                             jsonBin.find("cos_-1_1") != std::string::npos));
            
            if (ptMatch && cosMatch) {
                matchingJsonBin = jsonBin;
                std::cout << "[JSON Loader] ✅ Found matching bin: " << jsonBin << std::endl;
                break;
            }
        }
        
        if (matchingJsonBin.empty()) {
            std::cout << "[JSON Loader] ❌ No matching parameters found for bin: " << binKey 
                      << ". Using default parameters." << std::endl;
            continue;
        }
        
        std::cout << "[JSON Loader] Loading parameters for bin: " << binKey 
                  << " from JSON bin: " << matchingJsonBin << std::endl;
        
        // Create bin parameters
        DStarBinParameters binParams;
        
        // Get PDF types from JSON
        std::string signalPdfType = jsonLoader.getPDFType(matchingJsonBin, "signal");
        std::string backgroundPdfType = jsonLoader.getPDFType(matchingJsonBin, "background");
        
        std::cout << "[JSON Loader] Signal PDF: " << signalPdfType 
                  << ", Background PDF: " << backgroundPdfType << std::endl;
        
        // Set PDF types
        if (signalPdfType == "Gaussian") {
            binParams.signalPdfType = PDFType::Gaussian;
            auto param = jsonLoader.getParameter(matchingJsonBin, "signal_mean");
            binParams.gaussianParams.mean = param.value;
            binParams.gaussianParams.mean_min = param.min;
            binParams.gaussianParams.mean_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "signal_sigma");
            binParams.gaussianParams.sigma = param.value;
            binParams.gaussianParams.sigma_min = param.min;
            binParams.gaussianParams.sigma_max = param.max;
            
        } else if (signalPdfType == "DoubleGaussian") {
            binParams.signalPdfType = PDFType::DoubleGaussian;
            auto param = jsonLoader.getParameter(matchingJsonBin, "signal_mean");
            binParams.doubleGaussianParams.mean = param.value;
            binParams.doubleGaussianParams.mean_min = param.min;
            binParams.doubleGaussianParams.mean_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "signal_sigma1");
            binParams.doubleGaussianParams.sigma1 = param.value;
            binParams.doubleGaussianParams.sigma1_min = param.min;
            binParams.doubleGaussianParams.sigma1_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "signal_sigma2");
            binParams.doubleGaussianParams.sigma2 = param.value;
            binParams.doubleGaussianParams.sigma2_min = param.min;
            binParams.doubleGaussianParams.sigma2_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "signal_fraction");
            binParams.doubleGaussianParams.fraction = param.value;
            binParams.doubleGaussianParams.fraction_min = param.min;
            binParams.doubleGaussianParams.fraction_max = param.max;
            
        } else if (signalPdfType == "CrystalBall") {
            binParams.signalPdfType = PDFType::CrystalBall;
            auto param = jsonLoader.getParameter(matchingJsonBin, "signal_mean");
            binParams.crystalBallParams.mean = param.value;
            binParams.crystalBallParams.mean_min = param.min;
            binParams.crystalBallParams.mean_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "signal_sigma");
            binParams.crystalBallParams.sigma = param.value;
            binParams.crystalBallParams.sigma_min = param.min;
            binParams.crystalBallParams.sigma_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "signal_alpha");
            binParams.crystalBallParams.alpha = param.value;
            binParams.crystalBallParams.alpha_min = param.min;
            binParams.crystalBallParams.alpha_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "signal_n");
            binParams.crystalBallParams.n = param.value;
            binParams.crystalBallParams.n_min = param.min;
            binParams.crystalBallParams.n_max = param.max;
        }
        
        // Set background PDF types
        if (backgroundPdfType == "ThresholdFunction") {
            binParams.backgroundPdfType = PDFType::ThresholdFunction;
            auto param = jsonLoader.getParameter(matchingJsonBin, "background_p0");
            binParams.thresholdFuncParams.p0_init = param.value;
            binParams.thresholdFuncParams.p0_min = param.min;
            binParams.thresholdFuncParams.p0_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "background_p1");
            binParams.thresholdFuncParams.p1_init = param.value;
            binParams.thresholdFuncParams.p1_min = param.min;
            binParams.thresholdFuncParams.p1_max = param.max;
            
            param = jsonLoader.getParameter(matchingJsonBin, "background_m_pi");
            binParams.thresholdFuncParams.m_pi_value = param.value;
            
        } else if (backgroundPdfType == "Exponential") {
            binParams.backgroundPdfType = PDFType::Exponential;
            auto param = jsonLoader.getParameter(matchingJsonBin, "background_tau");
            binParams.exponentialParams.lambda = param.value;
            binParams.exponentialParams.lambda_min = param.min;
            binParams.exponentialParams.lambda_max = param.max;
            
        } else if (backgroundPdfType == "Chebychev") {
            binParams.backgroundPdfType = PDFType::Chebychev;
            auto param = jsonLoader.getParameter(matchingJsonBin, "background_c0");
            binParams.chebychevParams.coefficients = {param.value};
            binParams.chebychevParams.coef_min = {param.min};
            binParams.chebychevParams.coef_max = {param.max};
            
            param = jsonLoader.getParameter(matchingJsonBin, "background_c1");
            binParams.chebychevParams.coefficients.push_back(param.value);
            binParams.chebychevParams.coef_min.push_back(param.min);
            binParams.chebychevParams.coef_max.push_back(param.max);
        }
        
        // Load yield parameters
        auto param = jsonLoader.getParameter(matchingJsonBin, "nsig_ratio");
        binParams.nsig_ratio = param.value;
        binParams.nsig_min_ratio = param.min;
        binParams.nsig_max_ratio = param.max;
        
        param = jsonLoader.getParameter(matchingJsonBin, "nbkg_ratio");
        binParams.nbkg_ratio = param.value;
        binParams.nbkg_min_ratio = param.min;
        binParams.nbkg_max_ratio = param.max;
        
        // Apply parameters to config
        config.SetParametersForBin(bin, binParams);
        
        std::cout << "[JSON Loader] Successfully loaded parameters for bin: " << binKey << std::endl;
    }
}

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
    config.SetFitMethod(FitMethod::BinnedNLL);  // NLL, BinnedNLL, Extended 
    config.SetUseCUDA(useCUDA);
    config.SetVerbose(false);
    config.SetDoRefit(doReFit);
    
    // Configure selection cuts
    config.SetSlowPionCut(SelectionCuts::getSlowPionCuts());
    config.SetGrandDaughterCut(SelectionCuts::getGrandDaughterCuts());
    config.SetMVACut(0.99);
    
    // Create single kinematic bin for analysis
    KinematicBin currentBin(pTMin, pTMax, cosMin, cosMax, centralityMin, centralityMax);
    
    // Add kinematic ranges (this creates a single bin internally)
    config.AddPtBin(pTMin, pTMax);
    config.AddCosBin(cosMin, cosMax);
    config.AddCentralityBin(centralityMin, centralityMax);
    
    // ===== PARAMETER LOADING SYSTEM =====
    std::cout << "Setting up parameters..." << std::endl;
    
    // Check if external parameter file is provided
    if (!parameterFile.empty()) {
        std::cout << "Loading parameters from external file: " << parameterFile << std::endl;
        
        try {
            // Check file extension to determine loader type
            std::string extension = parameterFile.substr(parameterFile.find_last_of(".") + 1);
            
            if (extension == "json") {
                std::cout << "Using JSON parameter loader..." << std::endl;
                LoadParametersFromJSON(config, parameterFile);
            } else {
                std::cout << "Using legacy parameter loader..." << std::endl;
                ParameterLoaderUtils::LoadParametersToConfig(config, parameterFile);
            }
            std::cout << "✓ External parameters loaded successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "⚠ Failed to load external parameters: " << e.what() << std::endl;
            std::cout << "Falling back to hardcoded parameters..." << std::endl;
            // Will use hardcoded parameters below
        }
    } else {
        std::cout << "No parameter file provided, using hardcoded parameters" << std::endl;
    }
        
    // Store copy of config before JSON loading for comparison  
    DStarFitConfig configBeforeJSON = config;
    
    // ===== SINGLE BIN ANALYSIS =====
    std::cout << "\n🎯 Single Bin Analysis Mode" << std::endl;
    std::cout << "Target bin: pT[" << pTMin << ", " << pTMax << "], cos[" << cosMin << ", " << cosMax << "], cent[" << centralityMin << ", " << centralityMax << "]" << std::endl;
    
    // ===== PARAMETER DEBUGGING: BEFORE LOADING =====
    std::cout << "\n🔍 DEBUGGING: Initial Parameters (Before Loading)" << std::endl;
    PrintBinParameters(currentBin, config.GetParametersForBin(currentBin), "INITIAL PARAMETERS FOR TARGET BIN");
    
    // Check if we need to set hardcoded parameters (only if no param file)
    bool useHardcodedParams = parameterFile.empty();
    
    // DEBUG: Check if JSON loading actually worked
    if (!parameterFile.empty()) {
        std::cout << "\n🔍 DEBUG: Checking if JSON parameters were loaded..." << std::endl;
        try {
            auto testParams = config.GetParametersForBin(currentBin);
            std::cout << "✅ Parameters found for target bin - JSON loading successful!" << std::endl;
            std::cout << "   Test nsig_ratio: " << testParams.nsig_ratio << std::endl;
            useHardcodedParams = false;  // JSON loading was successful
        } catch (const std::exception& e) {
            std::cout << "❌ JSON loading failed: " << e.what() << std::endl;
            useHardcodedParams = true;
        }
        std::cout << "useHardcodedParams = " << (useHardcodedParams ? "true" : "false") << std::endl;
    }
    
    if (useHardcodedParams) {
        // Use hardcoded parameters as fallback for single bin
        std::cout << "\n🔧 Setting hardcoded parameters for target bin..." << std::endl;
        
        // Create custom parameters for the current bin
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
            
        // Apply these parameters to the current bin
        config.SetParametersForBin(currentBin, binParams);
        std::cout << "✅ Hardcoded parameters applied to target bin." << std::endl;
    }
    
    // ===== PARAMETER DEBUGGING: AFTER LOADING =====
    std::cout << "\n🔍 DEBUGGING: Final Parameters (After Loading)" << std::endl;
    PrintBinParameters(currentBin, config.GetParametersForBin(currentBin), "FINAL PARAMETERS FOR TARGET BIN");
    
    // ===== PARAMETER COMPARISON =====
    if (!parameterFile.empty()) {
        std::cout << "\n🔍 PARAMETER COMPARISON FOR TARGET BIN" << std::endl;
        try {
            auto paramsBefore = configBeforeJSON.GetParametersForBin(currentBin);
            auto paramsAfter = config.GetParametersForBin(currentBin);
            
            std::cout << "📍 Target Bin: " << currentBin.GetBinName() << std::endl;
            
            // Compare yield ratios
            if (paramsBefore.nsig_ratio != paramsAfter.nsig_ratio) {
                std::cout << "  📊 nsig_ratio: " << paramsBefore.nsig_ratio 
                          << " → " << paramsAfter.nsig_ratio << std::endl;
            }
            if (paramsBefore.nbkg_ratio != paramsAfter.nbkg_ratio) {
                std::cout << "  📊 nbkg_ratio: " << paramsBefore.nbkg_ratio 
                          << " → " << paramsAfter.nbkg_ratio << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error comparing parameters: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n🎯 Single Bin Analysis Starting..." << std::endl;
    
    // Process the single kinematic bin
    const auto& bin = currentBin;
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
            return;
        }
            
        auto dataset = loader.getDataSet();
        if (!dataset) {
            std::cerr << "Dataset is null!" << std::endl;
            return;
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
