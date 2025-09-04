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
#include "RooGaussian.h"
#include "RooArgSet.h"
#include "TFile.h"
#include "RooFit.h"
#include "RooRealVar.h"
#include "TH1F.h"
#include "TH1D.h"
#include "RooPlot.h"

enum class FitMethod {
    NLL,        // Negative Log Likelihood (unbinned)
    BinnedNLL,  // Binned Negative Log Likelihood  
    Extended,   // Extended Maximum Likelihood (no robust range expansion)
    Robust      // Robust Extended fit (iterative with range expansion)
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
    virtual std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config, RooRealVar* massVar) {
        return Execute(pdf, data, config);  // Default implementation ignores massVar
    }
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
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config, RooRealVar* massVar) override;
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
        Binned,
        GaussianConstraint
    };
    
    static std::unique_ptr<FitStrategy> CreateStrategy(StrategyType type);
    static std::unique_ptr<FitStrategy> CreateConstraintStrategy(const std::string& mcFilePath, 
                                                                const std::vector<std::string>& constraintParams);
    static std::unique_ptr<FitStrategy> CreateGaussianConstraintStrategy(
        RooDataSet* mcDataset,
        RooAbsPdf* mcSignalPdf,
        const std::vector<std::string>& paramNames,
        double sigmaScale = 1.0);
    // Map FitMethod to StrategyType explicitly
    static StrategyType GetStrategyTypeFromFitMethod(FitMethod method) {
        switch (method) {
            case FitMethod::NLL:       return StrategyType::Basic;
            case FitMethod::BinnedNLL: return StrategyType::Binned;
            case FitMethod::Extended:  return StrategyType::Basic;   // Extended-only (no robust widening)
            case FitMethod::Robust:    return StrategyType::Robust;  // Robust (iterative)
        }
        return StrategyType::Basic;
    }
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
        case FitMethod::Robust:
            // Robust uses extended likelihood as well
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

// GaussianConstraintStrategy implementation
class GaussianConstraintStrategy : public FitStrategy {
public:
    GaussianConstraintStrategy(RooDataSet* mcDataset,
                               RooAbsPdf* mcSignalPdf,
                               const std::vector<std::string>& paramNames,
                               double sigmaScale = 1.0)
        : mcDataset_(mcDataset), mcSignalPdf_(mcSignalPdf), paramNames_(paramNames), sigmaScale_(sigmaScale) {}

