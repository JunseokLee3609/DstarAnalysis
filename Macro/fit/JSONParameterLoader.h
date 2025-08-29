#ifndef JSON_PARAMETER_LOADER_H
#define JSON_PARAMETER_LOADER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <set>
#include <algorithm>

// Simple JSON parser for parameter loading
class SimpleJSON {
private:
    std::map<std::string, std::string> data_;
    std::string current_path_;
    
    void parseObject(const std::string& json, const std::string& prefix = "");
    std::string trim(const std::string& str);
    std::string extractStringValue(const std::string& value);
    
public:
    void parse(const std::string& json);
    void loadFromFile(const std::string& filename);
    
    std::string getString(const std::string& key, const std::string& defaultValue = "");
    double getDouble(const std::string& key, double defaultValue = 0.0);
    int getInt(const std::string& key, int defaultValue = 0);
    bool getBool(const std::string& key, bool defaultValue = false);
    
    bool hasKey(const std::string& key) const;
    std::vector<std::string> getKeys(const std::string& prefix = "") const;
    void printAll() const;
};

// Parameter structure for a single fit parameter
struct FitParameter {
    double value;
    double min;
    double max;
    bool isFixed;
    
    FitParameter() : value(0.0), min(-999.0), max(999.0), isFixed(false) {}
    FitParameter(double v, double minVal, double maxVal, bool fixed = false) 
        : value(v), min(minVal), max(maxVal), isFixed(fixed) {}
};

// Bin identifier structure
struct BinIdentifier {
    double ptMin, ptMax;
    double cosMin, cosMax;
    double centralityMin, centralityMax;
    
    std::string getBinKey() const {
        std::ostringstream oss;
        oss << "pt_" << ptMin << "_" << ptMax 
            << "_cos_" << cosMin << "_" << cosMax
            << "_cent_" << centralityMin << "_" << centralityMax;
        return oss.str();
    }
};

// JSON Parameter Loader class
class JSONParameterLoader {
private:
    SimpleJSON json_;
    std::map<std::string, std::map<std::string, FitParameter>> binParameters_;
    
    FitParameter parseParameter(const std::string& basePath);
    std::string sanitizeKey(const std::string& key);
    
public:
    JSONParameterLoader() = default;
    
    // Load parameters from JSON file
    void loadFromFile(const std::string& filename);
    
    // Get parameter for specific bin and parameter name
    FitParameter getParameter(const BinIdentifier& bin, const std::string& paramName);
    FitParameter getParameter(const std::string& binKey, const std::string& paramName);
    
    // Check if parameter exists
    bool hasParameter(const BinIdentifier& bin, const std::string& paramName);
    bool hasParameter(const std::string& binKey, const std::string& paramName);
    
    // Get all available bins
    std::vector<std::string> getAvailableBins() const;
    
    // Get all parameters for a specific bin
    std::map<std::string, FitParameter> getBinParameters(const std::string& binKey);
    
    // Utility functions
    void printLoadedParameters() const;
    std::string getPDFType(const BinIdentifier& bin, const std::string& pdfCategory); // signal or background
    std::string getPDFType(const std::string& binKey, const std::string& pdfCategory);
};

// Implementation
inline std::string SimpleJSON::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n\"");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n\"");
    return str.substr(start, end - start + 1);
}

inline std::string SimpleJSON::extractStringValue(const std::string& value) {
    std::string trimmed = trim(value);
    if (trimmed.front() == '"' && trimmed.back() == '"') {
        return trimmed.substr(1, trimmed.length() - 2);
    }
    return trimmed;
}

