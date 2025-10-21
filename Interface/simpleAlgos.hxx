#ifndef __SIMPLE_ALGOS_HXX
#define __SIMPLE_ALGOS_HXX

#include "TMath.h"
#include "Math/ProbFuncMathCore.h"

namespace algo{
    template <typename T>
    std::vector<int> sortIndex(const T v, size_t vsize){
        std::vector<int> idx(vsize);
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&v](int i1, int i2){return v[i1] < v[i2];});
        return idx;
    };
    double rapidity(double pt, double eta, double phi, double mass){
        return  ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<Double32_t>>(pt, eta, phi, mass).Rapidity();
        // return 0.5*log((sqrt(pt*pt*cosh(eta)*cosh(eta)+mass*mass)+pt*cosh(eta))/(sqrt(pt*pt*cosh(eta)*cosh(eta)+mass*mass)-pt*cosh(eta)));
    };
    double rapidity(TLorentzVector& v){
        return rapidity(v.Pt(), v.Eta(), v.Phi(), v.M());
    };
    double LLRTest(double nll1, int p1, double nll2, int p2){
            return ROOT::Math::chisquared_cdf(-2*(nll2 -nll1), p2-p1);
    };
    template <size_t n>
    unsigned int findBestFitInMaxtrix(int (&label) [n], double (&array) [n][n], unsigned int eval = 0){
        auto traverseIndex = [&](unsigned int i){
            for(auto idx : ROOT::TSeqI(i+1, n)){
                double p = LLRTest(array[i][i], label[i], array[i][idx], label[idx]);
                std::cout << Form("BKGLLR : i, idx : (%d, %d), nll1, nll2 : (%.3f, %.3f)", (int) i, idx, array[i][i], array[i][idx] ) << std::endl;
                if(p > 0.68){
                    return (unsigned int) idx;
                }
            }
            return i;
        };
        unsigned int tmp = traverseIndex(eval);
        if( tmp == eval || tmp >= n ) return eval;
        return findBestFitInMaxtrix<n>(label, array, tmp);
    };
    template<typename T>
    std::vector<T> getEqualRangeV(T lo, T hi, int nbins){
        T width = (hi - lo)/((double) nbins);
        std::vector<T> _res_ = {};
        _res_.reserve(nbins+1);
        for( auto _t_ = lo; _t_ < hi - (width/2.); _t_ += width){
            _res_.push_back(_t_);
        }
        _res_.push_back(hi);
        return _res_;
    };
};


template <typename T>
void scalePointAndError(T* gin, double scale){
    size_t numPoint = gin->GetN();
    double* point_y = gin->GetY();
    double* point_y_errlo = gin->GetEYlow();
    double* point_y_errhi = gin->GetEYhigh();
    std::cout << scale<< std::endl;
    for(unsigned int idx : ROOT::TSeqU(numPoint)){
        gin->SetPointY(idx, point_y[idx]/scale );
        gin->SetPointEYlow(idx, point_y_errlo[idx]/scale);
        gin->SetPointEYhigh(idx, point_y_errhi[idx]/scale);
        // std::cout << gin->GetPointX(idx) << ", before : " << point_y[idx] <<", after : " <<  point_y[idx]/scale << std::endl;
        // std::cout << gin->GetPointX(idx) << ", updated : " << gin->GetPointY(idx) << std::endl;
    }
};

template <typename T>
void scaleTGraph(T* gin, T* gout = nullptr) {
    if ( gout == nullptr) gout = gin;
    size_t numPoint = gin->GetN();
    double integral = gin->Integral();
    scalePointAndError<T>(gout, integral);

};

template <typename T>
T* getScaleTGraph(T* gin) {
    size_t numPoint = gin->GetN();
    double integral = 0 ;
    for(auto idx : ROOT::TSeqU(numPoint)){
        integral += gin->GetPointY(idx);
    }

    T* gout = (T*) gin->Clone(Form("clone_%s", gin->GetName()));
    scalePointAndError<T>(gout, integral);
    return gout;
};

#endif