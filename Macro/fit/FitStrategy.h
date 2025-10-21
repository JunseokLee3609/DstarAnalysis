#ifndef FIT_STRATEGY_H
#define FIT_STRATEGY_H

#include <memory>
#include <string>
#include <vector>
#include <map>
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
#include "Opt.h"

class FitStrategy {
public:
    virtual ~FitStrategy() = default;
    virtual std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) = 0;
    virtual std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config, RooRealVar* massVar) {
        return Execute(pdf, data, config);  // Default implementation ignores massVar
    }
    virtual std::string GetName() const = 0;
    
protected:
    RooLinkedList CreateFitOptions(const FitOpt& config);
    bool ShouldSkipParameterAdjustment(const std::string& paramName, const std::vector<std::string>& skipList);
    void AdjustParameterLimits(RooFitResult* result, RooAbsPdf* pdf, const FitOpt& config);
};

class BasicFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override;
    std::string GetName() const override { return "BasicFit"; }
};

class RobustFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override;
    std::string GetName() const override { return "RobustFit"; }
    
private:
    std::unique_ptr<RooFitResult> PerformIterativeFit(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config);
    bool CheckFitConvergence(RooFitResult* result);
};

class MCFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override;
    std::string GetName() const override { return "MCFit"; }
};

class ConstraintFitStrategy : public FitStrategy {
public:
    ConstraintFitStrategy(const std::string& mcFilePath, const std::vector<std::string>& constraintParams);
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override;
    std::string GetName() const override { return "ConstraintFit"; }
    
private:
    std::string mcFilePath_;
    std::vector<std::string> constraintParameters_;
    void ApplyConstraints(RooAbsPdf* pdf);
};

class BinnedFitStrategy : public FitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override;
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config, RooRealVar* massVar) override;
    std::string GetName() const override { return "BinnedFit"; }
    
private:
    std::unique_ptr<RooDataHist> CreateBinnedData(RooDataSet* data, RooRealVar* massVar, int nBins);
};

// Factory for creating fit strategies
class FitStrategyFactory {
public:
    static std::unique_ptr<FitStrategy> CreateStrategy(FitMethod method);
    // Create strategy, then call Execute on it from caller.
    static std::unique_ptr<FitStrategy> CreateConstraintStrategy(const std::string& mcFilePath, 
                                                                const std::vector<std::string>& constraintParams);
    static std::unique_ptr<FitStrategy> CreateGaussianConstraintStrategy(
        RooDataSet* mcDataset,
        RooAbsPdf* mcSignalPdf,
        const std::vector<std::string>& paramNames,
        double sigmaScale = 1.0);
    static std::unique_ptr<FitStrategy> CreateGaussianConstraintFromFileStrategy(
        const std::string& mcResultFile,
        const std::vector<std::string>& paramsToConstrain,
        const std::string& resultObjName = "fitResult",
        double sigmaScale = 1.0);
    // Combined: Gaussian constraints from MC (signal) + sideband prefit constraints (background)
    static std::unique_ptr<FitStrategy> CreateGaussianConstraintWithSBStrategy(
        RooDataSet* mcDataset,
        RooAbsPdf* mcSignalPdf,
        RooAbsPdf* backgroundPdf,
        const std::vector<std::string>& signalParamNames,
        double sbLoMin, double sbLoMax,
        double sbHiMin, double sbHiMax,
        double sigmaScaleSignal = 1.0,
        double sigmaScaleBkg = 2.0);
    static std::unique_ptr<FitStrategy> CreateSidebandPrefitBackgroundConstraintStrategy(
        RooAbsPdf* backgroundPdf,
        double sbLoMin, double sbLoMax,
        double sbHiMin, double sbHiMax,
        double sigmaScale);
};

// Implementation
inline RooLinkedList FitStrategy::CreateFitOptions(const FitOpt& config) {
    RooLinkedList fitOpts;
    
    fitOpts.Add(new RooCmdArg(RooFit::NumCPU(config.numCPU)));
    fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(config.verbose ? 1 : -1)));
    fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Minimizer(config.strategy.c_str(), config.minimizer.c_str())));
    fitOpts.Add(new RooCmdArg(RooFit::Strategy(config.strategyLevel)));
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
        case FitMethod::GaussianConstraint:
            // Handled by MassFitterV2; default to extended
            fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
            break;
        case FitMethod::GaussianConstraintWithSB:
            // Combined constraints handled externally; default to extended
            fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
            break;
        case FitMethod::FixedFromMC:
            // Handled by MassFitterV2; default to extended
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

