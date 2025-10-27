#ifndef BDT_PARAMETER_LOADER_H
#define BDT_PARAMETER_LOADER_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include "TBufferJSON.h"
#include "TList.h"
#include "TObjString.h"
#include <iomanip> // Required for std::fixed and std::setprecision

// BDT-specific parameter structure
struct BDTFitParameter {
    double value;
    double min;
    double max;
    bool fixed;
    
    BDTFitParameter() : value(0.0), min(-999.0), max(999.0), fixed(false) {}
    BDTFitParameter(double v, double mn, double mx, bool f = false) 
        : value(v), min(mn), max(mx), fixed(f) {}
};

// PDF type information
struct PDFTypeInfo {
    std::string signal_pdf_type;
    std::string background_pdf_type;
    
    PDFTypeInfo() : signal_pdf_type("DBCrystalBall"), background_pdf_type("RooDstD0BG") {}
};

// Bin identifier with MVA threshold
struct BDTBinIdentifier {
    enum class VarType { PT, RAPIDITY, CENTRALITY };
    
    VarType varType;
    double varMin;
    double varMax;
    double mvaThreshold;  // MVA threshold for this specific fit
    
    BDTBinIdentifier(int kinVarInt, double min, double max, double mva = -1.0) 
        : varMin(min), varMax(max), mvaThreshold(mva) {
        if (kinVarInt == 0) varType = VarType::PT;
        else if (kinVarInt == 1) varType = VarType::RAPIDITY;
        else varType = VarType::CENTRALITY;
    }
    
    std::string getVarName() const {
        switch(varType) {
            case VarType::PT: return "pT";
            case VarType::RAPIDITY: return "y";
            case VarType::CENTRALITY: return "centrality";
        }
        return "unknown";
    }
    
    // Create kinematic bin key (without MVA)
    std::string getKinematicBinKey() const {
        std::ostringstream oss;
        oss << getVarName() << "_";
        
        // Format varMin - use one decimal place
        oss << std::fixed << std::setprecision(1) << varMin << "_";
        
        // Format varMax - use one decimal place
        oss << std::fixed << std::setprecision(1) << varMax;
        
        std::string key = oss.str();
        // Replace dots with underscores
        std::replace(key.begin(), key.end(), '.', '_');
        return key;
    }
    
    // Create full bin key with MVA
    std::string getFullBinKey() const {
        if (mvaThreshold < 0) return getKinematicBinKey();
        
        std::ostringstream oss;
        oss << getKinematicBinKey() << "_mva_" << std::fixed << std::setprecision(3) << mvaThreshold;
        std::string key = oss.str();
        // Replace dots with underscores
        std::replace(key.begin(), key.end(), '.', '_');
        return key;
    }
    
    // Get MVA bin key for JSON lookup
    std::string getMVABinKey() const {
        std::ostringstream oss;
        oss << "mva_" << std::fixed << std::setprecision(3) << mvaThreshold;
        std::string key = oss.str();
        std::replace(key.begin(), key.end(), '.', '_');
        return key;
    }
};

// BDT Parameter Loader class
class BDTParameterLoader {
private:
    std::map<std::string, std::map<std::string, BDTFitParameter>> binParameters_;
    std::map<std::string, PDFTypeInfo> pdfTypes_;  // Store PDF types per bin
    std::string jsonFile_;
    
    // Simple JSON parser (lightweight)
    std::string trim(const std::string& str) const {
        size_t start = str.find_first_not_of(" \t\r\n\"");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n\"");
        return str.substr(start, end - start + 1);
    }
    
    double parseDouble(const std::string& json, const std::string& key, double defaultVal = 0.0) const {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return defaultVal;
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return defaultVal;
        pos++;
        
        size_t end = json.find_first_of(",}", pos);
        if (end == std::string::npos) return defaultVal;
        
        try {
            return std::stod(trim(json.substr(pos, end - pos)));
        } catch (...) {
            return defaultVal;
        }
    }
    
    bool parseBool(const std::string& json, const std::string& key, bool defaultVal = false) const {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return defaultVal;
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return defaultVal;
        pos++;
        
        // Skip whitespace after colon
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) {
            pos++;
        }
        
        // Check for "true" or "false" directly
        if (pos + 4 <= json.length() && json.substr(pos, 4) == "true") {
            return true;
        } else if (pos + 5 <= json.length() && json.substr(pos, 5) == "false") {
            return false;
        }
        
        // Fallback: try to extract and trim the value
        size_t end = json.find_first_of(",}", pos);
        if (end == std::string::npos) return defaultVal;
        
        std::string value = trim(json.substr(pos, end - pos));
        return (value == "true" || value == "1");
    }
    
    std::string parseString(const std::string& json, const std::string& key, const std::string& defaultVal = "") const {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return defaultVal;
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return defaultVal;
        pos++;
        
        // Find opening quote
        pos = json.find("\"", pos);
        if (pos == std::string::npos) return defaultVal;
        pos++;
        
        // Find closing quote
        size_t end = json.find("\"", pos);
        if (end == std::string::npos) return defaultVal;
        
        return json.substr(pos, end - pos);
    }
    
    std::string extractBlock(const std::string& json, const std::string& key) const {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return "";
        
        pos = json.find("{", pos);
        if (pos == std::string::npos) return "";
        
        int braceCount = 1;
        size_t start = pos + 1;
        pos++;
        
        while (pos < json.length() && braceCount > 0) {
            if (json[pos] == '{') braceCount++;
            else if (json[pos] == '}') braceCount--;
            pos++;
        }
        
        return json.substr(start, pos - start - 1);
    }
    
    BDTFitParameter parseParameter(const std::string& paramBlock) const {
        BDTFitParameter param;
        param.value = parseDouble(paramBlock, "value", 0.0);
        param.min = parseDouble(paramBlock, "min", -999.0);
        param.max = parseDouble(paramBlock, "max", 999.0);
        param.fixed = parseBool(paramBlock, "fixed", false);
        return param;
    }
    
