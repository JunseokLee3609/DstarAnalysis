#ifndef ENHANCED_PLOTMANAGER_H
#define ENHANCED_PLOTMANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
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
#include "RooFitResult.h"
#include "TStyle.h"
#include "TDirectory.h"
#include "RooFit.h"
#include "Opt.h"
#include "Helper.h"

/**
 * @brief Enhanced PlotManager for D* meson analysis plotting
 * 
 * Features:
 * - Unified workspace loading with proper error handling
 * - Enhanced plotting with auto-generated legends
 * - Publication-ready plot formatting
 * - Support for both D* and D0 analysis
 * - Comprehensive diagnostics and validation
 */
class EnhancedPlotManager {
public:
    // Constructor
    EnhancedPlotManager(const FitOpt& opt, const std::string& inputDir, const std::string& outputFile, 
                       const std::string& outputDir, bool isPP = false, bool isDstar = true);
    
    // Destructor
    ~EnhancedPlotManager();
    
    // Main plotting methods
    bool DrawRawDistribution(const std::string& outputName = "");
    bool DrawFittedModel(bool drawPull = true, const std::string& outputName = "");
    bool DrawComparisonPlot(const std::vector<std::string>& fileList, 
                           const std::vector<std::string>& legendLabels);
    
    // Utility methods
    bool IsValid() const { return isValid_; }
    void SetPlotStyle(const std::string& style = "CMS");
    void PrintSummary() const;
    void PrintWorkspaceContents() const;

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
    void DrawPullFrame(RooPlot* frame, TPad* pullPad);
    RooAbsPdf* ExtractComponent(const std::string& namePattern);
    void DrawParameterPad(TPad* paramPad);
    void AddLegendEntry(const std::string& name, int color, const std::string& option = "l");
    TLegend* CreateLegend(double x1, double y1, double x2, double y2);
    void SetupCanvas(TCanvas* canvas);
    void AddLabels(TPad* pad);
    std::string GetOutputFileName(const std::string& baseName, const std::string& suffix = "");
    void CleanupMemory();
};

// Implementation
EnhancedPlotManager::EnhancedPlotManager(const FitOpt& opt, const std::string& inputDir, const std::string& outputFile, 
                                       const std::string& outputDir, bool isPP, bool isDstar)
    : opt_(opt), fileDir_(inputDir), filename_(outputFile), outputDir_(outputDir), 
      isDstar_(isDstar), isPP_(isPP), isValid_(false), colorIndex_(1),
      ws_(nullptr), dataset_(nullptr), pdf_(nullptr), var_(nullptr), fitResult_(nullptr) {
    
    std::cout << "=== Enhanced PlotManager Initialization ===" << std::endl;
    std::cout << "[PlotManager] File: " << fileDir_ << "/" << filename_ << std::endl;
    std::cout << "[PlotManager] Looking for workspace: " << opt_.wsName << std::endl;
    std::cout << "[PlotManager] Dataset: " << opt_.datasetName << std::endl;
    std::cout << "[PlotManager] Variable: " << opt_.massVar << std::endl;
    
    // Load and validate workspace
    if (LoadWorkspace() && ValidateObjects()) {
        isValid_ = true;
        std::cout << "[PlotManager] ✓ Successfully initialized!" << std::endl;
        std::cout << "[PlotManager] Dataset entries: " << dataset_->sumEntries() << std::endl;
        
        // Set up variable range
        var_->setRange("analysis", opt_.massMin, opt_.massMax);
        var_->setMin(opt_.massMin);
        var_->setMax(opt_.massMax);
        
        // Set default plot style
        SetPlotStyle("CMS");
        
        // Generate legends if not already set
        const_cast<FitOpt&>(opt_).GenerateLegends();
        
        std::cout << "[PlotManager] Auto-generated legends:" << std::endl;
        std::cout << "  pT: " << opt_.pTLegend << std::endl;
        std::cout << "  cos: " << opt_.cosLegend << std::endl;
        std::cout << "  centrality: " << opt_.centLegend << std::endl;
        
    } else {
        std::cerr << "[PlotManager] ✗ Initialization failed!" << std::endl;
        std::cout << "[PlotManager] This is expected if the fit results were not saved to a ROOT file." << std::endl;
        std::cout << "[PlotManager] The analysis will continue with basic plotting only." << std::endl;
    }
    std::cout << "=============================================" << std::endl;
}

