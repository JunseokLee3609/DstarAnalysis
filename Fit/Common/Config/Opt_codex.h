#ifndef OPT_H
#define OPT_H
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include "Params.h"
#include "FitCommonConfig.h"

// 공통 범용 유틸: 키네마틱 구간 정보
struct KinematicBin {
    double pTMin = 0.0;
    double pTMax = 0.0;
    double cosMin = -2.0;
    double cosMax = 2.0;
    double centralityMin = 0.0;
    double centralityMax = 100.0;

    KinematicBin() = default;
    KinematicBin(double ptMin, double ptMax, double cosMinIn, double cosMaxIn,
                 double centMinIn = 0.0, double centMaxIn = 100.0)
        : pTMin(ptMin), pTMax(ptMax), cosMin(cosMinIn), cosMax(cosMaxIn),
          centralityMin(centMinIn), centralityMax(centMaxIn) {}

    std::string GetBinName() const;

    bool HasCosRestriction(double fullCosMin = -2.0, double fullCosMax = 2.0) const {
        return cosMin > fullCosMin || cosMax < fullCosMax;
    }

    bool HasCentralityRestriction(double fullCentMin = 0.0, double fullCentMax = 100.0) const {
        return centralityMin > fullCentMin || centralityMax < fullCentMax;
    }
};

namespace detail {
inline std::string SanitizeLabel(double value, int precision = 3) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    std::string str = oss.str();
    while (!str.empty() && str.back() == '0') {
        str.pop_back();
    }
    if (!str.empty() && str.back() == '.') {
        str.pop_back();
    }
    if (str.empty()) {
        str = "0";
    }
    std::replace(str.begin(), str.end(), '-', 'm');
    std::replace(str.begin(), str.end(), '.', 'p');
    return str;
}
} // namespace detail

inline std::string BuildBinName(const KinematicBin& bin,
                                const std::string& ptPrefix = "pT",
                                const std::string& cosPrefix = "cos",
                                const std::string& centPrefix = "cent") {
    std::string name = ptPrefix + "_" + detail::SanitizeLabel(bin.pTMin) + "_" + detail::SanitizeLabel(bin.pTMax) +
                       "_" + cosPrefix + "_" + detail::SanitizeLabel(bin.cosMin) + "_" + detail::SanitizeLabel(bin.cosMax);
    if (bin.HasCentralityRestriction()) {
        name += "_" + centPrefix + "_" + detail::SanitizeLabel(bin.centralityMin) + "_" + detail::SanitizeLabel(bin.centralityMax);
    }
    return name;
}

inline std::string KinematicBin::GetBinName() const {
    return BuildBinName(*this);
}

struct FitOpt : public FitCommonSettings {
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
    std::string mvaLegend;
    std::string ELabel;
    std::string pdfName = "total_pdf";

    
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
    double massMin = 0.0;
    double massMax = 10.0;
    bool useDeltaMass = false;
    double deltaMassMin = 0.0;
    double deltaMassMax = 1.0;
    std::string deltaMassVar = "deltaMass";
    
    // === Kinematic Cuts ===
    double pTMin = 0.0;
    double pTMax = 100.0;
    double etaMin = -1.0;
    double etaMax = 1.0;
    double cosMin = -1.0;
    double cosMax = 1.0;
    double centMin = 0;
    double centMax = 100;
    double mvaMin = 0.0;
    double dcaMin = 0.0;
    double dcaMax = 1.0;
    bool drawDcaLegend = true;
    bool isPP = false;
    bool drawMvaLegend = isPP ? false : true; // ppRef에서는 MVA 범례를 기본적으로 그리지 않음
    
    // === Binning (optional) ===
    std::vector<double> ptBins;
    std::vector<double> etaBins;
    std::vector<double> centBins;
    std::vector<double> dcaBins = {0,0.0012,0.0023,0.0039,0.0059,0.0085,0.0160,0.0281,0.0476,0.07};
    
    // === Fitting Options (unified with FitConfig) ===
    bool doFit = true;
    
    // === Output Options ===
    std::string outputPlotDir = "plots/";
    std::string outputMCDir = "roots/MC/";    
    bool savePlots = true;           
    bool saveWorkspace = true;       
    std::string outputMCFile;
    std::string outputMCSwap0File;
    std::string outputMCSwap1File;
    std::string outputDCAFile;

    struct ConstraintConfig {
        struct SidebandWindow {
            double lowMin = 0.0;
            double lowMax = 0.0;
            double highMin = 0.0;
            double highMax = 0.0;
            double sigmaScale = 2.0;

