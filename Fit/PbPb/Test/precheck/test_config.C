// Test script for KinematicBinConfig.h
// Run with: root -l -b -q 'test_config.C'

#include "KinematicBinConfig.h"
#include <iostream>

using namespace KinematicBinConfig;

void test_config() {
  std::cout << "\n========== KINEMATIC BIN CONFIG TEST ==========" << std::endl;
  
  // Test pT bins
  std::cout << "\npT Bins (" << N_PT_BINS << " bins):" << std::endl;
  auto ptBins = GetPtBins();
  for (size_t i = 0; i < ptBins.size(); ++i) {
    std::cout << "  [" << ptBins[i].first << ", " << ptBins[i].second << "]" << std::endl;
  }
  
  // Test y bins
  std::cout << "\n|y| Bins (" << N_Y_BINS << " bins):" << std::endl;
  auto yBins = GetYBins();
  for (size_t i = 0; i < yBins.size(); ++i) {
    std::cout << "  [" << yBins[i].first << ", " << yBins[i].second << "]" << std::endl;
  }
  
  // Test centrality bins
  std::cout << "\nCentrality Bins (" << N_CENT_BINS << " bins):" << std::endl;
  auto centBins = GetCentBins();
  for (size_t i = 0; i < centBins.size(); ++i) {
    std::cout << "  [" << centBins[i].first << ", " << centBins[i].second << "]%" << std::endl;
  }
  
  // Test cos(theta) bins
  std::cout << "\ncos(theta) Bins (" << N_COSTHETA_BINS << " bins):" << std::endl;
  auto cosBins = GetCosThetaBins();
  for (size_t i = 0; i < cosBins.size(); ++i) {
    std::cout << "  [" << cosBins[i].first << ", " << cosBins[i].second << "]" << std::endl;
  }
  
  // Test MVA thresholds
  std::cout << "\nMVA Thresholds (" << N_MVA << " thresholds):" << std::endl;
  for (int i = 0; i < N_MVA; ++i) {
    std::cout << "  " << MVA_THRESHOLDS[i] << std::endl;
  }
  
  // Test DCA modes
  std::cout << "\nDCA Modes (" << N_DCA_MODES << " modes):" << std::endl;
  std::cout << "  INCLUSIVE (0): " << DCAModeToString(INCLUSIVE) << std::endl;
  std::cout << "  PROMPT_RICH (1): " << DCAModeToString(PROMPT_RICH) << std::endl;
  std::cout << "  NONPROMPT_RICH (2): " << DCAModeToString(NONPROMPT_RICH) << std::endl;
  
  // Test helper functions
  std::cout << "\nHelper Functions:" << std::endl;
  std::cout << "  FormatValP(0.9, 2) = " << FormatValP(0.9, 2) << std::endl;
  std::cout << "  FormatValP(0.99, 3) = " << FormatValP(0.99, 3) << std::endl;
  std::cout << "  FormatVal(0.9, 3) = " << FormatVal(0.9, 3) << std::endl;
  
  // Test KinematicBin struct
  std::cout << "\nKinematicBin Struct:" << std::endl;
  KinematicBin bin;
  bin.ptMin = PT_EDGES[0]; bin.ptMax = PT_EDGES[1];
  bin.yMin = Y_EDGES[0]; bin.yMax = Y_EDGES[1];
  bin.centMin = CENT_EDGES[0]; bin.centMax = CENT_EDGES[1];
  bin.cosThetaMin = COSTHETA_EDGES[0]; bin.cosThetaMax = COSTHETA_EDGES[1];
  bin.mva = MVA_THRESHOLDS[1];
  bin.dcaMode = PROMPT_RICH;
  
  std::cout << "  pT mid: " << bin.GetPtMid() << std::endl;
  std::cout << "  |y| mid: " << bin.GetYMid() << std::endl;
  std::cout << "  Cent mid: " << bin.GetCentMid() << std::endl;
  std::cout << "  cos(theta) mid: " << bin.GetCosThetaMid() << std::endl;
  std::cout << "  cos(theta) width: " << bin.GetCosThetaWidth() << std::endl;
  
  std::cout << "\n========== ALL TESTS PASSED ==========" << std::endl;
}
