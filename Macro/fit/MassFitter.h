#ifndef MASS_FITTER_H
#define MASS_FITTER_H

#include <string>
#include <map>
#include <memory>
#include <vector>
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
    // 생성자 - nsig, nbkg 비율만 받도록 수정
    MassFitter(const std::string& name, std::string& massvar, double massMin, double massMax,
               double nsig_ratio = 0.1, double nsig_min_ratio = 0.0, double nsig_max_ratio = 1.0,
               double nbkg_ratio = 0, double nbkg_min_ratio = 0.0, double nbkg_max_ratio = 1.0);
    ~MassFitter();
    
    // 데이터 설정
    template <typename SigPar, typename BkgPar>
    void PerformFit(FitOpt opt, RooDataSet* dataset, bool inclusive, const std::string pTbin,const std::string etabin, SigPar sigParams, BkgPar bkgParams);
    template <typename SigMCPar>
    void PerformMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams);
    template <typename SigMCPar>
    void PerformDCAMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams);
    template <typename SigMCPar, typename SigPar, typename BkgPar>
    void PerformConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams, SigPar sigParams, BkgPar bkgParams);
    template <typename SigPar, typename BkgPar>
    void PerformConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigPar sigParams, BkgPar bkgParams);
    template <typename SigPar, typename BkgPar>
    void PerformDCAConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigPar sigParams, BkgPar bkgParams);
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
    RooFitResult* FitMC(bool useMinos, bool useHesse, bool verbose ,bool useCUDA, double rangeMin , double rangeMax);
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
    RooWorkspace* GetWorkspace() const { return workspace_.get(); } 
    RooFitResult* GetFitResult() const { return fit_result_.get(); }
    RooFitResult* GetMCFitResult() const { return mc_fit_result_; }
    RooWorkspace* GetMCWorkspace() const { return mcworkspace_; }
    // void SaveSPlotResults(const std::string& filePath,const std::string& fileName, bool saveWorkspace =true);
    
    // Delta mass 모드 설정 (D* 분석용)
    void UseDeltaMass(bool use = true, double daughterMassMin = 1.8, double daughterMassMax = 1.9);
    
private:
    std::unique_ptr<RooFitResult> fixFit(RooFitResult* initialResult);
    std::unique_ptr<RooAbsPdf> MakeGaussian(const GaussianParams& params, const std::string& name = "gauss");
    std::unique_ptr<RooAbsPdf> MakeDoubleGaussian(const DoubleGaussianParams& params, const std::string& name = "doublegauss");
    std::unique_ptr<RooAbsPdf> MakeCrystalBall(const CrystalBallParams& params, const std::string& name = "cb");
    std::unique_ptr<RooAbsPdf> MakeDBCrystalBall(const DBCrystalBallParams& params, const std::string& name = "doublecb");
    std::unique_ptr<RooAbsPdf> MakeDoubleDBCrystalBall(const DoubleDBCrystalBallParams& params, const std::string& name = "doublecb");
    std::unique_ptr<RooAbsPdf> MakeVoigtian(const VoigtianParams& params, const std::string& name = "voigt");

    std::unique_ptr<RooAbsPdf> MakeExponential(const ExponentialBkgParams& params, const std::string& name = "exp");
    std::unique_ptr<RooAbsPdf> MakeChebychev(const ChebychevBkgParams& params, const std::string& name = "cheb");
    std::unique_ptr<RooAbsPdf> MakePolynomial(const PolynomialBkgParams& params, const std::string& name = "poly");
    std::unique_ptr<RooAbsPdf> MakeRooDstD0Bg(const PhenomenologicalParams& params, const std::string& name = "dstd0");

    // 전체 PDF 생성
    void CombinePDFs();
    void CombineMCPDFs();
    void CombinePDFs(RooAbsPdf* signal_pdf, RooAbsPdf* background_pdf);
    void CombinePDFs(RooAbsPdf* signal_pdf_swap0,RooAbsPdf* signal_pdf_swap1, RooAbsPdf* background_pdf);
    void Clear();

    // 멤버 변수들
    std::string name_;                   // 인스턴스 이름
    std::string massVar_;                // 질량 변수 이름
    bool useDeltaMass_;                  // Delta mass 모드 여부

    // RooFit 변수들 (스마트 포인터 사용)
    std::unique_ptr<RooRealVar> mass_;                   // 주 질량 변수
    std::unique_ptr<RooRealVar> massDaughter1_;          // 딸 입자 질량 변수  
    std::unique_ptr<RooRealVar> massPion_;               // 파이온 질량 변수
    RooRealVar* activeMassVar_;          // 활성 질량 변수 포인터 (위 중 하나를 가리킴)

    // PDF 객체들 (스마트 포인터 사용)
    std::unique_ptr<RooAbsPdf> signal_pdf_;              // 신호 PDF
    std::unique_ptr<RooAbsPdf> swap0_pdf_;               // Swap0 PDF
    std::unique_ptr<RooAbsPdf> swap1_pdf_;               // Swap1 PDF
    std::unique_ptr<RooAbsPdf> background_pdf_;          // 배경 PDF
    std::unique_ptr<RooAddPdf> total_pdf_;               // 전체 PDF (신호 + 배경)

    // 데이터 객체들
    RooDataSet* full_data_;              // 전체 데이터셋 (외부에서 전달받음)
    RooDataSet* reduced_data_;           // 컷 적용된 데이터셋 (외부에서 전달받음)

    // 결과 객체들 (스마트 포인터 사용)
    std::unique_ptr<RooFitResult> fit_result_;           // 피팅 결과
    RooFitResult* mc_fit_result_; 
    std::unique_ptr<RooWorkspace> workspace_;            // 작업공간
    RooWorkspace* mcworkspace_;

    // 신호 및 배경 수득률 변수들 (스마트 포인터 사용)
    std::unique_ptr<RooRealVar> nsig_;                   // 신호 수득률
    std::unique_ptr<RooRealVar> nbkg_;                   // 배경 수득률
    std::unique_ptr<RooRealVar> frac_;                   // 배경 수득률
    std::unique_ptr<RooFormulaVar> nSigSwap_;

    // PDF 파라미터 보관용 벡터 (스마트 포인터 사용)
    std::vector<std::unique_ptr<RooRealVar>> parameters_;

    double nsig_ratio_;              
    double nsig_min_ratio_;          
    double nsig_max_ratio_;          
    double nbkg_ratio_;              
    double nbkg_min_ratio_;          
    double nbkg_max_ratio_;          
};


