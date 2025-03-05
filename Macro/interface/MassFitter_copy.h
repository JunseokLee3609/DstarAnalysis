#ifndef MASS_FITTER_H
#define MASS_FITTER_H

// Required ROOT includes
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"
#include "RooFitResult.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooPolynomial.h"
#include "RooChebychev.h"
#include "RooExponential.h"
#include "RooCBShape.h"
#include "RooVoigtian.h"
#include "RooBreitWigner.h"
#include "RooBernstein.h"
#include "RooAddPdf.h"
#include "RooAbsPdf.h"
#include "RooPlot.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"
#include "../Tools/Style/CMS_lumi.C"

#include <string>
#include <memory>
#include <vector>
#define DEBUG true
#define PION_MASS 0.13957018 


class MassFitter {
public:
    std::string name;
    std::string massVar_;
    bool useMVA_;
    // Add new member variable for massPion formula
    RooRealVar* mass_;                 // Parent mass variable
    RooRealVar* massDaughter1_;        // Daughter mass variable
    // RooFormulaVar* massPion_;          // Calculated pion mass
    RooRealVar* massPion_;          // Calculated pion mass
    RooRealVar* activeMassVar_;        // Pointer to active mass variable (either mass_ or massPion_)
    // Parameter structures for different PDFs
    struct GaussianParams {
        double mean;
        double sigma;
        double mean_min, mean_max;
        double sigma_min, sigma_max;
    };

    struct DoubleGaussianParams {
        double mean1, sigma1;
        double mean2, sigma2;
        double fraction;
        double mean1_min, mean1_max, sigma1_min, sigma1_max;
        double mean2_min, mean2_max, sigma2_min, sigma2_max;
        double fraction_min, fraction_max;
    };

    struct CrystalBallParams {
        double mean, sigma, alpha, n;
        double mean_min, mean_max;
        double sigma_min, sigma_max;
        double alpha_min, alpha_max;
        double n_min, n_max;
    };
    struct DBCrystalBallParams {
        double mean, sigmaL,sigmaR, alphaL, nL, alphaR, nR;
        double mean_min, mean_max;
        double sigmaL_min, sigmaL_max;
        double sigmaR_min, sigmaR_max;
        double alphaL_min, alphaL_max;
        double nL_min, nL_max;
        double alphaR_min, alphaR_max;
        double nR_min, nR_max;
    };

    struct VoigtianParams {
        double mean, sigma, width;
        double mean_min, mean_max;
        double sigma_min, sigma_max;
        double width_min, width_max;
    };

    struct BreitWignerParams {
        double mean, width;
        double mean_min, mean_max;
        double width_min, width_max;
    };
    struct PhenomenologicalParams {
        // double m0, m0_min, m0_max;
        double p0, p0_min, p0_max;
        double p1, p1_min, p1_max;
        double p2, p2_min, p2_max;
    };

    struct BackgroundParams {
        std::vector<double> coefficients;
        std::vector<double> coef_min;
        std::vector<double> coef_max;
    };

    // Modified constructor
    MassFitter(std::string name_) :
        mass_(nullptr),
        massDaughter1_(nullptr),
        massPion_(nullptr),
        activeMassVar_(nullptr),
        pT_(nullptr),
        eta_(nullptr),
        phi_(nullptr),
        y_(nullptr),
        mva_(nullptr),
        data_(nullptr),
        reduced_data_(nullptr),
        signal_pdf_(nullptr),
        background_pdf_(nullptr),
        total_pdf_(nullptr),
        fit_result_(nullptr)
    {
        name = name_;
    }
    void setMassVariable(const std::string& massVar) {
        massVar_ = massVar;
        if (massVar == "mass") {
            if(activeMassVar_!=nullptr){delete activeMassVar_;}
            mass_ = new RooRealVar("mass", "D^{*} Invariant Mass", 1.90, 2.1, "GeV/c^{2}");
            activeMassVar_ = mass_;
        } 
        else if (massVar == "massPion") {
            if(activeMassVar_!=nullptr){delete activeMassVar_;}
            // mass_ = new RooRealVar("mass", "Parent Mass", 1.90, 2.1, "GeV/c^{2}");
            // massDaughter1_ = new RooRealVar("massDaughter1", "Daughter Mass", 0.0, 2.0, "GeV/c^{2}");
            massPion_ = new RooRealVar("massPion", "#Delta M(m_{K#pi#pi}-m_{K#pi})", 0.140, 0.155, "GeV/c^{2}");
            activeMassVar_ = dynamic_cast<RooRealVar*>(massPion_);
        }
        else if (massVar == "massD0") {
            if(activeMassVar_!=nullptr){delete activeMassVar_;}
            massDaughter1_ = new RooRealVar("massDaugther1", "Daughter Mass", 1.8, 1.9, "GeV/c^{2}");
            activeMassVar_ = dynamic_cast<RooRealVar*>(massDaughter1_);
        }
        else {
            throw std::invalid_argument("Invalid mass variable");
        }
    }
 

