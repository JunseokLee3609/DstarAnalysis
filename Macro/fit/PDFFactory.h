#ifndef PDF_FACTORY_H
#define PDF_FACTORY_H

#include <memory>
#include <string>
#include <vector>
#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooAddPdf.h"
#include "RooCBShape.h"
#include "RooCrystalBall.h"
#include "RooVoigtian.h"
#include "RooExponential.h"
#include "RooChebychev.h"
#include "RooPolynomial.h"
#include "RooGenericPdf.h"
#include "RooArgList.h"
#include "TString.h"
#include "TMath.h"
#include "Params.h"

class PDFFactory {
public:
    PDFFactory(RooRealVar* massVar);
    ~PDFFactory() = default;

    // Signal PDF creators
    std::unique_ptr<RooAbsPdf> CreateGaussian(const PDFParams::GaussianParams& params, const std::string& name = "gauss");
    std::unique_ptr<RooAbsPdf> CreateDoubleGaussian(const PDFParams::DoubleGaussianParams& params, const std::string& name = "doublegauss");
    std::unique_ptr<RooAbsPdf> CreateCrystalBall(const PDFParams::CrystalBallParams& params, const std::string& name = "cb");
    std::unique_ptr<RooAbsPdf> CreateDBCrystalBall(const PDFParams::DBCrystalBallParams& params, const std::string& name = "dbcb");
    std::unique_ptr<RooAbsPdf> CreateDoubleDBCrystalBall(const PDFParams::DoubleDBCrystalBallParams& params, const std::string& name = "ddbcb");
    std::unique_ptr<RooAbsPdf> CreateVoigtian(const PDFParams::VoigtianParams& params, const std::string& name = "voigt");
    
    // Background PDF creators
    std::unique_ptr<RooAbsPdf> CreateExponential(const PDFParams::ExponentialBkgParams& params, const std::string& name = "exp");
    std::unique_ptr<RooAbsPdf> CreateChebychev(const PDFParams::ChebychevBkgParams& params, const std::string& name = "cheb");
    std::unique_ptr<RooAbsPdf> CreatePolynomial(const PDFParams::PolynomialBkgParams& params, const std::string& name = "poly");
    std::unique_ptr<RooAbsPdf> CreatePhenomenological(const PDFParams::PhenomenologicalParams& params, const std::string& name = "phenom");
    std::unique_ptr<RooAbsPdf> CreateExpErf(const PDFParams::ExpErfBkgParams& params, const std::string& name = "experf");
    std::unique_ptr<RooAbsPdf> CreateDstBg(const PDFParams::DstBkgParams& params, const std::string& name = "dstbg");
    std::unique_ptr<RooAbsPdf> CreateThresholdFunction(const PDFParams::ThresholdFuncParams& params, const std::string& name = "threshold");
    
    // Parameter management
    const std::vector<std::unique_ptr<RooRealVar>>& GetParameters() const { return parameters_; }
    void ClearParameters() { parameters_.clear(); }
    
private:
    RooRealVar* massVar_;
    std::vector<std::unique_ptr<RooRealVar>> parameters_;
    
    // Helper methods
    template<typename T>
    std::unique_ptr<RooRealVar> CreateParameter(const std::string& name, const std::string& title, 
                                                T value, T min, T max);
    void StoreParameter(std::unique_ptr<RooRealVar> param);
};

template<typename T>
std::unique_ptr<RooRealVar> PDFFactory::CreateParameter(const std::string& name, const std::string& title,
                                                        T value, T min, T max) {
    return std::make_unique<RooRealVar>(name.c_str(), title.c_str(), value, min, max);
}

