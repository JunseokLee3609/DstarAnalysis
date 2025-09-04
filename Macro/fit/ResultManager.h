#ifndef RESULT_MANAGER_H
#define RESULT_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "RooFitResult.h"
#include "RooWorkspace.h"
#include "RooAbsPdf.h"
#include "RooDataSet.h"
#include "RooPlot.h"
#include "RooRealVar.h"
#include "RooAbsReal.h"
#include "TCanvas.h"
#include "TFile.h"

struct FitResults {
    std::unique_ptr<RooFitResult> fitResult;
    std::unique_ptr<RooWorkspace> workspace;
    std::map<std::string, double> yields;
    std::map<std::string, double> yieldErrors;
    double chiSquare = 0.0;
    double ndf = 0.0;
    double reducedChiSquare = 0.0;
    std::string fitType;
    std::string timestamp;
};

struct PlotOptions {
    bool drawData = true;
    bool drawFit = true;
    bool drawComponents = true;
    bool drawResiduals = true;
    bool drawPulls = true;
    int nbins = 100;
    std::string title = "";
    std::string xAxisTitle = "Mass (GeV/c^{2})";
    std::string yAxisTitle = "Events";
    bool logScale = false;
    std::vector<std::string> legendLabels;
};

class ResultManager {
public:
    ResultManager(const std::string& outputPath = "");
    ~ResultManager() = default;
    
    // Result storage and management
    void StoreResult(const std::string& name, std::unique_ptr<RooFitResult> fitResult, 
                     std::unique_ptr<RooWorkspace> workspace, const std::string& fitType = "");
    void StoreYields(const std::string& name, const std::map<std::string, RooRealVar*>& yieldVars);
    // New: store yields from generic RooAbsReal expressions, with error propagation from fit result
    void StoreYieldsFromAbsReal(const std::string& name, const std::map<std::string, RooAbsReal*>& yieldExprs);
    void CalculateChiSquare(const std::string& name, RooAbsPdf* pdf, RooDataSet* data, 
                           RooRealVar* massVar, int nbins = 100);
    
    // Result access
    FitResults* GetResult(const std::string& name);
    const FitResults* GetResult(const std::string& name) const;
    std::vector<std::string> GetResultNames() const;
    bool HasResult(const std::string& name) const;
    
    // Yield extraction
    double GetSignalYield(const std::string& name) const;
    double GetSignalYieldError(const std::string& name) const;
    double GetBackgroundYield(const std::string& name) const;
    double GetBackgroundYieldError(const std::string& name) const;
    double GetTotalYield(const std::string& name) const;
    
    // Quality metrics
    double GetChiSquare(const std::string& name) const;
    double GetNDF(const std::string& name) const;
    double GetReducedChiSquare(const std::string& name) const;
    bool IsGoodFit(const std::string& name, double maxChiSquare = 5.0) const;
    
    // Plotting
    std::unique_ptr<RooPlot> CreatePlot(const std::string& name, RooRealVar* massVar, 
                                        RooAbsPdf* pdf, RooDataSet* data, 
                                        const PlotOptions& options = PlotOptions{});
    std::unique_ptr<TCanvas> CreateCanvas(const std::string& name, RooRealVar* massVar,
                                          RooAbsPdf* pdf, RooDataSet* data,
                                          const PlotOptions& options = PlotOptions{});
    
    // File I/O
    void SaveResults(const std::string& fileName, bool saveWorkspaces = true);
    void SaveResult(const std::string& name, const std::string& fileName, bool saveWorkspace = true);
    void LoadResults(const std::string& fileName);
    
    // Comparison and analysis
    void CompareResults(const std::vector<std::string>& resultNames, const std::string& outputFile);
    void PrintSummary(const std::string& name = "") const;
    void ExportToText(const std::string& fileName) const;
    void ExportToJSON(const std::string& fileName) const;
    
    // Workspace management
    RooWorkspace* GetWorkspace(const std::string& name) const;
    void MergeWorkspaces(const std::vector<std::string>& names, const std::string& mergedName);
    
    // Utilities
    void Clear();
    void RemoveResult(const std::string& name);
    void SetOutputPath(const std::string& path) { outputPath_ = path; }
    std::string GetOutputPath() const { return outputPath_; }
    
private:
    std::map<std::string, std::unique_ptr<FitResults>> results_;
    std::string outputPath_;
    
    // Helper methods
    std::string GetTimestamp() const;
    void ExtractYieldFromVar(FitResults* result, const std::string& yieldName, RooRealVar* var);
    void StylePlot(RooPlot* plot, const PlotOptions& options);
    void AddResidualPad(TCanvas* canvas, RooPlot* mainPlot, RooRealVar* massVar, 
                       RooAbsPdf* pdf, RooDataSet* data);
};

// Implementation
inline ResultManager::ResultManager(const std::string& outputPath) : outputPath_(outputPath) {}

inline void ResultManager::StoreResult(const std::string& name, std::unique_ptr<RooFitResult> fitResult,
                                       std::unique_ptr<RooWorkspace> workspace, const std::string& fitType) {
    auto result = std::make_unique<FitResults>();
    result->fitResult = std::move(fitResult);
    result->workspace = std::move(workspace);
    result->fitType = fitType;
    result->timestamp = GetTimestamp();
    
    results_[name] = std::move(result);
}