    // Alternate constructor: load constraints from saved MC fit result file
    GaussianConstraintStrategy(const std::string& mcResultFile,
                               const std::vector<std::string>& paramNames,
                               double sigmaScale = 1.0,
                               const std::string& fitResultObjName = "fitResult")
        : mcResultFile_(mcResultFile), paramNames_(paramNames), sigmaScale_(sigmaScale), fitResultObjName_(fitResultObjName) {}

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) override {
        return Execute(pdf, data, config, nullptr);
    }

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config, RooRealVar* massVar) override {
        if (!pdf || !data) return nullptr;

        // 1) Build constraints: either from saved MC fit or on-the-fly MC fit
        std::map<std::string, std::pair<double,double>> constraints;
        if (!mcResultFile_.empty()) {
            std::unique_ptr<TFile> fin(TFile::Open(mcResultFile_.c_str(), "READ"));
            if (!fin || fin->IsZombie()) {
                std::cerr << "GaussianConstraintStrategy: Cannot open file " << mcResultFile_ << std::endl;
                return nullptr;
            }
            auto* rf = dynamic_cast<RooFitResult*>(fin->Get(fitResultObjName_.c_str()));
            if (!rf) {
                std::cerr << "GaussianConstraintStrategy: RooFitResult '" << fitResultObjName_ << "' not found" << std::endl;
                return nullptr;
            }
            auto finals = rf->floatParsFinal();
            for (const auto& name : paramNames_) {
                RooAbsArg* arg = nullptr;
                for (int i=0; i<finals.getSize(); ++i) {
                    if (std::string(finals.at(i)->GetName()) == name) { arg = finals.at(i); break; }
                }
                if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                    double mu = v->getVal();
                    double err = v->getError();
                    if (err <= 0) {
                        double eh = std::abs(v->getErrorHi());
                        double el = std::abs(v->getErrorLo());
                        err = 0.5 * (eh + el);
                        if (err <= 0) err = 1e-6;
                    }
                    constraints[name] = {mu, std::max(err * sigmaScale_, 1e-6)};
                }
            }
        } else {
            if (!mcDataset_ || !mcSignalPdf_) return nullptr;
            FitConfig mcCfg = config;
            mcCfg.useMinos = false;
            auto mcOpts = CreateFitOptions(mcCfg);
            auto mcResult = std::unique_ptr<RooFitResult>(mcSignalPdf_->fitTo(*mcDataset_, mcOpts));
            if (!mcResult) return nullptr;

            std::unique_ptr<RooArgSet> mcVars(mcSignalPdf_->getVariables());
            for (const auto& name : paramNames_) {
                if (auto* v = dynamic_cast<RooRealVar*>(mcVars->find(name.c_str()))) {
                    double mu = v->getVal();
                    double err = v->getError();
                    if (err <= 0) {
                        double eh = std::abs(v->getErrorHi());
                        double el = std::abs(v->getErrorLo());
                        err = 0.5 * (eh + el);
                        if (err <= 0) err = 1e-6;
                    }
                    constraints[name] = {mu, std::max(err * sigmaScale_, 1e-6)};
                }
            }
        }

        // 2) Build RooGaussian external constraints on data parameters
        std::unique_ptr<RooArgSet> dataVars(pdf->getVariables());
        RooArgSet ext;
        std::vector<std::unique_ptr<RooRealVar>> keepMeans;
        std::vector<std::unique_ptr<RooRealVar>> keepSigmas;
        std::vector<std::unique_ptr<RooGaussian>> keepGauss;
        for (const auto& [pname, ms] : constraints) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(pname.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("mc_mean_" + pname).c_str(), ("mc_mean_" + pname).c_str(), ms.first);
                auto sigma = std::make_unique<RooRealVar>(("mc_sigma_" + pname).c_str(), ("mc_sigma_" + pname).c_str(), ms.second);
                auto gauss = std::make_unique<RooGaussian>(("constr_" + pname).c_str(), ("constr_" + pname).c_str(), *target, *mean, *sigma);
                ext.add(*gauss);
                keepMeans.push_back(std::move(mean));
                keepSigmas.push_back(std::move(sigma));
                keepGauss.push_back(std::move(gauss));
            }
        }

        // 3) Create fit options and add constraints
        auto opts = CreateFitOptions(config);
        if (ext.getSize() > 0) {
            opts.Add(new RooCmdArg(RooFit::ExternalConstraints(ext)));
        }

        // 4) Handle binned fit if requested
        if (config.fitMethod == FitMethod::BinnedNLL) {
            RooRealVar* m = massVar;
            if (!m) {
                auto argSet = data->get();
                for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
                    if (auto* rv = dynamic_cast<RooRealVar*>(var)) { m = rv; break; }
                }
            }
            if (!m) return nullptr;
            auto binnedVar = std::unique_ptr<RooRealVar>(dynamic_cast<RooRealVar*>(m->clone((std::string(m->GetName()) + "_binned").c_str())));
            if (!binnedVar) return nullptr;
            binnedVar->setBins(config.histogramBins);
            auto binnedData = std::make_unique<RooDataHist>((std::string(data->GetName()) + "_binned").c_str(),
                                                            (std::string(data->GetTitle()) + " (binned)").c_str(),
                                                            RooArgSet(*binnedVar), *data);
            return std::unique_ptr<RooFitResult>(pdf->fitTo(*binnedData, opts));
        }

        // 5) Unbinned fit
        return std::unique_ptr<RooFitResult>(pdf->fitTo(*data, opts));
    }

    std::string GetName() const override { return "GaussianConstraint"; }

private:
    RooDataSet* mcDataset_ = nullptr;
    RooAbsPdf* mcSignalPdf_ = nullptr;
    std::string mcResultFile_;
    std::vector<std::string> paramNames_;
    double sigmaScale_ = 1.0;
    std::string fitResultObjName_ = "fitResult";
};

// BinnedFitStrategy implementation
inline std::unique_ptr<RooFitResult> BinnedFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) {
    if (!pdf || !data) return nullptr;
    
    // Try to find the first RooRealVar in the dataset (fallback method)
    RooRealVar* massVar = nullptr;
    auto argSet = data->get();
    for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
        if (auto* realVar = dynamic_cast<RooRealVar*>(var)) {
            massVar = realVar;
            break;
        }
    }
    
    if (!massVar) {
        std::cerr << "BinnedFitStrategy: Could not find mass variable in dataset" << std::endl;
        return nullptr;
    }
    
    return Execute(pdf, data, config, massVar);
}

inline std::unique_ptr<RooFitResult> BinnedFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config, RooRealVar* massVar) {
    if (!pdf || !data || !massVar) return nullptr;
    
    // Create binned data using the provided mass variable
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
        case StrategyType::GaussianConstraint:
            // This variant needs extra inputs; use the dedicated factory helper instead
            return std::make_unique<BasicFitStrategy>();
        default:
            return std::make_unique<BasicFitStrategy>();
    }
}

inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateConstraintStrategy(const std::string& mcFilePath, 
                                                                                const std::vector<std::string>& constraintParams) {
    return std::make_unique<ConstraintFitStrategy>(mcFilePath, constraintParams);
}

inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateGaussianConstraintStrategy(
    RooDataSet* mcDataset,
    RooAbsPdf* mcSignalPdf,
    const std::vector<std::string>& paramNames,
    double sigmaScale) {
    return std::make_unique<GaussianConstraintStrategy>(mcDataset, mcSignalPdf, paramNames, sigmaScale);
}

// File-based Gaussian constraint creation is handled directly in MassFitterV2 now

#endif // FIT_STRATEGY_H