inline void FitStrategy::AdjustParameterLimits(RooFitResult* result, RooAbsPdf* pdf, const FitOpt& config) {
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

inline std::unique_ptr<RooFitResult> BasicFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) {
    if (!pdf || !data) return nullptr;
    
    // Try initial strategyLevel, then fall back to 1 and 0 if needed
    for (int lvl = config.strategyLevel; lvl >= 0; --lvl) {
        FitOpt cfg = config;
        cfg.strategyLevel = lvl;
        auto fitOpts = CreateFitOptions(cfg);
        auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
        if (!result || result->status() == 0) return result;
        // Otherwise, continue to try lower strategy levels
        if (lvl == 0) return result; // give back the last attempt if all failed
    }
    return nullptr;
}

inline std::unique_ptr<RooFitResult> RobustFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) {
    if (!pdf || !data) return nullptr;
    
    return PerformIterativeFit(pdf, data, config);
}

inline std::unique_ptr<RooFitResult> RobustFitStrategy::PerformIterativeFit(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) {
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
        
        // If still failing, try loosening RooFit strategy: 1 then 0
        if (newResult && newResult->status() != 0) {
            int fallbackLevels[2] = {1, 0};
            for (int i = 0; i < 2 && newResult->status() != 0; ++i) {
                FitOpt retryConfig = config;
                retryConfig.strategyLevel = fallbackLevels[i];
                auto retryOpts2 = CreateFitOptions(retryConfig);
                newResult = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, retryOpts2));
            }
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

inline std::unique_ptr<RooFitResult> MCFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) {
    if (!pdf || !data) return nullptr;
    
    // MC fitting typically uses simpler options
    FitOpt mcConfig = config;
    mcConfig.useMinos = false;  // Usually don't need Minos for MC
    
    // Try initial strategyLevel, then 1 and 0
    for (int lvl = mcConfig.strategyLevel; lvl >= 0; --lvl) {
        mcConfig.strategyLevel = lvl;
        auto fitOpts = CreateFitOptions(mcConfig);
        auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
        if (!result || result->status() == 0) return result;
        if (lvl == 0) return result;
    }
    return nullptr;
}

inline ConstraintFitStrategy::ConstraintFitStrategy(const std::string& mcFilePath, const std::vector<std::string>& constraintParams)
    : mcFilePath_(mcFilePath), constraintParameters_(constraintParams) {}

