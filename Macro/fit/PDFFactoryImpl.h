#ifndef PDF_FACTORY_IMPL_H
#define PDF_FACTORY_IMPL_H

#include "PDFFactory.h"
#include "TestFramework.h"
#include <typeinfo>

/**
 * @brief Concrete implementation of IPDFFactory that wraps the original PDFFactory
 * 
 * This adapter class allows the original PDFFactory to work with the dependency injection
 * framework by implementing the IPDFFactory interface.
 */
class PDFFactoryImpl : public IPDFFactory {
public:
    explicit PDFFactoryImpl(RooRealVar* massVar) 
        : factory_(std::make_unique<PDFFactory>(massVar)) {}
    
    ~PDFFactoryImpl() = default;
    
    std::unique_ptr<RooAbsPdf> CreateSignalPDF(const std::string& type, const void* params) override {
        return CreatePDFInternal(type, params, true);
    }
    
    std::unique_ptr<RooAbsPdf> CreateBackgroundPDF(const std::string& type, const void* params) override {
        return CreatePDFInternal(type, params, false);
    }
    
    const std::vector<std::unique_ptr<RooRealVar>>& GetParameters() const override {
        return factory_->GetParameters();
    }
    
    void ClearParameters() override {
        factory_->ClearParameters();
    }
    
private:
    std::unique_ptr<PDFFactory> factory_;
    
    std::unique_ptr<RooAbsPdf> CreatePDFInternal(const std::string& type, const void* params, bool isSignal) {
        if (!factory_) return nullptr;
        
        try {
            // Signal PDFs
            if (isSignal) {
                if (type == "Gaussian") {
                    const auto* p = static_cast<const PDFParams::GaussianParams*>(params);
                    return factory_->CreateGaussian(*p);
                }
                else if (type == "DoubleGaussian") {
                    const auto* p = static_cast<const PDFParams::DoubleGaussianParams*>(params);
                    return factory_->CreateDoubleGaussian(*p);
                }
                else if (type == "CrystalBall") {
                    const auto* p = static_cast<const PDFParams::CrystalBallParams*>(params);
                    return factory_->CreateCrystalBall(*p);
                }
                else if (type == "DBCrystalBall") {
                    const auto* p = static_cast<const PDFParams::DBCrystalBallParams*>(params);
                    return factory_->CreateDBCrystalBall(*p);
                }
                else if (type == "DoubleDBCrystalBall") {
                    const auto* p = static_cast<const PDFParams::DoubleDBCrystalBallParams*>(params);
                    return factory_->CreateDoubleDBCrystalBall(*p);
                }
                else if (type == "Voigtian") {
                    const auto* p = static_cast<const PDFParams::VoigtianParams*>(params);
                    return factory_->CreateVoigtian(*p);
                }
            }
            // Background PDFs
            else {
                if (type == "Exponential") {
                    const auto* p = static_cast<const PDFParams::ExponentialBkgParams*>(params);
                    return factory_->CreateExponential(*p);
                }
                else if (type == "Chebychev") {
                    const auto* p = static_cast<const PDFParams::ChebychevBkgParams*>(params);
                    return factory_->CreateChebychev(*p);
                }
                else if (type == "Polynomial") {
                    const auto* p = static_cast<const PDFParams::PolynomialBkgParams*>(params);
                    return factory_->CreatePolynomial(*p);
                }
                else if (type == "Phenomenological") {
                    const auto* p = static_cast<const PDFParams::PhenomenologicalParams*>(params);
                    return factory_->CreatePhenomenological(*p);
                }
                else if (type == "ExpErf") {
                    const auto* p = static_cast<const PDFParams::ExpErfBkgParams*>(params);
                    return factory_->CreateExpErf(*p);
                }
                else if (type == "DstBg") {
                    const auto* p = static_cast<const PDFParams::DstBkgParams*>(params);
                    return factory_->CreateDstBg(*p);
                }
                else if (type == "ThresholdFunction") {
                    const auto* p = static_cast<const PDFParams::ThresholdFuncParams*>(params);
                    return factory_->CreateThresholdFunction(*p);
                }
            }
            
            // Unknown type
            std::cerr << "Unknown PDF type: " << type << std::endl;
            return nullptr;
            
        } catch (const std::exception& e) {
            std::cerr << "Error creating PDF of type " << type << ": " << e.what() << std::endl;
            return nullptr;
        }
    }
};

/**
 * @brief Factory function to create a PDFFactoryImpl instance
 * 
 * This function provides a convenient way to create a PDFFactoryImpl that implements
 * the IPDFFactory interface, making it compatible with dependency injection.
 */
inline std::unique_ptr<IPDFFactory> CreatePDFFactory(RooRealVar* massVar) {
    return std::make_unique<PDFFactoryImpl>(massVar);
}

#endif // PDF_FACTORY_IMPL_H