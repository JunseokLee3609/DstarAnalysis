#ifndef ROBUST_PARAMETER_MANAGER_H
#define ROBUST_PARAMETER_MANAGER_H

// Fallback for DEFAULT_CPU_COUNT if not defined elsewhere
#ifndef DEFAULT_CPU_COUNT
#define DEFAULT_CPU_COUNT 4
#endif

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <iostream>
#include <stdexcept>
// Added headers needed by inline implementations
#include <algorithm>
#include <cmath>
#include <iomanip>

#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooDataSet.h"
#include "RooLinkedList.h"
#include "RooFit.h"
#include "RooArgList.h"
#include "RooArgSet.h"

#include "Params.h"
#include "Opt.h"
#include "RooGaussian.h"
#include "TFile.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TRandom.h"
#include "TLatex.h"
#include "TStopwatch.h"

// Forward declarations - only define if not already defined by Helper.h
#ifndef HELPER_H_H
enum class LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3 };

class FittingLogger {
public:
    static void setLogLevel(LogLevel level) {}
    static void info(const std::string& message) { std::cout << "[INFO] " << message << std::endl; }
    static void warning(const std::string& message) { std::cout << "[WARNING] " << message << std::endl; }
    static void error(const std::string& message) { std::cout << "[ERROR] " << message << std::endl; }
    static void debug(const std::string& message) { std::cout << "[DEBUG] " << message << std::endl; }
};

class ScopedTimer {
private:
    std::chrono::steady_clock::time_point start_time_;
    std::string operation_name_;
public:
    explicit ScopedTimer(const std::string& operation_name) 
        : operation_name_(operation_name), start_time_(std::chrono::steady_clock::now()) {}
    ~ScopedTimer() {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
        std::cout << "[TIMER] " << operation_name_ << " took " << duration.count() << " ms" << std::endl;
    }
};

class PerformanceMonitor {
public:
    static void recordOperation(const std::string& operation, double time_seconds) {}
    static void printStatistics() { std::cout << "[PERF] Performance statistics would be shown here" << std::endl; }
};
#endif // HELPER_H_H

// 매개변수 타입 열거형 - 타입 안전성 보장
enum class ParameterType {
    MEAN,
    SIGMA, 
    SIGMA1,
    SIGMA2,
    ALPHA,
    ALPHA_L,
    ALPHA_R,
    N,
    N_L,
    N_R,
    FRACTION,
    LAMBDA,
    P0, P1, P2,
    UNKNOWN
};

// PDF 타입 열거형
enum class PDFType {
    GAUSSIAN,
    DOUBLE_GAUSSIAN,
    CRYSTAL_BALL,
    DB_CRYSTAL_BALL,
    DOUBLE_DB_CRYSTAL_BALL,
    VOIGTIAN,
    EXPONENTIAL,
    CHEBYCHEV,
    POLYNOMIAL,
    PHENOMENOLOGICAL,
    UNKNOWN
};

// 매개변수 메타데이터 구조체
struct ParameterInfo {
    ParameterType type;
    std::string name;
    double defaultValue;
    double minValue;
    double maxValue;
    bool isConstant;
    double tolerance;
    std::string description;
    std::chrono::steady_clock::time_point lastModified;
    
    ParameterInfo(ParameterType t = ParameterType::UNKNOWN,
                 const std::string& n = "",
                 double defVal = 0.0,
                 double minVal = -1e6,
                 double maxVal = 1e6,
                 bool constant = false,
                 double tol = 1e-6,
                 const std::string& desc = "") 
        : type(t), name(n), defaultValue(defVal), minValue(minVal), 
          maxValue(maxVal), isConstant(constant), tolerance(tol), 
          description(desc), lastModified(std::chrono::steady_clock::now()) {}
};

// 견고한 매개변수 관리 클래스
class RobustParameterManager {
private:
    std::unordered_map<std::string, std::unique_ptr<RooRealVar>> parameters_;
    std::unordered_map<std::string, ParameterInfo> parameterInfo_;
    std::map<PDFType, std::vector<ParameterType>> pdfParameterMap_;
    
    // 상태 저장용
    std::map<std::string, std::map<std::string, double>> savedStates_;
    std::map<std::string, std::map<std::string, std::pair<double, double>>> savedRanges_;
    
    // 매개변수 이름에서 타입 추출
    ParameterType extractParameterType(const std::string& name) const;
    
    // PDF 타입별 기본 매개변수 설정
    void setupDefaultParameters();
    
    // 로깅
    void logParameterChange(const std::string& name, const std::string& action, 
                           double oldValue = 0, double newValue = 0) const;
    
public:
    RobustParameterManager();
    ~RobustParameterManager() = default;
    
    // 매개변수 생성 및 관리
    RooRealVar* createParameter(const std::string& name, 
                               ParameterType type,
                               double value, 
                               double min, 
                               double max,
                               const std::string& description = "");
    
    RooRealVar* getParameter(const std::string& name) const;
    bool hasParameter(const std::string& name) const;
    
    // 매개변수 설정 - 예외 안전성 보장
    void setParameterValue(const std::string& name, double value);
    void setParameterRange(const std::string& name, double min, double max);
    void setParameterConstant(const std::string& name, bool constant = true);
    void setParameterError(const std::string& name, double error);
    
    // 매개변수 정보 조회
    ParameterInfo getParameterInfo(const std::string& name) const;
    std::vector<std::string> getParameterNames() const;
    std::vector<std::string> getParametersByType(ParameterType type) const;
    