std::unique_ptr<RooAbsPdf> MassFitter::MakeGaussian(const GaussianParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr; // 활성 변수 확인

    auto mean = std::make_unique<RooRealVar>(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    auto sigma = std::make_unique<RooRealVar>(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);

    // 파라미터들을 저장
    RooRealVar* meanPtr = mean.get();
    RooRealVar* sigmaPtr = sigma.get();
    parameters_.push_back(std::move(mean));
    parameters_.push_back(std::move(sigma));

    // 생성된 PDF 포인터 반환
    return std::make_unique<RooGaussian>(name.c_str(), ("Gaussian_" + name).c_str(),
                          *activeMassVar_, *meanPtr, *sigmaPtr);
}
std::unique_ptr<RooFitResult> MassFitter::fixFit(RooFitResult* initialResult) {
    
    RooFitResult* tempResult = initialResult;
    bool needRefit = true;
    int fitnum = 0;
    
    while(needRefit) {
        if (fitnum > 0) tempResult->Print("v");

        auto fpf  = tempResult->floatParsFinal();
        auto cpf  = tempResult->constPars();
        auto* modelVars = total_pdf_->getVariables();
        
        // 상수 파라미터 고정 유지
        for(int idx = 0; idx < cpf.getSize(); ++idx) {
            auto var = (TObject*)&cpf[idx];
            std::string _vname_ = string(var->GetName());
            cout << " >> Fixing parameter: " << _vname_ << std::endl;
            auto pdfVar = dynamic_cast<RooRealVar*>(modelVars->find(_vname_.c_str()));
            if (pdfVar) pdfVar->setConstant(kTRUE);
        }

        // Floating 파라미터 범위 조정
        for(int idx = 0; idx < fpf.getSize(); ++idx) {
            auto var = dynamic_cast<RooRealVar*>(&fpf[idx]);
            if (!var) continue;

            string _vname_ = string(var->GetName());
            auto pdfVar = dynamic_cast<RooRealVar*>(modelVars->find(_vname_.c_str()));
            if (!pdfVar) continue;

            double _val_ = var->getVal();
            double _limup_ = var->getMax();
            double _limdown_ = var->getMin();
            double _errup_ = var->getErrorHi();
            double _errdo_ = var->getErrorLo();
            
            if( _val_ + 3 * _errup_ > _limup_ || (_val_ > 0 && 1.2 * _val_ > _limup_) || (_val_ < 0 && 0.8 * _val_ > _limup_)) {
                if(_vname_.find("frac") != std::string::npos){ cout << "skip exapnding upper limit for frac" << std::endl; continue; } 
                if(_vname_.find("nsig") != std::string::npos) {cout << "skip exapnding upper limit for nsig" << std::endl; continue; }
                if(_vname_.find("nbkg") != std::string::npos) {cout << "skip exapnding upper limit for nbkg" << std::endl; continue; }
                if(_vname_.find("alpha") != std::string::npos) {cout << "skip exapnding upper limit for alpha" << std::endl; continue; }
                if(_vname_.find("n") != std::string::npos){cout << "Expanding upper limit for n" << std::endl; pdfVar->setMax(pdfVar->getMax() * 2); continue;}
                if(_vname_.find("mean") != std::string::npos) {cout << "skip exapnding upper limit for mean" << std::endl; continue; }
                // if(_vname_.find("p0")!= std::string::npos || _vname_.find("p1") != std::string::npos || _vname_.find("p2") != std::string::npos) {
                if(_vname_.find("p0")!= std::string::npos) {
                    // pdfVar->setMax(pdfVar->getMax() * 2);
                    std::cout << " >> Expanding upper limit for " << _vname_ << " to " << pdfVar->getMax() << std::endl;
                    continue;
                }
                pdfVar->setMax(_val_ + _errup_ * 9.0);
                std::cout << " >> Expanding upper limit for " << _vname_ << " to " << pdfVar->getMax() << std::endl;
            }
            if(_val_ + 3 * _errdo_ < _limdown_ || (_val_ > 0 && 0.8 * _val_ < _limdown_) || (_val_ < 0 && 1.2 * _val_ < _limdown_)) {
                if(_vname_.find("sigma") != std::string::npos) {cout << "skip exapnding lower limit for sigma" << std::endl; continue; }
                if(_vname_.find("frac") != std::string::npos) { cout << "skip exapnding lower limit for frac" << std::endl; continue; }
                if(_vname_.find("nsig") != std::string::npos) {cout << "skip exapnding lower limit for nsig" << std::endl; continue; }
                if(_vname_.find("nbkg") != std::string::npos) {cout << "skip exapnding lower limit for nbkg" << std::endl; continue; }
                if(_vname_.find("mean") != std::string::npos) {cout << "skip exapnding lower limit for mean" << std::endl; continue; }
                // if(_vname_.find("p0")!= std::string::npos || _vname_.find("p1") != std::string::npos || _vname_.find("p2") != std::string::npos) {
                if(_vname_.find("p0")!= std::string::npos) {
                    pdfVar->setMin(pdfVar->getMin() * 0.5);
                    std::cout << " >> Expanding lower limit for " << _vname_ << " to " << pdfVar->getMin() << std::endl;
                    continue;
                }
                // if(_vname_.find("alpha") != std::string::npos){pdfVar->setMin(pdfVar->getMin() * 0.1);} continue;
                if(_vname_.find("alpha") != std::string::npos) {cout << "skip exapnding lower limit for alpha" << std::endl; continue; }
                if(_vname_.find("n") != std::string::npos){
                    std::cout << "Expanding lower limit for n" << std::endl; 
                }
                pdfVar->setMin(_val_ + _errdo_ * 9.0);
                std::cout << " >> Expanding lower limit for " << _vname_ << " to " << pdfVar->getMin() << std::endl;
            }
        }
        delete modelVars;

        RooLinkedList fitOpts;
        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(24)));
        fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(1)));
        fitOpts.Add(new RooCmdArg(RooFit::Save()));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit","Minimizer")));
        fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
        fitOpts.Add(new RooCmdArg(RooFit::SumW2Error(true)));
        fitOpts.Add(new RooCmdArg(RooFit::Optimize(1)));
        fitOpts.Add(new RooCmdArg(RooFit::Hesse(true)));
        // fitOpts.Add(new RooCmdArg(RooFit::Minos(true)));
        fitOpts.Add(new RooCmdArg(RooFit::Range("analysis")));
        // fitOpts.Add(new RooCmdArg(RooFit::))

        
        // if (fitnum > 0) delete tempResult; // 이전 루프의 결과 삭제

        tempResult = total_pdf_->fitTo(*reduced_data_, fitOpts);
        // fitOpts.Add(new RooCmdArg(RooFit::Strategy(2)));
           int ntry = -1;
    while(tempResult->status() !=0 && (ntry+1)<3){
        ntry++;
        std::cout << "Initial fit failed with status " << initialResult->status() 
                  << ". Retrying..." << std::endl;
        tempResult = total_pdf_->fitTo(*reduced_data_,
                                        RooFit::Minimizer("Minuit","Minimizer"),
                                        RooFit::NumCPU(24),
    #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
                                        // RooFit::EvalBackend(useCUDA ? "cuda" : "CPU"),  
    #endif
                                        RooFit::Optimize(1),
                                        RooFit::Extended(true),
                                        RooFit::Save(true),
                                        // RooFit::Minos(true),
                                        RooFit::Hesse(true),
                                        // RooFit::PrintLevel(verbose ? 1 : -1),
                                        RooFit::Strategy(2-ntry),
                                        RooFit::Range("analysis"));
    }
        fitnum++;
        
        auto fpf2 = tempResult->floatParsFinal();
        bool touchlimit = false;
         for( int idx=0;idx<fpf2.getSize();idx++ ){
                auto var = (RooRealVar*) &fpf2[idx];
                double _val_ = var->getVal();
                double _limup_ = var->getMax();
                double _limdown_ = var->getMin();
                double _errup_ = var->getErrorHi();
                double _errdo_ = var->getErrorLo();
                if( _val_ + 3 * _errup_ > _limup_ || (_val_ > 0 && 1.2 * _val_ > _limup_) || (_val_ < 0 && 0.8 * _val_ > _limup_)) touchlimit=1;
                if(_val_ + 3 * _errdo_ < _limdown_ || (_val_ > 0 && 0.8 * _val_ < _limdown_) || (_val_ < 0 && 1.2 * _val_ < _limdown_)) touchlimit=1;
            }

        
        needRefit = ((tempResult->statusCodeHistory(0) != 0) || (tempResult->statusCodeHistory(1) != 0) || touchlimit);
    	
        std::cout << "\n\n--- Post-correction Fit Attempt #" << fitnum << " ---" << std::endl;
		tempResult->Print("v");
        
        if (fitnum > 2 && needRefit) { // 최대 3번 시도 후 실패 시 중단
            std::cout << "Fit still fails after " << fitnum << " attempts. Giving up." << std::endl;
            break;
        }
    }
    
    return std::unique_ptr<RooFitResult>(tempResult);
}

