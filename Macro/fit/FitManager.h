#ifndef FIT_MANAGER_H
#define FIT_MANAGER_H

#include "MassFitter.h"
#include "DefaultOptions.h"
#include "PDFSelector.h"
#include "BinningAdjuster.h"
#include "TFile.h"
#include "RooDataSet.h"
#include <memory>
#include <iostream>

class FitManager {
public:
    FitManager(const std::string& filepath, const std::string& datasetName);
    ~FitManager();
    void PerformFit(const std::string& particleType, const std::vector<double>& ptBins, const std::vector<double>& etaBins);

private:
    std::string filepath_;
    std::string datasetName_;
    std::unique_ptr<TFile> file_;
    std::unique_ptr<RooDataSet> dataset_;
};

FitManager::FitManager(const std::string& filepath, const std::string& datasetName)
    : filepath_(filepath), datasetName_(datasetName), file_(nullptr), dataset_(nullptr) {
    file_ = std::unique_ptr<TFile>(TFile::Open(filepath.c_str()));
    if (!file_ || file_->IsZombie()) {
        std::cerr << "Error: Failed to open file " << filepath << std::endl;
        return;
    }

    dataset_ = std::unique_ptr<RooDataSet>((RooDataSet*)file_->Get(datasetName.c_str()));
    if (!dataset_) {
        std::cerr << "Error: Failed to load dataset from file" << std::endl;
        file_->Close();
        return;
    }
}

FitManager::~FitManager() {
    if (file_) {
        file_->Close();
    }
}

void FitManager::PerformFit(const std::string& particleType, const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DefaultOptions opt;
    if (particleType == "D0") {
        opt = DefaultOptions::D0Default();
    } else if (particleType == "Dstar") {
        opt = DefaultOptions::DstarDefault();
    }

    // Adjust parameters based on binning
    // Create MassFitter object
    MassFitter fitter(opt.name, opt.massMin, opt.massMax);

    // Set data and apply cut
    fitter.SetData(dataset_.get());
    fitter.ApplyCut(opt.cutExpr);
    

    // Create and set PDFs
    if(particleType =="D0"){

    }
    fitter.SetSignalPDF(signalPDF);
    fitter.SetBackgroundPDF(backgroundPDF);

    // Perform fit and save results
    fitter.Fit(opt.useMinos, opt.useHesse, opt.verbose);
    fitter.SaveResults(opt.outputDir + "/" + opt.outputPrefix + "_results.root", opt.saveWorkspace);
}
#endif // FIT_MANAGER_H