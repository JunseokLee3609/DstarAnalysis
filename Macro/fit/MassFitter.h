#ifndef MASS_FITTER_H
#define MASS_FITTER_H

#include <string>
#include <map>
#include <memory>
#include "TCanvas.h"
#include "TFile.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooAddPdf.h"
#include "RooFitResult.h"
#include "RooWorkspace.h"
#include "RooPlot.h"
#include "RooGaussian.h"
#include "RooChebychev.h"
#include "RooPolynomial.h"
#include "RooExponential.h"
#include "RooCBShape.h"
#include "RooCrystalBall.h"
#include "RooVoigtian.h"
#include "RooFormulaVar.h"
#include "RooGenericPdf.h"
#include "Params.h"
#include "Opt.h"
#include "../Tools/Style/CMS_lumi.h"
#include "Helper.h"

// 상수 정의
constexpr double PION_MASS = 0.13957039;
constexpr double DELTAMASS = 0.1454257;

// 외부 파라미터 구조체 사용을 위한 타입 별칭 정의
using GaussianParams = PDFParams::GaussianParams;
using DoubleGaussianParams = PDFParams::DoubleGaussianParams;
using CrystalBallParams = PDFParams::CrystalBallParams;
using DBCrystalBallParams = PDFParams::DBCrystalBallParams;
using DoubleDBCrystalBallParams = PDFParams::DoubleDBCrystalBallParams;
using VoigtianParams = PDFParams::VoigtianParams;
using BreitWignerParams = PDFParams::BreitWignerParams;
using PhenomenologicalParams = PDFParams::PhenomenologicalParams;
using ExponentialBkgParams = PDFParams::ExponentialBkgParams;
using ChebychevBkgParams = PDFParams::ChebychevBkgParams;
using PolynomialBkgParams = PDFParams::PolynomialBkgParams;

class MassFitter {
public:
    // 생성자 & 소멸자
    MassFitter(const std::string& name,std::string& massvar, double massMin, double massMax);
    ~MassFitter();
    
    
    // 데이터 설정
    template <typename SigPar, typename BkgPar>
    void PerformFit(FitOpt opt, RooDataSet* dataset, bool inclusive, const std::string pTbin,const std::string etabin, SigPar sigParams, BkgPar bkgParams);
    template <typename SigMCPar>
    void PerformMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams);
    template <typename SigMCPar, typename SigPar, typename BkgPar>
    void PerformConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams, SigPar sigParams, BkgPar bkgParams);
    template <typename SigPar, typename BkgPar>
    void PerformConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigPar sigParams, BkgPar bkgParams);
    template <typename SigMCSwap1Par, typename SigMCSwap2Par>
    void PerformSwapMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCSwap1Par sigParams1, SigMCSwap2Par sigParams2);
    void ApplyCut(const std::string& cutExpr);
    void SetData(RooDataSet* dataset);
    
    // Signal PDF 설정 메서드
    void SetSignalPDF(const GaussianParams& params,const std::string &name);
    void SetSignalPDF(const DoubleGaussianParams& params, const std::string &name);
    void SetSignalPDF(const CrystalBallParams& params, const std::string &name);
    void SetSignalPDF(const DBCrystalBallParams& params, const std::string &name);
    void SetSignalPDF(const DoubleDBCrystalBallParams& params, const std::string &name);
    void SetSignalPDF(const VoigtianParams& params, const std::string &name);
    
    // Background PDF 설정 메서드
    void SetBackgroundPDF(const ExponentialBkgParams& params, const std::string& name);
    void SetBackgroundPDF(const ChebychevBkgParams& params, const std::string& name);
    void SetBackgroundPDF(const PolynomialBkgParams& params, const std::string& name);
    void SetBackgroundPDF(const PhenomenologicalParams& params, const std::string &name);
    
    // 피팅 수행
    RooFitResult* Fit(bool useMinos, bool useHesse, bool verbose ,bool useCUDA, double rangeMin , double rangeMax);
    RooFitResult* ConstraintFit(std::string fileMCPath,std::string fileMCFile, vector<std::string> constraintParameters, bool useMinos, bool useHesse, bool verbose, bool useCUDA, double rangeMin, double rangeMax);
    bool MapAndFixParametersFromMCSwap(
        const std::string& mcDirPath,
        const std::string& mcSwap0File,
        const std::string& mcSwap1File,
        const std::vector<std::string>& parameterNames
    );
    
    // 결과 접근 메서드
    double GetSignalYield() const;
    double GetSignalYieldError() const;
    double GetBackgroundYield() const;
    double GetBackgroundYieldError() const;
    double GetChiSquare() const;
    double GetNDF() const;
    double GetReducedChiSquare() const;
    
    // 저장 메서드
    void SaveResults(const std::string& filePath,const std::string& fileName,bool saveWorkspace = false);
    void SaveMCResults(const std::string& filePath,const std::string& fileName, bool saveWorkspace =true);
    RooWorkspace* GetWorkspace() const { return workspace_; } 
    // void SaveSPlotResults(const std::string& filePath,const std::string& fileName, bool saveWorkspace =true);
    
    // Delta mass 모드 설정 (D* 분석용)
    void UseDeltaMass(bool use = true, double daughterMassMin = 1.8, double daughterMassMax = 1.9);
    
private:
    // 내부 PDF 생성 메서드 (RooAbsPdf* 반환하도록 수정)
    RooAbsPdf* MakeGaussian(const GaussianParams& params, const std::string& name = "gauss");
    RooAbsPdf* MakeDoubleGaussian(const DoubleGaussianParams& params, const std::string& name = "doublegauss");
    RooAbsPdf* MakeCrystalBall(const CrystalBallParams& params, const std::string& name = "cb");
    RooAbsPdf* MakeDBCrystalBall(const DBCrystalBallParams& params, const std::string& name = "doublecb");
    RooAbsPdf* MakeDoubleDBCrystalBall(const DoubleDBCrystalBallParams& params, const std::string& name = "doublecb");
    RooAbsPdf* MakeVoigtian(const VoigtianParams& params, const std::string& name = "voigt");

    RooAbsPdf* MakeExponential(const ExponentialBkgParams& params, const std::string& name = "exp");
    RooAbsPdf* MakeChebychev(const ChebychevBkgParams& params, const std::string& name = "cheb");
    RooAbsPdf* MakePolynomial(const PolynomialBkgParams& params, const std::string& name = "poly");
    RooAbsPdf* MakeRooDstD0Bg(const PhenomenologicalParams& params, const std::string& name = "dstd0");

    // 전체 PDF 생성
    void CombinePDFs();
    void CombineMCPDFs();
    void CombinePDFs(RooAbsPdf* signal_pdf, RooAbsPdf* background_pdf);
    void CombinePDFs(RooAbsPdf* signal_pdf_swap0,RooAbsPdf* signal_pdf_swap1, RooAbsPdf* background_pdf);
    void Clear();

    // 멤버 변수들
    std::string name_;                   // 인스턴스 이름
    std::string massVar_;                // 질량 변수 이름
    bool useMVA_;                        // MVA 사용 여부
    bool useDeltaMass_;                  // Delta mass 모드 여부

    // RooFit 변수들
    RooRealVar* mass_;                   // 주 질량 변수
    RooRealVar* massDaughter1_;          // 딸 입자 질량 변수
    RooRealVar* massPion_;               // 파이온 질량 변수
    RooRealVar* activeMassVar_;          // 활성 질량 변수 포인터

    // PDF 객체들
    RooAbsPdf* signal_pdf_;              // 신호 PDF
    RooAbsPdf* swap0_pdf_;              // 신호 PDF
    RooAbsPdf* swap1_pdf_;              // 신호 PDF
    RooAbsPdf* background_pdf_;          // 배경 PDF
    RooAddPdf* total_pdf_;               // 전체 PDF (신호 + 배경)

    // 데이터 객체들
    RooDataSet* full_data_;              // 전체 데이터셋
    RooDataSet* reduced_data_;           // 컷 적용된 데이터셋

    // 결과 객체들
    RooFitResult* fit_result_;           // 피팅 결과
    RooWorkspace* workspace_;            // 작업공간 (선택적으로 저장)

    // 신호 및 배경 수득률 변수들
    RooRealVar* nsig_;                   // 신호 수득률
    RooRealVar* nbkg_;                   // 배경 수득률
    RooRealVar* frac_;                   // 배경 수득률
    RooFormulaVar* nSigSwap_;

    // PDF 파라미터 보관용 맵
    std::map<std::string, RooRealVar*> parameters_;
};


RooAbsPdf* MassFitter::MakeGaussian(const GaussianParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr; // 활성 변수 확인

    RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma = new RooRealVar(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);

    // 생성된 PDF 포인터 반환
    return new RooGaussian(name.c_str(), ("Gaussian_" + name).c_str(),
                          *activeMassVar_, *mean, *sigma);
}

