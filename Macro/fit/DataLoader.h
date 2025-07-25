#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include <map>

// Forward declarations for ROOT classes
class TFile;
class TTree;
class RooDataSet;
class RooWorkspace;

class DataLoader {
public:
    // Constructors & destructors
    DataLoader();
    explicit DataLoader(const std::string& filename);
    ~DataLoader();
    
    // Loading methods
    bool loadFile(const std::string& filename);
    bool loadTree(const std::string& treename);
    bool loadRooDataSet(const std::string& datasetname, const std::string& workspacename = "");
    
    // Data access methods
    TTree* getTree(const std::string& name = "") const;
    RooDataSet* getDataSet(const std::string& name = "") const;

    // Utility methods
    void clear();
    bool isLoaded() const;
    void print() const;
    
private:
    // Member variables
    std::unique_ptr<TFile> fFile;
    std::map<std::string, TTree*> fTrees;
    std::map<std::string, RooDataSet*> fDataSets;
    RooWorkspace* fWorkspace;
    
    bool fIsLoaded;
    std::string fCurrentHistName;
    std::string fCurrentTreeName;
    std::string fCurrentDataSetName;
    
    // Private helper methods
    void initializeDefaults();
};// Constructors & destructor implementations
DataLoader::DataLoader() {
    initializeDefaults();
}

DataLoader::DataLoader(const std::string& filename) {
    initializeDefaults();
    loadFile(filename);
}

DataLoader::~DataLoader() {
    clear();
}

// Initialize default values
void DataLoader::initializeDefaults() {
    fFile = nullptr;
    fWorkspace = nullptr;
    fIsLoaded = false;
    fCurrentHistName = "";
    fCurrentTreeName = "";
    fCurrentDataSetName = "";
}

// Loading methods implementations
bool DataLoader::loadFile(const std::string& filename) {
    // Clear any existing data
    clear();
    
    // Try to open the file
    fFile.reset(TFile::Open(filename.c_str()));
    if (!fFile || fFile->IsZombie()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    fIsLoaded = true;
    return true;
}

bool DataLoader::loadTree(const std::string& treename) {
    if (!fIsLoaded || !fFile) {
        std::cerr << "Error: No file loaded. Call loadFile() first." << std::endl;
        return false;
    }
    
    TTree* tree = nullptr;
    fFile->GetObject(treename.c_str(), tree);
    
    if (!tree) {
        std::cerr << "Error: Could not find tree " << treename << std::endl;
        return false;
    }
    
    fTrees[treename] = tree;
    fCurrentTreeName = treename;
    return true;
}

bool DataLoader::loadRooDataSet(const std::string& datasetname, const std::string& workspacename) {
    if (!fIsLoaded || !fFile) {
        std::cerr << "Error: No file loaded. Call loadFile() first." << std::endl;
        return false;
    }
    
    RooDataSet* dataset = nullptr;
    
    if (!workspacename.empty()) {
        // Try to get dataset from workspace
        fWorkspace = nullptr;
        fFile->GetObject(workspacename.c_str(), fWorkspace);
        
        if (!fWorkspace) {
            std::cerr << "Error: Could not find workspace : " << workspacename << std::endl;
            return false;
        }
        
        dataset = dynamic_cast<RooDataSet*>(fWorkspace->data(datasetname.c_str()));
        if (!dataset) {
            std::cerr << "Error: Could not find dataset : " << datasetname 
                      << " in workspace : " << workspacename << std::endl;
            return false;
        }
    } else {
        // Try to get dataset directly from file
        fFile->GetObject(datasetname.c_str(), dataset);
        
        if (!dataset) {
            std::cerr << "Error: Could not find dataset : " << datasetname << std::endl;
            return false;
        }
    }
    
    fDataSets[datasetname] = dataset;
    fCurrentDataSetName = datasetname;
    return true;
}

// Data access methods implementations
TTree* DataLoader::getTree(const std::string& name) const {
    // If name is empty, return the current tree
    std::string treename = name.empty() ? fCurrentTreeName : name;
    
    if (treename.empty()) {
        std::cerr << "Error: No tree name specified and no current tree." << std::endl;
        return nullptr;
    }
    
    auto iter = fTrees.find(treename);
    if (iter == fTrees.end()) {
        std::cerr << "Error: Tree " << treename << " not loaded." << std::endl;
        return nullptr;
    }
    
    return iter->second;
}

RooDataSet* DataLoader::getDataSet(const std::string& name) const {
    // If name is empty, return the current dataset
    std::string datasetname = name.empty() ? fCurrentDataSetName : name;
    
    if (datasetname.empty()) {
        std::cerr << "Error: No dataset name specified and no current dataset." << std::endl;
        return nullptr;
    }
    
    auto iter = fDataSets.find(datasetname);
    if (iter == fDataSets.end()) {
        std::cerr << "Error: Dataset " << datasetname << " not loaded." << std::endl;
        return nullptr;
    }
    
    return iter->second;
}

// Utility methods implementations
void DataLoader::clear() {
    // Close file (which will delete trees owned by the file)
    if (fFile) {
        fFile->Close();
        fFile.reset();
    }
    
    // Clear containers (trees/datasets were owned by file, not us)
    fTrees.clear();
    fDataSets.clear();
    
    fWorkspace = nullptr;
    fIsLoaded = false;
    fCurrentHistName = "";
    fCurrentTreeName = "";
    fCurrentDataSetName = "";
}

bool DataLoader::isLoaded() const {
    return fIsLoaded;
}

void DataLoader::print() const {
    if (!fIsLoaded) {
        std::cout << "No file currently loaded." << std::endl;
        return;
    }
    
    std::cout << "Currently loaded file: " << fFile->GetName() << std::endl;
    
    std::cout << "\nLoaded trees:" << std::endl;
    if (fTrees.empty()) {
        std::cout << "  None" << std::endl;
    } else {
        for (const auto& pair : fTrees) {
            std::cout << "  " << pair.first;
            if (pair.first == fCurrentTreeName) std::cout << " (current)";
            std::cout << std::endl;
        }
    }
    
    std::cout << "\nLoaded datasets:" << std::endl;
    if (fDataSets.empty()) {
        std::cout << "  None" << std::endl;
    } else {
        for (const auto& pair : fDataSets) {
            std::cout << "  " << pair.first;
            if (pair.first == fCurrentDataSetName) std::cout << " (current)";
            std::cout << " - Entries: " << pair.second->numEntries() << std::endl;
        }
    }
    
    std::cout << std::endl;
}

#endif // DATA_LOADER_H