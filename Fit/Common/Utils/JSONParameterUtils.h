#ifndef JSON_PARAMETER_UTILS_H
#define JSON_PARAMETER_UTILS_H

#include "JSONParameterLoader.h"
#include "../Config/DStarFitConfig.h"
#include <map>
#include <string>
#include <iostream>

// Structure to hold parameter fixed flags information
struct ParameterFixedInfo {
    std::map<std::string, bool> fixedFlags;
    bool hasJSONConfig = false;
    
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

    void setHasJSONConfig(bool value) { hasJSONConfig = value; }
    bool HasJSONConfig() const { return hasJSONConfig; }
};

// Helper function to load bin parameters from JSON using automatic parameter matching
// Returns both the parameters and the fixed flags information
std::pair<DStarBinParameters, ParameterFixedInfo> LoadBinParametersFromJSONWithFixedInfo(const JSONParameterLoader& jsonLoader, const BinIdentifier& binId) {
    DStarBinParameters binParams;
    ParameterFixedInfo fixedInfo;

    bool hasJSONConfig = false;
    bool baseLoaded = false;

    if (binId.hasDcaRange()) {
        BinIdentifier baseId = binId;
        baseId.dcaMin = std::numeric_limits<double>::quiet_NaN();
        baseId.dcaMax = std::numeric_limits<double>::quiet_NaN();

        auto base = LoadBinParametersFromJSONWithFixedInfo(jsonLoader, baseId);
        binParams = base.first;
        fixedInfo = base.second;
        hasJSONConfig = base.second.HasJSONConfig();
        baseLoaded = true;
    }

    // Get PDF types from JSON  
    std::string signalPdfType = jsonLoader.getPDFType(binId, "signal");
    std::string backgroundPdfType = jsonLoader.getPDFType(binId, "background");

    if (signalPdfType.empty() && baseLoaded) {
        switch (binParams.signalPdfType) {
            case PDFType::Gaussian: signalPdfType = "Gaussian"; break;
            case PDFType::DoubleGaussian: signalPdfType = "DoubleGaussian"; break;
            case PDFType::CrystalBall: signalPdfType = "CrystalBall"; break;
            case PDFType::DBCrystalBall: signalPdfType = "DBCrystalBall"; break;
            case PDFType::DoubleDBCrystalBall: signalPdfType = "DoubleDBCrystalBall"; break;
            case PDFType::Voigtian: signalPdfType = "Voigtian"; break;
            case PDFType::BreitWigner: signalPdfType = "BreitWigner"; break;
            default: break;
        }
        if (!signalPdfType.empty()) hasJSONConfig = true;
    }

    if (backgroundPdfType.empty() && baseLoaded) {
        switch (binParams.backgroundPdfType) {
            case PDFType::Exponential: backgroundPdfType = "Exponential"; break;
            case PDFType::ThresholdFunction: backgroundPdfType = "ThresholdFunction"; break;
            case PDFType::Chebychev: backgroundPdfType = "Chebychev"; break;
            case PDFType::Phenomenological: backgroundPdfType = "Phenomenological"; break;
            case PDFType::Phenomenological2: backgroundPdfType = "Phenomenological2"; break;
            case PDFType::Polynomial: backgroundPdfType = "Polynomial"; break;
            case PDFType::ExpErf: backgroundPdfType = "ExpErf"; break;
            case PDFType::DstBkg: backgroundPdfType = "DstBkg"; break;
            case PDFType::DstD0: backgroundPdfType = "DstD0"; break;
            default: break;
        }
        if (!backgroundPdfType.empty()) hasJSONConfig = true;
    }
    
    std::cout << "[JSON Utils] Signal PDF: " << signalPdfType 
              << ", Background PDF: " << backgroundPdfType << std::endl;
    
    bool hasSignalConfig = !signalPdfType.empty();
    bool hasBackgroundConfig = !backgroundPdfType.empty();

    if (!hasSignalConfig) {
        std::cout << "[JSON Utils] No signal PDF configuration found for requested bin; using defaults." << std::endl;
    }
    if (!hasBackgroundConfig) {
        std::cout << "[JSON Utils] No background PDF configuration found for requested bin; using defaults." << std::endl;
    }

    // Set Signal PDF parameters based on type
    if (signalPdfType == "Gaussian") {
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
        binParams.signalPdfType = PDFType::DBCrystalBall;
        auto param = jsonLoader.getParameter(binId, "signal_mean");
        binParams.dbCrystalBallParams.mean = param.value;
        binParams.dbCrystalBallParams.mean_min = param.min;
        binParams.dbCrystalBallParams.mean_max = param.max;
        fixedInfo.addFixedFlag("mean_dbcb", param.isFixed);

        // Unify to symmetric sigma if available, else derive from sigmaL/sigmaR
        if (jsonLoader.hasParameter(binId, "signal_sigma")) {
            param = jsonLoader.getParameter(binId, "signal_sigma");
            binParams.dbCrystalBallParams.sigma = param.value;
            binParams.dbCrystalBallParams.sigma_min = param.min;
            binParams.dbCrystalBallParams.sigma_max = param.max;
            fixedInfo.addFixedFlag("sigma_dbcb", param.isFixed);
            std::cout << "[JSON Utils][DBCB] Using symmetric sigma from 'signal_sigma': "
                      << binParams.dbCrystalBallParams.sigma << " ["
                      << binParams.dbCrystalBallParams.sigma_min << ", "
                      << binParams.dbCrystalBallParams.sigma_max << "]" << std::endl;
        } else {
            auto pL = jsonLoader.hasParameter(binId, "signal_sigmaL")
                          ? jsonLoader.getParameter(binId, "signal_sigmaL")
                          : FitParameter();
            auto pR = jsonLoader.hasParameter(binId, "signal_sigmaR")
                          ? jsonLoader.getParameter(binId, "signal_sigmaR")
                          : FitParameter();
            binParams.dbCrystalBallParams.sigmaL = pL.value;
            binParams.dbCrystalBallParams.sigmaL_min = pL.min;
            binParams.dbCrystalBallParams.sigmaL_max = pL.max;
            fixedInfo.addFixedFlag("sigmaL_dbcb", pL.isFixed);

            binParams.dbCrystalBallParams.sigmaR = pR.value;
            binParams.dbCrystalBallParams.sigmaR_min = pR.min;
            binParams.dbCrystalBallParams.sigmaR_max = pR.max;
            fixedInfo.addFixedFlag("sigmaR_dbcb", pR.isFixed);

            // Use average as unified sigma and average of bounds as range
            binParams.dbCrystalBallParams.sigma = 0.5 * (pL.value + pR.value);
            binParams.dbCrystalBallParams.sigma_min = 0.5 * (pL.min + pR.min);
            binParams.dbCrystalBallParams.sigma_max = 0.5 * (pL.max + pR.max);
            std::cout << "[JSON Utils][DBCB] Derived symmetric sigma from L/R: sigmaL="
                      << pL.value << ", sigmaR=" << pR.value
                      << " -> sigma(sym)=" << binParams.dbCrystalBallParams.sigma << " ["
                      << binParams.dbCrystalBallParams.sigma_min << ", "
                      << binParams.dbCrystalBallParams.sigma_max << "]" << std::endl;
        }

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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
        binParams.backgroundPdfType = PDFType::Phenomenological;
        auto param = jsonLoader.getParameter(binId, "background_p0");
        binParams.phenomenologicalParams.p0 = param.value;
        binParams.phenomenologicalParams.p0_min = param.min;
        binParams.phenomenologicalParams.p0_max = param.max;
        fixedInfo.addFixedFlag("p0_phenom", param.isFixed);

        param = jsonLoader.getParameter(binId, "background_m");
        binParams.phenomenologicalParams.m = param.value;
        binParams.phenomenologicalParams.m_min = param.min;
        binParams.phenomenologicalParams.m_max = param.max;
        fixedInfo.addFixedFlag("m_phenom", param.isFixed);

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

    } else if (backgroundPdfType == "Phenomenological2" || backgroundPdfType == "Phenomelogical2") {
        hasJSONConfig = true;
        // Support both correct and typo spellings
        binParams.backgroundPdfType = PDFType::Phenomenological2;
        auto param = jsonLoader.getParameter(binId, "background_m");
        binParams.phenomenological2Params.m = param.value;
        binParams.phenomenological2Params.m_min = param.min;
        binParams.phenomenological2Params.m_max = param.max;
        fixedInfo.addFixedFlag("m_phenom2", param.isFixed);

        // lambda parameter (aliases: "p0" and "tau" also accepted for backward compatibility)
        if (jsonLoader.hasParameter(binId, "background_lambda")) {
            param = jsonLoader.getParameter(binId, "background_lambda");
        } else if (jsonLoader.hasParameter(binId, "background_p0")) {
            param = jsonLoader.getParameter(binId, "background_p0");
        } else {
            param = jsonLoader.getParameter(binId, "background_tau");
        }
        binParams.phenomenological2Params.lambda = param.value;
        binParams.phenomenological2Params.lambda_min = param.min;
        binParams.phenomenological2Params.lambda_max = param.max;
        fixedInfo.addFixedFlag("lambda_phenom2", param.isFixed);

        // Optional pion mass override
        if (jsonLoader.hasParameter(binId, "background_m_pi")) {
            param = jsonLoader.getParameter(binId, "background_m_pi");
            binParams.phenomenological2Params.m_pi_value = param.value;
        }
        
    } else if (backgroundPdfType == "Polynomial") {
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
        hasJSONConfig = true;
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
    } else if (backgroundPdfType == "DstD0") {
        hasJSONConfig = true;
        binParams.backgroundPdfType = PDFType::DstD0;
        auto param = jsonLoader.getParameter(binId, "background_p0");
        binParams.dstD0Params.p0 = param.value;
        binParams.dstD0Params.p0_min = param.min;
        binParams.dstD0Params.p0_max = param.max;
        fixedInfo.addFixedFlag("p0_dstd0", param.isFixed);

        param = jsonLoader.getParameter(binId, "background_p1");
        binParams.dstD0Params.p1 = param.value;
        binParams.dstD0Params.p1_min = param.min;
        binParams.dstD0Params.p1_max = param.max;
        fixedInfo.addFixedFlag("p1_dstd0", param.isFixed);

        param = jsonLoader.getParameter(binId, "background_p2");
        binParams.dstD0Params.p2 = param.value;
        binParams.dstD0Params.p2_min = param.min;
        binParams.dstD0Params.p2_max = param.max;
        fixedInfo.addFixedFlag("p2_dstd0", param.isFixed);
    }
    
    // Load yield parameters
    if (jsonLoader.hasParameter(binId, "nsig_ratio")) {
        auto param = jsonLoader.getParameter(binId, "nsig_ratio");
        binParams.nsig_ratio = param.value;
        binParams.nsig_min_ratio = param.min;
        binParams.nsig_max_ratio = param.max;
        fixedInfo.addFixedFlag("nsig_ratio", param.isFixed);
        hasJSONConfig = true;
    }
    if (jsonLoader.hasParameter(binId, "nbkg_ratio")) {
        auto param = jsonLoader.getParameter(binId, "nbkg_ratio");
        binParams.nbkg_ratio = param.value;
        binParams.nbkg_min_ratio = param.min;
        binParams.nbkg_max_ratio = param.max;
        fixedInfo.addFixedFlag("nbkg_ratio", param.isFixed);
        hasJSONConfig = true;
    }

    if (!hasJSONConfig) {
        if (hasSignalConfig || hasBackgroundConfig || jsonLoader.hasParameter(binId, "nsig_ratio") ||
            jsonLoader.hasParameter(binId, "nbkg_ratio")) {
            hasJSONConfig = true;
        }
    }

    fixedInfo.setHasJSONConfig(hasJSONConfig);
    if (!hasJSONConfig) {
        std::ostringstream binDesc;
        binDesc << binId.getBinKey();
        if (binId.hasDcaRange()) {
            binDesc << ".dca[" << binId.dcaMin << ", " << binId.dcaMax << "]";
        }
        std::cout << "[JSON Utils] No JSON configuration found for bin '"
                  << binDesc.str() << "'. Using default parameters." << std::endl;
    }

    // Debug print
    std::cout << "[JSON Utils] Collected fixed flags:" << std::endl;
    fixedInfo.printFixedFlags();
    
    return std::make_pair(binParams, fixedInfo);
}

