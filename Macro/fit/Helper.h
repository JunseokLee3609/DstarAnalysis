#ifndef HELPER_H_H
#define HELPER_H_H

#include <map>
#include <utility>
#include "Params.h"

using ParamKey = std::pair<double, double>;
using D0ParamValue = std::pair<PDFParams::GaussianParams, PDFParams::PolynomialBkgParams>;
using DStarParamValue1 = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarParamValue2 = std::pair<PDFParams::DoubleDBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using DStarMCParamValue = std::pair<PDFParams::DBCrystalBallParams, PDFParams::DBCrystalBallParams>;
using D0ParamMap = std::map<ParamKey, D0ParamValue>;
using DStarParamMap1 = std::map<ParamKey, DStarParamValue1>;
using DStarParamMap2 = std::map<ParamKey, DStarParamValue2>;
using DStarMCParamMap = std::map<ParamKey, DStarMCParamValue>;

TFile* createFileInDir(const std::string& dirPath, const std::string& filePath) {
    // 디렉토리 확인
    TSystemDirectory dir("dir", dirPath.c_str());
    void* dirCheck = gSystem->OpenDirectory(dirPath.c_str());

    if (!dirCheck) { // 폴더가 없으면 생성
        std::cout << "Directory does not exist. Creating: " << dirPath << std::endl;
        gSystem->MakeDirectory(dirPath.c_str());
    } else {
        std::cout << "Directory exists: " << dirPath << std::endl;
        gSystem->FreeDirectory(dirCheck);
    }

    // 새로운 TFile 생성
    TFile* file = TFile::Open((dirPath+filePath).c_str(), "RECREATE");
    if (file && file->IsOpen()) {
        std::cout << "Successfully created file: " << filePath << std::endl;
        // file->Close();
    } else {
        std::cerr << "Failed to create file: " << filePath << std::endl;
    }
    return file;
}
void createDir(const std::string& dirPath) {
    // 디렉토리 확인
    TSystemDirectory dir("dir", dirPath.c_str());
    void* dirCheck = gSystem->OpenDirectory(dirPath.c_str());

    if (!dirCheck) { // 폴더가 없으면 생성
        std::cout << "Directory does not exist. Creating: " << dirPath << std::endl;
        gSystem->MakeDirectory(dirPath.c_str());
    } else {
        std::cout << "Directory exists: " << dirPath << std::endl;
        gSystem->FreeDirectory(dirCheck);
    }
}