inline void SimpleJSON::parseObject(const std::string& json, const std::string& prefix) {
    size_t pos = 0;
    while (pos < json.length()) {
        // Find key
        size_t keyStart = json.find('"', pos);
        if (keyStart == std::string::npos) break;
        keyStart++;
        
        size_t keyEnd = json.find('"', keyStart);
        if (keyEnd == std::string::npos) break;
        
        std::string key = json.substr(keyStart, keyEnd - keyStart);
        std::string fullKey = prefix.empty() ? key : prefix + "." + key;
        
        // Find value
        size_t colonPos = json.find(':', keyEnd);
        if (colonPos == std::string::npos) break;
        
        size_t valueStart = colonPos + 1;
        while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t')) {
            valueStart++;
        }
        
        if (valueStart >= json.length()) break;
        
        size_t valueEnd;
        if (json[valueStart] == '{') {
            // Nested object
            int braceCount = 1;
            valueEnd = valueStart + 1;
            while (valueEnd < json.length() && braceCount > 0) {
                if (json[valueEnd] == '{') braceCount++;
                else if (json[valueEnd] == '}') braceCount--;
                valueEnd++;
            }
            std::string nestedJson = json.substr(valueStart + 1, valueEnd - valueStart - 2);
            parseObject(nestedJson, fullKey);
        } else {
            // Simple value
            if (json[valueStart] == '"') {
                valueEnd = json.find('"', valueStart + 1);
                if (valueEnd != std::string::npos) valueEnd++;
            } else {
                valueEnd = json.find_first_of(",}", valueStart);
            }
            
            if (valueEnd == std::string::npos) valueEnd = json.length();
            
            std::string value = json.substr(valueStart, valueEnd - valueStart);
            size_t commaPos = value.find(',');
            if (commaPos != std::string::npos) {
                value = value.substr(0, commaPos);
            }
            
            data_[fullKey] = trim(value);
        }
        
        pos = valueEnd;
    }
}

inline void SimpleJSON::parse(const std::string& json) {
    data_.clear();
    parseObject(json);
}

inline void SimpleJSON::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open JSON file: " + filename);
    }
    
    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    parse(buffer.str());
}

inline std::string SimpleJSON::getString(const std::string& key, const std::string& defaultValue) {
    auto it = data_.find(key);
    if (it != data_.end()) {
        return extractStringValue(it->second);
    }
    return defaultValue;
}

