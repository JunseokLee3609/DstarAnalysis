#include "RobustParameterManager.h"
#include "RooFitResult.h"
#include "RooAbsPdf.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooArgList.h"
#include "RooArgSet.h"
#include "TMatrixDSym.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// ComprehensiveFitDiagnostics 구현
ComprehensiveFitDiagnostics::DiagnosticResult 
ComprehensiveFitDiagnostics::diagnose(const RooFitResult* result,
                                    RooAbsPdf* pdf,
                                    RooDataSet* data,
                                    RobustParameterManager* paramManager) {
    
    DiagnosticResult diagnostic;
    
    if (!result || !pdf || !data) {
        diagnostic.converged = false;
        diagnostic.qualityScore = 0.0;
        diagnostic.recommendation = "Invalid input: null pointer detected";
        return diagnostic;
    }
    
    // 기본 수렴 정보
    diagnostic.converged = (result->status() == 0);
    diagnostic.status = result->status();
    diagnostic.logLikelihood = result->minNll();
    diagnostic.covarianceMatrixValid = (result->covQual() >= 2);
    
    // 자유도 및 카이제곱 계산
    diagnostic.ndf = data->numEntries() - result->floatParsFinal().getSize();
    
    // 근사적 카이제곱 계산 (로그 우도로부터)
    if (std::isfinite(diagnostic.logLikelihood)) {
        diagnostic.chi2ndf = 2.0 * diagnostic.logLikelihood / diagnostic.ndf;
    } else {
        diagnostic.chi2ndf = -1;
    }
    
    // AIC/BIC 점수 계산
    int nParams = result->floatParsFinal().getSize();
    diagnostic.aicScore = calculateAIC(result, nParams);
    diagnostic.bicScore = calculateBIC(result, nParams, data->numEntries());
    
    // 오류 유효성 검사
    diagnostic.hasValidErrors = true;
    const RooArgList& finalPars = result->floatParsFinal();
    for (int i = 0; i < finalPars.getSize(); ++i) {
        auto* var = dynamic_cast<const RooRealVar*>(&finalPars[i]);
        if (var && (var->getError() <= 0 || !std::isfinite(var->getError()))) {
            diagnostic.hasValidErrors = false;
            break;
        }
    }
    
    // 매개변수가 경계에 있는지 확인
    diagnostic.problematicParameters = findProblematicParameters(result);
    diagnostic.parametersAtLimits = !diagnostic.problematicParameters.empty();
    
    // 경고 생성
    diagnostic.warnings = generateWarnings(result, pdf, data);
    
    // 품질 점수 계산
    diagnostic.qualityScore = calculateQualityScore(diagnostic);
    
    // 권장사항 생성
    diagnostic.recommendation = generateRecommendation(diagnostic);
    
    // 상세 보고서 생성
    generateDetailedReport(diagnostic, result, pdf, data);
    
    return diagnostic;
}

