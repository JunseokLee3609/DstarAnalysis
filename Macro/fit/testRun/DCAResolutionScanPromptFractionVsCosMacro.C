#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include "DCAResolutionScan.cpp"
namespace {
std::vector<double> ParseCsvDoubles(const char* csv, const std::vector<double>& fallback) {
    if (!csv) return fallback;
    std::string str(csv);
    if (str.empty()) return fallback;
    std::vector<double> values;
    std::stringstream ss(str);
    double val = 0.0;
    char sep = 0;
    while (ss >> val) {
        values.push_back(val);
        if (!(ss >> sep)) break;
        if (sep != ',' && sep != ' ' && sep != '\t') {
            ss.unget();
        }
    }
    if (values.empty()) {
        std::cerr << "[DCAResolutionScanPromptFractionVsCosMacro] Warning: could not parse '" << str
                  << "', using fallback list." << std::endl;
        return fallback;
    }
    return values;
}
}
void DCAResolutionScanPromptFractionVsCosMacro(double ptMin = 5.0,
                                               double ptMax = 7.0,
                                               int centMin = 0,
                                               int centMax = 100,
                                               const char* scaleCsv = "0.8,0.9,1.0,1.1,1.2,1.3,1.4",
                                               const char* cosEdgeCsv = "") {
    const std::vector<double> defaultScales = {0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4};
    const std::vector<double> defaultCosEdges = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
    std::vector<double> scales = ParseCsvDoubles(scaleCsv, defaultScales);
    std::vector<double> cosEdges = ParseCsvDoubles(cosEdgeCsv, defaultCosEdges);
    if (cosEdges.size() < 2) {
        std::cerr << "[DCAResolutionScanPromptFractionVsCosMacro] Error: need at least two cos edges." << std::endl;
        return;
    }
    std::vector<std::pair<double, double>> cosBins;
    cosBins.reserve(cosEdges.size() - 1);
    for (size_t i = 0; i + 1 < cosEdges.size(); ++i) {
        cosBins.emplace_back(cosEdges[i], cosEdges[i + 1]);
    }
    std::cout << "[DCAResolutionScanPromptFractionVsCosMacro] Running with pT=" << ptMin << "-" << ptMax
              << ", centrality=" << centMin << "-" << centMax << "%" << std::endl;
    std::cout << "  scales = ";
    for (size_t i = 0; i < scales.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << scales[i];
    }
    std::cout << std::endl;
    std::cout << "  cos edges = ";
    for (size_t i = 0; i < cosEdges.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << cosEdges[i];
    }
    std::cout << std::endl;
    DCAResolutionScanPromptFractionVsCos(ptMin, ptMax, cosBins, centMin, centMax, scales);
}
