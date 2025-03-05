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

// 상수 정의
constexpr double PION_MASS = 0.13957039;

// 외부 파라미터 구조체 사용을 위한 타입 별칭 정의
using GaussianParams = PDFParams::GaussianParams;
using DoubleGaussianParams = PDFParams::DoubleGaussianParams;
using CrystalBallParams = PDFParams::CrystalBallParams;
using DBCrystalBallParams = PDFParams::DBCrystalBallParams;
using VoigtianParams = PDFParams::VoigtianParams;
using BreitWignerParams = PDFParams::BreitWignerParams;
using PhenomenologicalParams = PDFParams::PhenomenologicalParams;
using ExponentialBkgParams = PDFParams::ExponentialBkgParams;
using ChebychevBkgParams = PDFParams::ChebychevBkgParams;
using PolynomialBkgParams = PDFParams::PolynomialBkgParams;

class MassFitter {
public:
    // 생성자 & 소멸자
    MassFitter(const std::string& name, double massMin, double massMax);
    ~MassFitter();
    
    
    // 데이터 설정
    template <typename SigPar, typename BkgPar>
    void init(FitOpt opt,std::string filepath_, bool inclusive, const std::string pTbin,const std::string etabin, SigPar sigParams, BkgPar bkgParams);
    void SetData(RooDataSet* dataset);
    void ApplyCut(const std::string& cutExpr);
    
    // Signal PDF 설정 메서드
    void SetSignalPDF(const GaussianParams& params,const std::string &name);
    void SetSignalPDF(const DoubleGaussianParams& params, const std::string &name);
    void SetSignalPDF(const CrystalBallParams& params, const std::string &name);
    void SetSignalPDF(const DBCrystalBallParams& params, const std::string &name);
    void SetSignalPDF(const VoigtianParams& params, const std::string &name);
    
    // Background PDF 설정 메서드
    void SetBackgroundPDF(const ExponentialBkgParams& params, const std::string& name);
    void SetBackgroundPDF(const ChebychevBkgParams& params, const std::string& name);
    void SetBackgroundPDF(const PolynomialBkgParams& params, const std::string& name);
    void SetBackgroundPDF(const PhenomenologicalParams& params, const std::string &name);
    
    // 피팅 수행
    RooFitResult* Fit(bool useMinos = false, bool useHesse = true, bool verbose = false, double rangeMin = 1.9, double rangeMax = 2.1);
    
    // 결과 접근 메서드
    double GetSignalYield() const;
    double GetSignalYieldError() const;
    double GetBackgroundYield() const;
    double GetBackgroundYieldError() const;
    double GetChiSquare() const;
    double GetNDF() const;
    double GetReducedChiSquare() const;
    
    // 플로팅 메서드
    // TCanvas* PlotFit(bool drawPull = true, const std::string& title = "");
    
    // 저장 메서드
    void SaveResults(const std::string& filename, bool saveWorkspace = false);
    void PlotResult(bool drawFitModel, std::string filename_, double pTMin, double pTMax, double etaMin, double etaMax);
    void PlotResult(const std::string& filename, bool saveWorkspace = false);
    
    // Delta mass 모드 설정 (D* 분석용)
    void UseDeltaMass(bool use = true, double daughterMassMin = 1.8, double daughterMassMax = 1.9);
    
private:
    // 내부 PDF 생성 메서드
    void MakeGaussian(const GaussianParams& params, const std::string& name = "gauss");
    void MakeDoubleGaussian(const DoubleGaussianParams& params, const std::string& name = "doublegauss");
    void MakeCrystalBall(const CrystalBallParams& params, const std::string& name = "cb");
    void MakeDBCrystalBall(const DBCrystalBallParams& params, const std::string& name = "doublecb");
    void MakeVoigtian(const VoigtianParams& params, const std::string& name = "voigt");
    
    void MakeExponential(const ExponentialBkgParams& params, const std::string& name = "exp");
    void MakeChebychev(const ChebychevBkgParams& params, const std::string& name = "cheb");
    void MakePolynomial(const PolynomialBkgParams& params, const std::string& name = "poly");
    void MakePhenomenological(const PhenomenologicalParams& params, const std::string& name = "phenom");
    
    // 전체 PDF 생성
    void CombinePDFs();
    
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
    
    // PDF 파라미터 보관용 맵
    std::map<std::string, RooRealVar*> parameters_;
};

