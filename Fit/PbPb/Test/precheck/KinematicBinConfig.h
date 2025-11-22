// KinematicBinConfig.h
// Global kinematic bin configuration for D* analysis
// Defines pT, |y|, centrality, cos(theta), and MVA bins

#ifndef KINEMATIC_BIN_CONFIG_H
#define KINEMATIC_BIN_CONFIG_H

#include <vector>
#include <array>
#include <string>
#include <cmath>
#include <sstream> // Required for std::ostringstream
#include <iomanip>  // Required for std::fixed and std::setprecision
#include <algorithm> // Required for std::replace

namespace KinematicBinConfig {

// ===== Static bin definitions =====

// pT bins [GeV]: [5,7], [7,10], [10,20], [20,30], [30,50]
static const std::array<double, 6> PT_EDGES = {5.0, 7.0, 10.0, 20.0, 30.0, 50.0};
static const int N_PT_BINS = 5;

// |y| bins: [0,0.3], [0.3,0.8], [0.8,1.2]
static const std::array<double, 4> Y_EDGES = {0.0, 0.3, 0.8, 1.2};
static const int N_Y_BINS = 3;

// Centrality bins [%]: [0,10], [10,30], [30,50]
static const std::array<double, 4> CENT_EDGES = {0.0, 10.0, 30.0, 50.0};
static const int N_CENT_BINS = 3;

// cos(theta) bins: [-1, -0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4, 0.6, 0.8, 1]
static const std::array<double, 11> COSTHETA_EDGES = {-1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
static const int N_COSTHETA_BINS = 10;

// MVA thresholds
static const std::array<double, 3> MVA_THRESHOLDS = {0.9, 0.99, 0.999};
static const int N_MVA = 3;

// DCA modes
enum DCAMode {
  INCLUSIVE = 0,
  PROMPT_RICH = 1,
  NONPROMPT_RICH = 2
};

static const int N_DCA_MODES = 3;

// ===== Helper functions =====

inline std::string DCAModeToString(int dcaMode) {
  switch(dcaMode) {
    case INCLUSIVE:      return "inclusive";
    case PROMPT_RICH:    return "prompt_rich";
    case NONPROMPT_RICH: return "nonprompt_rich";
    default:             return "unknown";
  }
}

inline int StringToDCAMode(const std::string& str) {
  if (str == "inclusive") return INCLUSIVE;
  if (str == "prompt_rich") return PROMPT_RICH;
  if (str == "nonprompt_rich") return NONPROMPT_RICH;
  return -1;
}

// Format double with precision and replace '.' with 'p'
inline std::string FormatValP(double val, int prec) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(prec) << val;
  std::string result = oss.str();
  std::replace(result.begin(), result.end(), '.', 'p');
  return result;
}

// Format double without dot-to-p conversion
inline std::string FormatVal(double val, int prec) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(prec) << val;
  return oss.str();
}

// ===== Bin iterators and validators =====

struct KinematicBin {
  double ptMin, ptMax;
  double yMin, yMax;
  double centMin, centMax;
  double cosThetaMin, cosThetaMax;
  double mva;
  int dcaMode;

  std::string ToString() const {
    std::ostringstream oss;
    oss << "pt[" << ptMin << "," << ptMax << "]_"
        << "y[" << yMin << "," << yMax << "]_"
        << "cent[" << centMin << "," << centMax << "]_"
        << "cosTheta[" << cosThetaMin << "," << cosThetaMax << "]_"
        << "mva" << mva << "_"
        << DCAModeToString(dcaMode);
    return oss.str();
  }

  double GetPtMid() const { return 0.5 * (ptMin + ptMax); }
  double GetYMid() const { return 0.5 * (yMin + yMax); }
  double GetCentMid() const { return 0.5 * (centMin + centMax); }
  double GetCosThetaMid() const { return 0.5 * (cosThetaMin + cosThetaMax); }
  double GetCosThetaWidth() const { return 0.5 * (cosThetaMax - cosThetaMin); }
};

// Get all pT bins
inline std::vector<std::pair<double, double>> GetPtBins() {
  std::vector<std::pair<double, double>> bins;
  for (int i = 0; i < N_PT_BINS; ++i) {
    bins.push_back({PT_EDGES[i], PT_EDGES[i+1]});
  }
  return bins;
}

// Get all |y| bins
inline std::vector<std::pair<double, double>> GetYBins() {
  std::vector<std::pair<double, double>> bins;
  for (int i = 0; i < N_Y_BINS; ++i) {
    bins.push_back({Y_EDGES[i], Y_EDGES[i+1]});
  }
  return bins;
}

// Get all centrality bins
inline std::vector<std::pair<double, double>> GetCentBins() {
  std::vector<std::pair<double, double>> bins;
  for (int i = 0; i < N_CENT_BINS; ++i) {
    bins.push_back({CENT_EDGES[i], CENT_EDGES[i+1]});
  }
  return bins;
}

// Get all cos(theta) bins
inline std::vector<std::pair<double, double>> GetCosThetaBins() {
  std::vector<std::pair<double, double>> bins;
  for (int i = 0; i < N_COSTHETA_BINS; ++i) {
    bins.push_back({COSTHETA_EDGES[i], COSTHETA_EDGES[i+1]});
  }
  return bins;
}

} // namespace KinematicBinConfig

#endif // KINEMATIC_BIN_CONFIG_H