EnhancedPlotManager::~EnhancedPlotManager() {
    CleanupMemory();
}

bool EnhancedPlotManager::LoadWorkspace() {
    // Construct full path to ROOT file
    std::string fullPath = Form("%s/%s", fileDir_.c_str(), filename_.c_str());
    
    // Check if file exists first
    std::ifstream testFile(fullPath);
    if (!testFile.good()) {
        std::cout << "[PlotManager] Info: ROOT file does not exist: " << fullPath << std::endl;
        std::cout << "[PlotManager] This is normal if no fit results were saved to disk." << std::endl;
        std::cout << "[PlotManager] Available files in " << fileDir_ << "/:" << std::endl;
        system(Form("ls -la %s/", fileDir_.c_str()));
        return false;
    }
    testFile.close();
    
    // Open ROOT file
    file_ = std::make_unique<TFile>(fullPath.c_str());
    
    if (!file_ || file_->IsZombie()) {
        std::cerr << "[PlotManager] Error: Failed to open file " << fullPath << std::endl;
        return false;
    }
    
    // Load workspace - try multiple possible names
    std::vector<std::string> wsNames = {
        opt_.wsName,        // Configured name (e.g., "workspace_DStar")
        "workspace_DStar_" + opt_.GetBinName(),  // Full name with bin info  
        "workspace_DStar_" + opt_.name,          // Try with full opt_.name
        "workspace",         // Fallback generic name
        // MC workspaces saved by MassFitterV2::PerformMCFit
        "mcWorkspace_" + opt_.name
    };
    
    for (const auto& wsName : wsNames) {
        ws_ = dynamic_cast<RooWorkspace*>(file_->Get(wsName.c_str()));
        if (ws_) {
            std::cout << "[PlotManager] ✓ Successfully loaded workspace: " << wsName << std::endl;
            break;
        }
    }
    
    if (!ws_) {
        std::cerr << "[PlotManager] Error: Failed to load workspace. Tried names: ";
        for (const auto& name : wsNames) std::cerr << "'" << name << "' ";
        std::cerr << std::endl;
        std::cout << "[PlotManager] Available objects in file:" << std::endl;
        file_->ls();
        return false;
    }
    
    return true;
}