std::unique_ptr<RooAbsPdf> MassFitter::MakeDoubleGaussian(const DoubleGaussianParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;

    auto mean1 = std::make_unique<RooRealVar>(("mean1_" + name).c_str(), "First Mean",
                                      params.mean, params.mean_min, params.mean_max);
    auto sigma1 = std::make_unique<RooRealVar>(("sigma1_" + name).c_str(), "First Sigma",
                                       params.sigma1, params.sigma1_min, params.sigma1_max);
    auto mean2 = std::make_unique<RooRealVar>(("mean2_" + name).c_str(), "Second Mean",
                                      params.mean, params.mean_min, params.mean_max);
    auto sigma2 = std::make_unique<RooRealVar>(("sigma2_" + name).c_str(), "Second Sigma",
                                       params.sigma2, params.sigma2_min, params.sigma2_max);
    auto frac = std::make_unique<RooRealVar>(("frac_" + name).c_str(), "Fraction",
                                     params.fraction, params.fraction_min, params.fraction_max);

    // 파라미터 포인터들을 저장
    RooRealVar* mean1Ptr = mean1.get();
    RooRealVar* sigma1Ptr = sigma1.get();
    RooRealVar* mean2Ptr = mean2.get();
    RooRealVar* sigma2Ptr = sigma2.get();
    RooRealVar* fracPtr = frac.get();
    
    parameters_.push_back(std::move(mean1));
    parameters_.push_back(std::move(sigma1));
    parameters_.push_back(std::move(mean2));
    parameters_.push_back(std::move(sigma2));
    parameters_.push_back(std::move(frac));

    auto gauss1 = std::make_unique<RooGaussian>(("gauss1_" + name).c_str(), "First Gaussian",
                                         *activeMassVar_, *mean1Ptr, *sigma1Ptr);
    auto gauss2 = std::make_unique<RooGaussian>(("gauss2_" + name).c_str(), "Second Gaussian",
                                         *activeMassVar_, *mean1Ptr, *sigma2Ptr); // Note: mean1 is used for both

    RooGaussian* gauss1Ptr = gauss1.get();
    RooGaussian* gauss2Ptr = gauss2.get();
    
    // 가우시안 PDF들도 parameters_에 저장하기 위해 별도 벡터 사용
    // (RooAbsPdf는 RooRealVar과 다른 타입이므로 별도 관리 필요)
    gauss1.release(); // 메모리 관리를 RooAddPdf에 위임
    gauss2.release(); // 메모리 관리를 RooAddPdf에 위임

    // 생성된 PDF 포인터 반환
    return std::make_unique<RooAddPdf>(name.c_str(), ("DoubleGaussian_" + name).c_str(),
                        RooArgList(*gauss1Ptr, *gauss2Ptr), *fracPtr);
}
std::unique_ptr<RooAbsPdf> MassFitter::MakeCrystalBall(const CrystalBallParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;

    auto mean = std::make_unique<RooRealVar>(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    auto sigma = std::make_unique<RooRealVar>(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    auto alpha = std::make_unique<RooRealVar>(("alpha_" + name).c_str(), "Alpha",
                                      params.alpha, params.alpha_min, params.alpha_max);
    auto n = std::make_unique<RooRealVar>(("n_" + name).c_str(), "N",
                                  params.n, params.n_min, params.n_max);

    // 파라미터 포인터들을 저장
    RooRealVar* meanPtr = mean.get();
    RooRealVar* sigmaPtr = sigma.get();
    RooRealVar* alphaPtr = alpha.get();
    RooRealVar* nPtr = n.get();
    
    parameters_.push_back(std::move(mean));
    parameters_.push_back(std::move(sigma));
    parameters_.push_back(std::move(alpha));
    parameters_.push_back(std::move(n));

    // 생성된 PDF 포인터 반환
    return std::make_unique<RooCBShape>(name.c_str(), ("CrystalBall_" + name).c_str(),
                         *activeMassVar_, *meanPtr, *sigmaPtr, *alphaPtr, *nPtr);
}

std::unique_ptr<RooAbsPdf> MassFitter::MakeDBCrystalBall(const DBCrystalBallParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;

    auto mean = std::make_unique<RooRealVar>(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    // auto sigmaL = std::make_unique<RooRealVar>(("sigmaL_" + name).c_str(), "sigmaL",
    //                                   params.sigmaL, params.sigmaL_min, params.sigmaL_max);
    // auto sigmaR = std::make_unique<RooRealVar>(("sigmaR_" + name).c_str(), "sigmaR",
    //                                   params.sigmaR, params.sigmaR_min, params.sigmaR_max);
    auto sigma = std::make_unique<RooRealVar>(("sigma_" + name).c_str(), "sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    auto alphaL = std::make_unique<RooRealVar>(("alphaL_" + name).c_str(), "AlphaL",
                                       params.alphaL, params.alphaL_min, params.alphaL_max);
    auto nL = std::make_unique<RooRealVar>(("nL_" + name).c_str(), "NL",
                                   params.nL, params.nL_min, params.nL_max);
    auto alphaR = std::make_unique<RooRealVar>(("alphaR_" + name).c_str(), "AlphaR",
                                       params.alphaR, params.alphaR_min, params.alphaR_max);
    auto nR = std::make_unique<RooRealVar>(("nR_" + name).c_str(), "NR",
                                   params.nR, params.nR_min, params.nR_max);

    // 파라미터 포인터들을 저장
    RooRealVar* meanPtr = mean.get();
    // RooRealVar* sigmaLPtr = sigmaL.get();
    // RooRealVar* sigmaRPtr = sigmaR.get();
    RooRealVar* sigmaPtr = sigma.get();
    RooRealVar* alphaLPtr = alphaL.get();
    RooRealVar* nLPtr = nL.get();
    RooRealVar* alphaRPtr = alphaR.get();
    RooRealVar* nRPtr = nR.get();
    
    parameters_.push_back(std::move(mean));
    // parameters_.push_back(std::move(sigmaL));
    // parameters_.push_back(std::move(sigmaR));
    parameters_.push_back(std::move(sigma));

    parameters_.push_back(std::move(alphaL));
    parameters_.push_back(std::move(nL));
    parameters_.push_back(std::move(alphaR));
    parameters_.push_back(std::move(nR));

    // 생성된 PDF 포인터 반환
    return std::make_unique<RooCrystalBall>(name.c_str(), ("DoubleSidedCrystalBall_" + name).c_str(),
                                //   *activeMassVar_, *meanPtr, *sigmaLPtr, *sigmaRPtr, *alphaLPtr, *nLPtr, *alphaRPtr, *nRPtr);
                                  *activeMassVar_, *meanPtr, *sigmaPtr, *alphaLPtr, *nLPtr, *alphaRPtr, *nRPtr);
}

std::unique_ptr<RooAbsPdf> MassFitter::MakeDoubleDBCrystalBall(const DoubleDBCrystalBallParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;
    
    auto mean1 = std::make_unique<RooRealVar>(("mean1_" + name).c_str(), "Mean",
                                     params.mean1, params.mean_min, params.mean_max);
    auto sigmaL1 = std::make_unique<RooRealVar>(("sigmaL1_" + name).c_str(), "sigmaL",
                                      params.sigmaL1, params.sigmaL_min, params.sigmaL_max);
    auto sigmaR1 = std::make_unique<RooRealVar>(("sigmaR1_" + name).c_str(), "sigmaR",
                                      params.sigmaR1, params.sigmaR_min, params.sigmaR_max);
    auto alphaL1 = std::make_unique<RooRealVar>(("alphaL1_" + name).c_str(), "AlphaL",
                                       params.alphaL1, params.alphaL_min, params.alphaL_max);
    auto nL1 = std::make_unique<RooRealVar>(("nL1_" + name).c_str(), "NL",
                                   params.nL1, params.nL_min, params.nL_max);
    auto alphaR1 = std::make_unique<RooRealVar>(("alphaR1_" + name).c_str(), "AlphaR",
                                       params.alphaR1, params.alphaR_min, params.alphaR_max);
    auto nR1 = std::make_unique<RooRealVar>(("nR1_" + name).c_str(), "NR",
                                   params.nR1, params.nR_min, params.nR_max);
    auto mean2 = std::make_unique<RooRealVar>(("mean2_" + name).c_str(), "Mean",
                                     params.mean2, params.mean_min, params.mean_max);
    auto sigmaL2 = std::make_unique<RooRealVar>(("sigmaL2_" + name).c_str(), "sigmaL",
                                      params.sigmaL2, params.sigmaL_min, params.sigmaL_max);
    auto sigmaR2 = std::make_unique<RooRealVar>(("sigmaR2_" + name).c_str(), "sigmaR",
                                      params.sigmaR2, params.sigmaR_min, params.sigmaR_max);
    auto alphaL2 = std::make_unique<RooRealVar>(("alphaL2_" + name).c_str(), "AlphaL",
                                       params.alphaL2, params.alphaL_min, params.alphaL_max);
    auto nL2 = std::make_unique<RooRealVar>(("nL2_" + name).c_str(), "NL",
                                   params.nL2, params.nL_min, params.nL_max);
    auto alphaR2 = std::make_unique<RooRealVar>(("alphaR2_" + name).c_str(), "AlphaR",
                                       params.alphaR2, params.alphaR_min, params.alphaR_max);
    auto nR2 = std::make_unique<RooRealVar>(("nR2_" + name).c_str(), "NR",
                                   params.nR2, params.nR_min, params.nR_max);
    auto frac = std::make_unique<RooRealVar>(("frac_" + name).c_str(), "Fraction",
                                  params.fraction, params.fraction_min, params.fraction_max);

    // 파라미터 포인터들을 저장
    RooRealVar* mean1Ptr = mean1.get();
    RooRealVar* sigmaL1Ptr = sigmaL1.get();
    RooRealVar* sigmaR1Ptr = sigmaR1.get();
    RooRealVar* alphaL1Ptr = alphaL1.get();
    RooRealVar* nL1Ptr = nL1.get();
    RooRealVar* alphaR1Ptr = alphaR1.get();
    RooRealVar* nR1Ptr = nR1.get();
    RooRealVar* mean2Ptr = mean2.get();
    RooRealVar* sigmaL2Ptr = sigmaL2.get();
    RooRealVar* sigmaR2Ptr = sigmaR2.get();
    RooRealVar* alphaL2Ptr = alphaL2.get();
    RooRealVar* nL2Ptr = nL2.get();
    RooRealVar* alphaR2Ptr = alphaR2.get();
    RooRealVar* nR2Ptr = nR2.get();
    RooRealVar* fracPtr = frac.get();
    
    parameters_.push_back(std::move(mean1));
    parameters_.push_back(std::move(sigmaL1));
    parameters_.push_back(std::move(sigmaR1));
    parameters_.push_back(std::move(alphaL1));
    parameters_.push_back(std::move(nL1));
    parameters_.push_back(std::move(alphaR1));
    parameters_.push_back(std::move(nR1));
    parameters_.push_back(std::move(mean2));
    parameters_.push_back(std::move(sigmaL2));
    parameters_.push_back(std::move(sigmaR2));
    parameters_.push_back(std::move(alphaL2));
    parameters_.push_back(std::move(nL2));
    parameters_.push_back(std::move(alphaR2));
    parameters_.push_back(std::move(nR2));
    parameters_.push_back(std::move(frac));

    auto CB1 = std::make_unique<RooCrystalBall>("CB1", ("DoubleSidedCrystalBall1_" + name).c_str(),
                       *activeMassVar_, *mean1Ptr, *sigmaL1Ptr, *sigmaR1Ptr, *alphaL1Ptr, *nL1Ptr, *alphaR1Ptr, *nR1Ptr);
    auto CB2 = std::make_unique<RooCrystalBall>("CB2", ("DoubleSidedCrystalBall2_" + name).c_str(),
                                  *activeMassVar_, *mean2Ptr, *sigmaL2Ptr, *sigmaR2Ptr, *alphaL2Ptr, *nL2Ptr, *alphaR2Ptr, *nR2Ptr);
    
    RooCrystalBall* CB1Ptr = CB1.get();
    RooCrystalBall* CB2Ptr = CB2.get();
    
    CB1.release(); // 메모리 관리를 RooAddPdf에 위임
    CB2.release(); // 메모리 관리를 RooAddPdf에 위임
                            
    return std::make_unique<RooAddPdf>(name.c_str(), ("DoubleSidedCrystalBall_" + name).c_str(),
                        RooArgList(*CB1Ptr, *CB2Ptr), *fracPtr);
}

std::unique_ptr<RooAbsPdf> MassFitter::MakeVoigtian(const VoigtianParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;
    
    auto mean = std::make_unique<RooRealVar>(("mean_" + name).c_str(), "Mean",
                                     params.mean, params.mean_min, params.mean_max);
    auto sigma = std::make_unique<RooRealVar>(("sigma_" + name).c_str(), "Sigma",
                                      params.sigma, params.sigma_min, params.sigma_max);
    auto width = std::make_unique<RooRealVar>(("width_" + name).c_str(), "Width",
                                      params.width, params.width_min, params.width_max);
    
    // 파라미터 포인터들을 저장
    RooRealVar* meanPtr = mean.get();
    RooRealVar* sigmaPtr = sigma.get();
    RooRealVar* widthPtr = width.get();
    
    parameters_.push_back(std::move(mean));
    parameters_.push_back(std::move(sigma));
    parameters_.push_back(std::move(width));
    
    return std::make_unique<RooVoigtian>(name.c_str(), ("Voigtian_" + name).c_str(),
                          *activeMassVar_, *meanPtr, *widthPtr, *sigmaPtr);
}

std::unique_ptr<RooAbsPdf> MassFitter::MakeExponential(const ExponentialBkgParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;
    
    auto slope = std::make_unique<RooRealVar>(("slope_" + name).c_str(), "Slope",
                                      params.lambda,
                                      params.lambda_min,
                                      params.lambda_max);
    
    RooRealVar* slopePtr = slope.get();
    parameters_.push_back(std::move(slope));
    
    return std::make_unique<RooExponential>(name.c_str(), ("Exponential_" + name).c_str(),
                             *activeMassVar_, *slopePtr);
}

std::unique_ptr<RooAbsPdf> MassFitter::MakeChebychev(const ChebychevBkgParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;
    
    RooArgList coefList;
    for (size_t i = 0; i < params.coefficients.size(); ++i) {
        auto coef = std::make_unique<RooRealVar>(Form("cheb_coef%lu_%s", i, name.c_str()),
                                         Form("Coefficient %lu", i),
                                         params.coefficients[i],
                                         params.coef_min[i],
                                         params.coef_max[i]);
        coefList.add(*coef.get());
        parameters_.push_back(std::move(coef));
    }
    
    return std::make_unique<RooChebychev>(name.c_str(), ("Chebyshev_" + name).c_str(),
                           *activeMassVar_, coefList);
}
std::unique_ptr<RooAbsPdf> MassFitter::MakePolynomial(const PolynomialBkgParams& params, const std::string& name) {
    if (!activeMassVar_) return nullptr;
    
    RooArgList coefList;
    for (size_t i = 0; i < params.coefficients.size(); ++i) {
        auto coef = std::make_unique<RooRealVar>(Form("poly_coef%lu_%s", i, name.c_str()),
                                         Form("Coefficient %lu", i),
                                         params.coefficients[i],
                                         params.coef_min[i],
                                         params.coef_max[i]);
        coefList.add(*coef.get());
        parameters_.push_back(std::move(coef));
    }
    
    return std::make_unique<RooChebychev>(name.c_str(), ("Polynomial_" + name).c_str(),
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
std::unique_ptr<RooAbsPdf> MassFitter::MakeRooDstD0Bg(const PhenomenologicalParams& params, const std::string& name = "dstd0") {
    if (!activeMassVar_) return nullptr;
    
    auto m0 = std::make_unique<RooRealVar>("m0", "m0", PION_MASS);        // Parameter m0
    auto p0 = std::make_unique<RooRealVar>("p0", "p0", params.p0, params.p0_min, params.p0_max);          // Parameter p0
    auto p1 = std::make_unique<RooRealVar>("p1", "p1", params.p1, params.p1_min, params.p1_max);          // Parameter p1
    auto p2 = std::make_unique<RooRealVar>("p2", "p2", params.p2, params.p2_min, params.p2_max);          // Parameter p2
    
    // 파라미터 포인터들을 저장
    RooRealVar* m0Ptr = m0.get();
    RooRealVar* p0Ptr = p0.get();
    RooRealVar* p1Ptr = p1.get();
    RooRealVar* p2Ptr = p2.get();
    
    parameters_.push_back(std::move(m0));
    parameters_.push_back(std::move(p0));
    parameters_.push_back(std::move(p1));
    parameters_.push_back(std::move(p2));
    
    return std::make_unique<RooDstD0BG>(name.c_str(), ("RooDstD0Bg" + name).c_str(), *activeMassVar_, *m0Ptr, *p0Ptr, *p1Ptr, *p2Ptr);
}


void MassFitter::CombinePDFs() {
    if (!signal_pdf_ || !background_pdf_){
        std::cerr << "Signal and background PDFs must be set before combining." << std::endl;
        return;
    } 
    int a = reduced_data_->sumEntries();
    cout << "total Entries : " << a << endl; 
    
    int nsig_init = static_cast<int>(a * nsig_ratio_);
    int nsig_min = static_cast<int>(a * nsig_min_ratio_);
    int nsig_max = static_cast<int>(a * nsig_max_ratio_);
    
    int nbkg_init = static_cast<int>(a * nbkg_ratio_);
    int nbkg_min = static_cast<int>(a * nbkg_min_ratio_);
    int nbkg_max = static_cast<int>(a * nbkg_max_ratio_);
    
    nsig_ = std::make_unique<RooRealVar>("nsig", "Signal Yield", nsig_init, nsig_min, nsig_max);
    nbkg_ = std::make_unique<RooRealVar>("nbkg", "Background Yield", nbkg_init, nbkg_min, nbkg_max);
    
    cout << "nsig: " << nsig_init << " [" << nsig_min << ", " << nsig_max << "]" << endl;
    cout << "nbkg: " << nbkg_init << " [" << nbkg_min << ", " << nbkg_max << "]" << endl;
    
    total_pdf_ = std::make_unique<RooAddPdf>("total_pdf", "Signal + Background",
                        RooArgList(*signal_pdf_, *background_pdf_),
                        RooArgList(*nsig_, *nbkg_));
}

void MassFitter::CombinePDFs(RooAbsPdf* signal_pdf, RooAbsPdf* background_pdf) {
    if (!signal_pdf || !background_pdf){
        std::cerr << "Signal and background PDFs must be set before combining." << std::endl;
        return;
    } 
    int a = reduced_data_->sumEntries();
    cout << "total Entries : " << a << endl; 
    
    // 비율을 기준으로 초기값과 범위 계산
    int nsig_init = static_cast<int>(a * nsig_ratio_);
    int nsig_min = static_cast<int>(a * nsig_min_ratio_);
    int nsig_max = static_cast<int>(a * nsig_max_ratio_);
    
    int nbkg_init = static_cast<int>(a * nbkg_ratio_);
    int nbkg_min = static_cast<int>(a * nbkg_min_ratio_);
    int nbkg_max = static_cast<int>(a * nbkg_max_ratio_);
    
    nsig_ = std::make_unique<RooRealVar>("nsig", "Signal Yield", nsig_init, nsig_min, nsig_max);
    nbkg_ = std::make_unique<RooRealVar>("nbkg", "Background Yield", nbkg_init, nbkg_min, nbkg_max);
    
    total_pdf_ = std::make_unique<RooAddPdf>("total_pdf", "Signal + Background",
                        RooArgList(*signal_pdf, *background_pdf),
                        RooArgList(*nsig_, *nbkg_));
}
void MassFitter::CombinePDFs(RooAbsPdf* signal_pdf_swap0,RooAbsPdf* signal_pdf_swap1, RooAbsPdf* background_pdf) {
    if (!signal_pdf_swap0 || !signal_pdf_swap1 || !background_pdf){
        std::cerr << "Signal and background PDFs must be set before combining." << std::endl;
        return;
    } 
    int a = reduced_data_->sumEntries();
    cout << "total Entries : " << a << endl; 
    
    // 비율을 기준으로 초기값과 범위 계산
    int nsig_init = static_cast<int>(a * nsig_ratio_);
    int nsig_min = static_cast<int>(a * nsig_min_ratio_);
    int nsig_max = static_cast<int>(a * nsig_max_ratio_);
    
    int nbkg_init = static_cast<int>(a * nbkg_ratio_);
    int nbkg_min = static_cast<int>(a * nbkg_min_ratio_);
    int nbkg_max = static_cast<int>(a * nbkg_max_ratio_);
    
    nsig_ = std::make_unique<RooRealVar>("nsig", "Signal Yield", nsig_init, nsig_min, nsig_max);
    nbkg_ = std::make_unique<RooRealVar>("nbkg", "Background Yield", nbkg_init, nbkg_min, nbkg_max);
    frac_ = std::make_unique<RooRealVar>("frac_sig_DStar", "Fraction", 0.5, 0., 1.2);
    nSigSwap_ = std::make_unique<RooFormulaVar>("nSigSwap", "nsig * frac_sig_DStar", RooArgList(*nsig_, *frac_));
    
    cout << "Signal PDF Swap 0 : " << signal_pdf_swap0->GetName() << endl;
    
    total_pdf_ = std::make_unique<RooAddPdf>("total_pdf", "Signal + Background",
                        RooArgList(*signal_pdf_swap0, *signal_pdf_swap1, *background_pdf),
                        RooArgList(*nsig_, *nSigSwap_, *nbkg_));
}
void MassFitter::CombineMCPDFs() {
    if (!signal_pdf_ || !reduced_data_) {
        std::cerr << "Error: Signal PDF or dataset not set" << std::endl;
        return;
    }
    
    // MC의 경우 extended 없이 그냥 signal PDF만 사용
    total_pdf_ = std::unique_ptr<RooAddPdf>(
        static_cast<RooAddPdf*>(signal_pdf_->clone(string("total_pdf").c_str()))
    );
}
MassFitter::MassFitter(const std::string& name, std::string& massVar, double massMin, double massMax,
                       double nsig_ratio, double nsig_min_ratio, double nsig_max_ratio,
                       double nbkg_ratio, double nbkg_min_ratio, double nbkg_max_ratio) 
    // 비율을 기준으로 초기값과 범위 계산
    : name_(name), 
      massVar_(massVar), 
      useDeltaMass_(false), 
      signal_pdf_(nullptr), 
      background_pdf_(nullptr), 
      total_pdf_(nullptr),
      full_data_(nullptr),
      reduced_data_(nullptr),
      fit_result_(nullptr),
      workspace_(nullptr),
      nsig_(nullptr),
      nbkg_(nullptr),
      nsig_ratio_(nsig_ratio), nsig_min_ratio_(nsig_min_ratio), nsig_max_ratio_(nsig_max_ratio),
      nbkg_ratio_(nbkg_ratio), nbkg_min_ratio_(nbkg_min_ratio), nbkg_max_ratio_(nbkg_max_ratio) {
    
    // 질량 변수 생성 (스마트 포인터 사용)
    mass_ = std::make_unique<RooRealVar>(massVar_.c_str(), "mass", massMin, massMax, "GeV/c^{2}");
    
    // 활성 질량 변수 초기화
    activeMassVar_ = mass_.get();
    
    // 워크스페이스 초기화 (스마트 포인터 사용)
    workspace_ = std::make_unique<RooWorkspace>(("ws_" + name_).c_str());
}

MassFitter::~MassFitter() {
    // 스마트 포인터는 자동으로 해제됨
    // raw 포인터들만 명시적으로 정리
    Clear();
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
        massDaughter1_ = std::make_unique<RooRealVar>("massDaughter1", "Daughter Mass", daughterMassMin, daughterMassMax, "GeV/c^{2}");
        massPion_ = std::make_unique<RooRealVar>(("deltaMass_" + name_).c_str(), "Delta Mass", 0.135, 0.16, "GeV/c^{2}");
        activeMassVar_ = massPion_.get(); // Delta mass를 활성 변수로 설정
    } else {
        // 표준 질량 모드
        activeMassVar_ = mass_.get();
    }
}

// Signal PDF 설정 메서드 - 타입 문자열 제거
void MassFitter::SetSignalPDF(const GaussianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeGaussian(params, pdfName); // 스마트 포인터 이동
}

void MassFitter::SetSignalPDF(const DoubleGaussianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeDoubleGaussian(params, pdfName); // 스마트 포인터 이동
}

void MassFitter::SetSignalPDF(const CrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeCrystalBall(params, pdfName); // 스마트 포인터 이동
}

void MassFitter::SetSignalPDF(const DBCrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeDBCrystalBall(params, pdfName); // 스마트 포인터 이동
}
void MassFitter::SetSignalPDF(const DoubleDBCrystalBallParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeDoubleDBCrystalBall(params, pdfName); // 스마트 포인터 이동
}

void MassFitter::SetSignalPDF(const VoigtianParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "sig_" + name_ : name;
    signal_pdf_ = MakeVoigtian(params, pdfName); // 스마트 포인터 이동
}

// Background PDF 설정 메서드 - 스마트 포인터 사용하도록 수정
void MassFitter::SetBackgroundPDF(const ExponentialBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakeExponential(params, pdfName); // 스마트 포인터 이동
}

void MassFitter::SetBackgroundPDF(const ChebychevBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakeChebychev(params, pdfName); // 스마트 포인터 이동
}

void MassFitter::SetBackgroundPDF(const PolynomialBkgParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakePolynomial(params, pdfName); // 스마트 포인터 이동
}
void MassFitter::SetBackgroundPDF(const PhenomenologicalParams& params, const std::string& name) {
    std::string pdfName = name.empty() ? "bkg_" + name_ : name;
    background_pdf_ = MakeRooDstD0Bg(params, pdfName); // 스마트 포인터 이동
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
    RooLinkedList fitOpts;
    fitOpts.Add(new RooCmdArg(RooFit::NumCPU(24)));
    #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    fitOpts.Add(new RooCmdArg(RooFit::EvalBackend(useCUDA ? "cuda" : "CPU")));
    #endif
    fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Minos(useMinos)));
    fitOpts.Add(new RooCmdArg(RooFit::Optimize(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Hesse(useHesse)));
    fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(verbose ? 1 : -1)));
    fitOpts.Add(new RooCmdArg(RooFit::Range("analysis")));

    
    RooFitResult* initialResult = total_pdf_->fitTo(*reduced_data_, fitOpts);
    initialResult->Print("v");

    int ntry = 0;
    while(((initialResult->statusCodeHistory(0) != 0) || (initialResult->statusCodeHistory(1) != 0)) && (ntry)<3){
    delete initialResult; // 이전 결과 삭제
    RooLinkedList fitOptstemp;
    fitOptstemp.Add(new RooCmdArg(RooFit::NumCPU(24)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Minimizer("Minuit","Minimizer")));
    #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    fitOptstemp.Add(new RooCmdArg(RooFit::EvalBackend(useCUDA ? "cuda" : "CPU")));
    #endif
    fitOptstemp.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Save(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Minos(useMinos)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Optimize(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Hesse(useHesse)));
    fitOptstemp.Add(new RooCmdArg(RooFit::PrintLevel(verbose ? 1 : -1)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Range("analysis")));
    fitOptstemp.Add(new RooCmdArg(RooFit::Strategy(2-ntry))); 
        std::cout << "Initial fit failed with status " << initialResult->status() 
                  << ". Retrying..." << std::endl;
        initialResult = total_pdf_->fitTo(*reduced_data_,fitOptstemp);
    cout << " #### Fit attempt " << ntry + 1 << " completed with status: " << initialResult->status() << "####" << std::endl;
        initialResult->Print("v");
        ntry++;
    }
   
    // bool touchlimit = false;
    // auto finalPars = initialResult->floatParsFinal();
    // for (int idx = 0; idx < finalPars.getSize(); ++idx) {
    //     auto var = dynamic_cast<RooRealVar*>(&finalPars[idx]);
    //     if (var) {
    //         if (var->getVal() >= var->getMax() - 1e-6 || var->getVal() <= var->getMin() + 1e-6) {
    //             touchlimit = true;
    //             break;
    //         }
    //     }
    // }

    // bool needFinalFitRefit = (initialResult->statusCodeHistory(0) != 0) || (initialResult->statusCodeHistory(1) != 0) || touchlimit;
    
    // if (needFinalFitRefit && !useMinos) { // Minos가 켜져 있으면 statusCode가 0이 아닐 수 있음
    //     std::cout << "Initial fit was unstable. Attempting to fix and refit..." << std::endl;
    //     fit_result_ = fixFit(initialResult); // fixFit은 unique_ptr을 반환
    // } else {
    //     fit_result_.reset(initialResult); // 문제가 없으면 소유권만 이전
    // }
    // // 결과 출력
    // if (verbose) {
    //     fit_result_->Print("v");
    // } else {
    //     std::cout << "Fit completed. Signal yield: " << GetSignalYield() 
    //               << " ± " << GetSignalYieldError() << std::endl;
    //     std::cout << "Chi2/NDF = " << GetReducedChiSquare() << std::endl;
    // }
    initialResult->Print("v");
    fit_result_.reset(initialResult); // 문제가 없으면 소유권만 이전
    
    // 워크스페이스에 결과 저장
    workspace_->import(*total_pdf_);
    
    return fit_result_.get();
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
            if(paramName.find("sigma")!=std::string::npos) continue;
            
            // pdfParam->setVal(fitParam->getVal());      
            // pdfParam->setMin(fitParam->getMin());
            // pdfParam->setMax(fitParam->getMax());
            // cout <<fitParam->GetName() << ": " <<  fitParam->getMin() << " " << fitParam->getMax() << " " << fitParam->getVal() << std::endl;
            // pdfParam->setError(fitParam->getError());
            
            // constraintParameters 목록에 있는 파라미터만 constant로 설정
            bool shouldConstrain = false;
            for (const auto& constParam : constraintParameters) {
                if (paramName.find("nsig")!= std::string::npos) continue; // nsig 파라미터는 제외
                if (paramName.find("nbkg")!= std::string::npos) continue;
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


    RooLinkedList fitOpts;
    fitOpts.Add(new RooCmdArg(RooFit::NumCPU(24)));
    fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit","Minimizer")));
    #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    fitOpts.Add(new RooCmdArg(RooFit::EvalBackend(useCUDA ? "cuda" : "CPU")));
    #endif
    fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Minos(useMinos)));
    fitOpts.Add(new RooCmdArg(RooFit::Optimize(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Hesse(useHesse)));
    fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(verbose ? 1 : -1)));
    fitOpts.Add(new RooCmdArg(RooFit::Range("analysis")));
    // fitOpts.Add(new RooCmdArg(RooFit::Strategy(2))); // 전략 2로 설정 (더 강력한 피팅 전략)

    
    // [수정 시작] Fit과 동일한 자동 복구 로직 적용
    // 1. 초기 피팅 수행
    RooFitResult* initialResult = total_pdf_->fitTo(*reduced_data_,fitOpts);
    initialResult->Print("v");

        
    int ntry = 0;
    while(((initialResult->statusCodeHistory(0) != 0) || (initialResult->statusCodeHistory(1) != 0)) && (ntry)<3){
    delete initialResult; // 이전 결과 삭제
    RooLinkedList fitOptstemp;
    fitOptstemp.Add(new RooCmdArg(RooFit::NumCPU(24)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Minimizer("Minuit","Minimizer")));
    #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    fitOptstemp.Add(new RooCmdArg(RooFit::EvalBackend(useCUDA ? "cuda" : "CPU")));
    #endif
    fitOptstemp.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Save(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Minos(useMinos)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Optimize(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Hesse(useHesse)));
    fitOptstemp.Add(new RooCmdArg(RooFit::PrintLevel(verbose ? 1 : -1)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Range("analysis")));
    fitOptstemp.Add(new RooCmdArg(RooFit::Strategy(2-ntry))); 
        std::cout << "Initial fit failed with status " << initialResult->status() 
                  << ". Retrying..." << std::endl;
        initialResult = total_pdf_->fitTo(*reduced_data_,fitOptstemp);
    cout << " #### Fit attempt " << ntry + 1 << " completed with status: " << initialResult->status() << "####" << std::endl;
        initialResult->Print("v");
        ntry++;
    }
    // bool touchlimit = false;
    // auto finalPars = initialResult->floatParsFinal();
    //   for( int idx=0;idx<finalPars.getSize();idx++ ){
    //             auto var = (RooRealVar*) &finalPars[idx];
    //             double _val_ = var->getVal();
    //             double _errup_ = var->getErrorHi();
    //             double _errdo_ = var->getErrorLo();
    //             double _errsym_ = var->getError();
    //             double _limup_ = var->getMax();
    //             double _limdown_ = var->getMin();
    //             if(_val_==_limup_ || _val_==_limdown_) touchlimit=1;
    //             if( _val_ + 3* _errup_ > _limup_ || (_val_ >0 && 1.2*_val_ > _limup_ )|| (_val_ <0 && 0.8*_val_ > _limup_ )) touchlimit=1;
    //             if(_val_ + 3*_errdo_ < _limdown_ || (_val_ >0 && 0.8*_val_ < _limdown_ )|| (_val_ <0 && 1.2*_val_ < _limdown_ )) touchlimit=1;}


    // bool needFinalFitRefit = (initialResult->statusCodeHistory(0) != 0) || (initialResult->statusCodeHistory(1) != 0) || touchlimit;

    // // // 3. 필요시 fixFit 호출
    // if (needFinalFitRefit && !useMinos) {
    //     std::cout << "Initial constrained fit was unstable. Attempting to fix and refit..." << std::endl;
    //     fit_result_ = fixFit(initialResult); // fixFit은 unique_ptr을 반환
    // } else {
    //     fit_result_.reset(initialResult); // 문제가 없으면 소유권만 이전
    // }
    // // 결과 출력
    // if (verbose) {
    //     fit_result_->Print("v");
    // } else {
    //     std::cout << "Fit completed. Signal yield: " << GetSignalYield() 
    //               << " ± " << GetSignalYieldError() << std::endl;
    //     std::cout << "Chi2/NDF = " << GetReducedChiSquare() << std::endl;
    // }
    
    
    initialResult->Print("v");
    fit_result_.reset(initialResult); // 문제가 없으면 소유권만 이전
    // 워크스페이스에 결과 저장
    workspace_->import(*total_pdf_);
    delete fitResult;
    file->Close();
    
    return fit_result_.get();
}

