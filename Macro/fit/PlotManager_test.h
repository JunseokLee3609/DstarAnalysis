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
#include "TParameter.h" // Add this include

class PlotManager {
public:
    PlotManager(FitOpt& opt, const std::string& inputDir, const std::string& outputFile, const std::string& outputDir);
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
    FitOpt opt_;
    TFile* file_;
    RooDataSet* dataset_;
    RooAbsPdf* pdf_;
    RooRealVar* var_;
    RooFitResult* fitResult_;
    RooWorkspace* ws_;
    double nsigPureVal_ = -1.0; // Initialize with a default value
    double nsigPureErr_ = -1.0; // Initialize with a default value
    int colorIndex = 1; // Start with a different color for each component
    std::vector<std::pair<std::string, int>> legendEntries; // 컴포넌트 이름과 색상 저장

    void DrawPullFrame(RooPlot* frame);
    RooAbsPdf* ExtractComponent(const std::string& namePattern);
    void PlotComponents(RooPlot* frame, RooAbsPdf* Pdf);
    void DrawParameterPad();
};
PlotManager::PlotManager(FitOpt& opt, const std::string& inputDir, const std::string& outputFile, const std::string& outputDir)
    : opt_(opt),fileDir_(inputDir), filename_(outputFile),outputDir_(outputDir), datasetName_(opt.datasetName), pdfName_(opt.pdfName), varName_(opt.massVar), fitResultName_(opt.fitResultName), wsName_(opt.wsName), plotName_(opt.plotName), file_(nullptr), dataset_(nullptr), pdf_(nullptr), var_(nullptr), fitResult_(nullptr) {
    file_ = TFile::Open(Form("%s/%s",fileDir_.c_str(), filename_.c_str()));
    if(file_) cout << "File Opened :: " << Form("%s/%s",fileDir_.c_str(), filename_.c_str()) <<endl; // Corrected path
    if (!file_ || file_->IsZombie()) {
        std::cerr << "Error: Failed to open file " << filename_ << std::endl;
        return;
    }
    ws_ = (RooWorkspace*)file_->Get(wsName_.c_str());
    if (!ws_) {
        std::cerr << "Error: Failed to load ws from file" << std::endl;
        // Don't close file yet, might need it for TParameters
        // file_->Close();
        // return; // Continue to try reading TParameters
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
        std::cerr << "Warning: Failed to load fit result from file. Parameter pad might be incomplete." << std::endl;
        // Don't return yet, try reading TParameters
    }

    // --- Read nSigPure parameters ---
    TParameter<double>* pureYieldValParam = dynamic_cast<TParameter<double>*>(file_->Get("nSigPure_Value"));
    TParameter<double>* pureYieldErrParam = dynamic_cast<TParameter<double>*>(file_->Get("nSigPure_Error"));

    if (pureYieldValParam) {
        nsigPureVal_ = pureYieldValParam->GetVal();
        std::cout << "INFO: Read nSigPure_Value = " << nsigPureVal_ << std::endl;
    } else {
        std::cerr << "Warning: Could not read nSigPure_Value from file." << std::endl;
    }

    if (pureYieldErrParam) {
        nsigPureErr_ = pureYieldErrParam->GetVal();
         std::cout << "INFO: Read nSigPure_Error = " << nsigPureErr_ << std::endl;
    } else {
        std::cerr << "Warning: Could not read nSigPure_Error from file." << std::endl;
    }
    // --- End Read nSigPure parameters ---

    // Now you can close the file if ws_ is also null, or handle errors appropriately
    if (!ws_ && !fitResult_ && !pureYieldValParam && !pureYieldErrParam) {
         std::cerr << "Error: Failed to load essential components (ws, fitResult, nSigPure) from file." << std::endl;
         file_->Close();
         return;
    }
    // Note: file_ remains open until PlotManager is destroyed or explicitly closed.
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
RooAbsPdf* PlotManager::ExtractComponent(const std::string& namePattern) {
    const RooArgSet* components = pdf_->getComponents();
    RooAbsPdf* foundPdf = nullptr;
    
    // Use range-based for loop instead of deprecated iterator
    for (const auto& obj : *components) {
        RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(obj);
        if (pdfComponent && std::string(pdfComponent->GetName()).find(namePattern) != std::string::npos) {
            foundPdf = pdfComponent;
            break;
        }
    }
    
    if (foundPdf) {
        cout << "foundPdf: " << foundPdf->GetName() << endl;
    }
    
    return foundPdf;
}

void PlotManager::DrawFittedModel(bool drawPull) {
    if (!dataset_ || !pdf_ || !var_ || !fitResult_) {
        std::cerr << "Error: Dataset, PDF, variable, or fit result not loaded" << std::endl;
        return;
    }

    // 범례 엔트리 초기화
    legendEntries.clear();
    colorIndex = 1;

    TCanvas* canvas = new TCanvas("canvas", "", 1200, 800);

    // Create pads for main plot, pull distribution, and parameter pad
    TPad* mainPad = new TPad("mainPad", "", 0.0, 0.3, 0.7, 1.0);
    TPad* pullPad = new TPad("pullPad", "", 0.0, 0.0, 0.7, 0.3);
    TPad* paramPad = new TPad("paramPad", "", 0.7, 0.0, 1.0, 1.0);

    mainPad->SetBottomMargin(0.00);
    mainPad->SetTopMargin(0.12);
    mainPad->SetLeftMargin(0.1);
    pullPad->SetTopMargin(0.05);
    pullPad->SetBottomMargin(0.3);

    mainPad->Draw();
    pullPad->Draw();
    paramPad->Draw();

    // Draw the main plot
    mainPad->cd();
    var_->Print("v");
    RooPlot* frame = var_->frame(RooFit::Bins(50),RooFit::Title(""),RooFit::Range("analysis"));
    dataset_->plotOn(frame, RooFit::Name("datapoints"), RooFit::MarkerStyle(kFullCircle), RooFit::MarkerSize(0.8));
    
    // Plot the total PDF
    pdf_->plotOn(frame, RooFit::Name("model"), RooFit::LineColor(kRed), RooFit::LineWidth(2),
    RooFit::NormRange("analysis"), RooFit::Range("analysis"));
    // 데이터 및 전체 모델 항목 추가
    legendEntries.push_back(std::make_pair("Data", kBlack));
    legendEntries.push_back(std::make_pair("Total PDF", kBlue));

    
    // Extract and plot background component
    RooAbsPdf* backgroundPdf = ExtractComponent("tot");
    if (backgroundPdf) {
        PlotComponents(frame, backgroundPdf);
    }
    frame->SetTitle(""); 
    frame->Draw();
    
    // 범례 생성 및 그리기
    TLegend* legend = new TLegend(0.7, 0.65, 0.95, 0.85);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    
    // 데이터 포인트 추가
    // legend->AddEntry(frame->findObject("datapoints"), "Data", "P");
    
    // 전체 모델 추가
    // legend->AddEntry(frame->findObject("model"), "Total Fit", "L");
    legend->AddEntry(frame->findObject("datapoints"), "Data", "PE");
    legend->AddEntry(frame->findObject("model"), "Fit", "L");
    legend->AddEntry(frame->findObject("signal"), "D^{0}+#bar{D}^{0} Signal", "F");
    legend->AddEntry(frame->findObject("swapped"), "K-#pi swapped", "F");
    legend->AddEntry(frame->findObject("bkg_pdf"), "Background", "L");
    
    // 컴포넌트 추가
    // for (const auto& entry : legendEntries) {
    //     // 처음 두 항목(데이터 및 전체 모델)은 이미 추가했으므로 건너뜀
    //     if (entry.first != "Data" && entry.first != "Total PDF") {
    //         TObject* obj = frame->findObject(entry.first.c_str());
    //         if (obj) {
    //             // 이름을 보기 좋게 가공 (선택적)
    //             std::string displayName = entry.first;
    //             // // sig_, bkg_ 등의 접두사 제거
    //             // size_t pos = displayName.find("_");
    //             // if (pos != std::string::npos) {
    //             //     displayName = displayName.substr(pos + 1);
    //             // }
                
    //             legend->AddEntry(obj, displayName.c_str(), "L");
    //         }
    //     }
    // }
    
    legend->Draw();
    TLatex latex;
    latex.SetNDC(); // Use Normalized Device Coordinates
    latex.SetTextFont(42); // Regular font
    latex.SetTextSize(0.04); // Adjust text size

    // CMS Preliminary Text
    latex.SetTextFont(61); // Bold
    latex.SetTextSize(0.05);
    latex.DrawLatex(0.13, 0.90, "CMS"); // Adjust position (Left-Top)
    latex.SetTextFont(52); // Italic
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.20, 0.90, "Work In Progress"); // Adjust position (Left-Top)

    // Collision System Text
    latex.SetTextFont(42); // Regular
    latex.SetTextSize(0.04);
    latex.SetTextAlign(31); // Align right
    // latex.DrawLatex(0.93, 0.95, Form("%s #sqrt{s_{NN}} = %.2f TeV", opt_.collisionSystem.c_str(), opt_.energy)); // Use opt_ members

    // Kinematic Information Text
    latex.SetTextAlign(11); // Align left
    float startY = 0.80; // Starting vertical position
    float stepY = 0.05;  // Vertical step
    int lineCount = 0;

    latex.DrawLatex(0.14, startY - lineCount * stepY, Form("%.1f < p_{T} < %.1f GeV/c", opt_.pTMin, opt_.pTMax)); lineCount++;
    latex.DrawLatex(0.14, startY - lineCount * stepY, Form("|y| < %.1f", 1.0)); lineCount++; // Assuming symmetric rapidity cut
    if (opt_.mvaMin > -1.0) { // Only draw MVA if cut is applied (adjust condition if needed)
        latex.DrawLatex(0.14, startY - lineCount * stepY, Form("MVA > %.3f", opt_.mvaMin)); lineCount++;
    }
    if (opt_.centMin >= 0 && opt_.centMax >= 0) { // Only draw centrality if valid range
        latex.DrawLatex(0.14, startY - lineCount * stepY, Form("Cent : %.0f-%.0f%%", opt_.centMin, opt_.centMax)); lineCount++;
    }

    // Draw the pull distribution
    if (drawPull) {
        pullPad->cd();
        DrawPullFrame(frame);
    }

    // Draw the fitted parameter values
    paramPad->cd();
    DrawParameterPad();

    createDir(Form("%s/",outputDir_.c_str()));
    canvas->SaveAs(Form("%s/%s",outputDir_.c_str(),plotName_.c_str()));

    delete canvas;
    delete legend; // 메모리 누수 방지
}

