#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <iostream>
#include <cassert>
#include "PDFFactory.h"
#include "FitStrategy.h"
#include "ResultManager.h"
#include "ErrorHandler.h"

// Forward declarations for interface classes
class IPDFFactory;
class IFitStrategy;
class IResultManager;
class IDataProvider;

// Data provider interface for dependency injection
class IDataProvider {
public:
    virtual ~IDataProvider() = default;
    virtual RooDataSet* GetDataSet(const std::string& name) = 0;
    virtual RooRealVar* GetMassVariable() = 0;
    virtual bool IsValid() const = 0;
};

// PDF Factory interface for dependency injection
class IPDFFactory {
public:
    virtual ~IPDFFactory() = default;
    virtual std::unique_ptr<RooAbsPdf> CreateSignalPDF(const std::string& type, const void* params) = 0;
    virtual std::unique_ptr<RooAbsPdf> CreateBackgroundPDF(const std::string& type, const void* params) = 0;
    virtual const std::vector<std::unique_ptr<RooRealVar>>& GetParameters() const = 0;
    virtual void ClearParameters() = 0;
};

// Fit Strategy interface for dependency injection
class IFitStrategy {
public:
    virtual ~IFitStrategy() = default;
    virtual std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) = 0;
    virtual std::string GetName() const = 0;
};

// Result Manager interface for dependency injection
class IResultManager {
public:
    virtual ~IResultManager() = default;
    virtual void StoreResult(const std::string& name, std::unique_ptr<RooFitResult> fitResult,
                           std::unique_ptr<RooWorkspace> workspace, const std::string& fitType = "") = 0;
    virtual FitResults* GetResult(const std::string& name) = 0;
    virtual double GetSignalYield(const std::string& name) const = 0;
    virtual double GetSignalYieldError(const std::string& name) const = 0;
    virtual bool IsGoodFit(const std::string& name, double maxChiSquare = 5.0) const = 0;
};

// Mock implementations for testing

// Mock Data Provider
class MockDataProvider : public IDataProvider {
public:
    MockDataProvider() {
        // Create mock mass variable
        massVar_ = std::make_unique<RooRealVar>("mass", "Mass", 1.8, 2.0);
        
        // Create mock dataset with some dummy data
        dataset_ = std::make_unique<RooDataSet>("mockData", "Mock Data", RooArgSet(*massVar_));
        
        // Add some mock data points
        for (int i = 0; i < 1000; ++i) {
            double mass = 1.8 + 0.2 * gRandom->Uniform();
            massVar_->setVal(mass);
            dataset_->add(RooArgSet(*massVar_));
        }
    }
    
    RooDataSet* GetDataSet(const std::string& name) override {
        if (name == "test" || name.empty()) {
            return dataset_.get();
        }
        return nullptr;
    }
    
    RooRealVar* GetMassVariable() override {
        return massVar_.get();
    }
    
    bool IsValid() const override {
        return massVar_ != nullptr && dataset_ != nullptr && dataset_->numEntries() > 0;
    }
    
    // Test helpers
    void SetDatasetSize(int size) {
        if (size <= 0) return;
        
        dataset_->reset();
        for (int i = 0; i < size; ++i) {
            double mass = 1.8 + 0.2 * gRandom->Uniform();
            massVar_->setVal(mass);
            dataset_->add(RooArgSet(*massVar_));
        }
    }
    
    void SetMassRange(double min, double max) {
        massVar_ = std::make_unique<RooRealVar>("mass", "Mass", min, max);
        dataset_ = std::make_unique<RooDataSet>("mockData", "Mock Data", RooArgSet(*massVar_));
    }
    
private:
    std::unique_ptr<RooRealVar> massVar_;
    std::unique_ptr<RooDataSet> dataset_;
};

// Mock PDF Factory
class MockPDFFactory : public IPDFFactory {
public:
    MockPDFFactory(RooRealVar* massVar) : massVar_(massVar) {}
    