    // Modified destructor
    virtual ~MassFitter() {
        // Cleanup();
    }
    void refit(RooFitResult* result) {
        int Miuit = result->statusCodeHistory(0);
        int HESSE = result->statusCodeHistory(1);
        int Minos = result->statusCodeHistory(2);
        if (Miuit != 0 || HESSE != 0 || Minos != 0) {
            std::cout << "Fit failed, try to refit " <<"Minimizer : " << Miuit << "HESSE : " << HESSE << "Minos : " << Minos << std::endl;
        }

        int ntry = -1;
        RooFitResult* resFail = result;
        while (resFail->status() != 0 && ++ntry < 3) {
            TIterator *iter = resFail->floatParsFinal().createIterator();
            RooRealVar* param = nullptr;
            while ((param = (RooRealVar*)iter->Next())) {
                RooRealVar* paramInPdf = (RooRealVar*)total_pdf_->getVariables()->find(param->GetName());
                if (paramInPdf) {
                    paramInPdf->setVal(param->getVal());
                    paramInPdf->setError(param->getError());
                }
            }
            delete iter;
            resFail = total_pdf_->fitTo(*reduced_data_,
                                        RooFit::Minimizer("Minuit", "minimize"),
                                        RooFit::Optimize(1),
                                        RooFit::Save(),
                                        RooFit::Range("analysis"),
                                        // RooFit::Minos(true),
                                        RooFit::Strategy(2 - ntry));
        }
        std::cout << "Fit result flag after iteration " <<"Minimizer : " << Miuit << "HESSE : " << HESSE << "Minos : " << Minos << std::endl;

    }

    // Data handling methods
    bool LoadData(const std::string& filename, const std::string& wsName, const std::string& dataName, bool useMVA=true) {
        useMVA_ = useMVA;
        TFile* file = TFile::Open(filename.c_str());
        if (!file || file->IsZombie()) {
            return false;
        }
        RooWorkspace* ws = (RooWorkspace*)file->Get(wsName.c_str());
        if (!ws) {
            file->Close();
            return false;
        }
        RooDataSet* newData = (RooDataSet*)ws->data(dataName.c_str());
        if (!newData) {
            file->Close();
            return false;
        }
        data_ = (RooDataSet*)newData->Clone();
        //mass_ = (RooRealVar*)data_->get()->find("mass");
        pT_ = (RooRealVar*)data_->get()->find("pT");
        eta_ = (RooRealVar*)data_->get()->find("eta");
        phi_ = (RooRealVar*)data_->get()->find("phi");
        // y_ = (RooRealVar*)data_->get()->find("y");
        delete newData;
        
        if(useMVA_){
            mva_ = (RooRealVar*)data_->get()->find("mva");
        }
        
        file->Close();
        return true;
    }

    bool ReduceData(const std::string& cut) {
        if (!data_) return false;
        
        delete reduced_data_;  // Clean up any existing reduced dataset
        reduced_data_ = (RooDataSet*)data_->reduce(cut.c_str());
        
        return (reduced_data_ && reduced_data_->numEntries() > 0);
    }



