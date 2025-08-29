#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <exception>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

// Custom exceptions for MassFitter
class MassFitterException : public std::exception {
public:
    explicit MassFitterException(const std::string& message, const std::string& context = "")
        : message_(message), context_(context) {
        timestamp_ = GetCurrentTimestamp();
    }
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    const std::string& GetMessage() const { return message_; }
    const std::string& GetContext() const { return context_; }
    const std::string& GetTimestamp() const { return timestamp_; }
    
    virtual std::string GetDetailedMessage() const {
        std::ostringstream oss;
        oss << "[" << timestamp_ << "] ";
        if (!context_.empty()) {
            oss << "[" << context_ << "] ";
        }
        oss << message_;
        return oss.str();
    }
    
private:
    std::string message_;
    std::string context_;
    std::string timestamp_;
    
    static std::string GetCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

class PDFCreationException : public MassFitterException {
public:
    PDFCreationException(const std::string& pdfType, const std::string& reason)
        : MassFitterException("Failed to create " + pdfType + " PDF: " + reason, "PDFFactory") {}
};

class FitException : public MassFitterException {
public:
    FitException(const std::string& fitType, int status, const std::string& details = "")
        : MassFitterException("Fit failed with status " + std::to_string(status) + 
                             (details.empty() ? "" : ": " + details), "FitStrategy"),
          fitType_(fitType), status_(status) {}
    
    const std::string& GetFitType() const { return fitType_; }
    int GetStatus() const { return status_; }
    
private:
    std::string fitType_;
    int status_;
};

class DataException : public MassFitterException {
public:
    DataException(const std::string& operation, const std::string& reason)
        : MassFitterException("Data operation '" + operation + "' failed: " + reason, "DataHandler") {}
};

class ConfigurationException : public MassFitterException {
public:
    ConfigurationException(const std::string& parameter, const std::string& issue)
        : MassFitterException("Configuration error for '" + parameter + "': " + issue, "ConfigManager") {}
};

class ValidationException : public MassFitterException {
public:
    ValidationException(const std::string& validationType, const std::vector<std::string>& errors)
        : MassFitterException("Validation failed for " + validationType, "Validator") {
        for (const auto& error : errors) {
            errors_.push_back(error);
        }
    }
    
    const std::vector<std::string>& GetValidationErrors() const { return errors_; }
    
    std::string GetDetailedMessage() const override {
        std::ostringstream oss;
        oss << MassFitterException::GetDetailedMessage() << std::endl;
        for (const auto& error : errors_) {
            oss << "  - " << error << std::endl;
        }
        return oss.str();
    }
    
private:
    std::vector<std::string> errors_;
};

// Error severity levels
enum class ErrorSeverity {
    Info,
    Warning,
    Error,
    Critical
};

// Error report structure
struct ErrorReport {
    std::string message;
    std::string context;
    ErrorSeverity severity;
    std::string timestamp;
    std::string stackTrace;
    std::map<std::string, std::string> metadata;
};

// Error handler interface
class IErrorHandler {
public:
    virtual ~IErrorHandler() = default;
    virtual void HandleError(const ErrorReport& report) = 0;
    virtual void HandleException(const std::exception& e, const std::string& context = "") = 0;
    virtual void SetLogLevel(ErrorSeverity level) = 0;
    virtual ErrorSeverity GetLogLevel() const = 0;
};

// Console error handler
class ConsoleErrorHandler : public IErrorHandler {
public:
    void HandleError(const ErrorReport& report) override {
        if (report.severity >= logLevel_) {
            std::cerr << FormatErrorReport(report) << std::endl;
        }
    }
    
    void HandleException(const std::exception& e, const std::string& context = "") override {
        ErrorReport report;
        report.message = e.what();
        report.context = context;
        report.severity = ErrorSeverity::Error;
        report.timestamp = GetCurrentTimestamp();
        
        HandleError(report);
    }
    
    void SetLogLevel(ErrorSeverity level) override { logLevel_ = level; }
    ErrorSeverity GetLogLevel() const override { return logLevel_; }
    
private:
    ErrorSeverity logLevel_ = ErrorSeverity::Warning;
    
