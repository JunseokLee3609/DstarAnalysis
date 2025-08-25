#ifndef FIT_STRATEGY_H
#define FIT_STRATEGY_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "RooFitResult.h"
#include "RooAbsPdf.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooLinkedList.h"
#include "RooFit.h"
#include "RooRealVar.h"
#include "TH1F.h"
#include "TH1D.h"
#include "RooPlot.h"

enum class FitMethod {
    NLL,        // Negative Log Likelihood (unbinned)
    BinnedNLL,  // Binned Negative Log Likelihood  
    Extended    // Extended Maximum Likelihood
};

struct FitConfig {
    bool useMinos = false;
    bool useHesse = true;
    bool useCUDA = false;
    bool verbose = false;
    int numCPU = 24;
    int maxRetries = 3;
    std::string strategy = "Minuit";
    std::string minimizer = "Minimizer";
    double rangeMin = 0.0;
    double rangeMax = 0.0;
    std::string rangeName = "analysis";
    
    // Fit method selection
    FitMethod fitMethod = FitMethod::NLL;
    
    // Binned fit specific settings
    int histogramBins = 100;
    
    // Parameter adjustment settings
    double parameterExpansionFactor = 9.0;
    double limitCheckFactor = 3.0;
    bool enableParameterAdjustment = true;
    
    // Skip adjustment for specific parameters
    std::vector<std::string> skipUpperLimitAdjustment = {"frac", "nsig", "nbkg", "alpha", "m0", "mean"};
    std::vector<std::string> skipLowerLimitAdjustment = {"sigma", "frac", "nsig", "nbkg", "m0", "mean", "alpha"};
    std::vector<std::string> allowUpperExpansion = {"n", "p0"};
    std::vector<std::string> allowLowerExpansion = {"p0"};
};

class FitStrategy {
public:
    virtual ~FitStrategy() = default;
    virtual std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) = 0;
    virtual std::string GetName() const = 0;
    
protected:
    RooLinkedList CreateFitOptions(const FitConfig& config);
    bool ShouldSkipParameterAdjustment(const std::string& paramName, const std::vector<std::string>& skipList);
    void AdjustParameterLimits(RooFitResult* result, RooAbsPdf* pdf, const FitConfig& config);
};

class BasicFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) override;
    std::string GetName() const override { return "BasicFit"; }
};

class RobustFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) override;
    std::string GetName() const override { return "RobustFit"; }
    
private:
    std::unique_ptr<RooFitResult> PerformIterativeFit(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config);
    bool CheckFitConvergence(RooFitResult* result);
};

class MCFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) override;
    std::string GetName() const override { return "MCFit"; }
};

class ConstraintFitStrategy : public FitStrategy {
public:
    ConstraintFitStrategy(const std::string& mcFilePath, const std::vector<std::string>& constraintParams);
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) override;
    std::string GetName() const override { return "ConstraintFit"; }
    
private:
    std::string mcFilePath_;
    std::vector<std::string> constraintParameters_;
    void ApplyConstraints(RooAbsPdf* pdf);
};

class BinnedFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) override;
    std::string GetName() const override { return "BinnedFit"; }
    
private:
    std::unique_ptr<RooDataHist> CreateBinnedData(RooDataSet* data, RooRealVar* massVar, int nBins);
};

// Factory for creating fit strategies
class FitStrategyFactory {
public:
    enum class StrategyType {
        Basic,
        Robust, 
        MC,
        Constraint,
        Binned
    };
    
    static std::unique_ptr<FitStrategy> CreateStrategy(StrategyType type);
    static std::unique_ptr<FitStrategy> CreateConstraintStrategy(const std::string& mcFilePath, 
                                                                const std::vector<std::string>& constraintParams);
};

// Implementation
inline RooLinkedList FitStrategy::CreateFitOptions(const FitConfig& config) {
    RooLinkedList fitOpts;
    
    fitOpts.Add(new RooCmdArg(RooFit::NumCPU(config.numCPU)));
    fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(config.verbose ? 1 : -1)));
    fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Minimizer(config.strategy.c_str(), config.minimizer.c_str())));
    fitOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Optimize(1)));
    
    // Configure fit method specific options
    switch (config.fitMethod) {
        case FitMethod::NLL:
            // Standard unbinned negative log likelihood (default)
            fitOpts.Add(new RooCmdArg(RooFit::Extended(false)));
            break;
            
        case FitMethod::BinnedNLL:
            // Binned negative log likelihood fit
            fitOpts.Add(new RooCmdArg(RooFit::Extended(false)));
            // Note: Dataset should be converted to RooDataHist before fitting
            break;
            
        case FitMethod::Extended:
            // Extended maximum likelihood fit
            fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
            break;
    }
    
    if (config.useHesse) {
        fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
    }
    
    if (config.useMinos) {
        fitOpts.Add(new RooCmdArg(RooFit::Minos(true)));
    }
    
    if (!config.rangeName.empty()) {
        fitOpts.Add(new RooCmdArg(RooFit::Range(config.rangeName.c_str())));
    }
    
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    if (config.useCUDA) {
        fitOpts.Add(new RooCmdArg(RooFit::EvalBackend("cuda")));
    }
