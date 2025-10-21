#ifndef SIMPLE_DATASET_MANAGER_HEADER_ONLY_H
#define SIMPLE_DATASET_MANAGER_HEADER_ONLY_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include "../Utils/JSONParameterLoader.h"  // Use existing SimpleJSON class

struct DatasetInfo {
    std::string file;
    std::string dataset_name;
    std::string description;
    
    DatasetInfo() = default;
    DatasetInfo(const std::string& f, const std::string& ds, const std::string& desc = "")
        : file(f), dataset_name(ds), description(desc) {}
    
    void Print() const {
        std::cout << "File: " << file << std::endl;
        std::cout << "Dataset: " << dataset_name << std::endl;
        std::cout << "Description: " << description << std::endl;
    }
};

class SimpleDatasetManager {
private:
    std::map<std::string, DatasetInfo> datasets_;
    std::string defaultData_;
    std::string defaultMC_;
    std::string configFile_;
    bool loaded_;
    
    // Simple JSON parsing using existing SimpleJSON class
    bool ParseDatasetsJSON(const std::string& filename) {
        SimpleJSON jsonParser;
        jsonParser.loadFromFile(filename);
        
        // Get all keys and find dataset entries
        auto allKeys = jsonParser.getKeys();
        
        for (const auto& key : allKeys) {
            // Check if this is a dataset entry (starts with "datasets." and ends with ".file")
            if (key.find("datasets.") == 0 && key.find(".file") == key.length() - 5) {
                // Extract dataset name (e.g., "datasets.PP_Data.file" -> "PP_Data")
                std::string datasetKey = key.substr(9); // Remove "datasets."
                datasetKey = datasetKey.substr(0, datasetKey.length() - 5); // Remove ".file"
                
                std::string fileKey = "datasets." + datasetKey + ".file";
                std::string nameKey = "datasets." + datasetKey + ".dataset_name";
                std::string descKey = "datasets." + datasetKey + ".description";
                
                std::string file = jsonParser.getString(fileKey, "");
                std::string dataset_name = jsonParser.getString(nameKey, "datasetHX");
                std::string description = jsonParser.getString(descKey, "");
                
                if (!file.empty()) {
                    datasets_[datasetKey] = DatasetInfo(file, dataset_name, description);
                }
            }
        }
        
        // Parse defaults
        defaultData_ = jsonParser.getString("defaults.data", "");
        defaultMC_ = jsonParser.getString("defaults.mc", "");
        
        return true;
    }
    
public:
    SimpleDatasetManager(const std::string& configFile = "datasets.json") 
        : configFile_(configFile), loaded_(false) {
        LoadConfig();
    }
    
    ~SimpleDatasetManager() = default;
    
    // Load JSON configuration
    bool LoadConfig() {
        loaded_ = ParseDatasetsJSON(configFile_);
        if (loaded_) {
            std::cout << "[SimpleDatasetManager] Dataset configuration loaded from: " << configFile_ << std::endl;
            std::cout << "[SimpleDatasetManager] Loaded " << datasets_.size() << " datasets." << std::endl;
            
            // Debug: Print all loaded datasets
            std::cout << "[SimpleDatasetManager] === Loaded Datasets ===" << std::endl;
            for (const auto& pair : datasets_) {
                std::cout << "[SimpleDatasetManager]   " << pair.first << ":" << std::endl;
                std::cout << "[SimpleDatasetManager]     File: " << pair.second.file << std::endl;
                std::cout << "[SimpleDatasetManager]     Dataset Name: " << pair.second.dataset_name << std::endl;
                std::cout << "[SimpleDatasetManager]     Description: " << pair.second.description << std::endl;
            }
            
            // Debug: Print defaults
            std::cout << "[SimpleDatasetManager] === Defaults ===" << std::endl;
            std::cout << "[SimpleDatasetManager]   Default Data: " << defaultData_ << std::endl;
            std::cout << "[SimpleDatasetManager]   Default MC: " << defaultMC_ << std::endl;
            std::cout << "[SimpleDatasetManager] ==========================" << std::endl;
        } else {
            std::cerr << "[SimpleDatasetManager] Failed to load dataset configuration from: " << configFile_ << std::endl;
        }
        return loaded_;
    }
    
    // Get dataset by simple string key
    DatasetInfo GetDataset(const std::string& key) {
        // Debug: trace dataset lookup requests
        std::cout << "[SimpleDatasetManager] GetDataset request key='" << key << "'" << std::endl;
        if (!loaded_) {
            std::cerr << "Config not loaded!" << std::endl;
            return DatasetInfo{};
        }
        
        auto it = datasets_.find(key);
        if (it != datasets_.end()) {
            return it->second;
        } else {
            std::cerr << "Dataset '" << key << "' not found!" << std::endl;
            return DatasetInfo{};
        }
    }
    
