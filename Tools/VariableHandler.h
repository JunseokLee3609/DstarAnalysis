#ifndef VARIABLEHANDLER_H
#define VARIABLEHANDLER_H

#include <string>
#include <vector>
#include <map>
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooFormulaVar.h"
#include "RooArgSet.h"
#include "TTree.h"
#include "TMath.h"

enum class VarType {
    FLOAT,
    SHORT,
    INT,
    BOOL,
    FORMULA
};

struct CategoryState {
    std::string name;
    int value;
};

struct VarDef {
    std::string name;
    VarType type;
    double min;
    double max;
    std::vector<CategoryState> states;
    std::string formula;
    std::vector<std::string> dependVars;

    VarDef(const std::string& n, VarType t, double mn, double mx) 
        : name(n), type(t), min(mn), max(mx) {}
    
    VarDef(const std::string& n, VarType t = VarType::BOOL) 
        : name(n), type(t), min(0), max(1) {
        if (t == VarType::BOOL) {
            states.push_back({"False", 0});
            states.push_back({"True", 1});
        }
    }
    
    VarDef(const std::string& n, const std::string& f, const std::vector<std::string>& deps)
        : name(n), type(VarType::FORMULA), min(0), max(0), formula(f), dependVars(deps) {}
};

class VariableHandler {
public:
    static std::vector<VarDef> createStandardVariables(bool isMC = true, bool isD0 = false, bool isPP = true);
    
    static std::vector<VarDef> createD0Variables(bool isMC = true, bool isPP = true);
    
    static std::vector<VarDef> createDStarVariables(bool isMC = true, bool isPP = true);
    
    static void setupBranches(TTree* tree, const std::vector<VarDef>& variables,
                             std::map<std::string, float>& floatMap,
                             std::map<std::string, short>& shortMap,
                             std::map<std::string, int>& intMap,
                             std::map<std::string, bool>& boolMap);
    
    static RooArgSet createRooVarSet(const std::vector<VarDef>& variables,
                                    std::map<std::string, RooAbsArg*>& varMap);
    
    static void setVariableValues(const std::vector<VarDef>& variables,
                                 const std::map<std::string, RooAbsArg*>& varMap,
                                 const std::map<std::string, float>& floatMap,
                                 const std::map<std::string, short>& shortMap,
                                 const std::map<std::string, int>& intMap,
                                 const std::map<std::string, bool>& boolMap,
                                 bool& skipEntry);
    
    static bool validateRequiredBranches(TTree* tree, const std::vector<VarDef>& variables, 
                                        bool needsCS = false, bool needsHX = false);

private:
    static void addCommonVariables(std::vector<VarDef>& variables);
    static void addMCVariables(std::vector<VarDef>& variables);
    static void addPbPbVariables(std::vector<VarDef>& variables);
    static void addD0SpecificVariables(std::vector<VarDef>& variables);
    static void addDStarSpecificVariables(std::vector<VarDef>& variables);
};

std::vector<VarDef> VariableHandler::createStandardVariables(bool isMC, bool isD0, bool isPP) {
    if (isD0) {
        return createD0Variables(isMC, isPP);
    } else {
        return createDStarVariables(isMC, isPP);
    }
}

std::vector<VarDef> VariableHandler::createD0Variables(bool isMC, bool isPP) {
    std::vector<VarDef> variables;
    
    addCommonVariables(variables);
    addD0SpecificVariables(variables);
    
    if (isMC) {
        addMCVariables(variables);
    }
    
    if (!isPP) {
        addPbPbVariables(variables);
    }
    
    return variables;
}

std::vector<VarDef> VariableHandler::createDStarVariables(bool isMC, bool isPP) {
    std::vector<VarDef> variables;
    
    addCommonVariables(variables);
    addDStarSpecificVariables(variables);
    
    if (isMC) {
        addMCVariables(variables);
    }
    
    if (!isPP) {
        addPbPbVariables(variables);
    }
    
    return variables;
}

