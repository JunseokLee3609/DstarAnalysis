#ifndef GLOBALCUTS_H
#define GLOBALCUTS_H

#include <string>
#include <map>

struct GlobalCuts {
    static constexpr double PT_MIN = 4.0;
    static constexpr double PT_MAX = 100.0;
    static constexpr double ETA_MIN = -1.0;
    static constexpr double ETA_MAX = 1.0;
    static constexpr double Y_MIN = -1.0;
    static constexpr double Y_MAX = 1.0;
    static constexpr double COS_MIN = 0.0;
    static constexpr double COS_MAX = 1.0;
    static constexpr double CENT_MIN = 0.0;
    static constexpr double CENT_MAX = 100.0;
    static constexpr double MVA_MIN = 0.0;
    static constexpr double DCA_MIN = 0.0;
    static constexpr double DCA_MAX = 0.07;
    
    static const std::string SUB_DIR;
    
    static std::string getDataCutExpr() {
        return Form("abs(y)<%0.1f && pT>%0.1f && pT<%0.1f", Y_MAX, PT_MIN, PT_MAX);
    }
    
    static std::string getMCCutExpr() {
        return Form("abs(y)<%0.1f && pT>%0.1f && pT<%0.1f && matchGEN==1", Y_MAX, PT_MIN, PT_MAX);
    }
    
    static std::string getDStarCutExpr() {
        return Form("abs(y)<%0.1f && abs(cosThetaHX)>%0.2f && abs(cosThetaHX)<%0.2f && pT>%0.1f && pT<%0.1f", 
                   Y_MAX, COS_MIN, COS_MAX, PT_MIN, PT_MAX);
    }
    
    static std::string getDStarMCCutExpr() {
        return Form("abs(y)<%0.1f && abs(cosThetaHX)>%0.2f && abs(cosThetaHX)<%0.2f && pT>%0.1f && pT<%0.1f && matchGEN==1", 
                   Y_MAX, COS_MIN, COS_MAX, PT_MIN, PT_MAX);
    }
    
    static std::string getD0CutExpr() {
        return Form("abs(y)<%0.1f && Centrality>%0.1f && Centrality<%0.1f && pT>%0.1f && pT<%0.1f && mva>%0.2f", 
                   Y_MAX, CENT_MIN, CENT_MAX, PT_MIN, PT_MAX, MVA_MIN);
    }
    
    static std::string getD0MCCutExpr() {
        return Form("abs(y)<%0.1f && Centrality>%0.1f && Centrality<%0.1f && pT>%0.1f && pT<%0.1f && mva>%0.2f && matchGEN==1", 
                   Y_MAX, CENT_MIN, CENT_MAX, PT_MIN, PT_MAX, MVA_MIN);
    }
};

const std::string GlobalCuts::SUB_DIR = "";

#endif // GLOBALCUTS_H