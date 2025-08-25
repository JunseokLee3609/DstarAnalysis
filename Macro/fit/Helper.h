#ifndef HELPER_H_H
#define HELPER_H_H

#include <map>
#include <utility>
#include "Params.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
// #include <filesystem> // C++17 디렉토리 처리를 위한 헤더

// 로깅 레벨 정의
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

// 간단한 로거 클래스
class FittingLogger {
private:
    static LogLevel current_level_;
    static std::unique_ptr<std::ofstream> log_file_;
    static std::string getCurrentTimeString();
    
public:
    static void setLogLevel(LogLevel level);
    static void setLogFile(const std::string& filename);
    static void log(LogLevel level, const std::string& message);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void logFitResult(const RooFitResult* result, const std::string& context);
};

// Bin identification structure
struct BinInfo {
    double pTMin, pTMax;
    double cosMin, cosMax;
    double dcaMin, dcaMax;
    
    BinInfo(double ptMin = -999, double ptMax = -999, 
            double cosThMin = -999, double cosThMax = -999,
            double dcaRangeMin = -999, double dcaRangeMax = -999) 
        : pTMin(ptMin), pTMax(ptMax), cosMin(cosThMin), cosMax(cosThMax), 
          dcaMin(dcaRangeMin), dcaMax(dcaRangeMax) {}
    
    // Comparison operator for map key
    bool operator<(const BinInfo& other) const {
        if (pTMin != other.pTMin) return pTMin < other.pTMin;
        if (pTMax != other.pTMax) return pTMax < other.pTMax;
        if (cosMin != other.cosMin) return cosMin < other.cosMin;
        if (cosMax != other.cosMax) return cosMax < other.cosMax;
        if (dcaMin != other.dcaMin) return dcaMin < other.dcaMin;
        return dcaMax < other.dcaMax;
    }
    
    // String representation
    std::string toString() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        if (pTMin != -999 && pTMax != -999) {
            ss << "pT[" << pTMin << "-" << pTMax << "]";
        }
        if (cosMin != -999 && cosMax != -999) {
            ss << "_cos[" << cosMin << "-" << cosMax << "]";
        }
        if (dcaMin != -999 && dcaMax != -999) {
            ss << "_dca[" << dcaMin << "-" << dcaMax << "]";
        }
        return ss.str();
    }
};

// Fit status information
struct FitStatus {
    int status;
    std::string fitType;  // "Mass", "DCA", "MC", "DCA_Slice", etc.
    std::string errorMessage;
    double chi2ndf;
    int ndf;
    std::string additionalInfo;
    
    FitStatus(int stat = -1, const std::string& type = "", 
              const std::string& error = "", double chi2 = -1, int ndof = -1,
              const std::string& info = "") 
        : status(stat), fitType(type), errorMessage(error), chi2ndf(chi2), 
          ndf(ndof), additionalInfo(info) {}
};

// Global fit status tracker
extern std::map<BinInfo, std::vector<FitStatus>> g_fitStatusMap;

// Function type for fit status callback
using FitStatusCallback = std::function<void(const BinInfo&, const FitStatus&)>;

// =============================================================================
// FIT STATUS TRACKING FUNCTIONS
// =============================================================================

// Function to initialize fit status tracking
void initializeFitStatusTracking();

// Function to add fit status
void addFitStatus(const BinInfo& binInfo, const FitStatus& fitStatus);

// Function to check and record fit result status
void checkAndRecordFitStatus(RooFitResult* fitResult, const BinInfo& binInfo, 
                            const std::string& fitType, const std::string& additionalInfo = "");

// Function to set global fit status callback
void setGlobalFitStatusCallback(const FitStatusCallback& callback);

// Function to print all failed fits
void printFailedFits();

// Function to save fit status to file
void saveFitStatusToFile(const std::string& filename);

// Function to get fit status summary
std::pair<int, int> getFitStatusSummary(); // returns (total_fits, failed_fits)

// =============================================================================
// PERFORMANCE MONITORING UTILITIES
// =============================================================================

