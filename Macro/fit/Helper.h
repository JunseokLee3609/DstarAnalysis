#ifndef HELPER_H_H
#define HELPER_H_H

#include <map>
#include <utility>
#include "Params.h"

using ParamKey = std::pair<double, double>;
using ParamValue = std::pair<PDFParams::GaussianParams, PDFParams::PolynomialBkgParams>;
using ParamMap = std::map<ParamKey, ParamValue>;

ParamMap ParamMaker(const std::vector<double>& ptBins, const std::vector<double>& etaBins) {
    ParamMap params;

    for (double pt : ptBins) {
        for (double eta : etaBins) {
            ParamKey key = std::make_pair(pt, eta);
            ParamValue value;
            // Initialize GaussianParams and PhenomenologicalParams
            // value.first.mean = pt * 0.1;
            // value.first.sigma = eta * 0.01;
            // value.second.mode = "exponential";
            params[key] = value;
        }
    }

    return params;
}

#endif 