            bool IsConfigured() const {
                return (lowMin < lowMax) && (highMin < highMax);
            }
        };

        std::vector<std::string> parameters;
        std::string mcResultFile;
        double gaussianSigmaScale = 1.0;
        double signalSigmaScale = 1.0;
        double backgroundSigmaScale = 2.0;
        SidebandWindow sideband;

        bool HasGaussianParameters() const { return !parameters.empty(); }
        bool HasMCResultFile() const { return !mcResultFile.empty(); }
        bool HasSidebandWindow() const { return sideband.IsConfigured(); }
    } constraint;

    // === Advanced fitting controls (legacy FitConfig fields) ===
    int strategyLevel = 2;               // RooFit strategy level: start with 2, then 1 -> 0 on fallback
    double rangeMin = 0.0;
    double rangeMax = 0.0;
    std::string rangeName = "analysis";

    double parameterExpansionFactor = 9.0;
    double limitCheckFactor = 3.0;
    bool enableParameterAdjustment = true;

    std::vector<std::string> skipUpperLimitAdjustment {"frac", "nsig", "nbkg", "alpha", "m0", "mean"};
    std::vector<std::string> skipLowerLimitAdjustment {"sigma", "frac", "nsig", "nbkg", "m0", "mean", "alpha"};
    std::vector<std::string> allowUpperExpansion {"n", "p0"};
    std::vector<std::string> allowLowerExpansion {"p0"};

    // === Constructors ===
    FitOpt() :
        name("default"),
        datasetName("dataset"),
        outputFile("fit_output.root"),
        fitResultName("fitResult"),
        massVar("mass"),
        plotName("fit_plot"),
        plotMCName("fit_plot_MC"),
        wsName("workspace_default"),
        outputDir("results/"),
        subDir(""),
        pTLegend(""),
        yLegend(""),
        centLegend(""),
        cosLegend(""),
        dcaLegend(""),
        ELabel(""),
        pdfName("total_pdf")
    {
        fitMethod = FitMethod::BinnedNLL;
        useCUDA = true;
        histogramBins = 50;
        strategy = "Minuit";
        minimizer = "Minimizer";
    }

    explicit FitOpt(const FitCommonSettings& settings) : FitOpt() {
        settings.CopyCommonSettingsTo(*this);
    }

    // Utility to set workspace name based on analysis type
    void SetWorkspaceName(const std::string& analysisType) {
        wsName = "workspace_" + analysisType;
    }
    
    // === Utility Methods ===
    
    // Convert to FitConfig for modern fitters
    FitOpt ToFitConfig() const {
        FitOpt config(*this);
        config.rangeMin = massMin;
        config.rangeMax = massMax;
        return config;
    }

    FitCommonSettings ToCommonSettings() const {
        FitCommonSettings common;
        CopyCommonSettingsTo(common);
        return common;
    }
    
    // Generate complete cut expression
    std::string GetCutString(bool useAbsCos = false) const {
        std::string cuts = BuildKinematicCut(GetKinematicBin(), useAbsCos);
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
        return BuildBinName(GetKinematicBin());
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
        
        if (drawMvaLegend) {
            mvaLegend = Form("MVA > %.3f", mvaMin);
        }
        if (!drawDcaLegend) {
            dcaLegend.clear();
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
    KinematicBin GetKinematicBin() const {
        return KinematicBin{pTMin, pTMax, cosMin, cosMax, centMin, centMax};
    }

public:
    void ApplyKinematicBin(const KinematicBin& bin) {
        pTMin = bin.pTMin;
        pTMax = bin.pTMax;
        cosMin = bin.cosMin;
        cosMax = bin.cosMax;
        centMin = bin.centralityMin;
        centMax = bin.centralityMax;
    }

    std::string BuildKinematicCut(const KinematicBin& bin, bool useAbsCos = false) const {
        std::ostringstream cuts;
        cuts << ptVar << " > " << bin.pTMin << " && " << ptVar << " < " << bin.pTMax;

        if (bin.HasCosRestriction()) {
            std::string cosExpr = useAbsCos ? "abs(" + cosVar + ")" : cosVar;
            cuts << " && " << cosExpr << " > " << bin.cosMin
                 << " && " << cosExpr << " < " << bin.cosMax;
        }

        if (bin.HasCentralityRestriction()) {
            cuts << " && " << centVar << " >= " << bin.centralityMin
                 << " && " << centVar << " < " << bin.centralityMax;
        }

        return cuts.str();
    }
};

#endif // OPT_H