// RAII 스타일 타이머 클래스
class ScopedTimer {
private:
    std::chrono::steady_clock::time_point start_time_;
    std::string operation_name_;
    
public:
    explicit ScopedTimer(const std::string& operation_name);
    ~ScopedTimer();
    double getElapsedSeconds() const;
};

// 성능 통계 수집 클래스
class PerformanceMonitor {
private:
    static std::map<std::string, std::vector<double>> operation_times_;
    
public:
    static void recordOperation(const std::string& operation, double time_seconds);
    static void printStatistics();
    static void saveStatistics(const std::string& filename);
    static void reset();
};

// =============================================================================
// DCA FITTER HELPER FUNCTIONS
// =============================================================================

// Structure to pass DCA slice information
struct DCASliceInfo {
    double dcaMin, dcaMax;
    int sliceIndex;
    std::string sliceName;
    
    DCASliceInfo(double dMin, double dMax, int idx, const std::string& name)
        : dcaMin(dMin), dcaMax(dMax), sliceIndex(idx), sliceName(name) {}
};

// Callback function type for DCA slice fits
using DCASliceFitCallback = std::function<void(const BinInfo&, const DCASliceInfo&, 
                                              RooFitResult*, const std::string&)>;

// Function to register DCA slice fit callback
void registerDCASliceFitCallback(const DCASliceFitCallback& callback);

// Function to notify DCA slice fit completion
void notifyDCASliceFit(const BinInfo& binInfo, const DCASliceInfo& sliceInfo, 
                      RooFitResult* fitResult, const std::string& fitType);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Function to create BinInfo from FitOpt
BinInfo createBinInfoFromFitOpt(const FitOpt& opt, double dcaMin = -999, double dcaMax = -999);

// Function to extract fit information from RooFitResult
FitStatus extractFitStatus(RooFitResult* fitResult, const std::string& fitType, 
                          const std::string& additionalInfo = "");

using ParamKey = std::pair<double, double>;
using D0ParamValue = std::pair<PDFParams::DoubleGaussianParams, PDFParams::PolynomialBkgParams>;
using D0MCParamValue = std::pair<PDFParams::DoubleGaussianParams, PDFParams::GaussianParams>;
// using DStarParamValue1 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::DstBkgParams>;
using DStarParamValue1 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarParamValue2 = std::pair<PDFParams::DoubleDBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarParamValue3 = std::pair<PDFParams::DoubleGaussianParams, PDFParams::PhenomenologicalParams>;
// using DStarParamValue3 = std::pair<PDFParams::DoubleGaussianParams, PDFParams::DstBkgParams>;
using DStarParamValue4 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PolynomialBkgParams>;
using DStarParamValue5 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::ExponentialBkgParams>;
using DStarMCParamValue = std::pair<PDFParams::DBCrystalBallParams, PDFParams::DBCrystalBallParams>;
using D0ParamMap = std::map<ParamKey, D0ParamValue>;
using D0MCParamMap = std::map<ParamKey, D0MCParamValue>;
using DStarParamMap1 = std::map<ParamKey, DStarParamValue1>;
using DStarParamMap2 = std::map<ParamKey, DStarParamValue2>;
using DStarParamMap3 = std::map<ParamKey, DStarParamValue3>;
using DStarParamMap4 = std::map<ParamKey, DStarParamValue4>;
using DStarParamMap5 = std::map<ParamKey, DStarParamValue5>;
using DStarMCParamMap = std::map<ParamKey, DStarMCParamValue>;
using DStarParamValue6 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::ExponentialBkgParams>;
using DStarParamMap6 = std::map<ParamKey, DStarParamValue6>;
using DStarParamValue7 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::ExpErfBkgParams>;
using DStarParamMap7 = std::map<ParamKey, DStarParamValue7>;

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

// Global fit status tracker
std::map<BinInfo, std::vector<FitStatus>> g_fitStatusMap;

// Global callback function
FitStatusCallback g_globalFitStatusCallback = nullptr;

// Global DCA slice fit callback
DCASliceFitCallback g_dcaSliceFitCallback = nullptr;

// =============================================================================
// FIT STATUS TRACKING FUNCTIONS
// =============================================================================

void initializeFitStatusTracking() {
    g_fitStatusMap.clear();
    std::cout << "Fit status tracking initialized." << std::endl;
}