bool EnhancedPlotManager::ValidateObjects() {
    // Load dataset
    dataset_ = dynamic_cast<RooDataSet*>(ws_->data(opt_.datasetName.c_str()));
    if (!dataset_) {
        std::cerr << "[PlotManager] Error: Failed to load dataset '" << opt_.datasetName << "' from workspace" << std::endl;
        std::cout << "[PlotManager] Available datasets in workspace:" << std::endl;
        auto dataList = ws_->allData();
        for (auto* data : dataList) {
            if (data) std::cout << "  - " << data->GetName() << std::endl;
        }
        return false;
    }
    
    // Load variable
    var_ = dynamic_cast<RooRealVar*>(ws_->var(opt_.massVar.c_str()));
    if (!var_) {
        std::cerr << "[PlotManager] Error: Failed to load variable '" << opt_.massVar << "' from workspace" << std::endl;
        std::cout << "[PlotManager] Available variables in workspace:" << std::endl;
        RooArgSet vars = ws_->allVars();
        vars.Print();
        return false;
    }
    
    // Load PDF (optional) - try multiple common names (include MC signal-only case)
    if (opt_.doFit) {
        std::vector<std::string> pdfNames = {
            "totalPdf",         // MassFitterV2 uses this name
            "total_pdf",        // Alternative common name
            "pdf",              // Generic name
            "pdf_" + opt_.name, // Name with prefix
            opt_.name + "_pdf", // Name with suffix
            "mcSignal",         // MC-only signal PDF name used in PerformMCFit
            "signal",           // Generic signal name
            "model"             // Another common total name
        };
        
        for (const auto& pdfName : pdfNames) {
            pdf_ = dynamic_cast<RooAbsPdf*>(ws_->pdf(pdfName.c_str()));
            if (pdf_) {
                std::cout << "[PlotManager] ✓ Found PDF: " << pdfName << std::endl;
                break;
            }
        }
        
        if (!pdf_) {
            // Fallback: choose the first available PDF in workspace
            RooArgSet pdfs = ws_->allPdfs();
            if (pdfs.getSize() > 0) {
                RooFIter it = pdfs.fwdIterator();
                if (RooAbsArg* arg = it.next()) {
                    pdf_ = dynamic_cast<RooAbsPdf*>(arg);
                    if (pdf_) {
                        std::cout << "[PlotManager] Fallback: Using first available PDF: " << pdf_->GetName() << std::endl;
                    }
                }
            }
            if (!pdf_) {
                std::cout << "[PlotManager] Warning: No PDF found. Raw plots only." << std::endl;
                std::cout << "[PlotManager] Available PDFs in workspace:" << std::endl;
                pdfs.Print();
            }
        }
    }
    
    // Load fit result (optional)
    fitResult_ = dynamic_cast<RooFitResult*>(file_->Get(opt_.fitResultName.c_str()));
    if (!fitResult_) {
        std::cout << "[PlotManager] Warning: No fit result found. Limited functionality." << std::endl;
    } else {
        std::cout << "[PlotManager] ✓ Loaded fit result: " << opt_.fitResultName << std::endl;
    }
    
    return true;
}