inline std::unique_ptr<RooFitResult> ConstraintFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) {
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

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override {
        return Execute(pdf, data, config, nullptr);
    }

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config, RooRealVar* massVar) override {
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
            FitOpt mcCfg = config;
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
        // Debug: summarize prepared constraints
        std::cout << "[GC][Debug] Prepared " << constraints.size() 
                  << " constraints (sigmaScale=" << sigmaScale_ << ")" << std::endl;
        if (constraints.empty()) {
            std::cout << "[GC][Debug] No constraints were built. Check param names and MC inputs." << std::endl;
        }
        int applied = 0, missing = 0;
        for (const auto& [pname, ms] : constraints) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(pname.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("mc_mean_" + pname).c_str(), ("mc_mean_" + pname).c_str(), ms.first);
                auto sigma = std::make_unique<RooRealVar>(("mc_sigma_" + pname).c_str(), ("mc_sigma_" + pname).c_str(), ms.second);
                // Ensure positive sigma range and fix both mean/sigma constants
                sigma->setMin(1e-12);
                sigma->setConstant(true);
                mean->setConstant(true);
                auto gauss = std::make_unique<RooGaussian>(("constr_" + pname).c_str(), ("constr_" + pname).c_str(), *target, *mean, *sigma);
                ext.add(*gauss);
                keepMeans.push_back(std::move(mean));
                keepSigmas.push_back(std::move(sigma));
                keepGauss.push_back(std::move(gauss));
                std::cout << "[GC][Debug] Apply constraint to '" << pname << "': mu=" << ms.first
                          << ", sigma=" << ms.second << ", data init=" << target->getVal() << std::endl;
                applied++;
            } else {
                std::cout << "[GC][Debug][WARN] Data var not found for constraint '" << pname << "'" << std::endl;
                missing++;
            }
        }
        std::cout << "[GC][Debug] Constraints applied: " << applied << ", missing: " << missing << std::endl;

        // 3) Create fit options and add constraints
        auto opts = CreateFitOptions(config);
        if (ext.getSize() > 0) {
            opts.Add(new RooCmdArg(RooFit::ExternalConstraints(ext)));
            std::cout << "[GC][Debug] ExternalConstraints attached: count=" << ext.getSize() << std::endl;
        } else {
            std::cout << "[GC][Debug] No ExternalConstraints attached" << std::endl;
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
            // Try initial strategyLevel, then 1 and 0
            std::unique_ptr<RooFitResult> result;
            for (int lvl = config.strategyLevel; lvl >= 0; --lvl) {
                FitOpt cfg2 = config;
                cfg2.strategyLevel = lvl;
                auto opts2 = CreateFitOptions(cfg2);
                if (ext.getSize() > 0) opts2.Add(new RooCmdArg(RooFit::ExternalConstraints(ext)));
                result = std::unique_ptr<RooFitResult>(pdf->fitTo(*binnedData, opts2));
                if (!result || result->status() == 0) break;
            }
            // Post-fit debug: compare to MC means
            if (result) {
                auto finals = result->floatParsFinal();
                for (const auto& [pname, ms] : constraints) {
                    RooAbsArg* arg = nullptr;
                    for (int i=0; i<finals.getSize(); ++i) {
                        if (std::string(finals.at(i)->GetName()) == pname) { arg = finals.at(i); break; }
                    }
                    if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                        double val = v->getVal();
                        double err = v->getError();
                        std::cout << "[GC][Debug][PostFit] " << pname << " = " << val
                                  << " ± " << err << " (MC mu=" << ms.first
                                  << ", pull=" << ((err>0)?(val - ms.first)/err:0.0) << ")" << std::endl;
                    }
                }
            }
            return result;
        }

        // 5) Unbinned fit
        std::unique_ptr<RooFitResult> result;
        for (int lvl = config.strategyLevel; lvl >= 0; --lvl) {
            FitOpt cfg2 = config;
            cfg2.strategyLevel = lvl;
            auto opts2 = CreateFitOptions(cfg2);
            if (ext.getSize() > 0) opts2.Add(new RooCmdArg(RooFit::ExternalConstraints(ext)));
            result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, opts2));
            if (!result || result->status() == 0) break;
        }
        // Post-fit debug: compare to MC means
        if (result) {
            auto finals = result->floatParsFinal();
            for (const auto& [pname, ms] : constraints) {
                RooAbsArg* arg = nullptr;
                for (int i=0; i<finals.getSize(); ++i) {
                    if (std::string(finals.at(i)->GetName()) == pname) { arg = finals.at(i); break; }
                }
                if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                    double val = v->getVal();
                    double err = v->getError();
                    std::cout << "[GC][Debug][PostFit] " << pname << " = " << val
                              << " ± " << err << " (MC mu=" << ms.first
                              << ", pull=" << ((err>0)?(val - ms.first)/err:0.0) << ")" << std::endl;
                }
            }
        }
        return result;
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