D0ParamMap D0ParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins, int order) {
    D0ParamMap params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            D0ParamValue value;
            double polyinit = 0.;
            double polymax = 1;
            double polymin = -1;
            for(int k =0; k < order; k++) {
                value.second.coefficients.push_back(polyinit);
                value.second.coef_max.push_back(polymax);
                value.second.coef_min.push_back(polymin);
            }
            params[key] = value;
        }
    }

    return params;
}
DStarParamMap1 DStarParamMaker1(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap1 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue1 value;
                value.first.mean = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                value.first.sigmaR = 0.0005;
                value.first.sigmaR_min = 0.0001;
                value.first.sigmaR_max = 0.01;
                value.first.sigmaL = 0.0005;
                value.first.sigmaL_min = 0.0001;
                value.first.sigmaL_max = 0.01;
                
                value.first.alphaL = 2.;
                value.first.alphaL_min = 0.0001;
                value.first.alphaL_max = 10;
                value.first.nL = 1.5;
                value.first.nL_min = 1;
                value.first.nL_max = 10;
            
                value.first.alphaR = 2.;
                value.first.alphaR_min = 0.0001;
                value.first.alphaR_max = 10.;
                value.first.nR = 1.5;
                value.first.nR_min = 1;
                value.first.nR_max = 10;
                value.second.p0 = 0.1;
                value.second.p0_min = 0.0;
                value.second.p0_max = 5.0;
                value.second.p1 = 2;
                value.second.p1_min = -10.0;
                value.second.p1_max = 10.0;
                value.second.p2 = -2;
                value.second.p2_min = -10.0;
                value.second.p2_max = 10.0;
            params[key] = value;
        }
    }

    return params;
}
DStarParamMap2 DStarParamMaker2(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap2 params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue2 value;
                value.first.mean1 = 0.1455;        // D* mass in GeV
                value.first.mean2 = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                value.first.sigmaR1 = 0.0005;
                value.first.sigmaR2 = 0.0005;
                value.first.sigmaR_min = 0.0001;
                value.first.sigmaR_max = 0.01;
                value.first.sigmaL1 = 0.0005;
                value.first.sigmaL2 = 0.0005;
                value.first.sigmaL_min = 0.0001;
                value.first.sigmaL_max = 0.01;
                
                value.first.alphaL1 = 2.;
                value.first.alphaL2 = 2.;
                value.first.alphaL_min = 0.005;
                value.first.alphaL_max = 10;
                value.first.nL1 = 1.5;
                value.first.nL2 = 1.5;
                value.first.nL_min = 1;
                value.first.nL_max = 10;
            
                value.first.alphaR1 = 2.;
                value.first.alphaR2 = 2.;
                value.first.alphaR_min = 0.005;
                value.first.alphaR_max = 20.;
                value.first.nR1 = 1.5;
                value.first.nR2 = 1.5;
                value.first.nR_min = 1;
                value.first.nR_max = 20;
                value.second.p0 = 0.1;
                value.second.p0_min = 0.0;
                value.second.p0_max = 5.0;
                value.second.p1 = 2;
                value.second.p1_min = -10.0;
                value.second.p1_max = 10.0;
                value.second.p2 = -2;
                value.second.p2_min = -10.0;
                value.second.p2_max = 10.0;
            params[key] = value;
        }
    }

    return params;
}
DStarMCParamMap DStarMCParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarMCParamMap params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarMCParamValue value;
                value.first.mean = 0.1455;        // D* mass in GeV
                value.first.mean_min = 0.1452;
                value.first.mean_max = 0.1458;
                value.first.sigmaR = 0.0005;
                value.first.sigmaR_min = 0.0001;
                value.first.sigmaR_max = 0.01;
                value.first.sigmaL = 0.0005;
                value.first.sigmaL_min = 0.0001;
                value.first.sigmaL_max = 0.01;
                
                value.first.alphaL = 2.;
                value.first.alphaL_min = 0.0001;
                value.first.alphaL_max = 10;
                value.first.nL = 1.5;
                value.first.nL_min = 1;
                value.first.nL_max = 10;
            
                value.first.alphaR = 2.;
                value.first.alphaR_min = 0.0001;
                value.first.alphaR_max = 10.;
                value.first.nR = 1.5;
                value.first.nR_min = 1;
                value.first.nR_max = 10;

                value.second.mean = 0.1455;        // D* mass in GeV
                value.second.mean_min = 0.1452;
                value.second.mean_max = 0.1458;
                value.second.sigmaR = 0.0005;
                value.second.sigmaR_min = 0.0001;
                value.second.sigmaR_max = 0.01;
                value.second.sigmaL = 0.0005;
                value.second.sigmaL_min = 0.0001;
                value.second.sigmaL_max = 0.01;
                
                value.second.alphaL = 2.;
                value.second.alphaL_min = 0.0001;
                value.second.alphaL_max = 10;
                value.second.nL = 1.5;
                value.second.nL_min = 1;
                value.second.nL_max = 10;
            
                value.second.alphaR = 2.;
                value.second.alphaR_min = 0.0001;
                value.second.alphaR_max = 10.;
                value.second.nR = 1.5;
                value.second.nR_min = 1;
                value.second.nR_max = 10;
                
            params[key] = value;
        }
    }

    return params;
}


#endif 