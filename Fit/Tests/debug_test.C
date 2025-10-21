// Debug test with detailed logging
#include "../Analysis/DatasetRegistry.h"

void debug_test() {
    std::cout << "=== Debug Test with Detailed Logging ===" << std::endl;
    
    // Test PP Dataset with debugging
    std::cout << "\n--- Testing PP Dataset with Debug Info ---" << std::endl;
    auto ppConfig = DStarAnalysis::DatasetRegistry::GetPPDataset();
    std::cout << "Final PP Data: " << ppConfig.dataFile << std::endl;
    std::cout << "Final PP MC: " << ppConfig.mcFile << std::endl;
    
    // Test PbPb Dataset with debugging
    std::cout << "\n--- Testing PbPb Dataset with Debug Info ---" << std::endl;
    auto pbpbConfig = DStarAnalysis::DatasetRegistry::GetPbPbDataset();
    std::cout << "Final PbPb Data: " << pbpbConfig.dataFile << std::endl;
    std::cout << "Final PbPb MC: " << pbpbConfig.mcFile << std::endl;
    
    // Test custom datasets with debugging
    std::cout << "\n--- Testing Custom Datasets with Debug Info ---" << std::endl;
    auto ppV2Config = DStarAnalysis::DatasetRegistry::GetPPDataV2();
    std::cout << "Final PP_Data_v2: " << ppV2Config.dataFile << std::endl;
    
    auto pbpbMVA095Config = DStarAnalysis::DatasetRegistry::GetPbPbDataMVA095();
    std::cout << "Final PbPb_Data_mva0p95: " << pbpbMVA095Config.dataFile << std::endl;
    
    std::cout << "\n=== Debug Test Completed ===" << std::endl;
}
