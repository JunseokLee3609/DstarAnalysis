#ifndef SIMPLE_DATASET_MANAGER_HEADER_ONLY_H
#define SIMPLE_DATASET_MANAGER_HEADER_ONLY_H

#include <cstdlib>
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

    static bool FileExists(const std::string& path) {
        std::ifstream fin(path.c_str(), std::ios::in);
        return fin.good();
    }

    void ClearCachedData() {
        datasets_.clear();
        defaultData_.clear();
        defaultMC_.clear();
        loaded_ = false;
    }

    bool EnsureConfigPath() {
        if (!configFile_.empty()) {
            return true;
        }
        const char* envPath = std::getenv("DSTAR_DATASET_CONFIG");
        if (envPath && *envPath) {
            configFile_ = envPath;
            return true;
        }
        return false;
    }

    bool ParseDatasetsJSON(const std::string& filename) {
        SimpleJSON jsonParser;
        jsonParser.loadFromFile(filename);

        auto allKeys = jsonParser.getKeys();

        for (const auto& key : allKeys) {
            if (key.find("datasets.") == 0 && key.find(".file") == key.length() - 5) {
                std::string datasetKey = key.substr(9);
                datasetKey = datasetKey.substr(0, datasetKey.length() - 5);

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

        defaultData_ = jsonParser.getString("defaults.data", "");
        defaultMC_ = jsonParser.getString("defaults.mc", "");

        return true;
    }

public:
    SimpleDatasetManager(const std::string& configFile = std::string(), bool autoLoad = true)
        : configFile_(configFile), loaded_(false) {
        if (configFile_.empty()) {
            const char* envPath = std::getenv("DSTAR_DATASET_CONFIG");
            if (envPath && *envPath) {
                configFile_ = envPath;
            }
        }
        if (autoLoad) {
            EnsureLoaded();
        }
    }

    ~SimpleDatasetManager() = default;

    void SetConfigFile(const std::string& configFile, bool loadNow = true) {
        configFile_ = configFile;
        ClearCachedData();
        if (loadNow) {
            EnsureLoaded();
        }
    }

    bool EnsureLoaded() {
        if (loaded_) {
            return true;
        }

        if (!EnsureConfigPath()) {
            std::cerr << "[SimpleDatasetManager] No dataset configuration path set. Set DSTAR_DATASET_CONFIG or call SetConfigFile()." << std::endl;
            return false;
        }

        if (!FileExists(configFile_)) {
            std::cerr << "[SimpleDatasetManager] Dataset configuration file not found: " << configFile_ << std::endl;
            return false;
        }

        return LoadConfig();
    }

    bool LoadConfig() {
        if (configFile_.empty()) {
            std::cerr << "[SimpleDatasetManager] Cannot load dataset configuration: path is empty." << std::endl;
            return false;
        }
        if (!FileExists(configFile_)) {
            std::cerr << "[SimpleDatasetManager] Dataset configuration file not found: " << configFile_ << std::endl;
            return false;
        }

        ClearCachedData();
        loaded_ = ParseDatasetsJSON(configFile_);
        if (loaded_) {
            std::cout << "[SimpleDatasetManager] Dataset configuration loaded from: " << configFile_ << std::endl;
            std::cout << "[SimpleDatasetManager] Loaded " << datasets_.size() << " datasets." << std::endl;

            std::cout << "[SimpleDatasetManager] === Loaded Datasets ===" << std::endl;
            for (const auto& pair : datasets_) {
                std::cout << "[SimpleDatasetManager]   " << pair.first << ":" << std::endl;
                std::cout << "[SimpleDatasetManager]     File: " << pair.second.file << std::endl;
                std::cout << "[SimpleDatasetManager]     Dataset Name: " << pair.second.dataset_name << std::endl;
                std::cout << "[SimpleDatasetManager]     Description: " << pair.second.description << std::endl;
            }

            std::cout << "[SimpleDatasetManager] === Defaults ===" << std::endl;
            std::cout << "[SimpleDatasetManager]   Default Data: " << defaultData_ << std::endl;
            std::cout << "[SimpleDatasetManager]   Default MC: " << defaultMC_ << std::endl;
            std::cout << "[SimpleDatasetManager] ==========================" << std::endl;
        } else {
            std::cerr << "[SimpleDatasetManager] Failed to load dataset configuration from: " << configFile_ << std::endl;
        }
        return loaded_;
    }

    DatasetInfo GetDataset(const std::string& key) {
        std::cout << "[SimpleDatasetManager] GetDataset request key='" << key << "'" << std::endl;
        if (!EnsureLoaded()) {
            std::cerr << "[SimpleDatasetManager] Config not loaded!" << std::endl;
            return DatasetInfo{};
        }

        auto it = datasets_.find(key);
        if (it != datasets_.end()) {
            return it->second;
        }

        std::cerr << "[SimpleDatasetManager] Dataset '" << key << "' not found!" << std::endl;
        return DatasetInfo{};
    }

    DatasetInfo GetDefaultData() {
        if (!EnsureLoaded() || defaultData_.empty()) {
            return DatasetInfo{};
        }
        return GetDataset(defaultData_);
    }

    DatasetInfo GetDefaultMC() {
        if (!EnsureLoaded() || defaultMC_.empty()) {
            return DatasetInfo{};
        }
        return GetDataset(defaultMC_);
    }

    std::vector<std::string> GetAvailableDatasets() {
        if (!EnsureLoaded()) {
            return {};
        }
        std::vector<std::string> keys;
        for (const auto& pair : datasets_) {
            keys.push_back(pair.first);
        }
        return keys;
    }

    void PrintAvailableDatasets() {
        if (!EnsureLoaded()) {
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
            auto defaultData = GetDataset(defaultData_);
            std::cout << "Default Data (" << defaultData_ << "): " << defaultData.file << std::endl;
        }
        if (!defaultMC_.empty()) {
            auto defaultMC = GetDataset(defaultMC_);
            std::cout << "Default MC (" << defaultMC_ << "): " << defaultMC.file << std::endl;
        }
    }

    bool IsLoaded() const { return loaded_; }

    std::string GetConfigFile() const { return configFile_; }

    bool Reload() {
        loaded_ = false;
        return EnsureLoaded();
    }
};
// Global instance for easy access
SimpleDatasetManager gDatasetManager(std::string(), false);

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
    
    if (!gDatasetManager.EnsureLoaded()) {
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