    // 매개변수 유효성 검증
    bool validateParameter(const std::string& name) const;
    bool validateAllParameters() const;
    std::vector<std::string> getInvalidParameters() const;
    
    // 스마트 매개변수 범위 조정 - 물리학적 제약 고려
    void smartAdjustParameterRanges(const std::vector<std::string>& paramNames, 
                                  double expandFactor = 1.5);
    void resetParameterRanges(const std::vector<std::string>& paramNames);
    
    // PDF별 매개변수 그룹 관리
    std::vector<std::string> getParametersForPDF(PDFType pdfType, 
                                                 const std::string& pdfName) const;
    void initializeParametersForPDF(PDFType pdfType, const std::string& pdfName);
    
    // 매개변수 상태 저장/복원 - 안전한 상태 관리
    void saveParameterState(const std::string& stateName);
    void restoreParameterState(const std::string& stateName);
    void clearParameterState(const std::string& stateName);
    std::vector<std::string> listSavedStates() const;
    
    // 통계 및 진단
    void printParameterSummary() const;
    void printParameterCorrelations() const;
    std::map<std::string, double> getParameterValues() const;
    std::map<std::string, double> getParameterErrors() const;
    
    // 물리학적 제약 적용
    void applyPhysicsConstraints();
    void applySensibleDefaults(PDFType pdfType);
    void optimizeInitialValues(const std::string& datasetName = "");
    
    // 매개변수 추적 및 모니터링
    void enableParameterTracking(bool enable = true);
    std::vector<std::pair<std::string, std::chrono::steady_clock::time_point>> 
        getRecentlyModifiedParameters(int minutes = 10) const;
    
private:
    bool trackingEnabled_ = false;
    mutable std::vector<std::string> modificationLog_;
};

// 피팅 전략 열거형
enum class FitStrategy {
    STANDARD,       // 기본 피팅
    ROBUST,         // 견고한 피팅 (여러 시도)
    CONSTRAINED,    // 제약 피팅
    SEQUENTIAL,     // 순차적 피팅
    ADAPTIVE        // 적응적 피팅
};

// 개선된 피팅 전략 관리 클래스
class SmartFitStrategyManager {
private:
    FitStrategy currentStrategy_;
    std::map<FitStrategy, std::function<RooFitResult*(RooAbsPdf*, RooDataSet*)>> strategies_;
    std::vector<FitStrategy> fallbackStrategies_;
    
    // 피팅 성공률 추적
    std::map<FitStrategy, std::pair<int, int>> strategyStats_; // {성공, 총시도}
    
public:
    SmartFitStrategyManager();
    
    void setStrategy(FitStrategy strategy);
    FitStrategy getCurrentStrategy() const;
    void setFallbackStrategies(const std::vector<FitStrategy>& strategies);
    
    // 개선된 피팅 실행 - 자동 전략 전환
    std::unique_ptr<RooFitResult> executeFit(RooAbsPdf* pdf, 
                                           RooDataSet* data,
                                           RobustParameterManager* paramManager = nullptr,
                                           const std::map<std::string, bool>& options = {});
    
    // 적응적 피팅 - 가장 성공 가능성이 높은 전략부터 시도
    std::unique_ptr<RooFitResult> adaptiveFit(RooAbsPdf* pdf, 
                                            RooDataSet* data,
                                            RobustParameterManager* paramManager = nullptr);
    
    // 피팅 품질 평가 - 더 포괄적인 평가
    double evaluateFitQuality(const RooFitResult* result, 
                             RooAbsPdf* pdf, 
                             RooDataSet* data) const;
    
    bool isFitAcceptable(const RooFitResult* result,
                        RooAbsPdf* pdf,
                        RooDataSet* data,
                        double qualityThreshold = 0.7) const;
    
    // 전략별 성공률 통계
    void printStrategyStatistics() const;
    FitStrategy recommendStrategy() const;
    
private:
    void updateStrategyStats(FitStrategy strategy, bool success);
    std::unique_ptr<RooFitResult> executeStandardFit(RooAbsPdf* pdf, RooDataSet* data);
    std::unique_ptr<RooFitResult> executeRobustFit(RooAbsPdf* pdf, RooDataSet* data, RobustParameterManager* paramManager);
    std::unique_ptr<RooFitResult> executeConstrainedFit(RooAbsPdf* pdf, RooDataSet* data);
    std::unique_ptr<RooFitResult> executeSequentialFit(RooAbsPdf* pdf, RooDataSet* data, RobustParameterManager* paramManager);
    std::unique_ptr<RooFitResult> executeAdaptiveFit(RooAbsPdf* pdf, RooDataSet* data, RobustParameterManager* paramManager);
};

// 포괄적인 피팅 진단 클래스
class ComprehensiveFitDiagnostics {
public:
    struct DiagnosticResult {
        bool converged;
        bool hasValidErrors;
        bool parametersAtLimits;
        bool covarianceMatrixValid;
        double chi2ndf;
        double logLikelihood;
        double aicScore;  // AIC (Akaike Information Criterion)
        double bicScore;  // BIC (Bayesian Information Criterion)
        int status;
        int ndf;
        std::vector<std::string> problematicParameters;
        std::vector<std::string> warnings;
        std::string recommendation;
        double qualityScore;  // 0-1 점수
        std::string detailedReport;
    };
    
    static DiagnosticResult diagnose(const RooFitResult* result,
                                   RooAbsPdf* pdf,
                                   RooDataSet* data,
                                   RobustParameterManager* paramManager = nullptr);
    
