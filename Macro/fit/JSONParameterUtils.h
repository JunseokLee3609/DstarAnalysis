#ifndef JSON_PARAMETER_UTILS_H
#define JSON_PARAMETER_UTILS_H

#include "JSONParameterLoader.h"
#include "DStarFitConfig.h"
#include <map>
#include <string>

// Structure to hold parameter fixed flags information
struct ParameterFixedInfo {
    std::map<std::string, bool> fixedFlags;
    
    void addFixedFlag(const std::string& paramName, bool isFixed) {
        fixedFlags[paramName] = isFixed;
    }
    
    bool isParameterFixed(const std::string& paramName) const {
        auto it = fixedFlags.find(paramName);
        return it != fixedFlags.end() ? it->second : false;
    }
    
    void printFixedFlags() const {
        std::cout << "[ParameterFixedInfo] Fixed flags:" << std::endl;
        for (const auto& pair : fixedFlags) {
            if (pair.second) {
                std::cout << "  " << pair.first << ": FIXED" << std::endl;
            }
        }
    }
};

// Helper function to load bin parameters from JSON using automatic parameter matching
// Returns both the parameters and the fixed flags information
std::pair<DStarBinParameters, ParameterFixedInfo> LoadBinParametersFromJSONWithFixedInfo(const JSONParameterLoader& jsonLoader, const BinIdentifier& binId) {
    DStarBinParameters binParams;
    ParameterFixedInfo fixedInfo;
    
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
        fixedInfo.addFixedFlag("mean_gauss", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigma");
        binParams.gaussianParams.sigma = param.value;
        binParams.gaussianParams.sigma_min = param.min;
        binParams.gaussianParams.sigma_max = param.max;
        fixedInfo.addFixedFlag("sigma_gauss", param.isFixed);
        
    } else if (signalPdfType == "DoubleGaussian") {
        binParams.signalPdfType = PDFType::DoubleGaussian;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.doubleGaussianParams.mean = param.value;
        binParams.doubleGaussianParams.mean_min = param.min;
        binParams.doubleGaussianParams.mean_max = param.max;
        fixedInfo.addFixedFlag("mean_doublegauss", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigma1");
        binParams.doubleGaussianParams.sigma1 = param.value;
        binParams.doubleGaussianParams.sigma1_min = param.min;
        binParams.doubleGaussianParams.sigma1_max = param.max;
        fixedInfo.addFixedFlag("sigma1_doublegauss", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigma2");
        binParams.doubleGaussianParams.sigma2 = param.value;
        binParams.doubleGaussianParams.sigma2_min = param.min;
        binParams.doubleGaussianParams.sigma2_max = param.max;
        fixedInfo.addFixedFlag("sigma2_doublegauss", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_fraction");
        binParams.doubleGaussianParams.fraction = param.value;
        binParams.doubleGaussianParams.fraction_min = param.min;
        binParams.doubleGaussianParams.fraction_max = param.max;
        fixedInfo.addFixedFlag("fraction_doublegauss", param.isFixed);
        
    } else if (signalPdfType == "CrystalBall") {
        binParams.signalPdfType = PDFType::CrystalBall;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.crystalBallParams.mean = param.value;
        binParams.crystalBallParams.mean_min = param.min;
        binParams.crystalBallParams.mean_max = param.max;
        fixedInfo.addFixedFlag("mean_cb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigma");
        binParams.crystalBallParams.sigma = param.value;
        binParams.crystalBallParams.sigma_min = param.min;
        binParams.crystalBallParams.sigma_max = param.max;
        fixedInfo.addFixedFlag("sigma_cb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_alpha");
        binParams.crystalBallParams.alpha = param.value;
        binParams.crystalBallParams.alpha_min = param.min;
        binParams.crystalBallParams.alpha_max = param.max;
        fixedInfo.addFixedFlag("alpha_cb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_n");
        binParams.crystalBallParams.n = param.value;
        binParams.crystalBallParams.n_min = param.min;
        binParams.crystalBallParams.n_max = param.max;
        fixedInfo.addFixedFlag("n_cb", param.isFixed);
        
    } else if (signalPdfType == "DBCrystalBall") {
        binParams.signalPdfType = PDFType::DBCrystalBall;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.dbCrystalBallParams.mean = param.value;
        binParams.dbCrystalBallParams.mean_min = param.min;
        binParams.dbCrystalBallParams.mean_max = param.max;
        fixedInfo.addFixedFlag("mean_dbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigmaL");
        binParams.dbCrystalBallParams.sigmaL = param.value;
        binParams.dbCrystalBallParams.sigmaL_min = param.min;
        binParams.dbCrystalBallParams.sigmaL_max = param.max;
        fixedInfo.addFixedFlag("sigmaL_dbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigmaR");
        binParams.dbCrystalBallParams.sigmaR = param.value;
        binParams.dbCrystalBallParams.sigmaR_min = param.min;
        binParams.dbCrystalBallParams.sigmaR_max = param.max;
        fixedInfo.addFixedFlag("sigmaR_dbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_alphaL");
        binParams.dbCrystalBallParams.alphaL = param.value;
        binParams.dbCrystalBallParams.alphaL_min = param.min;
        binParams.dbCrystalBallParams.alphaL_max = param.max;
        fixedInfo.addFixedFlag("alphaL_dbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_alphaR");
        binParams.dbCrystalBallParams.alphaR = param.value;
        binParams.dbCrystalBallParams.alphaR_min = param.min;
        binParams.dbCrystalBallParams.alphaR_max = param.max;
        fixedInfo.addFixedFlag("alphaR_dbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_nL");
        binParams.dbCrystalBallParams.nL = param.value;
        binParams.dbCrystalBallParams.nL_min = param.min;
        binParams.dbCrystalBallParams.nL_max = param.max;
        fixedInfo.addFixedFlag("nL_dbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_nR");
        binParams.dbCrystalBallParams.nR = param.value;
        binParams.dbCrystalBallParams.nR_min = param.min;
        binParams.dbCrystalBallParams.nR_max = param.max;
        fixedInfo.addFixedFlag("nR_dbcb", param.isFixed);
        
    } else if (signalPdfType == "DoubleDBCrystalBall") {
        binParams.signalPdfType = PDFType::DoubleDBCrystalBall;
        auto param = jsonLoader.getParameter(binId, "signal_mean1");
        binParams.doubleDBCrystalBallParams.mean1 = param.value;
        binParams.doubleDBCrystalBallParams.mean_min = param.min;
        binParams.doubleDBCrystalBallParams.mean_max = param.max;
        fixedInfo.addFixedFlag("mean1_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_mean2");
        binParams.doubleDBCrystalBallParams.mean2 = param.value;
        // mean_min/max already set
        fixedInfo.addFixedFlag("mean2_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigmaL1");
        binParams.doubleDBCrystalBallParams.sigmaL1 = param.value;
        binParams.doubleDBCrystalBallParams.sigmaL_min = param.min;
        binParams.doubleDBCrystalBallParams.sigmaL_max = param.max;
        fixedInfo.addFixedFlag("sigmaL1_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigmaL2");
        binParams.doubleDBCrystalBallParams.sigmaL2 = param.value;
        fixedInfo.addFixedFlag("sigmaL2_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigmaR1");
        binParams.doubleDBCrystalBallParams.sigmaR1 = param.value;
        binParams.doubleDBCrystalBallParams.sigmaR_min = param.min;
        binParams.doubleDBCrystalBallParams.sigmaR_max = param.max;
        fixedInfo.addFixedFlag("sigmaR1_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigmaR2");
        binParams.doubleDBCrystalBallParams.sigmaR2 = param.value;
        fixedInfo.addFixedFlag("sigmaR2_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_alphaL1");
        binParams.doubleDBCrystalBallParams.alphaL1 = param.value;
        binParams.doubleDBCrystalBallParams.alphaL_min = param.min;
        binParams.doubleDBCrystalBallParams.alphaL_max = param.max;
        fixedInfo.addFixedFlag("alphaL1_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_alphaL2");
        binParams.doubleDBCrystalBallParams.alphaL2 = param.value;
        fixedInfo.addFixedFlag("alphaL2_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_alphaR1");
        binParams.doubleDBCrystalBallParams.alphaR1 = param.value;
        binParams.doubleDBCrystalBallParams.alphaR_min = param.min;
        binParams.doubleDBCrystalBallParams.alphaR_max = param.max;
        fixedInfo.addFixedFlag("alphaR1_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_alphaR2");
        binParams.doubleDBCrystalBallParams.alphaR2 = param.value;
        fixedInfo.addFixedFlag("alphaR2_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_nL1");
        binParams.doubleDBCrystalBallParams.nL1 = param.value;
        binParams.doubleDBCrystalBallParams.nL_min = param.min;
        binParams.doubleDBCrystalBallParams.nL_max = param.max;
        fixedInfo.addFixedFlag("nL1_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_nL2");
        binParams.doubleDBCrystalBallParams.nL2 = param.value;
        fixedInfo.addFixedFlag("nL2_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_nR1");
        binParams.doubleDBCrystalBallParams.nR1 = param.value;
        binParams.doubleDBCrystalBallParams.nR_min = param.min;
        binParams.doubleDBCrystalBallParams.nR_max = param.max;
        fixedInfo.addFixedFlag("nR1_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_nR2");
        binParams.doubleDBCrystalBallParams.nR2 = param.value;
        fixedInfo.addFixedFlag("nR2_ddbcb", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_fraction");
        binParams.doubleDBCrystalBallParams.fraction = param.value;
        binParams.doubleDBCrystalBallParams.fraction_min = param.min;
        binParams.doubleDBCrystalBallParams.fraction_max = param.max;
        fixedInfo.addFixedFlag("fraction_ddbcb", param.isFixed);
        
    } else if (signalPdfType == "Voigtian") {
        binParams.signalPdfType = PDFType::Voigtian;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.voigtianParams.mean = param.value;
        binParams.voigtianParams.mean_min = param.min;
        binParams.voigtianParams.mean_max = param.max;
        fixedInfo.addFixedFlag("mean_voigt", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_sigma");
        binParams.voigtianParams.sigma = param.value;
        binParams.voigtianParams.sigma_min = param.min;
        binParams.voigtianParams.sigma_max = param.max;
        fixedInfo.addFixedFlag("sigma_voigt", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_width");
        binParams.voigtianParams.width = param.value;
        binParams.voigtianParams.width_min = param.min;
        binParams.voigtianParams.width_max = param.max;
        fixedInfo.addFixedFlag("width_voigt", param.isFixed);
        
    } else if (signalPdfType == "BreitWigner") {
        binParams.signalPdfType = PDFType::BreitWigner;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.breitWignerParams.mean = param.value;
        binParams.breitWignerParams.mean_min = param.min;
        binParams.breitWignerParams.mean_max = param.max;
        fixedInfo.addFixedFlag("mean_bw", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "signal_width");
        binParams.breitWignerParams.width = param.value;
        binParams.breitWignerParams.width_min = param.min;
        binParams.breitWignerParams.width_max = param.max;
        fixedInfo.addFixedFlag("width_bw", param.isFixed);
    }
    
    // Set Background PDF parameters based on type
    if (backgroundPdfType == "ThresholdFunction") {
        binParams.backgroundPdfType = PDFType::ThresholdFunction;
        auto param = jsonLoader.getParameter(binId, "background_p0");
        binParams.thresholdFuncParams.p0_init = param.value;
        binParams.thresholdFuncParams.p0_min = param.min;
        binParams.thresholdFuncParams.p0_max = param.max;
        fixedInfo.addFixedFlag("p0_threshold", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_p1");
        binParams.thresholdFuncParams.p1_init = param.value;
        binParams.thresholdFuncParams.p1_min = param.min;
        binParams.thresholdFuncParams.p1_max = param.max;
        fixedInfo.addFixedFlag("p1_threshold", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_m_pi");
        binParams.thresholdFuncParams.m_pi_value = param.value;
        fixedInfo.addFixedFlag("m_pi_threshold", param.isFixed);
        
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
        fixedInfo.addFixedFlag("lambda_exp", param.isFixed);
        
    } else if (backgroundPdfType == "Chebychev") {
        binParams.backgroundPdfType = PDFType::Chebychev;
        auto param = jsonLoader.getParameter(binId, "background_c0");
        binParams.chebychevParams.coefficients = {param.value};
        binParams.chebychevParams.coef_min = {param.min};
        binParams.chebychevParams.coef_max = {param.max};
        fixedInfo.addFixedFlag("c0_cheb", param.isFixed);
        
        if (jsonLoader.hasParameter(binId, "background_c1")) {
            param = jsonLoader.getParameter(binId, "background_c1");
            binParams.chebychevParams.coefficients.push_back(param.value);
            binParams.chebychevParams.coef_min.push_back(param.min);
            binParams.chebychevParams.coef_max.push_back(param.max);
            fixedInfo.addFixedFlag("c1_cheb", param.isFixed);
        }
        
        if (jsonLoader.hasParameter(binId, "background_c2")) {
            param = jsonLoader.getParameter(binId, "background_c2");
            binParams.chebychevParams.coefficients.push_back(param.value);
            binParams.chebychevParams.coef_min.push_back(param.min);
            binParams.chebychevParams.coef_max.push_back(param.max);
            fixedInfo.addFixedFlag("c2_cheb", param.isFixed);
        }
        
    } else if (backgroundPdfType == "Phenomenological") {
        binParams.backgroundPdfType = PDFType::Phenomenological;
        auto param = jsonLoader.getParameter(binId, "background_p0");
        binParams.phenomenologicalParams.p0 = param.value;
        binParams.phenomenologicalParams.p0_min = param.min;
        binParams.phenomenologicalParams.p0_max = param.max;
        fixedInfo.addFixedFlag("p0_phenom", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_p1");
        binParams.phenomenologicalParams.p1 = param.value;
        binParams.phenomenologicalParams.p1_min = param.min;
        binParams.phenomenologicalParams.p1_max = param.max;
        fixedInfo.addFixedFlag("p1_phenom", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_p2");
        binParams.phenomenologicalParams.p2 = param.value;
        binParams.phenomenologicalParams.p2_min = param.min;
        binParams.phenomenologicalParams.p2_max = param.max;
        fixedInfo.addFixedFlag("p2_phenom", param.isFixed);
        
    } else if (backgroundPdfType == "Polynomial") {
        binParams.backgroundPdfType = PDFType::Polynomial;
        auto param = jsonLoader.getParameter(binId, "background_c0");
        binParams.polynomialParams.coefficients = {param.value};
        binParams.polynomialParams.coef_min = {param.min};
        binParams.polynomialParams.coef_max = {param.max};
        fixedInfo.addFixedFlag("c0_poly", param.isFixed);
        
        if (jsonLoader.hasParameter(binId, "background_c1")) {
            param = jsonLoader.getParameter(binId, "background_c1");
            binParams.polynomialParams.coefficients.push_back(param.value);
            binParams.polynomialParams.coef_min.push_back(param.min);
            binParams.polynomialParams.coef_max.push_back(param.max);
            fixedInfo.addFixedFlag("c1_poly", param.isFixed);
        }
        
        if (jsonLoader.hasParameter(binId, "background_c2")) {
            param = jsonLoader.getParameter(binId, "background_c2");
            binParams.polynomialParams.coefficients.push_back(param.value);
            binParams.polynomialParams.coef_min.push_back(param.min);
            binParams.polynomialParams.coef_max.push_back(param.max);
            fixedInfo.addFixedFlag("c2_poly", param.isFixed);
        }
        
    } else if (backgroundPdfType == "ExpErf") {
        binParams.backgroundPdfType = PDFType::ExpErf;
        auto param = jsonLoader.getParameter(binId, "background_err_mu");
        binParams.expErfParams.err_mu = param.value;
        binParams.expErfParams.err_mu_min = param.min;
        binParams.expErfParams.err_mu_max = param.max;
        fixedInfo.addFixedFlag("err_mu_experf", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_err_sigma");
        binParams.expErfParams.err_sigma = param.value;
        binParams.expErfParams.err_sigma_min = param.min;
        binParams.expErfParams.err_sigma_max = param.max;
        fixedInfo.addFixedFlag("err_sigma_experf", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_m_lambda");
        binParams.expErfParams.m_lambda = param.value;
        binParams.expErfParams.m_lambda_min = param.min;
        binParams.expErfParams.m_lambda_max = param.max;
        fixedInfo.addFixedFlag("m_lambda_experf", param.isFixed);
        
    } else if (backgroundPdfType == "DstBkg") {
        binParams.backgroundPdfType = PDFType::DstBkg;
        auto param = jsonLoader.getParameter(binId, "background_p0");
        binParams.dstBkgParams.p0 = param.value;
        binParams.dstBkgParams.p0_min = param.min;
        binParams.dstBkgParams.p0_max = param.max;
        fixedInfo.addFixedFlag("p0_dstbkg", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_p1");
        binParams.dstBkgParams.p1 = param.value;
        binParams.dstBkgParams.p1_min = param.min;
        binParams.dstBkgParams.p1_max = param.max;
        fixedInfo.addFixedFlag("p1_dstbkg", param.isFixed);
        
        param = jsonLoader.getParameter(binId, "background_p2");
        binParams.dstBkgParams.p2 = param.value;
        binParams.dstBkgParams.p2_min = param.min;
        binParams.dstBkgParams.p2_max = param.max;
        fixedInfo.addFixedFlag("p2_dstbkg", param.isFixed);
    }
    
    // Load yield parameters
    auto param = jsonLoader.getParameter(binId, "nsig_ratio");
    binParams.nsig_ratio = param.value;
    binParams.nsig_min_ratio = param.min;
    binParams.nsig_max_ratio = param.max;
    fixedInfo.addFixedFlag("nsig_ratio", param.isFixed);
    
    param = jsonLoader.getParameter(binId, "nbkg_ratio");
    binParams.nbkg_ratio = param.value;
    binParams.nbkg_min_ratio = param.min;
    binParams.nbkg_max_ratio = param.max;
    fixedInfo.addFixedFlag("nbkg_ratio", param.isFixed);
    
    // Debug print
    std::cout << "[JSON Utils] Collected fixed flags:" << std::endl;
    fixedInfo.printFixedFlags();
    
    return std::make_pair(binParams, fixedInfo);
}

// Backward compatibility function - calls the new function and discards fixed info
DStarBinParameters LoadBinParametersFromJSON(const JSONParameterLoader& jsonLoader, const BinIdentifier& binId) {
    return LoadBinParametersFromJSONWithFixedInfo(jsonLoader, binId).first;
}

#endif // JSON_PARAMETER_UTILS_H