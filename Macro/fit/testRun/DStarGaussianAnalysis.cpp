#include "../DStarFitConfig.h"
#include "../MassFitterV2.h"
#include "../DataLoader.h"
#include "../EnhancedPlotManager.h"
#include "../JSONParameterUtils.h"
#include "../ParameterDebugUtils.h"

// Simple macro to run a single-bin Gaussian signal fit for D* analysis
void DStarGaussianAnalysis(bool doReFit = true, bool plotFit = true, bool useCUDA = true,
                           float pTMin = 10, float pTMax = 100, float cosMin = -2, float cosMax = 2,
                           int centralityMin = 0, int centralityMax = 100, bool isMC = false,
                           const std::string& parameterFile = "") {
    // 1) Configure analysis
    DStarFitConfig config;
    config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    config.SetMCFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_Physics_MC_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    config.SetDatasetName("datasetEP");
    config.SetOutputSubDir("/DStar_Gaussian_Analysis/");
    config.SetFitMethod(FitMethod::BinnedNLL);
    config.SetUseCUDA(useCUDA);
    config.SetVerbose(false);
    config.SetDoRefit(doReFit);
    config.SetUseAbsCosCuts(true);

    // Define single bin
    config.AddPtBin(pTMin, pTMax);
    config.AddCosBin(cosMin, cosMax);
    config.AddCentralityBin(centralityMin, centralityMax);
    KinematicBin bin(pTMin, pTMax, cosMin, cosMax, centralityMin, centralityMax);

    // 2) Load parameters from JSON if provided; fallback to Gaussian defaults
    DStarBinParameters binParams;
    bool usedJSON = false;
    if (!parameterFile.empty()) {
        try {
            std::cout << "[Gaussian] Loading parameters from JSON: " << parameterFile << std::endl;
            JSONParameterLoader jsonLoader;
            jsonLoader.loadFromFile(parameterFile);
            BinIdentifier binId;
            binId.ptMin = bin.pTMin; binId.ptMax = bin.pTMax;
            binId.cosMin = bin.cosMin; binId.cosMax = bin.cosMax;
            binId.centralityMin = bin.centralityMin; binId.centralityMax = bin.centralityMax;
            auto loaded = LoadBinParametersFromJSONWithFixedInfo(jsonLoader, binId);
            binParams = loaded.first;
            config.SetParametersForBin(bin, binParams);
            config.SetFixedFlagsForBin(bin, loaded.second.fixedFlags);
            usedJSON = true;
            ParameterDebug::PrintBinParameters(bin, binParams, "Loaded JSON Parameters");
        } catch (const std::exception& e) {
            std::cerr << "[Gaussian] JSON loading failed: " << e.what() << "\n[Gaussian] Falling back to defaults." << std::endl;
        }
    }
    if (!usedJSON) {
        binParams = config.GetParametersForBin(bin);
        // Force Gaussian + Threshold defaults if still generic
        binParams.signalPdfType = PDFType::Gaussian;
        binParams.backgroundPdfType = PDFType::ThresholdFunction;
        binParams.gaussianParams.mean = 0.1455;
        binParams.gaussianParams.mean_min = 0.1452;
        binParams.gaussianParams.mean_max = 0.1458;
        binParams.gaussianParams.sigma = 0.0006;
        binParams.gaussianParams.sigma_min = 0.0001;
        binParams.gaussianParams.sigma_max = 0.01;
        binParams.thresholdFuncParams.p0_init = 1.0;
        binParams.thresholdFuncParams.p0_min = -10.0;
        binParams.thresholdFuncParams.p0_max = 10.0;
        binParams.thresholdFuncParams.p1_init = -1.0;
        binParams.thresholdFuncParams.p1_min = -10.0;
        binParams.thresholdFuncParams.p1_max = 10.0;
        binParams.thresholdFuncParams.m_pi_value = 0.13957;
        binParams.nsig_ratio = 0.01; binParams.nsig_min_ratio = 0.0; binParams.nsig_max_ratio = 1.0;
        binParams.nbkg_ratio = 0.01; binParams.nbkg_min_ratio = 0.0; binParams.nbkg_max_ratio = 1.0;
        config.SetParametersForBin(bin, binParams);
    }

    // 3) Prepare fitter and data
    auto fitOpt = config.CreateFitOpt(bin);
    auto fitter = CreateDStarFitter(bin, config);

    DataLoader loader(config.GetDataFilePath());
    if (!loader.loadRooDataSet(config.GetDatasetName())) {
        std::cerr << "[Gaussian] Failed to load dataset: " << config.GetDatasetName() << std::endl;
        return;
    }
    auto dataset = loader.getDataSet();
    if (!dataset) {
        std::cerr << "[Gaussian] Dataset is null" << std::endl;
        return;
    }

    // 4) Perform Gaussian fit
    bool fitSuccess = false;
    if (doReFit) {
        auto signalParamsVariant = binParams.GetSignalParamsByType();
        auto backgroundParamsVariant = binParams.GetBackgroundParamsByType();
        std::visit([&](auto&& sParams) {
            std::visit([&](auto&& bParams) {
                fitSuccess = fitter->PerformFit(fitOpt, dataset, sParams, bParams, bin.GetBinName());
            }, backgroundParamsVariant);
        }, signalParamsVariant);
    } else {
        // Check existing results
        std::string inputDir = fitOpt.outputDir;
        std::string inputFile = fitOpt.outputFile + ".root";
        std::ifstream fin(inputDir + "/" + inputFile);
        fitSuccess = fin.good();
    }

    if (!fitSuccess) {
        std::cerr << "[Gaussian] Fit failed" << std::endl;
        return;
    }

    // 5) Save results
    std::cout << "[Gaussian] Fit successful. Saving results..." << std::endl;
    std::string category = isMC ? "/MC" : "/Data";
    std::string outDir = fitOpt.outputDir + fitOpt.subDir + category;
    createDir(Form("%s/", outDir.c_str()));
    fitter->SaveResult(bin.GetBinName(), outDir, fitOpt.outputFile + ".root", true);
    fitter->ExportResults("json", outDir + "/" + fitOpt.outputFile + "_results.json");

    // 6) Perform MC fit and save (for Gaussian constraints)
    try {
        std::cout << "[Gaussian] Performing MC fit for constraints..." << std::endl;
        DataLoader mcLoader(config.GetMCFilePath());
        if (!mcLoader.loadRooDataSet(config.GetDatasetName())) {
            std::cerr << "[Gaussian][MC] Failed to load MC dataset: " << config.GetDatasetName() << std::endl;
        } else {
            auto mcDataset = mcLoader.getDataSet();
            if (mcDataset) {
                bool mcFitSuccess = false;
                auto signalParamsVariant = binParams.GetSignalParamsByType();
                std::visit([&](auto&& sParams) {
                    mcFitSuccess = fitter->PerformMCFit(fitOpt, mcDataset, sParams, bin.GetBinName() + "_MC");
                }, signalParamsVariant);

                if (mcFitSuccess) {
                    std::string mcOutDir = fitOpt.outputDir + fitOpt.subDir + "/MC";
                    createDir(Form("%s/", mcOutDir.c_str()));
                    fitter->SaveResult(bin.GetBinName() + "_MC", mcOutDir, fitOpt.outputFile + ".root", true);
                    fitter->ExportResults("json", mcOutDir + "/" + fitOpt.outputFile + "_results.json");
                    std::cout << "[Gaussian][MC] MC fit saved: " << mcOutDir << "/" << fitOpt.outputFile << ".root" << std::endl;

                    // Plot MC results as well
                    try {
                        std::string mcPlotDir = mcOutDir + "/plots";
                        EnhancedPlotManager mcPlotManager(fitOpt, mcOutDir, fitOpt.outputFile + ".root", mcPlotDir, false, true);
                        if (mcPlotManager.IsValid()) {
                            mcPlotManager.PrintSummary();
                            mcPlotManager.DrawRawDistribution("raw_MC_" + bin.GetBinName());
                            mcPlotManager.DrawFittedModel(true, "fitted_MC_" + bin.GetBinName());
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "[Gaussian][MC] Plotting error: " << e.what() << std::endl;
                    }
                } else {
                    std::cerr << "[Gaussian][MC] MC fit failed" << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Gaussian][MC] Exception: " << e.what() << std::endl;
    }

    // 7) Gaussian-constrained fit using saved MC RooFitResult
    try {
        std::cout << "[Gaussian] Running Gaussian-constrained fit using saved MC result..." << std::endl;
        // Choose constraint parameters by signal PDF type
        std::vector<std::string> constrainParams;
        switch (binParams.signalPdfType) {
            case PDFType::Gaussian:            constrainParams = {"mean", "sigma"}; break;
            case PDFType::DoubleGaussian:      constrainParams = {"mean", "sigma1"}; break;
            case PDFType::CrystalBall:         constrainParams = {"mean", "sigma"}; break;
            case PDFType::DBCrystalBall:       constrainParams = {"mean", "sigma"}; break;
            case PDFType::DoubleDBCrystalBall: constrainParams = {"mean", "sigma"}; break;
            case PDFType::Voigtian:            constrainParams = {"mean"}; break;
            case PDFType::BreitWigner:         constrainParams = {"mean"}; break;
            default:                           constrainParams = {"mean"}; break;
        }

        std::string mcResultFile = fitOpt.outputDir + fitOpt.subDir + "/MC/" + fitOpt.outputFile + ".root";
        bool gcSuccess = false;
        auto signalParamsVariant = binParams.GetSignalParamsByType();
        auto backgroundParamsVariant = binParams.GetBackgroundParamsByType();
        std::visit([&](auto&& sParams) {
            std::visit([&](auto&& bParams) {
                gcSuccess = fitter->PerformGaussianConstraintFitWithMCFile(
                    fitOpt, dataset,
                    sParams, bParams,
                    mcResultFile,
                    constrainParams,
                    bin.GetBinName() + std::string("_GC")
                );
            }, backgroundParamsVariant);
        }, signalParamsVariant);

        if (gcSuccess) {
            std::string dataOutDir = fitOpt.outputDir + fitOpt.subDir + "/Data";
            createDir(Form("%s/", dataOutDir.c_str()));
            fitter->SaveResult(bin.GetBinName() + "_GC", dataOutDir, fitOpt.outputFile + "_gauss.root", true);
            fitter->ExportResults("json", dataOutDir + "/" + fitOpt.outputFile + "_gauss_results.json");
            std::cout << "[Gaussian] Gaussian-constrained fit saved." << std::endl;
        } else {
            std::cerr << "[Gaussian] Gaussian-constrained fit failed" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Gaussian] Exception in constrained fit: " << e.what() << std::endl;
    }

    // 8) Plot
    if (plotFit) {
        try {
            std::string plotDir = outDir + "/plots";
            EnhancedPlotManager plotManager(fitOpt, outDir, fitOpt.outputFile + ".root", plotDir, false, true);
            if (plotManager.IsValid()) {
                plotManager.PrintSummary();
                plotManager.DrawRawDistribution("raw_" + bin.GetBinName());
                plotManager.DrawFittedModel(true, "fitted_" + bin.GetBinName());
            }
        } catch (const std::exception& e) {
            std::cerr << "[Gaussian] Plotting error: " << e.what() << std::endl;
        }
    }

    std::cout << "[Gaussian] Done." << std::endl;
}
