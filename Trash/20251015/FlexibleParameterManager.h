#ifndef FLEXIBLE_PARAMETER_MANAGER_H
#define FLEXIBLE_PARAMETER_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <iomanip>
#include "DStarFitConfig.h"

/**
 * @brief Simple JSON-like parser for parameter files (without external dependencies)
 */
class SimpleConfigParser {
public:
    struct ConfigValue {
        enum Type { STRING, DOUBLE, INT, BOOL };
        Type type;
        std::string stringValue;
        double doubleValue;
        int intValue;
        bool boolValue;
        
        ConfigValue(const std::string& val) : type(STRING), stringValue(val) {}
        ConfigValue(double val) : type(DOUBLE), doubleValue(val) {}
        ConfigValue(int val) : type(INT), intValue(val) {}
        ConfigValue(bool val) : type(BOOL), boolValue(val) {}
        
        std::string AsString() const { return stringValue; }
        double AsDouble() const { return type == DOUBLE ? doubleValue : std::stod(stringValue); }
        int AsInt() const { return type == INT ? intValue : std::stoi(stringValue); }
        bool AsBool() const { return type == BOOL ? boolValue : (stringValue == "true"); }
    };
    
    std::map<std::string, std::map<std::string, ConfigValue>> sections;
    
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename) const;
    void Set(const std::string& section, const std::string& key, const ConfigValue& value);
    ConfigValue Get(const std::string& section, const std::string& key, const ConfigValue& defaultValue) const;
    bool HasSection(const std::string& section) const;
    bool HasKey(const std::string& section, const std::string& key) const;
    void Clear();
    std::vector<std::string> GetSections() const;
};

/**
 * @brief Enhanced parameter manager with external configuration file support
 */
class FlexibleParameterManager {
public:
    struct BinConfig {
        double pTMin, pTMax;
        double cosMin, cosMax;
        int centralityMin, centralityMax;
        
        std::string signalPDFType;
        std::string backgroundPDFType;
        
        // Yield parameters
        double nsig_ratio, nsig_min_ratio, nsig_max_ratio;
        double nbkg_ratio, nbkg_min_ratio, nbkg_max_ratio;
        
        // Dynamic parameter storage
        std::map<std::string, double> signalParams;
        std::map<std::string, double> backgroundParams;
        
        BinConfig() {
            // Default values
            pTMin = 4.0; pTMax = 100.0;
            cosMin = -2.0; cosMax = 2.0;
            centralityMin = 0; centralityMax = 100;
            signalPDFType = "DoubleGaussian";
            backgroundPDFType = "ThresholdFunction";
            nsig_ratio = 0.01; nsig_min_ratio = 0.0; nsig_max_ratio = 1.0;
            nbkg_ratio = 0.01; nbkg_min_ratio = 0.0; nbkg_max_ratio = 1.0;
        }
        
        std::string GetBinName() const {
            return "pT_" + std::to_string(pTMin) + "_" + std::to_string(pTMax) + 
                   "_cos_" + std::to_string(cosMin) + "_" + std::to_string(cosMax) +
                   "_cent_" + std::to_string(centralityMin) + "_" + std::to_string(centralityMax);
        }
        
        KinematicBin ToKinematicBin() const {
            return KinematicBin(pTMin, pTMax, cosMin, cosMax, centralityMin, centralityMax);
        }
    };
    
    explicit FlexibleParameterManager(const std::string& configFile = "");
    ~FlexibleParameterManager() = default;
    
    // Configuration file operations
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename) const;
    void CreateTemplateFile(const std::string& filename, const std::vector<KinematicBin>& bins = {}) const;
    
    // Parameter access by bin
    DStarBinParameters GetParametersForBin(const KinematicBin& bin) const;
    void SetParametersForBin(const KinematicBin& bin, const DStarBinParameters& params);
    
    // Direct parameter access
    template<typename T>
    void SetParameter(const std::string& binName, const std::string& category, 
                     const std::string& paramName, const T& value);
    
    template<typename T>
    T GetParameter(const std::string& binName, const std::string& category, 
                   const std::string& paramName, const T& defaultValue) const;
    
    // Global parameter operations
    template<typename T>
    void SetGlobalParameter(const std::string& category, const std::string& paramName, const T& value);
    
    template<typename T>
    T GetGlobalParameter(const std::string& category, const std::string& paramName, const T& defaultValue) const;
    
    // Bin management
    void AddBin(const KinematicBin& bin);
    void RemoveBin(const KinematicBin& bin);
    std::vector<std::string> GetConfiguredBins() const;
    std::vector<KinematicBin> GetAllBins() const;
    
    // Parameter validation and utilities
    bool ValidateConfiguration() const;
    void PrintSummary() const;
    void PrintBinParameters(const std::string& binName) const;
    bool IsValid() const { return isValid_; }
    
    // Batch operations
    void ApplyParameterToAllBins(const std::string& category, const std::string& paramName, double value);
    void CopyParametersFromBin(const std::string& sourceBin, const std::string& targetBin);
    
    // Advanced features
    void OptimizeParametersForPtRange(double pTMin, double pTMax);
    void SetParameterScanRange(const std::string& binName, const std::string& category,
                              const std::string& paramName, double min, double max, int steps);
    std::vector<std::map<std::string, double>> GetParameterScanPoints(const std::string& binName) const;
    
