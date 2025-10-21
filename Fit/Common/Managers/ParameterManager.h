#ifndef KINEMATIC_PARAMETER_MANAGER_H
#define KINEMATIC_PARAMETER_MANAGER_H

#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include "Params.h"

// Structure to represent a kinematic bin
struct KinematicBin {
    double ptMin;
    double ptMax;
    double cosMin;
    double cosMax;
    
    KinematicBin(double ptMin=4, double ptMax=100, double cosMin = -2, double cosMax = 2)
        : ptMin(ptMin), ptMax(ptMax), cosMin(cosMin), cosMax(cosMax) {}
    
    bool contains(double pt, double cos) const {
        return (pt >= ptMin && pt < ptMax && cos >= cosMin && cos < cosMax);
    }
    
    // Ordering for map
    bool operator<(const KinematicBin& other) const {
        if (ptMin != other.ptMin) return ptMin < other.ptMin;
        if (ptMax != other.ptMax) return ptMax < other.ptMax;
        if (cosMin != other.cosMin) return cosMin < other.cosMin;
        return cosMax < other.cosMax;
    }
};

// Template class to manage parameters for a specific signal + background combination
template<typename SignalParams, typename BackgroundParams>
class KinematicParameterManager {
public:
    using ParamValue = std::pair<SignalParams, BackgroundParams>;
    
    KinematicParameterManager() {
        // Initialize default parameters
        defaultParams = std::make_pair(SignalParams{}, BackgroundParams{});
    }
    
    // Set default parameters that apply if no specific bin matches
    void setDefaultParameters(const SignalParams& signal, const BackgroundParams& background) {
        defaultParams = std::make_pair(signal, background);
    }
    
    // Set parameters for a specific kinematic bin
    void setParameters(const KinematicBin& bin, const SignalParams& signal, const BackgroundParams& background) {
        paramMap[bin] = std::make_pair(signal, background);
    }
    
    // Get parameters for a specific pt, cos point
    ParamValue getParameters(double pt, double cos) const {
        // Check if any bin contains this pt, cos
        for (const auto& entry : paramMap) {
            if (entry.first.contains(pt, cos)) {
                return entry.second;
            }
        }
        
        // If no bin matches, return default parameters
        return defaultParams;
    }
    
    // Get signal parameters for a specific pt, cos point
    SignalParams getSignalParams(double pt, double cos) const {
        return getParameters(pt, cos).first;
    }
    
    // Get background parameters for a specific pt, cos point
    BackgroundParams getBackgroundParams(double pt, double cos) const {
        return getParameters(pt, cos).second;
    }
    
    // Set single parameter value for signal in a specific bin
    template<typename T>
    void setSignalParameter(const KinematicBin& bin, const std::string& paramName, T value) {
        // If bin doesn't exist, initialize with default parameters
        if (paramMap.find(bin) == paramMap.end()) {
            paramMap[bin] = defaultParams;
        }
        
        // Set the specific parameter
        setSpecificParameter(&(paramMap[bin].first), paramName, value);
    }
    
    // Set single parameter value for background in a specific bin
    template<typename T>
    void setBackgroundParameter(const KinematicBin& bin, const std::string& paramName, T value) {
        // If bin doesn't exist, initialize with default parameters
        if (paramMap.find(bin) == paramMap.end()) {
            paramMap[bin] = defaultParams;
        }
        
        // Set the specific parameter
        setSpecificParameter(&(paramMap[bin].second), paramName, value);
    }
    
    // List all defined bins
    std::vector<KinematicBin> getDefinedBins() const {
        std::vector<KinematicBin> bins;
        bins.reserve(paramMap.size());
        for (const auto& entry : paramMap) {
            bins.push_back(entry.first);
        }
        return bins;
    }
    
    // Apply a parameter modification to all defined bins
    template<typename T>
    void setSignalParameterForAllBins(const std::string& paramName, T value) {
        for (auto& entry : paramMap) {
            setSpecificParameter(&(entry.second.first), paramName, value);
        }
        // Also set for default
        setSpecificParameter(&(defaultParams.first), paramName, value);
    }
    