public:
    BDTParameterLoader() = default;
    
    // Load from JSON file
    void loadFromFile(const std::string& filename) {
        jsonFile_ = filename;
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[BDTParameterLoader] ERROR: Cannot open file: " << filename << std::endl;
            return;
        }
        
        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();
        file.close();
        
        // Parse bins
        std::string binsBlock = extractBlock(json, "bins");
        if (binsBlock.empty()) {
            std::cerr << "[BDTParameterLoader] ERROR: No 'bins' section found!" << std::endl;
            return;
        }
        
        // Find all bin keys (simplified parsing)
        size_t pos = 0;
        while ((pos = binsBlock.find("\"bin_", pos)) != std::string::npos) {
            size_t keyStart = pos + 1;
            size_t keyEnd = binsBlock.find("\"", keyStart);
            if (keyEnd == std::string::npos) break;
            
            std::string binName = binsBlock.substr(keyStart, keyEnd - keyStart);
            std::string binBlock = extractBlock(binsBlock, binName);
            
            if (!binBlock.empty()) {
                parseBin(binName, binBlock);
            }
            
            pos = keyEnd;
        }
    }
    
    void parseBin(const std::string& binName, const std::string& binBlock) {
        // Get bin info
        std::string binInfo = extractBlock(binBlock, "bin_info");
        if (binInfo.empty()) return;
        
        std::string varType = trim(extractStringValue(binInfo, "var_type"));
        double varMin = parseDouble(binInfo, "var_min", -999.0);
        double varMax = parseDouble(binInfo, "var_max", -999.0);
        
        if (varMin == -999.0 || varMax == -999.0 || varType.empty()) return;
        
        // Create kinematic bin key - always use one decimal format for consistency
        std::ostringstream kinBinKey;
        kinBinKey << varType << "_";
        kinBinKey << std::fixed << std::setprecision(1) << varMin << "_";
        kinBinKey << std::fixed << std::setprecision(1) << varMax;
        std::string kinematicKey = kinBinKey.str();
        // Replace dots with underscores
        std::replace(kinematicKey.begin(), kinematicKey.end(), '.', '_');
        
        // Parse MVA bins
        std::string mvaBinsBlock = extractBlock(binBlock, "mva_bins");
        if (mvaBinsBlock.empty()) return;
        
        // Find all MVA bin keys
        size_t pos = 0;
        while ((pos = mvaBinsBlock.find("\"mva_", pos)) != std::string::npos) {
            size_t keyStart = pos + 1;
            size_t keyEnd = mvaBinsBlock.find("\"", keyStart);
            if (keyEnd == std::string::npos) break;
            
            std::string mvaKey = mvaBinsBlock.substr(keyStart, keyEnd - keyStart);
            std::string mvaBinBlock = extractBlock(mvaBinsBlock, mvaKey);
            
            if (!mvaBinBlock.empty()) {
                // Parse this MVA bin
                parseMVABin(kinematicKey, mvaKey, mvaBinBlock);
            }
            
            pos = keyEnd;
        }
    }
    
    void parseMVABin(const std::string& kinBinKey, const std::string& mvaKey, 
                     const std::string& mvaBinBlock) {
        // Create full bin key: kinematic_mva
        std::string fullKey = kinBinKey + "_" + mvaKey;
        
        // Parse PDF types
        PDFTypeInfo pdfInfo;
        std::string signalBlock = extractBlock(mvaBinBlock, "signal_pdf");
        std::string bkgBlock = extractBlock(mvaBinBlock, "background_pdf");
        
        pdfInfo.signal_pdf_type = parseString(signalBlock, "type", "DBCrystalBall");
        pdfInfo.background_pdf_type = parseString(bkgBlock, "type", "RooDstD0BG");
        pdfTypes_[fullKey] = pdfInfo;
        
        // Parse signal parameters
        std::string signalParams = extractBlock(signalBlock, "parameters");
        
        if (!signalParams.empty()) {
            parseParameters(fullKey, "signal_", signalParams);
        }
        
        // Parse background parameters
        std::string bkgParams = extractBlock(bkgBlock, "parameters");
        
        if (!bkgParams.empty()) {
            parseParameters(fullKey, "background_", bkgParams);
        }
        
        // Parse yield ratios
        parseYieldRatios(fullKey, mvaBinBlock);
    }
    
    std::string extractStringValue(const std::string& json, const std::string& key) const {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return "";
        
        pos = json.find(":", pos);
        if (pos == std::string::npos) return "";
        pos++;
        
        size_t end = json.find_first_of(",}", pos);
        if (end == std::string::npos) return "";
        
        return trim(json.substr(pos, end - pos));
    }
    
    void parseParameters(const std::string& binKey, const std::string& prefix, 
                        const std::string& paramsBlock) {
        // Parse each parameter in the block
        std::vector<std::string> paramNames = {"mean", "sigma", "sigmaL", "sigmaR", 
                                               "alphaL", "alphaR", "nL", "nR",
                                               "m", "lambda", "p0", "p1", "p2", "m_pi"};
        
        for (const auto& paramName : paramNames) {
            std::string paramBlock = extractBlock(paramsBlock, paramName);
            if (!paramBlock.empty()) {
                BDTFitParameter param = parseParameter(paramBlock);
                std::string fullName = prefix + paramName;
                binParameters_[binKey][fullName] = param;
            }
        }
    }
    
    void parseYieldRatios(const std::string& binKey, const std::string& mvaBinBlock) {
        std::string yieldBlock = extractBlock(mvaBinBlock, "yield_ratios");
        if (yieldBlock.empty()) return;
        
        double nsigRatio = parseDouble(yieldBlock, "nsig_ratio", 0.5);
        double nbkgRatio = parseDouble(yieldBlock, "nbkg_ratio", 0.5);
        
        // Fallback to complementary ratio if one side is defaulted
        if (nbkgRatio == 0.5 && nsigRatio != 0.5) nbkgRatio = std::max(0.0, std::min(1.0, 1.0 - nsigRatio));
        if (nsigRatio == 0.5 && nbkgRatio != 0.5) nsigRatio = std::max(0.0, std::min(1.0, 1.0 - nbkgRatio));
        
        BDTFitParameter nsigParam(nsigRatio, 0.0, 1.0, false);
        BDTFitParameter nbkgParam(nbkgRatio, 0.0, 1.0, false);
        
        binParameters_[binKey]["yield_nsig_ratio"] = nsigParam;
        binParameters_[binKey]["yield_nbkg_ratio"] = nbkgParam;
    }
    
    // Get parameter for specific bin and MVA
    BDTFitParameter getParameter(const BDTBinIdentifier& bin, const std::string& paramName) const {
        std::string binKey = bin.getFullBinKey();
        
        auto binIt = binParameters_.find(binKey);
        if (binIt != binParameters_.end()) {
            auto paramIt = binIt->second.find(paramName);
            if (paramIt != binIt->second.end()) {
                return paramIt->second;
            }
        }
        
        // Print available bins for debugging
        if (binParameters_.find(binKey) == binParameters_.end()) {
            std::cerr << "[BDTParameterLoader] WARNING: Bin '" << binKey << "' not found!" << std::endl;
            std::cerr << "  Available bins matching kinematic bin:" << std::endl;
            std::string kinKey = bin.getKinematicBinKey();
            for (const auto& b : binParameters_) {
                if (b.first.find(kinKey) == 0) {
                    std::cerr << "    " << b.first << std::endl;
                }
            }
        }
        
        std::cerr << "[BDTParameterLoader] WARNING: Parameter '" << paramName 
                  << "' not found for bin '" << binKey << "'. Using default." << std::endl;
        return BDTFitParameter();
    }
    
    // Check if parameter exists
    bool hasParameter(const BDTBinIdentifier& bin, const std::string& paramName) const {
        std::string binKey = bin.getFullBinKey();
        auto binIt = binParameters_.find(binKey);
        if (binIt != binParameters_.end()) {
            return binIt->second.find(paramName) != binIt->second.end();
        }
        return false;
    }
    
    // Get all parameters for a bin
    std::map<std::string, BDTFitParameter> getBinParameters(const BDTBinIdentifier& bin) const {
        std::string binKey = bin.getFullBinKey();
        auto it = binParameters_.find(binKey);
        if (it != binParameters_.end()) {
            return it->second;
        }
        return std::map<std::string, BDTFitParameter>();
    }
    
    // Get PDF types for a specific bin
    PDFTypeInfo getPDFTypes(const BDTBinIdentifier& bin) const {
        std::string binKey = bin.getFullBinKey();
        
        auto it = pdfTypes_.find(binKey);
        if (it != pdfTypes_.end()) {
            return it->second;
        }
        
        // Return default if not found
        std::cerr << "[BDTParameterLoader] WARNING: PDF types not found for bin '" 
                  << binKey << "', using defaults" << std::endl;
        return PDFTypeInfo();
    }
    
    // Get background PDF type for a specific bin
    std::string getBackgroundPDFType(const BDTBinIdentifier& bin) const {
        return getPDFTypes(bin).background_pdf_type;
    }
    
    // Get signal PDF type for a specific bin
    std::string getSignalPDFType(const BDTBinIdentifier& bin) const {
        return getPDFTypes(bin).signal_pdf_type;
    }
    
    // Print loaded parameters
    void printParameters() const {
        std::cout << "\n=== BDT Loaded Parameters ===" << std::endl;
        for (const auto& binPair : binParameters_) {
            std::cout << "Bin: " << binPair.first << std::endl;
            for (const auto& paramPair : binPair.second) {
                const auto& param = paramPair.second;
                std::cout << "  " << paramPair.first << ": " 
                          << param.value << " [" << param.min << ", " << param.max << "]"
                          << (param.fixed ? " FIXED" : "") << std::endl;
            }
        }
        std::cout << "=============================" << std::endl;
    }
    
    // Print summary of available MVA bins for a kinematic bin
    void printKinematicBinSummary(const BDTBinIdentifier& bin) const {
        std::string kinKey = bin.getKinematicBinKey();
        std::cout << "\n=== Kinematic Bin: " << kinKey << " ===" << std::endl;
        std::cout << "Available MVA thresholds:" << std::endl;
        
        int count = 0;
        for (const auto& b : binParameters_) {
            if (b.first.find(kinKey) == 0) {
                // Extract MVA threshold from key
                size_t mvaPos = b.first.find("mva_");
                if (mvaPos != std::string::npos) {
                    std::string mvaStr = b.first.substr(mvaPos);
                    std::cout << "  - " << mvaStr << std::endl;
                    count++;
                }
            }
        }
        std::cout << "Total: " << count << " MVA bins" << std::endl;
        std::cout << "================================" << std::endl;
    }
    
    // Print matched parameters for specific MVA threshold
    void printMatchedBinForMVA(const BDTBinIdentifier& bin, double mvaThreshold) const {
        // Create bin identifier with MVA
        BDTBinIdentifier mvaBin(bin.varType == BDTBinIdentifier::VarType::PT ? 0 :
                                bin.varType == BDTBinIdentifier::VarType::RAPIDITY ? 1 : 2,
                                bin.varMin, bin.varMax, mvaThreshold);
        
        std::string binKey = mvaBin.getFullBinKey();
        std::cout << "\n--- MVA Threshold: " << mvaThreshold << " ---" << std::endl;
        
        auto it = binParameters_.find(binKey);
        if (it != binParameters_.end()) {
            std::cout << "✓ Found parameters for bin: " << binKey << std::endl;
            
            // Print signal parameters
            std::cout << "  Signal: ";
            for (const auto& paramPair : it->second) {
                if (paramPair.first.find("signal_") == 0) {
                    std::cout << paramPair.first.substr(7) << "=" << std::fixed << std::setprecision(4) << paramPair.second.value << " ";
                }
            }
            std::cout << std::endl;
            
            // Print background parameters
            std::cout << "  Background: ";
            for (const auto& paramPair : it->second) {
                if (paramPair.first.find("background_") == 0) {
                    std::cout << paramPair.first.substr(11) << "=" << std::fixed << std::setprecision(4) << paramPair.second.value << " ";
                }
            }
            std::cout << std::endl;
            
            // Print yield ratios
            std::cout << "  Yield ratios: ";
            for (const auto& paramPair : it->second) {
                if (paramPair.first.find("yield_") == 0) {
                    std::cout << paramPair.first.substr(6) << "=" << std::fixed << std::setprecision(3) << paramPair.second.value << " ";
                }
            }
            std::cout << std::endl;
        } else {
            std::cout << "  WARNING: No parameters found for this MVA bin!" << std::endl;
        }
    }
};

#endif // BDT_PARAMETER_LOADER_H