class GaussianConstraintFromFileStrategy : public FitStrategy {
public:
    GaussianConstraintFromFileStrategy(const std::string& mcResultFile,
                                       const std::vector<std::string>& params,
                                       const std::string& resultObject = "fitResult",
                                       double sigmaScale = 1.0)
        : mcResultFile_(mcResultFile), paramNames_(params), resultObjectName_(resultObject), sigmaScale_(sigmaScale) {}

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override {
        return Execute(pdf, data, config, nullptr);
    }

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config, RooRealVar* massVar) override {
        if (!pdf || !data) return nullptr;

        std::unique_ptr<TFile> fin(TFile::Open(mcResultFile_.c_str(), "READ"));
        if (!fin || fin->IsZombie()) {
            std::cerr << "[GC-File] ERROR: Cannot open MC result file '" << mcResultFile_ << "'" << std::endl;
            return nullptr;
        }

        auto* rf = dynamic_cast<RooFitResult*>(fin->Get(resultObjectName_.c_str()));
        if (!rf) {
            std::cerr << "[GC-File] ERROR: RooFitResult '" << resultObjectName_ << "' not found in file '"
                      << mcResultFile_ << "'" << std::endl;
            return nullptr;
        }

        std::map<std::string, std::pair<double, double>> constraints;
        auto finals = rf->floatParsFinal();
        for (const auto& name : paramNames_) {
            RooAbsArg* arg = nullptr;
            for (int i = 0; i < finals.getSize(); ++i) {
                if (std::string(finals.at(i)->GetName()) == name) { arg = finals.at(i); break; }
            }
            if (auto* v = dynamic_cast<RooRealVar*>(arg)) {
                double mu = v->getVal();
                double err = v->getError();
                if (err <= 0) {
                    double eh = std::abs(v->getErrorHi());
                    double el = std::abs(v->getErrorLo());
                    err = 0.5 * (eh + el);
                }
                err = std::max(err * sigmaScale_, 1e-6);
                constraints[name] = {mu, err};
                std::cout << "[GC-File] Constraint prepared for '" << name << "': mu=" << mu
                          << ", sigma=" << err << std::endl;
            }
        }

        std::unique_ptr<RooArgSet> dataVars(pdf->getVariables());
        RooArgSet constraintPdfs;
        std::vector<std::unique_ptr<RooRealVar>> meansKeep;
        std::vector<std::unique_ptr<RooRealVar>> sigmasKeep;
        std::vector<std::unique_ptr<RooGaussian>> gaussKeep;

        for (const auto& [pname, muSigma] : constraints) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(pname.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("gc_mean_" + pname).c_str(), ("gc_mean_" + pname).c_str(), muSigma.first);
                auto sigma = std::make_unique<RooRealVar>(("gc_sigma_" + pname).c_str(), ("gc_sigma_" + pname).c_str(), muSigma.second);
                sigma->setMin(1e-12);
                sigma->setConstant(true);
                mean->setConstant(true);
                auto gauss = std::make_unique<RooGaussian>(("gc_constr_" + pname).c_str(), ("gc_constr_" + pname).c_str(), *target, *mean, *sigma);
                constraintPdfs.add(*gauss);
                meansKeep.push_back(std::move(mean));
                sigmasKeep.push_back(std::move(sigma));
                gaussKeep.push_back(std::move(gauss));
            } else {
                std::cout << "[GC-File][WARN] Parameter '" << pname << "' not found in model; constraint skipped" << std::endl;
            }
        }

        auto fitOpts = CreateFitOptions(config);
        if (constraintPdfs.getSize() > 0) {
            fitOpts.Add(new RooCmdArg(RooFit::ExternalConstraints(constraintPdfs)));
        }

        if (massVar && !config.rangeName.empty()) {
            massVar->setRange(config.rangeName.c_str(), config.rangeMin, config.rangeMax);
        }

        auto result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
        if (!result) {
            std::cerr << "[GC-File] ERROR: Fit returned null result" << std::endl;
            return nullptr;
        }

        return result;
    }

    std::string GetName() const override { return "GaussianConstraintFromFile"; }

private:
    std::string mcResultFile_;
    std::vector<std::string> paramNames_;
    std::string resultObjectName_;
    double sigmaScale_ = 1.0;
};