    static void printDiagnostics(const DiagnosticResult& result);
    static std::string generateRecommendation(const DiagnosticResult& result);
    static void generateDetailedReport(DiagnosticResult& result, 
                                     const RooFitResult* fitResult,
                                     RooAbsPdf* pdf,
                                     RooDataSet* data);
    
private:
    static double calculateQualityScore(const DiagnosticResult& result);
    static std::vector<std::string> findProblematicParameters(const RooFitResult* result);
    static std::vector<std::string> generateWarnings(const RooFitResult* result, 
                                                    RooAbsPdf* pdf, 
                                                    RooDataSet* data);
    static double calculateAIC(const RooFitResult* result, int nParams);
    static double calculateBIC(const RooFitResult* result, int nParams, int nData);
};

// 스마트 매개변수 초기화 클래스
class SmartParameterInitializer {
public:
    // 데이터 기반 스마트 초기화
    static void smartInitializeGaussianParameters(RobustParameterManager& manager,
                                                 const std::string& name,
                                                 RooDataSet* data,
                                                 const std::string& varName);
    
    static void smartInitializeCrystalBallParameters(RobustParameterManager& manager,
                                                   const std::string& name,
                                                   RooDataSet* data,
                                                   const std::string& varName);
    
    static void smartInitializePolynomialParameters(RobustParameterManager& manager,
                                                  const std::string& name,
                                                  int order,
                                                  RooDataSet* data,
                                                  const std::string& varName);
    
    // 고급 데이터 분석 기반 초기화
    static void estimateFromDataAdvanced(RooDataSet* data,
                                       const std::string& varName,
                                       double& mean,
                                       double& sigma,
                                       double& skewness,
                                       double& kurtosis,
                                       double& mode);
    
    // 입자별 물리학적 제약 적용
    static void applyParticleSpecificConstraints(RobustParameterManager& manager,
                                               PDFType pdfType,
                                               const std::string& particleName);
    
    // 적응적 초기값 최적화
    static void optimizeInitialValuesIteratively(RobustParameterManager& manager,
                                                RooAbsPdf* pdf,
                                                RooDataSet* data,
                                                int maxIterations = 10);
    
private:
    static std::pair<double, double> findOptimalRange(RooDataSet* data, 
                                                     const std::string& varName,
                                                     double nsigma = 3.0);
    static double estimateBackgroundLevel(RooDataSet* data, 
                                        const std::string& varName,
                                        double signalRegionMin,
                                        double signalRegionMax);
};

// ============================
// Inline implementations moved
// ============================

// RobustParameterManager 구현
inline RobustParameterManager::RobustParameterManager() {
    setupDefaultParameters();
    FittingLogger::info("RobustParameterManager initialized with default parameters");
}

inline void RobustParameterManager::setupDefaultParameters() {
    // PDF 타입별 매개변수 매핑 설정
    pdfParameterMap_[PDFType::GAUSSIAN] = {ParameterType::MEAN, ParameterType::SIGMA};
    pdfParameterMap_[PDFType::DOUBLE_GAUSSIAN] = {ParameterType::MEAN, ParameterType::SIGMA1, ParameterType::SIGMA2, ParameterType::FRACTION};
    pdfParameterMap_[PDFType::CRYSTAL_BALL] = {ParameterType::MEAN, ParameterType::SIGMA, ParameterType::ALPHA, ParameterType::N};
    pdfParameterMap_[PDFType::DB_CRYSTAL_BALL] = {ParameterType::MEAN, ParameterType::SIGMA, ParameterType::ALPHA_L, ParameterType::ALPHA_R, ParameterType::N_L, ParameterType::N_R};
    pdfParameterMap_[PDFType::VOIGTIAN] = {ParameterType::MEAN, ParameterType::SIGMA};
    pdfParameterMap_[PDFType::EXPONENTIAL] = {ParameterType::LAMBDA};
    pdfParameterMap_[PDFType::CHEBYCHEV] = {ParameterType::P0, ParameterType::P1, ParameterType::P2};
    pdfParameterMap_[PDFType::POLYNOMIAL] = {ParameterType::P0, ParameterType::P1, ParameterType::P2};
}

inline ParameterType RobustParameterManager::extractParameterType(const std::string& name) const {
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    if (lowerName.find("mean") != std::string::npos) return ParameterType::MEAN;
    if (lowerName.find("sigma1") != std::string::npos) return ParameterType::SIGMA1;
    if (lowerName.find("sigma2") != std::string::npos) return ParameterType::SIGMA2;
    if (lowerName.find("sigma") != std::string::npos) return ParameterType::SIGMA;
    if (lowerName.find("alpha_l") != std::string::npos) return ParameterType::ALPHA_L;
    if (lowerName.find("alpha_r") != std::string::npos) return ParameterType::ALPHA_R;
    if (lowerName.find("alpha") != std::string::npos) return ParameterType::ALPHA;
    if (lowerName.find("n_l") != std::string::npos) return ParameterType::N_L;
    if (lowerName.find("n_r") != std::string::npos) return ParameterType::N_R;
    if (lowerName.find("n_") != std::string::npos || lowerName == "n") return ParameterType::N;
    if (lowerName.find("frac") != std::string::npos) return ParameterType::FRACTION;
    if (lowerName.find("lambda") != std::string::npos) return ParameterType::LAMBDA;
    if (lowerName.find("p0") != std::string::npos) return ParameterType::P0;
    if (lowerName.find("p1") != std::string::npos) return ParameterType::P1;
    if (lowerName.find("p2") != std::string::npos) return ParameterType::P2;
    
    return ParameterType::UNKNOWN;
}