RooAbsPdf* MassFitter::MakeDoubleGaussian(const DoubleGaussianParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;

    RooRealVar* mean1 = new RooRealVar(("mean1_" + name).c_str(), "First Mean",
                                      params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma1 = new RooRealVar(("sigma1_" + name).c_str(), "First Sigma",
                                       params.sigma1, params.sigma1_min, params.sigma1_max);
    RooRealVar* mean2 = new RooRealVar(("mean2_" + name).c_str(), "Second Mean",
                                      params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma2 = new RooRealVar(("sigma2_" + name).c_str(), "Second Sigma",
                                       params.sigma2, params.sigma2_min, params.sigma2_max);
    RooRealVar* frac = new RooRealVar(("frac_" + name).c_str(), "Fraction",
                                     params.fraction, params.fraction_min, params.fraction_max);

    RooGaussian* gauss1 = new RooGaussian(("gauss1_" + name).c_str(), "First Gaussian",
                                         *activeMassVar_, *mean1, *sigma1);
    RooGaussian* gauss2 = new RooGaussian(("gauss2_" + name).c_str(), "Second Gaussian",
                                         *activeMassVar_, *mean1, *sigma2); // Note: mean1 is used for both

    // 생성된 PDF 포인터 반환
    return new RooAddPdf(name.c_str(), ("DoubleGaussian_" + name).c_str(),
                        RooArgList(*gauss1, *gauss2), *frac);
}

RooAbsPdf* MassFitter::MakeCrystalBall(const CrystalBallParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;

    RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma = new RooRealVar(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    RooRealVar* alpha = new RooRealVar(("alpha_" + name).c_str(), "Alpha",
                                      params.alpha, params.alpha_min, params.alpha_max);
    RooRealVar* n = new RooRealVar(("n_" + name).c_str(), "N",
                                  params.n, params.n_min, params.n_max);

    // 생성된 PDF 포인터 반환
    return new RooCBShape(name.c_str(), ("CrystalBall_" + name).c_str(),
                         *activeMassVar_, *mean, *sigma, *alpha, *n);
}

RooAbsPdf* MassFitter::MakeDBCrystalBall(const DBCrystalBallParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;

    RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigmaL = new RooRealVar(("sigmaL_" + name).c_str(), "sigmaL",
                                      params.sigmaL, params.sigmaL_min, params.sigmaL_max);
    RooRealVar* sigmaR = new RooRealVar(("sigmaR_" + name).c_str(), "sigmaR",
                                      params.sigmaR, params.sigmaR_min, params.sigmaR_max);
    RooRealVar* alphaL = new RooRealVar(("alphaL_" + name).c_str(), "AlphaL",
                                       params.alphaL, params.alphaL_min, params.alphaL_max);
    RooRealVar* nL = new RooRealVar(("nL_" + name).c_str(), "NL",
                                   params.nL, params.nL_min, params.nL_max);
    RooRealVar* alphaR = new RooRealVar(("alphaR_" + name).c_str(), "AlphaR",
                                       params.alphaR, params.alphaR_min, params.alphaR_max);
    RooRealVar* nR = new RooRealVar(("nR_" + name).c_str(), "NR",
                                   params.nR, params.nR_min, params.nR_max);

    // 생성된 PDF 포인터 반환
    return new RooCrystalBall(name.c_str(), ("DoubleSidedCrystalBall_" + name).c_str(),
                                  *activeMassVar_, *mean, *sigmaL,*sigmaR, *alphaL, *nL, *alphaR, *nR);
}

RooAbsPdf* MassFitter::MakeDoubleDBCrystalBall(const DoubleDBCrystalBallParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;

    // RooRealVar* mean1 = new RooRealVar(("mean1_" + name).c_str(), "Mean",
    //                                  params.mean1, params.mean_min, params.mean_max);
    // RooRealVar* sigmaL1 = new RooRealVar(("sigmaL1_" + name).c_str(), "sigmaL",
    // if (!activeMassVar_) return nullptr;
    
    RooRealVar* mean1 = new RooRealVar(("mean1_" + name).c_str(), "Mean",
                                     params.mean1, params.mean_min, params.mean_max);
    RooRealVar* sigmaL1 = new RooRealVar(("sigmaL1_" + name).c_str(), "sigmaL",
                                      params.sigmaL1, params.sigmaL_min, params.sigmaL_max);
    RooRealVar* sigmaR1 = new RooRealVar(("sigmaR1_" + name).c_str(), "sigmaR",
                                      params.sigmaR1, params.sigmaR_min, params.sigmaR_max);
    RooRealVar* alphaL1 = new RooRealVar(("alphaL1_" + name).c_str(), "AlphaL",
                                       params.alphaL1, params.alphaL_min, params.alphaL_max);
    RooRealVar* nL1 = new RooRealVar(("nL1_" + name).c_str(), "NL",
                                   params.nL1, params.nL_min, params.nL_max);
    RooRealVar* alphaR1 = new RooRealVar(("alphaR1_" + name).c_str(), "AlphaR",
                                       params.alphaR1, params.alphaR_min, params.alphaR_max);
    RooRealVar* nR1 = new RooRealVar(("nR1_" + name).c_str(), "NR",
                                   params.nR1, params.nR_min, params.nR_max);
    RooRealVar* mean2 = new RooRealVar(("mean2_" + name).c_str(), "Mean",
                                     params.mean2, params.mean_min, params.mean_max);
    RooRealVar* sigmaL2 = new RooRealVar(("sigmaL2_" + name).c_str(), "sigmaL",
                                      params.sigmaL2, params.sigmaL_min, params.sigmaL_max);
    RooRealVar* sigmaR2 = new RooRealVar(("sigmaR2_" + name).c_str(), "sigmaR",
                                      params.sigmaR2, params.sigmaR_min, params.sigmaR_max);
    RooRealVar* alphaL2 = new RooRealVar(("alphaL2_" + name).c_str(), "AlphaL",
                                       params.alphaL2, params.alphaL_min, params.alphaL_max);
    RooRealVar* nL2 = new RooRealVar(("nL2_" + name).c_str(), "NL",
                                   params.nL2, params.nL_min, params.nL_max);
    RooRealVar* alphaR2 = new RooRealVar(("alphaR2_" + name).c_str(), "AlphaR",
                                       params.alphaR2, params.alphaR_min, params.alphaR_max);
    RooRealVar* nR2 = new RooRealVar(("nR2_" + name).c_str(), "NR",
                                   params.nR2, params.nR_min, params.nR_max);
    RooCrystalBall* CB1 = new RooCrystalBall("CB1", ("DoubleSidedCrystalBall1_" + name).c_str(),
                       *activeMassVar_, *mean1, *sigmaL1,*sigmaR1, *alphaL1, *nL1, *alphaR1, *nR1);
    RooCrystalBall* CB2 = new RooCrystalBall("CB2", ("DoubleSidedCrystalBall2_" + name).c_str(),
                                  *activeMassVar_, *mean2, *sigmaL2,*sigmaR2, *alphaL2, *nL2, *alphaR2, *nR2);
    RooRealVar* frac = new RooRealVar(("frac_" + name).c_str(), "Fraction",
                                  params.fraction, params.fraction_min, params.fraction_max);                            
    return new RooAddPdf(name.c_str(), ("DoubleSidedCrystalBall_" + name).c_str(),
                        RooArgList(*CB1, *CB2), *frac);
}

RooAbsPdf* MassFitter::MakeVoigtian(const VoigtianParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma = new RooRealVar(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    RooRealVar* width = new RooRealVar(("width_" + name).c_str(), "Width",
                                      params.width, params.width_min, params.width_max);
    
    return new RooVoigtian(name.c_str(), ("Voigtian_" + name).c_str(),
                          *activeMassVar_, *mean, *width, *sigma);
}

RooAbsPdf* MassFitter::MakeExponential(const ExponentialBkgParams& params, const std::string& name) {
    // if (!activeMassVar_ || params.coefficients.empty()) return nullptr;
    
    RooRealVar* slope = new RooRealVar(("slope_" + name).c_str(), "Slope",
                                      params.lambda,
                                      params.lambda_min,
                                      params.lambda_max);
    
    return new RooExponential(name.c_str(), ("Exponential_" + name).c_str(),
                             *activeMassVar_, *slope);
}

RooAbsPdf* MassFitter::MakeChebychev(const ChebychevBkgParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooArgList coefList;
    for (size_t i = 0; i < params.coefficients.size(); ++i) {
        RooRealVar* coef = new RooRealVar(Form("cheb_coef%lu_%s", i, name.c_str()),
                                         Form("Coefficient %lu", i),
                                         params.coefficients[i],
                                         params.coef_min[i],
                                         params.coef_max[i]);
        coefList.add(*coef);
    }
    
    return new RooChebychev(name.c_str(), ("Chebyshev_" + name).c_str(),
                           *activeMassVar_, coefList);
}
RooAbsPdf* MassFitter::MakePolynomial(const PolynomialBkgParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooArgList coefList;
    for (size_t i = 0; i < params.coefficients.size(); ++i) {
        RooRealVar* coef = new RooRealVar(Form("poly_coef%lu_%s", i, name.c_str()),
                                         Form("Coefficient %lu", i),
                                         params.coefficients[i],
                                         params.coef_min[i],
                                         params.coef_max[i]);
        coefList.add(*coef);
    }
    
    return new RooChebychev(name.c_str(), ("Polynomial_" + name).c_str(),
                           *activeMassVar_, coefList);
}
// void MassFitter::MakePhenomenological(const PhenomenologicalParams& params, const std::string& name) {
//     // if (!activeMassVar_) return nullptr;
    
//     RooRealVar* m0 = new RooRealVar("m0", "m0",PION_MASS );        // Parameter m0
//     RooRealVar* p0 = new RooRealVar("p0", "p0", params.p0, params.p0_min, params.p0_max);          // Parameter p0
//     RooRealVar* p1 = new RooRealVar("p1", "p1", params.p1, params.p1_min, params.p1_max);          // Parameter p1
//     RooRealVar* p2 = new RooRealVar("p2", "p2", params.p2, params.p2_min, params.p2_max);          // Parameter p2
//     RooFormulaVar* exp_term= new RooFormulaVar("exp_term", "1 - exp(-(@0 - @1)/@2)", RooArgList(*activeMassVar_, *m0, *p0));
//     RooFormulaVar* pow_term= new RooFormulaVar("pow_term", "pow(@0/@1, @2)", RooArgList(*activeMassVar_, *m0, *p1));
//     RooFormulaVar* lin_term= new RooFormulaVar("lin_term", "@0 * (@1/@2 - 1)", RooArgList(*p2, *activeMassVar_, *m0));
    
//     background_pdf_= new RooGenericPdf("pdf", "exp_term * pow_term + lin_term",RooArgList(*exp_term, *pow_term, *lin_term));
// }
RooAbsPdf* MassFitter::MakeRooDstD0Bg(const PhenomenologicalParams& params, const std::string& name = "dstd0") {
    
    RooRealVar* m0 = new RooRealVar("m0", "m0", PION_MASS );        // Parameter m0
    RooRealVar* p0 = new RooRealVar("p0", "p0", params.p0, params.p0_min, params.p0_max);          // Parameter p0
    RooRealVar* p1 = new RooRealVar("p1", "p1", params.p1, params.p1_min, params.p1_max);          // Parameter p1
    RooRealVar* p2 = new RooRealVar("p2", "p2", params.p2, params.p2_min, params.p2_max);          // Parameter p2
    
    return new RooDstD0BG(name.c_str(), ("RooDstD0Bg" + name).c_str(), *activeMassVar_,*m0,*p0,*p1,*p2);
}


void MassFitter::CombinePDFs() {
    if (!signal_pdf_ || !background_pdf_){
        std::cerr << "Signal and background PDFs must be set before combining." << std::endl;
        return;
    } 
    int a =reduced_data_->sumEntries();
    cout << "total Emtries : " << a << endl; 
    
    nsig_ = new RooRealVar("nsig", "Signal Yield", a/100, a/1000, a);
    nbkg_ = new RooRealVar("nbkg", "Background Yield", a/100, a/10000, a);
    
    total_pdf_= new RooAddPdf("total_pdf", "Signal + Background",
                        RooArgList(*signal_pdf_, *background_pdf_),
                        RooArgList(*nsig_, *nbkg_));
}
void MassFitter::CombinePDFs(RooAbsPdf* signal_pdf, RooAbsPdf* background_pdf) {
    if (!signal_pdf_ || !background_pdf_){
        std::cerr << "Signal and background PDFs must be set before combining." << std::endl;
        return;
    } 
    int a =reduced_data_->sumEntries();
    cout << "total Emtries : " << a << endl; 
    
    nsig_ = new RooRealVar("nsig", "Signal Yield", a/100,0, a);
    nbkg_ = new RooRealVar("nbkg", "Background Yield", a/100, 0, a);
    
    total_pdf_= new RooAddPdf("total_pdf", "Signal + Background",
                        RooArgList(*signal_pdf, *background_pdf),
                        RooArgList(*nsig_, *nbkg_));
}
void MassFitter::CombinePDFs(RooAbsPdf* signal_pdf_swap0,RooAbsPdf* signal_pdf_swap1, RooAbsPdf* background_pdf) {
    // if (!signal_pdf_ || !background_pdf_){
    //     std::cerr << "Signal and background PDFs must be set before combining." << std::endl;
    //     return;
    // } 
    int a =reduced_data_->sumEntries();
    cout << "total Emtries : " << a << endl; 
    
    nsig_ = new RooRealVar("nsig", "Signal Yield", a/100,0, a);
    nbkg_ = new RooRealVar("nbkg", "Background Yield", a/100, 0, a);
    frac_ = new RooRealVar("frac_sig_DStar", "Fraction", 0.5, 0., 1.2);
    nSigSwap_ = new RooFormulaVar("nSigSwap", "nsig * frac_sig_DStar", RooArgList(*nsig_, *frac_));
    
   cout << "Signal PDF Swap 0 : " << signal_pdf_swap0->GetName() << endl;
    
    total_pdf_= new RooAddPdf("total_pdf", "Signal + Background",
                        RooArgList(*signal_pdf_swap0,*signal_pdf_swap1, *background_pdf),
                        RooArgList(*nsig_,*nSigSwap_, *nbkg_));
}
void MassFitter::CombineMCPDFs() {
    if (!signal_pdf_ || !reduced_data_) {
        std::cerr << "Error: Signal PDF, background PDF, or dataset not set" << std::endl;
        return;
    }
    int a =reduced_data_->sumEntries();
    // signal_pdf_->SetName("total_pdf");
    nsig_= new RooRealVar("nsig", "Signal Yield", 1000, 0., a);
    // RooRealVar* nbkg = new RooRealVar("nbkg", "Background Yield", a/1000, a/10000, a);

    
    // total_pdf_=  dynamic_cast<RooAddPdf*>(signal_pdf_->Clone());
    // total_pdf_=  dynamic_cast<RooAddPdf*>(new RooExtendPdf("total_pdf", "Extended Signal PDF", *signal_pdf_, *nsig));
    total_pdf_= new RooAddPdf("total_pdf", "Extended Signal PDF",
        RooArgList(*signal_pdf_),
        RooArgList(*nsig_));
    // delete nsig;
}
MassFitter::MassFitter(const std::string& name, std::string& massVar, double massMin, double massMax) 
    : name_(name), 
      massVar_(massVar), 
      useMVA_(false), 
      useDeltaMass_(false), 
      signal_pdf_(nullptr), 
      background_pdf_(nullptr), 
      total_pdf_(nullptr),
      full_data_(nullptr),
      reduced_data_(nullptr),
      fit_result_(nullptr),
      workspace_(nullptr),
      nsig_(nullptr),
      nbkg_(nullptr) {
    
    // 질량 변수 생성
    mass_ = new RooRealVar(massVar_.c_str(), "mass", massMin, massMax, "GeV/c^{2}");
    
    // 활성 질량 변수 초기화
    activeMassVar_ = mass_;
    
    // 워크스페이스 초기화
    workspace_ = new RooWorkspace(("ws_" + name_).c_str());
}

MassFitter::~MassFitter() {
    delete workspace_;
    delete mass_;
}

void MassFitter::SetData(RooDataSet* dataset) {
    if (!dataset) {
        std::cerr << "Error: Null dataset provided" << std::endl;
        return;
    }
    
    full_data_ = dataset;
    reduced_data_ = dataset;  // 초기에는 전체 데이터셋 사용
}

void MassFitter::ApplyCut(const std::string& cutExpr) {
    if (!full_data_) {
        std::cerr << "Error: No dataset available for applying cuts" << std::endl;
        return;
    }
    
    // 조건을 적용하여 데이터셋 생성
    reduced_data_ = (RooDataSet*)full_data_->reduce(cutExpr.c_str());
    
    std::cout << "Applied cut: " << cutExpr << std::endl;
    std::cout << "Entries after cut: " << reduced_data_->numEntries() << std::endl;
    workspace_->import(*mass_);
    workspace_->import(*reduced_data_);
}

void MassFitter::UseDeltaMass(bool use, double daughterMassMin, double daughterMassMax) {
    useDeltaMass_ = use;
    
    if (use) {
        // D* - D0 질량차이 모드
        massDaughter1_ = new RooRealVar("massDaughter1", "Daughter Mass", daughterMassMin, daughterMassMax, "GeV/c^{2}");
        massPion_ = new RooRealVar(("deltaMass_" + name_).c_str(), "Delta Mass", 0.135, 0.16, "GeV/c^{2}");
        activeMassVar_ = massPion_; // Delta mass를 활성 변수로 설정
    } else {
        // 표준 질량 모드
        activeMassVar_ = mass_;
    }
}

// Signal PDF 설정 메서드 - 타입 문자열 제거
void MassFitter::SetSignalPDF(const GaussianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeGaussian(params, pdfName); // 반환된 포인터 할당
}

void MassFitter::SetSignalPDF(const DoubleGaussianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeDoubleGaussian(params, pdfName); // 반환된 포인터 할당
}

void MassFitter::SetSignalPDF(const CrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeCrystalBall(params, pdfName); // 반환된 포인터 할당
}

void MassFitter::SetSignalPDF(const DBCrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeDBCrystalBall(params, pdfName); // 반환된 포인터 할당
}
void MassFitter::SetSignalPDF(const DoubleDBCrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeDoubleDBCrystalBall(params, pdfName); // 반환된 포인터 할당
}

void MassFitter::SetSignalPDF(const VoigtianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeVoigtian(params, pdfName); // 반환된 포인터 할당
}

// Background PDF 설정 메서드 - 반환된 포인터 사용하도록 수정
void MassFitter::SetBackgroundPDF(const ExponentialBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakeExponential(params, pdfName); // 반환된 포인터 할당
}

void MassFitter::SetBackgroundPDF(const ChebychevBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakeChebychev(params, pdfName); // 반환된 포인터 할당
}

void MassFitter::SetBackgroundPDF(const PolynomialBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakePolynomial(params, pdfName); // 반환된 포인터 할당
}
void MassFitter::SetBackgroundPDF(const PhenomenologicalParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakeRooDstD0Bg(params, pdfName); // 반환된 포인터 할당
}



RooFitResult* MassFitter::Fit(bool useMinos, bool useHesse, bool verbose, bool useCUDA, double rangeMin, double rangeMax) {
    if (!total_pdf_) {
        std::cerr << "Error: Signal PDF, background PDF, or dataset not set" << std::endl;
        return nullptr;
    }
    
    // 신호+배경 PDF 생성
    mass_->setRange("analysis", rangeMin, rangeMax);
    
    // 로그 레벨 조정
    // RooMsgService::instance().setGlobalKillBelow(verbose ? RooFit::INFO : RooFit::WARNING);
    
    // 피팅 수행
    fit_result_ = total_pdf_->fitTo(*reduced_data_, 
                                    RooFit::NumCPU(24),
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00) // Check if ROOT version is 6.26.00 or later
                                    RooFit::EvalBackend(useCUDA ? "cuda" : "CPU"),
#endif
                                    // RooFit::EvalBackend(useCUDA ? "cuda" : "CPU"),
                                    RooFit::Extended(true), 
                                    RooFit::Save(true),
                                    RooFit::Minos(useMinos),
                                    RooFit::Optimize(true),
                                    RooFit::Hesse(useHesse),
                                    // RooFit::Strategy(2),
                                    RooFit::PrintLevel(verbose ? 1 : -1),
                                    RooFit::Range("analysis"));
    
    // 결과 출력
    if (verbose) {
        fit_result_->Print("v");
    } else {
        std::cout << "Fit completed. Signal yield: " << GetSignalYield() 
                  << " ± " << GetSignalYieldError() << std::endl;
        std::cout << "Chi2/NDF = " << GetReducedChiSquare() << std::endl;
    }
    
    // 워크스페이스에 결과 저장
    workspace_->import(*total_pdf_);
    
    return fit_result_;
}
RooFitResult* MassFitter::ConstraintFit(std::string fileMCPath,std::string fileMCFile, vector<std::string> constraintParameters, bool useMinos, bool useHesse, bool verbose, bool useCUDA, double rangeMin, double rangeMax) {
    std::string fullPath = fileMCPath;
    
    if (!fullPath.empty() && fullPath.back() != '/') fullPath += '/';
    
    fullPath += fileMCFile;
    
    TFile *file = TFile::Open((fullPath).c_str());
    RooFitResult* fitResult = (RooFitResult*)file->Get("fitResult");
    if (!fitResult) {
        std::cerr << "Error: Fit result is null!" << std::endl;
    }
    if (!total_pdf_ || !reduced_data_) {
        std::cerr << "Error: Total PDF or dataset not set" << std::endl;
        return nullptr;
    }
    
    // 신호+배경 PDF 생성
    // CombinePDFs();
    RooArgList fitParams = fitResult->floatParsFinal();
    RooArgSet massSet(*mass_);
    RooArgSet* pdfParams = total_pdf_->getParameters(&massSet);
    for (int i = 0; i < fitParams.getSize(); ++i) {
        RooRealVar* fitParam = dynamic_cast<RooRealVar*>(fitParams.at(i));
        if (!fitParam) continue;
        
        const std::string paramName = fitParam->GetName();
        RooRealVar* pdfParam = dynamic_cast<RooRealVar*>(pdfParams->find(paramName.c_str()));
        cout << pdfParam << " " <<fitParam << endl;
        
        if (pdfParam) {
            // 모든 파라미터에 대해 값과 오차 설정
            pdfParam->setVal(fitParam->getVal());      
            pdfParam->setError(fitParam->getError());
            
            // constraintParameters 목록에 있는 파라미터만 constant로 설정
            bool shouldConstrain = false;
            for (const auto& constParam : constraintParameters) {
                if (paramName.find(constParam) != std::string::npos) {
                    shouldConstrain = true;
                    break;
                }
            }
            
            if (shouldConstrain) {
                pdfParam->setConstant(true);
                std::cout << "Parameter '" << paramName << "' set to " 
                          << fitParam->getVal() << " ± " << fitParam->getError() 
                          << " and FIXED" << std::endl;
            } else {
                pdfParam->setConstant(false);
                std::cout << "Parameter '" << paramName << "' set to " 
                          << fitParam->getVal() << " ± " << fitParam->getError() 
                          << " (floating)" << std::endl;
            }
        } else {
            std::cerr << "Warning: Parameter '" << paramName 
                      << "' from fit result not found in total_pdf!" << std::endl;
        }
    }
    mass_->setRange("analysis", rangeMin, rangeMax);
    
    
    
    // 로그 레벨 조정
    RooMsgService::instance().setGlobalKillBelow(verbose ? RooFit::INFO : RooFit::WARNING);
    
    // 피팅 수행
    fit_result_ = total_pdf_->fitTo(*reduced_data_,
                                    RooFit::NumCPU(24),
                                    // RooFit::EvalBackend(useCUDA ? "cuda" : "CPU"),
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00) // Check if ROOT version is 6.26.00 or later
                                    RooFit::EvalBackend(useCUDA ? "cuda" : "CPU"),
#endif
                                    RooFit::Extended(true),
                                    RooFit::Save(true),
                                    RooFit::Minos(useMinos),
                                    RooFit::Hesse(useHesse),
                                    RooFit::PrintLevel(verbose ? 1 : -1),
                                    RooFit::Range("analysis"));

    // 결과 출력
    if (verbose) {
        fit_result_->Print("v");
    } else {
        std::cout << "Fit completed. Signal yield: " << GetSignalYield() 
                  << " ± " << GetSignalYieldError() << std::endl;
        std::cout << "Chi2/NDF = " << GetReducedChiSquare() << std::endl;
    }
    
    // 워크스페이스에 결과 저장
    workspace_->import(*total_pdf_);
    delete fitResult;
    file->Close();
    
    return fit_result_;
}

// RooFitResult* MassFitter::ConstraintBinnedFit(std::string fileMCPath, std::string fileMCFile, std::string dataHistoName, std::string dataHistoFilePath, vector<std::string> constraintParameters, bool useMinos, bool useHesse, bool verbose, bool useCUDA, double rangeMin, double rangeMax) {
//     std::string fullMCPath = fileMCPath;
//     if (!fullMCPath.empty() && fullMCPath.back() != '/') fullMCPath += '/';
//     fullMCPath += fileMCFile;

//     TFile *mcFile = TFile::Open((fullMCPath).c_str());
//     if (!mcFile || mcFile->IsZombie()) {
//         std::cerr << "Error: Failed to open MC fit result file: " << fullMCPath << std::endl;
//         if(mcFile) delete mcFile; // Close if opened, then delete
//         return nullptr;
//     }
//     RooFitResult* mcFitResult = (RooFitResult*)mcFile->Get("fitResult");
//     if (!mcFitResult) {
//         std::cerr << "Error: Fit result 'fitResult' not found in MC file: " << fullMCPath << std::endl;
//         mcFile->Close();
//         delete mcFile;
//         return nullptr;
//     }

//     TFile *dataFile = TFile::Open(dataHistoFilePath.c_str());
//     if (!dataFile || dataFile->IsZombie()) {
//         std::cerr << "Error: Failed to open data histogram file: " << dataHistoFilePath << std::endl;
//         // mcFitResult is from mcFile, so don't delete it here if mcFile is closed properly
//         mcFile->Close(); // mcFitResult becomes invalid after this if owned by mcFile
//         delete mcFile;
//         if(dataFile) delete dataFile;
//         return nullptr;
//     }
//     TH1* dataTH1 = (TH1*)dataFile->Get(dataHistoName.c_str());
//     if (!dataTH1) {
//         std::cerr << "Error: Data histogram '" << dataHistoName << "' not found in file: " << dataHistoFilePath << std::endl;
//         dataFile->Close();
//         delete dataFile;
//         mcFile->Close();
//         delete mcFile;
//         return nullptr;
//     }

//     RooDataHist* binnedData = new RooDataHist("binnedData", "Binned Data", *mass_, dataTH1);

//     if (!total_pdf_ || !binnedData) {
//         std::cerr << "Error: Total PDF or binned dataset not set" << std::endl;
//         delete binnedData;
//         dataFile->Close();
//         delete dataFile;
//         mcFile->Close();
//         delete mcFile;
//         return nullptr;
//     }

//     RooArgList fitParams = mcFitResult->floatParsFinal();
//     RooArgSet massSet(*mass_);
//     RooArgSet* pdfParams = total_pdf_->getParameters(&massSet);
//     for (int i = 0; i < fitParams.getSize(); ++i) {
//         RooRealVar* fitParam = dynamic_cast<RooRealVar*>(fitParams.at(i));
//         if (!fitParam) continue;
        
//         const std::string paramName = fitParam->GetName();
//         RooRealVar* pdfParam = dynamic_cast<RooRealVar*>(pdfParams->find(paramName.c_str()));
//         // cout << pdfParam << " " <<fitParam << endl; // User's original debug line
        
//         if (pdfParam) {
//             pdfParam->setVal(fitParam->getVal());      
//             pdfParam->setError(fitParam->getError());
            
//             bool shouldConstrain = false;
//             for (const auto& constParam : constraintParameters) {
//                 if (paramName.find(constParam) != std::string::npos) {
//                     shouldConstrain = true;
//                     break;
//                 }
//             }
            
//             if (shouldConstrain) {
//                 pdfParam->setConstant(true);
//                 std::cout << "Parameter '" << paramName << "' set to " 
//                           << fitParam->getVal() << " ± " << fitParam->getError() 
//                           << " and FIXED" << std::endl;
//             } else {
//                 pdfParam->setConstant(false);
//                 std::cout << "Parameter '" << paramName << "' set to " 
//                           << fitParam->getVal() << " ± " << fitParam->getError() 
//                           << " (floating)" << std::endl;
//             }
//         } else {
//             std::cerr << "Warning: Parameter '" << paramName 
//                       << "' from fit result not found in total_pdf!" << std::endl;
//         }
//     }
//     mass_->setRange("analysis", rangeMin, rangeMax);
    
//     RooMsgService::instance().setGlobalKillBelow(verbose ? RooFit::INFO : RooFit::WARNING);
    
//     fit_result_ = total_pdf_->fitTo(*binnedData,
//                                     RooFit::NumCPU(24),
// #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
//                                     RooFit::EvalBackend(useCUDA ? "cuda" : "CPU"),
// #endif
//                                     RooFit::Extended(true),
//                                     RooFit::Save(true),
//                                     RooFit::Minos(useMinos),
//                                     RooFit::Hesse(useHesse),
//                                     RooFit::PrintLevel(verbose ? 1 : -1),
//                                     RooFit::Range("analysis"));

//     if (verbose) {
//         if (fit_result_) fit_result_->Print("v");
//     } else {
//         std::cout << "Fit completed. Signal yield: " << GetSignalYield() 
//                   << " ± " << GetSignalYieldError() << std::endl;
        
    
//     workspace_->import(*total_pdf_);
//     workspace_->import(*binnedData); // Import the binned data as well

//     // Cleanup
//     // mcFitResult is owned by mcFile, no need to delete explicitly if mcFile is closed.
//     // The original code had 'delete fitResult' (which is mcFitResult here) then 'file->Close()'.
//     // Sticking to that pattern, though it's generally safer to let TFile manage its objects.
//     delete mcFitResult; 
//     mcFile->Close();
//     delete mcFile;

//     // dataTH1 is owned by dataFile.
//     dataFile->Close();
//     delete dataFile;
//     delete binnedData; // Created with new
    
//     return fit_result_;
// }
double MassFitter::GetSignalYield() const {
    return nsig_ ? nsig_->getVal() : 0.0;
}

double MassFitter::GetSignalYieldError() const {
    return nsig_ ? nsig_->getError() : 0.0;
}

double MassFitter::GetBackgroundYield() const {
    return nbkg_ ? nbkg_->getVal() : 0.0;
}

double MassFitter::GetBackgroundYieldError() const {
    return nbkg_ ? nbkg_->getError() : 0.0;
}

double MassFitter::GetChiSquare() const {
    if (!fit_result_ || !activeMassVar_ || !total_pdf_ || !reduced_data_) return -1.0;
    
    // 데이터를 히스토그램으로 변환하여 Chi2 계산
    RooPlot* frame = activeMassVar_->frame();
    reduced_data_->plotOn(frame);
    total_pdf_->plotOn(frame);
    
    double chi2 = frame->chiSquare();
    delete frame;
    
    return chi2;
}
void MassFitter::Clear() {
    // Reset all pointers
    //delete pdfs
    delete signal_pdf_;
    delete background_pdf_;
    delete total_pdf_;
    delete fit_result_;
    reduced_data_->Clear();
    delete workspace_;
    delete nsig_;
    delete nbkg_;
    signal_pdf_ = nullptr;
    background_pdf_ = nullptr;
    total_pdf_ = nullptr;
    full_data_ = nullptr;
    reduced_data_ = nullptr;
    fit_result_ = nullptr;
    workspace_ = nullptr;
    swap0_pdf_= nullptr;
    swap1_pdf_= nullptr;
    nsig_ = nullptr;
    nbkg_ = nullptr;
}

double MassFitter::GetNDF() const {
    if (!fit_result_) return 0.0;
    return fit_result_->floatParsFinal().getSize();
}

double MassFitter::GetReducedChiSquare() const {
    double ndf = GetNDF();
    return (ndf > 0) ? GetChiSquare() / ndf : -1.0;
}
template <typename SigPar, typename BkgPar>
void MassFitter::PerformFit(FitOpt opt,RooDataSet* dataset, bool inclusive, const std::string pTbin, const std::string etabin, SigPar sigParams, BkgPar bkgParams) {
    // Open ROOT file
    // TFile* file = TFile::Open(filepath_.c_str());
    // if (!file || file->IsZombie()) {
    //     std::cerr << "Error: Failed to open file " << filepath_ << std::endl;
    //     return;
    // }
    
    // // Get the dataset from the file
    // RooDataSet* dataset = dynamic_cast<RooDataSet*>(file->Get(opt.datasetName.c_str()));
    // if (!dataset) {
    //     std::cerr << "Error: Failed to load dataset '" << opt.datasetName << "' from file" << std::endl;
    //     file->Close();
    //     return;
    // }
    // // Set the dataset
    SetData(dataset);
    
    // // Apply cuts based on inclusive flag
    // if (inclusive) {
    //     ApplyCut(opt.cutExpr);
    // } else {
    //     ApplyCut(opt.cutExpr + "&&" + pTbin + "&&" + etabin);
    // }

    // Create and set PDFs
    SetSignalPDF(sigParams, "");
    SetBackgroundPDF(bkgParams, "");
    CombinePDFs();

    // Perform fit and save results
    if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    SaveResults(opt.outputDir,opt.outputFile, opt.saveWorkspace);
    
    // Close the file (dataset is already copied to MassFitter)
    // file->Close();
}
template <typename SigMCPar>
void MassFitter::PerformMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams) {
    full_data_ = MCdataset;

    if (inclusive) {
        ApplyCut(opt.cutMCExpr);
    } else {
        ApplyCut(opt.cutMCExpr + "&&" + pTbin + "&&" + etabin);
    }
    // Create and set PDFs
    SetSignalPDF(sigMCParams,"");
    // SetBackgroundPDF(bkgParams);
    CombineMCPDFs();
    // Perform fit and save results
    if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    SaveMCResults(opt.outputMCDir,opt.outputMCFile, opt.saveWorkspace);
    Clear();
    workspace_ = new RooWorkspace(("ws_" + name_).c_str());
    // full_data_ = MCdataset;

    // if (inclusive) {
    //     ApplyCut(opt.cutMCExpr +"&& isSwap==0" );
    // } else {
    //     ApplyCut(opt.cutMCExpr +"&& isSwap==0" + "&&" + pTbin + "&&" + etabin);
    // }
    // // Create and set PDFs
    // SetSignalPDF(sigParams1,"Swap0");
    // // SetBackgroundPDF(bkgParams);
    // CombineMCPDFs();
    // // Perform fit and save results
    // if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    // SaveMCResults(opt.outputMCDir,opt.outputMCSwap0File, opt.saveWorkspace);
    // Clear();
    // workspace_ = new RooWorkspace(("ws_" + name_).c_str());

    // full_data_ = MCdataset;
    // if (inclusive) {
    //     ApplyCut(opt.cutMCExpr +"&& isSwap==1" );
    // } else {
    //     ApplyCut(opt.cutMCExpr +"&& isSwap==1" + "&&" + pTbin + "&&" + etabin);
    // }
    // // Create and set PDFs
    // SetSignalPDF(sigParams2,"Swap1");
    // // SetBackgroundPDF(bkgParams);
    // CombineMCPDFs();
    // // Perform fit and save results
    // if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    // SaveMCResults(opt.outputMCDir,opt.outputMCSwap1File, opt.saveWorkspace);
    // Clear();

    // workspace_ = new RooWorkspace(("ws_" + name_).c_str());

}
template <typename SigMCSwap1Par, typename SigMCSwap2Par>
void MassFitter::PerformSwapMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCSwap1Par sigParams1, SigMCSwap2Par sigParams2) {
    full_data_ = MCdataset;

    if (inclusive) {
        ApplyCut(opt.cutMCExpr +"&& isSwap==0" );
    } else {
        ApplyCut(opt.cutMCExpr +"&& isSwap==0" + "&&" + pTbin + "&&" + etabin);
    }
    // Create and set PDFs
    SetSignalPDF(sigParams1,"Swap0");
    // SetBackgroundPDF(bkgParams);
    CombineMCPDFs();
    // Perform fit and save results
    if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    SaveMCResults(opt.outputMCDir,opt.outputMCSwap0File, opt.saveWorkspace);
    Clear();
    workspace_ = new RooWorkspace(("ws_" + name_).c_str());

    full_data_ = MCdataset;
    if (inclusive) {
        ApplyCut(opt.cutMCExpr +"&& isSwap==1" );
    } else {
        ApplyCut(opt.cutMCExpr +"&& isSwap==1" + "&&" + pTbin + "&&" + etabin);
    }
    // Create and set PDFs
    SetSignalPDF(sigParams2,"Swap1");
    // SetBackgroundPDF(bkgParams);
    CombineMCPDFs();
    // Perform fit and save results
    if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    SaveMCResults(opt.outputMCDir,opt.outputMCSwap1File, opt.saveWorkspace);
    Clear();

    workspace_ = new RooWorkspace(("ws_" + name_).c_str());

}
bool MassFitter::MapAndFixParametersFromMCSwap(
    const std::string& mcDirPath,
    const std::string& mcSwap0File,
    const std::string& mcSwap1File,
    const std::vector<std::string>& parameterNames
) {
    // 경로 구성
    std::string fullPathSwap0 = mcDirPath;
    std::string fullPathSwap1 = mcDirPath;
    
    if (!fullPathSwap0.empty() && fullPathSwap0.back() != '/') fullPathSwap0 += '/';
    if (!fullPathSwap1.empty() && fullPathSwap1.back() != '/') fullPathSwap1 += '/';
    
    fullPathSwap0 += mcSwap0File;
    fullPathSwap1 += mcSwap1File;
    
    // 파일 열기
    std::cout << "Opening MC Swap0 file: " << fullPathSwap0 << std::endl;
    TFile* fileSwap0 = TFile::Open(fullPathSwap0.c_str(), "READ");
    if (!fileSwap0 || fileSwap0->IsZombie()) {
        std::cerr << "Error: Could not open Swap0 file " << fullPathSwap0 << std::endl;
        if (fileSwap0) delete fileSwap0;
        return false;
    }
    
    std::cout << "Opening MC Swap1 file: " << fullPathSwap1 << std::endl;
    TFile* fileSwap1 = TFile::Open(fullPathSwap1.c_str(), "READ");
    if (!fileSwap1 || fileSwap1->IsZombie()) {
        std::cerr << "Error: Could not open Swap1 file " << fullPathSwap1 << std::endl;
        if (fileSwap0) delete fileSwap0;
        if (fileSwap1) delete fileSwap1;
        return false;
    }
    
    // 피팅 결과 로드
    RooFitResult* resultSwap0 = (RooFitResult*)fileSwap0->Get("fitResult");
    if (!resultSwap0) {
        std::cerr << "Error: Could not find fitResult in " << fullPathSwap0 << std::endl;
        fileSwap0->Close();
        fileSwap1->Close();
        delete fileSwap0;
        delete fileSwap1;
        return false;
    }
    
    RooFitResult* resultSwap1 = (RooFitResult*)fileSwap1->Get("fitResult");
    if (!resultSwap1) {
        std::cerr << "Error: Could not find fitResult in " << fullPathSwap1 << std::endl;
        fileSwap0->Close();
        fileSwap1->Close();
        delete fileSwap0;
        delete fileSwap1;
        return false;
    }
    
    // 기본 파라미터 이름 정의
    std::vector<std::string> baseParamNames = {"mean_Swap1", "sigma_Swap1","mean1_Swap0","mean2_Swap0","sigma1_Swap0","sigma2_Swap0"};
    // std::vector<std::string> baseParamNames = {"mean", "sigmaL", "sigmaR", "alphaL", "alphaR", "nL", "nR"};
    
    // DBCB 파라미터 맵 구조: <baseParamName, <value, error>>
    std::map<std::string, std::pair<double, double>> swap0ParamsMap;
    std::map<std::string, std::pair<double, double>> swap1ParamsMap;
    std::map<std::string, std::pair<double, double>> swapParamsMap;
    
    // Swap0 파라미터 추출
    RooArgList finalParamsSwap0 = resultSwap0->floatParsFinal();
    std::cout << "Parameters from Swap0:" << std::endl;
    
    for (int i = 0; i < finalParamsSwap0.getSize(); ++i) {
        RooRealVar* param = dynamic_cast<RooRealVar*>(finalParamsSwap0.at(i));
        if (!param) continue;
        
        std::string fullName = param->GetName();
        // cout << fullName << endl;
        double value = param->getVal();
        double error = param->getError();
        
        // 파라미터 이름에서 "_Swap0" 제거하여 베이스 이름 추출
        for (const auto baseName : baseParamNames) {
            // cout << baseName << endl;
            // cout << fullName.find(baseName) != std::string::npos << endl;
            if (fullName.find(baseName) != std::string::npos) {
                swapParamsMap[baseName] = std::make_pair(value, error);
                std::cout << " - " << baseName << " (from " << fullName << "): " 
                          << value << " ± " << error << std::endl;
                break;
            }
        }
        
        // nsig 파라미터 처리
        if (fullName == "nsig") {
            swap0ParamsMap["nsig"] = std::make_pair(value, error);
            std::cout << " - nsig: " << value << " ± " << error << std::endl;
        }
    }
    
    // Swap1 파라미터 추출
    RooArgList finalParamsSwap1 = resultSwap1->floatParsFinal();
    std::cout << "Parameters from Swap1:" << std::endl;
    
    for (int i = 0; i < finalParamsSwap1.getSize(); ++i) {
        RooRealVar* param = dynamic_cast<RooRealVar*>(finalParamsSwap1.at(i));
        if (!param) continue;
        
        std::string fullName = param->GetName();
        double value = param->getVal();
        double error = param->getError();
        
        
        // 파라미터 이름에서 "_Swap1" 제거하여 베이스 이름 추출
        for (const auto& baseName : baseParamNames) {
            if (fullName.find(baseName)!=std::string::npos) {
                swapParamsMap[baseName] = std::make_pair(value, error);
                std::cout << " - " << baseName << " (from " << fullName << "): " 
                          << value << " ± " << error << std::endl;
                break;
            }
        } 
        
        // nsig 파라미터 처리
        if (fullName == "nsig") {
            swap1ParamsMap["nsig"] = std::make_pair(value, error);
            std::cout << " - nsig: " << value << " ± " << error << std::endl;
        }
    }
    
    // nsig 확인 및 fraction 계산
    double nsig0 = 0, nsig0Err = 0;
    double nsig1 = 0, nsig1Err = 0;
    
    auto nsig0Iter = swap0ParamsMap.find("nsig");
    if (nsig0Iter != swap0ParamsMap.end()) {
        nsig0 = nsig0Iter->second.first;
        nsig0Err = nsig0Iter->second.second;
    }
    
    auto nsig1Iter = swap1ParamsMap.find("nsig");
    if (nsig1Iter != swap1ParamsMap.end()) {
        nsig1 = nsig1Iter->second.first;
        nsig1Err = nsig1Iter->second.second;
    }
    
    // double totalYield = nsig0 + nsig1;
    // double fraction = (totalYield > 0) ? nsig1 / totalYield : 0.5;
    // double fractionErr = 0.0;
    // if (totalYield > 0) {
    //     double term1 = (nsig1 / (totalYield * totalYield)) * nsig0Err;
    //     double term2 = (nsig0 / (totalYield * totalYield)) * nsig1Err;
    //     fractionErr = sqrt(term1*term1 + term2*term2);
    // }

double fraction = (nsig0 > 0) ? nsig1 / nsig0 : 0.5; // Redefine fraction as nsig1 / nsig0
double fractionErr = 0.0;
if (nsig0 > 0) {
    // Error propagation for fraction = nsig1 / nsig0
    double term1 = nsig1Err / nsig0; // Contribution from nsig1 uncertainty
    double term2 = (nsig1 * nsig0Err) / (nsig0 * nsig0); // Contribution from nsig0 uncertainty
    fractionErr = sqrt(term1 * term1 + term2 * term2);
}
    
    std::cout << "Calculated fraction (Swap0/(Swap0+Swap1)): " << fraction << " ± " << fractionErr << std::endl;
    
    // 현재 total_pdf의 파라미터 세트 가져오기
    if (!total_pdf_) {
        std::cerr << "Error: Total PDF is not initialized" << std::endl;
        fileSwap0->Close();
        fileSwap1->Close();
        delete fileSwap0;
        delete fileSwap1;
        return false;
    }
    
    RooArgSet* pdfParams = total_pdf_->getParameters(RooArgSet(*activeMassVar_));
    if (!pdfParams) {
        std::cerr << "Error: Could not get parameters from total PDF" << std::endl;
        fileSwap0->Close();
        fileSwap1->Close();
        delete fileSwap0;
        delete fileSwap1;
        return false;
    }
    
    // 고정할 파라미터 확인 및 설정
    std::cout << "\nSetting parameters in total PDF:" << std::endl;
    std::vector<std::string> fixedParamsList; // 실제 고정된 파라미터 목록

    auto findParamContaining = [](RooArgSet* params, const std::string& nameSubstr) -> RooRealVar* {
        for (auto& param : *params) {
            RooAbsArg* arg = dynamic_cast<RooAbsArg*>(param);
            if (!arg) continue;
            
            std::string name = arg->GetName();
            if (name.find(nameSubstr) != std::string::npos) {
                return dynamic_cast<RooRealVar*>(arg);
            }
        }
        return nullptr;
    };
    
    // fraction 파라미터 찾기 및 고정
    // RooRealVar* paramMean1 = findParamContaining(pdfParams, "mean1");
    
    // if (paramMean1) {
    //     paramMean1->setVal(DELTAMASS);
    //     paramMean1->setConstant(true);
    //     std::cout << " - Fixed " << paramMean1->GetName() << " = " << DELTAMASS << endl;
    // }
    // RooRealVar* paramMean2 = findParamContaining(pdfParams, "mean2");
    // // cout << swap0ParamsMap.find(baseName) != swap0ParamsMap.end() << endl;
    
    // if (paramMean2) {
    //     paramMean2->setVal(DELTAMASS);
    //     paramMean2->setConstant(true);
    //     std::cout << " - Fixed " << paramMean2->GetName() << " = " << DELTAMASS << endl;
    // }
    
    RooRealVar* fracParam = dynamic_cast<RooRealVar*>(pdfParams->find(("frac_sig_DStar")));
    if (fracParam) {
        fracParam->setVal(fraction);
        // fracParam->setError(fractionErr);
        fracParam->setConstant(true);
        fixedParamsList.push_back(fracParam->GetName());
        std::cout << " - Fixed " << fracParam->GetName() << " = " << fraction 
                 << " ± " << fractionErr << std::endl;
    } else {
        std::cout << " - Warning: fraction parameter not found in the PDF" << std::endl;
    }
    // // Create a RooFormulaVar for calculating pure signal yield (non-swap component)
    // RooRealVar* nsigVar = dynamic_cast<RooRealVar*>(pdfParams->find("nsig"));
    
    // if (nsigVar && fracParam) {
    //     // Create a formula to calculate the pure signal yield: nsig * fraction
    //     nSigSwap_ = new RooFormulaVar("nSigPure", "Pure signal yield (non-swap)", 
    //                                  "@0*@1", RooArgList(*nsigVar, *fracParam));
        
    //     // Import into workspace for later retrieval
    //     if (workspace_) {
    //         workspace_->import(*nSigSwap_);
    //     }
        
    //     std::cout << " - Created formula for pure signal yield: nSigPure = nsig * " 
    //               << fraction << std::endl;
    //     std::cout << " - Estimated initial pure signal yield: " << nsigVar->getVal() * fraction 
    //               << " events" << std::endl;
    // } else {
    //     std::cout << " - Warning: Could not create pure signal formula (nsig or frac not found)" << std::endl;
    // }
    
    // // 기본 파라미터 고정
    for (const auto& baseName : parameterNames) {
        // baseName+"1" 파라미터 (Swap0)
        std::string param1Name = baseName;
        // cout << param1->GetName() << endl;
        cout << param1Name << endl;
        RooRealVar* param1 = findParamContaining(pdfParams, param1Name);
        // cout << swap0ParamsMap.find(baseName) != swap0ParamsMap.end() << endl;
        
        if (param1) {
            double value = swapParamsMap[baseName].first;
            double error = swapParamsMap[baseName].second;
            
            param1->setVal(value);
            param1->setError(error);
            param1->setConstant(true);
            fixedParamsList.push_back(param1->GetName());
            
            std::cout << " - Fixed " << param1->GetName() << " = " << value << " ± " << error
                     << " (from " << baseName << "_Swap0)" << std::endl;
        }
        
        // baseName+"2" 파라미터 (Swap1)
        // std::string param2Name = baseName;
        // RooRealVar* param2 = findParamContaining(pdfParams, param2Name);
        
        // if (param2) {
        //     double value = swap1ParamsMap[baseName].first;
        //     double error = swap1ParamsMap[baseName].second;
            
        //     param2->setVal(value);
        //     param2->setError(error);
        //     param2->setConstant(true);
        //     fixedParamsList.push_back(param2->GetName());
            
        //     std::cout << " - Fixed " << param2->GetName() << " = " << value << " ± " << error
        //              << " (from " << baseName << "_Swap1)" << std::endl;
        // }
    }
    
    // // 범위 설정
    // for (const auto& baseName : baseParamNames) {
    //     bool isBaseFixed = std::find(parameterNames.begin(), parameterNames.end(), baseName) != parameterNames.end();
        
    //     // 고정하지 않는 파라미터는 범위 설정
    //     if (!isBaseFixed) {
    //         double value1 = 0, error1 = 0;
    //         double value2 = 0, error2 = 0;
    //         bool hasValue1 = false, hasValue2 = false;
            
    //         // Swap0 파라미터 값 가져오기
    //         if (swap0ParamsMap.find(baseName) != swap0ParamsMap.end()) {
    //             value1 = swap0ParamsMap[baseName].first;
    //             error1 = swap0ParamsMap[baseName].second;
    //             hasValue1 = true;
    //         }
            
    //         // Swap1 파라미터 값 가져오기
    //         if (swap1ParamsMap.find(baseName) != swap1ParamsMap.end()) {
    //             value2 = swap1ParamsMap[baseName].first;
    //             error2 = swap1ParamsMap[baseName].second;
    //             hasValue2 = true;
    //         }
            
    //         // 값이 있다면 범위 설정
    //         if (hasValue1 || hasValue2) {
    //             double maxError = std::max(error1, error2);
                
    //             // baseName+"1" 파라미터
    //             std::string param1Name = baseName + "1";
    //             RooRealVar* param1 = findParamContaining(pdfParams, param1Name);
                
    //             if (param1 && hasValue1) {
    //                 // 값은 설정하되 고정하지는 않음
    //                 param1->setVal(value1);
    //                 // param1->setError(error1);
                    
    //                 // // 범위 설정 (오차의 5배)
    //                 // double minVal = value1 - 5 * maxError;
    //                 // double maxVal = value1 + 5 * maxError;
                    
    //                 // // 특정 파라미터는 음수가 되지 않도록 설정
    //                 // if ((baseName == "sigmaL" || baseName == "sigmaR" || 
    //                 //      baseName == "alphaL" || baseName == "alphaR" || 
    //                 //      baseName == "nL" || baseName == "nR") && minVal <= 0) {
    //                 //     minVal = 0.00001;
    //                 // }
                    
    //                 // param1->setRange(minVal, maxVal);
    //                 // std::cout << " - Set " << param1Name << " = " << value1 << " ± " << error1
    //                 //          << " (range: " << minVal << " - " << maxVal << ", not fixed)" << std::endl;
    //             }
                
    //             // baseName+"2" 파라미터
    //             std::string param2Name = baseName + "2";
    //             RooRealVar* param2 = findParamContaining(pdfParams, param2Name);
                
    //             if (param2 && hasValue2) {
    //                 // 값은 설정하되 고정하지는 않음
    //                 param2->setVal(value2);
    //                 // param2->setError(error2);
                    
    //                 // // 범위 설정 (오차의 5배)
    //                 // double minVal = value2 - 5 * maxError;
    //                 // double maxVal = value2 + 5 * maxError;
                    
    //                 // // 특정 파라미터는 음수가 되지 않도록 설정
    //                 // if ((baseName == "sigmaL" || baseName == "sigmaR" || 
    //                 //      baseName == "alphaL" || baseName == "alphaR" || 
    //                 //      baseName == "nL" || baseName == "nR") && minVal <= 0) {
    //                 //     minVal = 0.00001;
    //                 // }
                    
    //                 // param2->setRange(minVal, maxVal);
    //                 // std::cout << " - Set " << param2Name << " = " << value2 << " ± " << error2
    //                 //          << " (range: " << minVal << " - " << maxVal << ", not fixed)" << std::endl;
    //             }
    //         }
    //     }
    // }
    
    // // 고정된 파라미터 목록 요약
    // std::cout << "\nFixed Parameters Summary:" << std::endl;
    // for (const auto& paramName : fixedParamsList) {
    //     RooRealVar* param = dynamic_cast<RooRealVar*>(pdfParams->find(paramName.c_str()));
    //     if (param) {
    //         std::cout << " - " << paramName << " = " << param->getVal() 
    //                  << " ± " << param->getError() << std::endl;
    //     }
    // }
    
    // 자원 정리
    fileSwap0->Close();
    fileSwap1->Close();
    delete fileSwap0;
    delete fileSwap1;
    
    std::cout << "Parameter mapping and fixing completed successfully." << std::endl;
    return true;
}




