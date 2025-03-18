#ifndef HELPER_H_H
#define HELPER_H_H

#include <map>
#include <utility>
#include "Params.h"

using ParamKey = std::pair<double, double>;
using D0ParamValue = std::pair<PDFParams::GaussianParams, PDFParams::PolynomialBkgParams>;
using DStarParamValue = std::pair<PDFParams::DBCrystalBallParams, PDFParams::PhenomenologicalParams>;
using D0ParamMap = std::map<ParamKey, D0ParamValue>;
using DStarParamMap = std::map<ParamKey, DStarParamValue>;

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
DStarParamMap DStarParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    DStarParamMap params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            DStarParamValue value;
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


#endif 