class SidebandPrefitBackgroundConstraintStrategy : public FitStrategy {
public:
    SidebandPrefitBackgroundConstraintStrategy(RooAbsPdf* backgroundPdf,
                                               double sbLoMin, double sbLoMax,
                                               double sbHiMin, double sbHiMax,
                                               double sigmaScale)
        : backgroundPdf_(backgroundPdf),
          sbLoMin_(sbLoMin), sbLoMax_(sbLoMax),
          sbHiMin_(sbHiMin), sbHiMax_(sbHiMax),
          sigmaScale_(sigmaScale) {}

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override {
        return Execute(pdf, data, config, nullptr);
    }

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config, RooRealVar* massVar) override {
        if (!pdf || !data || !backgroundPdf_) {
            std::cerr << "[SB] ERROR: Missing pdf/data/background" << std::endl;
            return nullptr;
        }

        if (!massVar) {
            std::cerr << "[SB] ERROR: Mass variable is null" << std::endl;
            return nullptr;
        }

        // Define sideband ranges on the mass variable
        massVar->setRange("SBlo", sbLoMin_, sbLoMax_);
        massVar->setRange("SBhi", sbHiMin_, sbHiMax_);

        // Prefit background in sidebands only
        RooLinkedList preOpts;
        preOpts.Add(new RooCmdArg(RooFit::NumCPU(config.numCPU)));
        preOpts.Add(new RooCmdArg(RooFit::PrintLevel(config.verbose ? 1 : -1)));
        preOpts.Add(new RooCmdArg(RooFit::Save(true)));
        preOpts.Add(new RooCmdArg(RooFit::Minimizer(config.strategy.c_str(), config.minimizer.c_str())));
        preOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
        preOpts.Add(new RooCmdArg(RooFit::Range("SBlo,SBhi")));

        std::cout << "[SB] Prefitting background in sidebands [" << sbLoMin_ << ", " << sbLoMax_
                  << "] U [" << sbHiMin_ << ", " << sbHiMax_ << "]" << std::endl;
        auto prefit = std::unique_ptr<RooFitResult>(backgroundPdf_->fitTo(*data, preOpts));
        if (!prefit) {
            std::cerr << "[SB] ERROR: Background prefit returned null" << std::endl;
            return nullptr;
        }

        // Build constraints from background parameters
        std::map<std::string, std::pair<double, double>> constr;
        std::unique_ptr<RooArgSet> bkgVars(backgroundPdf_->getVariables());
        for (auto it = bkgVars->fwdIterator(); auto* var = it.next();) {
            if (auto* v = dynamic_cast<RooRealVar*>(var)) {
                std::string name = v->GetName();
                bool isBkg = name.size() >= 11 && name.rfind("_background") == name.size() - 11;
                if (!isBkg || v->isConstant()) continue;
                double mu = v->getVal();
                double err = v->getError();
                if (err <= 0) {
                    double eh = std::abs(v->getErrorHi());
                    double el = std::abs(v->getErrorLo());
                    err = 0.5 * (eh + el);
                }
                err = std::max(err * sigmaScale_, 1e-6);
                constr[name] = {mu, err};
                std::cout << "[SB] Constraint prepared for '" << name << "': mu=" << mu
                          << ", sigma=" << err << std::endl;
            }
        }

        std::unique_ptr<RooArgSet> dataVars(pdf->getVariables());
        RooArgSet constraintPdfs;
        std::vector<std::unique_ptr<RooRealVar>> meanKeep;
        std::vector<std::unique_ptr<RooRealVar>> sigmaKeep;
        std::vector<std::unique_ptr<RooGaussian>> gaussKeep;
        for (const auto& [name, ms] : constr) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(name.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("sb_mean_" + name).c_str(), ("sb_mean_" + name).c_str(), ms.first);
                auto sigma = std::make_unique<RooRealVar>(("sb_sigma_" + name).c_str(), ("sb_sigma_" + name).c_str(), ms.second);
                sigma->setMin(1e-12);
                sigma->setConstant(true);
                mean->setConstant(true);
                auto gauss = std::make_unique<RooGaussian>(("sb_constr_" + name).c_str(), ("sb_constr_" + name).c_str(), *target, *mean, *sigma);
                constraintPdfs.add(*gauss);
                meanKeep.push_back(std::move(mean));
                sigmaKeep.push_back(std::move(sigma));
                gaussKeep.push_back(std::move(gauss));
            } else {
                std::cout << "[SB][WARN] Parameter '" << name << "' not found in total model" << std::endl;
            }
        }

        auto fitOpts = CreateFitOptions(config);
        if (constraintPdfs.getSize() > 0) {
            fitOpts.Add(new RooCmdArg(RooFit::ExternalConstraints(constraintPdfs)));
        }

        std::unique_ptr<RooFitResult> result;
        if (config.fitMethod == FitMethod::BinnedNLL) {
            RooRealVar* mass = massVar;
            if (!mass) {
                auto argSet = data->get();
                for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
                    if (auto* rv = dynamic_cast<RooRealVar*>(var)) { mass = rv; break; }
                }
            }
            if (!mass) {
                std::cerr << "[SB] ERROR: Mass variable not found for binned fit" << std::endl;
                return nullptr;
            }
            auto binnedMass = std::unique_ptr<RooRealVar>(dynamic_cast<RooRealVar*>(mass->clone((std::string(mass->GetName()) + "_binned").c_str())));
            binnedMass->setBins(config.histogramBins);
            auto binnedData = std::make_unique<RooDataHist>((std::string(data->GetName()) + "_binned").c_str(),
                                                            (std::string(data->GetTitle()) + " (binned)").c_str(),
                                                            RooArgSet(*binnedMass), *data);
            result = std::unique_ptr<RooFitResult>(pdf->fitTo(*binnedData, fitOpts));
        } else {
            result = std::unique_ptr<RooFitResult>(pdf->fitTo(*data, fitOpts));
        }

        return result;
    }

    std::string GetName() const override { return "SidebandPrefitBackgroundConstraint"; }

