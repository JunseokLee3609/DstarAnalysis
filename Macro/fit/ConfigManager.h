#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include "FitStrategy.h"

// Configuration structures
struct ParameterAdjustmentConfig {
    bool enableAdjustment = true;
    double expansionFactor = 9.0;
    double limitCheckFactor = 3.0;
    std::vector<std::string> skipUpperLimitParams = {"frac", "nsig", "nbkg", "alpha", "m0", "mean"};
    std::vector<std::string> skipLowerLimitParams = {"sigma", "frac", "nsig", "nbkg", "m0", "mean", "alpha"};
    std::vector<std::string> allowUpperExpansionParams = {"n", "p0"};
    std::vector<std::string> allowLowerExpansionParams = {"p0"};
};

struct FitRetryConfig {
    int maxRetries = 3;
    std::vector<std::string> retryStrategies = {"Minuit", "Minuit2", "GSLMultiMin"};
    std::vector<int> strategyLevels = {0, 1, 2};
    bool useProgressiveStrategy = true;
};

struct PlottingConfig {
    int defaultNbins = 100;
    bool drawComponents = true;
    bool drawResiduals = true;
    bool drawPulls = true;
    std::string defaultXTitle = "Mass (GeV/c^{2})";
    std::string defaultYTitle = "Events";
    std::map<std::string, int> colorScheme = {
        {"data", 1},
        {"signal", 2}, 
        {"background", 4},
        {"total", 1}
    };
    std::map<std::string, int> lineStyles = {
        {"signal", 2},      // dashed
        {"background", 3},  // dotted
        {"total", 1}        // solid
    };
};

struct QualityConfig {
    double maxChiSquare = 5.0;
    double maxReducedChiSquare = 3.0;
    int maxFitStatus = 0;
    double minSignalYield = 0.0;
    double maxParameterError = 1.0;
    bool requireMinosErrors = false;
};

// Main configuration class
class ConfigManager {
public:
    static ConfigManager& Instance();
    
    // Configuration loading and saving
    bool LoadFromFile(const std::string& configFile);
    bool SaveToFile(const std::string& configFile) const;
    void LoadDefaults();
    
    // Getters for configuration sections
    const FitConfig& GetFitConfig() const { return fitConfig_; }
    const ParameterAdjustmentConfig& GetParameterAdjustmentConfig() const { return paramAdjustConfig_; }
    const FitRetryConfig& GetFitRetryConfig() const { return fitRetryConfig_; }
    const PlottingConfig& GetPlottingConfig() const { return plottingConfig_; }
    const QualityConfig& GetQualityConfig() const { return qualityConfig_; }
    
    // Setters for configuration sections
    void SetFitConfig(const FitConfig& config) { fitConfig_ = config; }
    void SetParameterAdjustmentConfig(const ParameterAdjustmentConfig& config) { paramAdjustConfig_ = config; }
    void SetFitRetryConfig(const FitRetryConfig& config) { fitRetryConfig_ = config; }
    void SetPlottingConfig(const PlottingConfig& config) { plottingConfig_ = config; }
    void SetQualityConfig(const QualityConfig& config) { qualityConfig_ = config; }
    
    // Individual parameter access
    template<typename T>
    T GetParameter(const std::string& section, const std::string& key, const T& defaultValue = T{}) const;
    
    template<typename T>
    void SetParameter(const std::string& section, const std::string& key, const T& value);
    
    // Configuration validation
    bool ValidateConfiguration() const;
    std::vector<std::string> GetValidationErrors() const;
    
    // Environment-specific configurations
    void LoadEnvironmentConfig(const std::string& environment = "default");
    void SetConfigPath(const std::string& path) { configPath_ = path; }
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // Configuration sections
    FitConfig fitConfig_;
    ParameterAdjustmentConfig paramAdjustConfig_;
    FitRetryConfig fitRetryConfig_;
    PlottingConfig plottingConfig_;
    QualityConfig qualityConfig_;
    
    // Internal storage for flexible parameter access
    std::map<std::string, std::map<std::string, std::string>> parameters_;
    std::string configPath_ = "./";
    
    // Helper methods
    void ParseConfigLine(const std::string& line);
    std::vector<std::string> ParseStringList(const std::string& value) const;
    std::map<std::string, int> ParseIntMap(const std::string& value) const;
    std::string SerializeStringList(const std::vector<std::string>& list) const;
    std::string SerializeIntMap(const std::map<std::string, int>& map) const;
};

// Plugin-based fitting strategy registry
class FitStrategyRegistry {
public:
    using StrategyCreator = std::function<std::unique_ptr<FitStrategy>()>;
    
    static FitStrategyRegistry& Instance();
    
    // Strategy registration
    void RegisterStrategy(const std::string& name, StrategyCreator creator);
    std::unique_ptr<FitStrategy> CreateStrategy(const std::string& name) const;
    std::vector<std::string> GetAvailableStrategies() const;
    