void VariableHandler::addCommonVariables(std::vector<VarDef>& variables) {
    variables.push_back({"mass", VarType::FLOAT, 1.7, 2.25});
    variables.push_back({"pT", VarType::FLOAT, 0.0, 50.0});
    variables.push_back({"eta", VarType::FLOAT, -2.5, 2.5});
    variables.push_back({"phi", VarType::FLOAT, -TMath::Pi(), TMath::Pi()});
    variables.push_back({"y", VarType::FLOAT, -1, 1});
    variables.push_back({"dca3D", VarType::FLOAT, 0, 10});
}

void VariableHandler::addMCVariables(std::vector<VarDef>& variables) {
    variables.push_back({"isSwap", VarType::BOOL});
    variables.push_back({"isMC", VarType::BOOL});
    variables.push_back({"matchGEN", VarType::BOOL});
    variables.push_back({"matchGen_D1ancestorFlavor_", VarType::INT, 0, 10});
}

void VariableHandler::addPbPbVariables(std::vector<VarDef>& variables) {
    variables.push_back({"centrality", VarType::SHORT, 0, 200});
    variables.push_back({"Centrality", "centrality/2", {"centrality"}});
    variables.push_back({"mva", VarType::FLOAT, 0.2, 1});
}

void VariableHandler::addD0SpecificVariables(std::vector<VarDef>& variables) {
    // Add D0-specific variables here if needed
}

void VariableHandler::addDStarSpecificVariables(std::vector<VarDef>& variables) {
    variables.push_back({"massDaugther1", VarType::FLOAT, 1.7, 2.1});
    variables.push_back({"massPion", "mass - massDaugther1", {"mass", "massDaugther1"}});
    variables.push_back({"pTD1", VarType::FLOAT, 0.0, 50.0});
    variables.push_back({"EtaD1", VarType::FLOAT, -2.5, 2.5});
    variables.push_back({"PhiD1", VarType::FLOAT, -TMath::Pi(), TMath::Pi()});
    variables.push_back({"pTD2", VarType::FLOAT, 0.0, 50.0});
    variables.push_back({"EtaD2", VarType::FLOAT, -2.5, 2.5});
    variables.push_back({"pTGrandD1", VarType::FLOAT, 0.0, 50.0});
    variables.push_back({"EtaGrandD1", VarType::FLOAT, -2.5, 2.5});
    variables.push_back({"pTGrandD2", VarType::FLOAT, 0.0, 50.0});
    variables.push_back({"EtaGrandD2", VarType::FLOAT, -2.5, 2.5});
}

void VariableHandler::setupBranches(TTree* tree, const std::vector<VarDef>& variables,
                                   std::map<std::string, float>& floatMap,
                                   std::map<std::string, short>& shortMap,
                                   std::map<std::string, int>& intMap,
                                   std::map<std::string, bool>& boolMap) {
    for (const auto& varDef : variables) {
        if (varDef.type == VarType::FORMULA) continue;
        
        switch (varDef.type) {
            case VarType::FLOAT:
                floatMap[varDef.name] = 0.0f;
                tree->SetBranchAddress(varDef.name.c_str(), &floatMap[varDef.name]);
                break;
            case VarType::SHORT:
                shortMap[varDef.name] = 0;
                tree->SetBranchAddress(varDef.name.c_str(), &shortMap[varDef.name]);
                break;
            case VarType::INT:
                intMap[varDef.name] = 0;
                tree->SetBranchAddress(varDef.name.c_str(), &intMap[varDef.name]);
                break;
            case VarType::BOOL:
                boolMap[varDef.name] = false;
                tree->SetBranchAddress(varDef.name.c_str(), &boolMap[varDef.name]);
                break;
            default:
                break;
        }
    }
}

