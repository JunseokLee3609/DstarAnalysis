#ifndef PARAMS_H
#define PARAMS_H

#include <vector>

namespace PDFParams {

    // Gaussian Parameters
    struct GaussianParams {
        double mean = 1.864;
        double sigma = 0.01;
        double mean_min = 1.86;
        double mean_max = 1.87;
        double sigma_min = 0.001;
        double sigma_max = 0.02;
    };

    // Double Gaussian Parameters
    struct DoubleGaussianParams {
        double mean1 = 0.0;
        double sigma1 = 1.0;
        double mean2 = 0.0;
        double sigma2 = 2.0;
        double mean1_min = -10.0;
        double mean1_max = 10.0;
        double sigma1_min = 0.1;
        double sigma1_max = 10.0;
        double mean2_min = -10.0;
        double mean2_max = 10.0;
        double sigma2_min = 0.1;
        double sigma2_max = 10.0;
        double fraction = 0.5;
        double fraction_min = 0.0;
        double fraction_max = 1.0;
    };

    // Crystal Ball Parameters
    struct CrystalBallParams {
        double mean = 0.0;
        double sigma = 1.0;
        double alpha = 1.0;
        double n = 1.0;
        double mean_min = -10.0;
        double mean_max = 10.0;
        double sigma_min = 0.1;
        double sigma_max = 10.0;
        double alpha_min = 0.1;
        double alpha_max = 10.0;
        double n_min = 0.1;
        double n_max = 10.0;
    };

    // Double-Sided Crystal Ball Parameters
    struct DBCrystalBallParams {
        double mean = 0.0;
        double sigmaL = 1.0;
        double sigmaR = 1.0;
        double alphaL = 1.0;
        double nL = 1.0;
        double alphaR = 1.0;
        double nR = 1.0;
        double mean_min = -10.0;
        double mean_max = 10.0;
        double sigmaL_min = 0.1;
        double sigmaL_max = 10.0;
        double sigmaR_min = 0.1;
        double sigmaR_max = 10.0;
        double alphaL_min = 0.1;
        double alphaL_max = 10.0;
        double nL_min = 0.1;
        double nL_max = 10.0;
        double alphaR_min = 0.1;
        double alphaR_max = 10.0;
        double nR_min = 0.1;
        double nR_max = 10.0;
    };

    // Voigtian Parameters
    struct VoigtianParams {
        double mean = 0.0;
        double sigma = 1.0;
        double width = 1.0;
        double mean_min = -10.0;
        double mean_max = 10.0;
        double sigma_min = 0.1;
        double sigma_max = 10.0;
        double width_min = 0.1;
        double width_max = 10.0;
    };

    // Breit-Wigner Parameters
    struct BreitWignerParams {
        double mean = 0.0;
        double width = 1.0;
        double mean_min = -10.0;
        double mean_max = 10.0;
        double width_min = 0.1;
        double width_max = 10.0;
    };

    // Phenomenological Parameters
    struct PhenomenologicalParams {
        double p0;
        double p0_min;
        double p0_max;
        double p1;
        double p1_min; 
        double p1_max; 
        double p2; 
        double p2_min; 
        double p2_max;
    };

    // Exponential Background Parameters
    struct ExponentialBkgParams {
        double lambda = -1.0;
        double lambda_min = -10.0;
        double lambda_max = 0.0;
    };

    // Chebychev Background Parameters
    struct ChebychevBkgParams {
        std::vector<double> coefficients = {0.0};
        std::vector<double> coef_min = {-10.0};
        std::vector<double> coef_max = {10.0};
    };

    // Polynomial Background Parameters
    struct PolynomialBkgParams {
        std::vector<double> coefficients; 
        std::vector<double> coef_min;
        std::vector<double> coef_max;
    };

}

#endif // PARAMS_H