// Implementation of signal PDF creators
inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateGaussian(const PDFParams::GaussianParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    auto mean = CreateParameter("mean_" + name, "Mean", params.mean, params.mean_min, params.mean_max);
    auto sigma = CreateParameter("sigma_" + name, "Sigma", params.sigma, params.sigma_min, params.sigma_max);
    
    RooRealVar* meanPtr = mean.get();
    RooRealVar* sigmaPtr = sigma.get();
    
    StoreParameter(std::move(mean));
    StoreParameter(std::move(sigma));
    
    return std::make_unique<RooGaussian>(name.c_str(), ("Gaussian_" + name).c_str(),
                                         *massVar_, *meanPtr, *sigmaPtr);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateDoubleGaussian(const PDFParams::DoubleGaussianParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    auto mean = CreateParameter("mean_" + name, "Mean", params.mean, params.mean_min, params.mean_max);
    auto sigma1 = CreateParameter("sigma1_" + name, "First Sigma", params.sigma1, params.sigma1_min, params.sigma1_max);
    auto sigma2 = CreateParameter("sigma2_" + name, "Second Sigma", params.sigma2, params.sigma2_min, params.sigma2_max);
    auto frac = CreateParameter("frac_" + name, "Fraction", params.fraction, params.fraction_min, params.fraction_max);
    
    RooRealVar* meanPtr = mean.get();
    RooRealVar* sigma1Ptr = sigma1.get();
    RooRealVar* sigma2Ptr = sigma2.get();
    RooRealVar* fracPtr = frac.get();
    
    StoreParameter(std::move(mean));
    StoreParameter(std::move(sigma1));
    StoreParameter(std::move(sigma2));
    StoreParameter(std::move(frac));
    
    auto gauss1 = std::make_unique<RooGaussian>(("gauss1_" + name).c_str(), "First Gaussian",
                                                *massVar_, *meanPtr, *sigma1Ptr);
    auto gauss2 = std::make_unique<RooGaussian>(("gauss2_" + name).c_str(), "Second Gaussian",
                                                *massVar_, *meanPtr, *sigma2Ptr);
    
    RooGaussian* gauss1Ptr = gauss1.release();
    RooGaussian* gauss2Ptr = gauss2.release();
    
    return std::make_unique<RooAddPdf>(name.c_str(), ("DoubleGaussian_" + name).c_str(),
                                       RooArgList(*gauss1Ptr, *gauss2Ptr), *fracPtr);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateCrystalBall(const PDFParams::CrystalBallParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    auto mean = CreateParameter("mean_" + name, "Mean", params.mean, params.mean_min, params.mean_max);
    auto sigma = CreateParameter("sigma_" + name, "Sigma", params.sigma, params.sigma_min, params.sigma_max);
    auto alpha = CreateParameter("alpha_" + name, "Alpha", params.alpha, params.alpha_min, params.alpha_max);
    auto n = CreateParameter("n_" + name, "N", params.n, params.n_min, params.n_max);
    
    RooRealVar* meanPtr = mean.get();
    RooRealVar* sigmaPtr = sigma.get();
    RooRealVar* alphaPtr = alpha.get();
    RooRealVar* nPtr = n.get();
    
    StoreParameter(std::move(mean));
    StoreParameter(std::move(sigma));
    StoreParameter(std::move(alpha));
    StoreParameter(std::move(n));
    
    return std::make_unique<RooCBShape>(name.c_str(), ("CrystalBall_" + name).c_str(),
                                        *massVar_, *meanPtr, *sigmaPtr, *alphaPtr, *nPtr);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateDBCrystalBall(const PDFParams::DBCrystalBallParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    auto mean = CreateParameter("mean_" + name, "Mean", params.mean, params.mean_min, params.mean_max);
    auto sigma = CreateParameter("sigma_" + name, "Sigma", params.sigma, params.sigma_min, params.sigma_max);
    auto alphaL = CreateParameter("alphaL_" + name, "Alpha Left", params.alphaL, params.alphaL_min, params.alphaL_max);
    auto nL = CreateParameter("nL_" + name, "N Left", params.nL, params.nL_min, params.nL_max);
    auto alphaR = CreateParameter("alphaR_" + name, "Alpha Right", params.alphaR, params.alphaR_min, params.alphaR_max);
    auto nR = CreateParameter("nR_" + name, "N Right", params.nR, params.nR_min, params.nR_max);
    
    RooRealVar* meanPtr = mean.get();
    RooRealVar* sigmaPtr = sigma.get();
    RooRealVar* alphaLPtr = alphaL.get();
    RooRealVar* nLPtr = nL.get();
    RooRealVar* alphaRPtr = alphaR.get();
    RooRealVar* nRPtr = nR.get();
    
    StoreParameter(std::move(mean));
    StoreParameter(std::move(sigma));
    StoreParameter(std::move(alphaL));
    StoreParameter(std::move(nL));
    StoreParameter(std::move(alphaR));
    StoreParameter(std::move(nR));
    
    return std::make_unique<RooCrystalBall>(name.c_str(), ("DoubleSidedCrystalBall_" + name).c_str(),
                                            *massVar_, *meanPtr, *sigmaPtr, *alphaLPtr, *nLPtr, *alphaRPtr, *nRPtr);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateVoigtian(const PDFParams::VoigtianParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    auto mean = CreateParameter("mean_" + name, "Mean", params.mean, params.mean_min, params.mean_max);
    auto width = CreateParameter("width_" + name, "Width", params.width, params.width_min, params.width_max);
    auto sigma = CreateParameter("sigma_" + name, "Sigma", params.sigma, params.sigma_min, params.sigma_max);
    
    RooRealVar* meanPtr = mean.get();
    RooRealVar* widthPtr = width.get();
    RooRealVar* sigmaPtr = sigma.get();
    
    StoreParameter(std::move(mean));
    StoreParameter(std::move(width));
    StoreParameter(std::move(sigma));
    
    return std::make_unique<RooVoigtian>(name.c_str(), ("Voigtian_" + name).c_str(),
                                         *massVar_, *meanPtr, *widthPtr, *sigmaPtr);
}

// Implementation of background PDF creators
inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateExponential(const PDFParams::ExponentialBkgParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    auto lambda = CreateParameter("lambda_" + name, "Lambda", params.lambda, params.lambda_min, params.lambda_max);
    RooRealVar* lambdaPtr = lambda.get();
    StoreParameter(std::move(lambda));
    
    return std::make_unique<RooExponential>(name.c_str(), ("Exponential_" + name).c_str(),
                                            *massVar_, *lambdaPtr);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateChebychev(const PDFParams::ChebychevBkgParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    RooArgList coeffs;
    for (size_t i = 0; i < params.coefficients.size(); ++i) {
        auto coeff = CreateParameter("c" + std::to_string(i) + "_" + name, 
                                     "Coefficient " + std::to_string(i),
                                     params.coefficients[i], params.coef_min[i], params.coef_max[i]);
        coeffs.add(*coeff.get());
        StoreParameter(std::move(coeff));
    }
    
    return std::make_unique<RooChebychev>(name.c_str(), ("Chebychev_" + name).c_str(),
                                          *massVar_, coeffs);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreatePolynomial(const PDFParams::PolynomialBkgParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    RooArgList coeffs;
    for (size_t i = 0; i < params.coefficients.size(); ++i) {
        auto coeff = CreateParameter("p" + std::to_string(i) + "_" + name,
                                     "Polynomial Coefficient " + std::to_string(i),
                                     params.coefficients[i], params.coef_min[i], params.coef_max[i]);
        coeffs.add(*coeff.get());
        StoreParameter(std::move(coeff));
    }
    
    return std::make_unique<RooPolynomial>(name.c_str(), ("Polynomial_" + name).c_str(),
                                           *massVar_, coeffs);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreateThresholdFunction(const PDFParams::ThresholdFuncParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    // Validate parameters
    try {
        params.validate();
    } catch (const PDFParams::ParameterException& e) {
        std::cerr << "ThresholdFunction parameter validation failed: " << e.what() << std::endl;
        return nullptr;
    }
    
    // Create p0 parameter (amplitude/normalization)
    auto p0 = CreateParameter("thresh_p0_" + name, "Threshold amplitude", 
                             params.p0_init, params.p0_min, params.p0_max);
    
    // Create p1 parameter (exponential slope)
    auto p1 = CreateParameter("thresh_p1_" + name, "Exponential slope",
                             params.p1_init, params.p1_min, params.p1_max);
    
    // Create M_pi constant
    auto m_pi_const = CreateParameter("m_pi_" + name, "Pion mass", 
                                     params.m_pi_value, params.m_pi_value, params.m_pi_value);
    m_pi_const->setConstant(true);
    
    // Store parameter pointers before moving
    RooRealVar* p0Ptr = p0.get();
    RooRealVar* p1Ptr = p1.get();
    RooRealVar* m_piPtr = m_pi_const.get();
    
    // Store parameters
    StoreParameter(std::move(p0));
    StoreParameter(std::move(p1));
    StoreParameter(std::move(m_pi_const));
    
    // Create the threshold function formula
    // p0 * sqrt(max(0, DeltaM - M_pi)) * exp(p1 * (DeltaM - M_pi))
    std::string formula = TString::Format("@0 * TMath::Sqrt(TMath::Max(0.0, @1 - @2)) * TMath::Exp(@3 * (@1 - @2))").Data();
    
    // Create RooArgList with parameters in the correct order
    RooArgList varList;
    varList.add(*p0Ptr);        // @0
    varList.add(*massVar_);     // @1 
    varList.add(*m_piPtr);      // @2
    varList.add(*p1Ptr);        // @3
    
    // Create and return the Generic PDF
    return std::make_unique<RooGenericPdf>(name.c_str(), 
                                          ("ThresholdFunc_" + name).c_str(),
                                          formula.c_str(),
                                          varList);
}

inline std::unique_ptr<RooAbsPdf> PDFFactory::CreatePhenomenological(const PDFParams::PhenomenologicalParams& params, const std::string& name) {
    if (!massVar_) return nullptr;
    
    auto p0 = CreateParameter("phenom_p0_" + name, "Phenomenological p0", 
                             params.p0, params.p0_min, params.p0_max);
    auto p1 = CreateParameter("phenom_p1_" + name, "Phenomenological p1",
                             params.p1, params.p1_min, params.p1_max);
    auto p2 = CreateParameter("phenom_p2_" + name, "Phenomenological p2",
                             params.p2, params.p2_min, params.p2_max);
    
    RooRealVar* p0Ptr = p0.get();
    RooRealVar* p1Ptr = p1.get();
    RooRealVar* p2Ptr = p2.get();
    
    StoreParameter(std::move(p0));
    StoreParameter(std::move(p1));
    StoreParameter(std::move(p2));
    
    // Create phenomenological function: p0 * exp(p1 * x + p2 * x^2)
    std::string formula = "@0 * TMath::Exp(@1 * @3 + @2 * @3 * @3)";
    
    RooArgList varList;
    varList.add(*p0Ptr);    // @0
    varList.add(*p1Ptr);    // @1
    varList.add(*p2Ptr);    // @2
    varList.add(*massVar_); // @3
    
    return std::make_unique<RooGenericPdf>(name.c_str(), 
                                          ("Phenomenological_" + name).c_str(),
                                          formula.c_str(),
                                          varList);
}

inline void PDFFactory::StoreParameter(std::unique_ptr<RooRealVar> param) {
    parameters_.push_back(std::move(param));
}

inline PDFFactory::PDFFactory(RooRealVar* massVar) : massVar_(massVar) {}

#endif // PDF_FACTORY_H