    std::unique_ptr<RooAbsPdf> CreateSignalPDF(const std::string& type, const void* params) override {
        if (type == "Gaussian") {
            auto mean = std::make_unique<RooRealVar>("mean", "Mean", 1.86, 1.85, 1.87);
            auto sigma = std::make_unique<RooRealVar>("sigma", "Sigma", 0.01, 0.005, 0.02);
            
            RooRealVar* meanPtr = mean.get();
            RooRealVar* sigmaPtr = sigma.get();
            
            parameters_.push_back(std::move(mean));
            parameters_.push_back(std::move(sigma));
            
            return std::make_unique<RooGaussian>("mockGauss", "Mock Gaussian", *massVar_, *meanPtr, *sigmaPtr);
        }
        return nullptr;
    }
    
    std::unique_ptr<RooAbsPdf> CreateBackgroundPDF(const std::string& type, const void* params) override {
        if (type == "Exponential") {
            auto lambda = std::make_unique<RooRealVar>("lambda", "Lambda", -1.0, -5.0, 0.0);
            RooRealVar* lambdaPtr = lambda.get();
            parameters_.push_back(std::move(lambda));
            
            return std::make_unique<RooExponential>("mockExp", "Mock Exponential", *massVar_, *lambdaPtr);
        }
        return nullptr;
    }
    
    const std::vector<std::unique_ptr<RooRealVar>>& GetParameters() const override {
        return parameters_;
    }
    
    void ClearParameters() override {
        parameters_.clear();
    }
    
    // Test helpers
    void SetShouldFail(bool shouldFail) { shouldFail_ = shouldFail; }
    int GetCreateCallCount() const { return createCallCount_; }
    void ResetCallCount() { createCallCount_ = 0; }
    
private:
    RooRealVar* massVar_;
    std::vector<std::unique_ptr<RooRealVar>> parameters_;
    bool shouldFail_ = false;
    mutable int createCallCount_ = 0;
};

// Mock Fit Strategy
class MockFitStrategy : public IFitStrategy {
public:
    std::unique_ptr<RooFitResult> Execute(RooAbsPdf* pdf, RooDataSet* data, const FitConfig& config) override {
        executeCallCount_++;
        lastConfig_ = config;
        
        if (shouldFail_) {
            throw FitException("MockFit", -1, "Mock fit failure");
        }
        
        // Create a mock fit result
        // Note: In real implementation, this would be the actual fit result from RooFit
        // For testing, we return nullptr or a mock result
        return nullptr; // Simplified for testing
    }
    
    std::string GetName() const override {
        return "MockFitStrategy";
    }
    
    // Test helpers
    void SetShouldFail(bool shouldFail) { shouldFail_ = shouldFail; }
    int GetExecuteCallCount() const { return executeCallCount_; }
    const FitConfig& GetLastConfig() const { return lastConfig_; }
    void ResetCallCount() { executeCallCount_ = 0; }
    
private:
    bool shouldFail_ = false;
    int executeCallCount_ = 0;
    FitConfig lastConfig_;
};

// Mock Result Manager
class MockResultManager : public IResultManager {
public:
    void StoreResult(const std::string& name, std::unique_ptr<RooFitResult> fitResult,
                    std::unique_ptr<RooWorkspace> workspace, const std::string& fitType = "") override {
        mockResults_[name] = MockResult{
            std::move(fitResult),
            std::move(workspace),
            fitType,
            1000.0, // mock signal yield
            50.0,   // mock signal error
            true    // mock good fit
        };
        storeCallCount_++;
    }
    
    FitResults* GetResult(const std::string& name) override {
        // Return nullptr for mock (simplified)
        return nullptr;
    }
    
    double GetSignalYield(const std::string& name) const override {
        auto it = mockResults_.find(name);
        return (it != mockResults_.end()) ? it->second.signalYield : 0.0;
    }
    
    double GetSignalYieldError(const std::string& name) const override {
        auto it = mockResults_.find(name);
        return (it != mockResults_.end()) ? it->second.signalError : 0.0;
    }
    
    bool IsGoodFit(const std::string& name, double maxChiSquare = 5.0) const override {
        auto it = mockResults_.find(name);
        return (it != mockResults_.end()) ? it->second.isGoodFit : false;
    }
    