inline RooRealVar* RobustParameterManager::createParameter(const std::string& name, 
                                                   ParameterType type,
                                                   double value, 
                                                   double min, 
                                                   double max,
                                                   const std::string& description) {
    
    if (hasParameter(name)) {
        FittingLogger::warning("Parameter " + name + " already exists, returning existing parameter");
        return getParameter(name);
    }
    
    // 매개변수 정보 저장
    ParameterInfo info(type, name, value, min, max, false, 1e-6, description);
    parameterInfo_[name] = info;
    
    // RooRealVar 생성
    auto parameter = std::make_unique<RooRealVar>(name.c_str(), description.c_str(), value, min, max);
    RooRealVar* paramPtr = parameter.get();
    
    // 소유권 이전
    parameters_[name] = std::move(parameter);
    
    logParameterChange(name, "created", 0, value);
    
    return paramPtr;
}

inline RooRealVar* RobustParameterManager::getParameter(const std::string& name) const {
    auto it = parameters_.find(name);
    if (it != parameters_.end()) {
        return it->second.get();
    }
    return nullptr;
}

inline bool RobustParameterManager::hasParameter(const std::string& name) const {
    return parameters_.find(name) != parameters_.end();
}

inline void RobustParameterManager::setParameterValue(const std::string& name, double value) {
    auto* param = getParameter(name);
    if (!param) {
        throw std::invalid_argument("Parameter " + name + " not found");
    }
    
    double oldValue = param->getVal();
    param->setVal(value);
    
    // 정보 업데이트
    if (parameterInfo_.find(name) != parameterInfo_.end()) {
        parameterInfo_[name].lastModified = std::chrono::steady_clock::now();
    }
    
    logParameterChange(name, "value_changed", oldValue, value);
}

inline void RobustParameterManager::setParameterRange(const std::string& name, double min, double max) {
    auto* param = getParameter(name);
    if (!param) {
        throw std::invalid_argument("Parameter " + name + " not found");
    }
    
    param->setRange(min, max);
    
    // 정보 업데이트
    if (parameterInfo_.find(name) != parameterInfo_.end()) {
        parameterInfo_[name].minValue = min;
        parameterInfo_[name].maxValue = max;
        parameterInfo_[name].lastModified = std::chrono::steady_clock::now();
    }
    
    logParameterChange(name, "range_changed", min, max);
}

inline void RobustParameterManager::setParameterConstant(const std::string& name, bool constant) {
    auto* param = getParameter(name);
    if (!param) {
        throw std::invalid_argument("Parameter " + name + " not found");
    }
    
    param->setConstant(constant);
    
    // 정보 업데이트
    if (parameterInfo_.find(name) != parameterInfo_.end()) {
        parameterInfo_[name].isConstant = constant;
        parameterInfo_[name].lastModified = std::chrono::steady_clock::now();
    }
    
    logParameterChange(name, constant ? "fixed" : "released", 0, 0);
}

inline void RobustParameterManager::setParameterError(const std::string& name, double error) {
    auto* param = getParameter(name);
    if (!param) {
        throw std::invalid_argument("Parameter " + name + " not found");
    }
    
    param->setError(error);
    logParameterChange(name, "error_set", 0, error);
}

inline ParameterInfo RobustParameterManager::getParameterInfo(const std::string& name) const {
    auto it = parameterInfo_.find(name);
    if (it != parameterInfo_.end()) {
        return it->second;
    }
    
    // 기본값 반환
    return ParameterInfo();
}

inline std::vector<std::string> RobustParameterManager::getParameterNames() const {
    std::vector<std::string> names;
    for (const auto& [name, param] : parameters_) {
        names.push_back(name);
    }
    return names;
}

inline std::vector<std::string> RobustParameterManager::getParametersByType(ParameterType type) const {
    std::vector<std::string> names;
    for (const auto& [name, info] : parameterInfo_) {
        if (info.type == type) {
            names.push_back(name);
        }
    }
    return names;
}

inline bool RobustParameterManager::validateParameter(const std::string& name) const {
    auto* param = getParameter(name);
    if (!param) return false;
    
    double value = param->getVal();
    double min = param->getMin();
    double max = param->getMax();
    double error = param->getError();
    
    // 값이 범위 내에 있는지 확인
    if (value < min || value > max) return false;
    
    // 오류가 양수인지 확인
    if (error <= 0 || !std::isfinite(error)) return false;
    
    // 값이 finite인지 확인
    if (!std::isfinite(value)) return false;
    
    return true;
}

inline bool RobustParameterManager::validateAllParameters() const {
    for (const auto& name : getParameterNames()) {
        if (!validateParameter(name)) {
            return false;
        }
    }
    return true;
}

inline std::vector<std::string> RobustParameterManager::getInvalidParameters() const {
    std::vector<std::string> invalid;
    for (const auto& name : getParameterNames()) {
        if (!validateParameter(name)) {
            invalid.push_back(name);
        }
    }
    return invalid;
}

