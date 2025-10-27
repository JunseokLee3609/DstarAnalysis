#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include <vector>
#include <iostream>

// 히스토그램을 피팅하고 rho_00과 그 오차를 반환하는 함수
std::pair<double, double> fit_and_extract_rho00(TH1* histogram, double initial_guess, TCanvas* c = nullptr) {
    if (!histogram || histogram->GetEntries() == 0) {
        std::cerr << "Warning: Histogram is empty or null." << std::endl;
        return {-999, -999};
    }

    TF1 *fitFunc = new TF1("fitFunc", "[0]*(1 - [1] + (3*[1] - 1)*x*x)", -1, 1);
    fitFunc->SetParName(0, "N_{0}");
    fitFunc->SetParName(1, "#rho_{00}");

    fitFunc->SetParameter(0, histogram->GetMaximum());
    fitFunc->SetParameter(1, initial_guess);
    fitFunc->SetParLimits(0, 0, histogram->GetMaximum() * 10);
    fitFunc->SetParLimits(1, 0.0, 1.0);

    TFitResultPtr fitResult = histogram->Fit(fitFunc, "S Q R");

    // --- 추가: 히스토그램과 fit 함수 그리기 ---
    if (c) {
        c->cd();
        histogram->SetMarkerStyle(20);
        histogram->SetMarkerSize(1.2);
        histogram->SetLineWidth(2);
        // y축 최소값을 최대값의 0.2로 설정
        double yMax = histogram->GetMaximum();
        histogram->GetYaxis()->SetRangeUser(yMax * 0.2, yMax * 1.2);
        histogram->Draw("E");
        fitFunc->SetLineColor(kRed+1);
        fitFunc->SetLineWidth(2);
        fitFunc->Draw("Same");
        TLatex latex;
        latex.SetNDC();
        latex.SetTextSize(0.045);
        latex.DrawLatex(0.18, 0.85, Form("Fit: N_{0}[1-#rho_{00}+cos^{2}#theta(3#rho_{00}-1)]"));
        if (fitResult->IsValid()) {
            latex.DrawLatex(0.18, 0.78, Form("#rho_{00} = %.3f #pm %.3f", fitFunc->GetParameter(1), fitFunc->GetParError(1)));
        }
        c->Update();
    }
    // --- 끝 ---

    if (fitResult->IsValid()) {
        double rho00 = fitFunc->GetParameter(1);
        double rho00_err = fitFunc->GetParError(1);
        delete fitFunc;
        return {rho00, rho00_err};
    } else {
        std::cerr << "Warning: Fit failed for histogram " << histogram->GetName() << std::endl;
        delete fitFunc;
        return {-999, -999};
    }
}

