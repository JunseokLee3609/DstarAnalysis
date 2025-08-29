#ifndef PARAMS_H
#define PARAMS_H

#include <vector>
#include <stdexcept>
#include <string>

namespace PDFParams {

    // 매개변수 유효성 검증을 위한 기본 클래스
    class ParameterException : public std::exception {
    private:
        std::string message_;
    public:
        explicit ParameterException(const std::string& msg) : message_(msg) {}
        const char* what() const noexcept override { return message_.c_str(); }
    };

    // Gaussian Parameters
    struct GaussianParams {
        double mean = 1.864;
        double sigma = 0.01;
        double mean_min = 1.86;
        double mean_max = 1.87;
        double sigma_min = 0.001;
        double sigma_max = 0.02;
        
        // 유효성 검증 메서드
        void validate() const {
            if (mean_min >= mean_max) {
                throw ParameterException("Gaussian: mean_min >= mean_max");
            }
            if (sigma_min >= sigma_max) {
                throw ParameterException("Gaussian: sigma_min >= sigma_max");
            }
            if (sigma <= 0) {
                throw ParameterException("Gaussian: sigma must be positive");
            }
            if (sigma_min <= 0) {
                throw ParameterException("Gaussian: sigma_min must be positive");
            }
            if (mean < mean_min || mean > mean_max) {
                throw ParameterException("Gaussian: mean outside allowed range");
            }
            if (sigma < sigma_min || sigma > sigma_max) {
                throw ParameterException("Gaussian: sigma outside allowed range");
            }
        }
    };

    // Double Gaussian Parameters
    struct DoubleGaussianParams {
        double mean = 0.0;
        double sigma1 = 1.0;
        double sigma2 = 2.0;
        double mean_min = -10.0;
        double mean_max = 10.0;
        double sigma1_min = 0.1;
        double sigma1_max = 10.0;
        double sigma2_min = 0.1;
        double sigma2_max = 10.0;
        double fraction = 0.5;
        double fraction_min = 0.0;
        double fraction_max = 1.0;
    };
    struct ExpErfBkgParams {
        double err_mu = 5.0;
        double err_mu_min = 4.0;
        double err_mu_max = 8.5;
        double err_sigma = 0.5;
        double err_sigma_min = 0.0;
        double err_sigma_max = 10.0;
        double m_lambda = 1.0;
        double m_lambda_min = 0.0;
        double m_lambda_max = 20.0;
    };
    struct DstBkgParams {
        double p0 = 0.1; // p0 parameter
        double p0_min = 0.0; // Minimum value for p0
        double p0_max = 5.0; // Maximum value for p0
        double p1 = 1.0; // p1 parameter
        double p1_min = 0.0; // Minimum value for p1
        double p1_max = 10.0; // Maximum value for p1
        double p2 = -2.0; // p2 parameter
        double p2_min = -10.0; // Minimum value for p2
        double p2_max = 10.0; // Maximum value for p2
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
        
        // 유효성 검증 메서드
        void validate() const {
            if (mean_min >= mean_max) {
                throw ParameterException("CrystalBall: mean_min >= mean_max");
            }
            if (sigma_min >= sigma_max) {
                throw ParameterException("CrystalBall: sigma_min >= sigma_max");
            }
            if (alpha_min >= alpha_max) {
                throw ParameterException("CrystalBall: alpha_min >= alpha_max");
            }
            if (n_min >= n_max) {
                throw ParameterException("CrystalBall: n_min >= n_max");
            }
            if (sigma <= 0) {
                throw ParameterException("CrystalBall: sigma must be positive");
            }
            if (sigma_min <= 0) {
                throw ParameterException("CrystalBall: sigma_min must be positive");
            }
            if (alpha <= 0) {
                throw ParameterException("CrystalBall: alpha must be positive");
            }
            if (n <= 0) {
                throw ParameterException("CrystalBall: n must be positive");
            }
            // 범위 검사
            if (mean < mean_min || mean > mean_max) {
                throw ParameterException("CrystalBall: mean outside allowed range");
            }
            if (sigma < sigma_min || sigma > sigma_max) {
                throw ParameterException("CrystalBall: sigma outside allowed range");
            }
            if (alpha < alpha_min || alpha > alpha_max) {
                throw ParameterException("CrystalBall: alpha outside allowed range");
            }
            if (n < n_min || n > n_max) {
                throw ParameterException("CrystalBall: n outside allowed range");
            }
        }
    };

    // Double-Sided Crystal Ball Parameters
    struct DBCrystalBallParams {
        double mean = 0.0;
        double sigmaL = 1.0;
        double sigmaR = 1.0;
        double sigma = 1.0;
        double alphaL = 1.0;
        double nL = 1.0;
        double alphaR = 1.0;
        double nR = 1.0;
        double mean_min = -10.0;
        double mean_max = 10.0;
        double sigma_min = 0.1;
        double sigma_max = 10.0;
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
    struct DoubleDBCrystalBallParams {
        double mean1 = 0.1455;
        double mean2 = 0.1455;
        double mean_min = -10.0;
        double mean_max = 10.0;

        double sigmaL1 = 1.0;
        double sigmaL2 = 1.0;
        double sigmaL_min = 0.1;
        double sigmaL_max = 10.0;

        double sigmaR2 = 1.0;
        double sigmaR1 = 1.0;
        double sigmaR_min = 0.1;
        double sigmaR_max = 10.0;
        
        double alphaL1 = 1.0;
        double alphaL2 = 1.0;
        double alphaL_min = 0.1;
        double alphaL_max = 10.0;

        double alphaR1 = 1.0;
        double alphaR2 = 1.0;
        double alphaR_min = 0.1;
        double alphaR_max = 10.0;

        double nL1 = 1.0;
        double nL2 = 1.0;
        double nL_min = 0.1;
        double nL_max = 10.0;

        double nR1 = 1.0;
        double nR2 = 1.0;
        double nR_min = 0.1;
        double nR_max = 10.0;
        
        double fraction = 0.5;
        double fraction_min = 0.0;
        double fraction_max = 1.0;
        
        
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
        double p0=0.1;
        double p0_min= 0.0;
        double p0_max=-5.0;
        double p1=2.0;
        double p1_min=10.0; 
        double p1_max=-10.0; 
        double p2=-2.0; 
        double p2_min=-10.0; 
        double p2_max=10.0;
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

    // Threshold Function Parameters
    struct ThresholdFuncParams {
        double p0_init = 1.0;        // Amplitude parameter initial value
        double p0_min = 0.0;         // Amplitude parameter minimum value
        double p0_max = 100.0;       // Amplitude parameter maximum value
        
        double p1_init = -1.0;       // Exponential slope initial value (usually negative)
        double p1_min = -10.0;       // Exponential slope minimum value
        double p1_max = 0.0;         // Exponential slope maximum value
        
        double m_pi_value = 0.13957; // Charged pion mass in GeV (customizable)
        
        // Validation method
        void validate() const {
            if (p0_min >= p0_max) {
                throw ParameterException("ThresholdFunc: p0_min >= p0_max");
            }
            if (p1_min >= p1_max) {
                throw ParameterException("ThresholdFunc: p1_min >= p1_max");
            }
            if (p0_init < p0_min || p0_init > p0_max) {
                throw ParameterException("ThresholdFunc: p0_init outside allowed range");
            }
            if (p1_init < p1_min || p1_init > p1_max) {
                throw ParameterException("ThresholdFunc: p1_init outside allowed range");
            }
            if (m_pi_value <= 0.0) {
                throw ParameterException("ThresholdFunc: m_pi_value must be positive");
            }
        }
    };

}

#endif // PARAMS_H