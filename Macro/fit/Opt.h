#ifndef OPT_H
#define OPT_H
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include "Params.h"
#include "../Tools/GlobalCuts.h"
#include "FitStrategy.h"  // For FitMethod enum

// 설정 관련 예외 클래스
class ConfigException : public std::exception {
private:
    std::string message_;
public:
    explicit ConfigException(const std::string& msg) : message_(msg) {}
    const char* what() const noexcept override { return message_.c_str(); }
}; 

struct FitOpt {
    // === Core Identity ===
    std::string name;               
    
    // === Data & Files ===
    std::string datasetName;        
    std::string cutExpr;            
    std::string cutMCExpr;          
    std::string outputFile;
    std::string outputDir;
    std::string subDir;
    
    // === Plot Settings & Legends ===
    std::string plotName;
    std::string plotMCName;
    std::string fitResultName;
    std::string wsName;
    std::string pTLegend;
    std::string yLegend;
    std::string centLegend;
    std::string cosLegend;
    std::string dcaLegend;
    std::string ELabel;

    
    // === Variable Names ===
    std::string massVar;            
    std::string ptVar = "pT";              
    std::string etaVar = "eta";             
    std::string yVar = "y";
    std::string centVar = "Centrality";            
    std::string mvaVar = "mva";            
    std::string cosVar = "cosThetaHX";            
    std::string dcaVar = "dca";
    
    // === Mass Range ===
    double massMin = 0.139;
    double massMax = 0.150;
    bool useDeltaMass = false;
    double deltaMassMin = 0.0;
    double deltaMassMax = 1.0;
    std::string deltaMassVar = "deltaMass";
    
    // === Kinematic Cuts ===
    double pTMin = 10.0;
    double pTMax = 100.0;
    double etaMin = -2.4;
    double etaMax = 2.4;
    double cosMin = -2.0;
    double cosMax = 2.0;
    double centMin = 0;
    double centMax = 100;
    double mvaMin = 0.99;
    double dcaMin = 0.0;
    double dcaMax = 0.1;
    
    // === Binning (optional) ===
    std::vector<double> ptBins;
    std::vector<double> etaBins;
    std::vector<double> centBins;
    std::vector<double> dcaBins = {0,0.0012,0.0023,0.0039,0.0059,0.0085,0.0160,0.0281,0.0476,0.07};
    
    // === Fitting Options (unified with FitConfig) ===
    FitMethod fitMethod = FitMethod::BinnedNLL;
    bool useMinos = false;             
    bool useHesse = true;            
    bool verbose = false;             
    bool useCUDA = true;
    bool doFit = true;
    int numCPU = 24;
    int histogramBins = 50;  // For binned fits
    int maxRetries = 3;
    std::string strategy = "Minuit";     // Minimizer type: "Minuit", "Minuit2", "GSLMultiMin"
    std::string minimizer = "Minimizer";    // Algorithm: "Migrad", "Simplex", "Combined"
    
    // === Output Options ===
    std::string outputPlotDir = "plots/";
    std::string outputMCDir = "roots/MC/";    
    bool savePlots = true;           
    bool saveWorkspace = true;       

    // === Constructors ===
    FitOpt() : 
        name("default"),
        datasetName("datasetHX"),
        outputFile("DStar_fit.root"),
        fitResultName("fitResult"),
        massVar("massPion"),
        plotName("fit_plot"),
        plotMCName("fit_plot_MC"),
        wsName("workspace_default"),  // Will be updated by preset methods
        outputDir("results/"),
        subDir(""),
        pTLegend(""),
        yLegend(""),
        centLegend(""),
        cosLegend(""),
        dcaLegend(""),
        ELabel("")
    {}
    
    // Utility to set workspace name based on analysis type
    void SetWorkspaceName(const std::string& analysisType) {
        wsName = "workspace_" + analysisType;
    }
    
    // === Utility Methods ===
    
    // Convert to FitConfig for modern fitters
    FitConfig ToFitConfig() const {
        FitConfig config;
        config.fitMethod = fitMethod;
        config.useMinos = useMinos;
        config.useHesse = useHesse;
        config.useCUDA = useCUDA;
        config.verbose = verbose;
        config.numCPU = numCPU;
        config.histogramBins = histogramBins;
        config.maxRetries = maxRetries;
        config.strategy = strategy;
        config.minimizer = minimizer;
        config.rangeMin = massMin;
        config.rangeMax = massMax;
        return config;
    }
    
