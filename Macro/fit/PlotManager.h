#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include <string>
#include "TFile.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooPlot.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLatex.h"
#include "TLegend.h"
#include "RooFitResult.h"
#include "Opt.h"

class PlotManager {
public:
    PlotManager(FitOpt opt);
    void DrawRawDistribution();
    void DrawFittedModel(bool drawPull = false);

private:
    std::string filename_;
    std::string datasetName_;
    std::string pdfName_;
    std::string varName_;
    std::string fitResultName_;
    std::string wsName_;
    std::string plotName_;
    TFile* file_;
    RooDataSet* dataset_;
    RooAbsPdf* pdf_;
    RooRealVar* var_;
    RooFitResult* fitResult_;
    RooWorkspace* ws_;
    int colorIndex = 1; // Start with a different color for each component

    void DrawPullFrame(RooPlot* frame);
    RooAbsPdf* ExtractComponent(const std::string& namePattern);
    void PlotComponents(RooPlot* frame, RooAbsPdf* Pdf);
    void DrawParameterPad();
};
PlotManager::PlotManager(FitOpt opt)
    : filename_(opt.outputFile), datasetName_(opt.datasetName), pdfName_(opt.pdfName), varName_(opt.massVar), fitResultName_(opt.fitResultName), wsName_(opt.wsName), plotName_(opt.plotName), file_(nullptr), dataset_(nullptr), pdf_(nullptr), var_(nullptr), fitResult_(nullptr) {
    file_ = TFile::Open(Form("roots/%s.root",filename_.c_str()));
    if (!file_ || file_->IsZombie()) {
        std::cerr << "Error: Failed to open file " << filename_ << std::endl;
        return;
    }
    ws_ = (RooWorkspace*)file_->Get(wsName_.c_str());
    if (!ws_) {
        std::cerr << "Error: Failed to load ws from file" << std::endl;
        file_->Close();
        return;
    }

    dataset_ = (RooDataSet*)ws_->data(datasetName_.c_str());
    if (!dataset_) {
        std::cerr << "Error: Failed to load dataset from workspace" << std::endl;
        file_->Close();
        return;
    }
    cout << "Total Entry : " << dataset_->sumEntries() << endl;
    if(opt.doFit){ 
    pdf_ = (RooAbsPdf*)ws_->pdf(pdfName_.c_str());
    if (!pdf_) {
        std::cerr << "Error: Failed to load PDF from file" << std::endl;
        file_->Close();
        return;
    }}
    const RooArgSet* vars = dataset_->get();
    
    if (vars) {
            var_ = (RooRealVar*)vars->find(varName_.c_str());
    }
    var_=(RooRealVar*)ws_->var(varName_.c_str());
    if (!var_) {
        std::cerr << "Error: Failed to load variable from RooDataSet" << std::endl;
        file_->Close();
        return;
    }
    var_->setRange("analysis", opt.massMin, opt.massMax);
    // var_->setMin(opt.massMin);
    // var_->setMax(opt.massMax);

    fitResult_ = (RooFitResult*)file_->Get(fitResultName_.c_str());
    if (!fitResult_) {
        std::cerr << "Error: Failed to load fit result from file" << std::endl;
        file_->Close();
        return;
    }
}

void PlotManager::DrawRawDistribution() {
    if (!dataset_ || !var_) {
        // std::cerr << "Error: Dataset or variable not loaded" << std::endl;
        return;
    }

    TCanvas* canvas = new TCanvas("canvas", "Raw Distribution", 800, 600);
    RooPlot* frame = var_->frame(RooFit::Range("analysis"));
    dataset_->plotOn(frame);
    frame->Draw();
    canvas->SaveAs(Form("plots_raw/RawDist%s.pdf",plotName_.c_str()));
    delete canvas;
}
RooAbsPdf* PlotManager::ExtractComponent(const std::string& namePattern) {
    const RooArgSet* components = pdf_->getComponents();
    TIterator* iter = components->createIterator();
    RooAbsArg* component;
    RooAbsPdf* foundPdf = nullptr;
    
    while ((component = (RooAbsArg*)iter->Next())) {
        RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(component);
        if (pdfComponent && std::string(pdfComponent->GetName()).find(namePattern) != std::string::npos) {
            foundPdf = pdfComponent;
            break;
        }
    }
    cout << "foundPdf: " << foundPdf->GetName() << endl;
    
    delete iter;
    return foundPdf;
}