inline void RobustParameterManager::smartAdjustParameterRanges(const std::vector<std::string>& paramNames, 
                                                       double expandFactor) {
    for (const std::string& name : paramNames) {
        auto* param = getParameter(name);
        if (!param) continue;
        
        double currentMin = param->getMin();
        double currentMax = param->getMax();
        double currentValue = param->getVal();
        double range = currentMax - currentMin;
        
        // 물리학적 제약 고려
        ParameterType type = extractParameterType(name);
        double newMin = currentMin;
        double newMax = currentMax;
        
        switch (type) {
            case ParameterType::SIGMA:
            case ParameterType::SIGMA1:
            case ParameterType::SIGMA2:
                // 시그마는 항상 양수여야 함
                newMin = std::max(0.001, currentMin);
                newMax = currentMax * expandFactor;
                break;
            case ParameterType::FRACTION:
                // 분율은 0과 1 사이
                newMin = 0.0;
                newMax = 1.0;
                break;
            case ParameterType::N:
            case ParameterType::N_L:
            case ParameterType::N_R:
                // Crystal Ball n 매개변수는 양수
                newMin = std::max(0.1, currentMin);
                newMax = currentMax * expandFactor;
                break;
            default:
                // 일반적인 경우 대칭적으로 확장
                double center = (currentMin + currentMax) / 2.0;
                double halfRange = range * expandFactor / 2.0;
                newMin = center - halfRange;
                newMax = center + halfRange;
                break;
        }
        
        setParameterRange(name, newMin, newMax);
        
        FittingLogger::debug("Adjusted range for " + name + ": [" + 
                           std::to_string(newMin) + ", " + std::to_string(newMax) + "]");
    }
}

inline void RobustParameterManager::resetParameterRanges(const std::vector<std::string>& paramNames) {
    for (const std::string& name : paramNames) {
        auto it = parameterInfo_.find(name);
        if (it != parameterInfo_.end()) {
            setParameterRange(name, it->second.minValue, it->second.maxValue);
        }
    }
}

inline void RobustParameterManager::saveParameterState(const std::string& stateName) {
    std::map<std::string, double> values;
    std::map<std::string, std::pair<double, double>> ranges;
    
    for (const auto& [name, param] : parameters_) {
        values[name] = param->getVal();
        ranges[name] = {param->getMin(), param->getMax()};
    }
    
    savedStates_[stateName] = values;
    savedRanges_[stateName] = ranges;
    
    FittingLogger::debug("Parameter state '" + stateName + "' saved");
}

inline void RobustParameterManager::restoreParameterState(const std::string& stateName) {
    auto stateIt = savedStates_.find(stateName);
    auto rangeIt = savedRanges_.find(stateName);
    
    if (stateIt == savedStates_.end()) {
        FittingLogger::warning("Parameter state '" + stateName + "' not found");
        return;
    }
    
    for (const auto& [name, value] : stateIt->second) {
        if (hasParameter(name)) {
            setParameterValue(name, value);
        }
    }
    
    if (rangeIt != savedRanges_.end()) {
        for (const auto& [name, range] : rangeIt->second) {
            if (hasParameter(name)) {
                setParameterRange(name, range.first, range.second);
            }
        }
    }
    
    FittingLogger::debug("Parameter state '" + stateName + "' restored");
}

inline void RobustParameterManager::clearParameterState(const std::string& stateName) {
    savedStates_.erase(stateName);
    savedRanges_.erase(stateName);
    FittingLogger::debug("Parameter state '" + stateName + "' cleared");
}

inline std::vector<std::string> RobustParameterManager::listSavedStates() const {
    std::vector<std::string> states;
    for (const auto& [name, values] : savedStates_) {
        states.push_back(name);
    }
    return states;
}

inline void RobustParameterManager::printParameterSummary() const {
    std::cout << "\n========== PARAMETER SUMMARY ==========" << std::endl;
    std::cout << "Total parameters: " << parameters_.size() << std::endl;
    
    for (const auto& [name, param] : parameters_) {
        auto info = getParameterInfo(name);
        std::cout << name << ": " 
                  << param->getVal() << " ± " << param->getError()
                  << " [" << param->getMin() << ", " << param->getMax() << "]";
        
        if (param->isConstant()) {
            std::cout << " (CONSTANT)";
        }
        
        std::cout << std::endl;
    }
    std::cout << "=======================================" << std::endl;
}

inline std::map<std::string, double> RobustParameterManager::getParameterValues() const {
    std::map<std::string, double> values;
    for (const auto& [name, param] : parameters_) {
        values[name] = param->getVal();
    }
    return values;
}

inline std::map<std::string, double> RobustParameterManager::getParameterErrors() const {
    std::map<std::string, double> errors;
    for (const auto& [name, param] : parameters_) {
        errors[name] = param->getError();
    }
    return errors;
}

inline void RobustParameterManager::applyPhysicsConstraints() {
    // 물리학적 제약 적용
    for (const auto& [name, param] : parameters_) {
        ParameterType type = extractParameterType(name);
        
        switch (type) {
            case ParameterType::SIGMA:
            case ParameterType::SIGMA1:
            case ParameterType::SIGMA2:
                // 시그마는 항상 양수
                if (param->getVal() <= 0) {
                    param->setVal(0.001);
                }
                param->setMin(0.0001);
                break;
                
            case ParameterType::FRACTION:
                // 분율은 0과 1 사이
                param->setRange(0.0, 1.0);
                if (param->getVal() < 0 || param->getVal() > 1) {
                    param->setVal(0.5);
                }
                break;
                
            case ParameterType::N:
            case ParameterType::N_L:
            case ParameterType::N_R:
                // Crystal Ball n 매개변수는 양수
                if (param->getVal() <= 0) {
                    param->setVal(1.0);
                }
                param->setMin(0.1);
                break;
                
            default:
                break;
        }
    }
}

