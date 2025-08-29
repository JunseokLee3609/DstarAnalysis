#include "../DStarFitConfig.h"
#include "../MassFitterV2.h" 
#include "../DataLoader.h"

void quickTest() {
    std::cout << "=== Quick Fit Test ===" << std::endl;
    
    DataLoader loader("/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_Physics/RDS_Physics_Data_DStar_PbPb_mva0p9_PbPb_Aug22_v1.root");
    if(!loader.loadRooDataSet("datasetHX")) {
        std::cout << "Failed to load dataset" << std::endl;
        return;
    }
    
    auto dataset = loader.getDataSet();
    std::cout << "Original dataset: " << dataset->numEntries() << " entries" << std::endl;
    
    // Very restrictive cut to get small sample
    std::string restrictiveCut = "pT > 15.0 && pT < 20.0 && mva > 0.995";
    
    try {
        auto reducedData = dataset->reduce(restrictiveCut.c_str());
        RooDataSet* smallDataset = (RooDataSet*)reducedData;
        std::cout << "After restrictive cuts: " << smallDataset->numEntries() << " entries" << std::endl;
        
        if (smallDataset->numEntries() < 100) {
            std::cout << "Sample too small, trying less restrictive cut..." << std::endl;
            restrictiveCut = "pT > 10.0 && pT < 50.0 && mva > 0.99";
            delete smallDataset;
            reducedData = dataset->reduce(restrictiveCut.c_str());
            smallDataset = (RooDataSet*)reducedData;
            std::cout << "After less restrictive cuts: " << smallDataset->numEntries() << " entries" << std::endl;
        }
        
        if (smallDataset->numEntries() > 1000000) {
            std::cout << "Still too large, further restricting..." << std::endl;
            delete smallDataset;
            restrictiveCut = "pT > 50.0 && pT < 100.0 && mva > 0.995";
            reducedData = dataset->reduce(restrictiveCut.c_str());
            smallDataset = (RooDataSet*)reducedData;
            std::cout << "Final sample size: " << smallDataset->numEntries() << " entries" << std::endl;
        }
        
        // Try a simple fit without complex configuration
        MassFitterV2 fitter("quickTest", "mass", 0.13, 0.16, 0.1, 0.01, 0.5, 0.1, 0.01, 0.5);
        
        fitter.SetData(smallDataset);
        
        std::cout << "Simple fitter created, data set." << std::endl;
        std::cout << "Data size for fitting: " << fitter.GetData()->numEntries() << " events" << std::endl;
        
        std::cout << "Basic setup completed successfully." << std::endl;
        
        delete smallDataset;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "Quick test completed." << std::endl;
}