#endif
    
    return fitOpts;
}

inline bool FitStrategy::ShouldSkipParameterAdjustment(const std::string& paramName, const std::vector<std::string>& skipList) {
    for (const auto& skipParam : skipList) {
        if (paramName.find(skipParam) != std::string::npos) {
            return true;
        }
    }
    return false;
}

inline void FitStrategy::AdjustParameterLimits(RooFitResult* result, RooAbsPdf* pdf, const FitConfig& config) {
    if (!config.enableParameterAdjustment || !result) return;
    
    auto fpf = result->floatParsFinal();
    auto modelVars = pdf->getVariables();
    
    for (int idx = 0; idx < fpf.getSize(); ++idx) {
        auto var = dynamic_cast<RooRealVar*>(&fpf[idx]);
        if (!var) continue;
        
        std::string paramName = var->GetName();
        auto pdfVar = dynamic_cast<RooRealVar*>(modelVars->find(paramName.c_str()));
        if (!pdfVar) continue;
        
        double val = var->getVal();
        double upperLimit = var->getMax();
        double lowerLimit = var->getMin();
        double upperError = var->getErrorHi();
        double lowerError = var->getErrorLo();
        
        // Check upper limit adjustment
        if (val + config.limitCheckFactor * upperError > upperLimit || 
            (val > 0 && 1.2 * val > upperLimit) || 
            (val < 0 && 0.8 * val > upperLimit)) {
            
            if (!ShouldSkipParameterAdjustment(paramName, config.skipUpperLimitAdjustment)) {
                bool allowExpansion = false;
                for (const auto& allowParam : config.allowUpperExpansion) {
                    if (paramName.find(allowParam) != std::string::npos) {
                        allowExpansion = true;
                        break;
                    }
                }
                
                if (allowExpansion) {
                    if (paramName.find("n") != std::string::npos) {
                        pdfVar->setMax(pdfVar->getMax() * 2);
                    } else {
                        pdfVar->setMax(val + upperError * config.parameterExpansionFactor);
                    }
                }
            }
        }
        
        // Check lower limit adjustment
        if (val + config.limitCheckFactor * lowerError < lowerLimit || 
            (val > 0 && 0.8 * val < lowerLimit) || 
            (val < 0 && 1.2 * val < lowerLimit)) {
            
            if (!ShouldSkipParameterAdjustment(paramName, config.skipLowerLimitAdjustment)) {
                bool allowExpansion = false;
                for (const auto& allowParam : config.allowLowerExpansion) {
                    if (paramName.find(allowParam) != std::string::npos) {
                        allowExpansion = true;
                        break;
                    }
                }
                
                if (allowExpansion) {
                    pdfVar->setMin(val + lowerError * config.parameterExpansionFactor);
                }
            }
        }
    }
    
    delete modelVars;
}

inline std::unique_ptr<RooFitResult> BasicFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) {
    if (!pdf || !data) return nullptr;
    
    auto fitOpts = CreateFitOptions(config);
    auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
    
    return result;
}

inline std::unique_ptr<RooFitResult> RobustFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) {
    if (!pdf || !data) return nullptr;
    
    return PerformIterativeFit(pdf, data, config);
}

inline std::unique_ptr<RooFitResult> RobustFitStrategy::PerformIterativeFit(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) {
    std::unique_ptr<RooFitResult> currentResult;
    bool needRefit = true;
    int fitAttempts = 0;
    
    // Initial fit
    auto fitOpts = CreateFitOptions(config);
    currentResult = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
    
    while (needRefit && fitAttempts < config.maxRetries) {
        if (fitAttempts > 0) {
            currentResult->Print("v");
        }
        
        // Adjust parameter limits based on current result
        AdjustParameterLimits(currentResult.get(), pdf, config);
        
        // Retry fit with adjusted parameters
        auto retryOpts = CreateFitOptions(config);
        auto newResult = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, retryOpts));
        
        // Check for retry with different strategy
        int retryCount = 0;
        while (newResult->status() != 0 && retryCount < config.maxRetries) {
            FitConfig retryConfig = config;
            retryConfig.strategy = "Strategy(" + std::to_string(2 - retryCount) + ")";
            
            auto retryOpts2 = CreateFitOptions(retryConfig);
            newResult = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, retryOpts2));
            retryCount++;
        }
        
        currentResult = std::move(newResult);
        needRefit = !CheckFitConvergence(currentResult.get());
        fitAttempts++;
        
        if (fitAttempts >= config.maxRetries && needRefit) {
            std::cout << "Fit still fails after " << fitAttempts << " attempts. Using last result." << std::endl;
            break;
        }
    }
    
    return currentResult;
}