    void MakeGaussian(const GaussianParams& params, const std::string& name = "gauss") {
        // if (!activeMassVar_) return nullptr;
        
        RooRealVar* mean = new RooRealVar(("mean_" + name).c_str(), "Mean",
                                         params.mean, params.mean_min, params.mean_max);
        RooRealVar* sigma = new RooRealVar(("sigma_" + name).c_str(), "Sigma",
                                          params.sigma, params.sigma_min, params.sigma_max);
        
        signal_pdf_= new RooGaussian(name.c_str(), ("Gaussian_" + name).c_str(), 
                              *activeMassVar_, *mean, *sigma);
    }

    void MakeDoubleGaussian(const DoubleGaussianParams& params, const std::string& name = "doublegauss") {
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

    void MakeCrystalBall(const CrystalBallParams& params, const std::string& name = "cb") {
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

    void MakeDBCrystalBall(const DBCrystalBallParams& params, const std::string& name = "doublecb") {
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

    void MakeVoigtian(const VoigtianParams& params, const std::string& name = "voigt") {
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

    void MakeExponential(const BackgroundParams& params, const std::string& name = "exp") {
        // if (!activeMassVar_ || params.coefficients.empty()) return nullptr;
        
        RooRealVar* slope = new RooRealVar(("slope_" + name).c_str(), "Slope",
                                          params.coefficients[0],
                                          params.coef_min[0],
                                          params.coef_max[0]);
        
        background_pdf_= new RooExponential(name.c_str(), ("Exponential_" + name).c_str(),
                                 *activeMassVar_, *slope);
    }

    void MakeChebychev(const BackgroundParams& params, const std::string& name = "cheb") {
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
    void MakePolynomial(const BackgroundParams& params, const std::string& name = "poly") {
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
    void MakePhenomenological(const PhenomenologicalParams& params, const std::string& name = "phenom") {
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

    void CombinePDFs() {
        // if (!signal_pdf_ || !background_pdf_) return nullptr;
        int a =reduced_data_->sumEntries();
        
        RooRealVar* nsig = new RooRealVar("nsig", "Signal Yield", 1000, 0., 2.*a);
        RooRealVar* nbkg = new RooRealVar("nbkg", "Background Yield", 1e4, a/100., 2.*a);
        
        total_pdf_= new RooAddPdf("total_pdf", "Signal + Background",
                            RooArgList(*signal_pdf_, *background_pdf_),
                            RooArgList(*nsig, *nbkg));
    }

    void PerformFit(double pTMin, double pTMax,bool constraint=false) {
        if (!total_pdf_){
	cout << "no total pdf" << endl;
	 return false;}
        RooDataSet* dataToFit = reduced_data_;
        if (!dataToFit){
	cout << "no data to fit" << endl;
	 return false;
	}
    #ifdef DEBUG
    cout << "Performing fit" << endl;
    #endif
    
    if(constraint){
    //     delete total_pdf_;
        // RooArgSet* signalParams = signal_pdf_->getVariables();
        std::string filename = Form("roots/test_MC_%s_%s_pT%dto%d.root",name.c_str(),massVar_.c_str(),int(pTMin),int(pTMax));
        TFile file(filename.c_str(), "READ");
        total_pdf_ = dynamic_cast<RooAddPdf*>(file.Get("total_pdf"));

    RooArgSet* params = total_pdf_->getParameters(RooDataSet());
    TIterator* iter = params->createIterator();
    RooRealVar* param;
    while ((param = (RooRealVar*)iter->Next())) {
        if(strcmp(param->GetName(),"mean_dstar_sig")==0) continue;
        // if(strcmp(param->GetName(),"alphaR_dstar_sig")==0) continue;
        // if(strcmp(param->GetName(),"alphaL_dstar_sig")==0) continue;
        if(strcmp(param->GetName(),"sigmaR_dstar_sig")==0) continue;
        if(strcmp(param->GetName(),"sigmaL_dstar_sig")==0) continue;
        // if(strcmp(param->GetName(),"nR_dstar_sig")==0) continue;
        // if(strcmp(param->GetName(),"nL_dstar_sig")==0) continue;
        if(strcmp(param->GetName(),"nbkg")==0) continue;
        if(strcmp(param->GetName(),"nsig")==0) continue;
        if(strcmp(param->GetName(),"p0")==0) continue;
        if(strcmp(param->GetName(),"p1")==0) continue;
        if(strcmp(param->GetName(),"p2")==0) continue;
        cout << param->GetName() << " is set to be a constant" << endl;
        param->setConstant(true);
    }
    delete iter;
        fit_result_ = total_pdf_->fitTo(*dataToFit,RooFit::Minimizer("Minuit", "minimize"),RooFit::Optimize(1), RooFit::Extended(),RooFit::NumCPU(40), RooFit::Range("analysis"), RooFit::Save(), RooFit::PrintLevel(1));

    }
    else{

        fit_result_ = total_pdf_->fitTo(*dataToFit,RooFit::Minimizer("Minuit", "minimize"),RooFit::Optimize(1), RooFit::Extended(),RooFit::NumCPU(40), RooFit::Range("analysis"), RooFit::Save(), RooFit::PrintLevel(1));
        // refit(fit_result_);
    }
    cout <<"end" << endl;
    }
bool PlotResult(double pTMin, double pTMax, double etaMin=-1, double etaMax=1) {
    std::string filename = Form("roots/test_%s_%s_pT%dto%d.root",name.c_str(),massVar_.c_str(),int(pTMin),int(pTMax));
    TFile file(filename.c_str(), "READ");
    fit_result_ = dynamic_cast<RooFitResult*>(file.Get("fit_result"));
    total_pdf_ = dynamic_cast<RooAddPdf*>(file.Get("total_pdf"));
    reduced_data_ = dynamic_cast<RooDataSet*>(file.Get("reduced_data"));
    RooRealVar* mass = dynamic_cast<RooRealVar*>(total_pdf_->getVariables()->find("massPion"));
    activeMassVar_ = mass;
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
    CMS_lumi(canvas,"");
    
    canvas->Update();
    canvas->Modified();
    canvas->SaveAs(Form("Plots/test_%s_%s_pT%dto%d.pdf",name.c_str(),massVar_.c_str(),int(pTMin),int(pTMax)));
    return true;
    }

    void SetMassRange(std::string rangeName,double min, double max) {
        if (dynamic_cast<RooRealVar*>(activeMassVar_)) {
            dynamic_cast<RooRealVar*>(activeMassVar_)->setRange(rangeName.c_str(), min, max);
        }
    }

    int GetDataSize() const {
        if (reduced_data_) return reduced_data_->numEntries();
        // if (data_) return data_->numEntries();
        return 0;
    }

    RooDataSet* GetReducedData() const { return reduced_data_; }
void saveFitModel(double pTMin, double pTMax) {
        cout << "save model" << endl;
        std::string filename=Form("roots/test_%s_%s_pT%dto%d.root",name.c_str(),massVar_.c_str(),int(pTMin),int(pTMax));
        TFile file(filename.c_str(), "RECREATE");
        file.cd();
        if (fit_result_) {
            fit_result_->Write("fit_result");
        }
        if (total_pdf_) {
            total_pdf_->Write("total_pdf");
        }
        if (reduced_data_) {
            reduced_data_->Write("reduced_data");
        }
        file.Close();
    }

private:
    // Basic variables
    RooRealVar* eta_;              
    RooRealVar* pT_;              
    RooRealVar* phi_;              
    RooRealVar* y_;              
    RooRealVar* mva_;              
    RooDataSet* reduced_data_;      
    RooDataSet* data_;      
    
    // Fit components
    RooAbsPdf* signal_pdf_;         
    RooAbsPdf* background_pdf_;     
    RooAddPdf* total_pdf_;          
    RooFitResult* fit_result_;
    
    // Utility methods
    void Cleanup(){
        delete mass_;
        delete massDaughter1_;
        delete massPion_;
        delete mass_;
        delete eta_;              
        delete pT_;              
        delete phi_;              
        delete y_;              
        delete mva_;  
        delete reduced_data_;
        delete data_;
        delete signal_pdf_;
        delete background_pdf_;
        delete total_pdf_;
        delete fit_result_;
    
    } 

};

#endif // MASS_FITTER_H