inline void LoadParametersFromJSONToConfig(DStarFitConfig& config,
                                           const std::string& jsonFile,
                                           bool ignoreCentralityInMatching = true,
                                           const KinematicBin* ensureExactBin = nullptr) {
    JSONParameterLoader jsonLoader;
    jsonLoader.setIgnoreCentralityInMatching(ignoreCentralityInMatching);
    jsonLoader.loadFromFile(jsonFile);

    std::cout << "[JSON Loader] Loading parameters from: " << jsonFile << std::endl;

    auto configBins = config.GetAllKinematicBins();
    size_t successCount = 0;

    for (const auto& bin : configBins) {
        BinIdentifier binId;
        binId.ptMin = bin.pTMin;
        binId.ptMax = bin.pTMax;
        binId.cosMin = bin.cosMin;
        binId.cosMax = bin.cosMax;
        binId.centralityMin = bin.centralityMin;
        binId.centralityMax = bin.centralityMax;

        std::cout << "[JSON Loader] Loading parameters for bin: " << bin.GetBinName() << std::endl;

        try {
            auto loaded = LoadBinParametersFromJSONWithFixedInfo(jsonLoader, binId);
            DStarBinParameters binParams = loaded.first;
            ParameterFixedInfo fixedInfo = loaded.second;

            config.SetParametersForBin(bin, binParams);
            config.SetFixedFlagsForBin(bin, fixedInfo.fixedFlags);

            std::cout << "[JSON Loader] ✅ Successfully loaded parameters and fixed flags for bin: "
                      << bin.GetBinName() << std::endl;
            ++successCount;
        } catch (const std::exception& e) {
            std::cout << "[JSON Loader] ❌ Failed to load parameters for bin " << bin.GetBinName()
                      << ": " << e.what() << ". Using defaults." << std::endl;
        }
    }

    if (ensureExactBin) {
        try {
            BinIdentifier binId;
            binId.ptMin = ensureExactBin->pTMin;
            binId.ptMax = ensureExactBin->pTMax;
            binId.cosMin = ensureExactBin->cosMin;
            binId.cosMax = ensureExactBin->cosMax;
            binId.centralityMin = ensureExactBin->centralityMin;
            binId.centralityMax = ensureExactBin->centralityMax;

            auto loaded = LoadBinParametersFromJSONWithFixedInfo(jsonLoader, binId);
            config.SetParametersForBin(*ensureExactBin, loaded.first);
            config.SetFixedFlagsForBin(*ensureExactBin, loaded.second.fixedFlags);

            std::cout << "[JSON Loader] ✅ Populated parameters for EXACT current bin key: "
                      << ensureExactBin->GetBinName() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[JSON Loader] ⚠️  Could not set exact current bin key: "
                      << e.what() << std::endl;
        }
    }

    if (successCount == 0) {
        std::cout << "[JSON Loader] ⚠️  No bins were updated from JSON file." << std::endl;
    }
}

// Backward compatibility function - calls the new function and discards fixed info
DStarBinParameters LoadBinParametersFromJSON(const JSONParameterLoader& jsonLoader, const BinIdentifier& binId) {
    return LoadBinParametersFromJSONWithFixedInfo(jsonLoader, binId).first;
}

#endif // JSON_PARAMETER_UTILS_H
