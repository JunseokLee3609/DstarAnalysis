#ifndef JSON_PARAMETER_UTILS_H
#define JSON_PARAMETER_UTILS_H

#include "JSONParameterLoader.h"
#include "DStarFitConfig.h"

// Helper function to load bin parameters from JSON using automatic parameter matching
DStarBinParameters LoadBinParametersFromJSON(const JSONParameterLoader& jsonLoader, const BinIdentifier& binId) {
    DStarBinParameters binParams;
    
    // Get PDF types from JSON  
    std::string signalPdfType = jsonLoader.getPDFType(binId, "signal");
    std::string backgroundPdfType = jsonLoader.getPDFType(binId, "background");
    
    std::cout << "[JSON Utils] Signal PDF: " << signalPdfType 
              << ", Background PDF: " << backgroundPdfType << std::endl;
    
    // Set Signal PDF parameters based on type
    if (signalPdfType == "Gaussian") {
        binParams.signalPdfType = PDFType::Gaussian;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.gaussianParams.mean = param.value;
        binParams.gaussianParams.mean_min = param.min;
        binParams.gaussianParams.mean_max = param.max;
        
        param = jsonLoader.getParameter(binId, "signal_sigma");
        binParams.gaussianParams.sigma = param.value;
        binParams.gaussianParams.sigma_min = param.min;
        binParams.gaussianParams.sigma_max = param.max;
        
    } else if (signalPdfType == "DoubleGaussian") {
        binParams.signalPdfType = PDFType::DoubleGaussian;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.doubleGaussianParams.mean = param.value;
        binParams.doubleGaussianParams.mean_min = param.min;
        binParams.doubleGaussianParams.mean_max = param.max;
        
        param = jsonLoader.getParameter(binId, "signal_sigma1");
        binParams.doubleGaussianParams.sigma1 = param.value;
        binParams.doubleGaussianParams.sigma1_min = param.min;
        binParams.doubleGaussianParams.sigma1_max = param.max;
        
        param = jsonLoader.getParameter(binId, "signal_sigma2");
        binParams.doubleGaussianParams.sigma2 = param.value;
        binParams.doubleGaussianParams.sigma2_min = param.min;
        binParams.doubleGaussianParams.sigma2_max = param.max;
        
        param = jsonLoader.getParameter(binId, "signal_fraction");
        binParams.doubleGaussianParams.fraction = param.value;
        binParams.doubleGaussianParams.fraction_min = param.min;
        binParams.doubleGaussianParams.fraction_max = param.max;
        
    } else if (signalPdfType == "CrystalBall") {
        binParams.signalPdfType = PDFType::CrystalBall;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.crystalBallParams.mean = param.value;
        binParams.crystalBallParams.mean_min = param.min;
        binParams.crystalBallParams.mean_max = param.max;
        
        param = jsonLoader.getParameter(binId, "signal_sigma");
        binParams.crystalBallParams.sigma = param.value;
        binParams.crystalBallParams.sigma_min = param.min;
        binParams.crystalBallParams.sigma_max = param.max;
        
        param = jsonLoader.getParameter(binId, "signal_alpha");
        binParams.crystalBallParams.alpha = param.value;
        binParams.crystalBallParams.alpha_min = param.min;
        binParams.crystalBallParams.alpha_max = param.max;
        
        param = jsonLoader.getParameter(binId, "signal_n");
        binParams.crystalBallParams.n = param.value;
        binParams.crystalBallParams.n_min = param.min;
        binParams.crystalBallParams.n_max = param.max;
    }
    
    // Set Background PDF parameters based on type
    if (backgroundPdfType == "ThresholdFunction") {
        binParams.backgroundPdfType = PDFType::ThresholdFunction;
        auto param = jsonLoader.getParameter(binId, "background_p0");
        binParams.thresholdFuncParams.p0_init = param.value;
        binParams.thresholdFuncParams.p0_min = param.min;
        binParams.thresholdFuncParams.p0_max = param.max;
        
        param = jsonLoader.getParameter(binId, "background_p1");
        binParams.thresholdFuncParams.p1_init = param.value;
        binParams.thresholdFuncParams.p1_min = param.min;
        binParams.thresholdFuncParams.p1_max = param.max;
        
        param = jsonLoader.getParameter(binId, "background_m_pi");
        binParams.thresholdFuncParams.m_pi_value = param.value;
        
    } else if (backgroundPdfType == "Exponential") {
        binParams.backgroundPdfType = PDFType::Exponential;
        auto param = jsonLoader.getParameter(binId, "background_lambda");
        if (!jsonLoader.hasParameter(binId, "background_lambda")) {
            // Try alternative names
            param = jsonLoader.getParameter(binId, "background_tau");
        }
        binParams.exponentialParams.lambda = param.value;
        binParams.exponentialParams.lambda_min = param.min;
        binParams.exponentialParams.lambda_max = param.max;
        
    } else if (backgroundPdfType == "Chebychev") {
        binParams.backgroundPdfType = PDFType::Chebychev;
        auto param = jsonLoader.getParameter(binId, "background_c0");
        binParams.chebychevParams.coefficients = {param.value};
        binParams.chebychevParams.coef_min = {param.min};
        binParams.chebychevParams.coef_max = {param.max};
        
        if (jsonLoader.hasParameter(binId, "background_c1")) {
            param = jsonLoader.getParameter(binId, "background_c1");
            binParams.chebychevParams.coefficients.push_back(param.value);
            binParams.chebychevParams.coef_min.push_back(param.min);
            binParams.chebychevParams.coef_max.push_back(param.max);
        }
    }
    
    // Load yield parameters
    auto param = jsonLoader.getParameter(binId, "nsig_ratio");
    binParams.nsig_ratio = param.value;
    binParams.nsig_min_ratio = param.min;
    binParams.nsig_max_ratio = param.max;
    
    param = jsonLoader.getParameter(binId, "nbkg_ratio");
    binParams.nbkg_ratio = param.value;
    binParams.nbkg_min_ratio = param.min;
    binParams.nbkg_max_ratio = param.max;
    
    return binParams;
}

#endif // JSON_PARAMETER_UTILS_H