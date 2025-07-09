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
#include "Helper.h"

class PlotManager {
public:
    PlotManager(FitOpt& opt, const std::string& inputDir, const std::string& outputFile, const std::string& outputDir,bool isPP, bool isDstar);
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
    std::string fileDir_;
    std::string outputDir_;
    bool isDstar_;
    bool isPP_;
    TFile* file_;
    RooDataSet* dataset_;
    FitOpt opt_;
    RooAbsPdf* pdf_;
    RooRealVar* var_;
    RooFitResult* fitResult_;
    RooWorkspace* ws_;
    int colorIndex = 1; // Start with a different color for each component
    std::vector<std::pair<std::string, int>> legendEntries; // 컴포넌트 이름과 색상 저장

    void DrawPullFrame(RooPlot* frame);
    RooAbsPdf* ExtractComponent(const std::string& namePattern);
    void DrawParameterPad();
};
PlotManager::PlotManager(FitOpt& opt, const std::string& inputDir, const std::string& outputFile, const std::string& outputDir,bool isPP, bool isDstar)
    : opt_(opt), fileDir_(inputDir), filename_(outputFile),outputDir_(outputDir), datasetName_(opt.datasetName), pdfName_(opt.pdfName), varName_(opt.massVar), fitResultName_(opt.fitResultName), wsName_(opt.wsName), plotName_(opt.plotName), file_(nullptr), dataset_(nullptr), pdf_(nullptr), var_(nullptr), fitResult_(nullptr), isDstar_(isDstar),isPP_(isPP) {
    file_ = TFile::Open(Form("%s/%s",fileDir_.c_str(), filename_.c_str()));
    if(file_) cout << "File Opened :: " << Form("%s/%s",fileDir_.c_str(), filename_.c_str()) <<endl;
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
        std::cerr << "Error: Failed to load dataset from workspace : " << datasetName_ <<  std::endl;
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
    var_->setMin(opt.massMin);
    var_->setMax(opt.massMax);

    fitResult_ = (RooFitResult*)file_->Get(fitResultName_.c_str());
    if (!fitResult_) {
        std::cerr << "Error: Failed to load fit result from file" << std::endl;
        file_->Close();
        // return;
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
    createDir(Form("%s/",outputDir_.c_str()));
    canvas->SaveAs(Form("%s/RawDist%s",outputDir_.c_str(),plotName_.c_str()));
    delete canvas;
}

void PlotManager::DrawFittedModel(bool drawPull) {
    if (!dataset_ || !pdf_ || !var_ || !fitResult_) {
        std::cerr << "Error: Dataset, PDF, variable, or fit result not loaded" << std::endl;
        return;
    }

    // --- Canvas and Pad Setup ---
    TCanvas* canvas = new TCanvas("canvas", "", 1200, 800);
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

    // --- Main Plot ---
    mainPad->cd();
    RooPlot* frame = var_->frame(RooFit::Bins(120), RooFit::Title(""), RooFit::Range("analysis"));

    // 1. Plot Data
    dataset_->plotOn(frame, RooFit::Name("datapoints"), RooFit::MarkerStyle(kFullCircle), RooFit::MarkerSize(0.8));

    // 2. Identify and Plot Components (assuming standard naming conventions)
    RooAbsPdf* signalPdf = ExtractComponent("sig") ? ExtractComponent("sig") : ExtractComponent("total_pdf");
    RooAbsPdf* swappedPdf = ExtractComponent("Swap1");
    RooAbsPdf* combinatorialPdf = isDstar_ ? ExtractComponent("bkg") : ExtractComponent("bkg");
    if (signalPdf) {
        pdf_->plotOn(frame, RooFit::Components(*signalPdf), RooFit::Name("signal"),
                     RooFit::FillColor(kAzure-9), RooFit::FillStyle(3354),
                     RooFit::DrawOption("F"),
                     RooFit::LineColor(kAzure-9), RooFit::LineStyle(kDotted), RooFit::LineWidth(2),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
        pdf_->plotOn(frame, RooFit::Components(*signalPdf), RooFit::Name("signal_line"),
                     RooFit::LineColor(kAzure-9), RooFit::LineStyle(kDotted), RooFit::LineWidth(2),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
    }


    // Plot Combinatorial Background (dashed line)
    if (combinatorialPdf) {
        pdf_->plotOn(frame, RooFit::Components(*combinatorialPdf), RooFit::Name("combinatorial"),
                     RooFit::LineColor(kBlue), RooFit::LineStyle(kDashed), RooFit::LineWidth(2),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
    }

    // Plot Swapped Component (filled area, solid outline)
    if (swappedPdf) {
        pdf_->plotOn(frame, RooFit::Components(*swappedPdf), RooFit::Name("swapped"),
                     RooFit::FillColor(kGreen + 2), RooFit::FillStyle(3354),
                     RooFit::DrawOption("F"),
                     RooFit::LineColor(kGreen + 2), RooFit::LineStyle(kSolid), RooFit::LineWidth(1),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
        pdf_->plotOn(frame, RooFit::Components(*swappedPdf), RooFit::Name("swapped_line"),
                     RooFit::LineColor(kGreen + 2), RooFit::LineStyle(kSolid), RooFit::LineWidth(2),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
    }

    // Plot Signal Component (filled area, dotted outline)
    // 3. Plot Total Fit Line (Solid Red)
    pdf_->plotOn(frame, RooFit::Name("model"), RooFit::LineColor(kRed), RooFit::LineWidth(2),
                 RooFit::NormRange("analysis"), RooFit::Range("analysis"));

    // 4. Re-plot Data Points on top
    dataset_->plotOn(frame, RooFit::Name("datapoints_top"), RooFit::MarkerStyle(kFullCircle), RooFit::MarkerSize(0.8));

    frame->GetYaxis()->SetTitleOffset(1.2);
    frame->GetYaxis()->SetTitle(Form("Entries / (%.3f GeV/c^{2})", var_->getBinWidth(0)));
    frame->SetTitle("");
    frame->Draw();

    // --- Legend ---
    TLegend* legend = new TLegend(0.65, 0.60, 0.93, 0.88);
    // TLegend* legend = new TLegend(0.65, 0.08, 0.93, 0.36);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.04);

    legend->AddEntry(frame->findObject("datapoints"), "Data", "PE");
    legend->AddEntry(frame->findObject("model"), "Fit", "L");

    if (signalPdf) {
        isDstar_ ? legend->AddEntry(frame->findObject("signal"), "D^{*} Signal", "F") : legend->AddEntry(frame->findObject("signal"), "D^{0}+#bar{D}^{0} Signal", "F");
    }
    if (swappedPdf) {
        legend->AddEntry(frame->findObject("swapped"), "K-#pi swapped", "F");
    }
    if (combinatorialPdf) {
        legend->AddEntry(frame->findObject("combinatorial"), "Combinatorial", "L");
    }

    legend->Draw();

    // --- CMS Preliminary Text ---
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(61);
    latex.SetTextSize(0.05);
    latex.DrawLatex(0.12, 0.90, "CMS");
    latex.SetTextFont(52);
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.20, 0.90, "Preliminary");

    latex.SetTextFont(42);
    latex.SetTextSize(0.04);
    latex.SetTextAlign(11);
    isPP_ ? latex.DrawLatex(0.55, 0.90, "ppRef #sqrt{s_{NN}} = 5.32 TeV") : latex.DrawLatex(0.55, 0.90, "PbPb #sqrt{s_{NN}} = 5.32 TeV");
    // isPP_ ? latex.DrawLatex(0.55, 0.90, "Proton-Oxygen #sqrt{s_{NN}} = 9.62 TeV") : latex.DrawLatex(0.93, 0.90, "PbPb #sqrt{s_{NN}} = 5.32 TeV");

    double x_pos = 0.13;
    double y_pos = 0.80;
    latex.DrawLatex(x_pos,y_pos, Form(" %.1f < p_{T} < %.1f GeV/c", opt_.pTMin, opt_.pTMax));
    latex.DrawLatex(x_pos,y_pos - 0.07,"-1 < |y| < 1");
    isDstar_ ? latex.DrawLatex(x_pos,y_pos-0.14, Form("%.2f < |cos#theta_{HX}| < %0.2f", opt_.cosMin, opt_.cosMax)) : latex.DrawLatex(0.93, 0.60, Form("mva < %f", opt_.mvaMin));


    latex.SetTextAlign(11);

    // --- Pull Plot ---
    if (drawPull) {
        pullPad->cd();
        DrawPullFrame(frame);
    }

    // --- Parameter Pad ---
    paramPad->cd();
    DrawParameterPad();

    // --- Save Canvas ---
    createDir(Form("%s/", outputDir_.c_str()));
    canvas->SaveAs(Form("%s/%s", outputDir_.c_str(), plotName_.c_str()));

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
    latex.DrawLatex(0.1, 0.9, Form("STATUS %s : %d, %s : %d", fitResult_->statusLabelHistory(0), 
                                      fitResult_->statusCodeHistory(0),
                                      fitResult_->statusLabelHistory(1), 
                                      fitResult_->statusCodeHistory(1)));
    

    const RooArgList& params = fitResult_->floatParsFinal();
    for (int i = 0; i < params.getSize(); ++i) {
        RooRealVar* param = (RooRealVar*)params.at(i);
        latex.DrawLatex(0.1, 0.85 - i * 0.05, Form("%s = %.5f #pm %.5f", param->GetName(), param->getVal(), param->getError()));
    }
}

RooAbsPdf* PlotManager::ExtractComponent(const std::string& namePattern) {
    if (std::string(pdf_->GetName()).find(namePattern) != std::string::npos) {
    }

    const RooArgSet* components = pdf_->getComponents();
    RooAbsPdf* foundPdf = nullptr;

    if (components) {
        for (const auto& obj : *components) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(obj);
                std::cout << "Found component '" << namePattern << "': " << pdfComponent->GetName() << std::endl;
            if (pdfComponent && std::string(pdfComponent->GetName()).find(namePattern) != std::string::npos) {
                foundPdf = pdfComponent;
                // std::cout << "Found component '" << namePattern << "': " << foundPdf->GetName() << std::endl;
                break;
            }
        }
    }

    if (!foundPdf) {
         std::cout << "Warning: Component containing '" << namePattern << "' not found." << std::endl;
    }

    return foundPdf;
}

#endif // PLOTMANAGER_H