void EnhancedPlotManager::SetPlotStyle(const std::string& style) {
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

std::string EnhancedPlotManager::GetOutputFileName(const std::string& baseName, const std::string& suffix) {
    std::string outputName = outputDir_ + "/" + baseName;
    if (!suffix.empty()) {
        outputName += "_" + suffix;
    }
    outputName += ".pdf";
    return outputName;
}

TLegend* EnhancedPlotManager::CreateLegend(double x1, double y1, double x2, double y2) {
    TLegend* legend = new TLegend(x1, y1, x2, y2);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextFont(plotStyle_.labelFont);
    legend->SetTextSize(plotStyle_.labelSize * 0.9);
    return legend;
}

void EnhancedPlotManager::AddLabels(TPad* pad) {
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
    collisionText += ", #sqrt{s_{NN}} = 5.32 TeV";
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

void EnhancedPlotManager::PrintSummary() const {
    std::cout << "\n=== PlotManager Summary ===" << std::endl;
    std::cout << "File: " << fileDir_ << "/" << filename_ << std::endl;
    std::cout << "Workspace: " << opt_.wsName << std::endl;
    std::cout << "Dataset: " << opt_.datasetName;
    if (dataset_) std::cout << " (" << dataset_->sumEntries() << " entries)";
    std::cout << std::endl;
    std::cout << "Variable: " << opt_.massVar << " [" << opt_.massMin << ", " << opt_.massMax << "]" << std::endl;
    std::cout << "PDF available: " << (pdf_ ? "Yes" : "No") << std::endl;
    std::cout << "Fit result available: " << (fitResult_ ? "Yes" : "No") << std::endl;
    std::cout << "Valid: " << (isValid_ ? "Yes" : "No") << std::endl;
    std::cout << "========================" << std::endl;
}

void EnhancedPlotManager::PrintWorkspaceContents() const {
    if (!ws_) {
        std::cout << "No workspace loaded!" << std::endl;
        return;
    }
    
    std::cout << "\n=== Workspace Contents ===" << std::endl;
    std::cout << "Workspace name: " << ws_->GetName() << std::endl;
    
    std::cout << "\nDatasets:" << std::endl;
    auto dataList = ws_->allData();
    for (auto* data : dataList) {
        if (data) std::cout << "  - " << data->GetName() << std::endl;
    }
    
    std::cout << "\nPDFs:" << std::endl;
    RooArgSet pdfs = ws_->allPdfs();
    pdfs.Print();
    
    std::cout << "\nVariables:" << std::endl;
    RooArgSet vars = ws_->allVars();
    vars.Print();
    std::cout << "=========================" << std::endl;
}

// Enhanced plotting methods
bool EnhancedPlotManager::DrawRawDistribution(const std::string& outputName) {
    if (!dataset_ || !var_) {
        std::cerr << "[PlotManager] Error: Dataset or variable not available" << std::endl;
        return false;
    }

    std::cout << "[PlotManager] Drawing raw distribution..." << std::endl;
    
    TCanvas* canvas = new TCanvas("canvas_raw", "Raw Distribution", plotStyle_.canvasWidth, plotStyle_.canvasHeight);
    SetupCanvas(canvas);
    
    RooPlot* frame = var_->frame(RooFit::Range("analysis"), RooFit::Title(""));
    dataset_->plotOn(frame, RooFit::MarkerStyle(20), RooFit::MarkerSize(0.8));
    
    std::string xTitle = opt_.massVar;
    if (isDstar_) {
        xTitle = "#Delta m = m(K#pi#pi) - m(K#pi) [GeV/c^{2}]";
    } else {
        xTitle = "m(K#pi) [GeV/c^{2}]";
    }
    
    frame->SetXTitle(xTitle.c_str());
    frame->SetYTitle("Events / bin");
    frame->Draw();
    
    // Add labels
    AddLabels(dynamic_cast<TPad*>(gPad));
    
    // Save plot
    createDir(Form("%s/", outputDir_.c_str()));
    std::string filename = outputName.empty() ? 
        GetOutputFileName("RawDist_" + opt_.GetBinName()) : 
        GetOutputFileName(outputName);
    canvas->SaveAs(filename.c_str());
    
    std::cout << "[PlotManager] ✓ Raw distribution saved: " << filename << std::endl;
    
    delete canvas;
    return true;
}

bool EnhancedPlotManager::DrawFittedModel(bool drawPull, const std::string& outputName) {
    // Check each component individually for better debugging
    bool hasDataset = (dataset_ != nullptr);
    bool hasPdf = (pdf_ != nullptr);
    bool hasVar = (var_ != nullptr);
    
    std::cout << "[PlotManager] DrawFittedModel status check:" << std::endl;
    std::cout << "  Dataset available: " << (hasDataset ? "YES" : "NO") << std::endl;
    std::cout << "  PDF available: " << (hasPdf ? "YES" : "NO") << std::endl;
    std::cout << "  Variable available: " << (hasVar ? "YES" : "NO") << std::endl;
    
    if (!hasDataset) {
        std::cerr << "[PlotManager] Error: Dataset not available for fitted model plotting" << std::endl;
        return false;
    }
    
    if (!hasVar) {
        std::cerr << "[PlotManager] Error: Variable not available for fitted model plotting" << std::endl;
        return false;
    }
    
    if (!hasPdf) {
        std::cerr << "[PlotManager] Error: PDF not available for fitted model plotting" << std::endl;
        std::cout << "[PlotManager] Note: This might happen if the PDF search failed in ValidateObjects()" << std::endl;
        std::cout << "[PlotManager] Suggestion: Try using DrawRawDistribution() instead" << std::endl;
        return false;
    }

    std::cout << "[PlotManager] Drawing fitted model..." << std::endl;
    
    // Canvas setup matching PlotManager_backup.h style
    TCanvas* canvas = new TCanvas("canvas", "", 1200, 800);
    TPad* mainPad = new TPad("mainPad", "", 0.0, 0.3, 0.7, 1.0);
    TPad* pullPad = new TPad("pullPad", "", 0.0, 0.0, 0.7, 0.3);
    TPad* paramPad = new TPad("paramPad", "", 0.7, 0.0, 1.0, 1.0);

    // Set margins matching backup style
    mainPad->SetBottomMargin(0.00);
    mainPad->SetTopMargin(0.12);
    mainPad->SetLeftMargin(0.1);
    pullPad->SetTopMargin(0.01);
    pullPad->SetBottomMargin(0.3);

    mainPad->Draw();
    if (drawPull) pullPad->Draw();
    paramPad->Draw();
    
    // Main plot
    mainPad->cd();
    RooPlot* frame = var_->frame(RooFit::Bins(60), RooFit::Title(""), RooFit::Range("analysis"));
    
    // 1. Plot Data first
    dataset_->plotOn(frame, RooFit::Name("datapoints"), RooFit::MarkerStyle(kFullCircle), RooFit::MarkerSize(0.8));

    // 2. Identify and Plot Components with fill styles (matching backup style)
    RooAbsPdf* signalPdf = ExtractComponent("sig");
    RooAbsPdf* swappedPdf = ExtractComponent("Swap");
    // Try multiple common background name patterns
    RooAbsPdf* combinatorialPdf = ExtractComponent("bkg");
    if (!combinatorialPdf) combinatorialPdf = ExtractComponent("background");
    if (!combinatorialPdf) combinatorialPdf = ExtractComponent("phenom");
    if (!combinatorialPdf) combinatorialPdf = ExtractComponent("exp");
    if (!combinatorialPdf) combinatorialPdf = ExtractComponent("threshold");
    if (!combinatorialPdf) combinatorialPdf = ExtractComponent("poly");
    if (!combinatorialPdf) combinatorialPdf = ExtractComponent("cheb");
    if (!combinatorialPdf) combinatorialPdf = ExtractComponent("dst");
    
    // Plot Signal Component (filled area with dotted outline)
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

    // 3. Plot Total Fit Line (Solid Red)
    pdf_->plotOn(frame, RooFit::Name("model"), RooFit::LineColor(kRed), RooFit::LineWidth(2),
                 RooFit::NormRange("analysis"), RooFit::Range("analysis"));

    // 4. Re-plot Data Points on top
    dataset_->plotOn(frame, RooFit::Name("datapoints_top"), RooFit::MarkerStyle(kFullCircle), RooFit::MarkerSize(0.8));

    // Set axis properties matching backup style
    frame->GetYaxis()->SetTitleOffset(1.2);
    frame->GetYaxis()->SetTitle(Form("Entries / (%.3f GeV/c^{2})", var_->getBinWidth(0)));
    // frame->GetYaxis()->SetLabelSize(0.0);
    frame->SetTitle("");
    
    // --- Legend matching backup style ---
    TLegend* legend = new TLegend(0.65, 0.60, 0.93, 0.88);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.04);

    legend->AddEntry(frame->findObject("datapoints"), "Data", "PE");
    legend->AddEntry(frame->findObject("model"), "Fit", "L");

    if (signalPdf) {
        isDstar_ ? legend->AddEntry(frame->findObject("signal"), "D^{*} Signal", "F") 
                 : legend->AddEntry(frame->findObject("signal"), "D^{0}+#bar{D}^{0} Signal", "F");
    }
    if (swappedPdf) {
        legend->AddEntry(frame->findObject("swapped"), "K-#pi swapped", "F");
    }
    if (combinatorialPdf) {
        legend->AddEntry(frame->findObject("combinatorial"), "Combinatorial", "L");
    }
    
    // Set axis titles
    std::string xTitle = isDstar_ ? 
        "#Delta m = m(K#pi#pi) - m(K#pi) [GeV/c^{2}]" : 
        "m(K#pi) [GeV/c^{2}]";
    frame->SetXTitle(xTitle.c_str());
    frame->SetYTitle("Events / (0.4 MeV/c^{2})");
    frame->Draw();
    legend->Draw();
    
    // --- CMS Labels matching backup style ---
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

    // Physics labels matching backup positions
    double x_pos = 0.13;
    double y_pos = 0.80;
    latex.DrawLatex(x_pos, y_pos, opt_.pTLegend.c_str());
    latex.DrawLatex(x_pos, y_pos - 0.07, opt_.yLegend.c_str());
    latex.DrawLatex(x_pos, y_pos - 0.14, opt_.centLegend.c_str());
    
    // Draw pull plot if requested
    if (drawPull && pullPad) {
        DrawPullFrame(frame, pullPad);
    }
    
    // Draw parameter pad
    DrawParameterPad(paramPad);
    
    // Save plot
    createDir(Form("%s/", outputDir_.c_str()));
    std::string filename = outputName.empty() ? 
        GetOutputFileName("FittedModel_" + opt_.GetBinName()) : 
        GetOutputFileName(outputName);
    canvas->SaveAs(filename.c_str());
    
    std::cout << "[PlotManager] ✓ Fitted model saved: " << filename << std::endl;
    
    delete canvas;
    return true;
}

void EnhancedPlotManager::SetupCanvas(TCanvas* canvas) {
    canvas->SetLeftMargin(plotStyle_.leftMargin);
    canvas->SetRightMargin(plotStyle_.rightMargin);
    canvas->SetTopMargin(plotStyle_.topMargin);
    canvas->SetBottomMargin(plotStyle_.bottomMargin);
}

void EnhancedPlotManager::DrawPullFrame(RooPlot* frame, TPad* pullPad) {
    pullPad->cd();
    RooHist* pullHist = frame->pullHist("datapoints", "model");
    RooPlot* pullFrame = var_->frame(RooFit::Title(" "), RooFit::Range("analysis"));
    pullFrame->addPlotable(static_cast<RooPlotable*>(pullHist), "P");
    pullFrame->SetMinimum(-5);
    pullFrame->SetMaximum(5);
    pullFrame->GetYaxis()->SetTitle("");
    pullFrame->GetYaxis()->SetTitleOffset(0.3);
    pullFrame->GetYaxis()->SetTitleSize(0.1);
    pullFrame->GetYaxis()->SetNdivisions(505);
    // pullFrame->GetYaxis()->SetLabelSize(0.0);
    pullFrame->GetXaxis()->SetTitleSize(0.1);
    pullFrame->GetXaxis()->SetLabelSize(0.1);
    pullFrame->Draw();
    TLine* line0 = new TLine(var_->getMin("analysis"), 0, var_->getMax("analysis"), 0);
    line0->SetLineStyle(2);
    line0->Draw();
}

RooAbsPdf* EnhancedPlotManager::ExtractComponent(const std::string& namePattern) {
    if (!ws_) return nullptr;
    
    RooArgSet pdfs = ws_->allPdfs();
    RooFIter iter = pdfs.fwdIterator();
    RooAbsPdf* foundPdf = nullptr;
    
    while (RooAbsArg* pdfArg = iter.next()) {
        RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(pdfArg);
        if (pdfComponent) {
            std::string pdfName = pdfComponent->GetName();
            if (pdfName.find(namePattern) != std::string::npos) {
                foundPdf = pdfComponent;
                break;
            }
        }
    }
    
    return foundPdf;
}

void EnhancedPlotManager::DrawParameterPad(TPad* paramPad) {
    paramPad->cd();
    
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.03);
    
    if (fitResult_) {
        latex.DrawLatex(0.1, 0.9, Form("STATUS %s : %d, %s : %d", 
                                      fitResult_->statusLabelHistory(0), 
                                      fitResult_->statusCodeHistory(0),
                                      fitResult_->statusLabelHistory(1), 
                                      fitResult_->statusCodeHistory(1)));

        const RooArgList& params = fitResult_->floatParsFinal();
        for (int i = 0; i < params.getSize(); ++i) {
            RooRealVar* param = (RooRealVar*)params.at(i);
            latex.DrawLatex(0.1, 0.85 - i * 0.05, 
                          Form("%s = %.5f #pm %.5f", param->GetName(), param->getVal(), param->getError()));
        }
    } else {
        latex.DrawLatex(0.1, 0.9, "No fit result available");
    }
}

void EnhancedPlotManager::CleanupMemory() {
    // ROOT objects are owned by TFile or RooWorkspace
    // std::unique_ptr<TFile> will handle cleanup automatically
}

#endif // ENHANCED_PLOTMANAGER_H
