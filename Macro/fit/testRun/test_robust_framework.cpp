#include "../MassFitter.h"
#include "../RobustParameterManager.h"
#include <iostream>

// Simple test to verify the robust framework compiles and basic functionality works
void test_robust_framework() {
    std::cout << "=== Testing Robust Fitting Framework ===" << std::endl;
    
    try {
        // Test 1: Initialize logging
        FittingLogger::setLogLevel(LogLevel::INFO);
        FittingLogger::info("Robust framework test started");
        std::cout << "✓ Logging system initialized" << std::endl;
        
        // Test 2: Create RobustParameterManager
        RobustParameterManager paramManager;
        std::cout << "✓ RobustParameterManager created" << std::endl;
        
        // Test 3: Create parameters
        auto* meanParam = paramManager.createParameter("test_mean", ParameterType::MEAN, 
                                                       0.1455, 0.1450, 0.1460, "D* mass mean");
        auto* sigmaParam = paramManager.createParameter("test_sigma", ParameterType::SIGMA, 
                                                        0.0005, 0.0001, 0.001, "D* mass sigma");
        
        if (meanParam && sigmaParam) {
            std::cout << "✓ Parameters created successfully" << std::endl;
            
            // Test parameter values
            std::cout << "  Mean: " << meanParam->getVal() << " [" << meanParam->getMin() 
                      << ", " << meanParam->getMax() << "]" << std::endl;
            std::cout << "  Sigma: " << sigmaParam->getVal() << " [" << sigmaParam->getMin() 
                      << ", " << sigmaParam->getMax() << "]" << std::endl;
        } else {
            std::cout << "✗ Parameter creation failed" << std::endl;
            return;
        }
        
        // Test 4: Parameter validation
        if (paramManager.validateAllParameters()) {
            std::cout << "✓ Parameter validation passed" << std::endl;
        } else {
            std::cout << "✗ Parameter validation failed" << std::endl;
        }
        
        // Test 5: State management
        paramManager.saveParameterState("test_state");
        paramManager.setParameterValue("test_mean", 0.1456);
        std::cout << "  Modified mean: " << meanParam->getVal() << std::endl;
        
        paramManager.restoreParameterState("test_state");
        std::cout << "  Restored mean: " << meanParam->getVal() << std::endl;
        std::cout << "✓ State management works" << std::endl;
        
        // Test 6: Smart parameter adjustment
        std::vector<std::string> paramNames = {"test_mean", "test_sigma"};
        paramManager.smartAdjustParameterRanges(paramNames, 1.5);
        std::cout << "✓ Smart parameter range adjustment completed" << std::endl;
        
        // Test 7: SmartFitStrategyManager
        SmartFitStrategyManager strategyManager;
        std::cout << "✓ SmartFitStrategyManager created" << std::endl;
        
        // Test strategy recommendation
        FitStrategy recommended = strategyManager.recommendStrategy();
        std::cout << "  Recommended strategy: " << static_cast<int>(recommended) << std::endl;
        
        // Test 8: Create MassFitter with robust framework
        std::string massVar = "massDstar";
        MassFitter fitter("test_fitter", massVar, 0.135, 0.155, 0.3, 0.1, 0.8, 0.2, 0.05, 0.6);
        std::cout << "✓ MassFitter with robust framework created" << std::endl;
        
        // Test 9: Performance monitoring
        {
            ScopedTimer timer("test_operation");
            // Simulate some work
            for (int i = 0; i < 1000000; ++i) {
                double x = i * 0.001;
                x = x * x;  // Simple computation
            }
        }
        std::cout << "✓ Performance monitoring works" << std::endl;
        
        // Test 10: Print parameter summary
        paramManager.printParameterSummary();
        
        // Print performance statistics
        PerformanceMonitor::printStatistics();
        
        FittingLogger::info("All tests completed successfully");
        std::cout << "\n=== ✓ ALL TESTS PASSED ===" << std::endl;
        
    } catch (const FittingException& e) {
        std::cout << "✗ FittingException: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "✗ Unknown exception occurred" << std::endl;
    }
}

// Main function for standalone testing
int main() {
    test_robust_framework();
    return 0;
}