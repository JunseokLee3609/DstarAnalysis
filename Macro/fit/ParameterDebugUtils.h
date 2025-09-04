#ifndef PARAMETER_DEBUG_UTILS_H
#define PARAMETER_DEBUG_UTILS_H

#include <iostream>
#include <string>
#include "DStarFitConfig.h"

namespace ParameterDebug {

// Template function to get PDF type names
template<typename T> inline std::string GetPDFTypeNameDbg() { return "Unknown"; }

// Signal PDF type names
template<> inline std::string GetPDFTypeNameDbg<PDFParams::GaussianParams>() { return "Gaussian"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::DoubleGaussianParams>() { return "DoubleGaussian"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::CrystalBallParams>() { return "CrystalBall"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::DBCrystalBallParams>() { return "DBCrystalBall"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::DoubleDBCrystalBallParams>() { return "DoubleDBCrystalBall"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::VoigtianParams>() { return "Voigtian"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::BreitWignerParams>() { return "BreitWigner"; }

// Background PDF type names
template<> inline std::string GetPDFTypeNameDbg<PDFParams::ExponentialBkgParams>() { return "Exponential"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::ChebychevBkgParams>() { return "Chebychev"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::PhenomenologicalParams>() { return "Phenomenological"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::PolynomialBkgParams>() { return "Polynomial"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::ThresholdFuncParams>() { return "ThresholdFunction"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::ExpErfBkgParams>() { return "ExpErf"; }
template<> inline std::string GetPDFTypeNameDbg<PDFParams::DstBkgParams>() { return "DstBg"; }

inline void PrintBinParameters(const KinematicBin& bin, const DStarBinParameters& params, const std::string& title = "") {
    if (!title.empty()) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << title << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }

    std::cout << "📍 Bin: " << bin.GetBinName() << std::endl;
    std::cout << "   pT: [" << bin.pTMin << ", " << bin.pTMax << "] GeV/c" << std::endl;
    std::cout << "   cos(θ*): [" << bin.cosMin << ", " << bin.cosMax << "]" << std::endl;
    std::cout << "   Centrality: [" << bin.centralityMin << ", " << bin.centralityMax << "]%" << std::endl;

    std::cout << "\n🔍 PDF Configuration:" << std::endl;
    std::cout << "   Signal PDF: " << static_cast<int>(params.signalPdfType);
    switch(params.signalPdfType) {
        case PDFType::Gaussian: std::cout << " (Gaussian)"; break;
        case PDFType::DoubleGaussian: std::cout << " (DoubleGaussian)"; break;
        case PDFType::CrystalBall: std::cout << " (CrystalBall)"; break;
        case PDFType::DBCrystalBall: std::cout << " (DBCrystalBall)"; break;
        case PDFType::DoubleDBCrystalBall: std::cout << " (DoubleDBCrystalBall)"; break;
        case PDFType::Voigtian: std::cout << " (Voigtian)"; break;
        case PDFType::BreitWigner: std::cout << " (BreitWigner)"; break;
        default: std::cout << " (Other)"; break;
    }
    std::cout << std::endl;

    std::cout << "   Background PDF: " << static_cast<int>(params.backgroundPdfType);
    switch(params.backgroundPdfType) {
        case PDFType::Exponential: std::cout << " (Exponential)"; break;
        case PDFType::ThresholdFunction: std::cout << " (ThresholdFunction)"; break;
        case PDFType::Chebychev: std::cout << " (Chebychev)"; break;
        case PDFType::Phenomenological: std::cout << " (Phenomenological)"; break;
        case PDFType::Polynomial: std::cout << " (Polynomial)"; break;
        case PDFType::ExpErf: std::cout << " (ExpErf)"; break;
        case PDFType::DstBkg: std::cout << " (DstBkg)"; break;
        default: std::cout << " (Other)"; break;
    }
    std::cout << std::endl;

    // Signal parameters
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
        case PDFType::DBCrystalBall:
            std::cout << "   mean: " << params.dbCrystalBallParams.mean
                      << " [" << params.dbCrystalBallParams.mean_min << ", " << params.dbCrystalBallParams.mean_max << "]" << std::endl;
            // Prefer sigmaL/sigmaR if set, also print symmetric sigma
            std::cout << "   sigmaL: " << params.dbCrystalBallParams.sigmaL
                      << " [" << params.dbCrystalBallParams.sigmaL_min << ", " << params.dbCrystalBallParams.sigmaL_max << "]" << std::endl;
            std::cout << "   sigmaR: " << params.dbCrystalBallParams.sigmaR
                      << " [" << params.dbCrystalBallParams.sigmaR_min << ", " << params.dbCrystalBallParams.sigmaR_max << "]" << std::endl;
            std::cout << "   sigma(sym): " << params.dbCrystalBallParams.sigma
                      << " [" << params.dbCrystalBallParams.sigma_min << ", " << params.dbCrystalBallParams.sigma_max << "]" << std::endl;
            std::cout << "   alphaL: " << params.dbCrystalBallParams.alphaL
                      << " [" << params.dbCrystalBallParams.alphaL_min << ", " << params.dbCrystalBallParams.alphaL_max << "]" << std::endl;
            std::cout << "   nL: " << params.dbCrystalBallParams.nL
                      << " [" << params.dbCrystalBallParams.nL_min << ", " << params.dbCrystalBallParams.nL_max << "]" << std::endl;
            std::cout << "   alphaR: " << params.dbCrystalBallParams.alphaR
                      << " [" << params.dbCrystalBallParams.alphaR_min << ", " << params.dbCrystalBallParams.alphaR_max << "]" << std::endl;
            std::cout << "   nR: " << params.dbCrystalBallParams.nR
                      << " [" << params.dbCrystalBallParams.nR_min << ", " << params.dbCrystalBallParams.nR_max << "]" << std::endl;
            break;
        case PDFType::DoubleDBCrystalBall:
            std::cout << "   mean1: " << params.doubleDBCrystalBallParams.mean1
                      << " , mean2: " << params.doubleDBCrystalBallParams.mean2 << std::endl;
            std::cout << "   sigmaL1: " << params.doubleDBCrystalBallParams.sigmaL1
                      << ", sigmaL2: " << params.doubleDBCrystalBallParams.sigmaL2 << std::endl;
            std::cout << "   sigmaR1: " << params.doubleDBCrystalBallParams.sigmaR1
                      << ", sigmaR2: " << params.doubleDBCrystalBallParams.sigmaR2 << std::endl;
            std::cout << "   alphaL1: " << params.doubleDBCrystalBallParams.alphaL1
                      << ", alphaL2: " << params.doubleDBCrystalBallParams.alphaL2 << std::endl;
            std::cout << "   alphaR1: " << params.doubleDBCrystalBallParams.alphaR1
                      << ", alphaR2: " << params.doubleDBCrystalBallParams.alphaR2 << std::endl;
            std::cout << "   nL1: " << params.doubleDBCrystalBallParams.nL1
                      << ", nL2: " << params.doubleDBCrystalBallParams.nL2 << std::endl;
            std::cout << "   nR1: " << params.doubleDBCrystalBallParams.nR1
                      << ", nR2: " << params.doubleDBCrystalBallParams.nR2 << std::endl;
            std::cout << "   fraction: " << params.doubleDBCrystalBallParams.fraction << std::endl;
            break;
        case PDFType::Voigtian:
            std::cout << "   mean: " << params.voigtianParams.mean
                      << " [" << params.voigtianParams.mean_min << ", " << params.voigtianParams.mean_max << "]" << std::endl;
            std::cout << "   sigma: " << params.voigtianParams.sigma
                      << " [" << params.voigtianParams.sigma_min << ", " << params.voigtianParams.sigma_max << "]" << std::endl;
            std::cout << "   width: " << params.voigtianParams.width
                      << " [" << params.voigtianParams.width_min << ", " << params.voigtianParams.width_max << "]" << std::endl;
            break;
        case PDFType::BreitWigner:
            std::cout << "   mean: " << params.breitWignerParams.mean
                      << " [" << params.breitWignerParams.mean_min << ", " << params.breitWignerParams.mean_max << "]" << std::endl;
            std::cout << "   width: " << params.breitWignerParams.width
                      << " [" << params.breitWignerParams.width_min << ", " << params.breitWignerParams.width_max << "]" << std::endl;
            break;
        default:
            std::cout << "   (Parameters not displayed for this PDF type)" << std::endl;
            break;
    }

    // Background parameters
    std::cout << "\n🪨 Background Parameters:" << std::endl;
    switch(params.backgroundPdfType) {
        case PDFType::Exponential:
            std::cout << "   lambda: " << params.exponentialParams.lambda
                      << " [" << params.exponentialParams.lambda_min << ", " << params.exponentialParams.lambda_max << "]" << std::endl;
            break;
        case PDFType::Chebychev:
            std::cout << "   coefficients: [";
            for (size_t i=0;i<params.chebychevParams.coefficients.size();++i){
                std::cout << params.chebychevParams.coefficients[i];
                if (i+1<params.chebychevParams.coefficients.size()) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            break;
        case PDFType::Phenomenological:
            std::cout << "   p0: " << params.phenomenologicalParams.p0
                      << ", p1: " << params.phenomenologicalParams.p1
                      << ", p2: " << params.phenomenologicalParams.p2 << std::endl;
            break;
        case PDFType::Polynomial:
            std::cout << "   coefficients: [";
            for (size_t i=0;i<params.polynomialParams.coefficients.size();++i){
                std::cout << params.polynomialParams.coefficients[i];
                if (i+1<params.polynomialParams.coefficients.size()) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            break;
        case PDFType::ThresholdFunction:
            std::cout << "   p0_init: " << params.thresholdFuncParams.p0_init
                      << " [" << params.thresholdFuncParams.p0_min << ", " << params.thresholdFuncParams.p0_max << "]" << std::endl;
            std::cout << "   p1_init: " << params.thresholdFuncParams.p1_init
                      << " [" << params.thresholdFuncParams.p1_min << ", " << params.thresholdFuncParams.p1_max << "]" << std::endl;
            std::cout << "   m_pi: " << params.thresholdFuncParams.m_pi_value << std::endl;
            break;
        case PDFType::ExpErf:
            std::cout << "   err_mu: " << params.expErfParams.err_mu
                      << " [" << params.expErfParams.err_mu_min << ", " << params.expErfParams.err_mu_max << "]" << std::endl;
            std::cout << "   err_sigma: " << params.expErfParams.err_sigma
                      << " [" << params.expErfParams.err_sigma_min << ", " << params.expErfParams.err_sigma_max << "]" << std::endl;
            std::cout << "   m_lambda: " << params.expErfParams.m_lambda
                      << " [" << params.expErfParams.m_lambda_min << ", " << params.expErfParams.m_lambda_max << "]" << std::endl;
            break;
        case PDFType::DstBkg:
            std::cout << "   p0: " << params.dstBkgParams.p0
                      << " [" << params.dstBkgParams.p0_min << ", " << params.dstBkgParams.p0_max << "]" << std::endl;
            std::cout << "   p1: " << params.dstBkgParams.p1
                      << " [" << params.dstBkgParams.p1_min << ", " << params.dstBkgParams.p1_max << "]" << std::endl;
            std::cout << "   p2: " << params.dstBkgParams.p2
                      << " [" << params.dstBkgParams.p2_min << ", " << params.dstBkgParams.p2_max << "]" << std::endl;
            break;
        default:
            std::cout << "   (Parameters not displayed for this PDF type)" << std::endl;
            break;
    }

    // Yields
    std::cout << "\n📊 Yield Parameters:" << std::endl;
    std::cout << "   nsig_ratio: " << params.nsig_ratio
              << " [" << params.nsig_min_ratio << ", " << params.nsig_max_ratio << "]" << std::endl;
    std::cout << "   nbkg_ratio: " << params.nbkg_ratio
              << " [" << params.nbkg_min_ratio << ", " << params.nbkg_max_ratio << "]" << std::endl;

    std::cout << std::string(60, '-') << std::endl;
}

inline void PrintAllConfigParameters(const DStarFitConfig& config, const std::string& title = "") {
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

} // namespace ParameterDebug

#endif // PARAMETER_DEBUG_UTILS_H