void addFitStatus(const BinInfo& binInfo, const FitStatus& fitStatus) {
    g_fitStatusMap[binInfo].push_back(fitStatus);
    
    // Call global callback if registered
    if (g_globalFitStatusCallback) {
        g_globalFitStatusCallback(binInfo, fitStatus);
    }
}

void checkAndRecordFitStatus(RooFitResult* fitResult, const BinInfo& binInfo, 
                            const std::string& fitType, const std::string& additionalInfo) {
    FitStatus status = extractFitStatus(fitResult, fitType, additionalInfo);
    addFitStatus(binInfo, status);
    
    if (status.status != 0) {
        std::cout << "WARNING: " << fitType << " fit status = " << status.status 
                  << " for bin " << binInfo.toString();
        if (!additionalInfo.empty()) {
            std::cout << " (" << additionalInfo << ")";
        }
        std::cout << std::endl;
    } else {
        std::cout << "SUCCESS: " << fitType << " fit completed for bin " << binInfo.toString();
        if (status.chi2ndf > 0) {
            std::cout << " (χ²/ndf = " << status.chi2ndf << ")";
        }
        std::cout << std::endl;
    }
}

void setGlobalFitStatusCallback(const FitStatusCallback& callback) {
    g_globalFitStatusCallback = callback;
}