private:
    RooAbsPdf* backgroundPdf_ = nullptr; // non-owning
    double sbLoMin_;
    double sbLoMax_;
    double sbHiMin_;
    double sbHiMax_;
    double sigmaScale_;
};

// BinnedFitStrategy implementation
inline std::unique_ptr<RooFitResult> BinnedFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) {
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

// Combined GaussianConstraint (signal from MC) + Sideband background constraints
class GaussianConstraintWithSBStrategy : public FitStrategy {
public:
    GaussianConstraintWithSBStrategy(RooDataSet* mcDataset,
                                     RooAbsPdf* mcSignalPdf,
                                     RooAbsPdf* backgroundPdf,
                                     const std::vector<std::string>& sigParamNames,
                                     double sbLoMin, double sbLoMax,
                                     double sbHiMin, double sbHiMax,
                                     double sigmaScaleSignal = 1.0,
                                     double sigmaScaleBkg = 2.0)
        : mcDataset_(mcDataset), mcSignalPdf_(mcSignalPdf), backgroundPdf_(backgroundPdf),
          paramNames_(sigParamNames),
          sbLoMin_(sbLoMin), sbLoMax_(sbLoMax), sbHiMin_(sbHiMin), sbHiMax_(sbHiMax),
          sigmaScaleSignal_(sigmaScaleSignal), sigmaScaleBkg_(sigmaScaleBkg) {}

    std::string GetName() const override { return "GaussianConstraint+SB"; }

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config) override {
        return Execute(pdf, data, config, nullptr);
    }

    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config, RooRealVar* massVar) override {
        if (!pdf || !data || !mcDataset_ || !mcSignalPdf_ || !backgroundPdf_) return nullptr;

        // 1) Signal constraints from MC
        std::map<std::string, std::pair<double,double>> sigConstr;
        {
            FitOpt mcCfg = config; mcCfg.useMinos = false;
            auto mcOpts = CreateFitOptions(mcCfg);
            auto mcRes = std::unique_ptr<RooFitResult>(mcSignalPdf_->fitTo(*mcDataset_, mcOpts));
            if (!mcRes) return nullptr;
            std::unique_ptr<RooArgSet> sigVars(mcSignalPdf_->getVariables());
            for (const auto& name : paramNames_) {
                if (auto* v = dynamic_cast<RooRealVar*>(sigVars->find(name.c_str()))) {
                    double mu = v->getVal();
                    double err = v->getError();
                    if (err <= 0) {
                        double eh = std::abs(v->getErrorHi());
                        double el = std::abs(v->getErrorLo());
                        err = 0.5 * (eh + el);
                    }
                    if (err <= 0) err = 1e-6;
                    sigConstr[name] = {mu, std::max(err * sigmaScaleSignal_, 1e-6)};
                }
            }
        }

        // 2) Background prefit in SB → constraints
        std::map<std::string, std::pair<double,double>> bkgConstr;
        RooRealVar* m = massVar;
        if (!m) {
            auto argSet = data->get();
            for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
                if (auto* rv = dynamic_cast<RooRealVar*>(var)) { m = rv; break; }
            }
        }
        if (!m) return nullptr;
        m->setRange("SBlo", sbLoMin_, sbLoMax_);
        m->setRange("SBhi", sbHiMin_, sbHiMax_);
        {
            RooLinkedList preOpts;
            preOpts.Add(new RooCmdArg(RooFit::NumCPU(config.numCPU)));
            preOpts.Add(new RooCmdArg(RooFit::PrintLevel(config.verbose ? 1 : -1)));
            preOpts.Add(new RooCmdArg(RooFit::Save(true)));
            preOpts.Add(new RooCmdArg(RooFit::Minimizer(config.strategy.c_str(), config.minimizer.c_str())));
            preOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
            preOpts.Add(new RooCmdArg(RooFit::Range("SBlo,SBhi")));
            auto prefit = std::unique_ptr<RooFitResult>(backgroundPdf_->fitTo(*data, preOpts));
            if (!prefit) return nullptr;

            std::unique_ptr<RooArgSet> bkgVars(backgroundPdf_->getVariables());
            for (auto it = bkgVars->fwdIterator(); auto* var = it.next();) {
                if (auto* v = dynamic_cast<RooRealVar*>(var)) {
                    std::string pname = v->GetName();
                    bool isBkg = pname.size() >= 11 && pname.rfind("_background") == pname.size() - 11;
                    if (!isBkg) continue;
                    if (v->isConstant()) continue;
                    double mu = v->getVal();
                    double err = v->getError();
                    if (err <= 0) {
                        double eh = std::abs(v->getErrorHi());
                        double el = std::abs(v->getErrorLo());
                        err = 0.5 * (eh + el);
                    }
                    if (err <= 0) err = 1e-6;
                    bkgConstr[pname] = {mu, std::max(err * sigmaScaleBkg_, 1e-6)};
                }
            }
        }

        // 3) Build ExternalConstraints set for both signal and background
        std::unique_ptr<RooArgSet> dataVars(pdf->getVariables());
        RooArgSet consPdfs;
        std::vector<std::unique_ptr<RooRealVar>> keepMeans, keepSigmas;
        std::vector<std::unique_ptr<RooGaussian>> keepGauss;
        auto addConstr = [&](const std::string& name, const std::pair<double,double>& ms) {
            if (auto* target = dynamic_cast<RooRealVar*>(dataVars->find(name.c_str()))) {
                auto mean = std::make_unique<RooRealVar>(("gc_mean_" + name).c_str(), ("gc_mean_" + name).c_str(), ms.first);
                auto sigm = std::make_unique<RooRealVar>(("gc_sigma_" + name).c_str(), ("gc_sigma_" + name).c_str(), ms.second);
                sigm->setMin(1e-12); sigm->setConstant(true);
                mean->setConstant(true);
                auto gauss = std::make_unique<RooGaussian>(("gc_constr_" + name).c_str(), ("gc_constr_" + name).c_str(), *target, *mean, *sigm);
                consPdfs.add(*gauss);
                keepMeans.push_back(std::move(mean));
                keepSigmas.push_back(std::move(sigm));
                keepGauss.push_back(std::move(gauss));
            }
        };
        for (const auto& kv : sigConstr) addConstr(kv.first, kv.second);
        for (const auto& kv : bkgConstr) addConstr(kv.first, kv.second);

        // 4) Fit with constraints (support binned)
        std::unique_ptr<RooFitResult> result;
        auto runFitWithOpts = [&](const FitOpt& cfg)->std::unique_ptr<RooFitResult> {
            auto opts = CreateFitOptions(cfg);
            if (consPdfs.getSize() > 0) opts.Add(new RooCmdArg(RooFit::ExternalConstraints(consPdfs)));
            opts.Add(new RooCmdArg(RooFit::Extended(true)));

            auto printYieldDebug = [&]() {
                std::unique_ptr<RooArgSet> pdfVars(pdf->getVariables());
                if (!pdfVars) return;
                auto logVar = [&](const char* name) {
                    if (auto* v = dynamic_cast<RooRealVar*>(pdfVars->find(name))) {
                        std::cout << "[GC+SB][InitYield] " << name
                                  << " = " << v->getVal()
                                  << " [" << v->getMin()
                                  << ", " << v->getMax() << "]" << std::endl;
                    }
                };
                logVar("nsig");
                logVar("nbkg");
            };

            printYieldDebug();
            if (cfg.fitMethod == FitMethod::BinnedNLL) {
                RooRealVar* mv = massVar;
                if (!mv) {
                    auto argSet = data->get();
                    for (auto it = argSet->fwdIterator(); auto* var = it.next();) {
                        if (auto* rv = dynamic_cast<RooRealVar*>(var)) { mv = rv; break; }
                    }
                }
                if (!mv) return nullptr;
                auto binnedVar = std::unique_ptr<RooRealVar>(dynamic_cast<RooRealVar*>(mv->clone((std::string(mv->GetName()) + "_binned").c_str())));
                binnedVar->setBins(cfg.histogramBins);
                auto binnedData = std::make_unique<RooDataHist>((std::string(data->GetName()) + "_binned").c_str(),
                                                                (std::string(data->GetTitle()) + " (binned)").c_str(),
                                                                RooArgSet(*binnedVar), *data);
                return std::unique_ptr<RooFitResult>(pdf->fitTo(*binnedData, opts));
            }
            std::cout << "data size : " << data->numEntries() << std::endl;
            return std::unique_ptr<RooFitResult>(pdf->fitTo(*data, opts));
        };

        for (int lvl = config.strategyLevel; lvl >= 0; --lvl) {
            FitOpt tryCfg = config; tryCfg.strategyLevel = lvl;
            result = runFitWithOpts(tryCfg);
            result->Print("v");
            if (!result || result->status() == 0) break;
        }
        return result;
    }