void MassFitter::MakeGaussian(const GaussianParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma = new RooRealVar(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    
    signal_pdf_= new RooGaussian(name.c_str(), ("Gaussian_" + name).c_str(), 
                          *activeMassVar_, *mean, *sigma);
}

void MassFitter::MakeDoubleGaussian(const DoubleGaussianParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooRealVar* mean1 = new RooRealVar(("mean1_" + name).c_str(), "First Mean",
                                      params.mean1, params.mean1_min, params.mean1_max);
    RooRealVar* sigma1 = new RooRealVar(("sigma1_" + name).c_str(), "First Sigma",
                                       params.sigma1, params.sigma1_min, params.sigma1_max);
    RooRealVar* mean2 = new RooRealVar(("mean2_" + name).c_str(), "Second Mean",
                                      params.mean2, params.mean2_min, params.mean2_max);
    RooRealVar* sigma2 = new RooRealVar(("sigma2_" + name).c_str(), "Second Sigma",
                                       params.sigma2, params.sigma2_min, params.sigma2_max);
    RooRealVar* frac = new RooRealVar(("frac_" + name).c_str(), "Fraction",
                                     params.fraction, params.fraction_min, params.fraction_max);

    RooGaussian* gauss1 = new RooGaussian(("gauss1_" + name).c_str(), "First Gaussian",
                                         *activeMassVar_, *mean1, *sigma1);
    RooGaussian* gauss2 = new RooGaussian(("gauss2_" + name).c_str(), "Second Gaussian",
                                         *activeMassVar_, *mean1, *sigma2);
    
    signal_pdf_= new RooAddPdf(name.c_str(), ("DoubleGaussian_" + name).c_str(),
                        RooArgList(*gauss1, *gauss2), *frac);
}

void MassFitter::MakeCrystalBall(const CrystalBallParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma = new RooRealVar(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    RooRealVar* alpha = new RooRealVar(("alpha_" + name).c_str(), "Alpha",
                                      params.alpha, params.alpha_min, params.alpha_max);
    RooRealVar* n = new RooRealVar(("n_" + name).c_str(), "N",
                                  params.n, params.n_min, params.n_max);
    
    signal_pdf_= new RooCBShape(name.c_str(), ("CrystalBall_" + name).c_str(),
                         *activeMassVar_, *mean, *sigma, *alpha, *n);
}

void MassFitter::MakeDBCrystalBall(const DBCrystalBallParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
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
    
    signal_pdf_ = new RooCrystalBall(name.c_str(), ("DoubleSidedCrystalBall_" + name).c_str(),
                                  *activeMassVar_, *mean, *sigmaL,*sigmaR, *alphaL, *nL, *alphaR, *nR);
}

void MassFitter::MakeVoigtian(const VoigtianParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    RooRealVar* sigma = new RooRealVar(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    RooRealVar* width = new RooRealVar(("width_" + name).c_str(), "Width",
                                      params.width, params.width_min, params.width_max);
    
    signal_pdf_= new RooVoigtian(name.c_str(), ("Voigtian_" + name).c_str(),
                          *activeMassVar_, *mean, *width, *sigma);
}

void MassFitter::MakeExponential(const ExponentialBkgParams& params, const std::string& name) {
    // if (!activeMassVar_ || params.coefficients.empty()) return nullptr;
    
    RooRealVar* slope = new RooRealVar(("slope_" + name).c_str(), "Slope",
                                      params.lambda,
                                      params.lambda_min,
                                      params.lambda_max);
    
    background_pdf_= new RooExponential(name.c_str(), ("Exponential_" + name).c_str(),
                             *activeMassVar_, *slope);
}

void MassFitter::MakeChebychev(const ChebychevBkgParams& params, const std::string& name) {
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
    
    background_pdf_= new RooChebychev(name.c_str(), ("Chebyshev_" + name).c_str(),
                           *activeMassVar_, coefList);
}
void MassFitter::MakePolynomial(const PolynomialBkgParams& params, const std::string& name) {
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
    
    background_pdf_= new RooChebychev(name.c_str(), ("Polynomial_" + name).c_str(),
                           *activeMassVar_, coefList);
}
void MassFitter::MakePhenomenological(const PhenomenologicalParams& params, const std::string& name) {
    // if (!activeMassVar_) return nullptr;
    
    RooRealVar* m0 = new RooRealVar("m0", "m0",PION_MASS );        // Parameter m0
    RooRealVar* p0 = new RooRealVar("p0", "p0", params.p0, params.p0_min, params.p0_max);          // Parameter p0
    RooRealVar* p1 = new RooRealVar("p1", "p1", params.p1, params.p1_min, params.p1_max);          // Parameter p1
    RooRealVar* p2 = new RooRealVar("p2", "p2", params.p2, params.p2_min, params.p2_max);          // Parameter p2
    RooFormulaVar* exp_term= new RooFormulaVar("exp_term", "1 - exp(-(@0 - @1)/@2)", RooArgList(*activeMassVar_, *m0, *p0));
    RooFormulaVar* pow_term= new RooFormulaVar("pow_term", "pow(@0/@1, @2)", RooArgList(*activeMassVar_, *m0, *p1));
    RooFormulaVar* lin_term= new RooFormulaVar("lin_term", "@0 * (@1/@2 - 1)", RooArgList(*p2, *activeMassVar_, *m0));
    
    background_pdf_= new RooGenericPdf("pdf", "exp_term * pow_term + lin_term",RooArgList(*exp_term, *pow_term, *lin_term));
}

void MassFitter::CombinePDFs() {
    if (!signal_pdf_ || !background_pdf_){
        std::cerr << "Signal and background PDFs must be set before combining." << std::endl;
        return;
    } 
    int a =reduced_data_->sumEntries();
    
    RooRealVar* nsig = new RooRealVar("nsig", "Signal Yield", 1000, 0., 2.*a);
    RooRealVar* nbkg = new RooRealVar("nbkg", "Background Yield", 1e4, a/100., 2.*a);
    
    total_pdf_= new RooAddPdf("total_pdf", "Signal + Background",
                        RooArgList(*signal_pdf_, *background_pdf_),
                        RooArgList(*nsig, *nbkg));
}
MassFitter::MassFitter(const std::string& name, double massMin, double massMax) 
    : name_(name), 
      massVar_("mass"), 
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
    // 워크스페이스가 소유권을 가지므로 여기서는 명시적으로 삭제하지 않음
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
    MakeGaussian(params, pdfName);
}

void MassFitter::SetSignalPDF(const DoubleGaussianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    MakeDoubleGaussian(params, pdfName);
}

void MassFitter::SetSignalPDF(const CrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    MakeCrystalBall(params, pdfName);
}

void MassFitter::SetSignalPDF(const DBCrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    MakeDBCrystalBall(params, pdfName);
}

void MassFitter::SetSignalPDF(const VoigtianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    MakeVoigtian(params, pdfName);
}

// Background PDF 설정 메서드 - 타입 문자열 제거
void MassFitter::SetBackgroundPDF(const ExponentialBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    MakeExponential(params, pdfName);
}

void MassFitter::SetBackgroundPDF(const ChebychevBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    MakeChebychev(params, pdfName);
}

void MassFitter::SetBackgroundPDF(const PolynomialBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    MakePolynomial(params, pdfName);
}
void MassFitter::SetBackgroundPDF(const PhenomenologicalParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    MakePhenomenological(params, pdfName);
}

RooFitResult* MassFitter::Fit(bool useMinos, bool useHesse, bool verbose, double rangeMin, double rangeMax) {
    if (!signal_pdf_ || !background_pdf_ || !reduced_data_) {
        std::cerr << "Error: Signal PDF, background PDF, or dataset not set" << std::endl;
        return nullptr;
    }
    
    // 신호+배경 PDF 생성
    CombinePDFs();
    mass_->setRange("analysis", rangeMin, rangeMax);
    
    // 로그 레벨 조정
    RooMsgService::instance().setGlobalKillBelow(verbose ? RooFit::INFO : RooFit::WARNING);
    
    // 피팅 수행
    fit_result_ = total_pdf_->fitTo(*reduced_data_, 
                                    RooFit::Extended(true), 
                                    RooFit::Save(true),
                                    RooFit::Minos(useMinos),
                                    RooFit::Hesse(useHesse),
                                    RooFit::PrintLevel(verbose ? 1 : -1),
                                    RooFit::Range("analysis"),
                                    RooFit::NormRange("analysis"));
    
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
    workspace_->import(*reduced_data_);
    
    return fit_result_;
}

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

double MassFitter::GetNDF() const {
    if (!fit_result_) return 0.0;
    return fit_result_->floatParsFinal().getSize();
}

double MassFitter::GetReducedChiSquare() const {
    double ndf = GetNDF();
    return (ndf > 0) ? GetChiSquare() / ndf : -1.0;
}
template <typename SigPar, typename BkgPar>
void MassFitter::init(FitOpt opt, std::string filepath_, bool inclusive, const std::string pTbin, const std::string etabin, SigPar sigParams, BkgPar bkgParams) {
    // Open ROOT file
    TFile* file = TFile::Open(filepath_.c_str());
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Failed to open file " << filepath_ << std::endl;
        return;
    }
    
    // Get the dataset from the file
    RooDataSet* dataset = dynamic_cast<RooDataSet*>(file->Get(opt.datasetName.c_str()));
    if (!dataset) {
        std::cerr << "Error: Failed to load dataset '" << opt.datasetName << "' from file" << std::endl;
        file->Close();
        return;
    }
    
    // Set the dataset
    SetData(dataset);
    
    // Apply cuts based on inclusive flag
    if (inclusive) {
        ApplyCut(opt.cutExpr);
    } else {
        ApplyCut(opt.cutExpr + "&&" + pTbin + "&&" + etabin);
    }

    // Create and set PDFs
    SetSignalPDF(sigParams, "");
    SetBackgroundPDF(bkgParams, "");

    // Perform fit and save results
    Fit(opt.useMinos, opt.useHesse, opt.verbose,opt.massMin,opt.massMax);
    SaveResults(opt.outputFile, opt.saveWorkspace);
    
    // Close the file (dataset is already copied to MassFitter)
    file->Close();
}

void MassFitter::SaveResults(const std::string& filename, bool saveWorkspace) {
    TFile* outFile = new TFile(filename.c_str(), "RECREATE");
    // 피팅 결과 저장
    if (fit_result_) {
        fit_result_->Write("fitResult");
    }
    if (reduced_data_) {
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
    
    std::cout << "Results saved to " << filename << std::endl;
}

void MassFitter::PlotResult(bool drawFitModel, std::string filename_, double pTMin=0, double pTMax=0, double etaMin=0, double etaMax=0) {
    TFile file(filename_.c_str(), "READ");
    if(drawFitModel){
    fit_result_ = dynamic_cast<RooFitResult*>(file.Get("fit_result"));
    total_pdf_ = dynamic_cast<RooAddPdf*>(file.Get("total_pdf"));
    }
    // reduced_data_ = dynamic_cast<RooDataSet*>(file.Get("dataset"));
    // RooRealVar* mass = dynamic_cast<RooRealVar*>(total_pdf_->getVariables()->find("mass"));
    // activeMassVar_ = mass;
    if (!total_pdf_ || !fit_result_) cout << "no total pdf" << endl;
        RooDataSet* dataToPlot = reduced_data_;
        if (!dataToPlot){
	cout << "no data to Plot" << endl;
	 return false;
	}

    // Create canvas with two pads: main plot and pull distribution
    TCanvas* canvas = new TCanvas("canvas", "Fit Result", 1000, 800);
    canvas->Divide(1, 2);
    
    // Upper pad for main plot (takes 70% of canvas)
    TPad* padUp = new TPad("padUp", "padUp", 0, 0.2, 1, 1);
    padUp->SetBottomMargin(0.0);
    padUp->SetTopMargin(0.12);
    padUp->SetLeftMargin(0.1);
    padUp->Draw();
    
    // Lower pad for pull distribution (takes 30% of canvas)
    TPad* padDown = new TPad("padDown", "padDown", 0, 0, 1, 0.2);
    padDown->SetTopMargin(0.01);
    padDown->SetLeftMargin(0.1);
    padDown->SetBottomMargin(0.3);
    padDown->Draw();
    
    // Draw main plot
    padUp->cd();
    RooPlot* frame = activeMassVar_->frame(RooFit::Title(" "));
    
    // Data points
    cout << "adsasd" << endl;
    reduced_data_->plotOn(frame, RooFit::Name("data"));
    cout << "asd" << endl;
    
    // Full model
if(total_pdf_ != nullptr) {
    total_pdf_->plotOn(frame, RooFit::Name("fit"));
    
    // Signal component
    if (signal_pdf_) {
        // Check if signal_pdf_ is a composite PDF
        const RooArgSet* components = signal_pdf_->getComponents();
        if (components->getSize() > 1) {
            // Iterate over the components and plot each one
            TIterator* iter = components->createIterator();
            RooAbsArg* component;
            int colorIndex = 5; // Start with a different color for each component
            while ((component = (RooAbsArg*)iter->Next())) {
                RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(component);
                if (pdfComponent) {
                    total_pdf_->plotOn(frame, 
                                       RooFit::Components(*pdfComponent),
                                       RooFit::LineStyle(kDashed),
                                       RooFit::LineColor(colorIndex),
                                       RooFit::Name(pdfComponent->GetName()));
                    colorIndex++;
                }
            }
            delete iter;
        } else {
            // Plot the single signal component
            total_pdf_->plotOn(frame, 
                               RooFit::Components(*signal_pdf_),
                               RooFit::LineStyle(kDashed),
                               RooFit::LineColor(kRed),
                               RooFit::Name("signal"));
        }
    }
    
    // Background component
    if (background_pdf_) {
        total_pdf_->plotOn(frame, 
                           RooFit::Components(*background_pdf_),
                           RooFit::LineStyle(kDashed),
                           RooFit::LineColor(kGreen),
                           RooFit::Name("background"));
    }

    // Add chi2 value
    double chi2 = frame->chiSquare("fit", "data", fit_result_->floatParsFinal().getSize());
    
    // Create legend
    TLegend* legend = new TLegend(0.70, 0.2, 0.85, 0.35);
    legend->AddEntry(frame->findObject("data"), "Data", "p");
    legend->AddEntry(frame->findObject("fit"), "Total Fit", "l");
    if (signal_pdf_) {
    const RooArgSet* components = signal_pdf_->getComponents();
    if (components->getSize() > 1) {
        TIterator* iter = components->createIterator();
        RooAbsArg* component;
        while ((component = (RooAbsArg*)iter->Next())) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(component);
            if (pdfComponent) {
                legend->AddEntry(frame->findObject(pdfComponent->GetName()), pdfComponent->GetName(), "l");
            }
        }
        delete iter;
    } else {
        legend->AddEntry(frame->findObject("signal"), "Signal", "l");
    }
    }
    if (background_pdf_) legend->AddEntry(frame->findObject("background"), "Background", "l");
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    
    // Draw frame and legend
    frame->Draw();
    legend->Draw("same");
    
    // Add fit parameters text box
    TPaveText* paveText = new TPaveText(0.65, 0.10, 0.90, 0.45, "NDC");
    paveText->SetBorderSize(0);
    paveText->SetFillStyle(0);
    paveText->AddText(Form("#chi^{2}/ndf = %.2f", chi2));
    
    // Add fit parameters with errors
    RooArgList params = fit_result_->floatParsFinal();
    for (int i = 0; i < params.getSize(); ++i) {
        RooRealVar* param = (RooRealVar*)params.at(i);
        paveText->AddText(Form("%s = %.4f #pm %.4f", 
                              param->GetName(),
                              param->getVal(),
                              param->getError()));
    }
    // paveText->Draw("same");
    TPaveText* rangeText = new TPaveText(0.15, 0.65, 0.40, 0.8, "NDC");
    rangeText->SetBorderSize(0);
    rangeText->SetFillStyle(0);
    rangeText->AddText(Form("pT: %.1f - %.1f GeV/c", pTMin, pTMax));
    rangeText->AddText(Form("#eta: %.1f - %.1f", etaMin, etaMax));
    // if(useMVA_) rangeText->AddText(Form("mva: %.1f - %.1f", mvaMin, mvaMax));
    rangeText->Draw("same");
    
    // Draw pull distribution in lower pad
    padDown->cd();
    RooHist* pullHist = frame->pullHist("data", "fit");
    RooPlot* pullFrame = activeMassVar_->frame(RooFit::Title(" "));
    pullFrame->addPlotable(pullHist, "P");
    pullFrame->SetMinimum(-2);
    pullFrame->SetMaximum(2);
    pullFrame->GetYaxis()->SetTitle("");
    pullFrame->GetYaxis()->SetTitleOffset(0.3);
    pullFrame->GetYaxis()->SetTitleSize(0.1);
    pullFrame->GetYaxis()->SetNdivisions(505);
    pullFrame->GetYaxis()->SetLabelSize(0.15);
    pullFrame->GetXaxis()->SetTitleSize(0.12);
    pullFrame->GetXaxis()->SetLabelSize(0.15);
    pullFrame->Draw();

    
    // Draw horizontal lines at y=0, ±1, ±2
    TLine* line0 = new TLine(activeMassVar_->getMin(), 0, activeMassVar_->getMax(), 0);
    line0->SetLineStyle(2);
    line0->Draw();
    
    for (int i = 1; i <= 2; ++i) {
        TLine* lineUp = new TLine(activeMassVar_->getMin(), i, activeMassVar_->getMax(), i);
        TLine* lineDown = new TLine(activeMassVar_->getMin(), -i, activeMassVar_->getMax(), -i);
        lineUp->SetLineStyle(3);
        lineDown->SetLineStyle(3);
        lineUp->Draw();
        lineDown->Draw();
    }
    }
    else{ frame->Draw();}
    // CMS_lumi(canvas);
    
    canvas->Update();
    canvas->Modified();
    canvas->SaveAs(Form("../Plots/test_%s_%s_pT%dto%d.pdf",name_.c_str(),massVar_.c_str(),int(pTMin),int(pTMax)));
    return true;
    }

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