void PlotManager::DrawFittedModel(bool drawPull) {
    if (!dataset_ || !pdf_ || !var_ || !fitResult_) {
        std::cerr << "Error: Dataset, PDF, variable, or fit result not loaded" << std::endl;
        return;
    }

    TCanvas* canvas = new TCanvas("canvas", "", 1200, 800);

    // Create pads for main plot, pull distribution, and parameter pad
    TPad* mainPad = new TPad("mainPad", "", 0.0, 0.3, 0.7, 1.0);
    TPad* pullPad = new TPad("pullPad", "", 0.0, 0.0, 0.7, 0.3);
    TPad* paramPad = new TPad("paramPad", "", 0.7, 0.0, 1.0, 1.0);

    mainPad->SetBottomMargin(0.00);
    mainPad->SetTopMargin(0.12);
    mainPad->SetLeftMargin(0.1);
    pullPad->SetTopMargin(0.01);
    pullPad->SetBottomMargin(0.3);

    mainPad->Draw();
    pullPad->Draw();
    paramPad->Draw();

    // Draw the main plot
    mainPad->cd();
    var_->Print("v");
    RooPlot* frame = var_->frame(RooFit::Bins(120),RooFit::Title(""),RooFit::Range("analysis"));
    dataset_->plotOn(frame, RooFit::Name("datapoints"));
    
    // Clear any fit range attribute that might cause problems
    // pdf_->setStringAttribute("fitrange", nullptr);
    
    // Plot the total PDF
    pdf_->plotOn(frame, RooFit::Name("model"), RooFit::NormRange("analysis"),RooFit::Range("analysis"));
    // pdf_->getParameters(dataset_)->Print("v");
    
    // Extract and plot signal component
    // RooAbsPdf* signalPdf = ExtractComponent("sig");
    // if (signalPdf) {
    //     PlotComponents(frame, signalPdf);
    // }
    
    // Extract and plot background component
    RooAbsPdf* backgroundPdf = ExtractComponent("tot");
    if (backgroundPdf) {
        PlotComponents(frame, backgroundPdf);
    }
    
    frame->Draw();

    // Draw the pull distribution
    if (drawPull) {
        pullPad->cd();
        DrawPullFrame(frame);
    }

    // Draw the fitted parameter values
    paramPad->cd();
    DrawParameterPad();

    canvas->SaveAs(Form("plots/%s.pdf",plotName_.c_str()));
    delete canvas;
}

void PlotManager::DrawPullFrame(RooPlot* frame) {
    RooHist* pullHist = frame->pullHist("datapoints", "model");
    RooPlot* pullFrame = var_->frame(RooFit::Title(" "),RooFit::Range("analysis"));
    pullFrame->addPlotable(pullHist, "P");
    pullFrame->SetMinimum(-2);
    pullFrame->SetMaximum(2);
    pullFrame->GetYaxis()->SetTitle("");
    pullFrame->GetYaxis()->SetTitleOffset(0.3);
    pullFrame->GetYaxis()->SetTitleSize(0.1);
    pullFrame->GetYaxis()->SetNdivisions(505);
    pullFrame->GetYaxis()->SetLabelSize(0.1);
    pullFrame->GetXaxis()->SetTitleSize(0.1);
    pullFrame->GetXaxis()->SetLabelSize(0.1);
    pullFrame->Draw();
    TLine* line0 = new TLine(var_->getMin("analysis"), 0, var_->getMax("analysis"), 0);
    line0->SetLineStyle(2);
    line0->Draw();
}

void PlotManager::DrawParameterPad() {
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.03);

    const RooArgList& params = fitResult_->floatParsFinal();
    for (int i = 0; i < params.getSize(); ++i) {
        RooRealVar* param = (RooRealVar*)params.at(i);
        latex.DrawLatex(0.1, 0.9 - i * 0.05, Form("%s = %.3f #pm %.3f", param->GetName(), param->getVal(), param->getError()));
    }
}
void PlotManager::PlotComponents(RooPlot* frame, RooAbsPdf* Pdf) {
    const RooArgSet* components = Pdf->getComponents();
    if (components->getSize() > 1) {
        TIterator* iter = components->createIterator();
        RooAbsArg* component;
        while ((component = (RooAbsArg*)iter->Next())) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(component);
            
                Pdf->plotOn(frame, 
                                  RooFit::Components(*pdfComponent),
                                  RooFit::LineStyle(kDashed),
                                  RooFit::LineColor(colorIndex),
                                  RooFit::Name(pdfComponent->GetName()),
                                  RooFit::Range("analysis"),
                                  RooFit::NormRange("analysis"));
                colorIndex++;
            
        }
        delete iter;
    } else {
        Pdf->plotOn(frame, 
                          RooFit::Components(*Pdf),
                          RooFit::LineStyle(kDashed),
                          RooFit::LineColor(colorIndex),
                          RooFit::Name(Pdf->GetName()),
                        //   RooFit::Normalization(1.0, RooAbsReal::Raw),
                          RooFit::Range("analysis"),
                          RooFit::NormRange("analysis"));
                colorIndex++;
                        //   RooFit::NormRange("analysis"));
    }
}

#endif // PLOTMANAGER_H