inline void ResultManager::StoreYields(const std::string& name, const std::map<std::string, RooRealVar*>& yieldVars) {
    auto it = results_.find(name);
    if (it == results_.end()) return;
    
    for (const auto& [yieldName, var] : yieldVars) {
        ExtractYieldFromVar(it->second.get(), yieldName, var);
    }
}

inline void ResultManager::StoreYieldsFromAbsReal(const std::string& name, const std::map<std::string, RooAbsReal*>& yieldExprs) {
    auto it = results_.find(name);
    if (it == results_.end()) return;

    RooFitResult* fr = it->second->fitResult.get();
    for (const auto& kv : yieldExprs) {
        const std::string& yname = kv.first;
        RooAbsReal* expr = kv.second;
        if (!expr) continue;
        double val = expr->getVal();
        double err = 0.0;
        if (fr) {
            err = expr->getPropagatedError(*fr);
        }
        it->second->yields[yname] = val;
        it->second->yieldErrors[yname] = err;
    }
}

inline void ResultManager::CalculateChiSquare(const std::string& name, RooAbsPdf* pdf, RooDataSet* data,
                                              RooRealVar* massVar, int nbins) {
    auto it = results_.find(name);
    if (it == results_.end() || !pdf || !data || !massVar) return;
    
    auto plot = std::unique_ptr<RooPlot>(massVar->frame(nbins));
    data->plotOn(plot.get());
    pdf->plotOn(plot.get());
    
    double chi2 = plot->chiSquare();
    int ndf = nbins - it->second->fitResult->floatParsFinal().getSize();
    
    it->second->chiSquare = chi2 * ndf;  // Convert to actual chi2
    it->second->ndf = ndf;
    it->second->reducedChiSquare = chi2;
}

inline FitResults* ResultManager::GetResult(const std::string& name) {
    auto it = results_.find(name);
    return (it != results_.end()) ? it->second.get() : nullptr;
}

inline const FitResults* ResultManager::GetResult(const std::string& name) const {
    auto it = results_.find(name);
    return (it != results_.end()) ? it->second.get() : nullptr;
}

inline std::vector<std::string> ResultManager::GetResultNames() const {
    std::vector<std::string> names;
    for (const auto& [name, result] : results_) {
        names.push_back(name);
    }
    return names;
}

inline bool ResultManager::HasResult(const std::string& name) const {
    return results_.find(name) != results_.end();
}

inline double ResultManager::GetSignalYield(const std::string& name) const {
    const auto* result = GetResult(name);
    if (!result) return 0.0;
    
    auto it = result->yields.find("nsig");
    return (it != result->yields.end()) ? it->second : 0.0;
}

inline double ResultManager::GetSignalYieldError(const std::string& name) const {
    const auto* result = GetResult(name);
    if (!result) return 0.0;
    
    auto it = result->yieldErrors.find("nsig");
    return (it != result->yieldErrors.end()) ? it->second : 0.0;
}

inline double ResultManager::GetBackgroundYield(const std::string& name) const {
    const auto* result = GetResult(name);
    if (!result) return 0.0;
    
    auto it = result->yields.find("nbkg");
    return (it != result->yields.end()) ? it->second : 0.0;
}

inline double ResultManager::GetBackgroundYieldError(const std::string& name) const {
    const auto* result = GetResult(name);
    if (!result) return 0.0;
    
    auto it = result->yieldErrors.find("nbkg");
    return (it != result->yieldErrors.end()) ? it->second : 0.0;
}

inline double ResultManager::GetTotalYield(const std::string& name) const {
    return GetSignalYield(name) + GetBackgroundYield(name);
}

inline double ResultManager::GetChiSquare(const std::string& name) const {
    const auto* result = GetResult(name);
    return result ? result->chiSquare : 0.0;
}

inline double ResultManager::GetNDF(const std::string& name) const {
    const auto* result = GetResult(name);
    return result ? result->ndf : 0.0;
}

inline double ResultManager::GetReducedChiSquare(const std::string& name) const {
    const auto* result = GetResult(name);
    return result ? result->reducedChiSquare : 0.0;
}

inline bool ResultManager::IsGoodFit(const std::string& name, double maxChiSquare) const {
    const auto* result = GetResult(name);
    if (!result || !result->fitResult) return false;
    
    return (result->fitResult->status() == 0) && 
           (result->reducedChiSquare > 0) && 
           (result->reducedChiSquare < maxChiSquare);
}