    // Generate complete cut expression
    std::string GetCutString() const {
        std::string cuts = ptVar + " > " + std::to_string(pTMin) + " && " + ptVar + " < " + std::to_string(pTMax);
        if (cosMin > -2.0 || cosMax < 2.0) {
            cuts += " && " + cosVar + " > " + std::to_string(cosMin) + " && " + cosVar + " < " + std::to_string(cosMax);
        }
        if (mvaMin > 0.0) {
            cuts += " && " + mvaVar + " > " + std::to_string(mvaMin);
        }
        if (!cutExpr.empty()) {
            cuts += " && " + cutExpr;
        }
        return cuts;
    }
    
    // Generate bin name for output
    std::string GetBinName() const {
        return "pT_" + std::to_string(int(pTMin)) + "_" + std::to_string(int(pTMax)) + 
               "_cos_" + std::to_string(cosMin).substr(0,4) + "_" + std::to_string(cosMax).substr(0,4);
    }
    
    // === Legend Generation ===
    void GenerateLegends() {
        // pT: 소수점 첫째 자리까지 (%.1f)
        pTLegend = Form("%.1f < p_{T} < %.1f GeV/c", pTMin, pTMax);
        
        if (cosMin > -2.0 || cosMax < 2.0) {
            // cos: 소수점 둘째 자리까지 (%.2f)
            cosLegend = Form("%.2f < cos#theta* < %.2f", cosMin, cosMax);
        } else {
            cosLegend = "Full cos#theta* range";
        }
        
        if (centMin > 0 || centMax < 100) {
            centLegend = Form("%.0f-%.0f%%", centMin, centMax);
        } else {
            centLegend = "0-100%";
        }
        
        if (mvaMin > 0.0) {
            dcaLegend = Form("MVA > %.3f", mvaMin);
        }
        
        if (yVar == "y") {
            // y: 소수점 첫째 자리까지 (%.1f)
            yLegend = Form("|y| < %.1f", etaMax);
        } else {
            // eta: 소수점 첫째 자리까지 (%.1f)
            yLegend = Form("%.1f < #eta < %.1f", etaMin, etaMax);
        }
    }
    
    // Get formatted plot name with legends
    std::string GetFormattedPlotName() const {
        return plotName + "_" + GetBinName() + ".pdf";
    }
    
    // === Preset Configurations ===
    
    void DStarDataDefault() {
        name = "DStar";
        fitMethod = FitMethod::BinnedNLL;
        massVar = "massPion";
        massMin = 0.140; massMax = 0.150;
        pTMin = 10.0; pTMax = 100.0;
        cosMin = -2.0; cosMax = 2.0;
        mvaMin = 0.99;
        histogramBins = 50;
        outputDir = "results/DStar_Data/";
        plotName = "Plot_DStar_Data";
        wsName = "workspace_DStar";  // D* specific workspace
        GenerateLegends();  // Auto-generate legends
    }
    
    void DStarMCDefault() {
        DStarDataDefault();
        fitMethod = FitMethod::Extended;
        outputDir = "results/DStar_MC/";
        plotName = "Plot_DStar_MC";
        wsName = "workspace_DStar";  // Keep same workspace naming
        cutExpr = cutMCExpr;  // Use MC cuts if different
        GenerateLegends();
    }
    
    // === Simple Validation ===
    bool IsValid() const {
        return (massMin < massMax) && (pTMin < pTMax) && 
               !outputFile.empty() && !datasetName.empty();
    }
    
    // === Legacy D0 configurations (simplified) ===
    void D0DataDefault() {
        name = "D0";
        massVar = "mass";
        massMin = 1.74; massMax = 1.97;
        cutExpr = ptVar + " > " + std::to_string(pTMin);
        outputDir = "results/D0_Data/";
        plotName = "Plot_D0_Data";
        wsName = "workspace_D0";  // D0 specific workspace
        outputFile = "D0_" + GetBinName() + ".root";
        GenerateLegends();
    }
    
    void D0MCDefault() {
        D0DataDefault();
        name = "D0";
        massMin = 1.75; massMax = 2.00;
        cutMCExpr = "y<1 && y>-1 && matchGEN==1";
        cutExpr = "y<1 && y>-1 && mva >= " + std::to_string(mvaMin);
        outputDir = "results/D0_MC/";
        plotName = "Plot_D0_MC";
        wsName = "workspace_D0";  // Keep same workspace naming
        datasetName = "dataset";
        GenerateLegends();
    }
    
private:
    // Helper for legacy filename format
    std::string convertDotToP(double value) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << value;
        std::string str = oss.str();
        std::replace(str.begin(), str.end(), '.', 'p');
        std::replace(str.begin(), str.end(), '-', 'm');
        return str;
    }
};

#endif // OPT_H
