#include "Helper.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

// FittingLogger의 정적 멤버 정의
LogLevel FittingLogger::current_level_ = LogLevel::INFO;
std::unique_ptr<std::ofstream> FittingLogger::log_file_ = nullptr;

std::string FittingLogger::getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void FittingLogger::setLogLevel(LogLevel level) {
    current_level_ = level;
    log("INFO", "Log level set to: " + std::to_string(static_cast<int>(level)));
}

void FittingLogger::setLogFile(const std::string& filename) {
    if (log_file_) {
        log_file_->close();
    }
    
    log_file_ = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (!log_file_->is_open()) {
        std::cerr << "Warning: Could not open log file: " << filename << std::endl;
        log_file_.reset();
    } else {
        log("INFO", "Log file set to: " + filename);
    }
}

void FittingLogger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) {
        return;
    }
    
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:   levelStr = "DEBUG"; break;
        case LogLevel::INFO:    levelStr = "INFO "; break;
        case LogLevel::WARNING: levelStr = "WARN "; break;
        case LogLevel::ERROR:   levelStr = "ERROR"; break;
    }
    
    std::string logEntry = "[" + getCurrentTimeString() + "] [" + levelStr + "] " + message;
    
    // 콘솔 출력
    if (level >= LogLevel::WARNING) {
        std::cerr << logEntry << std::endl;
    } else {
        std::cout << logEntry << std::endl;
    }
    
    // 파일 출력
    if (log_file_ && log_file_->is_open()) {
        *log_file_ << logEntry << std::endl;
        log_file_->flush();
    }
}

void FittingLogger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void FittingLogger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void FittingLogger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void FittingLogger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void FittingLogger::logFitResult(const RooFitResult* result, const std::string& context) {
    if (!result) {
        warning("Null fit result in context: " + context);
        return;
    }
    
    std::stringstream ss;
    ss << "Fit result [" << context << "]: ";
    ss << "Status=" << result->status();
    ss << ", LogL=" << std::fixed << std::setprecision(2) << result->minNll();
    ss << ", CovQual=" << result->covQual();
    ss << ", NPars=" << result->floatParsFinal().getSize();
    
    info(ss.str());
    
    // 실패한 피팅에 대한 상세 정보
    if (result->status() != 0) {
        warning("Fit failed with status " + std::to_string(result->status()) + " in context: " + context);
        
        // 문제가 있는 매개변수 로깅
        const RooArgList& finalPars = result->floatParsFinal();
        for (int i = 0; i < finalPars.getSize(); ++i) {
            auto* var = dynamic_cast<const RooRealVar*>(&finalPars[i]);
            if (var) {
                double val = var->getVal();
                double err = var->getError();
                double min = var->getMin();
                double max = var->getMax();
                
                if (err <= 0 || !std::isfinite(err)) {
                    warning("Parameter " + std::string(var->GetName()) + " has invalid error: " + std::to_string(err));
                }
                
                double range = max - min;
                if (std::abs(val - min) < 0.01 * range) {
                    warning("Parameter " + std::string(var->GetName()) + " is at lower limit");
                } else if (std::abs(val - max) < 0.01 * range) {
                    warning("Parameter " + std::string(var->GetName()) + " is at upper limit");
                }
            }
        }
    }
}

// ScopedTimer 구현
ScopedTimer::ScopedTimer(const std::string& operation_name) 
    : start_time_(std::chrono::steady_clock::now()), operation_name_(operation_name) {
    FittingLogger::debug("Starting operation: " + operation_name_);
}

ScopedTimer::~ScopedTimer() {
    double elapsed = getElapsedSeconds();
    FittingLogger::debug("Completed operation: " + operation_name_ + 
                        " (took " + std::to_string(elapsed) + " seconds)");
    PerformanceMonitor::recordOperation(operation_name_, elapsed);
}

double ScopedTimer::getElapsedSeconds() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_);
    return duration.count() / 1e6;
}

// PerformanceMonitor의 정적 멤버 정의
std::map<std::string, std::vector<double>> PerformanceMonitor::operation_times_;

void PerformanceMonitor::recordOperation(const std::string& operation, double time_seconds) {
    operation_times_[operation].push_back(time_seconds);
}

void PerformanceMonitor::printStatistics() {
    std::cout << "\n========== PERFORMANCE STATISTICS ==========" << std::endl;
    
    for (const auto& [operation, times] : operation_times_) {
        if (times.empty()) continue;
        
        double total = 0.0;
        double min_time = times[0];
        double max_time = times[0];
        
        for (double time : times) {
            total += time;
            min_time = std::min(min_time, time);
            max_time = std::max(max_time, time);
        }
        
        double avg = total / times.size();
        
        // 표준편차 계산
        double variance = 0.0;
        for (double time : times) {
            variance += (time - avg) * (time - avg);
        }
        double stddev = std::sqrt(variance / times.size());
        
        std::cout << operation << ":" << std::endl;
        std::cout << "  Count: " << times.size() << std::endl;
        std::cout << "  Total: " << std::fixed << std::setprecision(3) << total << "s" << std::endl;
        std::cout << "  Average: " << std::fixed << std::setprecision(3) << avg << "s" << std::endl;
        std::cout << "  Min: " << std::fixed << std::setprecision(3) << min_time << "s" << std::endl;
        std::cout << "  Max: " << std::fixed << std::setprecision(3) << max_time << "s" << std::endl;
        std::cout << "  StdDev: " << std::fixed << std::setprecision(3) << stddev << "s" << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << "===========================================" << std::endl;
}

void PerformanceMonitor::saveStatistics(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        FittingLogger::error("Could not open performance statistics file: " + filename);
        return;
    }
    
    file << "Operation,Count,Total_Time,Average_Time,Min_Time,Max_Time,StdDev_Time" << std::endl;
    
    for (const auto& [operation, times] : operation_times_) {
        if (times.empty()) continue;
        
        double total = 0.0;
        double min_time = times[0];
        double max_time = times[0];
        
        for (double time : times) {
            total += time;
            min_time = std::min(min_time, time);
            max_time = std::max(max_time, time);
        }
        
        double avg = total / times.size();
        
        double variance = 0.0;
        for (double time : times) {
            variance += (time - avg) * (time - avg);
        }
        double stddev = std::sqrt(variance / times.size());
        
        file << operation << "," << times.size() << "," << total << "," 
             << avg << "," << min_time << "," << max_time << "," << stddev << std::endl;
    }
    
    file.close();
    FittingLogger::info("Performance statistics saved to: " + filename);
}

void PerformanceMonitor::reset() {
    operation_times_.clear();
    FittingLogger::info("Performance statistics reset");
}