// 메인 분석 함수
void fit_histograms_and_plot() {
    gStyle->SetOptStat(0);
    gStyle->SetErrorX(0.5);

    // --- 입력 파일 열기 ---
    TFile *inputPR = TFile::Open("output_prompt_aug21.root", "READ");
    TFile *inputNP = TFile::Open("output_nonprompt_aug04.root", "READ");
    if (!inputPR || inputPR->IsZombie()) {
        std::cerr << "Error: Cannot open input file 'input_histograms.root'.\n";
        std::cerr << "Please run 'create_dummy_histograms.C' first." << std::endl;
        return;
    }

    // --- pT 구간 설정 ---
    const int n_pt_bins_prompt = 3;
    int pt_bins_prompt_low[] = {5, 7, 10};
    int pt_bins_prompt_high[] = {7, 10, 20};

    const int n_pt_bins_nonprompt = 3;
    double pt_bins_nonprompt_low[] = {5.0, 7.0, 10.0};
    double pt_bins_nonprompt_high[] = {7.0, 10.0, 20.0};

    std::vector<double> pt_centers_p, pt_half_widths_p, rho00_values_p, rho00_stat_errors_p, rho00_syst_errors_p;
    std::vector<double> pt_centers_np, pt_half_widths_np, rho00_values_np, rho00_stat_errors_np, rho00_syst_errors_np;

    // --- Prompt (c -> D*+) 히스토그램 피팅 ---
    std::cout << "Fitting prompt histograms..." << std::endl;
    for (int i = 0; i < n_pt_bins_prompt; ++i) {
        TString hname = TString::Format("/RAWDStarSpinAlignAnalyzer/cosTheta_pt%dto%d", pt_bins_prompt_low[i],pt_bins_prompt_high[i]);
        TH1D *hist = (TH1D*)inputPR->Get(hname);
        if (!hist) {
            std::cerr << "Error: Cannot find histogram '" << hname << "' in the input file." << std::endl;
            continue;
        }
        // --- 추가: pt bin별로 캔버스 생성 및 저장 ---
        TCanvas* c = new TCanvas(Form("c_prompt_%d", i), Form("Prompt Fit %d", i), 600, 500);
        auto result = fit_and_extract_rho00(hist, 0.33, c); // 초기 추측값 0.3
        c->SaveAs(Form("fit_prompt_pt%dto%d.png", pt_bins_prompt_low[i], pt_bins_prompt_high[i]));
        delete c;
        // --- 끝 ---
        if (result.first > -999) {
            pt_centers_p.push_back((pt_bins_prompt_low[i] + pt_bins_prompt_high[i]) / 2.0);
            pt_half_widths_p.push_back((pt_bins_prompt_high[i] - pt_bins_prompt_low[i]) / 2.0);
            rho00_values_p.push_back(result.first);
            rho00_stat_errors_p.push_back(result.second);
            rho00_syst_errors_p.push_back(result.first * 0.08);
            std::cout << "  - " << hname << ": rho_00 = " << result.first << " +/- " << result.second << std::endl;
        }
    }

    // --- Non-Prompt (b -> D*+) 히스토그램 피팅 ---
    std::cout << "Fitting non-prompt histograms..." << std::endl;
    for (int i = 0; i < n_pt_bins_nonprompt; ++i) {
        TString hname = TString::Format("/RAWDStarSpinAlignAnalyzer_nonprompt/cosTheta_pt%dto%d", pt_bins_prompt_low[i],pt_bins_prompt_high[i]);
        TH1F *hist = (TH1F*)inputNP->Get(hname);
        if (!hist) {
            std::cerr << "Error: Cannot find histogram '" << hname << "' in the input file." << std::endl;
            continue;
        }
        // --- 추가: pt bin별로 캔버스 생성 및 저장 ---
        TCanvas* c = new TCanvas(Form("c_nonprompt_%d", i), Form("NonPrompt Fit %d", i), 600, 500);
        auto result = fit_and_extract_rho00(hist, 0.45, c); // 초기 추측값 0.45
        c->SaveAs(Form("fit_nonprompt_pt%dto%d.png", pt_bins_prompt_low[i], pt_bins_prompt_high[i]));
        delete c;
        // --- 끝 ---
        if (result.first > -999) {
            pt_centers_np.push_back((pt_bins_nonprompt_low[i] + pt_bins_nonprompt_high[i]) / 2.0);
            pt_half_widths_np.push_back((pt_bins_nonprompt_high[i] - pt_bins_nonprompt_low[i]) / 2.0);
            rho00_values_np.push_back(result.first);
            rho00_stat_errors_np.push_back(result.second);
            rho00_syst_errors_np.push_back(result.first * 0.06);
            std::cout << "  - " << hname << ": rho_00 = " << result.first << " +/- " << result.second << std::endl;
        }
    }

    inputPR->Close();
    inputNP->Close();

    // --- 최종 플롯 그리기 (이전 코드와 거의 동일) ---
    TCanvas *c_final = new TCanvas("c_final", "rho_00 vs pT", 800, 700);
    // ... (이하 플롯을 그리는 부분은 이전 답변의 코드와 동일합니다) ...
    c_final->SetLeftMargin(0.15);
    c_final->SetBottomMargin(0.15);

    auto gPrompt = new TGraphAsymmErrors(pt_centers_p.size(), &pt_centers_p[0], &rho00_values_p[0], &pt_half_widths_p[0], &pt_half_widths_p[0], &rho00_stat_errors_p[0], &rho00_stat_errors_p[0]);
    auto gNonPrompt = new TGraphAsymmErrors(pt_centers_np.size(), &pt_centers_np[0], &rho00_values_np[0], &pt_half_widths_np[0], &pt_half_widths_np[0], &rho00_stat_errors_np[0], &rho00_stat_errors_np[0]);
    
    auto gPromptSys = new TGraphAsymmErrors(pt_centers_p.size(), &pt_centers_p[0], &rho00_values_p[0], &pt_half_widths_p[0], &pt_half_widths_p[0], &rho00_syst_errors_p[0], &rho00_syst_errors_p[0]);
    auto gNonPromptSys = new TGraphAsymmErrors(pt_centers_np.size(), &pt_centers_np[0], &rho00_values_np[0], &pt_half_widths_np[0], &pt_half_widths_np[0], &rho00_syst_errors_np[0], &rho00_syst_errors_np[0]);

    // 스타일 설정...
    gPrompt->SetMarkerStyle(kFullSquare);
    gPrompt->SetMarkerColor(kRed+1);
    gPrompt->SetLineColor(kRed+1);
    // gPromptSys->SetFillColorAlpha(kRed+1, 0.35);
    // gPromptSys->SetFillStyle(1001);
    // gPromptSys->SetMarkerSize(0);

    gNonPrompt->SetMarkerStyle(kFullCircle);
    gNonPrompt->SetMarkerColor(kAzure-3);
    gNonPrompt->SetLineColor(kAzure-3);
    // gNonPromptSys->SetFillColorAlpha(kAzure-3, 0.5);
    // gNonPromptSys->SetFillStyle(1001);
    // gNonPromptSys->SetMarkerSize(0);
    
    TH1F *h_axis = new TH1F("h_axis", "", 1, 4, 20);
    h_axis->SetYTitle("#rho_{00}");
    h_axis->SetXTitle("p_{T} (GeV/c)");
    h_axis->GetYaxis()->SetRangeUser(0.15, 0.65);
    h_axis->GetYaxis()->SetTitleSize(0.05);
    h_axis->GetXaxis()->SetTitleSize(0.05);
    h_axis->GetYaxis()->SetTitleOffset(1.2);
    h_axis->Draw();

    // 이론 밴드...
    // auto gTheoryPrompt = new TGraphAsymmErrors(1);
    // gTheoryPrompt->SetPoint(0, 12, 0.32); gTheoryPrompt->SetPointError(0, 8, 8, 0.015, 0.015);
    // gTheoryPrompt->SetFillColorAlpha(kRed-7, 0.8); gTheoryPrompt->SetFillStyle(1001);
    // gTheoryPrompt->Draw("E2 same");
    
    // auto gTheoryNonPrompt = new TGraphAsymmErrors(1);
    // gTheoryNonPrompt->SetPoint(0, 12, 0.43); gTheoryNonPrompt->SetPointError(0, 8, 8, 0.02, 0.02);
    // gTheoryNonPrompt->SetFillColorAlpha(kAzure-9, 0.8); gTheoryNonPrompt->SetFillStyle(1001);
    // gTheoryNonPrompt->Draw("E2 same");
    
    // gNonPromptSys->Draw("E2 same");
    // gPromptSys->Draw("E2 same");
    gNonPrompt->Draw("PZ same");
    gPrompt->Draw("PZ same");

    // 텍스트 및 범례...
    TLatex latex;
    latex.SetNDC(); latex.SetTextFont(42);
    latex.SetTextSize(0.05); latex.DrawLatex(0.2, 0.82, "#bf{CMS}");
    latex.SetTextSize(0.04); latex.DrawLatex(0.2, 0.76, "pp, #sqrt{s} = 5.36 TeV");
    latex.DrawLatex(0.65, 0.82, "|y| < 0.8");

    TLegend *leg = new TLegend(0.22, 0.22, 0.45, 0.35);
    leg->SetBorderSize(0); leg->SetFillStyle(0);
    // leg->AddEntry((TObject*)nullptr, "Data", "");
    // leg->AddEntry(gPrompt, "c #rightarrow D^{*+}", "p");
    // leg->AddEntry(gNonPrompt, "b #rightarrow D^{*+}", "p");
    leg->AddEntry((TObject*)nullptr, "PYTHIA 8", "");
    leg->AddEntry(gPrompt, "c #rightarrow D^{*+}", "f");
    leg->AddEntry(gNonPrompt, "c #rightarrow D^{*+}", "f");
    // leg->AddEntry(gTheoryNonPrompt, "b #rightarrow D^{*+}", "f");
    leg->Draw();
    c_final->SaveAs("prompt.png");
}
