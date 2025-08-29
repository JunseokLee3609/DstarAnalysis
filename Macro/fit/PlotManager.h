#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include <string>
#include <memory>
#include <vector>
#include "TFile.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooPlot.h"
#include "RooWorkspace.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "RooFitResult.h"
#include "TStyle.h"
#include "TDirectory.h"
#include "Opt.h"
#include "Helper.h"

/**
 * @brief Enhanced PlotManager for D* meson analysis plotting
 * 
 * Features:
 * - Automatic workspace loading and validation
 * - Enhanced plotting with legends and annotations
 * - Publication-ready plot formatting
 * - Error handling and diagnostics
 */
class PlotManager {
public:
    // Constructor
    PlotManager(const FitOpt& opt, const std::string& inputDir, const std::string& outputFile, 
                const std::string& outputDir, bool isPP = false, bool isDstar = true);
    
    // Destructor
    ~PlotManager();
    
    // Main plotting methods
    bool DrawRawDistribution(const std::string& outputName = "");
    bool DrawFittedModel(bool drawPull = true, const std::string& outputName = "");
    bool DrawComparisonPlot(const std::vector<std::string>& fileList, 
                           const std::vector<std::string>& legendLabels);
    
    // Utility methods
    bool IsValid() const { return isValid_; }
    void SetPlotStyle(const std::string& style = "CMS");
    void PrintSummary() const;

private:
    // Core data members
    FitOpt opt_;
    std::string filename_;
    std::string fileDir_;
    std::string outputDir_;
    bool isDstar_;
    bool isPP_;
    bool isValid_;
    
    // ROOT objects (using smart pointers for better memory management)
    std::unique_ptr<TFile> file_;
    RooWorkspace* ws_;          // Owned by file
    RooDataSet* dataset_;       // Owned by workspace
    RooAbsPdf* pdf_;           // Owned by workspace
    RooRealVar* var_;          // Owned by workspace
    RooFitResult* fitResult_;  // Owned by file
    
    // Plotting configuration
    int colorIndex_;
    std::vector<std::pair<std::string, int>> legendEntries_;
    
    // Style settings
    struct PlotStyle {
        int canvasWidth = 1200;
        int canvasHeight = 800;
        double leftMargin = 0.12;
        double rightMargin = 0.05;
        double topMargin = 0.08;
        double bottomMargin = 0.12;
        int titleFont = 42;
        int labelFont = 42;
        double titleSize = 0.05;
        double labelSize = 0.04;
    } plotStyle_;

    // Internal methods
    bool LoadWorkspace();
    bool ValidateObjects();
    void DrawPullFrame(RooPlot* frame);
    RooAbsPdf* ExtractComponent(const std::string& namePattern);
    void DrawParameterPad(TPad* paramPad);
    void AddLegendEntry(const std::string& name, int color, const std::string& option = "l");
    TLegend* CreateLegend(double x1, double y1, double x2, double y2);
    void SetupCanvas(TCanvas* canvas);
    void AddLabels(TPad* pad);
    std::string GetOutputFileName(const std::string& baseName, const std::string& suffix = "");
    void CleanupMemory();
};
PlotManager::PlotManager(const FitOpt& opt, const std::string& inputDir, const std::string& outputFile, const std::string& outputDir,bool isPP, bool isDstar)
    : opt_(opt), fileDir_(inputDir), filename_(outputFile), outputDir_(outputDir), file_(nullptr), dataset_(nullptr), pdf_(nullptr), var_(nullptr), fitResult_(nullptr), isDstar_(isDstar), isPP_(isPP) {
    file_.reset(TFile::Open(Form("%s/%s",fileDir_.c_str(), filename_.c_str())));
    if(file_) std::cout << "File Opened :: " << Form("%s/%s",fileDir_.c_str(), filename_.c_str()) << std::endl;
    if (!file_ || file_->IsZombie()) {
        std::cerr << "Error: Failed to open file " << filename_ << std::endl;
        return;
    }
    ws_ = (RooWorkspace*)file_->Get(opt_.wsName.c_str());
    if (!ws_) {
        std::cerr << "Error: Failed to load ws from file" << std::endl;
        file_->Close();
        return;
    }

    dataset_ = (RooDataSet*)ws_->data(opt_.datasetName.c_str());
    if (!dataset_) {
        std::cerr << "Error: Failed to load dataset from workspace : " << opt_.datasetName <<  std::endl;
        file_->Close();
        return;
    }
    cout << "Total Entry : " << dataset_->sumEntries() << endl;
    if(opt.doFit){ 
        // Try common PDF names
        pdf_ = (RooAbsPdf*)ws_->pdf("total_pdf");
        if (!pdf_) pdf_ = (RooAbsPdf*)ws_->pdf("model");
        if (!pdf_) {
            std::cerr << "Warning: Failed to find PDF 'total_pdf' or 'model' in workspace" << std::endl;
        }
    }
    const RooArgSet* vars = dataset_->get();
    
    if (vars) {
            var_ = (RooRealVar*)vars->find(opt_.massVar.c_str());
    }
    var_=(RooRealVar*)ws_->var(opt_.massVar.c_str());
    if (!var_) {
        std::cerr << "Error: Failed to load variable from RooDataSet" << std::endl;
        file_->Close();
        return;
    }
    var_->setRange("analysis", opt.massMin, opt.massMax);
    var_->setMin(opt.massMin);
    var_->setMax(opt.massMax);

    fitResult_ = (RooFitResult*)file_->Get(opt_.fitResultName.c_str());
    if (!fitResult_) {
        std::cerr << "Error: Failed to load fit result from file" << std::endl;
        file_->Close();
        // return;
    }
}

