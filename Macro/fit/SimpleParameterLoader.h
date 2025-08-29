#ifndef SIMPLE_PARAMETER_LOADER_H
#define SIMPLE_PARAMETER_LOADER_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "DStarFitConfig.h"

/**
 * @brief Simple parameter loader from text files
 * 
 * 텍스트 파일 형식:
 * # 주석
 * [bin_name]
 * signal_pdf=DoubleGaussian
 * background_pdf=ThresholdFunction
 * signal.mean=0.1455
 * signal.sigma1=0.0005
 * background.p0_init=1.0
 * ...
 */
class SimpleParameterLoader {
public:
    explicit SimpleParameterLoader(const std::string& paramFile = "") {
        if (!paramFile.empty()) {
            LoadFromFile(paramFile);
        }
    }
    
    /**
     * @brief 파일에서 파라미터 로드
     */
    bool LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open parameter file: " << filename << std::endl;
            return false;
        }
        
        std::string line, currentSection;
        while (std::getline(file, line)) {
            // 빈 줄이나 주석 무시
            if (line.empty() || line[0] == '#') continue;
            
            // 섹션 헤더 처리 [bin_name]
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            
            // key=value 파싱
            auto equalPos = line.find('=');
            if (equalPos != std::string::npos && !currentSection.empty()) {
                std::string key = line.substr(0, equalPos);
                std::string value = line.substr(equalPos + 1);
                
                // 공백 제거
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                parameters_[currentSection][key] = value;
            }
        }
        
        file.close();
        return true;
    }
    
    /**
     * @brief bin에 해당하는 DStarBinParameters 반환
     */
    DStarBinParameters GetParametersForBin(const KinematicBin& bin) {
        std::string binKey = GetBinKey(bin);
        return GetParametersForBin(binKey);
    }
    
    /**
     * @brief bin 이름으로 DStarBinParameters 반환
     */
    DStarBinParameters GetParametersForBin(const std::string& binName) {
        DStarBinParameters params;
        
        if (parameters_.find(binName) == parameters_.end()) {
            std::cout << "No parameters found for bin: " << binName << ", using defaults" << std::endl;
            return params; // 기본값 반환
        }
        
        auto& binParams = parameters_[binName];
        
        // PDF 타입 설정
        if (binParams.count("signal_pdf")) {
            params.signalPdfType = StringToPDFType(binParams["signal_pdf"]);
        }
        if (binParams.count("background_pdf")) {
            params.backgroundPdfType = StringToPDFType(binParams["background_pdf"]);
        }
        
        // Yield parameters
        SetDoubleParam(binParams, "nsig_ratio", params.nsig_ratio);
        SetDoubleParam(binParams, "nsig_min_ratio", params.nsig_min_ratio);
        SetDoubleParam(binParams, "nsig_max_ratio", params.nsig_max_ratio);
        SetDoubleParam(binParams, "nbkg_ratio", params.nbkg_ratio);
        SetDoubleParam(binParams, "nbkg_min_ratio", params.nbkg_min_ratio);
        SetDoubleParam(binParams, "nbkg_max_ratio", params.nbkg_max_ratio);
        
        // Signal PDF parameters에 따라 설정
        SetSignalParameters(binParams, params);
        SetBackgroundParameters(binParams, params);
        
        return params;
    }
    
    /**
     * @brief 템플릿 파라미터 파일 생성
     */
    void CreateTemplateFile(const std::string& filename, const std::vector<KinematicBin>& bins) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot create template file: " << filename << std::endl;
            return;
        }
        
        file << "# D* Analysis Parameter Configuration\n";
        file << "# Format: [bin_name] followed by key=value pairs\n";
        file << "# Available signal PDFs: Gaussian, DoubleGaussian, CrystalBall, DBCrystalBall, Voigtian, BreitWigner\n";
        file << "# Available background PDFs: Exponential, Chebychev, Phenomenological, Polynomial, ThresholdFunction, ExpErf, DstBkg\n\n";
        
        for (const auto& bin : bins) {
            std::string binKey = GetBinKey(bin);
            file << "[" << binKey << "]\n";
            file << "# pT: " << bin.pTMin << "-" << bin.pTMax << ", cos: " << bin.cosMin << "-" << bin.cosMax << "\n";
            file << "signal_pdf=DoubleGaussian\n";
            file << "background_pdf=ThresholdFunction\n";
            file << "\n# Yield parameters\n";
            file << "nsig_ratio=0.01\n";
            file << "nsig_min_ratio=0.0\n";
            file << "nsig_max_ratio=1.0\n";
            file << "nbkg_ratio=0.01\n";
            file << "nbkg_min_ratio=0.0\n";
            file << "nbkg_max_ratio=1.0\n";
            file << "\n# Signal parameters (DoubleGaussian)\n";
            file << "signal.mean=0.1455\n";
            file << "signal.mean_min=0.1452\n";
            file << "signal.mean_max=0.1458\n";
            file << "signal.sigma1=0.0005\n";
            file << "signal.sigma1_min=0.0001\n";
            file << "signal.sigma1_max=0.01\n";
            file << "signal.sigma2=0.001\n";
            file << "signal.sigma2_min=0.0001\n";
            file << "signal.sigma2_max=0.01\n";
            file << "signal.fraction=0.7\n";
            file << "signal.fraction_min=0.0\n";
            file << "signal.fraction_max=1.0\n";
            file << "\n# Background parameters (ThresholdFunction)\n";
            file << "background.p0_init=1.0\n";
            file << "background.p0_min=-10.0\n";
            file << "background.p0_max=10.0\n";
            file << "background.p1_init=-1.0\n";
            file << "background.p1_min=-10.0\n";
            file << "background.p1_max=10.0\n";
            file << "background.m_pi_value=0.13957\n";
            file << "\n";
        }
        
        file.close();
        std::cout << "Template parameter file created: " << filename << std::endl;
    }
    
    /**
     * @brief 로드된 파라미터 정보 출력
     */
    void PrintLoadedParameters() const {
        for (const auto& bin : parameters_) {
            std::cout << "\n=== " << bin.first << " ===" << std::endl;
            for (const auto& param : bin.second) {
                std::cout << "  " << param.first << " = " << param.second << std::endl;
            }
        }
    }
    