    // Plugin loading
    bool LoadPlugin(const std::string& pluginPath);
    void LoadAllPlugins(const std::string& pluginDirectory);
    
private:
    FitStrategyRegistry() { RegisterDefaultStrategies(); }
    std::map<std::string, StrategyCreator> strategies_;
    
    void RegisterDefaultStrategies();
};

// Configuration-based fit strategy factory
class ConfigurableFitStrategyFactory {
public:
    static std::unique_ptr<FitStrategy> CreateFromConfig(const std::string& strategyName = "");
    static FitConfig CreateFitConfigFromManager();
};

// Implementation
inline ConfigManager& ConfigManager::Instance() {
    static ConfigManager instance;
    return instance;
}

inline void ConfigManager::LoadDefaults() {
    // Load default FitConfig
    fitConfig_ = FitConfig{};
    
    // Load default ParameterAdjustmentConfig
    paramAdjustConfig_ = ParameterAdjustmentConfig{};
    
    // Load default FitRetryConfig
    fitRetryConfig_ = FitRetryConfig{};
    
    // Load default PlottingConfig
    plottingConfig_ = PlottingConfig{};
    
    // Load default QualityConfig
    qualityConfig_ = QualityConfig{};
}

inline bool ConfigManager::LoadFromFile(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file " << configFile << ". Using defaults." << std::endl;
        LoadDefaults();
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        ParseConfigLine(line);
    }
    
    return true;
}

inline bool ConfigManager::SaveToFile(const std::string& configFile) const {
    std::ofstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create config file " << configFile << std::endl;
        return false;
    }
    
    file << "# MassFitter Configuration File" << std::endl;
    file << "# Generated automatically" << std::endl << std::endl;
    
    // Write FitConfig
    file << "[FitConfig]" << std::endl;
    file << "useMinos=" << (fitConfig_.useMinos ? "true" : "false") << std::endl;
    file << "useHesse=" << (fitConfig_.useHesse ? "true" : "false") << std::endl;
    file << "useCUDA=" << (fitConfig_.useCUDA ? "true" : "false") << std::endl;
    file << "verbose=" << (fitConfig_.verbose ? "true" : "false") << std::endl;
    file << "numCPU=" << fitConfig_.numCPU << std::endl;
    file << "maxRetries=" << fitConfig_.maxRetries << std::endl;
    file << "strategy=" << fitConfig_.strategy << std::endl;
    file << "minimizer=" << fitConfig_.minimizer << std::endl;
    file << std::endl;
    
    // Write ParameterAdjustmentConfig
    file << "[ParameterAdjustment]" << std::endl;
    file << "enableAdjustment=" << (paramAdjustConfig_.enableAdjustment ? "true" : "false") << std::endl;
    file << "expansionFactor=" << paramAdjustConfig_.expansionFactor << std::endl;
    file << "limitCheckFactor=" << paramAdjustConfig_.limitCheckFactor << std::endl;
    file << "skipUpperLimitParams=" << SerializeStringList(paramAdjustConfig_.skipUpperLimitParams) << std::endl;
    file << "skipLowerLimitParams=" << SerializeStringList(paramAdjustConfig_.skipLowerLimitParams) << std::endl;
    file << std::endl;
    
    // Write QualityConfig
    file << "[Quality]" << std::endl;
    file << "maxChiSquare=" << qualityConfig_.maxChiSquare << std::endl;
    file << "maxReducedChiSquare=" << qualityConfig_.maxReducedChiSquare << std::endl;
    file << "maxFitStatus=" << qualityConfig_.maxFitStatus << std::endl;
    file << std::endl;
    
    return true;
}

inline void ConfigManager::ParseConfigLine(const std::string& line) {
    std::istringstream iss(line);
    std::string key, value;
    
    if (std::getline(iss, key, '=') && std::getline(iss, value)) {
        // Remove whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Update configurations based on key
        if (key == "useMinos") fitConfig_.useMinos = (value == "true");
        else if (key == "useHesse") fitConfig_.useHesse = (value == "true");
        else if (key == "useCUDA") fitConfig_.useCUDA = (value == "true");
        else if (key == "verbose") fitConfig_.verbose = (value == "true");
        else if (key == "numCPU") fitConfig_.numCPU = std::stoi(value);
        else if (key == "maxRetries") fitConfig_.maxRetries = std::stoi(value);
        else if (key == "strategy") fitConfig_.strategy = value;
        else if (key == "minimizer") fitConfig_.minimizer = value;
        else if (key == "enableAdjustment") paramAdjustConfig_.enableAdjustment = (value == "true");
        else if (key == "expansionFactor") paramAdjustConfig_.expansionFactor = std::stod(value);
        else if (key == "limitCheckFactor") paramAdjustConfig_.limitCheckFactor = std::stod(value);
        else if (key == "maxChiSquare") qualityConfig_.maxChiSquare = std::stod(value);
        else if (key == "maxReducedChiSquare") qualityConfig_.maxReducedChiSquare = std::stod(value);
        else if (key == "maxFitStatus") qualityConfig_.maxFitStatus = std::stoi(value);
        else if (key == "skipUpperLimitParams") paramAdjustConfig_.skipUpperLimitParams = ParseStringList(value);
        else if (key == "skipLowerLimitParams") paramAdjustConfig_.skipLowerLimitParams = ParseStringList(value);
    }
}