bool PlotManager::DrawRawDistribution(const std::string& outputName) {
    if (!dataset_ || !var_) {
        // std::cerr << "Error: Dataset or variable not loaded" << std::endl;
        return false;
    }

    TCanvas* canvas = new TCanvas("canvas", "Raw Distribution", 800, 600);
    RooPlot* frame = var_->frame(RooFit::Range("analysis"));
    dataset_->plotOn(frame);
    frame->Draw();
    createDir(Form("%s/",outputDir_.c_str()));
    {
        std::string outname = GetOutputFileName("RawDist" + opt_.plotName);
        canvas->SaveAs(outname.c_str());
    }
    delete canvas;
    return true;
}

bool PlotManager::DrawFittedModel(bool drawPull, const std::string& outputName) {
    if (!dataset_ || !pdf_ || !var_ || !fitResult_) {
        std::cerr << "Error: Dataset, PDF, variable, or fit result not loaded" << std::endl;
        return false;
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
    RooPlot* frame = var_->frame(RooFit::Bins(60), RooFit::Title(""), RooFit::Range("analysis"));

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
    frame->GetYaxis()->SetLabelSize(0.0);
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
    latex.DrawLatex(0.20, 0.90, "Internal");

    latex.SetTextFont(42);
    latex.SetTextSize(0.04);
    latex.SetTextAlign(11);
    latex.DrawLatex(0.55, 0.90, opt_.ELabel.c_str());
    // isPP_ ? latex.DrawLatex(0.55, 0.90, "Proton-Oxygen #sqrt{s_{NN}} = 9.62 TeV") : latex.DrawLatex(0.93, 0.90, "PbPb #sqrt{s_{NN}} = 5.32 TeV");

    double x_pos = 0.13;
    double y_pos = 0.80;
    latex.DrawLatex(x_pos,y_pos, opt_.pTLegend.c_str());
    latex.DrawLatex(x_pos,y_pos - 0.07, opt_.yLegend.c_str());
    latex.DrawLatex(x_pos,y_pos-0.14, opt_.centLegend.c_str());


    latex.SetTextAlign(11);

    // --- Pull Plot ---
    if (drawPull) {
        pullPad->cd();
        DrawPullFrame(frame);
    }

    // --- Parameter Pad ---
    paramPad->cd();
    DrawParameterPad(paramPad);

    // --- Save Canvas ---
    createDir(Form("%s/", outputDir_.c_str()));
    {
        std::string outName = GetOutputFileName(opt_.plotName);
        canvas->SaveAs(outName.c_str());
    }

    delete canvas;
    return true;
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
    pullFrame->GetYaxis()->SetLabelSize(0.0);
    pullFrame->GetXaxis()->SetTitleSize(0.1);
    pullFrame->GetXaxis()->SetLabelSize(0.1);
    pullFrame->Draw();
    TLine* line0 = new TLine(var_->getMin("analysis"), 0, var_->getMax("analysis"), 0);
    line0->SetLineStyle(2);
    line0->Draw();
}

void PlotManager::DrawParameterPad(TPad* paramPad) {
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

// === New Enhanced Methods ===

void PlotManager::SetPlotStyle(const std::string& style) {
    if (style == "CMS") {
        gStyle->SetOptStat(0);
        gStyle->SetOptTitle(0);
        gStyle->SetPadLeftMargin(plotStyle_.leftMargin);
        gStyle->SetPadRightMargin(plotStyle_.rightMargin);
        gStyle->SetPadTopMargin(plotStyle_.topMargin);
        gStyle->SetPadBottomMargin(plotStyle_.bottomMargin);
        gStyle->SetTextFont(plotStyle_.titleFont);
        gStyle->SetLabelFont(plotStyle_.labelFont, "XYZ");
        gStyle->SetTitleFont(plotStyle_.titleFont, "XYZ");
        gStyle->SetLabelSize(plotStyle_.labelSize, "XYZ");
        gStyle->SetTitleSize(plotStyle_.titleSize, "XYZ");
    }
    std::cout << "[PlotManager] Applied " << style << " plot style" << std::endl;
}

std::string PlotManager::GetOutputFileName(const std::string& baseName, const std::string& suffix) {
    std::string outputName = outputDir_ + "/" + baseName;
    if (!suffix.empty()) {
        outputName += "_" + suffix;
    }
    outputName += ".pdf";
    return outputName;
}

TLegend* PlotManager::CreateLegend(double x1, double y1, double x2, double y2) {
    TLegend* legend = new TLegend(x1, y1, x2, y2);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextFont(plotStyle_.labelFont);
    legend->SetTextSize(plotStyle_.labelSize * 0.9);
    return legend;
}

void PlotManager::AddLabels(TPad* pad) {
    pad->cd();
    
    TLatex* cmsLabel = new TLatex();
    cmsLabel->SetTextFont(plotStyle_.titleFont);
    cmsLabel->SetTextSize(plotStyle_.titleSize * 1.1);
    cmsLabel->SetNDC();
    
    // CMS label
    cmsLabel->DrawLatex(0.12, 0.92, "CMS");
    cmsLabel->SetTextFont(plotStyle_.labelFont);
    cmsLabel->SetTextSize(plotStyle_.titleSize * 0.9);
    cmsLabel->DrawLatex(0.20, 0.92, "Work in Progress");
    
    // Collision system and energy
    std::string collisionText = isPP_ ? "pp" : "PbPb";
    collisionText += ", #sqrt{s_{NN}} = 5.02 TeV";
    cmsLabel->DrawLatex(0.65, 0.92, collisionText.c_str());
    
    // Physics labels from FitOpt - use auto-generated legends!
    double yPos = 0.85;
    if (!opt_.pTLegend.empty()) {
        cmsLabel->DrawLatex(0.65, yPos, opt_.pTLegend.c_str());
        yPos -= 0.05;
    }
    if (!opt_.cosLegend.empty()) {
        cmsLabel->DrawLatex(0.65, yPos, opt_.cosLegend.c_str());
        yPos -= 0.05;
    }
    if (!opt_.centLegend.empty() && !isPP_) {
        cmsLabel->DrawLatex(0.65, yPos, ("Centrality " + opt_.centLegend).c_str());
        yPos -= 0.05;
    }
}

void PlotManager::PrintSummary() const {
    std::cout << "\n=== PlotManager Summary ===" << std::endl;
    std::cout << "File: " << fileDir_ << "/" << filename_ << std::endl;
    std::cout << "Workspace: " << opt_.wsName << std::endl;
    std::cout << "Dataset: " << opt_.datasetName;
    if (dataset_) std::cout << " (" << dataset_->sumEntries() << " entries)";
    std::cout << std::endl;
    std::cout << "Variable: " << opt_.massVar << " [" << opt_.massMin << ", " << opt_.massMax << "]" << std::endl;
    std::cout << "PDF available: " << (pdf_ ? "Yes" : "No") << std::endl;
    std::cout << "Fit result available: " << (fitResult_ ? "Yes" : "No") << std::endl;
    std::cout << "========================" << std::endl;
}

// Enhanced plotting methods (duplicate removed)

void PlotManager::SetupCanvas(TCanvas* canvas) {
    canvas->SetLeftMargin(plotStyle_.leftMargin);
    canvas->SetRightMargin(plotStyle_.rightMargin);
    canvas->SetTopMargin(plotStyle_.topMargin);
    canvas->SetBottomMargin(plotStyle_.bottomMargin);
}

void PlotManager::CleanupMemory() {
    // ROOT objects are owned by TFile or RooWorkspace
    // std::unique_ptr<TFile> will handle cleanup automatically
}

#endif // PLOTMANAGER_H