private:
    std::map<std::string, std::map<std::string, std::string>> parameters_;
    
    std::string GetBinKey(const KinematicBin& bin) const {
        return "pT_" + std::to_string(bin.pTMin) + "_" + std::to_string(bin.pTMax) + 
               "_cos_" + std::to_string(bin.cosMin) + "_" + std::to_string(bin.cosMax);
    }
    
    PDFType StringToPDFType(const std::string& str) const {
        if (str == "Gaussian") return PDFType::Gaussian;
        if (str == "DoubleGaussian") return PDFType::DoubleGaussian;
        if (str == "CrystalBall") return PDFType::CrystalBall;
        if (str == "DBCrystalBall") return PDFType::DBCrystalBall;
        if (str == "Voigtian") return PDFType::Voigtian;
        if (str == "BreitWigner") return PDFType::BreitWigner;
        if (str == "Exponential") return PDFType::Exponential;
        if (str == "Chebychev") return PDFType::Chebychev;
        if (str == "Phenomenological") return PDFType::Phenomenological;
        if (str == "Polynomial") return PDFType::Polynomial;
        if (str == "ThresholdFunction") return PDFType::ThresholdFunction;
        if (str == "ExpErf") return PDFType::ExpErf;
        if (str == "DstBkg") return PDFType::DstBkg;
        
        std::cerr << "Unknown PDF type: " << str << ", using default" << std::endl;
        return PDFType::DoubleGaussian;
    }
    
    void SetDoubleParam(const std::map<std::string, std::string>& params, 
                        const std::string& key, double& target) {
        if (params.count(key)) {
            target = std::stod(params.at(key));
        }
    }
    
    void SetSignalParameters(const std::map<std::string, std::string>& params, DStarBinParameters& binParams) {
        // DoubleGaussian parameters
        SetDoubleParam(params, "signal.mean", binParams.doubleGaussianParams.mean);
        SetDoubleParam(params, "signal.mean_min", binParams.doubleGaussianParams.mean_min);
        SetDoubleParam(params, "signal.mean_max", binParams.doubleGaussianParams.mean_max);
        SetDoubleParam(params, "signal.sigma1", binParams.doubleGaussianParams.sigma1);
        SetDoubleParam(params, "signal.sigma1_min", binParams.doubleGaussianParams.sigma1_min);
        SetDoubleParam(params, "signal.sigma1_max", binParams.doubleGaussianParams.sigma1_max);
        SetDoubleParam(params, "signal.sigma2", binParams.doubleGaussianParams.sigma2);
        SetDoubleParam(params, "signal.sigma2_min", binParams.doubleGaussianParams.sigma2_min);
        SetDoubleParam(params, "signal.sigma2_max", binParams.doubleGaussianParams.sigma2_max);
        SetDoubleParam(params, "signal.fraction", binParams.doubleGaussianParams.fraction);
        SetDoubleParam(params, "signal.fraction_min", binParams.doubleGaussianParams.fraction_min);
        SetDoubleParam(params, "signal.fraction_max", binParams.doubleGaussianParams.fraction_max);
        
        // DBCrystalBall parameters
        SetDoubleParam(params, "signal.sigma", binParams.dbCrystalBallParams.sigma);
        SetDoubleParam(params, "signal.sigma_min", binParams.dbCrystalBallParams.sigma_min);
        SetDoubleParam(params, "signal.sigma_max", binParams.dbCrystalBallParams.sigma_max);
        SetDoubleParam(params, "signal.alphaL", binParams.dbCrystalBallParams.alphaL);
        SetDoubleParam(params, "signal.alphaL_min", binParams.dbCrystalBallParams.alphaL_min);
        SetDoubleParam(params, "signal.alphaL_max", binParams.dbCrystalBallParams.alphaL_max);
        SetDoubleParam(params, "signal.alphaR", binParams.dbCrystalBallParams.alphaR);
        SetDoubleParam(params, "signal.alphaR_min", binParams.dbCrystalBallParams.alphaR_min);
        SetDoubleParam(params, "signal.alphaR_max", binParams.dbCrystalBallParams.alphaR_max);
        SetDoubleParam(params, "signal.nL", binParams.dbCrystalBallParams.nL);
        SetDoubleParam(params, "signal.nL_min", binParams.dbCrystalBallParams.nL_min);
        SetDoubleParam(params, "signal.nL_max", binParams.dbCrystalBallParams.nL_max);
        SetDoubleParam(params, "signal.nR", binParams.dbCrystalBallParams.nR);
        SetDoubleParam(params, "signal.nR_min", binParams.dbCrystalBallParams.nR_min);
        SetDoubleParam(params, "signal.nR_max", binParams.dbCrystalBallParams.nR_max);
        
        // Gaussian parameters
        SetDoubleParam(params, "signal.mean", binParams.gaussianParams.mean);
        SetDoubleParam(params, "signal.mean_min", binParams.gaussianParams.mean_min);
        SetDoubleParam(params, "signal.mean_max", binParams.gaussianParams.mean_max);
        SetDoubleParam(params, "signal.sigma", binParams.gaussianParams.sigma);
        SetDoubleParam(params, "signal.sigma_min", binParams.gaussianParams.sigma_min);
        SetDoubleParam(params, "signal.sigma_max", binParams.gaussianParams.sigma_max);
    }
    
    void SetBackgroundParameters(const std::map<std::string, std::string>& params, DStarBinParameters& binParams) {
        // ThresholdFunction parameters
        SetDoubleParam(params, "background.p0_init", binParams.thresholdFuncParams.p0_init);
        SetDoubleParam(params, "background.p0_min", binParams.thresholdFuncParams.p0_min);
        SetDoubleParam(params, "background.p0_max", binParams.thresholdFuncParams.p0_max);
        SetDoubleParam(params, "background.p1_init", binParams.thresholdFuncParams.p1_init);
        SetDoubleParam(params, "background.p1_min", binParams.thresholdFuncParams.p1_min);
        SetDoubleParam(params, "background.p1_max", binParams.thresholdFuncParams.p1_max);
        SetDoubleParam(params, "background.m_pi_value", binParams.thresholdFuncParams.m_pi_value);
        
        // Phenomenological parameters
        SetDoubleParam(params, "background.p0", binParams.phenomenologicalParams.p0);
        SetDoubleParam(params, "background.p0_min", binParams.phenomenologicalParams.p0_min);
        SetDoubleParam(params, "background.p0_max", binParams.phenomenologicalParams.p0_max);
        SetDoubleParam(params, "background.p1", binParams.phenomenologicalParams.p1);
        SetDoubleParam(params, "background.p1_min", binParams.phenomenologicalParams.p1_min);
        SetDoubleParam(params, "background.p1_max", binParams.phenomenologicalParams.p1_max);
        SetDoubleParam(params, "background.p2", binParams.phenomenologicalParams.p2);
        SetDoubleParam(params, "background.p2_min", binParams.phenomenologicalParams.p2_min);
        SetDoubleParam(params, "background.p2_max", binParams.phenomenologicalParams.p2_max);
        
        // Exponential parameters
        SetDoubleParam(params, "background.lambda", binParams.exponentialParams.lambda);
        SetDoubleParam(params, "background.lambda_min", binParams.exponentialParams.lambda_min);
        SetDoubleParam(params, "background.lambda_max", binParams.exponentialParams.lambda_max);
    }
};