    // Test helpers
    void SetMockResult(const std::string& name, double yield, double error, bool isGood) {
        mockResults_[name] = MockResult{nullptr, nullptr, "", yield, error, isGood};
    }
    
    int GetStoreCallCount() const { return storeCallCount_; }
    void ResetCallCount() { storeCallCount_ = 0; }
    bool HasResult(const std::string& name) const {
        return mockResults_.find(name) != mockResults_.end();
    }
    
private:
    struct MockResult {
        std::unique_ptr<RooFitResult> fitResult;
        std::unique_ptr<RooWorkspace> workspace;
        std::string fitType;
        double signalYield;
        double signalError;
        bool isGoodFit;
    };
    
    std::map<std::string, MockResult> mockResults_;
    int storeCallCount_ = 0;
};

// Test utility functions
namespace TestUtils {
    
    // Create a test data provider with specific characteristics
    std::unique_ptr<MockDataProvider> CreateTestDataProvider(int dataSize = 1000, 
                                                             double massMin = 1.8, 
                                                             double massMax = 2.0) {
        auto provider = std::make_unique<MockDataProvider>();
        provider->SetMassRange(massMin, massMax);
        provider->SetDatasetSize(dataSize);
        return provider;
    }
    
    // Create a test PDF factory
    std::unique_ptr<MockPDFFactory> CreateTestPDFFactory(RooRealVar* massVar) {
        return std::make_unique<MockPDFFactory>(massVar);
    }
    
    // Create a test fit strategy
    std::unique_ptr<MockFitStrategy> CreateTestFitStrategy(bool shouldFail = false) {
        auto strategy = std::make_unique<MockFitStrategy>();
        strategy->SetShouldFail(shouldFail);
        return strategy;
    }
    
    // Create a test result manager
    std::unique_ptr<MockResultManager> CreateTestResultManager() {
        return std::make_unique<MockResultManager>();
    }
    
    // Test assertion helpers
    void AssertEqual(double expected, double actual, double tolerance = 1e-6, 
                    const std::string& message = "") {
        if (std::abs(expected - actual) > tolerance) {
            std::string error = "Assertion failed: expected " + std::to_string(expected) + 
                               " but got " + std::to_string(actual);
            if (!message.empty()) {
                error += " (" + message + ")";
            }
            throw std::runtime_error(error);
        }
    }
    
    void AssertTrue(bool condition, const std::string& message = "") {
        if (!condition) {
            std::string error = "Assertion failed: condition is false";
            if (!message.empty()) {
                error += " (" + message + ")";
            }
            throw std::runtime_error(error);
        }
    }
    
    void AssertNotNull(const void* ptr, const std::string& message = "") {
        if (ptr == nullptr) {
            std::string error = "Assertion failed: pointer is null";
            if (!message.empty()) {
                error += " (" + message + ")";
            }
            throw std::runtime_error(error);
        }
    }
    
    // Test runner for executing test functions
    class TestRunner {
    public:
        using TestFunction = std::function<void()>;
        
        void AddTest(const std::string& name, TestFunction test) {
            tests_[name] = test;
        }
        
        bool RunAllTests() {
            int passed = 0;
            int total = tests_.size();
            
            std::cout << "Running " << total << " tests..." << std::endl;
            
            for (const auto& [name, test] : tests_) {
                try {
                    std::cout << "Running " << name << "... ";
                    test();
                    std::cout << "PASSED" << std::endl;
                    passed++;
                } catch (const std::exception& e) {
                    std::cout << "FAILED: " << e.what() << std::endl;
                } catch (...) {
                    std::cout << "FAILED: Unknown exception" << std::endl;
                }
            }
            
            std::cout << std::endl << "Results: " << passed << "/" << total << " tests passed" << std::endl;
            return passed == total;
        }
        
        bool RunTest(const std::string& name) {
            auto it = tests_.find(name);
            if (it == tests_.end()) {
                std::cout << "Test '" << name << "' not found" << std::endl;
                return false;
            }
            
            try {
                std::cout << "Running " << name << "... ";
                it->second();
                std::cout << "PASSED" << std::endl;
                return true;
            } catch (const std::exception& e) {
                std::cout << "FAILED: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "FAILED: Unknown exception" << std::endl;
                return false;
            }
        }
        