private:
    SimpleConfigParser config_;
    std::map<std::string, BinConfig> binConfigs_;
    std::string configFile_;
    bool isValid_ = false;
    
    // Helper methods
    std::string BinToKey(const KinematicBin& bin) const;
    KinematicBin KeyToBin(const std::string& key) const;
    PDFType StringToPDFType(const std::string& str) const;
    std::string PDFTypeToString(PDFType type) const;
    
    void LoadBinFromConfig(const std::string& binName);
    void SaveBinToConfig(const std::string& binName) const;
    
    DStarBinParameters CreateBinParameters(const BinConfig& config) const;
    BinConfig CreateBinConfig(const KinematicBin& bin, const DStarBinParameters& params) const;
    
    void SetDefaultSignalParameters(BinConfig& config, PDFType pdfType) const;
    void SetDefaultBackgroundParameters(BinConfig& config, PDFType pdfType) const;
    
    bool ValidateBinConfig(const BinConfig& config) const;
    bool ValidateSignalParameters(const BinConfig& config) const;
    bool ValidateBackgroundParameters(const BinConfig& config) const;
};

// Template implementations
template<typename T>
void FlexibleParameterManager::SetParameter(const std::string& binName, const std::string& category, 
                                           const std::string& paramName, const T& value) {
    std::string key = category + "." + paramName;
    
    if constexpr (std::is_same_v<T, std::string>) {
        config_.Set(binName, key, SimpleConfigParser::ConfigValue(value));
    } else if constexpr (std::is_same_v<T, bool>) {
        config_.Set(binName, key, SimpleConfigParser::ConfigValue(value));
    } else if constexpr (std::is_integral_v<T>) {
        config_.Set(binName, key, SimpleConfigParser::ConfigValue(static_cast<int>(value)));
    } else if constexpr (std::is_floating_point_v<T>) {
        config_.Set(binName, key, SimpleConfigParser::ConfigValue(static_cast<double>(value)));
    }
}

template<typename T>
T FlexibleParameterManager::GetParameter(const std::string& binName, const std::string& category, 
                                        const std::string& paramName, const T& defaultValue) const {
    std::string key = category + "." + paramName;
    
    if constexpr (std::is_same_v<T, std::string>) {
        return config_.Get(binName, key, SimpleConfigParser::ConfigValue(defaultValue)).AsString();
    } else if constexpr (std::is_same_v<T, bool>) {
        return config_.Get(binName, key, SimpleConfigParser::ConfigValue(defaultValue)).AsBool();
    } else if constexpr (std::is_integral_v<T>) {
        return static_cast<T>(config_.Get(binName, key, SimpleConfigParser::ConfigValue(static_cast<int>(defaultValue))).AsInt());
    } else if constexpr (std::is_floating_point_v<T>) {
        return static_cast<T>(config_.Get(binName, key, SimpleConfigParser::ConfigValue(static_cast<double>(defaultValue))).AsDouble());
    }
    
    return defaultValue;
}

template<typename T>
void FlexibleParameterManager::SetGlobalParameter(const std::string& category, const std::string& paramName, const T& value) {
    SetParameter("global", category, paramName, value);
}

template<typename T>
T FlexibleParameterManager::GetGlobalParameter(const std::string& category, const std::string& paramName, const T& defaultValue) const {
    return GetParameter("global", category, paramName, defaultValue);
}

/**
 * @brief Convenience functions for easy parameter management
 */
namespace ParameterUtils {
    
    /**
     * @brief Create a basic parameter configuration file with common bins
     */
    inline void CreateBasicConfigFile(const std::string& filename) {
        FlexibleParameterManager manager;
        
        // Add common pT bins
        std::vector<KinematicBin> bins = {
            KinematicBin(4.0, 6.0, -2.0, 2.0, 0, 100),
            KinematicBin(6.0, 8.0, -2.0, 2.0, 0, 100),
            KinematicBin(8.0, 10.0, -2.0, 2.0, 0, 100),
            KinematicBin(10.0, 15.0, -2.0, 2.0, 0, 100),
            KinematicBin(15.0, 20.0, -2.0, 2.0, 0, 100),
            KinematicBin(20.0, 100.0, -2.0, 2.0, 0, 100)
        };
        
        manager.CreateTemplateFile(filename, bins);
        std::cout << "Basic configuration file created: " << filename << std::endl;
    }
    
    /**
     * @brief Load parameters and apply to DStarFitConfig
     */
    inline void ApplyParametersToConfig(DStarFitConfig& config, const std::string& paramFile) {
        FlexibleParameterManager paramManager(paramFile);
        
        if (!paramManager.IsValid()) {
            std::cerr << "Failed to load parameter file: " << paramFile << std::endl;
            return;
        }
        
        auto allBins = config.GetAllKinematicBins();
        for (const auto& bin : allBins) {
            auto params = paramManager.GetParametersForBin(bin);
            config.SetParametersForBin(bin, params);
        }
        
        std::cout << "Parameters applied from: " << paramFile << std::endl;
    }
    
    /**
     * @brief Save current DStarFitConfig parameters to file
     */
    inline void SaveConfigToParameterFile(const DStarFitConfig& config, const std::string& paramFile) {
        FlexibleParameterManager paramManager;
        
        auto allBins = config.GetAllKinematicBins();
        for (const auto& bin : allBins) {
            auto params = config.GetParametersForBin(bin);
            paramManager.SetParametersForBin(bin, params);
        }
        
        if (paramManager.SaveToFile(paramFile)) {
            std::cout << "Parameters saved to: " << paramFile << std::endl;
        } else {
            std::cerr << "Failed to save parameters to: " << paramFile << std::endl;
        }
    }
}

#endif // FLEXIBLE_PARAMETER_MANAGER_H