inline void RobustParameterManager::enableParameterTracking(bool enable) {
    trackingEnabled_ = enable;
    if (enable) {
        FittingLogger::info("Parameter tracking enabled");
    } else {
        FittingLogger::info("Parameter tracking disabled");
        modificationLog_.clear();
    }
}

inline void RobustParameterManager::logParameterChange(const std::string& name, const std::string& action, 
                                               double oldValue, double newValue) const {
    if (trackingEnabled_) {
        std::string logEntry = "Parameter " + name + " " + action;
        if (action == "value_changed") {
            logEntry += ": " + std::to_string(oldValue) + " -> " + std::to_string(newValue);
        } else if (action == "range_changed") {
            logEntry += ": [" + std::to_string(oldValue) + ", " + std::to_string(newValue) + "]";
        }
        
        modificationLog_.push_back(logEntry);
        FittingLogger::debug(logEntry);
    }
}

inline std::vector<std::pair<std::string, std::chrono::steady_clock::time_point>> 
RobustParameterManager::getRecentlyModifiedParameters(int minutes) const {
    
    std::vector<std::pair<std::string, std::chrono::steady_clock::time_point>> recent;
    auto cutoff = std::chrono::steady_clock::now() - std::chrono::minutes(minutes);
    
    for (const auto& [name, info] : parameterInfo_) {
        if (info.lastModified >= cutoff) {
            recent.push_back({name, info.lastModified});
        }
    }
    
    return recent;
}

// ============================
// SmartFitStrategyManager 구현
// ============================
inline SmartFitStrategyManager::SmartFitStrategyManager() : currentStrategy_(FitStrategy::STANDARD) {
    // 기본 fallback 전략 설정
    fallbackStrategies_ = {
        FitStrategy::STANDARD,
        FitStrategy::ROBUST,
        FitStrategy::CONSTRAINED,
        FitStrategy::SEQUENTIAL
    };
    
    // 전략별 통계 초기화
    for (auto strategy : {FitStrategy::STANDARD, FitStrategy::ROBUST, 
                         FitStrategy::CONSTRAINED, FitStrategy::SEQUENTIAL, 
                         FitStrategy::ADAPTIVE}) {
        strategyStats_[strategy] = {0, 0}; // {성공, 총시도}
    }
    
    FittingLogger::info("SmartFitStrategyManager initialized");
}

inline void SmartFitStrategyManager::setStrategy(FitStrategy strategy) {
    currentStrategy_ = strategy;
    FittingLogger::info("Fit strategy set to: " + std::to_string(static_cast<int>(strategy)));
}

inline FitStrategy SmartFitStrategyManager::getCurrentStrategy() const {
    return currentStrategy_;
}

inline void SmartFitStrategyManager::setFallbackStrategies(const std::vector<FitStrategy>& strategies) {
    fallbackStrategies_ = strategies;
    FittingLogger::info("Fallback strategies updated");
}

inline std::unique_ptr<RooFitResult> SmartFitStrategyManager::executeFit(
    RooAbsPdf* pdf, 
    RooDataSet* data,
    RobustParameterManager* paramManager,
    const std::map<std::string, bool>& options) {
    
    if (!pdf || !data) {
        FittingLogger::error("Invalid PDF or data pointer");
        return nullptr;
    }
    
    ScopedTimer timer("FitExecution");
    
    std::unique_ptr<RooFitResult> result;
    
    try {
        switch (currentStrategy_) {
            case FitStrategy::STANDARD:
                result = executeStandardFit(pdf, data);
                break;
            case FitStrategy::ROBUST:
                result = executeRobustFit(pdf, data, paramManager);
                break;
            case FitStrategy::CONSTRAINED:
                result = executeConstrainedFit(pdf, data);
                break;
            case FitStrategy::SEQUENTIAL:
                result = executeSequentialFit(pdf, data, paramManager);
                break;
            case FitStrategy::ADAPTIVE:
                result = executeAdaptiveFit(pdf, data, paramManager);
                break;
            default:
                FittingLogger::warning("Unknown strategy, falling back to STANDARD");
                result = executeStandardFit(pdf, data);
                break;
        }
        
        bool success = result && (result->status() == 0);
        updateStrategyStats(currentStrategy_, success);
        
        if (success) {
            FittingLogger::info("Fit completed successfully with strategy: " + 
                              std::to_string(static_cast<int>(currentStrategy_)));
        } else {
            FittingLogger::warning("Fit failed with strategy: " + 
                                 std::to_string(static_cast<int>(currentStrategy_)));
        }
        
    } catch (const std::exception& e) {
        FittingLogger::error("Exception during fit execution: " + std::string(e.what()));
        updateStrategyStats(currentStrategy_, false);
        return nullptr;
    }
    
    return result;
}

