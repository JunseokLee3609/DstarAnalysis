#pragma once

#include <cmath>

#include "../../Interface/simpleDMC.h"

namespace CandidateSelection {

struct D0RecoCuts {
    float minCandPt = -1.f;
    float maxCandPt = 100.f;
    float maxAbsY = 1.f;
    float minDauPt = 1.f;
    float maxAbsDauEta = 2.4f;
    float minMva = 0.9f;  // Added MVA cut
};

struct D0GenCuts {
    float minCandPt = -1.f;
    float maxCandPt = 100.f;
    float maxAbsY = 1.f;
    float minDauPt = 1.f;
    float maxAbsDauEta = 2.4f;
};

struct DStarRecoCuts {
    float minCandPt = 4.f;
    float maxCandPt = 100.f;
    float maxAbsY = 1.f;
    float maxAbsDauEta = 2.4f;
    float minGrandDauPt = 0.5f;
    float maxAbsGrandDauEta = 2.4f;
    float minMva = 0.9f;  // Added MVA cut
};

struct DStarGenCuts {
    float minCandPt = 4.f;
    float maxCandPt = 100.f;
    float maxAbsY = 1.f;
    float maxAbsDauEta = 2.4f;
    float minGrandDauPt = 0.5f;
    float maxAbsGrandDauEta = 2.4f;
    float minSoftPionPt = 0.4f;
};

inline const D0RecoCuts& D0DataRecoCuts() {
    static const D0RecoCuts cuts{};
    return cuts;
}

inline const D0RecoCuts& D0MCRecoCuts() {
    static const D0RecoCuts cuts{};
    return cuts;
}

inline const D0GenCuts& D0MCGenCuts() {
    static const D0GenCuts cuts{};
    return cuts;
}

inline const DStarRecoCuts& DStarDataRecoCuts() {
    static const DStarRecoCuts cuts{};
    return cuts;
}

inline const DStarRecoCuts& DStarMCRecoCuts() {
    static const DStarRecoCuts cuts{};
    return cuts;
}

inline const DStarGenCuts& DStarMCGenCuts() {
    static const DStarGenCuts cuts{};
    return cuts;
}

inline bool passLowerBound(float value, float minBound) {
    return (minBound < 0.f) || (value >= minBound);
}

inline bool passUpperBound(float value, float maxBound) {
    return (maxBound < 0.f) || (value <= maxBound);
}

template <typename TreeT>
inline bool PassD0Reco(const TreeT& tree, int idx, const D0RecoCuts& cuts) {
    return passLowerBound(tree.pT[idx], cuts.minCandPt) &&
           passUpperBound(tree.pT[idx], cuts.maxCandPt) &&
           passUpperBound(std::fabs(tree.y[idx]), cuts.maxAbsY) &&
           passLowerBound(tree.pTD1[idx], cuts.minDauPt) &&
           passLowerBound(tree.pTD2[idx], cuts.minDauPt) &&
           passUpperBound(std::fabs(tree.EtaD1[idx]), cuts.maxAbsDauEta) &&
           passUpperBound(std::fabs(tree.EtaD2[idx]), cuts.maxAbsDauEta) &&
           passLowerBound(tree.mva[idx], cuts.minMva);  // Added MVA cut
}

inline bool PassD0Data(const DataFormat::simpleDTreeevt& tree, int idx, const D0RecoCuts& cuts) {
    return PassD0Reco(tree, idx, cuts);
}

inline bool PassD0Data(const DataFormat::simpleDTreeevt& tree, int idx) {
    return PassD0Reco(tree, idx, D0DataRecoCuts());
}

inline bool PassD0MC(const DataFormat::simpleDMCTreeevt& tree, int idx, const D0RecoCuts& cuts) {
    return PassD0Reco(tree, idx, cuts);
}

inline bool PassD0MC(const DataFormat::simpleDMCTreeevt& tree, int idx) {
    return PassD0Reco(tree, idx, D0MCRecoCuts());
}

inline bool PassD0MCGen(const DataFormat::simpleDMCTreeevt& tree, int idx, const D0GenCuts& cuts) {
    return passLowerBound(tree.gen_pT[idx], cuts.minCandPt) &&
           passUpperBound(tree.gen_pT[idx], cuts.maxCandPt) &&
           passUpperBound(std::fabs(tree.gen_y[idx]), cuts.maxAbsY) &&
           passLowerBound(tree.gen_D0Dau1_pT[idx], cuts.minDauPt) &&
           passLowerBound(tree.gen_D0Dau2_pT[idx], cuts.minDauPt) &&
           passUpperBound(std::fabs(tree.gen_D0Dau1_eta[idx]), cuts.maxAbsDauEta) &&
           passUpperBound(std::fabs(tree.gen_D0Dau2_eta[idx]), cuts.maxAbsDauEta);
}

inline bool PassD0MCGen(const DataFormat::simpleDMCTreeevt& tree, int idx) {
    return PassD0MCGen(tree, idx, D0MCGenCuts());
}

template <typename TreeT>
inline bool PassDStarReco(const TreeT& tree, int idx, const DStarRecoCuts& cuts) {
    return passLowerBound(tree.pT[idx], cuts.minCandPt) &&
           passUpperBound(tree.pT[idx], cuts.maxCandPt) &&
           passUpperBound(std::fabs(tree.y[idx]), cuts.maxAbsY) &&
           passUpperBound(std::fabs(tree.EtaD1[idx]), cuts.maxAbsDauEta) &&
           passUpperBound(std::fabs(tree.EtaD2[idx]), cuts.maxAbsDauEta) &&
           passLowerBound(tree.pTGrandD1[idx], cuts.minGrandDauPt) &&
           passLowerBound(tree.pTGrandD2[idx], cuts.minGrandDauPt) &&
           passUpperBound(std::fabs(tree.EtaGrandD1[idx]), cuts.maxAbsGrandDauEta) &&
           passUpperBound(std::fabs(tree.EtaGrandD2[idx]), cuts.maxAbsGrandDauEta) &&
           passLowerBound(tree.mva[idx], cuts.minMva);  // Added MVA cut
}

inline bool PassDStarData(const DataFormat::simpleDStarDataTreeevt& tree, int idx, const DStarRecoCuts& cuts) {
    return PassDStarReco(tree, idx, cuts);
}

inline bool PassDStarData(const DataFormat::simpleDStarDataTreeevt& tree, int idx) {
    return PassDStarReco(tree, idx, DStarDataRecoCuts());
}

inline bool PassDStarMC(const DataFormat::simpleDStarMCTreeevt& tree, int idx, const DStarRecoCuts& cuts) {
    return PassDStarReco(tree, idx, cuts);
}

inline bool PassDStarMC(const DataFormat::simpleDStarMCTreeevt& tree, int idx) {
    return PassDStarReco(tree, idx, DStarMCRecoCuts());
}

inline bool PassDStarMCGen(const DataFormat::simpleDStarMCTreeevt& tree, int idx, const DStarGenCuts& cuts) {
    return passLowerBound(tree.gen_pT[idx], cuts.minCandPt) &&
           passUpperBound(tree.gen_pT[idx], cuts.maxCandPt) &&
           passUpperBound(std::fabs(tree.gen_y[idx]), cuts.maxAbsY) &&
           passLowerBound(tree.gen_D0Dau1_pT[idx], cuts.minGrandDauPt) &&
           passLowerBound(tree.gen_D0Dau2_pT[idx], cuts.minGrandDauPt) &&
           passUpperBound(std::fabs(tree.gen_D0Dau1_eta[idx]), cuts.maxAbsGrandDauEta) &&
           passUpperBound(std::fabs(tree.gen_D0Dau2_eta[idx]), cuts.maxAbsGrandDauEta) &&
           passLowerBound(tree.gen_D1pT[idx], cuts.minSoftPionPt);
}

inline bool PassDStarMCGen(const DataFormat::simpleDStarMCTreeevt& tree, int idx) {
    return PassDStarMCGen(tree, idx, DStarMCGenCuts());
}

} // namespace CandidateSelection

