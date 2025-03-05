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
    TFile* file_;
    RooDataSet* dataset_;
    RooAbsPdf* pdf_;
    RooRealVar* var_;
    RooFitResult* fitResult_;

    void DrawPullFrame(RooPlot* frame);
    RooAbsPdf* ExtractComponent(const std::string& namePattern);
    void PlotComponents(RooPlot* frame, RooAbsPdf* Pdf);
    void DrawParameterPad();
};
PlotManager::PlotManager(FitOpt opt)
    : filename_(opt.outputFile), datasetName_(opt.reducedDataName), pdfName_(opt.pdfName), varName_(opt.massVar), fitResultName_(opt.fitResultName), file_(nullptr), dataset_(nullptr), pdf_(nullptr), var_(nullptr), fitResult_(nullptr) {
    file_ = TFile::Open(filename_.c_str());
    if (!file_ || file_->IsZombie()) {
        std::cerr << "Error: Failed to open file " << filename_ << std::endl;
        return;
    }

    dataset_ = (RooDataSet*)file_->Get(datasetName_.c_str());
    if (!dataset_) {
        std::cerr << "Error: Failed to load dataset from file" << std::endl;
        file_->Close();
        return;
    }

    pdf_ = (RooAbsPdf*)file_->Get(pdfName_.c_str());
    if (!pdf_) {
        std::cerr << "Error: Failed to load PDF from file" << std::endl;
        file_->Close();
        return;
    }
    const RooArgSet* vars = dataset_->get();
    if (vars) {
            var_ = (RooRealVar*)vars->find(varName_.c_str());
    }
    if (!var_) {
        std::cerr << "Error: Failed to load variable from RooDataSet" << std::endl;
        file_->Close();
        return;
    }
    var_->setRange("analysis", opt.massMin, opt.massMax);

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
    RooPlot* frame = var_->frame(RooFit::Range("analysis"),RooFit::NormRange("analysis"));
    dataset_->plotOn(frame);
    frame->Draw();
    canvas->SaveAs("raw_distribution.png");
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

    mainPad->SetBottomMargin(0.02);
    pullPad->SetTopMargin(0.02);
    pullPad->SetBottomMargin(0.3);

    mainPad->Draw();
    pullPad->Draw();
    paramPad->Draw();

    // Draw the main plot
    mainPad->cd();
    RooPlot* frame = var_->frame(RooFit::Title("Fit Result"), RooFit::Range("analysis"));
    dataset_->plotOn(frame, RooFit::Range("analysis"), RooFit::NormRange("analysis"));
    
    // Clear any fit range attribute that might cause problems
    pdf_->setStringAttribute("fitrange", nullptr);
    
    // Plot the total PDF
    pdf_->plotOn(frame, RooFit::Range("analysis"), RooFit::NormRange("analysis"));
    
    // Extract and plot signal component
    RooAbsPdf* signalPdf = ExtractComponent("sig");
    if (signalPdf) {
        PlotComponents(frame, signalPdf);
    }
    
    // Extract and plot background component
    RooAbsPdf* backgroundPdf = ExtractComponent("bkg");
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

    canvas->SaveAs("fitted_model.png");
    delete canvas;
}

void PlotManager::DrawPullFrame(RooPlot* frame) {
    RooHist* pullHist = frame->pullHist();
    RooPlot* pullFrame = var_->frame(RooFit::Range("analysis"));
    pullFrame->addPlotable(pullHist, "P");
    pullFrame->GetYaxis()->SetTitle("Pull");
    pullFrame->GetYaxis()->SetTitleOffset(1.5);
    pullFrame->GetXaxis()->SetTitleSize(0.1);
    pullFrame->GetYaxis()->SetTitleSize(0.1);
    pullFrame->GetXaxis()->SetLabelSize(0.1);
    pullFrame->GetYaxis()->SetLabelSize(0.1);
    pullFrame->Draw();
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
        int colorIndex = 5; // Start with a different color for each component
        while ((component = (RooAbsArg*)iter->Next())) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(component);
            if (pdfComponent && std::string(pdfComponent->GetName()).find("sig") == 0) {
                Pdf->plotOn(frame, 
                                  RooFit::Components(*pdfComponent),
                                  RooFit::LineStyle(kDashed),
                                  RooFit::LineColor(colorIndex),
                                  RooFit::Name(pdfComponent->GetName()),
                                  RooFit::Range("analysis"));
                colorIndex++;
            }
        }
        delete iter;
    } else {
        Pdf->plotOn(frame, 
                          RooFit::Components(*Pdf),
                          RooFit::LineStyle(kDashed),
                          RooFit::LineColor(kRed),
                          RooFit::Name("background"),
                          RooFit::Range("analysis"));
    }
}

#endif // PLOTMANAGER_H