template <typename SigSwap0Par, typename SigSwap1Par, typename BkgPar>
void MassFitter::PerformConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigSwap0Par sigSwap0Params, SigSwap1Par sigSwap1Params, BkgPar bkgParams) {
    PerformSwapMCFit(opt,MCdata,inclusive,pTbin,etabin,sigSwap0Params,sigSwap1Params);
    full_data_ = data;
    if (inclusive) {
        ApplyCut(opt.cutExpr);
    } else {
        ApplyCut(opt.cutExpr + "&&" + pTbin + "&&" + etabin);
    }
    swap0_pdf_ = MakeDoubleGaussian(sigSwap0Params, "Swap0");
    swap1_pdf_ = MakeGaussian(sigSwap1Params, "Swap1");
    // frac_= new RooRealVar("frac_sig_DStar", "Fraction of Swap0", 0.5, 0.0, 1.0);
    // signal_pdf_ = new RooAddPdf("signal_pdf", "Signal PDF", RooArgList(*swap0_pdf_, *swap1_pdf_), RooArgList(*frac_));
    
    SetBackgroundPDF(bkgParams,"");
    // CombinePDFs(signal_pdf_, background_pdf_);
    CombinePDFs(swap0_pdf_,swap1_pdf_, background_pdf_);
    
    MapAndFixParametersFromMCSwap(opt.outputMCDir,opt.outputMCSwap0File,opt.outputMCSwap1File,opt.constraintParameters);

    // Define the pure signal yield formula AFTER nsig_ and frac_ are created


    if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    if (nsig_ && frac_) {
        //  nSigSwap_ = new RooFormulaVar("nSigPure", "Pure signal yield", "@0 * @1", RooArgList(*nsig_, *frac_));
        //  nSigSwap_ = nsig_;
         workspace_->import(*nSigSwap_); // Import into workspace
         std::cout << "INFO: Defined RooFormulaVar nSigPure_" << name_ << " = nsig * frac_sig_" << name_ << std::endl;
    } else {
         std::cerr << "ERROR: Cannot define nSigPure_ because nsig_ or frac_ is null." << std::endl;
         nSigSwap_ = nullptr;
    }

    // if(opt.doFit)ConstraintFit(opt.outputMCDir,opt.outputMCFile,opt.constraintParameters, opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    SaveResults(opt.outputDir,opt.outputFile, opt.saveWorkspace);
    Clear();
    delete swap0_pdf_;
    delete swap1_pdf_;

    
    // Close the file (dataset is already copied to MassFitter)
    // file->Close();
}
template <typename SigPar, typename BkgPar>
void MassFitter::PerformConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigPar sigParams, BkgPar bkgParams) {
    PerformMCFit(opt,MCdata,inclusive,pTbin,etabin,sigParams);
    full_data_ = data;
    if (inclusive) {
        ApplyCut(opt.cutExpr);
    } else {
        ApplyCut(opt.cutExpr + "&&" + pTbin + "&&" + etabin);
    }
    // frac_= new RooRealVar("frac_sig_DStar", "Fraction of Swap0", 0.5, 0.0, 1.0);
    // signal_pdf_ = new RooAddPdf("signal_pdf", "Signal PDF", RooArgList(*swap0_pdf_, *swap1_pdf_), RooArgList(*frac_));

    SetSignalPDF(sigParams,"");
    SetBackgroundPDF(bkgParams,"");
    CombinePDFs(signal_pdf_, background_pdf_);
    // CombinePDFs(swap0_pdf_,swap1_pdf_, background_pdf_);

    
    // MapAndFixParametersFromMCSwap(opt.outputMCDir,opt.outputMCSwap0File,opt.outputMCSwap1File,opt.constraintParameters);

    // Define the pure signal yield formula AFTER nsig_ and frac_ are created


    // if(opt.doFit)Fit(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    // if (nsig_ && frac_) {
    //     //  nSigSwap_ = new RooFormulaVar("nSigPure", "Pure signal yield", "@0 * @1", RooArgList(*nsig_, *frac_));
    //     //  nSigSwap_ = nsig_;
    //      workspace_->import(*nSigSwap_); // Import into workspace
    //      std::cout << "INFO: Defined RooFormulaVar nSigPure_" << name_ << " = nsig * frac_sig_" << name_ << std::endl;
    // } else {
    //      std::cerr << "ERROR: Cannot define nSigPure_ because nsig_ or frac_ is null." << std::endl;
    //      nSigSwap_ = nullptr;
    // }

    if(opt.doFit)ConstraintFit(opt.outputMCDir,opt.outputMCFile,opt.constraintParameters, opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    SaveResults(opt.outputDir,opt.outputFile, opt.saveWorkspace);
    Clear();

    
    // Close the file (dataset is already copied to MassFitter)
    // file->Close();
}

void MassFitter::SaveResults(const std::string& filePath,const std::string& fileName, bool saveWorkspace) {
    TFile* outFile = createFileInDir(filePath,fileName);
    if (!outFile || outFile->IsZombie()) {
         std::cerr << "ERROR: Could not create output file: " << filePath << "/" << fileName << std::endl;
         return;
    }
    outFile->cd(); // Ensure we are writing to the correct file

    if (fit_result_) {
        fit_result_->Write("fitResult");

        // Calculate and save pure signal yield if available
        if (nSigSwap_) {
            double pureYieldVal = nsig_->getVal();
            // Propagate error from the fit result
            double pureYieldErr = nsig_->getPropagatedError(*fit_result_);

            std::cout << "INFO: Calculated Pure Signal Yield = " << pureYieldVal << " +/- " << pureYieldErr << std::endl;

            // Save as TParameter<double> for easy retrieval
            TParameter<double>* pureYieldValParam = new TParameter<double>("nSigPure_Value", pureYieldVal);
            TParameter<double>* pureYieldErrParam = new TParameter<double>("nSigPure_Error", pureYieldErr);
            pureYieldValParam->Write();
            pureYieldErrParam->Write();
            delete pureYieldValParam;
            delete pureYieldErrParam;
        } else {
             std::cout << "INFO: nSigSwap_ not available, skipping pure yield saving." << std::endl;
        }
    } else {
         std::cout << "INFO: No fit result available to save." << std::endl;
    }

    if (reduced_data_) {
        cout << "writing entry : " << reduced_data_->sumEntries() << endl;
        reduced_data_->Write("reducedData");
    }
    if (total_pdf_) {
            total_pdf_->Write("total_pdf");
    }
    
    // 워크스페이스 저장 (모든 객체 포함)
    if (saveWorkspace && workspace_) {
        workspace_->Write();
    }
    
    outFile->Close();
    delete outFile;
    
    std::cout << "Results saved to " << fileName << std::endl;
}
void MassFitter::SaveMCResults(const std::string& filePath,const std::string& fileName, bool saveWorkspace) {
    TFile* outFile = createFileInDir(filePath,fileName);
    // 피팅 결과 저장
    if (fit_result_) {
        fit_result_->Write("fitResult");
    }
    else{
        cout << "fit result is null" << endl;
    }
    if (reduced_data_) {
        cout << "writing entry : " << reduced_data_->sumEntries() << endl;
        reduced_data_->Write("reducedData");
    }
    if (total_pdf_) {
            total_pdf_->Write("total_pdf");
    }
    
    if (saveWorkspace && workspace_) {
        workspace_->Write();
    }
    
    
    outFile->Close();
    delete outFile;
    
    std::cout << "MC Results saved to " << filePath+fileName << std::endl;
}

// void MassFitter::PlotResult(bool drawFitModel, std::string filename_, double pTMin=0, double pTMax=0, double etaMin=0, double etaMax=0) {
//     TFile file(filename_.c_str(), "READ");
//     if(drawFitModel){
//     // fit_result_ = dynamic_cast<RooFitResult*>(file.Get("fit_result"));
//     // total_pdf_ = dynamic_cast<RooAddPdf*>(file.Get("total_pdf"));
//     }
//     // reduced_data_ = dynamic_cast<RooDataSet*>(file.Get("dataset"));
//     // RooRealVar* mass = dynamic_cast<RooRealVar*>(total_pdf_->getVariables()->find("mass"));
//     // activeMassVar_ = mass;
//     if (!total_pdf_ || !fit_result_) cout << "no total pdf" << endl;
//         RooDataSet* dataToPlot = reduced_data_;
//         if (!dataToPlot){
// 	cout << "no data to Plot" << endl;
// 	 return;
// 	}

//     // Create canvas with two pads: main plot and pull distribution
//     TCanvas* canvas = new TCanvas("canvas", "Fit Result", 1000, 800);
//     canvas->Divide(1, 2);
    
//     // Upper pad for main plot (takes 70% of canvas)
//     TPad* padUp = new TPad("padUp", "padUp", 0, 0.2, 1, 1);
//     padUp->SetBottomMargin(0.0);
//     padUp->SetTopMargin(0.12);
//     padUp->SetLeftMargin(0.1);
//     padUp->Draw();
    
//     // Lower pad for pull distribution (takes 30% of canvas)
//     TPad* padDown = new TPad("padDown", "padDown", 0, 0, 1, 0.2);
//     padDown->SetTopMargin(0.01);
//     padDown->SetLeftMargin(0.1);
//     padDown->SetBottomMargin(0.3);
//     padDown->Draw();
    
//     // Draw main plot
//     padUp->cd();
//     RooPlot* frame = activeMassVar_->frame(RooFit::Bins(120),RooFit::Title(" "));
    
//     // Data points
//     cout << "adsasd" << endl;
//     reduced_data_->plotOn(frame, RooFit::Name("data"));
//     cout << "asd" << endl;
    
//     // Full model
// if(total_pdf_ != nullptr) {
//     total_pdf_->plotOn(frame, RooFit::Name("fit"));
    
//     // Signal component
//     if (signal_pdf_) {
//         // Check if signal_pdf_ is a composite PDF
//         const RooArgSet* components = signal_pdf_->getComponents();
//         if (components->getSize() > 1) {
//             // Iterate over the components and plot each one
//             TIterator* iter = components->createIterator();
//             RooAbsArg* component;
//             int colorIndex = 5; // Start with a different color for each component
//             while ((component = (RooAbsArg*)iter->Next())) {
//                 RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(component);
//                 if (pdfComponent) {
//                     total_pdf_->plotOn(frame, 
//                                        RooFit::Components(*pdfComponent),
//                                        RooFit::LineStyle(kDashed),
//                                        RooFit::LineColor(colorIndex),
//                                        RooFit::Name(pdfComponent->GetName()));
//                     colorIndex++;
//                 }
//             }
//             delete iter;
//         } else {
//             // Plot the single signal component
//             total_pdf_->plotOn(frame, 
//                                RooFit::Components(*signal_pdf_),
//                                RooFit::LineStyle(kDashed),
//                                RooFit::LineColor(kRed),
//                                RooFit::Name("signal"));
//         }
//     }
    
//     // Background component
//     if (background_pdf_) {
//         total_pdf_->plotOn(frame, 
//                            RooFit::Components(*background_pdf_),
//                            RooFit::LineStyle(kDashed),
//                            RooFit::LineColor(kGreen),
//                            RooFit::Name("background"));
//     }

//     // Add chi2 value
//     double chi2 = frame->chiSquare("fit", "data", fit_result_->floatParsFinal().getSize());
    
//     // Create legend
//     TLegend* legend = new TLegend(0.70, 0.2, 0.85, 0.35);
//     legend->AddEntry(frame->findObject("data"), "Data", "p");
//     legend->AddEntry(frame->findObject("fit"), "Total Fit", "l");
//     if (signal_pdf_) {
//     const RooArgSet* components = signal_pdf_->getComponents();
//     if (components->getSize() > 1) {
//         TIterator* iter = components->createIterator();
//         RooAbsArg* component;
//         while ((component = (RooAbsArg*)iter->Next())) {
//             RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(component);
//             if (pdfComponent) {
//                 legend->AddEntry(frame->findObject(pdfComponent->GetName()), pdfComponent->GetName(), "l");
//             }
//         }
//         delete iter;
//     } else {
//         legend->AddEntry(frame->findObject("signal"), "Signal", "l");
//     }
//     }
//     if (background_pdf_) legend->AddEntry(frame->findObject("background"), "Background", "l");
//     legend->SetBorderSize(0);
//     legend->SetFillStyle(0);
    
//     // Draw frame and legend
//     frame->Draw();
//     legend->Draw("same");
    
//     // Add fit parameters text box
//     TPaveText* paveText = new TPaveText(0.65, 0.10, 0.90, 0.45, "NDC");
//     paveText->SetBorderSize(0);
//     paveText->SetFillStyle(0);
//     paveText->AddText(Form("#chi^{2}/ndf = %.2f", chi2));
    
//     // Add fit parameters with errors
//     RooArgList params = fit_result_->floatParsFinal();
//     for (int i = 0; i < params.getSize(); ++i) {
//         RooRealVar* param = (RooRealVar*)params.at(i);
//         paveText->AddText(Form("%s = %.4f #pm %.4f", 
//                               param->GetName(),
//                               param->getVal(),
//                               param->getError()));
//     }
//     paveText->Draw("same");
//     TPaveText* rangeText = new TPaveText(0.15, 0.65, 0.40, 0.8, "NDC");
//     rangeText->SetBorderSize(0);
//     rangeText->SetFillStyle(0);
//     rangeText->AddText(Form("pT: %.1f - %.1f GeV/c", pTMin, pTMax));
//     rangeText->AddText(Form("#eta: %.1f - %.1f", etaMin, etaMax));
//     // if(useMVA_) rangeText->AddText(Form("mva: %.1f - %.1f", mvaMin, mvaMax));
//     rangeText->Draw("same");
    
//     // Draw pull distribution in lower pad
//     padDown->cd();
//     RooHist* pullHist = frame->pullHist("data", "fit");
//     RooPlot* pullFrame = activeMassVar_->frame(RooFit::Title(" "));
//     pullFrame->addPlotable(pullHist, "P");
//     pullFrame->SetMinimum(-2);
//     pullFrame->SetMaximum(2);
//     pullFrame->GetYaxis()->SetTitle("");
//     pullFrame->GetYaxis()->SetTitleOffset(0.3);
//     pullFrame->GetYaxis()->SetTitleSize(0.1);
//     pullFrame->GetYaxis()->SetNdivisions(505);
//     pullFrame->GetYaxis()->SetLabelSize(0.15);
//     pullFrame->GetXaxis()->SetTitleSize(0.12);
//     pullFrame->GetXaxis()->SetLabelSize(0.15);
//     pullFrame->Draw();

    
//     // Draw horizontal lines at y=0, ±1, ±2
//     TLine* line0 = new TLine(activeMassVar_->getMin(), 0, activeMassVar_->getMax(), 0);
//     line0->SetLineStyle(2);
//     line0->Draw();
    
//     for (int i = 1; i <= 2; ++i) {
//         TLine* lineUp = new TLine(activeMassVar_->getMin(), i, activeMassVar_->getMax(), i);
//         TLine* lineDown = new TLine(activeMassVar_->getMin(), -i, activeMassVar_->getMax(), -i);
//         lineUp->SetLineStyle(3);
//         lineDown->SetLineStyle(3);
//         lineUp->Draw();
//         lineDown->Draw();
//     }
//     }
//     else{ frame->Draw();}
//     // CMS_lumi(canvas);
    
//     canvas->Update();
//     canvas->Modified();
//     canvas->SaveAs(Form("../Plots/test_%s_%s_pT%dto%d.pdf",name_.c_str(),massVar_.c_str(),int(pTMin),int(pTMax)));
//     return;
//     }

// void MassFitter::init(FitOpt opt, string filepath) {
//     TFile* file = TFile::Open(filepath.c_str());
//     if (!file || file->IsZombie()) {
//         std::cerr << "Error: Failed to open file " << filepath << std::endl;
//         return;
//     }
//     // 데이터셋 로드
//     RooDataSet* dataset = (RooDataSet*)file->Get(opt.datasetName.c_str());
//     if (!dataset) {
//         std::cerr << "Error: Failed to load dataset from file" << std::endl;
//         file->Close();
//         return;
//     }
//     SetData(dataset);
//     ApplyCut(opt.cutExpr);
//     // 신호 PDF 설정
//     // file->Close();
// }

#endif // MASS_FITTER_H