void printFailedFits() {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "FIT STATUS SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    int totalFits = 0;
    int failedFits = 0;
    
    for (const auto& binEntry : g_fitStatusMap) {
        const BinInfo& binInfo = binEntry.first;
        const std::vector<FitStatus>& statusList = binEntry.second;
        
        bool hasFailed = false;
        for (const auto& fitStatus : statusList) {
            totalFits++;
            if (fitStatus.status != 0) {
                failedFits++;
                hasFailed = true;
            }
        }
        
        if (hasFailed) {
            std::cout << "\nBIN: " << binInfo.toString() << std::endl;
            std::cout << std::string(50, '-') << std::endl;
            
            for (const auto& fitStatus : statusList) {
                if (fitStatus.status != 0) {
                    std::cout << "  " << fitStatus.fitType << " fit: STATUS = " << fitStatus.status;
                    if (!fitStatus.errorMessage.empty()) {
                        std::cout << " (" << fitStatus.errorMessage << ")";
                    }
                    if (fitStatus.chi2ndf > 0) {
                        std::cout << " χ²/ndf = " << std::setprecision(3) << fitStatus.chi2ndf;
                    }
                    if (!fitStatus.additionalInfo.empty()) {
                        std::cout << " [" << fitStatus.additionalInfo << "]";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "TOTAL FITS: " << totalFits << std::endl;
    std::cout << "FAILED FITS: " << failedFits << std::endl;
    std::cout << "SUCCESS RATE: " << std::setprecision(2) << std::fixed 
              << (totalFits > 0 ? (double)(totalFits - failedFits) / totalFits * 100.0 : 0.0) << "%" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

void saveFitStatusToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing" << std::endl;
        return;
    }
    
    outFile << "BinInfo,FitType,Status,ErrorMessage,Chi2NDF,NDF,AdditionalInfo" << std::endl;
    
    for (const auto& binEntry : g_fitStatusMap) {
        const BinInfo& binInfo = binEntry.first;
        const std::vector<FitStatus>& statusList = binEntry.second;
        
        for (const auto& fitStatus : statusList) {
            outFile << binInfo.toString() << ","
                    << fitStatus.fitType << ","
                    << fitStatus.status << ","
                    << fitStatus.errorMessage << ","
                    << fitStatus.chi2ndf << ","
                    << fitStatus.ndf << ","
                    << fitStatus.additionalInfo << std::endl;
        }
    }
    
    outFile.close();
    std::cout << "Fit status saved to: " << filename << std::endl;
}

std::pair<int, int> getFitStatusSummary() {
    int totalFits = 0;
    int failedFits = 0;
    
    for (const auto& binEntry : g_fitStatusMap) {
        const std::vector<FitStatus>& statusList = binEntry.second;
        for (const auto& fitStatus : statusList) {
            totalFits++;
            if (fitStatus.status != 0) {
                failedFits++;
            }
        }
    }
    
    return std::make_pair(totalFits, failedFits);
}

// =============================================================================
// DCA FITTER HELPER FUNCTIONS
// =============================================================================

void registerDCASliceFitCallback(const DCASliceFitCallback& callback) {
    g_dcaSliceFitCallback = callback;
}

void notifyDCASliceFit(const BinInfo& binInfo, const DCASliceInfo& sliceInfo, 
                      RooFitResult* fitResult, const std::string& fitType) {
    // Record the fit status
    BinInfo sliceBinInfo = binInfo;
    sliceBinInfo.dcaMin = sliceInfo.dcaMin;
    sliceBinInfo.dcaMax = sliceInfo.dcaMax;
    
    std::string additionalInfo = "DCA slice " + std::to_string(sliceInfo.sliceIndex) + 
                                " (" + sliceInfo.sliceName + ")";
    
    checkAndRecordFitStatus(fitResult, sliceBinInfo, fitType, additionalInfo);
    
    // Call registered callback
    if (g_dcaSliceFitCallback) {
        g_dcaSliceFitCallback(binInfo, sliceInfo, fitResult, fitType);
    }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

BinInfo createBinInfoFromFitOpt(const FitOpt& opt, double dcaMin, double dcaMax) {
    return BinInfo(opt.pTMin, opt.pTMax, opt.cosMin, opt.cosMax, dcaMin, dcaMax);
}

FitStatus extractFitStatus(RooFitResult* fitResult, const std::string& fitType, 
                          const std::string& additionalInfo) {
    if (!fitResult) {
        return FitStatus(-999, fitType, "Fit result is null", -1, -1, additionalInfo);
    }
    
    int status = fitResult->status();
    double chi2ndf = fitResult->numStatusHistory();
    int ndf = fitResult->numInvalidNLL();
    
    std::string errorMsg = "";
    if (status != 0) {
        errorMsg = "Fit status: " + std::to_string(status);
    }
    
    return FitStatus(status, fitType, errorMsg, chi2ndf, ndf, additionalInfo);
}

// =============================================================================
// EXISTING HELPER FUNCTIONS
// =============================================================================

TFile* createFileInDir(const std::string& dirPath, const std::string& filePath) {
    // 디렉토리 확인
    TSystemDirectory dir("dir", dirPath.c_str());
    void* dirCheck = gSystem->OpenDirectory(dirPath.c_str());

    if (!dirCheck) { // 폴더가 없으면 생성
        std::cout << "Directory does not exist. Creating: " << dirPath << std::endl;
        gSystem->MakeDirectory(dirPath.c_str());
    } else {
        std::cout << "Directory exists: " << dirPath << std::endl;
        gSystem->FreeDirectory(dirCheck);
    }

    // 새로운 TFile 생성
    TFile* file = TFile::Open((dirPath+filePath).c_str(), "RECREATE");
    if (file && file->IsOpen()) {
        std::cout << "Successfully created file: " << filePath << std::endl;
        // file->Close();
    } else {
        std::cerr << "Failed to create file: " << filePath << std::endl;
    }
    return file;
}

void createDir(const std::string& dirPath) {
    // 디렉토리 확인
    TSystemDirectory dir("dir", dirPath.c_str());
    void* dirCheck = gSystem->OpenDirectory(dirPath.c_str());

    if (!dirCheck) { // 폴더가 없으면 생성
        std::cout << "Directory does not exist. Creating: " << dirPath << std::endl;
        gSystem->MakeDirectory(dirPath.c_str());
    } else {
        std::cout << "Directory exists: " << dirPath << std::endl;
        gSystem->FreeDirectory(dirCheck);
    }
}

D0ParamMap D0ParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins, int order) {
    D0ParamMap params;
    

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
        D0ParamValue value;
        value.first.mean = 1.864;        // D* mass in GeV
        value.first.mean_min = 1.862;
        value.first.mean_max = 1.868;
        value.first.sigma1 = 0.001;
        value.first.sigma1_min = 0.001;
        value.first.sigma1_max = 0.02;
        value.first.sigma2 = 0.005;
        value.first.sigma2_min = 0.001;
        value.first.sigma2_max = 0.03;
        value.first.fraction = 0.5;
            double polyinit = 0.;
            double polymax = 1;
            double polymin = -1;
            for(int k =0; k < order; k++) {
                value.second.coefficients.push_back(polyinit);
                value.second.coef_max.push_back(polymax);
                value.second.coef_min.push_back(polymin);
            }
            params[key] = value;
        }
    }

    return params;
}
DStarParamMap1 DStarParamMaker1(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap1 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue1 value;
                value.first.mean = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                // value.first.sigmaR = 0.0005;
                // value.first.sigmaR_min = 0.0001;
                // value.first.sigmaR_max = 0.01;
                // value.first.sigmaL = 0.0005;
                // value.first.sigmaL_min = 0.0001;
                // value.first.sigmaL_max = 0.01;
                
                value.first.alphaL = 2.;
                value.first.alphaL_min = 0.01;
                value.first.alphaL_max = 10;
                value.first.nL = 1.5;
                value.first.nL_min = 1;
                value.first.nL_max = 8;

                value.first.sigma = 0.0005;
                value.first.sigma_min = 0.0001;
                value.first.sigma_max = 0.01;
            
                value.first.alphaR = 2.;
                value.first.alphaR_min = 0.01;
                value.first.alphaR_max = 10.;
                value.first.nR = 1.5;
                value.first.nR_min = 1;
                value.first.nR_max = 8;
                value.second.p0 = 0.1;
                value.second.p0_min = 0.0001;
                value.second.p0_max = 5.0;
                value.second.p1 = 2;
                value.second.p1_min = 0.0;
                value.second.p1_max = 10.0;
                value.second.p2 = -2;
                value.second.p2_min = -10.0;
                value.second.p2_max = 10.0;
            params[key] = value;
        }
    }

    return params;
}
DStarParamMap2 DStarParamMaker2(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap2 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue2 value;
                value.first.mean1 = 0.1455;        // D* mass in GeV
                value.first.mean2 = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                // value.first.sigma = 0.0005;
                // value.first.sigma_min = 0.0001;
                // value.first.sigma_max = 0.01;
                value.first.sigmaR1 = 0.0005;
                value.first.sigmaR2 = 0.0005;
                value.first.sigmaR_min = 0.0001;
                value.first.sigmaR_max = 0.01;
                value.first.sigmaL1 = 0.0005;
                value.first.sigmaL2 = 0.0005;
                value.first.sigmaL_min = 0.0001;
                value.first.sigmaL_max = 0.01;
                
                value.first.alphaL1 = 2.;
                value.first.alphaL2 = 2.;
                value.first.alphaL_min = 0.005;
                value.first.alphaL_max = 10;
                value.first.nL1 = 1.5;
                value.first.nL2 = 1.5;
                value.first.nL_min = 1;
                value.first.nL_max = 10;
            
                value.first.alphaR1 = 2.;
                value.first.alphaR2 = 2.;
                value.first.alphaR_min = 0.005;
                value.first.alphaR_max = 10.;
                value.first.nR1 = 1.5;
                value.first.nR2 = 1.5;
                value.first.nR_min = 1;
                value.first.nR_max = 20;
                value.second.p0 = 0.1;
                value.second.p0_min = 0.0;
                value.second.p0_max = 5.0;
                value.second.p1 = 2;
                value.second.p1_min = -10.0;
                value.second.p1_max = 10.0;
                value.second.p2 = -2;
                value.second.p2_min = -10.0;
                value.second.p2_max = 10.0;
            params[key] = value;
        }
    }

    return params;
}
DStarParamMap3 DStarParamMaker3(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap3 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue3 value;
                value.first.mean = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                value.first.sigma1 = 0.001;
                value.first.sigma1_min = 0.0001;
                value.first.sigma1_max = 0.002;
                value.first.sigma2 = 0.0005;
                value.first.sigma2_min = 0.0001;
                value.first.sigma2_max = 0.003;
                value.first.fraction = 0.5;
                value.first.fraction_min = 0.0;
                value.first.fraction_max = 1.0;

                value.second.p0 = 0.1;
                value.second.p0_min = 0.0;
                value.second.p0_max = 5.0;
                value.second.p1 = 2;
                value.second.p1_min = -100.0;
                value.second.p1_max = 100.0;
                value.second.p2 = -2;
                value.second.p2_min = -100.0;
                value.second.p2_max = 100.0;

                
            params[key] = value;
        }
    }

    return params;
}
DStarMCParamMap DStarMCParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarMCParamMap params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarMCParamValue value;
                value.first.mean = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                // value.first.sigmaR = 0.0005;
                // value.first.sigmaR_min = 0.0001;
                // value.first.sigmaR_max = 0.01;
                // value.first.sigmaL = 0.0005;
                // value.first.sigmaL_min = 0.0001;
                // value.first.sigmaL_max = 0.01;
                value.first.sigma = 0.0005;
                value.first.sigma_min = 0.0001;
                value.first.sigma_max = 0.01;
                
                value.first.alphaL = 2.;
                value.first.alphaL_min = 0.0001;
                value.first.alphaL_max = 1;
                value.first.nL = 1.5;
                value.first.nL_min = 1;
                value.first.nL_max = 10;
            
                value.first.alphaR = 2.;
                value.first.alphaR_min = 0.0001;
                value.first.alphaR_max = 10.;
                value.first.nR = 1.5;
                value.first.nR_min = 1;
                value.first.nR_max = 10;

                value.second.mean = 0.1455;        // D* mass in GeV
                value.second.mean_min = 0.1452;
                value.second.mean_max = 0.1458;
                // value.second.sigmaR = 0.0005;
                // value.second.sigmaR_min = 0.0001;
                // value.second.sigmaR_max = 0.01;
                // value.second.sigmaL = 0.0005;
                // value.second.sigmaL_min = 0.0001;
                // value.second.sigmaL_max = 0.01;
                
                value.second.alphaL = 2.;
                value.second.alphaL_min = 0.0001;
                value.second.alphaL_max = 10;
                value.second.nL = 1.5;
                value.second.nL_min = 1;
                value.second.nL_max = 10;
            
                value.second.alphaR = 2.;
                value.second.alphaR_min = 0.0001;
                value.second.alphaR_max = 10.;
                value.second.nR = 1.5;
                value.second.nR_min = 1;
                value.second.nR_max = 10;
                
            params[key] = value;
        }
    }

    return params;
}
D0MCParamMap D0MCParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    D0MCParamMap params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            D0MCParamValue value;
                value.first.mean = 1.864;        // D* mass in GeV
                value.first.mean_min = 1.862;
                value.first.mean_max = 1.868;
                value.first.sigma1 = 0.001;
                value.first.sigma1_min = 0.0001;
                value.first.sigma1_max = 0.02;
                value.first.sigma2 = 0.005;
                value.first.sigma2_min = 0.0001;
                value.first.sigma2_max = 0.03;

                value.second.mean = 1.864;        // D* mass in GeV
                value.second.mean_min = 1.855;
                value.second.mean_max = 1.875;
                value.second.sigma = 0.05;
                value.second.sigma_min = 0.001;
                value.second.sigma_max = 0.2;


                
            params[key] = value;
        }
    }

    return params;
}
D0ParamMap DStarParamMaker4(const std::vector<double>& ptBins, const std::vector<double>& etaBins, int order) {
    D0ParamMap params;
    

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
        D0ParamValue value;
        value.first.mean = 2.010;        // D* mass in GeV
        value.first.mean_min = 2.005;
        value.first.mean_max = 2.015;
        value.first.sigma1 = 0.001;
        value.first.sigma1_min = 0.001;
        value.first.sigma1_max = 0.02;
        value.first.sigma2 = 0.005;
        value.first.sigma2_min = 0.001;
        value.first.sigma2_max = 0.03;
        value.first.fraction = 0.5;
            double polyinit = 0.;
            double polymax = 1;
            double polymin = -1;
            for(int k =0; k < order; k++) {
                value.second.coefficients.push_back(polyinit);
                value.second.coef_max.push_back(polymax);
                value.second.coef_min.push_back(polymin);
            }
            params[key] = value;
        }
    }

    return params;
}