/**
 * @brief 편의 함수들
 */
namespace ParameterLoaderUtils {
    
    /**
     * @brief 기본 템플릿 파일 생성
     */
    inline void CreateDefaultTemplate(const std::string& filename) {
        std::vector<KinematicBin> bins = {
            KinematicBin(4.0, 6.0, -2.0, 2.0, 0, 100),
            KinematicBin(6.0, 8.0, -2.0, 2.0, 0, 100),
            KinematicBin(8.0, 12.0, -2.0, 2.0, 0, 100),
            KinematicBin(12.0, 100.0, -2.0, 2.0, 0, 100)
        };
        
        SimpleParameterLoader loader;
        loader.CreateTemplateFile(filename, bins);
    }
    
    /**
     * @brief DStarFitConfig에 외부 파라미터 적용
     */
    inline void LoadParametersToConfig(DStarFitConfig& config, const std::string& paramFile) {
        SimpleParameterLoader loader(paramFile);
        
        auto allBins = config.GetAllKinematicBins();
        for (const auto& bin : allBins) {
            auto params = loader.GetParametersForBin(bin);
            config.SetParametersForBin(bin, params);
            
            std::cout << "Loaded parameters for bin: " << bin.GetBinName() 
                      << " (Signal: " << static_cast<int>(params.signalPdfType) 
                      << ", Background: " << static_cast<int>(params.backgroundPdfType) << ")" << std::endl;
        }
    }
}

#endif // SIMPLE_PARAMETER_LOADER_H