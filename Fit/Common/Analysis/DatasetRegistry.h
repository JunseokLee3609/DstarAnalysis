#ifndef DSTAR_DATASET_REGISTRY_H
#define DSTAR_DATASET_REGISTRY_H

#include "DStarSingleBinRunner.h"
#include "SimpleDatasetManager.h"

namespace DStarAnalysis {
namespace DatasetRegistry {

inline DatasetConfig GetPPDataset() {
    std::cout << "[DatasetRegistry] Getting PP Dataset..." << std::endl;
    
    // Try to get from JSON first
    if (gDatasetManager.EnsureLoaded()) {
        std::cout << "[DatasetRegistry] JSON manager is loaded, trying to get from JSON..." << std::endl;
        auto ppData = GetDataset("PP_Data");
        auto ppMC = GetDataset("PP_MC");
        
        if (!ppData.file.empty() && !ppMC.file.empty()) {
            std::cout << "[DatasetRegistry] Successfully loaded PP dataset from JSON:" << std::endl;
            std::cout << "[DatasetRegistry]   Data: " << ppData.file << std::endl;
            std::cout << "[DatasetRegistry]   MC: " << ppMC.file << std::endl;
            
            DatasetConfig cfg;
            cfg.dataFile = ppData.file;
            cfg.mcFile = ppMC.file;
            cfg.datasetName = ppData.dataset_name;
            return cfg;
        } else {
            std::cout << "[DatasetRegistry] JSON data incomplete, using fallback..." << std::endl;
        }
    } else {
        std::cout << "[DatasetRegistry] JSON manager not loaded, using fallback..." << std::endl;
    }
    
    // Fallback to hardcoded values
    std::cout << "[DatasetRegistry] Using hardcoded PP dataset values" << std::endl;
    DatasetConfig cfg;
    cfg.dataFile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_ppRef_pp_12Sep25_v1.root";
    cfg.mcFile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_ppRef_ppPU_NoCorrection_20Sep25_v1.root";
    cfg.datasetName = "datasetHX";
    return cfg;
}

inline DatasetConfig GetPbPbDataset() {
    std::cout << "[DatasetRegistry] Getting PbPb Dataset..." << std::endl;
    
    // Try to get from JSON first
    if (gDatasetManager.EnsureLoaded()) {
        std::cout << "[DatasetRegistry] JSON manager is loaded, trying to get from JSON..." << std::endl;
        auto pbpbData = GetDataset("PbPb_Data");
        auto pbpbMC = GetDataset("PbPb_MC");
        
        if (!pbpbData.file.empty() && !pbpbMC.file.empty()) {
            std::cout << "[DatasetRegistry] Successfully loaded PbPb dataset from JSON:" << std::endl;
            std::cout << "[DatasetRegistry]   Data: " << pbpbData.file << std::endl;
            std::cout << "[DatasetRegistry]   MC: " << pbpbMC.file << std::endl;
            
            DatasetConfig cfg;
            cfg.dataFile = pbpbData.file;
            cfg.mcFile = pbpbMC.file;
            cfg.datasetName = pbpbData.dataset_name;
            return cfg;
        } else {
            std::cout << "[DatasetRegistry] JSON data incomplete, using fallback..." << std::endl;
        }
    } else {
        std::cout << "[DatasetRegistry] JSON manager not loaded, using fallback..." << std::endl;
    }
    
    // Fallback to hardcoded values
    std::cout << "[DatasetRegistry] Using hardcoded PbPb dataset values" << std::endl;
    DatasetConfig cfg;
    cfg.dataFile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_PbPb_mva0p99_np_PbPb_Aug25_v1.root";
    cfg.mcFile = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root";
    cfg.datasetName = "datasetHX";
    return cfg;
}

// Custom dataset access functions
inline DatasetConfig GetCustomDataset(const std::string& datasetKey) {
    std::cout << "[DatasetRegistry] Getting custom dataset: " << datasetKey << std::endl;
    DatasetConfig cfg;
    
    if (gDatasetManager.EnsureLoaded()) {
        std::cout << "[DatasetRegistry] JSON manager is loaded, trying to get custom dataset..." << std::endl;
        auto info = GetDataset(datasetKey);
        if (!info.file.empty()) {
            std::cout << "[DatasetRegistry] Successfully loaded custom dataset from JSON:" << std::endl;
            std::cout << "[DatasetRegistry]   File: " << info.file << std::endl;
            std::cout << "[DatasetRegistry]   Dataset Name: " << info.dataset_name << std::endl;
            
            cfg.dataFile = info.file;
            cfg.datasetName = info.dataset_name;
            return cfg;
        } else {
            std::cout << "[DatasetRegistry] Custom dataset not found in JSON: " << datasetKey << std::endl;
        }
    } else {
        std::cout << "[DatasetRegistry] JSON manager not loaded, cannot get custom dataset" << std::endl;
    }
    
    // Fallback - return empty config
    std::cout << "[DatasetRegistry] Returning empty config for custom dataset: " << datasetKey << std::endl;
    return cfg;
}

// Convenience functions for specific custom datasets
inline DatasetConfig GetPPDataV2() {
    return GetCustomDataset("PP_Data_v2");
}

inline DatasetConfig GetPbPbDataMVA095() {
    return GetCustomDataset("PbPb_Data_mva0p95");
}

// Combined custom dataset (Data + MC)
inline DatasetConfig GetCustomCombinedDataset(const std::string& dataKey, const std::string& mcKey) {
    DatasetConfig cfg;
    
    if (gDatasetManager.EnsureLoaded()) {
        auto dataInfo = GetDataset(dataKey);
        auto mcInfo = GetDataset(mcKey);
        
        if (!dataInfo.file.empty() && !mcInfo.file.empty()) {
            cfg.dataFile = dataInfo.file;
            cfg.mcFile = mcInfo.file;
            cfg.datasetName = dataInfo.dataset_name;
            return cfg;
        }
    }
    
    return cfg;
}

} // namespace DatasetRegistry
} // namespace DStarAnalysis

#endif // DSTAR_DATASET_REGISTRY_H