void ComprehensiveFitDiagnostics::printDiagnostics(const DiagnosticResult& result) {
    std::cout << "\n=============== FIT DIAGNOSTICS ===============" << std::endl;
    std::cout << "Status: " << (result.converged ? "CONVERGED" : "FAILED") << std::endl;
    std::cout << "Fit Status Code: " << result.status << std::endl;
    std::cout << "Quality Score: " << std::fixed << std::setprecision(3) << result.qualityScore << "/1.0" << std::endl;
    
    if (result.chi2ndf > 0) {
        std::cout << "Chi2/NDF: " << std::fixed << std::setprecision(2) << result.chi2ndf << std::endl;
    }
    
    if (std::isfinite(result.logLikelihood)) {
        std::cout << "Log Likelihood: " << std::fixed << std::setprecision(1) << result.logLikelihood << std::endl;
        std::cout << "AIC Score: " << std::fixed << std::setprecision(1) << result.aicScore << std::endl;
        std::cout << "BIC Score: " << std::fixed << std::setprecision(1) << result.bicScore << std::endl;
    }
    
    std::cout << "Valid Errors: " << (result.hasValidErrors ? "YES" : "NO") << std::endl;
    std::cout << "Covariance Matrix: " << (result.covarianceMatrixValid ? "VALID" : "INVALID") << std::endl;
    std::cout << "Parameters at Limits: " << (result.parametersAtLimits ? "YES" : "NO") << std::endl;
    
    if (!result.problematicParameters.empty()) {
        std::cout << "Problematic Parameters: ";
        for (size_t i = 0; i < result.problematicParameters.size(); ++i) {
            std::cout << result.problematicParameters[i];
            if (i < result.problematicParameters.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    if (!result.warnings.empty()) {
        std::cout << "\nWARNINGS:" << std::endl;
        for (const auto& warning : result.warnings) {
            std::cout << "  • " << warning << std::endl;
        }
    }
    
    std::cout << "\nRECOMMENDATION:" << std::endl;
    std::cout << result.recommendation << std::endl;
    
    if (!result.detailedReport.empty()) {
        std::cout << "\nDETAILED REPORT:" << std::endl;
        std::cout << result.detailedReport << std::endl;
    }
    
    std::cout << "=============================================\n" << std::endl;
}

std::string ComprehensiveFitDiagnostics::generateRecommendation(const DiagnosticResult& result) {
    std::stringstream rec;
    
    if (result.qualityScore >= 0.8) {
        rec << "Excellent fit quality. Results are reliable.";
    } else if (result.qualityScore >= 0.6) {
        rec << "Good fit quality with minor issues. ";
        if (!result.hasValidErrors) {
            rec << "Consider running Hesse or Minos for better error estimates. ";
        }
        if (result.parametersAtLimits) {
            rec << "Some parameters are at limits - consider expanding parameter ranges. ";
        }
    } else if (result.qualityScore >= 0.4) {
        rec << "Moderate fit quality. ";
        if (!result.converged) {
            rec << "Try different fitting strategy or initial parameter values. ";
        }
        if (!result.covarianceMatrixValid) {
            rec << "Covariance matrix issues - run with Hesse(true). ";
        }
        if (result.parametersAtLimits) {
            rec << "Expand parameter ranges for constrained parameters. ";
        }
    } else if (result.qualityScore >= 0.2) {
        rec << "Poor fit quality. ";
        rec << "Try robust fitting strategy, check initial parameter values, ";
        rec << "or consider different PDF model. ";
        if (result.chi2ndf > 5.0) {
            rec << "Very high chi2/NDF suggests model inadequacy. ";
        }
    } else {
        rec << "Very poor fit quality. ";
        rec << "Fundamental issues detected. Check data quality, PDF model choice, ";
        rec << "and parameter initialization. Consider adaptive fitting strategy.";
    }
    
    return rec.str();
}

void ComprehensiveFitDiagnostics::generateDetailedReport(DiagnosticResult& result,
                                                       const RooFitResult* fitResult,
                                                       RooAbsPdf* pdf,
                                                       RooDataSet* data) {
    std::stringstream report;
    
    report << "Fit performed on " << data->numEntries() << " data points" << std::endl;
    report << "Number of floating parameters: " << fitResult->floatParsFinal().getSize() << std::endl;
    report << "Effective degrees of freedom: " << result.ndf << std::endl;
    
    if (fitResult->covQual() >= 0) {
        report << "Covariance matrix quality: " << fitResult->covQual() << " (0=failed, 1=approximate, 2=full, 3=accurate)" << std::endl;
    }
    
    // 매개변수 요약
    const RooArgList& finalPars = fitResult->floatParsFinal();
    if (finalPars.getSize() > 0) {
        report << "\nParameter Summary:" << std::endl;
        for (int i = 0; i < finalPars.getSize(); ++i) {
            auto* var = dynamic_cast<const RooRealVar*>(&finalPars[i]);
            if (var) {
                double val = var->getVal();
                double err = var->getError();
                double min = var->getMin();
                double max = var->getMax();
                
                report << "  " << var->GetName() << ": " 
                      << std::fixed << std::setprecision(6) << val 
                      << " ± " << err;
                
                // 범위 정보
                double range = max - min;
                if (std::abs(val - min) < 0.01 * range) {
                    report << " (AT LOWER LIMIT)";
                } else if (std::abs(val - max) < 0.01 * range) {
                    report << " (AT UPPER LIMIT)";
                }
                
                // 유의성
                if (err > 0) {
                    double significance = std::abs(val) / err;
                    if (significance < 1.0) {
                        report << " [INSIGNIFICANT]";
                    } else if (significance < 2.0) {
                        report << " [WEAK]";
                    }
                }
                
                report << std::endl;
            }
        }
    }
    
    // 상관관계 분석
    const TMatrixDSym& corMatrix = fitResult->correlationMatrix();
    if (corMatrix.GetNrows() > 1) {
        std::vector<std::pair<std::string, double>> highCorrelations;
        
        for (int i = 0; i < corMatrix.GetNrows(); ++i) {
            for (int j = i + 1; j < corMatrix.GetNcols(); ++j) {
                double corr = corMatrix(i, j);
                if (std::abs(corr) > 0.8) {
                    std::string parName1 = finalPars[i].GetName();
                    std::string parName2 = finalPars[j].GetName();
                    highCorrelations.push_back({parName1 + " - " + parName2, corr});
                }
            }
        }
        
        if (!highCorrelations.empty()) {
            report << "\nHigh Correlations (>0.8):" << std::endl;
            for (const auto& [names, corr] : highCorrelations) {
                report << "  " << names << ": " << std::fixed << std::setprecision(3) << corr << std::endl;
            }
        }
    }
    
    result.detailedReport = report.str();
}

double ComprehensiveFitDiagnostics::calculateQualityScore(const DiagnosticResult& result) {
    double score = 1.0;
    
    // 수렴 상태 (40% 가중치)
    if (!result.converged) {
        score *= 0.3;
    }
    
    // 공분산 행렬 품질 (20% 가중치)
    if (!result.covarianceMatrixValid) {
        score *= 0.7;
    }
    
    // 오류 유효성 (15% 가중치)
    if (!result.hasValidErrors) {
        score *= 0.8;
    }
    
    // 매개변수 경계 문제 (15% 가중치)
    if (result.parametersAtLimits) {
        int numProblematic = result.problematicParameters.size();
        score *= std::pow(0.9, numProblematic);
    }
    
    // 카이제곱 품질 (10% 가중치)
    if (result.chi2ndf > 0) {
        if (result.chi2ndf > 5.0) {
            score *= 0.5;
        } else if (result.chi2ndf > 3.0) {
            score *= 0.7;
        } else if (result.chi2ndf > 2.0) {
            score *= 0.9;
        }
    }
    
    return std::max(0.0, std::min(1.0, score));
}

std::vector<std::string> ComprehensiveFitDiagnostics::findProblematicParameters(const RooFitResult* result) {
    std::vector<std::string> problematic;
    
    const RooArgList& finalPars = result->floatParsFinal();
    for (int i = 0; i < finalPars.getSize(); ++i) {
        auto* var = dynamic_cast<const RooRealVar*>(&finalPars[i]);
        if (var) {
            double val = var->getVal();
            double min = var->getMin();
            double max = var->getMax();
            double range = max - min;
            
            // 경계에 너무 가까운 매개변수 확인
            if (std::abs(val - min) < 0.01 * range || 
                std::abs(val - max) < 0.01 * range) {
                problematic.push_back(var->GetName());
            }
            
            // 오류가 없거나 비정상적인 매개변수 확인
            double err = var->getError();
            if (err <= 0 || !std::isfinite(err)) {
                problematic.push_back(var->GetName());
            }
        }
    }
    
    return problematic;
}

std::vector<std::string> ComprehensiveFitDiagnostics::generateWarnings(const RooFitResult* result,
                                                                    RooAbsPdf* pdf,
                                                                    RooDataSet* data) {
    std::vector<std::string> warnings;
    
    // 수렴 상태 확인
    if (result->status() != 0) {
        warnings.push_back("Fit did not converge properly (status: " + std::to_string(result->status()) + ")");
    }
    
    // 공분산 행렬 확인
    if (result->covQual() < 2) {
        warnings.push_back("Covariance matrix quality is poor (quality: " + std::to_string(result->covQual()) + ")");
    }
    
    // 매개변수 개수 vs 데이터 포인트
    int nParams = result->floatParsFinal().getSize();
    int nData = data->numEntries();
    if (nData < 10 * nParams) {
        warnings.push_back("Low data-to-parameter ratio (" + std::to_string(nData) + " data points, " + 
                          std::to_string(nParams) + " parameters)");
    }
    
    // 카이제곱 확인
    int ndf = nData - nParams;
    if (ndf > 0) {
        double chi2ndf = 2.0 * result->minNll() / ndf;
        if (chi2ndf > 5.0) {
            warnings.push_back("Very high chi2/NDF (" + std::to_string(chi2ndf) + ") suggests poor model fit");
        } else if (chi2ndf < 0.1) {
            warnings.push_back("Very low chi2/NDF (" + std::to_string(chi2ndf) + ") may indicate overfitting");
        }
    }
    
    // 매개변수 경계 확인
    auto problematic = findProblematicParameters(result);
    if (problematic.size() > nParams / 2) {
        warnings.push_back("More than half of parameters have issues");
    }
    
    return warnings;
}

double ComprehensiveFitDiagnostics::calculateAIC(const RooFitResult* result, int nParams) {
    if (!std::isfinite(result->minNll())) {
        return std::numeric_limits<double>::infinity();
    }
    return 2.0 * result->minNll() + 2.0 * nParams;
}

double ComprehensiveFitDiagnostics::calculateBIC(const RooFitResult* result, int nParams, int nData) {
    if (!std::isfinite(result->minNll())) {
        return std::numeric_limits<double>::infinity();
    }
    return 2.0 * result->minNll() + nParams * std::log(nData);
}