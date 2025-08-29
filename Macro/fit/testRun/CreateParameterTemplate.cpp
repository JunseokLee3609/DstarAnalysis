#include "../SimpleParameterLoader.h"

/**
 * @brief Parameter template generator
 * 
 * 이 매크로는 다양한 bin 설정에 대한 parameter template 파일을 생성합니다.
 */

void CreateParameterTemplate(const std::string& templateName = "parameter_template.txt") {
    
    std::cout << "=== D* Parameter Template Generator ===" << std::endl;
    
    // 다양한 template 옵션
    if (templateName == "basic" || templateName == "parameter_template.txt") {
        std::cout << "Creating basic parameter template..." << std::endl;
        ParameterLoaderUtils::CreateDefaultTemplate("parameter_template.txt");
    }
    else if (templateName == "detailed") {
        std::cout << "Creating detailed parameter template with multiple pT bins..." << std::endl;
        
        std::vector<KinematicBin> bins = {
            // Fine pT binning
            KinematicBin(4.0, 5.0, -2.0, 2.0, 0, 100),
            KinematicBin(5.0, 6.0, -2.0, 2.0, 0, 100),
            KinematicBin(6.0, 7.0, -2.0, 2.0, 0, 100),
            KinematicBin(7.0, 8.0, -2.0, 2.0, 0, 100),
            KinematicBin(8.0, 10.0, -2.0, 2.0, 0, 100),
            KinematicBin(10.0, 12.0, -2.0, 2.0, 0, 100),
            KinematicBin(12.0, 15.0, -2.0, 2.0, 0, 100),
            KinematicBin(15.0, 20.0, -2.0, 2.0, 0, 100),
            KinematicBin(20.0, 100.0, -2.0, 2.0, 0, 100)
        };
        
        SimpleParameterLoader loader;
        loader.CreateTemplateFile("parameter_template_detailed.txt", bins);
    }
    else if (templateName == "cosTheta") {
        std::cout << "Creating cos(θ) binned parameter template..." << std::endl;
        
        std::vector<KinematicBin> bins = {
            // cos(θ) bins for different pT ranges
            KinematicBin(5.0, 10.0, -2.0, -1.0, 0, 100),  // backward
            KinematicBin(5.0, 10.0, -1.0, 0.0, 0, 100),   // central-backward
            KinematicBin(5.0, 10.0, 0.0, 1.0, 0, 100),    // central-forward
            KinematicBin(5.0, 10.0, 1.0, 2.0, 0, 100),    // forward
            
            KinematicBin(10.0, 20.0, -2.0, -1.0, 0, 100), 
            KinematicBin(10.0, 20.0, -1.0, 0.0, 0, 100),  
            KinematicBin(10.0, 20.0, 0.0, 1.0, 0, 100),   
            KinematicBin(10.0, 20.0, 1.0, 2.0, 0, 100),   
        };
        
        SimpleParameterLoader loader;
        loader.CreateTemplateFile("parameter_template_cosTheta.txt", bins);
    }
    else if (templateName == "single") {
        std::cout << "Creating single bin parameter template..." << std::endl;
        
        std::vector<KinematicBin> bins = {
            KinematicBin(10.0, 100.0, -2.0, 2.0, 0, 100)
        };
        
        SimpleParameterLoader loader;
        loader.CreateTemplateFile("parameter_template_single.txt", bins);
    }
    else {
        std::cout << "Creating custom parameter template: " << templateName << std::endl;
        ParameterLoaderUtils::CreateDefaultTemplate(templateName);
    }
    
    std::cout << "Template creation completed!" << std::endl;
    std::cout << "\n=== How to use ===" << std::endl;
    std::cout << "1. Edit the generated parameter file to tune your parameters" << std::endl;
    std::cout << "2. Run DStarAnalysisV2 with the parameter file:" << std::endl;
    std::cout << "   root 'DStarAnalysisV2.cpp(true, true, true, 10, 100, -2, 2, 0, 100, \"your_parameter_file.txt\")'" << std::endl;
}

void CreateParameterFromBins(float pTMin = 4, float pTMax = 100, 
                            float cosMin = -2, float cosMax = 2,
                            int nPtBins = 4, int nCosBins = 1,
                            const std::string& outputFile = "custom_parameters.txt") {
    
    std::cout << "=== Creating Custom Parameter Template ===" << std::endl;
    std::cout << "pT range: [" << pTMin << ", " << pTMax << "], bins: " << nPtBins << std::endl;
    std::cout << "cos(θ) range: [" << cosMin << ", " << cosMax << "], bins: " << nCosBins << std::endl;
    
    std::vector<KinematicBin> bins;
    
    // Create pT bins
    float pTStep = (pTMax - pTMin) / nPtBins;
    float cosStep = (cosMax - cosMin) / nCosBins;
    
    for (int i = 0; i < nPtBins; ++i) {
        float pTLow = pTMin + i * pTStep;
        float pTHigh = pTMin + (i + 1) * pTStep;
        
        for (int j = 0; j < nCosBins; ++j) {
            float cosLow = cosMin + j * cosStep;
            float cosHigh = cosMin + (j + 1) * cosStep;
            
            bins.emplace_back(pTLow, pTHigh, cosLow, cosHigh, 0, 100);
        }
    }
    
    SimpleParameterLoader loader;
    loader.CreateTemplateFile(outputFile, bins);
    
    std::cout << "Custom parameter template created: " << outputFile << std::endl;
    std::cout << "Total bins created: " << bins.size() << std::endl;
}

/**
 * @brief Test parameter loading
 */
void TestParameterLoading(const std::string& paramFile = "sample_parameters.txt") {
    std::cout << "=== Testing Parameter Loading ===" << std::endl;
    
    SimpleParameterLoader loader;
    
    if (!loader.LoadFromFile(paramFile)) {
        std::cerr << "Failed to load parameter file: " << paramFile << std::endl;
        return;
    }
    
    std::cout << "Successfully loaded parameters from: " << paramFile << std::endl;
    
    // Print loaded parameters
    loader.PrintLoadedParameters();
    
    // Test parameter retrieval for a sample bin
    KinematicBin testBin(10.0, 100.0, -2.0, 2.0, 0, 100);
    auto params = loader.GetParametersForBin(testBin);
    
    std::cout << "\n=== Test Bin Parameters ===" << std::endl;
    std::cout << "Signal PDF Type: " << static_cast<int>(params.signalPdfType) << std::endl;
    std::cout << "Background PDF Type: " << static_cast<int>(params.backgroundPdfType) << std::endl;
    std::cout << "Signal yield ratio: " << params.nsig_ratio << std::endl;
    std::cout << "Background yield ratio: " << params.nbkg_ratio << std::endl;
    
    if (params.signalPdfType == PDFType::DoubleGaussian) {
        std::cout << "DoubleGaussian mean: " << params.doubleGaussianParams.mean << std::endl;
        std::cout << "DoubleGaussian sigma1: " << params.doubleGaussianParams.sigma1 << std::endl;
        std::cout << "DoubleGaussian sigma2: " << params.doubleGaussianParams.sigma2 << std::endl;
        std::cout << "DoubleGaussian fraction: " << params.doubleGaussianParams.fraction << std::endl;
    }
    
    if (params.backgroundPdfType == PDFType::ThresholdFunction) {
        std::cout << "ThresholdFunction p0: " << params.thresholdFuncParams.p0_init << std::endl;
        std::cout << "ThresholdFunction p1: " << params.thresholdFuncParams.p1_init << std::endl;
    }
    
    std::cout << "\nParameter loading test completed successfully!" << std::endl;
}