#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <vector>
#include <iostream>
#include "TFile.h"
#include "TChain.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TList.h"

class FileManager {
public:
    static TFile* createOutputFile(const std::string& outputPath, const std::string& prefix, int jobIdx, const std::string& date = "");
    
    static TFile* createFileInDir(const std::string& dirPath, const std::string& filePath);
    
    static void loadRootFilesRecursively(TChain* chain, const std::string& folderPath);
    
    static void fillChainFromFileList(TChain* chain, const std::vector<std::string>& files);
    
    static std::vector<std::string> readFileList(const std::string& fileListPath);
    
    static bool ensureDirectoryExists(const std::string& dirPath);
    
    static std::string generateOutputFileName(const std::string& prefix, int jobIdx, const std::string& date, const std::string& extension = ".root");
    
private:
    static void addRootFilesFromDirectory(TChain* chain, const std::string& dirPath);
};

TFile* FileManager::createOutputFile(const std::string& outputPath, const std::string& prefix, int jobIdx, const std::string& date) {
    if (!ensureDirectoryExists(outputPath)) {
        std::cerr << "Error: Could not create output directory: " << outputPath << std::endl;
        return nullptr;
    }
    
    std::string path = outputPath;
    if (!path.empty() && path.back() != '/') path += '/';
    
    std::string fileName = generateOutputFileName(prefix, jobIdx, date);
    std::string fullPath = path + fileName;
    
    TFile* fout = new TFile(fullPath.c_str(), "RECREATE");
    if (fout && fout->IsOpen()) {
        std::cout << "Output file created: " << fullPath << std::endl;
    } else {
        std::cerr << "Error: Failed to create output file: " << fullPath << std::endl;
        delete fout;
        return nullptr;
    }
    return fout;
}

TFile* FileManager::createFileInDir(const std::string& dirPath, const std::string& filePath) {
    if (!ensureDirectoryExists(dirPath)) {
        std::cerr << "Error: Could not create directory: " << dirPath << std::endl;
        return nullptr;
    }
    
    std::string fullPath = dirPath;
    if (!fullPath.empty() && fullPath.back() != '/') fullPath += '/';
    fullPath += filePath;
    
    TFile* file = TFile::Open(fullPath.c_str(), "RECREATE");
    if (file && file->IsOpen()) {
        std::cout << "File created successfully: " << fullPath << std::endl;
    } else {
        std::cerr << "Error: Failed to create file: " << fullPath << std::endl;
    }
    return file;
}

void FileManager::loadRootFilesRecursively(TChain* chain, const std::string& folderPath) {
    if (folderPath.find(".root") != std::string::npos) {
        std::cout << "Adding file directly: " << folderPath << std::endl;
        chain->Add(folderPath.c_str());
        return;
    }
    
    addRootFilesFromDirectory(chain, folderPath);
}

void FileManager::addRootFilesFromDirectory(TChain* chain, const std::string& dirPath) {
    TSystemDirectory dir(dirPath.c_str(), dirPath.c_str());
    TList* files = dir.GetListOfFiles();
    if (!files) return;
    
    TIter next(files);
    TSystemFile* file;
    
    while ((file = (TSystemFile*)next())) {
        std::string fileName = file->GetName();
        if (fileName == "." || fileName == "..") continue;
        
        std::string fullPath = dirPath + "/" + fileName;
        
        if (file->IsDirectory()) {
            addRootFilesFromDirectory(chain, fullPath);
        } else if (fileName.find(".root") != std::string::npos) {
            std::cout << "Adding: " << fullPath << std::endl;
            chain->Add(fullPath.c_str());
        }
    }
}

void FileManager::fillChainFromFileList(TChain* chain, const std::vector<std::string>& files) {
    for (const auto& file : files) {
        chain->Add(file.c_str());
    }
}

std::vector<std::string> FileManager::readFileList(const std::string& fileListPath) {
    std::vector<std::string> files;
    std::ifstream inputFile(fileListPath);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file list: " << fileListPath << std::endl;
        return files;
    }
    
    std::string line;
    std::cout << "Reading file list: " << fileListPath << std::endl;
    while (getline(inputFile, line)) {
        if (!line.empty()) {
            std::cout << "  " << line << std::endl;
            files.push_back(line);
        }
    }
    inputFile.close();
    
    std::cout << "Total files loaded: " << files.size() << std::endl;
    return files;
}

bool FileManager::ensureDirectoryExists(const std::string& dirPath) {
    void* dirCheck = gSystem->OpenDirectory(dirPath.c_str());
    if (!dirCheck) {
        std::cout << "Creating directory: " << dirPath << std::endl;
        if (gSystem->MakeDirectory(dirPath.c_str()) != 0) {
            return false;
        }
    } else {
        gSystem->FreeDirectory(dirCheck);
    }
    return true;
}

std::string FileManager::generateOutputFileName(const std::string& prefix, int jobIdx, const std::string& date, const std::string& extension) {
    std::string fileName = prefix;
    if (jobIdx >= 0) {
        fileName += "_" + std::to_string(jobIdx);
    }
    if (!date.empty()) {
        fileName += "_" + date;
    }
    fileName += extension;
    return fileName;
}

#endif // FILEMANAGER_H