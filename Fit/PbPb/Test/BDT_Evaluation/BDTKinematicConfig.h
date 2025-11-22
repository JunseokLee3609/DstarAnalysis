#pragma once

#include <vector>
#include <utility>
#include <string>
#include <map>
#include <array>

#include "TH1D.h"

namespace BDTKinematicConfig {

enum class VarType { PT = 0, RAPIDITY = 1, CENTRALITY = 2, COSTHETA = 3 };

inline const char* VarName(VarType v){
  switch(v){
    case VarType::PT: return "pT";
    case VarType::RAPIDITY: return "y";
    case VarType::CENTRALITY: return "centrality";
    case VarType::COSTHETA: return "costheta";
  }
  return "pT";
}

inline const char* VarLabel(VarType v){
  switch(v){
    case VarType::PT: return "p_{T} [GeV/c]";
    case VarType::RAPIDITY: return "|y|";
    case VarType::CENTRALITY: return "Centrality [%]";
    case VarType::COSTHETA: return "cos(#theta)";
  }
  return "p_{T} [GeV/c]";
}

inline const std::vector<double>& GetEdges(VarType v){
  static const std::vector<double> ptEdges   = {5,7,10,20,30,50};
  static const std::vector<double> yEdges    = {0,0.5,1.0,1.5,2.0,2.5};
  static const std::vector<double> centEdges = {0,10,30,50,90};
  static const std::vector<double> cosEdges  = {0.0,0.2,0.4,0.6,0.8,1.0};
  switch(v){
    case VarType::PT: return ptEdges;
    case VarType::RAPIDITY: return yEdges;
    case VarType::CENTRALITY: return centEdges;
    case VarType::COSTHETA: return cosEdges;
  }
  return ptEdges;
}

// Fine bins for reweighting function (more accurate reweighting)
inline const std::vector<double>& GetFineBinsEdges(VarType v){
  static const std::vector<double> ptFineBinsEdges = {7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,24,26,28,30,35,40,50};
  static const std::vector<double> yEdges    = {0,0.5,1.0,1.5,2.0,2.5};
  static const std::vector<double> centEdges = {0,10,30,50,90};
  static const std::vector<double> cosEdges  = {0.0,0.2,0.4,0.6,0.8,1.0};
  switch(v){
    case VarType::PT: return ptFineBinsEdges;
    case VarType::RAPIDITY: return yEdges;
    case VarType::CENTRALITY: return centEdges;
    case VarType::COSTHETA: return cosEdges;
  }
  return ptFineBinsEdges;
}

inline int GetFineBinsNBins(VarType v){
  const auto& edges = GetFineBinsEdges(v);
  return edges.empty() ? 0 : static_cast<int>(edges.size()) - 1;
}

inline int GetNBins(VarType v){
  const auto& edges = GetEdges(v);
  return edges.empty() ? 0 : static_cast<int>(edges.size()) - 1;
}

inline std::vector<std::pair<double,double>> GetBins(VarType v){
  const auto& edges = GetEdges(v);
  std::vector<std::pair<double,double>> bins;
  if(edges.size() < 2) return bins;
  bins.reserve(edges.size() - 1);
  for(size_t i=0; i+1<edges.size(); ++i){
    bins.emplace_back(edges[i], edges[i+1]);
  }
  return bins;
}

// MVA configuration
inline const std::array<double, 5>& GetMVAThresholds(){
  static const std::array<double, 5> mvaThresholds = {0.9, 0.95, 0.99, 0.995, 0.999};
  return mvaThresholds;
}

inline int GetNMVA(){
  return GetMVAThresholds().size();
}

// Helper to format MVA for use as map key/filename
inline std::string FormatMVA(double mva){
  if(mva == 0.9) return "0p9";
  if(mva == 0.95) return "0p95";
  if(mva == 0.99) return "0p99";
  if(mva == 0.995) return "0p995";
  if(mva == 0.999) return "0p999";
  return "unknown";
}

// Histogram map structure indexed by MVA threshold
template<typename HistType>
class HistogramSet {
public:
  HistogramSet() {
    const auto& mvaThresholds = GetMVAThresholds();
    for(double mva : mvaThresholds) {
      histMap[mva] = nullptr;
    }
  }
  
  HistType* Get(double mva) { return histMap[mva]; }
  void Set(double mva, HistType* hist) { histMap[mva] = hist; }
  
  std::map<double, HistType*> histMap;
};

inline TH1D* CreateHistogram(const char* name, const char* title, VarType v){
  const auto& edges = GetEdges(v);
  return new TH1D(name, title, GetNBins(v), edges.data());
}

} // namespace BDTKinematicConfig