DStarParamMap4 DStarParamMaker5(const std::vector<double>& ptBins, const std::vector<double>& etaBins, int order) {
    DStarParamMap4 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue4 value;

            // DoubleDBCrystalBallParams 설정
                value.first.mean = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                // value.first.sigmaR = 0.0005;
                // value.first.sigmaR_min = 0.0001;
                // value.first.sigmaR_max = 0.01;
                // value.first.sigmaL = 0.0005;
                // value.first.sigmaL_min = 0.0001;
                // value.first.sigmaL_max = 0.01;
                
                value.first.alphaL = 2.;
                value.first.alphaL_min = 0.01;
                value.first.alphaL_max = 10;
                value.first.nL = 1.5;
                value.first.nL_min = 1;
                value.first.nL_max = 8;

                value.first.sigma = 0.0005;
                value.first.sigma_min = 0.0001;
                value.first.sigma_max = 0.01;
            
                value.first.alphaR = 2.;
                value.first.alphaR_min = 0.01;
                value.first.alphaR_max = 10.;
                value.first.nR = 1.5;
                value.first.nR_min = 1;
                value.first.nR_max = 8;

            // PolynomialBkgParams 설정
            double poly_init = 0.01;
            double poly_max = 1.0;
            double poly_min = -1.0;
            for (int k = 0; k < order; ++k) {
                value.second.coefficients.push_back(poly_init);
                value.second.coef_max.push_back(poly_max);
                value.second.coef_min.push_back(poly_min);
            }

            params[key] = value;
        }
    }

    return params;
}
DStarParamMap6 DStarParamMaker6(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap6 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue6 value;

            // DBCrystalBallParams 설정 (Signal)
            value.first.mean = 0.1455;        // D* mass in GeV
            value.first.mean_min = 0.1452;
            value.first.mean_max = 0.1458;
            
            value.first.sigma = 0.0005;
            value.first.sigma_min = 0.0001;
            value.first.sigma_max = 0.01;
            
            value.first.alphaL = 2.0;
            value.first.alphaL_min = 0.01;
            value.first.alphaL_max = 10.0;
            value.first.nL = 1.5;
            value.first.nL_min = 1.0;
            value.first.nL_max = 8.0;
        
            value.first.alphaR = 2.0;
            value.first.alphaR_min = 0.01;
            value.first.alphaR_max = 10.0;
            value.first.nR = 1.5;
            value.first.nR_min = 1.0;
            value.first.nR_max = 8.0;

            // ExponentialBkgParams 설정 (Background)
            value.second.lambda = -1.0;
            value.second.lambda_min = -10.0;
            value.second.lambda_max = 10.0;

            params[key] = value;
        }
    }

    return params;
}
DStarParamMap7 DStarParamMaker7(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap7 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue7 value;

            // DBCrystalBallParams 설정 (Signal)
            value.first.mean = 0.1455;        // D* mass in GeV
            value.first.mean_min = 0.1452;
            value.first.mean_max = 0.1458;
            
            value.first.sigma = 0.0005;
            value.first.sigma_min = 0.0001;
            value.first.sigma_max = 0.01;
            
            value.first.alphaL = 2.0;
            value.first.alphaL_min = 0.01;
            value.first.alphaL_max = 10.0;
            value.first.nL = 1.5;
            value.first.nL_min = 1.0;
            value.first.nL_max = 8.0;
        
            value.first.alphaR = 2.0;
            value.first.alphaR_min = 0.01;
            value.first.alphaR_max = 10.0;
            value.first.nR = 1.5;
            value.first.nR_min = 1.0;
            value.first.nR_max = 8.0;

            // ExpErfBkgParams 설정 (Background)
            value.second.err_mu = 0.139;
            value.second.err_mu_min = 0;
            value.second.err_mu_max = 1;
            value.second.err_sigma = 0.5;
            value.second.err_sigma_min = 0.0;
            value.second.err_sigma_max = 10.0;
            value.second.m_lambda = 1.0;
            value.second.m_lambda_min = 0.0;
            value.second.m_lambda_max = 20.0;

            params[key] = value;
        }
    }

    return params;
}





#endif