RooArgSet VariableHandler::createRooVarSet(const std::vector<VarDef>& variables,
                                          std::map<std::string, RooAbsArg*>& varMap) {
    RooArgSet varSet;
    
    for (const auto& varDef : variables) {
        if (varDef.type == VarType::FORMULA) continue;
        
        switch (varDef.type) {
            case VarType::FLOAT:
            case VarType::SHORT:
            case VarType::INT: {
                RooRealVar* var = new RooRealVar(varDef.name.c_str(), varDef.name.c_str(), 
                                                varDef.min, varDef.max);
                varSet.add(*var);
                varMap[varDef.name] = var;
                break;
            }
            case VarType::BOOL: {
                RooCategory* var = new RooCategory(varDef.name.c_str(), varDef.name.c_str());
                var->defineType("False", 0);
                var->defineType("True", 1);
                varSet.add(*var);
                varMap[varDef.name] = var;
                break;
            }
            default:
                break;
        }
    }
    
    return varSet;
}

void VariableHandler::setVariableValues(const std::vector<VarDef>& variables,
                                       const std::map<std::string, RooAbsArg*>& varMap,
                                       const std::map<std::string, float>& floatMap,
                                       const std::map<std::string, short>& shortMap,
                                       const std::map<std::string, int>& intMap,
                                       const std::map<std::string, bool>& boolMap,
                                       bool& skipEntry) {
    skipEntry = false;
    
    for (const auto& varDef : variables) {
        if (varDef.type == VarType::FORMULA) continue;
        
        auto varIt = varMap.find(varDef.name);
        if (varIt == varMap.end()) continue;
        
        RooAbsArg* var = varIt->second;
        
        switch (varDef.type) {
            case VarType::FLOAT: {
                auto it = floatMap.find(varDef.name);
                if (it != floatMap.end()) {
                    float value = it->second;
                    RooRealVar* realVar = static_cast<RooRealVar*>(var);
                    if (value < realVar->getMin() || value >= realVar->getMax()) {
                        skipEntry = true;
                        return;
                    }
                    realVar->setVal(value);
                }
                break;
            }
            case VarType::SHORT: {
                auto it = shortMap.find(varDef.name);
                if (it != shortMap.end()) {
                    short value = it->second;
                    RooRealVar* realVar = static_cast<RooRealVar*>(var);
                    if (value < realVar->getMin() || value >= realVar->getMax()) {
                        skipEntry = true;
                        return;
                    }
                    realVar->setVal(value);
                }
                break;
            }
            case VarType::INT: {
                auto it = intMap.find(varDef.name);
                if (it != intMap.end()) {
                    int value = it->second;
                    RooRealVar* realVar = static_cast<RooRealVar*>(var);
                    if (value < realVar->getMin() || value >= realVar->getMax()) {
                        skipEntry = true;
                        return;
                    }
                    realVar->setVal(value);
                }
                break;
            }
            case VarType::BOOL: {
                auto it = boolMap.find(varDef.name);
                if (it != boolMap.end()) {
                    bool value = it->second;
                    RooCategory* catVar = static_cast<RooCategory*>(var);
                    catVar->setIndex(value);
                }
                break;
            }
            default:
                break;
        }
    }
}

bool VariableHandler::validateRequiredBranches(TTree* tree, const std::vector<VarDef>& variables,
                                              bool needsCS, bool needsHX) {
    std::vector<std::string> requiredBranches;
    
    if (needsCS || needsHX) {
        requiredBranches = {"pT", "eta", "phi", "mass", 
                           "pTD1", "EtaD1", "PhiD1", "massDaugther1", 
                           "pTD2", "EtaD2", "PhiD2"};
        
        for (const auto& reqBranch : requiredBranches) {
            auto it = std::find_if(variables.begin(), variables.end(), 
                [&reqBranch](const VarDef& v) { 
                    return v.name == reqBranch && v.type == VarType::FLOAT; 
                });
            if (it == variables.end()) {
                if (!tree->GetBranch(reqBranch.c_str())) {
                    std::cerr << "Error: Required branch '" << reqBranch 
                              << "' not found for frame calculations." << std::endl;
                    return false;
                }
            }
        }
    }
    
    return true;
}

#endif // VARIABLEHANDLER_H