void PlotManager::DrawPullFrame(RooPlot* frame) {
    RooHist* pullHist = frame->pullHist("datapoints", "model");
    RooPlot* pullFrame = var_->frame(RooFit::Title(" "),RooFit::Range("analysis"));
    pullFrame->addPlotable(pullHist, "P");
    pullFrame->SetMinimum(-5);
    pullFrame->SetMaximum(5);
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
    latex.SetTextSize(0.03); // Adjust text size as needed

    int i = 0; // Counter for vertical positioning

    if (fitResult_) {
        const RooArgList& params = fitResult_->floatParsFinal();
        for (; i < params.getSize(); ++i) {
            RooRealVar* param = dynamic_cast<RooRealVar*>(params.at(i));
            if (param) {
                latex.DrawLatex(0.1, 0.9 - i * 0.05, Form("%s = %.5f #pm %.5f", param->GetName(), param->getVal(), param->getError()));
            }
        }
    } else {
         latex.DrawLatex(0.1, 0.9, "Fit result not available.");
         i = 1; // Increment i to reserve space
    }

    // Draw the nSigPure value if it was successfully read
    if (nsigPureVal_ >= 0 && nsigPureErr_ >= 0) { // Check if values are valid
         latex.DrawLatex(0.1, 0.9 - i * 0.05, Form("N_{sig}^{pure} = %.2f #pm %.2f", nsigPureVal_, nsigPureErr_));
         i++; // Increment counter for the next line
    } else {
         latex.DrawLatex(0.1, 0.9 - i * 0.05, "N_{sig}^{pure} not available.");
         i++;
    }

    // You can add other parameters or information here using the same pattern
    // latex.DrawLatex(0.1, 0.9 - i * 0.05, Form("AnotherParam = ..."));
    // i++;

}
void PlotManager::PlotComponents(RooPlot* frame, RooAbsPdf* Pdf) {
    const RooArgSet* components = Pdf->getComponents();
    if (components->getSize() > 1) {
        // Use range-based for loop instead of deprecated iterator
        for (const auto& obj : *components) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(obj);
            if (pdfComponent) {
                std::string compName = pdfComponent->GetName();
                // cout << compName << endl;
                if(compName=="bkg_D0"){
                    pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("bkg_pdf"),
                    //  RooFit::FillColor(kCyan + 2), 
                     RooFit::LineColor(kBlue), RooFit::LineWidth(2),
                     RooFit::LineStyle(kDotted),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
                }
            }

        }
        for (const auto& obj : *components) {
            RooAbsPdf* pdfComponent = dynamic_cast<RooAbsPdf*>(obj);
            // cout << pdfComponent->GetName() << endl;
            if (pdfComponent) {
                std::string compName = pdfComponent->GetName();
                // if(compName.find("tot") != std::string::npos){ colorIndex++; continue;}
                if(compName=="Swap0"){
                    pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("signal"),
                     RooFit::FillColor(kOrange-9), RooFit::FillStyle(3004),
                     RooFit::DrawOption("F"),
                     RooFit::LineColor(kOrange), RooFit::LineStyle(kDotted), RooFit::LineWidth(3),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
        pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("signal_line"),
                     RooFit::LineColor(kOrange), RooFit::LineStyle(kDotted), RooFit::LineWidth(3),
                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));

                }
                // else if(compName=="Swap1"){
                //         pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("swapped"),
                //                      RooFit::FillColor(kGreen + 3), RooFit::FillStyle(3354),
                //                      RooFit::DrawOption("F"),RooFit::AddTo("bkg_pdf"),
                //                      RooFit::LineColor(kGreen + 3), RooFit::LineStyle(kSolid), RooFit::LineWidth(2),
                //                      RooFit::Range("analysis"));
                //         pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("swapped_line"),
                //                      RooFit::LineColor(kGreen + 2),RooFit::AddTo("bkg_pdf"), RooFit::LineStyle(kSolid), RooFit::LineWidth(2),
                //                      RooFit::NormRange("analysis"), RooFit::Range("analysis"));
                // }
                else if(compName=="Swap1"){
                        pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("swapped"),
                                     RooFit::FillColor(kGreen + 3), RooFit::FillStyle(3354),
                                     RooFit::DrawOption("F"),
                                     RooFit::LineColor(kGreen + 3), RooFit::LineStyle(kSolid), RooFit::LineWidth(2),
                                     RooFit::Range("analysis"));
                        pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("swapped_line"),
                                     RooFit::LineColor(kGreen + 2), RooFit::LineStyle(kSolid), RooFit::LineWidth(2),
                                     RooFit::NormRange("analysis"), RooFit::Range("analysis"));
                }
            //     else if(compName=="signal_pdf"){
            //         pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("swapped"),
            //                      RooFit::FillColor(kGreen + 3), RooFit::FillStyle(3354),
            //                      RooFit::DrawOption("F"),
            //                      RooFit::LineColor(kGreen + 3), RooFit::LineStyle(kSolid), RooFit::LineWidth(2),RooFit::NormRange("analysis"),
            //                      RooFit::Range("analysis"));
            //         // pdf_->plotOn(frame, RooFit::Components(*pdfComponent), RooFit::Name("swapped_line"),
            //         //              RooFit::LineColor(kGreen + 2), RooFit::LineStyle(kSolid), RooFit::LineWidth(2),
            //         //              RooFit::NormRange("analysis"), RooFit::Range("analysis"));
            // }
            
                else{
                // Pdf->plotOn(frame, 
                //           RooFit::Components(*pdfComponent),
                //           RooFit::LineStyle(kDashed),
                //           RooFit::LineColor(colorIndex),
                //           RooFit::Name(compName.c_str()),
                //           RooFit::Range("analysis"),
                //           RooFit::NormRange("analysis"));
                continue;
                
                }
                // 범례 항목 추가
                legendEntries.push_back(std::make_pair(compName, colorIndex));
                colorIndex++;
            }
        }
    } else {
        std::string pdfName = Pdf->GetName();
        Pdf->plotOn(frame, 
                  RooFit::Components(*Pdf),
                  RooFit::LineStyle(kDashed),
                  RooFit::LineColor(colorIndex),
                  RooFit::Name(pdfName.c_str()),
                  RooFit::Range("analysis"),
                  RooFit::NormRange("analysis"));
        
        // 범례 항목 추가
        legendEntries.push_back(std::make_pair(pdfName, colorIndex));
        colorIndex++;
    }
}

#endif // PLOTMANAGER_H