inline std::vector<std::string> ConfigManager::ParseStringList(const std::string& value) const {
    std::vector<std::string> result;
    std::istringstream iss(value);
    std::string item;
    
    while (std::getline(iss, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

inline std::string ConfigManager::SerializeStringList(const std::vector<std::string>& list) const {
    std::ostringstream oss;
    for (size_t i = 0; i < list.size(); ++i) {
        if (i > 0) oss << ",";
        oss << list[i];
    }
    return oss.str();
}

inline bool ConfigManager::ValidateConfiguration() const {
    auto errors = GetValidationErrors();
    return errors.empty();
}

inline std::vector<std::string> ConfigManager::GetValidationErrors() const {
    std::vector<std::string> errors;
    
    if (fitConfig_.numCPU <= 0) {
        errors.push_back("numCPU must be greater than 0");
    }
    
    if (fitConfig_.maxRetries < 0) {
        errors.push_back("maxRetries cannot be negative");
    }
    
    if (paramAdjustConfig_.expansionFactor <= 0) {
        errors.push_back("expansionFactor must be positive");
    }
    
    if (qualityConfig_.maxChiSquare <= 0) {
        errors.push_back("maxChiSquare must be positive");
    }
    
    return errors;
}

// FitStrategyRegistry implementation
inline FitStrategyRegistry& FitStrategyRegistry::Instance() {
    static FitStrategyRegistry instance;
    return instance;
}

inline void FitStrategyRegistry::RegisterStrategy(const std::string& name, StrategyCreator creator) {
    strategies_[name] = creator;
}

inline std::unique_ptr<FitStrategy> FitStrategyRegistry::CreateStrategy(const std::string& name) const {
    auto it = strategies_.find(name);
    if (it != strategies_.end()) {
        return it->second();
    }
    
    std::cerr << "Warning: Unknown strategy '" << name << "'. Using BasicFitStrategy." << std::endl;
    return std::make_unique<BasicFitStrategy>();
}

inline std::vector<std::string> FitStrategyRegistry::GetAvailableStrategies() const {
    std::vector<std::string> names;
    for (const auto& [name, creator] : strategies_) {
        names.push_back(name);
    }
    return names;
}

inline void FitStrategyRegistry::RegisterDefaultStrategies() {
    RegisterStrategy("Basic", []() { return std::make_unique<BasicFitStrategy>(); });
    RegisterStrategy("Robust", []() { return std::make_unique<RobustFitStrategy>(); });
    RegisterStrategy("MC", []() { return std::make_unique<MCFitStrategy>(); });
}

// ConfigurableFitStrategyFactory implementation
inline std::unique_ptr<FitStrategy> ConfigurableFitStrategyFactory::CreateFromConfig(const std::string& strategyName) {
    std::string strategy = strategyName;
    if (strategy.empty()) {
        strategy = "Robust"; // Default strategy
    }
    
    return FitStrategyRegistry::Instance().CreateStrategy(strategy);
}

inline FitConfig ConfigurableFitStrategyFactory::CreateFitConfigFromManager() {
    const auto& config = ConfigManager::Instance().GetFitConfig();
    const auto& paramConfig = ConfigManager::Instance().GetParameterAdjustmentConfig();
    
    FitConfig fitConfig = config;
    fitConfig.parameterExpansionFactor = paramConfig.expansionFactor;
    fitConfig.limitCheckFactor = paramConfig.limitCheckFactor;
    fitConfig.enableParameterAdjustment = paramConfig.enableAdjustment;
    fitConfig.skipUpperLimitAdjustment = paramConfig.skipUpperLimitParams;
    fitConfig.skipLowerLimitAdjustment = paramConfig.skipLowerLimitParams;
    fitConfig.allowUpperExpansion = paramConfig.allowUpperExpansionParams;
    fitConfig.allowLowerExpansion = paramConfig.allowLowerExpansionParams;
    
    return fitConfig;
}

template<typename T>
inline T ConfigManager::GetParameter(const std::string& section, const std::string& key, const T& defaultValue) const {
    auto sectionIt = parameters_.find(section);
    if (sectionIt == parameters_.end()) {
        return defaultValue;
    }
    
    auto keyIt = sectionIt->second.find(key);
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    
    std::istringstream iss(keyIt->second);
    T value;
    iss >> value;
    return iss.fail() ? defaultValue : value;
}

template<typename T>
inline void ConfigManager::SetParameter(const std::string& section, const std::string& key, const T& value) {
    std::ostringstream oss;
    oss << value;
    parameters_[section][key] = oss.str();
}

#endif // CONFIG_MANAGER_H