inline double SimpleJSON::getDouble(const std::string& key, double defaultValue) {
    auto it = data_.find(key);
    if (it != data_.end()) {
        try {
            return std::stod(extractStringValue(it->second));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

inline int SimpleJSON::getInt(const std::string& key, int defaultValue) {
    auto it = data_.find(key);
    if (it != data_.end()) {
        try {
            return std::stoi(extractStringValue(it->second));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

inline bool SimpleJSON::getBool(const std::string& key, bool defaultValue) {
    auto it = data_.find(key);
    if (it != data_.end()) {
        std::string value = extractStringValue(it->second);
        return (value == "true" || value == "1");
    }
    return defaultValue;
}

inline bool SimpleJSON::hasKey(const std::string& key) const {
    return data_.find(key) != data_.end();
}

inline std::vector<std::string> SimpleJSON::getKeys(const std::string& prefix) const {
    std::vector<std::string> keys;
    for (const auto& pair : data_) {
        if (prefix.empty() || pair.first.substr(0, prefix.length()) == prefix) {
            keys.push_back(pair.first);
        }
    }
    return keys;
}

inline void SimpleJSON::printAll() const {
    for (const auto& pair : data_) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }
}

inline std::string JSONParameterLoader::sanitizeKey(const std::string& key) {
    std::string result = key;
    std::replace(result.begin(), result.end(), '.', '_');
    std::replace(result.begin(), result.end(), '-', '_');
    return result;
}

inline void JSONParameterLoader::loadFromFile(const std::string& filename) {
    json_.loadFromFile(filename);
    binParameters_.clear();
    
    std::cout << "[JSONParameterLoader] Loading parameters from: " << filename << std::endl;
    
    // Find all bins
    auto keys = json_.getKeys();
    std::set<std::string> binKeys;
    
    for (const auto& key : keys) {
        if (key.find("dstar_parameters.bins.") == 0) {
            size_t start = key.find("bins.") + 5;
            size_t end = key.find(".", start);
            if (end != std::string::npos) {
                std::string binKey = key.substr(start, end - start);
                binKeys.insert(binKey);
            }
        }
    }
    
    // Load parameters for each bin
    for (const auto& binKey : binKeys) {
        std::string binPrefix = "dstar_parameters.bins." + binKey;
        std::cout << "[JSONParameterLoader] Loading bin: " << binKey << std::endl;
        
        // Load signal parameters
        std::string signalPrefix = binPrefix + ".signal_pdf.parameters";
        auto signalKeys = json_.getKeys(signalPrefix);
        for (const auto& key : signalKeys) {
            if (key.find(".value") != std::string::npos) {
                std::string paramBase = key.substr(0, key.find(".value"));
                std::string paramName = "signal_" + paramBase.substr(paramBase.find_last_of('.') + 1);
                
                FitParameter param;
                param.value = json_.getDouble(paramBase + ".value", 0.0);
                param.min = json_.getDouble(paramBase + ".min", -999.0);
                param.max = json_.getDouble(paramBase + ".max", 999.0);
                param.isFixed = json_.getBool(paramBase + ".fixed", false);
                
                binParameters_[binKey][paramName] = param;
            }
        }
        
        // Load background parameters
        std::string bgPrefix = binPrefix + ".background_pdf.parameters";
        auto bgKeys = json_.getKeys(bgPrefix);
        for (const auto& key : bgKeys) {
            if (key.find(".value") != std::string::npos) {
                std::string paramBase = key.substr(0, key.find(".value"));
                std::string paramName = "background_" + paramBase.substr(paramBase.find_last_of('.') + 1);
                
                FitParameter param;
                param.value = json_.getDouble(paramBase + ".value", 0.0);
                param.min = json_.getDouble(paramBase + ".min", -999.0);
                param.max = json_.getDouble(paramBase + ".max", 999.0);
                param.isFixed = json_.getBool(paramBase + ".fixed", false);
                
                binParameters_[binKey][paramName] = param;
            }
        }
        
        // Load yield parameters
        std::string yieldPrefix = binPrefix + ".yields";
        auto yieldKeys = json_.getKeys(yieldPrefix);
        for (const auto& key : yieldKeys) {
            if (key.find(".value") != std::string::npos) {
                std::string paramBase = key.substr(0, key.find(".value"));
                std::string paramName = paramBase.substr(paramBase.find_last_of('.') + 1);
                
                FitParameter param;
                param.value = json_.getDouble(paramBase + ".value", 0.0);
                param.min = json_.getDouble(paramBase + ".min", -999.0);
                param.max = json_.getDouble(paramBase + ".max", 999.0);
                param.isFixed = json_.getBool(paramBase + ".fixed", false);
                
                binParameters_[binKey][paramName] = param;
            }
        }
    }
    
    std::cout << "[JSONParameterLoader] Loaded " << binKeys.size() << " bins with parameters." << std::endl;
}

inline FitParameter JSONParameterLoader::getParameter(const BinIdentifier& bin, const std::string& paramName) {
    return getParameter(bin.getBinKey(), paramName);
}

inline FitParameter JSONParameterLoader::getParameter(const std::string& binKey, const std::string& paramName) {
    auto binIt = binParameters_.find(binKey);
    if (binIt != binParameters_.end()) {
        auto paramIt = binIt->second.find(paramName);
        if (paramIt != binIt->second.end()) {
            return paramIt->second;
        }
    }
    
    std::cerr << "[JSONParameterLoader] Warning: Parameter '" << paramName 
              << "' not found for bin '" << binKey << "'. Using default." << std::endl;
    return FitParameter(); // Return default
}

inline bool JSONParameterLoader::hasParameter(const BinIdentifier& bin, const std::string& paramName) {
    return hasParameter(bin.getBinKey(), paramName);
}

inline bool JSONParameterLoader::hasParameter(const std::string& binKey, const std::string& paramName) {
    auto binIt = binParameters_.find(binKey);
    if (binIt != binParameters_.end()) {
        return binIt->second.find(paramName) != binIt->second.end();
    }
    return false;
}

inline std::vector<std::string> JSONParameterLoader::getAvailableBins() const {
    std::vector<std::string> bins;
    for (const auto& pair : binParameters_) {
        bins.push_back(pair.first);
    }
    return bins;
}

inline std::map<std::string, FitParameter> JSONParameterLoader::getBinParameters(const std::string& binKey) {
    auto it = binParameters_.find(binKey);
    if (it != binParameters_.end()) {
        return it->second;
    }
    return std::map<std::string, FitParameter>();
}

inline void JSONParameterLoader::printLoadedParameters() const {
    std::cout << "\n=== Loaded Parameters ===" << std::endl;
    for (const auto& binPair : binParameters_) {
        std::cout << "Bin: " << binPair.first << std::endl;
        for (const auto& paramPair : binPair.second) {
            const auto& param = paramPair.second;
            std::cout << "  " << paramPair.first << ": " 
                      << param.value << " [" << param.min << ", " << param.max << "]"
                      << (param.isFixed ? " (fixed)" : "") << std::endl;
        }
        std::cout << std::endl;
    }
}

inline std::string JSONParameterLoader::getPDFType(const BinIdentifier& bin, const std::string& pdfCategory) {
    return getPDFType(bin.getBinKey(), pdfCategory);
}

inline std::string JSONParameterLoader::getPDFType(const std::string& binKey, const std::string& pdfCategory) {
    std::string key = "dstar_parameters.bins." + binKey + "." + pdfCategory + "_pdf.type";
    return json_.getString(key, "");
}

#endif // JSON_PARAMETER_LOADER_H