    template<typename T>
    void setBackgroundParameterForAllBins(const std::string& paramName, T value) {
        for (auto& entry : paramMap) {
            setSpecificParameter(&(entry.second.second), paramName, value);
        }
        // Also set for default
        setSpecificParameter(&(defaultParams.second), paramName, value);
    }

private:
    std::map<KinematicBin, ParamValue> paramMap;
    ParamValue defaultParams;
    
    // Helper method to set a specific parameter value
    // Needs to be specialized for each parameter type
    template<typename T>
    void setSpecificParameter(SignalParams* params, const std::string& paramName, T value);
    
    template<typename T>
    void setSpecificParameter(BackgroundParams* params, const std::string& paramName, T value);
};

// Specializations for specific parameter types

// For GaussianParams
template<typename BkgParams>
template<>
void KinematicParameterManager<PDFParams::GaussianParams, BkgParams>::setSpecificParameter(
    PDFParams::GaussianParams* params, const std::string& paramName, double value) {
    if (paramName == "mean") params->mean = value;
    else if (paramName == "sigma") params->sigma = value;
    else if (paramName == "mean_min") params->mean_min = value;
    else if (paramName == "mean_max") params->mean_max = value;
    else if (paramName == "sigma_min") params->sigma_min = value;
    else if (paramName == "sigma_max") params->sigma_max = value;
    else std::cerr << "Unknown parameter: " << paramName << " for GaussianParams" << std::endl;
}

// For DBCrystalBallParams
template<typename BkgParams>
template<>
void KinematicParameterManager<PDFParams::DBCrystalBallParams, BkgParams>::setSpecificParameter(
    PDFParams::DBCrystalBallParams* params, const std::string& paramName, double value) {
    if (paramName == "mean") params->mean = value;
    else if (paramName == "mean_min") params->mean_min = value;
    else if (paramName == "mean_max") params->mean_max = value;
    
    else if (paramName == "sigmaL") params->sigmaL = value;
    else if (paramName == "sigmaL_min") params->sigmaL_min = value;
    else if (paramName == "sigmaL_max") params->sigmaL_max = value;
    
    else if (paramName == "sigmaR") params->sigmaR = value;
    else if (paramName == "sigmaR_min") params->sigmaR_min = value;
    else if (paramName == "sigmaR_max") params->sigmaR_max = value;
    
    else if (paramName == "alphaL") params->alphaL = value;
    else if (paramName == "alphaL_min") params->alphaL_min = value;
    else if (paramName == "alphaL_max") params->alphaL_max = value;
    
    else if (paramName == "alphaR") params->alphaR = value;
    else if (paramName == "alphaR_min") params->alphaR_min = value;
    else if (paramName == "alphaR_max") params->alphaR_max = value;
    
    else if (paramName == "nL") params->nL = value;
    else if (paramName == "nL_min") params->nL_min = value;
    else if (paramName == "nL_max") params->nL_max = value;
    
    else if (paramName == "nR") params->nR = value;
    else if (paramName == "nR_min") params->nR_min = value;
    else if (paramName == "nR_max") params->nR_max = value;
    
    else std::cerr << "Unknown parameter: " << paramName << " for DBCrystalBallParams" << std::endl;
}