inline std::unique_ptr<RooPlot> ResultManager::CreatePlot(const std::string& name, RooRealVar* massVar,
                                                          RooAbsPdf* pdf, RooDataSet* data,
                                                          const PlotOptions& options) {
    if (!massVar || !pdf || !data) return nullptr;
    
    auto plot = std::unique_ptr<RooPlot>(massVar->frame(options.nbins));
    
    if (options.drawData) {
        data->plotOn(plot.get());
    }
    
    if (options.drawFit) {
        pdf->plotOn(plot.get());
        
        if (options.drawComponents) {
            // Draw individual components with different line styles
            pdf->plotOn(plot.get(), RooFit::Components("signal*"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
            pdf->plotOn(plot.get(), RooFit::Components("background*"), RooFit::LineColor(kBlue), RooFit::LineStyle(kDotted));
        }
    }
    
    StylePlot(plot.get(), options);
    
    return plot;
}

inline std::unique_ptr<TCanvas> ResultManager::CreateCanvas(const std::string& name, RooRealVar* massVar,
                                                            RooAbsPdf* pdf, RooDataSet* data,
                                                            const PlotOptions& options) {
    auto canvas = std::make_unique<TCanvas>((name + "_canvas").c_str(), options.title.c_str(), 800, 600);
    
    if (options.drawResiduals || options.drawPulls) {
        canvas->Divide(1, 2);
        canvas->cd(1);
    }
    
    auto plot = CreatePlot(name, massVar, pdf, data, options);
    if (plot) {
        plot->Draw();
        
        if (options.drawResiduals || options.drawPulls) {
            AddResidualPad(canvas.get(), plot.get(), massVar, pdf, data);
        }
    }
    
    return canvas;
}

inline void ResultManager::SaveResults(const std::string& fileName, bool saveWorkspaces) {
    std::string fullPath = outputPath_.empty() ? fileName : outputPath_ + "/" + fileName;
    auto file = std::unique_ptr<TFile>(TFile::Open(fullPath.c_str(), "RECREATE"));
    
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot create file " << fullPath << std::endl;
        return;
    }
    
    for (const auto& [name, result] : results_) {
        if (result->fitResult) {
            result->fitResult->Write((name + "_fitResult").c_str());
        }
        
        if (saveWorkspaces && result->workspace) {
            result->workspace->Write((name + "_workspace").c_str());
        }
    }
    
    file->Close();
    std::cout << "Results saved to " << fullPath << std::endl;
}

inline void ResultManager::SaveResult(const std::string& name, const std::string& fileName, bool saveWorkspace) {
    const auto* result = GetResult(name);
    if (!result) return;
    
    std::string fullPath = outputPath_.empty() ? fileName : outputPath_ + "/" + fileName;
    auto file = std::unique_ptr<TFile>(TFile::Open(fullPath.c_str(), "RECREATE"));
    
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot create file " << fullPath << std::endl;
        return;
    }
    
    if (result->fitResult) {
        result->fitResult->Write("fitResult");
    }
    
    if (saveWorkspace && result->workspace) {
        result->workspace->Write("workspace");
    }
    
    file->Close();
}

inline RooWorkspace* ResultManager::GetWorkspace(const std::string& name) const {
    const auto* result = GetResult(name);
    return result ? result->workspace.get() : nullptr;
}

inline void ResultManager::Clear() {
    results_.clear();
}

inline void ResultManager::RemoveResult(const std::string& name) {
    results_.erase(name);
}

inline std::string ResultManager::GetTimestamp() const {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &tm);
    return std::string(buffer);
}

inline void ResultManager::ExtractYieldFromVar(FitResults* result, const std::string& yieldName, RooRealVar* var) {
    if (!result || !var) return;
    
    result->yields[yieldName] = var->getVal();
    result->yieldErrors[yieldName] = var->getError();
}

inline void ResultManager::StylePlot(RooPlot* plot, const PlotOptions& options) {
    if (!plot) return;
    
    plot->SetTitle(options.title.c_str());
    plot->GetXaxis()->SetTitle(options.xAxisTitle.c_str());
    plot->GetYaxis()->SetTitle(options.yAxisTitle.c_str());
    
    if (options.logScale) {
        gPad->SetLogy();
    }
}

inline void ResultManager::PrintSummary(const std::string& name) const {
    if (name.empty()) {
        // Print summary for all results
        for (const auto& [resultName, result] : results_) {
            std::cout << "\n=== " << resultName << " ===" << std::endl;
            PrintSummary(resultName);
        }
        return;
    }
    
    const auto* result = GetResult(name);
    if (!result) {
        std::cout << "No result found for: " << name << std::endl;
        return;
    }
    
    std::cout << "Fit Type: " << result->fitType << std::endl;
    std::cout << "Timestamp: " << result->timestamp << std::endl;
    
    if (result->fitResult) {
        std::cout << "Fit Status: " << result->fitResult->status() << std::endl;
        std::cout << "EDM: " << result->fitResult->edm() << std::endl;
    }
    
    std::cout << "Signal Yield: " << GetSignalYield(name) << " ± " << GetSignalYieldError(name) << std::endl;
    std::cout << "Background Yield: " << GetBackgroundYield(name) << " ± " << GetBackgroundYieldError(name) << std::endl;
    std::cout << "Total Yield: " << GetTotalYield(name) << std::endl;
    std::cout << "Chi2/NDF: " << GetReducedChiSquare(name) << " (" << GetChiSquare(name) << "/" << GetNDF(name) << ")" << std::endl;
    std::cout << "Good Fit: " << (IsGoodFit(name) ? "Yes" : "No") << std::endl;
}

#endif // RESULT_MANAGER_H