inline bool RobustFitStrategy::CheckFitConvergence(RooFitResult* result) {
    if (!result) return false;
    
    return (result->statusCodeHistory(0) == 0) && (result->statusCodeHistory(1) == 0);
}

inline std::unique_ptr<RooFitResult> MCFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) {
    if (!pdf || !data) return nullptr;
    
    // MC fitting typically uses simpler options
    FitConfig mcConfig = config;
    mcConfig.useMinos = false;  // Usually don't need Minos for MC
    
    auto fitOpts = CreateFitOptions(mcConfig);
    auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
    
    return result;
}

inline ConstraintFitStrategy::ConstraintFitStrategy(const std::string& mcFilePath, const std::vector<std::string>& constraintParams)
    : mcFilePath_(mcFilePath), constraintParameters_(constraintParams) {}

inline std::unique_ptr<RooFitResult> ConstraintFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) {
    if (!pdf || !data) return nullptr;
    
    // Apply constraints from MC fit results
    ApplyConstraints(pdf);
    
    auto fitOpts = CreateFitOptions(config);
    auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
    
    return result;
}

inline void ConstraintFitStrategy::ApplyConstraints(RooAbsPdf* pdf) {
    // Implementation would load MC results and apply constraints
    // This is a placeholder for the actual constraint application
    std::cout << "Applying constraints from " << mcFilePath_ << std::endl;
}

// BinnedFitStrategy implementation
inline std::unique_ptr<RooFitResult> BinnedFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) {
    if (!pdf || !data) return nullptr;
    
    // Get mass variable from the dataset
    auto massVar = dynamic_cast<RooRealVar*>(data->get()->find("mass"));
    if (!massVar) {
        // Try to find the first RooRealVar in the dataset
        auto argSet = data->get();
        for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
            if (auto* realVar = dynamic_cast<RooRealVar*>(var)) {
                massVar = realVar;
                break;
            }
        }
    }
    
    if (!massVar) {
        std::cerr << "BinnedFitStrategy: Could not find mass variable in dataset" << std::endl;
        return nullptr;
    }
    
    // Create binned data
    auto binnedData = CreateBinnedData(data, massVar, config.histogramBins);
    if (!binnedData) {
        std::cerr << "BinnedFitStrategy: Failed to create binned data" << std::endl;
        return nullptr;
    }
    
    // Create fit options for binned fit
    auto fitOpts = CreateFitOptions(config);
    
    // Perform binned fit
    auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*binnedData, fitOpts));
    
    return result;
}

inline std::unique_ptr<RooDataHist> BinnedFitStrategy::CreateBinnedData(RooDataSet* data, RooRealVar* massVar, int nBins) {
    if (!data || !massVar || nBins <= 0) return nullptr;
    
    // Create binning for the mass variable
    RooRealVar* binnedMassVar = dynamic_cast<RooRealVar*>(massVar->clone((std::string(massVar->GetName()) + "_binned").c_str()));
    if (!binnedMassVar) return nullptr;
    
    // Set uniform binning
    binnedMassVar->setBins(nBins);
    
    // Create RooDataHist from the dataset
    auto binnedData = std::make_unique<RooDataHist>(
        (std::string(data->GetName()) + "_binned").c_str(),
        (std::string(data->GetTitle()) + " (binned)").c_str(),
        RooArgSet(*binnedMassVar),
        *data
    );
    
    delete binnedMassVar; // Clean up the cloned variable
    
    return binnedData;
}

inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateStrategy(StrategyType type) {
    switch (type) {
        case StrategyType::Basic:
            return std::make_unique<BasicFitStrategy>();
        case StrategyType::Robust:
            return std::make_unique<RobustFitStrategy>();
        case StrategyType::MC:
            return std::make_unique<MCFitStrategy>();
        case StrategyType::Binned:
            return std::make_unique<BinnedFitStrategy>();
        default:
            return std::make_unique<BasicFitStrategy>();
    }
}

inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateConstraintStrategy(const std::string& mcFilePath, 
                                                                                const std::vector<std::string>& constraintParams) {
    return std::make_unique<ConstraintFitStrategy>(mcFilePath, constraintParams);
}

#endif // FIT_STRATEGY_H