// For DoubleDBCrystalBallParams
template<typename BkgParams>
template<>
void KinematicParameterManager<PDFParams::DoubleDBCrystalBallParams, BkgParams>::setSpecificParameter(
    PDFParams::DoubleDBCrystalBallParams* params, const std::string& paramName, double value) {
    // First crystal ball component
    if (paramName == "mean1") params->mean1 = value;
    else if (paramName == "sigmaL1") params->sigmaL1 = value;
    else if (paramName == "sigmaR1") params->sigmaR1 = value;
    else if (paramName == "alphaL1") params->alphaL1 = value;
    else if (paramName == "alphaR1") params->alphaR1 = value;
    else if (paramName == "nL1") params->nL1 = value;
    else if (paramName == "nR1") params->nR1 = value;
    
    // Second crystal ball component
    else if (paramName == "mean2") params->mean2 = value;
    else if (paramName == "sigmaL2") params->sigmaL2 = value;
    else if (paramName == "sigmaR2") params->sigmaR2 = value;
    else if (paramName == "alphaL2") params->alphaL2 = value;
    else if (paramName == "alphaR2") params->alphaR2 = value;
    else if (paramName == "nL2") params->nL2 = value;
    else if (paramName == "nR2") params->nR2 = value;
    
    // Shared min/max limits
    else if (paramName == "mean_min") params->mean_min = value;
    else if (paramName == "mean_max") params->mean_max = value;
    else if (paramName == "sigmaL_min") params->sigmaL_min = value;
    else if (paramName == "sigmaL_max") params->sigmaL_max = value;
    else if (paramName == "sigmaR_min") params->sigmaR_min = value;
    else if (paramName == "sigmaR_max") params->sigmaR_max = value;
    else if (paramName == "alphaL_min") params->alphaL_min = value;
    else if (paramName == "alphaL_max") params->alphaL_max = value;
    else if (paramName == "alphaR_min") params->alphaR_min = value;
    else if (paramName == "alphaR_max") params->alphaR_max = value;
    else if (paramName == "nL_min") params->nL_min = value;
    else if (paramName == "nL_max") params->nL_max = value;
    else if (paramName == "nR_min") params->nR_min = value;
    else if (paramName == "nR_max") params->nR_max = value;
    
    // Fraction parameters
    else if (paramName == "fraction") params->fraction = value;
    else if (paramName == "fraction_min") params->fraction_min = value;
    else if (paramName == "fraction_max") params->fraction_max = value;
    
    else std::cerr << "Unknown parameter: " << paramName << " for DoubleDBCrystalBallParams" << std::endl;
}

// For PolynomialBkgParams
template<typename SignalParams>
template<>
void KinematicParameterManager<SignalParams, PDFParams::PolynomialBkgParams>::setSpecificParameter(
    PDFParams::PolynomialBkgParams* params, const std::string& paramName, double value) {
    // For polynomial params, paramName should be "coef0", "coef1", etc.
    if (paramName.substr(0, 4) == "coef") {
        int index;
        try {
            index = std::stoi(paramName.substr(4));
            // Ensure vector has enough elements
            while (params->coefficients.size() <= index) {
                params->coefficients.push_back(0.0);
                params->coef_min.push_back(-1.0);
                params->coef_max.push_back(1.0);
            }
            params->coefficients[index] = value;
        } catch (...) {
            std::cerr << "Invalid coefficient index: " << paramName << std::endl;
        }
    }
    else std::cerr << "Unknown parameter: " << paramName << " for PolynomialBkgParams" << std::endl;
}

// For PhenomenologicalParams
template<typename SignalParams>
template<>
void KinematicParameterManager<SignalParams, PDFParams::PhenomenologicalParams>::setSpecificParameter(
    PDFParams::PhenomenologicalParams* params, const std::string& paramName, double value) {
    if (paramName == "p0") params->p0 = value;
    else if (paramName == "p1") params->p1 = value;
    else if (paramName == "p2") params->p2 = value;
    else if (paramName == "p0_min") params->p0_min = value;
    else if (paramName == "p0_max") params->p0_max = value;
    else if (paramName == "p1_min") params->p1_min = value;
    else if (paramName == "p1_max") params->p1_max = value;
    else if (paramName == "p2_min") params->p2_min = value;
    else if (paramName == "p2_max") params->p2_max = value;
    else std::cerr << "Unknown parameter: " << paramName << " for PhenomenologicalParams" << std::endl;
}

// Add more specializations for other parameter types as needed

#endif // KINEMATIC_PARAMETER_MANAGER_H