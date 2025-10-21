#ifndef DSTAR_FIT_TYPES_H
#define DSTAR_FIT_TYPES_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#if __cplusplus >= 201703L
#include <variant>
#endif

#include "Params.h"

enum class PDFType {
    Gaussian = 0,
    DoubleGaussian = 1,
    CrystalBall = 2,
    DBCrystalBall = 3,
    DoubleDBCrystalBall = 4,
    Voigtian = 5,
    BreitWigner = 6,
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

struct DStarBinParameters {
    PDFType signalPdfType = PDFType::DBCrystalBall;
    PDFType backgroundPdfType = PDFType::Phenomenological;

    double nsig_ratio = 0.01;
    double nsig_min_ratio = 0.0;
    double nsig_max_ratio = 1.0;
    double nbkg_ratio = 0.01;
    double nbkg_min_ratio = 0.0;
    double nbkg_max_ratio = 1.0;

    std::map<std::string, double> signalParamValues;
    std::map<std::string, std::pair<double, double>> signalParamRanges;
    std::map<std::string, double> backgroundParamValues;
    std::map<std::string, std::pair<double, double>> backgroundParamRanges;

    PDFParams::GaussianParams gaussianParams;
    PDFParams::DoubleGaussianParams doubleGaussianParams;
    PDFParams::CrystalBallParams crystalBallParams;
    PDFParams::DBCrystalBallParams dbCrystalBallParams;
    PDFParams::DoubleDBCrystalBallParams doubleDBCrystalBallParams;
    PDFParams::VoigtianParams voigtianParams;
    PDFParams::BreitWignerParams breitWignerParams;

    PDFParams::ExponentialBkgParams exponentialParams;
    PDFParams::ChebychevBkgParams chebychevParams;
    PDFParams::PhenomenologicalParams phenomenologicalParams;
    PDFParams::Phenomenological2Params phenomenological2Params;
    PDFParams::PolynomialBkgParams polynomialParams;
    PDFParams::ThresholdFuncParams thresholdFuncParams;
    PDFParams::ExpErfBkgParams expErfParams;
    PDFParams::DstBkgParams dstBkgParams;
    PDFParams::DstD0Params dstD0Params;

    PDFParams::DBCrystalBallParams signalParams;
    PDFParams::PhenomenologicalParams backgroundParams;

    DStarBinParameters() {
        SetDefaultSignalParams();
        SetDefaultBackgroundParams();
    }

    template<typename T>
    const T& GetSignalParams() const;

    template<typename T>
    const T& GetBackgroundParams() const;

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
            default:                           f(dbCrystalBallParams); break;
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
            default:                           f(phenomenologicalParams); break;
        }
    }

#if __cplusplus >= 201703L
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
            default: return dbCrystalBallParams;
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
            default: return phenomenologicalParams;
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

#endif // DSTAR_FIT_TYPES_H