// RooFitResult* MassFitter::ConstraintBinnedFit(std::string fileMCPath, std::string fileMCFile, std::string dataHistoName, std::string dataHistoFilePath, vector<std::string> constraintParameters, bool useMinos, bool useHesse, bool verbose, bool useCUDA, double rangeMin, double rangeMax) {
//     std::string fullMCPath = fileMCPath;
//     if (!fullMCPath.empty() && fullMCPath.back() != '/') fullMCPath += '/';
//     fullMCPath += fileMCFile;

//     TFile *mcFile = TFile::Open((fullMCPath).c_str());
//     if (!mcFile || mcFile->IsZombie()) {
//         std::cerr << "Error: Failed to open MC fit result file: " << fullMCPath << std::endl;
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
    // 스마트 포인터들은 자동으로 해제됨
    signal_pdf_.reset();
    background_pdf_.reset();
    total_pdf_.reset();
    fit_result_.reset();
    swap0_pdf_.reset();
    swap1_pdf_.reset();
    nsig_.reset();
    nbkg_.reset();
    frac_.reset();
    nSigSwap_.reset();
    parameters_.clear();
    
    // 데이터는 외부에서 관리되므로 nullptr로만 설정
    full_data_ = nullptr;
    reduced_data_ = nullptr;
    
    // 워크스페이스 재생성
    workspace_ = std::make_unique<RooWorkspace>(("ws_" + name_).c_str());
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
    
    // Apply cuts based on inclusive flag
    if (inclusive) {
        ApplyCut(opt.cutExpr);
    } else {
        ApplyCut(opt.cutExpr + "&&" + pTbin + "&&" + etabin);
    }

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
void MassFitter::PerformDCAMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams) {
    // full_data_ = MCdataset;
    SetData(MCdataset);

    // Create and set PDFs
    SetSignalPDF(sigMCParams,"");
    // SetBackgroundPDF(bkgParams);
    CombineMCPDFs();
    // Perform fit and save results
    if(opt.doFit)FitMC(opt.useMinos, opt.useHesse,opt.verbose,opt.useCUDA, opt.massMin,opt.massMax);
    SaveMCResults(opt.outputMCDir,opt.outputMCFile, opt.saveWorkspace);
    Clear();
    // workspace_ = std::make_unique<RooWorkspace>(("ws_" + name_).c_str());


}
template <typename SigMCPar>
void MassFitter::PerformMCFit(FitOpt opt, RooDataSet* MCdataset, bool inclusive, const std::string pTbin, const std::string etabin, SigMCPar sigMCParams) {
    full_data_ = MCdataset;

    if (inclusive) {
        ApplyCut(opt.cutMCExpr);
    } else {
        ApplyCut(opt.cutMCExpr + "&&" + pTbin + "&&" + etabin);
    }
    
    SetSignalPDF(sigMCParams,"");
    CombineMCPDFs(); // nsig, nbkg 없이 signal PDF만 사용
    
    // MC 전용 피팅 사용 (extended 없음)
    if(opt.doFit) FitMC(opt.useMinos, opt.useHesse, opt.verbose, opt.useCUDA, opt.massMin, opt.massMax);
    
    SaveMCResults(opt.outputMCDir, opt.outputMCFile, opt.saveWorkspace);
    Clear();
}
RooFitResult* MassFitter::FitMC(bool useMinos, bool useHesse, bool verbose, bool useCUDA, double rangeMin, double rangeMax) {
    if (!total_pdf_) {
        std::cerr << "Error: Signal PDF not set" << std::endl;
        return nullptr;
    }
    
    // MC는 extended 없이 피팅
    mass_->setRange("analysis", rangeMin, rangeMax);


     RooLinkedList fitOpts;
    fitOpts.Add(new RooCmdArg(RooFit::NumCPU(24)));
    fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit","Minimizer")));
    #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    fitOpts.Add(new RooCmdArg(RooFit::EvalBackend(useCUDA ? "cuda" : "CPU")));
    #endif
    fitOpts.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Save(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Minos(useMinos)));
    fitOpts.Add(new RooCmdArg(RooFit::Optimize(true)));
    fitOpts.Add(new RooCmdArg(RooFit::Hesse(useHesse)));
    fitOpts.Add(new RooCmdArg(RooFit::PrintLevel(verbose ? 1 : -1)));
    fitOpts.Add(new RooCmdArg(RooFit::Range("analysis")));
    // fitOpts.Add(new RooCmdArg(RooFit::Strategy(2))); // 전략 2로 설정 (더 강력한 피팅 전략)

    
    // [수정 시작] Fit과 동일한 자동 복구 로직 적용
    // 1. 초기 피팅 수행
    RooFitResult* initialResult = total_pdf_->fitTo(*reduced_data_,fitOpts);
    initialResult->Print("v");

    int ntry = 0;
    while(((initialResult->statusCodeHistory(0) != 0) || (initialResult->statusCodeHistory(1) != 0)) && (ntry)<3){
    delete initialResult; // 이전 결과 삭제
    RooLinkedList fitOptstemp;
    fitOptstemp.Add(new RooCmdArg(RooFit::NumCPU(24)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Minimizer("Minuit","Minimizer")));
    #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    fitOptstemp.Add(new RooCmdArg(RooFit::EvalBackend(useCUDA ? "cuda" : "CPU")));
    #endif
    fitOptstemp.Add(new RooCmdArg(RooFit::Extended(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Save(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Minos(useMinos)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Optimize(true)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Hesse(useHesse)));
    fitOptstemp.Add(new RooCmdArg(RooFit::PrintLevel(verbose ? 1 : -1)));
    fitOptstemp.Add(new RooCmdArg(RooFit::Range("analysis")));
    fitOptstemp.Add(new RooCmdArg(RooFit::Strategy(2-ntry))); 
        std::cout << "Initial fit failed with status " << initialResult->status() 
                  << ". Retrying..." << std::endl;
        initialResult->Print("v");
        initialResult = total_pdf_->fitTo(*reduced_data_,fitOptstemp);
    cout << " #### Fit attempt " << ntry + 1 << " completed with status: " << initialResult->status() << "####" << std::endl;
        ntry++;
    }
        
    // int ntry = -1;
    // while(initialResult->status() !=0 && (ntry+1)<3){
    //     std::cout << "Initial fit failed with status " << initialResult->status() 
    //               << ". Retrying..." << std::endl;
    //     initialResult = total_pdf_->fitTo(*reduced_data_,fitOpts);
    //     ntry++;
    // }
    // bool touchlimit = false;
    // auto finalPars = initialResult->floatParsFinal();
    //   for( int idx=0;idx<finalPars.getSize();idx++ ){
    //             auto var = (RooRealVar*) &finalPars[idx];
    //             double _val_ = var->getVal();
    //             double _errup_ = var->getErrorHi();
    //             double _errdo_ = var->getErrorLo();
    //             double _errsym_ = var->getError();
    //             double _limup_ = var->getMax();
    //             double _limdown_ = var->getMin();
    //             if(_val_==_limup_ || _val_==_limdown_) touchlimit=1;
    //             if( _val_ + 3* _errup_ > _limup_ || (_val_ >0 && 1.2*_val_ > _limup_ )|| (_val_ <0 && 0.8*_val_ > _limup_ )) touchlimit=1;
    //             if(_val_ + 3*_errdo_ < _limdown_ || (_val_ >0 && 0.8*_val_ < _limdown_ )|| (_val_ <0 && 1.2*_val_ < _limdown_ )) touchlimit=1;}


    // bool needFinalFitRefit = (initialResult->statusCodeHistory(0) != 0) || (initialResult->statusCodeHistory(1) != 0) || touchlimit;

    // // // 3. 필요시 fixFit 호출
    // if (needFinalFitRefit && !useMinos) {
    //     std::cout << "Initial constrained fit was unstable. Attempting to fix and refit..." << std::endl;
    //     fit_result_ = fixFit(initialResult); // fixFit은 unique_ptr을 반환
    // } else {
    //     fit_result_.reset(initialResult); // 문제가 없으면 소유권만 이전
    // }
    // // 결과 출력
    // if (verbose) {
    //     fit_result_->Print("v");
    // } else {
    //     // std::cout << "Fit completed. Signal yield: " << GetSignalYield() 
    //             //   << " ± " << GetSignalYieldError() << std::endl;
    //     std::cout << "Chi2/NDF = " << GetReducedChiSquare() << std::endl;
    // }
    
    // RooFitResult* initialResult = total_pdf_->fitTo(*reduced_data_, 
    //                                     RooFit::NumCPU(24),
    // #if ROOT_VERSION_CODE >= ROOT_VERSION(6, 26, 00)
    //                                     RooFit::EvalBackend(useCUDA ? "cuda" : "CPU"),
    // #endif
    //                                     RooFit::Save(true),
    //                                     RooFit::Minos(useMinos),
    //                                     RooFit::Optimize(true),
    //                                     RooFit::Hesse(useHesse),
    //                                     RooFit::PrintLevel(verbose ? 1 : -1),
    //                                     RooFit::Range("analysis"));
    //                                     // Extended 옵션 제거
   
    // // // 나머지 처리는 일반 Fit과 동일
    // // bool touchlimit = false;
    // // auto finalPars = initialResult->floatParsFinal();
    // // for (int idx = 0; idx < finalPars.getSize(); ++idx) {
    // //     auto var = dynamic_cast<RooRealVar*>(&finalPars[idx]);
    // //     if (var) {
    // //         if (var->getVal() >= var->getMax() - 1e-6 || var->getVal() <= var->getMin() + 1e-6) {
    // //             touchlimit = true;
    // //             break;
    // //         }
    // //     }
    // // }

    // // bool needFinalFitRefit = (initialResult->statusCodeHistory(0) != 0) || (initialResult->statusCodeHistory(1) != 0) || touchlimit;
    
    // // if (needFinalFitRefit && !useMinos) {
    // //     std::cout << "Initial MC fit was unstable. Attempting to fix and refit..." << std::endl;
    // //     fit_result_ = fixFit(initialResult);
    // // } else {
    initialResult->Print("v");
    fit_result_.reset(initialResult);
    // // }
    
    //     fit_result_.reset(initialResult);
    // // 결과 출력
    // if (verbose) {
    //     fit_result_->Print("v");
    // } else {
    //     std::cout << "MC Fit completed." << std::endl;
    // }
    
    // 워크스페이스에 결과 저장
    workspace_->import(*total_pdf_);
    mc_fit_result_ = (RooFitResult*)fit_result_->Clone("fitResult");
    mcworkspace_ = (RooWorkspace*)workspace_->Clone();
    
    return fit_result_.get();
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
    workspace_ = std::make_unique<RooWorkspace>(("ws_" + name_).c_str());

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

    workspace_ = std::make_unique<RooWorkspace>(("ws_" + name_).c_str());

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
    CombinePDFs(swap0_pdf_.get(), swap1_pdf_.get(), background_pdf_.get());
    
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
    // delete swap0_pdf_;
    // delete swap1_pdf_;

    
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
    CombinePDFs(signal_pdf_.get(), background_pdf_.get());
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

template <typename SigPar, typename BkgPar>
void MassFitter::PerformDCAConstraintFit(FitOpt opt,RooDataSet* data, RooDataSet* MCdata, bool inclusive, const std::string pTbin, const std::string etabin, SigPar sigParams, BkgPar bkgParams) {
    PerformDCAMCFit(opt,MCdata,inclusive,pTbin,etabin,sigParams);
    SetData(data);
    // full_data_ = data;

    // frac_= new RooRealVar("frac_sig_DStar", "Fraction of Swap0", 0.5, 0.0, 1.0);
    // signal_pdf_ = new RooAddPdf("signal_pdf", "Signal PDF", RooArgList(*swap0_pdf_, *swap1_pdf_), RooArgList(*frac_));

    SetSignalPDF(sigParams,"");
    SetBackgroundPDF(bkgParams,"");
    CombinePDFs(signal_pdf_.get(), background_pdf_.get());
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
    // Clear();

    
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
 int a =0;
    if (reduced_data_) {
        cout << "writing entry : " << reduced_data_->sumEntries() << endl;
        reduced_data_->Write("reducedData");
    }
    // if (total_pdf_) {
    //     cout << a++ << endl;
    //         total_pdf_->Write("total_pdf");
    //         cout << "total pdf is written" << endl;
    // }
    
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