inline std::unique_ptr<RooFitResult> SmartFitStrategyManager::adaptiveFit(
    RooAbsPdf* pdf, 
    RooDataSet* data,
    RobustParameterManager* paramManager) {
    
    FittingLogger::info("Starting adaptive fit");
    
    // 성공률 기반으로 전략 정렬
    std::vector<FitStrategy> sortedStrategies = fallbackStrategies_;
    std::sort(sortedStrategies.begin(), sortedStrategies.end(),
        [this](FitStrategy a, FitStrategy b) {
            auto statsA = strategyStats_[a];
            auto statsB = strategyStats_[b];
            double successRateA = (statsA.second > 0) ? 
                static_cast<double>(statsA.first) / statsA.second : 0.5;
            double successRateB = (statsB.second > 0) ? 
                static_cast<double>(statsB.first) / statsB.second : 0.5;
            return successRateA > successRateB;
        });
    
    for (auto strategy : sortedStrategies) {
        FittingLogger::info("Trying strategy: " + std::to_string(static_cast<int>(strategy)));
        
        // 매개변수 상태 저장
        if (paramManager) {
            paramManager->saveParameterState("adaptive_backup");
        }
        
        setStrategy(strategy);
        auto result = executeFit(pdf, data, paramManager);
        
        if (result && isFitAcceptable(result.get(), pdf, data)) {
            FittingLogger::info("Adaptive fit succeeded with strategy: " + 
                              std::to_string(static_cast<int>(strategy)));
            return result;
        }
        
        // 실패 시 매개변수 복원
        if (paramManager) {
            paramManager->restoreParameterState("adaptive_backup");
        }
        
        FittingLogger::warning("Strategy failed, trying next: " + 
                             std::to_string(static_cast<int>(strategy)));
    }
    
    FittingLogger::error("All adaptive fit strategies failed");
    return nullptr;
}

inline std::unique_ptr<RooFitResult> SmartFitStrategyManager::executeStandardFit(
    RooAbsPdf* pdf, RooDataSet* data) {
    
    FittingLogger::debug("Executing standard fit");
    
    RooLinkedList fitOpts;
    fitOpts.Add(new RooCmdArg(RooFit::NumCPU(DEFAULT_CPU_COUNT)));
    fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(-1)));
    fitOpts.Add(new RooCmdArg(RooFit::Save()));
    fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit2", "migrad")));
    fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Optimize(1)));
    fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
    
    auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
    
    // 기본 재시도 로직
    if (result && result->status() != 0) {
        FittingLogger::warning("Standard fit failed, retrying with different settings");
        
        RooLinkedList retryOpts;
        retryOpts.Add(new RooCmdArg(RooFit::NumCPU(DEFAULT_CPU_COUNT)));
        retryOpts.Add(new RooCmdArg(RooFit::PrintLevel(1)));
        retryOpts.Add(new RooCmdArg(RooFit::Save()));
        retryOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit2", "migrad")));
        retryOpts.Add(new RooCmdArg(RooFit::Extended(true)));
        retryOpts.Add(new RooCmdArg(RooFit::Strategy(2)));
        retryOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
        retryOpts.Add(new RooCmdArg(RooFit::Minos(false)));
        
        result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, retryOpts));
    }
    
    return result;
}

inline std::unique_ptr<RooFitResult> SmartFitStrategyManager::executeRobustFit(
    RooAbsPdf* pdf, RooDataSet* data, RobustParameterManager* paramManager) {
    
    FittingLogger::debug("Executing robust fit");
    
    const int maxRetries = 5;
    std::unique_ptr<RooFitResult> bestResult = nullptr;
    double bestQuality = -1.0;
    
    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        FittingLogger::debug("Robust fit attempt: " + std::to_string(attempt + 1));
        
        if (attempt > 0 && paramManager) {
            // 매개변수 범위를 점진적으로 확장
            auto paramNames = paramManager->getParameterNames();
            paramManager->smartAdjustParameterRanges(paramNames, 1.2 + 0.3 * attempt);
        }
        
        RooLinkedList fitOpts;
        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(DEFAULT_CPU_COUNT)));
        fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(-1)));
        fitOpts.Add(new RooCmdArg(RooFit::Save()));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit2", "migrad")));
        fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
        fitOpts.Add(new RooCmdArg(RooFit::Strategy(std::min(2, attempt + 1))));
        fitOpts.Add(new RooCmdArg(RooFit::Optimize(1)));
        
        if (attempt >= 2) {
            fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
        }
        if (attempt >= 3) {
            fitOpts.Add(new RooCmdArg(RooFit::Minos(true)));
        }
        
        auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
        
        if (result) {
            double quality = evaluateFitQuality(result.get(), pdf, data);
            
            if (quality > bestQuality) {
                bestQuality = quality;
                bestResult = std::move(result);
            }
            
            // 충분히 좋은 결과면 조기 종료
            if (bestQuality > 0.8 && bestResult->status() == 0) {
                FittingLogger::info("Robust fit converged early at attempt: " + 
                                  std::to_string(attempt + 1));
                break;
            }
        }
    }
    
    if (bestResult) {
        FittingLogger::info("Robust fit completed with quality: " + 
                          std::to_string(bestQuality));
    } else {
        FittingLogger::warning("Robust fit failed after all attempts");
    }
    
    return bestResult;
}

inline std::unique_ptr<RooFitResult> SmartFitStrategyManager::executeConstrainedFit(
    RooAbsPdf* pdf, RooDataSet* data) {
    
    FittingLogger::debug("Executing constrained fit");
    
    // 제약 조건이 있는 피팅 구현
    RooLinkedList fitOpts;
    fitOpts.Add(new RooCmdArg(RooFit::NumCPU(DEFAULT_CPU_COUNT)));
    fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(0)));
    fitOpts.Add(new RooCmdArg(RooFit::Save()));
    fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit2", "migrad")));
    fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Strategy(1)));
    fitOpts.Add(new RooCmdArg(RooFit::Optimize(1)));
    fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
    
    // Note: RooFit::Eps not available in ROOT 6.24, using default tolerance
    
    return std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
}