    private:
        std::map<std::string, TestFunction> tests_;
    };
}

// Dependency injection container for managing test dependencies
class TestDependencyContainer {
public:
    template<typename Interface, typename Implementation>
    void Register(std::unique_ptr<Implementation> implementation) {
        auto key = std::type_index(typeid(Interface));
        dependencies_[key] = std::move(implementation);
    }
    
    template<typename Interface>
    Interface* Get() {
        auto key = std::type_index(typeid(Interface));
        auto it = dependencies_.find(key);
        if (it != dependencies_.end()) {
            return static_cast<Interface*>(it->second.get());
        }
        return nullptr;
    }
    
    void Clear() {
        dependencies_.clear();
    }
    
private:
    std::map<std::type_index, std::unique_ptr<void, std::function<void(void*)>>> dependencies_;
};

// Integration test helpers for complete workflow testing
namespace IntegrationTestUtils {
    
    struct TestScenario {
        std::string name;
        std::string signalPdfType;
        std::string backgroundPdfType;
        std::string fitStrategy;
        int dataSize;
        bool expectSuccess;
        double expectedYield;
        double yieldTolerance;
    };
    
    bool RunScenario(const TestScenario& scenario, TestDependencyContainer& container) {
        try {
            std::cout << "Running scenario: " << scenario.name << std::endl;
            
            // Get dependencies
            auto dataProvider = container.Get<IDataProvider>();
            auto pdfFactory = container.Get<IPDFFactory>();
            auto fitStrategy = container.Get<IFitStrategy>();
            auto resultManager = container.Get<IResultManager>();
            
            TestUtils::AssertNotNull(dataProvider, "DataProvider not found");
            TestUtils::AssertNotNull(pdfFactory, "PDFFactory not found");
            TestUtils::AssertNotNull(fitStrategy, "FitStrategy not found");
            TestUtils::AssertNotNull(resultManager, "ResultManager not found");
            
            // Execute test workflow
            auto dataset = dataProvider->GetDataSet("test");
            auto massVar = dataProvider->GetMassVariable();
            
            auto signalPdf = pdfFactory->CreateSignalPDF(scenario.signalPdfType, nullptr);
            auto backgroundPdf = pdfFactory->CreateBackgroundPDF(scenario.backgroundPdfType, nullptr);
            
            TestUtils::AssertNotNull(signalPdf.get(), "Signal PDF creation failed");
            TestUtils::AssertNotNull(backgroundPdf.get(), "Background PDF creation failed");
            
            // Create combined PDF
            auto nsig = std::make_unique<RooRealVar>("nsig", "Signal yield", 1000, 0, 10000);
            auto nbkg = std::make_unique<RooRealVar>("nbkg", "Background yield", 1000, 0, 10000);
            
            auto totalPdf = std::make_unique<RooAddPdf>("total", "Total PDF",
                RooArgList(*signalPdf, *backgroundPdf), RooArgList(*nsig, *nbkg));
            
            // Perform fit
            FitConfig config;
            auto fitResult = fitStrategy->Execute(totalPdf.get(), dataset, config);
            
            // Store results
            resultManager->StoreResult(scenario.name, std::move(fitResult), nullptr, "test");
            
            // Validate results
            if (scenario.expectSuccess) {
                TestUtils::AssertTrue(resultManager->IsGoodFit(scenario.name), "Fit should succeed");
                
                if (scenario.expectedYield > 0) {
                    double yield = resultManager->GetSignalYield(scenario.name);
                    TestUtils::AssertEqual(scenario.expectedYield, yield, scenario.yieldTolerance,
                                         "Signal yield mismatch");
                }
            }
            
            std::cout << "Scenario " << scenario.name << " completed successfully" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "Scenario " << scenario.name << " failed: " << e.what() << std::endl;
            return false;
        }
    }
}

#endif // TEST_FRAMEWORK_H