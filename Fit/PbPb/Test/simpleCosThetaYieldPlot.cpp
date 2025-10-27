#include "RooRealVar.h"
#include "RooWorkspace.h"
#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TParameter.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TString.h"

#include <iostream>
#include <string>
#include <vector>

void simpleCosThetaYieldPlot() {
    gStyle->SetOptStat(0);
    
    const std::vector<std::pair<double, double>> ptBins = {
        {5.0, 7.0},
        {7.0, 10.0}
    };

    const std::vector<std::pair<double, double>> cosBins = {
        {0.0, 0.2},
        {0.2, 0.4},
        {0.4, 0.6},
        {0.6, 0.8},
        {0.8, 1.0}
    };

    const std::vector<std::pair<int, int>> centralityBins = {
        {0, 10},
        {30, 50}
    };

    const std::vector<double> cosEdges = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
    const std::string baseDir = "/home/jun502s/DstarAna/DStarAnalysis/Fit/PbPb/results/DStar_PbPb_Sep03_pTGrand_1.5_EtaGrand_1.6_pTD2_0.3_pow1/Data";

    for (size_t centIdx = 0; centIdx < centralityBins.size(); ++centIdx) {
        const auto &cent = centralityBins[centIdx];
        
        for (size_t ptIdx = 0; ptIdx < ptBins.size(); ++ptIdx) {
            const auto &pt = ptBins[ptIdx];
            
            auto *hist = new TH1D(
                Form("hYield_pt_%zu_cent_%zu", ptIdx, centIdx),
                Form("Yield vs |cos#vartheta*|;|cos#vartheta*|;Yield, p_{T} %.0f-%.0f GeV/c, Cent %d-%d%%", 
                     pt.first, pt.second, cent.first, cent.second),
                static_cast<int>(cosEdges.size() - 1),
                cosEdges.data());
            
            hist->Sumw2(true);
            hist->SetMarkerStyle(20);
            hist->SetMarkerSize(1.1);
            hist->SetMarkerColor(kBlue + 1);
            hist->SetLineColor(kBlue + 1);
            hist->SetLineWidth(2);
            hist->GetXaxis()->SetTitle("|cos#vartheta*|");
            hist->GetYaxis()->SetTitle("Yield");
            hist->GetXaxis()->SetTitleSize(0.045);
            hist->GetXaxis()->SetLabelSize(0.04);
            hist->GetYaxis()->SetTitleSize(0.045);
            hist->GetYaxis()->SetLabelSize(0.04);

            auto *canvas = new TCanvas(Form("cYield_pt_%zu_cent_%zu", ptIdx, centIdx),
                                       Form("Yield distribution, p_{T} %.0f-%.0f GeV/c, Cent %d-%d%%", 
                                            pt.first, pt.second, cent.first, cent.second),
                                       800, 600);
            canvas->SetLeftMargin(0.14);
            canvas->SetBottomMargin(0.13);
            canvas->SetRightMargin(0.05);
            canvas->SetTopMargin(0.08);

            for (size_t cosIdx = 0; cosIdx < cosBins.size(); ++cosIdx) {
                const auto &cosBin = cosBins[cosIdx];
                
                // 파일 경로 구성
                std::string cosStr = Form("cos_%.1f_%.1f", cosBin.first, cosBin.second);
                if (cosBin.first < 0) {
                    cosStr = Form("cos_m%.1f_m%.1f", -cosBin.first, -cosBin.second);
                }
                
                std::string fileName = Form("DStar_DCA_Yield_pT_%.0f_%.0f_%s_cent_%d_%d.root",
                                           pt.first, pt.second, cosStr.c_str(), cent.first, cent.second);
                
                std::string filePath = baseDir + "/dcahist/" + fileName;
                
                std::cout << "Trying to open: " << filePath << std::endl;
                
                TFile *file = TFile::Open(filePath.c_str(), "READ");
                if (!file || file->IsZombie()) {
                    std::cout << "Failed to open file: " << filePath << std::endl;
                    continue;
                }
                
                // nsig 값 추출
                RooWorkspace *ws = dynamic_cast<RooWorkspace *>(file->Get("ws_DStar"));
                if (!ws) {
                    std::cout << "Workspace not found in " << filePath << std::endl;
                    file->Close();
                    continue;
                }
                
                RooRealVar *nsig = ws->var("nsig");
                if (!nsig) {
                    std::cout << "nsig not found in " << filePath << std::endl;
                    file->Close();
                    continue;
                }
                
                double yield = nsig->getVal();
                double error = nsig->getError();
                
                std::cout << Form("pT %.0f-%.0f, |cos| %.1f-%.1f, Cent %d-%d%% : yield %.2f ± %.2f",
                                  pt.first, pt.second, cosBin.first, cosBin.second, cent.first, cent.second,
                                  yield, error) << std::endl;
                
                const int binIndex = static_cast<int>(cosIdx) + 1;
                hist->SetBinContent(binIndex, yield);
                hist->SetBinError(binIndex, error);
                
                file->Close();
            }

            canvas->cd();
            hist->Draw("E1P");
            TLegend *leg = new TLegend(0.55, 0.70, 0.88, 0.88);
            leg->SetBorderSize(0);
            leg->SetFillStyle(0);
            leg->SetTextSize(0.035);
            leg->AddEntry(hist, "Raw nsig", "lep");
            leg->Draw();

            const char *centSuffix = Form("_cent_%d_%d", cent.first, cent.second);
            canvas->SaveAs(Form("yield_vs_costheta_pt_%.0f_%.0f%s.png", 
                               pt.first, pt.second, centSuffix));
            canvas->SaveAs(Form("yield_vs_costheta_pt_%.0f_%.0f%s.pdf", 
                               pt.first, pt.second, centSuffix));
        }
    }
    
    std::cout << "Analysis completed! Check the generated plots." << std::endl;
}
