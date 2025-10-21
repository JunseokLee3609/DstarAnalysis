#ifndef DATAPROCESSORBASE_H
#define DATAPROCESSORBASE_H

#include <string>
#include <iostream>
#include <memory>
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "FileManager.h"
#include "ConfigManager.h"
#include "CentralityUtils.h"
#include "VariableHandler.h"

class DataProcessorBase {
protected:
    ProcessingConfig config;
    std::unique_ptr<FileManager> fileManager;
    
    // Progress tracking
    long long totalProcessed = 0;
    long long totalEvents = 0;
    
    // Timing
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    
public:
    DataProcessorBase(const ProcessingConfig& cfg);
    virtual ~DataProcessorBase() = default;
    
    virtual void process() = 0;
    
    void setConfig(const ProcessingConfig& cfg);
    ProcessingConfig getConfig() const { return config; }
    
    void printProgress(long long currentEvent, long long totalEvents, int interval = 10000) const;
    void printTimingInfo() const;
    
protected:
    virtual bool initialize();
    virtual void finalize();
    
    virtual bool validateConfig() const;
    
    std::unique_ptr<TChain> createChain(const std::string& treeName, const std::string& inputPath);
    
    TFile* createOutputFile() const;
    
    void logInfo(const std::string& message) const;
    void logWarning(const std::string& message) const;
    void logError(const std::string& message) const;
    
    long long determineEventRange(long long totalAvailable) const;
    
    bool shouldProcessEvent(long long eventIdx) const;
};

DataProcessorBase::DataProcessorBase(const ProcessingConfig& cfg) 
    : config(cfg), fileManager(std::make_unique<FileManager>()) {
    startTime = std::chrono::high_resolution_clock::now();
}

void DataProcessorBase::setConfig(const ProcessingConfig& cfg) {
    config = cfg;
}

void DataProcessorBase::printProgress(long long currentEvent, long long totalEvents, int interval) const {
    if (currentEvent % interval == 0) {
        float percentage = totalEvents > 0 ? (100.0f * currentEvent / totalEvents) : 0.0f;
        std::cout << "Processing event " << currentEvent << " / " << totalEvents 
                  << " (" << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
    }
}

void DataProcessorBase::printTimingInfo() const {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    
    std::cout << "=== Processing Summary ===" << std::endl;
    std::cout << "Total time: " << duration << " seconds" << std::endl;
    std::cout << "Events processed: " << totalProcessed << " / " << totalEvents << std::endl;
    
    if (totalProcessed > 0 && duration > 0) {
        float rate = static_cast<float>(totalProcessed) / duration;
        std::cout << "Processing rate: " << std::fixed << std::setprecision(1) 
                  << rate << " events/second" << std::endl;
    }
    
    float efficiency = totalEvents > 0 ? (100.0f * totalProcessed / totalEvents) : 0.0f;
    std::cout << "Processing efficiency: " << std::fixed << std::setprecision(1) 
              << efficiency << "%" << std::endl;
    std::cout << "=========================" << std::endl;
}

bool DataProcessorBase::initialize() {
    logInfo("Initializing data processor...");
    
    if (!validateConfig()) {
        logError("Configuration validation failed");
        return false;
    }
    
    config.print();
    return true;
}

void DataProcessorBase::finalize() {
    logInfo("Finalizing data processor...");
    printTimingInfo();
}

bool DataProcessorBase::validateConfig() const {
    if (!config.validate()) {
        return false;
    }
    
    // Additional validation specific to data processing
    if (config.startEvent < 0) {
        logError("Start event cannot be negative");
        return false;
    }
    
    if (config.endEvent != -1 && config.endEvent <= config.startEvent) {
        logError("End event must be greater than start event or -1 for all events");
        return false;
    }
    
    return true;
}

std::unique_ptr<TChain> DataProcessorBase::createChain(const std::string& treeName, const std::string& inputPath) {
    auto chain = std::make_unique<TChain>(treeName.c_str());
    
    // Check if input is a file list or directory/file path
    if (inputPath.find(".txt") != std::string::npos || inputPath.find(".list") != std::string::npos) {
        // File list
        auto files = FileManager::readFileList(inputPath);
        if (files.empty()) {
            logError("No files found in file list: " + inputPath);
            return nullptr;
        }
        FileManager::fillChainFromFileList(chain.get(), files);
    } else {
        // Directory or file path
        FileManager::loadRootFilesRecursively(chain.get(), inputPath);
    }
    
    long long entries = chain->GetEntries();
    if (entries == 0) {
        logError("No entries found in chain for tree: " + treeName);
        return nullptr;
    }
    
    logInfo("Chain created with " + std::to_string(entries) + " entries from tree: " + treeName);
    return chain;
}

TFile* DataProcessorBase::createOutputFile() const {
    return FileManager::createOutputFile(config.outputPath, config.outputPrefix, 
                                       config.jobIdx, config.date);
}

void DataProcessorBase::logInfo(const std::string& message) const {
    std::cout << "[INFO] " << message << std::endl;
}

void DataProcessorBase::logWarning(const std::string& message) const {
    std::cout << "[WARNING] " << message << std::endl;
}

void DataProcessorBase::logError(const std::string& message) const {
    std::cerr << "[ERROR] " << message << std::endl;
}

long long DataProcessorBase::determineEventRange(long long totalAvailable) const {
    totalEvents = totalAvailable;
    
    if (config.maxEntries > 0 && config.maxEntries < totalAvailable) {
        totalEvents = config.maxEntries;
        logInfo("Limiting processing to " + std::to_string(config.maxEntries) + " events");
    }
    
    long long endEvent = (config.endEvent == -1) ? totalEvents : 
                        std::min(static_cast<long long>(config.endEvent), totalEvents);
    
    return endEvent;
}

bool DataProcessorBase::shouldProcessEvent(long long eventIdx) const {
    return (eventIdx >= config.startEvent && 
            (config.endEvent == -1 || eventIdx < config.endEvent));
}

#endif // DATAPROCESSORBASE_H