    // Get default datasets
    DatasetInfo GetDefaultData() {
        if (!loaded_ || defaultData_.empty()) {
            return DatasetInfo{};
        }
        return GetDataset(defaultData_);
    }
    
    DatasetInfo GetDefaultMC() {
        if (!loaded_ || defaultMC_.empty()) {
            return DatasetInfo{};
        }
        return GetDataset(defaultMC_);
    }
    
    // List all available dataset keys
    std::vector<std::string> GetAvailableDatasets() {
        std::vector<std::string> keys;
        for (const auto& pair : datasets_) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    
    // Print all available datasets
    void PrintAvailableDatasets() {
        if (!loaded_) {
            std::cout << "Config not loaded!" << std::endl;
            return;
        }
        
        std::cout << "\n=== Available Datasets ===" << std::endl;
        
        for (const auto& pair : datasets_) {
            std::cout << pair.first << ": " << pair.second.file << std::endl;
            if (!pair.second.description.empty()) {
                std::cout << "  Description: " << pair.second.description << std::endl;
            }
        }
        
        std::cout << "\n=== Default Datasets ===" << std::endl;
        if (!defaultData_.empty()) {
            auto defaultData = GetDefaultData();
            std::cout << "Default Data (" << defaultData_ << "): " << defaultData.file << std::endl;
        }
        if (!defaultMC_.empty()) {
            auto defaultMC = GetDefaultMC();
            std::cout << "Default MC (" << defaultMC_ << "): " << defaultMC.file << std::endl;
        }
    }
    
    // Check if config is loaded
    bool IsLoaded() const { return loaded_; }
    
    // Get config file path
    std::string GetConfigFile() const { return configFile_; }
    
    // Reload config
    bool Reload() {
        datasets_.clear();
        defaultData_.clear();
        defaultMC_.clear();
        return LoadConfig();
    }
};

// Global instance for easy access
SimpleDatasetManager gDatasetManager;

// Convenience functions - simple string-based access
inline DatasetInfo GetDataset(const std::string& key) {
    return gDatasetManager.GetDataset(key);
}

// Common combinations
inline DatasetInfo GetPPData() { return GetDataset("PP_Data"); }
inline DatasetInfo GetPPMC() { return GetDataset("PP_MC"); }
inline DatasetInfo GetPbPbData() { return GetDataset("PbPb_Data"); }
inline DatasetInfo GetPbPbMC() { return GetDataset("PbPb_MC"); }

// Default datasets
inline DatasetInfo GetDefaultData() { return gDatasetManager.GetDefaultData(); }
inline DatasetInfo GetDefaultMC() { return gDatasetManager.GetDefaultMC(); }

// String combination helpers
inline std::string MakeDatasetKey(const std::string& collisionType, const std::string& dataType) {
    return collisionType + "_" + dataType;
}

inline DatasetInfo GetDataset(const std::string& collisionType, const std::string& dataType) {
    return GetDataset(MakeDatasetKey(collisionType, dataType));
}

// ROOT macro functions
inline void PrintDatasets() {
    gDatasetManager.PrintAvailableDatasets();
}

inline void ReloadDatasets() {
    gDatasetManager.Reload();
}

inline void TestDatasetManager() {
    std::cout << "=== Testing Dataset Manager ===" << std::endl;
    
    if (!gDatasetManager.IsLoaded()) {
        std::cout << "Failed to load dataset configuration!" << std::endl;
        return;
    }
    
    // Test direct access
    std::cout << "\n--- Direct Access ---" << std::endl;
    auto ppData = GetDataset("PP_Data");
    std::cout << "PP_Data: " << ppData.file << std::endl;
    
    auto pbpbMC = GetDataset("PbPb_MC");
    std::cout << "PbPb_MC: " << pbpbMC.file << std::endl;
    
    // Test combination
    std::cout << "\n--- String Combination ---" << std::endl;
    auto combined = GetDataset("PP", "Data");
    std::cout << "GetDataset('PP', 'Data'): " << combined.file << std::endl;
    
    // Test convenience functions
    std::cout << "\n--- Convenience Functions ---" << std::endl;
    auto ppData2 = GetPPData();
    auto pbpbData2 = GetPbPbData();
    std::cout << "GetPPData(): " << ppData2.file << std::endl;
    std::cout << "GetPbPbData(): " << pbpbData2.file << std::endl;
    
    // Test defaults
    std::cout << "\n--- Default Datasets ---" << std::endl;
    auto defaultData = GetDefaultData();
    auto defaultMC = GetDefaultMC();
    std::cout << "GetDefaultData(): " << defaultData.file << std::endl;
    std::cout << "GetDefaultMC(): " << defaultMC.file << std::endl;
}

#endif // SIMPLE_DATASET_MANAGER_HEADER_ONLY_H