private:
    RooDataSet* mcDataset_ = nullptr;     // non-owning
    RooAbsPdf*  mcSignalPdf_ = nullptr;   // non-owning
    RooAbsPdf*  backgroundPdf_ = nullptr; // non-owning
    std::vector<std::string> paramNames_;
    double sbLoMin_ = 0, sbLoMax_ = 0, sbHiMin_ = 0, sbHiMax_ = 0;
    double sigmaScaleSignal_ = 1.0, sigmaScaleBkg_ = 2.0;
};

inline std::unique_ptr<RooFitResult> BinnedFitStrategy::Execute(RooAbsPdf* pdf, RooDataSet* data, const FitOpt& config, RooRealVar* massVar) {
    if (!pdf || !data || !massVar) return nullptr;
    
    // Create binned data using the provided mass variable
    auto binnedData = CreateBinnedData(data, massVar, config.histogramBins);
    if (!binnedData) {
        std::cerr << "BinnedFitStrategy: Failed to create binned data" << std::endl;
        return nullptr;
    }
    
    // Create fit options for binned fit
    // Perform binned fit with strategy fallback 2 -> 1 -> 0
    std::unique_ptr<RooFitResult> result;
    for (int lvl = config.strategyLevel; lvl >= 0; --lvl) {
        FitOpt cfg = config;
        cfg.strategyLevel = lvl;
        auto fitOpts = CreateFitOptions(cfg);
        result = std::unique_ptr<RooFitResult>(pdf->fitTo(*binnedData, fitOpts));
        if (!result || result->status() == 0) break;
    }
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

inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateStrategy(FitMethod method) {
    switch (method) {
        case FitMethod::BinnedNLL:
            return std::make_unique<BinnedFitStrategy>();
        case FitMethod::Robust:
            return std::make_unique<RobustFitStrategy>();
        case FitMethod::NLL:
            return std::make_unique<BasicFitStrategy>();
        case FitMethod::Extended:
        case FitMethod::GaussianConstraint:
        case FitMethod::GaussianConstraintWithSB:
        case FitMethod::FixedFromMC:
            return std::make_unique<BasicFitStrategy>();
    }
    return std::make_unique<BasicFitStrategy>();
}

// Execute(FitMethod, ...) removed in favor of: CreateStrategy(method) + strategy->Execute(...)

// New creators for file-based GC and SB-prefit background constraints
inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateGaussianConstraintFromFileStrategy(
    const std::string& mcResultFile,
    const std::vector<std::string>& paramsToConstrain,
    const std::string& resultObjName,
    double sigmaScale) {
    return std::make_unique<GaussianConstraintFromFileStrategy>(mcResultFile, paramsToConstrain, resultObjName, sigmaScale);
}

inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateSidebandPrefitBackgroundConstraintStrategy(
    RooAbsPdf* backgroundPdf,
    double sbLoMin, double sbLoMax,
    double sbHiMin, double sbHiMax,
    double sigmaScale) {
    return std::make_unique<SidebandPrefitBackgroundConstraintStrategy>(backgroundPdf, sbLoMin, sbLoMax, sbHiMin, sbHiMax, sigmaScale);
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

inline std::unique_ptr<FitStrategy> FitStrategyFactory::CreateGaussianConstraintWithSBStrategy(
    RooDataSet* mcDataset,
    RooAbsPdf* mcSignalPdf,
    RooAbsPdf* backgroundPdf,
    const std::vector<std::string>& signalParamNames,
    double sbLoMin, double sbLoMax,
    double sbHiMin, double sbHiMax,
    double sigmaScaleSignal,
    double sigmaScaleBkg) {
    return std::make_unique<GaussianConstraintWithSBStrategy>(
        mcDataset, mcSignalPdf, backgroundPdf, signalParamNames,
        sbLoMin, sbLoMax, sbHiMin, sbHiMax,
        sigmaScaleSignal, sigmaScaleBkg);
}

// File-based Gaussian constraint creation is handled directly in MassFitterV2 now

#endif // FIT_STRATEGY_H
