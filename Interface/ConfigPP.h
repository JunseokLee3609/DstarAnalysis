#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>

struct SelectionCuts {
    // Daughter particle cuts
    static constexpr double PT_D1_MIN = 0;
    static constexpr double PT_D2_MIN = 0.4;
    static constexpr double ETA_D1_MAX = 2.4;
    static constexpr double ETA_D2_MAX = 2.4;
    
    // Grand-daughter particle cuts  
    static constexpr double PT_GRANDD1_MIN = 1.0;
    static constexpr double ETA_GRANDD1_MAX = 2.4;
    static constexpr double PT_GRANDD2_MIN = 1.0;
    static constexpr double ETA_GRANDD2_MAX = 2.4;
    
    // Parent particle cuts
    static constexpr double PT_PARENT_MIN = 1.0;
    static constexpr double PT_PARENT_MAX = 50.0;
    static constexpr double Y_PARENT_MAX = 1.0;
    
    // Track quality cuts
    static constexpr int NHIT_MIN = 10;
    static constexpr double PT_ERR_MAX = 0.1;
    static constexpr double ETA_DIFF_MAX = 1.0;
    static constexpr double PT_SUM_MIN = 1.6;
    static constexpr double POW = 3.0;
    
    
    // Directory configuration
    // Use header-only constexpr to avoid static initialization and ODR issues
    static constexpr const char* SUB_DIR = "DStar_ppRef_PU_NoCorrection_Sep20_pTGrand_1p0_EtaGrand_2p4_pTD2_0p4";
    
    // Helper functions to get common cut expressions
    static std::string getDaughterCuts() {
        return Form("pTD1>%0.1f && pTD2>%0.1f && abs(EtaD1)<%0.1f && abs(EtaD2)<%0.1f", 
                   PT_D1_MIN, PT_D2_MIN, ETA_D1_MAX, ETA_D2_MAX);
    }
    static std::string getSlowPionCuts(){
        return Form("pTD2>%0.1f && abs(EtaD2)<%0.1f && massDaugther1 > 1.82 && massDaugther1 < 1.92", 
                   PT_D2_MIN, ETA_D2_MAX);
    }
    
    static std::string getGrandDaughterCuts() {
        return Form("pTGrandD1>%0.1f && abs(EtaGrandD1)<%0.1f && pTGrandD2>%0.1f && abs(EtaGrandD2)<%0.1f", 
                   PT_GRANDD1_MIN, ETA_GRANDD1_MAX, PT_GRANDD2_MIN, ETA_GRANDD2_MAX);
    }
    static std::string getTrackQualityCuts() {
        return Form("nHitD1>%d && nHitD2>%d && pTErrD1<%0.2f && pTErrD2<%0.2f && etaDiffDau<%0.1f && pTSumDau>%0.1f", 
                   NHIT_MIN, NHIT_MIN, PT_ERR_MAX, PT_ERR_MAX, ETA_DIFF_MAX, PT_SUM_MIN);
    }
    
    // static std::string getAllCuts() {
    //     return getDaughterCuts() + " && " + getGrandDaughterCuts() + " && " + 
    //            getParentCuts() + " && " + getTrackQualityCuts();
    // }
};

// No out-of-line definition needed for constexpr const char*

#endif // CONFIGMANAGER_H