inline std::unique_ptr<RooFitResult> SmartFitStrategyManager::executeSequentialFit(
    RooAbsPdf* pdf, RooDataSet* data, RobustParameterManager* paramManager) {
    
    FittingLogger::debug("Executing sequential fit");
    
    if (!paramManager) {
        FittingLogger::warning("No parameter manager for sequential fit, falling back to standard");
        return executeStandardFit(pdf, data);
    }
    
    // 1단계: 주요 매개변수만 자유롭게 두고 피팅
    paramManager->saveParameterState("sequential_backup");
    
    // 모든 매개변수를 고정
    auto allParams = paramManager->getParameterNames();
    for (const auto& name : allParams) {
        if (name.find("mean") != std::string::npos || 
            name.find("sigma") != std::string::npos) {
            // 주요 매개변수는 자유롭게 둠
            continue;
        }
        paramManager->setParameterConstant(name, true);
    }
    
    // 1단계 피팅
    auto phase1Result = executeStandardFit(pdf, data);
    
    if (!phase1Result || phase1Result->status() != 0) {
        FittingLogger::warning("Sequential fit phase 1 failed");
        paramManager->restoreParameterState("sequential_backup");
        return nullptr;
    }
    
    // 2단계: 모든 매개변수 해제 후 피팅
    for (const auto& name : allParams) {
        paramManager->setParameterConstant(name, false);
    }
    
    auto phase2Result = executeStandardFit(pdf, data);
    
    if (phase2Result && phase2Result->status() == 0) {
        FittingLogger::info("Sequential fit completed successfully");
        return phase2Result;
    } else {
        FittingLogger::warning("Sequential fit phase 2 failed, using phase 1 result");
        return phase1Result;
    }
}

inline std::unique_ptr<RooFitResult> SmartFitStrategyManager::executeAdaptiveFit(
    RooAbsPdf* pdf, RooDataSet* data, RobustParameterManager* paramManager) {
    
    return adaptiveFit(pdf, data, paramManager);
}

inline double SmartFitStrategyManager::evaluateFitQuality(
    const RooFitResult* result, RooAbsPdf* pdf, RooDataSet* data) const {
    
    if (!result || !pdf || !data) return 0.0;
    
    double quality = 1.0;
    
    // 수렴 상태 확인
    if (result->status() != 0) {
        quality *= 0.3; // 수렴하지 않으면 품질 크게 감소
    }
    
    // 공분산 행렬 유효성
    if (result->covQual() < 2) {
        quality *= 0.7;
    }
    
    // 매개변수가 경계에 있는지 확인
    const RooArgList& finalParams = result->floatParsFinal();
    int parametersAtLimits = 0;
    for (int i = 0; i < finalParams.getSize(); ++i) {
        auto* var = dynamic_cast<const RooRealVar*>(&finalParams[i]);
        if (var) {
            double val = var->getVal();
            double min = var->getMin();
            double max = var->getMax();
            double range = max - min;
            
            if (std::abs(val - min) < 0.01 * range || 
                std::abs(val - max) < 0.01 * range) {
                parametersAtLimits++;
            }
        }
    }
    
    if (parametersAtLimits > 0) {
        quality *= std::pow(0.9, parametersAtLimits);
    }
    
    // 로그 우도 기반 품질 평가
    double logL = result->minNll();
    if (std::isfinite(logL) && logL < 0) {
        // 정상적인 로그 우도 범위에서 추가 점수
        quality *= 1.0;
    } else {
        quality *= 0.5;
    }
    
    return std::max(0.0, std::min(1.0, quality));
}

inline bool SmartFitStrategyManager::isFitAcceptable(
    const RooFitResult* result, RooAbsPdf* pdf, RooDataSet* data,
    double qualityThreshold) const {
    
    return evaluateFitQuality(result, pdf, data) >= qualityThreshold;
}

inline void SmartFitStrategyManager::updateStrategyStats(FitStrategy strategy, bool success) {
    strategyStats_[strategy].second++; // 총 시도 수 증가
    if (success) {
        strategyStats_[strategy].first++; // 성공 수 증가
    }
}

inline void SmartFitStrategyManager::printStrategyStatistics() const {
    std::cout << "\n=== Fit Strategy Statistics ===" << std::endl;
    for (const auto& [strategy, stats] : strategyStats_) {
        if (stats.second > 0) {
            double successRate = static_cast<double>(stats.first) / stats.second;
            std::cout << "Strategy " << static_cast<int>(strategy) 
                      << ": " << stats.first << "/" << stats.second 
                      << " (" << std::fixed << std::setprecision(1) 
                      << successRate * 100 << "%)" << std::endl;
        }
    }
    std::cout << "==============================\n" << std::endl;
}

inline FitStrategy SmartFitStrategyManager::recommendStrategy() const {
    FitStrategy best = FitStrategy::STANDARD;
    double bestRate = 0.0;
    
    for (const auto& [strategy, stats] : strategyStats_) {
        if (stats.second >= 3) { // 최소 3번 시도한 전략만 고려
            double rate = static_cast<double>(stats.first) / stats.second;
            if (rate > bestRate) {
                bestRate = rate;
                best = strategy;
            }
        }
    }
    
    return best;
}

// Fallback for DEFAULT_CPU_COUNT if not defined elsewhere
#ifndef DEFAULT_CPU_COUNT
#define DEFAULT_CPU_COUNT 4
#endif

#endif // ROBUST_PARAMETER_MANAGER_H