    std::string FormatErrorReport(const ErrorReport& report) {
        std::ostringstream oss;
        oss << "[" << report.timestamp << "] ";
        oss << "[" << SeverityToString(report.severity) << "] ";
        if (!report.context.empty()) {
            oss << "[" << report.context << "] ";
        }
        oss << report.message;
        return oss.str();
    }
    
    std::string SeverityToString(ErrorSeverity severity) {
        switch (severity) {
            case ErrorSeverity::Info: return "INFO";
            case ErrorSeverity::Warning: return "WARN";
            case ErrorSeverity::Error: return "ERROR";
            case ErrorSeverity::Critical: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    std::string GetCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// File error handler
class FileErrorHandler : public IErrorHandler {
public:
    explicit FileErrorHandler(const std::string& logFilePath) : logFilePath_(logFilePath) {}
    
    void HandleError(const ErrorReport& report) override {
        if (report.severity >= logLevel_) {
            WriteToFile(FormatErrorReport(report));
        }
    }
    
    void HandleException(const std::exception& e, const std::string& context = "") override {
        ErrorReport report;
        report.message = e.what();
        report.context = context;
        report.severity = ErrorSeverity::Error;
        report.timestamp = GetCurrentTimestamp();
        
        HandleError(report);
    }
    
    void SetLogLevel(ErrorSeverity level) override { logLevel_ = level; }
    ErrorSeverity GetLogLevel() const override { return logLevel_; }
    
    void SetLogFile(const std::string& path) { logFilePath_ = path; }
    
private:
    std::string logFilePath_;
    ErrorSeverity logLevel_ = ErrorSeverity::Info;
    
    void WriteToFile(const std::string& message) {
        std::ofstream file(logFilePath_, std::ios::app);
        if (file.is_open()) {
            file << message << std::endl;
        }
    }
    
    std::string FormatErrorReport(const ErrorReport& report) {
        std::ostringstream oss;
        oss << "[" << report.timestamp << "] ";
        oss << "[" << SeverityToString(report.severity) << "] ";
        if (!report.context.empty()) {
            oss << "[" << report.context << "] ";
        }
        oss << report.message;
        
        if (!report.stackTrace.empty()) {
            oss << std::endl << "Stack Trace:" << std::endl << report.stackTrace;
        }
        
        if (!report.metadata.empty()) {
            oss << std::endl << "Metadata:";
            for (const auto& [key, value] : report.metadata) {
                oss << std::endl << "  " << key << ": " << value;
            }
        }
        
        return oss.str();
    }
    
    std::string SeverityToString(ErrorSeverity severity) {
        switch (severity) {
            case ErrorSeverity::Info: return "INFO";
            case ErrorSeverity::Warning: return "WARN";
            case ErrorSeverity::Error: return "ERROR";
            case ErrorSeverity::Critical: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    std::string GetCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// Composite error handler (supports multiple handlers)
class CompositeErrorHandler : public IErrorHandler {
public:
    void AddHandler(std::unique_ptr<IErrorHandler> handler) {
        handlers_.push_back(std::move(handler));
    }
    
    void HandleError(const ErrorReport& report) override {
        for (auto& handler : handlers_) {
            handler->HandleError(report);
        }
    }
    
    void HandleException(const std::exception& e, const std::string& context = "") override {
        for (auto& handler : handlers_) {
            handler->HandleException(e, context);
        }
    }
    
    void SetLogLevel(ErrorSeverity level) override {
        logLevel_ = level;
        for (auto& handler : handlers_) {
            handler->SetLogLevel(level);
        }
    }
    
    ErrorSeverity GetLogLevel() const override { return logLevel_; }
    
private:
    std::vector<std::unique_ptr<IErrorHandler>> handlers_;
    ErrorSeverity logLevel_ = ErrorSeverity::Warning;
};

// Global error handler manager
class ErrorHandlerManager {
public:
    static ErrorHandlerManager& Instance() {
        static ErrorHandlerManager instance;
        return instance;
    }
    
    void SetHandler(std::unique_ptr<IErrorHandler> handler) {
        handler_ = std::move(handler);
    }
    
    IErrorHandler* GetHandler() { return handler_.get(); }
    
    void HandleError(const std::string& message, ErrorSeverity severity = ErrorSeverity::Error, 
                    const std::string& context = "") {
        if (handler_) {
            ErrorReport report;
            report.message = message;
            report.context = context;
            report.severity = severity;
            report.timestamp = GetCurrentTimestamp();
            handler_->HandleError(report);
        }
    }
    
    void HandleException(const std::exception& e, const std::string& context = "") {
        if (handler_) {
            handler_->HandleException(e, context);
        }
    }
    
    // Convenience methods
    void LogInfo(const std::string& message, const std::string& context = "") {
        HandleError(message, ErrorSeverity::Info, context);
    }
    
    void LogWarning(const std::string& message, const std::string& context = "") {
        HandleError(message, ErrorSeverity::Warning, context);
    }
    
    void LogError(const std::string& message, const std::string& context = "") {
        HandleError(message, ErrorSeverity::Error, context);
    }
    
    void LogCritical(const std::string& message, const std::string& context = "") {
        HandleError(message, ErrorSeverity::Critical, context);
    }
    
private:
    ErrorHandlerManager() {
        // Default to console handler
        handler_ = std::make_unique<ConsoleErrorHandler>();
    }
    
    std::unique_ptr<IErrorHandler> handler_;
    
    std::string GetCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// RAII-based exception safety helper
template<typename T>
class SafeOperation {
public:
    SafeOperation(std::function<T()> operation, const std::string& operationName = "")
        : operation_(operation), operationName_(operationName) {}
    
    T Execute() {
        try {
            return operation_();
        } catch (const MassFitterException& e) {
            ErrorHandlerManager::Instance().HandleException(e, operationName_);
            throw;
        } catch (const std::exception& e) {
            ErrorHandlerManager::Instance().HandleException(e, operationName_);
            throw MassFitterException("Unexpected error during " + operationName_ + ": " + e.what(), operationName_);
        } catch (...) {
            auto message = "Unknown error during " + operationName_;
            ErrorHandlerManager::Instance().LogCritical(message, operationName_);
            throw MassFitterException(message, operationName_);
        }
    }
    
    // Execute with fallback value on exception
    T ExecuteWithFallback(const T& fallbackValue) {
        try {
            return Execute();
        } catch (...) {
            return fallbackValue;
        }
    }
    
private:
    std::function<T()> operation_;
    std::string operationName_;
};

// Helper macros for error handling
#define SAFE_EXECUTE(operation, name) \
    SafeOperation<decltype(operation())>([&]() { return operation(); }, name).Execute()

#define SAFE_EXECUTE_WITH_FALLBACK(operation, fallback, name) \
    SafeOperation<decltype(operation())>([&]() { return operation(); }, name).ExecuteWithFallback(fallback)

#define LOG_AND_THROW(exception_type, message, context) \
    do { \
        ErrorHandlerManager::Instance().LogError(message, context); \
        throw exception_type(message, context); \
    } while(0)

// Parameter validation helpers
class Validator {
public:
    template<typename T>
    static void ValidateRange(const T& value, const T& min, const T& max, const std::string& paramName) {
        if (value < min || value > max) {
            throw ValidationException("Parameter " + paramName, 
                {"Value " + std::to_string(value) + " is outside range [" + 
                 std::to_string(min) + ", " + std::to_string(max) + "]"});
        }
    }
    
    static void ValidateNotNull(const void* ptr, const std::string& paramName) {
        if (ptr == nullptr) {
            throw ValidationException("Parameter " + paramName, {"Pointer is null"});
        }
    }
    
    static void ValidateNotEmpty(const std::string& str, const std::string& paramName) {
        if (str.empty()) {
            throw ValidationException("Parameter " + paramName, {"String is empty"});
        }
    }
    
    template<typename Container>
    static void ValidateNotEmpty(const Container& container, const std::string& paramName) {
        if (container.empty()) {
            throw ValidationException("Parameter " + paramName, {"Container is empty"});
        }
    }
};

#endif // ERROR_HANDLER_H