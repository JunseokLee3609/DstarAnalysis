#ifndef CORRECTION_UTILS_H
#define CORRECTION_UTILS_H

// ========================================
// Correction Utilities
// ========================================
// Helper functions for applying corrections in analysis
// ========================================

#include "TFile.h"
#include "TH3D.h"
#include <string>
#include <memory>

namespace CorrectionUtils {

/**
 * @brief Load efficiency map from file
 * @param filePath Path to efficiency map ROOT file
 * @param mapName Name of the histogram in the file
 * @return Unique pointer to TH3D efficiency map
 */
inline std::unique_ptr<TH3D> LoadEfficiencyMap(
    const std::string& filePath, 
    const std::string& mapName = "efficiency_map") 
{
    std::unique_ptr<TFile> file(TFile::Open(filePath.c_str()));
    if (!file || file->IsZombie()) {
        std::cerr << "ERROR: Cannot open efficiency file: " << filePath << std::endl;
        return nullptr;
    }
    
    TH3D* h_eff = dynamic_cast<TH3D*>(file->Get(mapName.c_str()));
    if (!h_eff) {
        std::cerr << "ERROR: Efficiency map '" << mapName << "' not found in file" << std::endl;
        return nullptr;
    }
    
    h_eff->SetDirectory(0);  // Detach from file
    return std::unique_ptr<TH3D>(h_eff);
}

/**
 * @brief Get efficiency weight for a given kinematic point
 * @param h_eff Efficiency map histogram
 * @param pT Transverse momentum
 * @param y Rapidity
 * @param cosTheta Cosine of helicity angle
 * @return Weight = 1/efficiency (or 0 if efficiency is too low)
 */
inline double GetEfficiencyWeight(
    TH3D* h_eff, 
    double pT, 
    double y, 
    double cosTheta,
    double minEfficiency = 1e-6) 
{
    if (!h_eff) return 1.0;
    
    int bin = h_eff->FindBin(pT, y, std::abs(cosTheta));
    double efficiency = h_eff->GetBinContent(bin);
    
    if (efficiency < minEfficiency) {
        return 0.0;  // Skip events with very low efficiency
    }
    
    return 1.0 / efficiency;
}

/**
 * @brief Load acceptance map from file
 */
inline std::unique_ptr<TH2D> LoadAcceptanceMap(
    const std::string& filePath, 
    const std::string& mapName = "acceptance_map") 
{
    std::unique_ptr<TFile> file(TFile::Open(filePath.c_str()));
    if (!file || file->IsZombie()) {
        std::cerr << "ERROR: Cannot open acceptance file: " << filePath << std::endl;
        return nullptr;
    }
    
    TH2D* h_acc = dynamic_cast<TH2D*>(file->Get(mapName.c_str()));
    if (!h_acc) {
        std::cerr << "ERROR: Acceptance map '" << mapName << "' not found in file" << std::endl;
        return nullptr;
    }
    
    h_acc->SetDirectory(0);
    return std::unique_ptr<TH2D>(h_acc);
}

} // namespace CorrectionUtils

#endif // CORRECTION_UTILS_H
