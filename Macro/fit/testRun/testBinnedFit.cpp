#include "../DStarFitConfig.h"
#include "../MassFitterV2.h"
#include "../DataLoader.h"

void testBinnedFit() {
    std::cout << "=== Testing Binned Fit ===" << std::endl;
    
    DStarFitConfig config;
    config.SetDataFilePath("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    config.SetDatasetName("datasetHX");
    config.SetFitMethod(FitMethod::BinnedNLL);
    config.SetDoRefit(true);
    config.AddPtBin(10,100);
    config.AddCosBin(-2,2);
    config.AddCentralityBin(0,100);
    
    auto bins = config.GetAllKinematicBins();
    std::cout << "Testing binned fit with " << bins.size() << " bins" << std::endl;
    
    for(const auto& bin : bins) {
        std::cout << "Bin: " << bin.GetBinName() << std::endl;
        auto fitOpt = config.CreateFitOpt(bin);
        std::cout << "Cut: " << fitOpt.cutExpr << std::endl;
        
        DataLoader loader(config.GetDataFilePath());
        if(loader.loadRooDataSet(config.GetDatasetName())) {
            auto dataset = loader.getDataSet();
            std::cout << "Dataset loaded: " << dataset->numEntries() << " entries" << std::endl;
            
            // Try to apply the cut
            try {
                auto reducedDataset = dataset->reduce(fitOpt.cutExpr.c_str());
                std::cout << "After cuts: " << reducedDataset->numEntries() << " entries" << std::endl;
                
                // Now try to create a fitter and perform binned fit
                auto fitter = CreateDStarFitter(bin, config);
                auto binParams = config.GetParametersForBin(bin);
                
                std::cout << "Attempting binned fit..." << std::endl;
                bool success = fitter->PerformFit(fitOpt, dataset, 
                                                 binParams.signalParams, 
                                                 binParams.backgroundParams,
                                                 bin.GetBinName());
                
                if(success) {
                    std::cout << "✓ Binned fit successful!" << std::endl;
                    fitter->PrintSummary(bin.GetBinName());
                } else {
                    std::cout << "✗ Binned fit failed!" << std::endl;
                }
                
            } catch(const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Failed to load dataset" << std::endl;
        }
        break; // Only test first bin
    }
}