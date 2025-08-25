#include "../Opt.h"
#include <TFile.h>
#include <TH3D.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <RooStats/SPlot.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooAddPdf.h>
#include <iostream>

double CalSplotweight(FitOpt D0Opt, TH3D* hist_eff, double mvaCut) {
    TFile *FitResultFile = TFile::Open((D0Opt.outputDir + "/" + D0Opt.outputFile).c_str());
    if (!FitResultFile || FitResultFile->IsZombie()) {
        std::cerr << "Error: Could not open fit result file." << std::endl;
        return -1;
    }

    RooWorkspace* ws = (RooWorkspace*)FitResultFile->Get("ws_D0");
    if (!ws) {
        std::cerr << "Error: Could not retrieve workspace." << std::endl;
        FitResultFile->Close();
        return -1;
    }

    RooAddPdf *total_pdf = (RooAddPdf*)ws->pdf("total_pdf");
    RooDataSet *data = (RooDataSet*)FitResultFile->Get("reducedData");
    RooRealVar *nSig = (RooRealVar*)ws->var("nsig");
    RooRealVar *nBkg = (RooRealVar*)ws->var("nbkg");
    RooRealVar *mass = (RooRealVar*)ws->var("mass");

    if (!total_pdf || !data || !nSig || !nBkg || !mass) {
        std::cerr << "Error: Could not retrieve objects from workspace." << std::endl;
        FitResultFile->Close();
        return -1;
    }

    mass->setRange("analysis", D0Opt.massMin, D0Opt.massMax);
    mass->setMin(D0Opt.massMin);
    mass->setMax(D0Opt.massMax);

    std::cout << "nsig range: [" << nSig->getMin() << ", " << nSig->getMax() << "]" << std::endl;
    std::cout << "nbkg range: [" << nBkg->getMin() << ", " << nBkg->getMax() << "]" << std::endl;

    RooStats::SPlot splot("splot", "", *data, total_pdf, RooArgList(*nSig, *nBkg), 
                          RooArgSet(), false, true, "rd_total", 
                          RooFit::NumCPU(8, 0), RooFit::Range("analysis"));

    RooDataSet* newrd = splot.GetSDataSet();
    if (!newrd) {
        std::cerr << "Error: SPlot dataset is null." << std::endl;
        FitResultFile->Close();
        return -1;
    }
// for (auto idx : ROOT::TSeqI(newrd->numEntries())) {
//     const RooArgSet* row = newrd->get(idx);
//     RooRealVar* nsig_sw = (RooRealVar*)row->find("nsig_sw");
//     RooRealVar* nbkg_sw = (RooRealVar*)row->find("nbkg_sw");
//     if (nsig_sw && nbkg_sw) {
//         double sum_sw = nsig_sw->getVal() + nbkg_sw->getVal();
//         if (fabs(sum_sw - 1.0) > 1e-6) {
//             std::cout << "Event " << idx << ": sWeight sum = " << sum_sw << " (not 1!)" << std::endl;
//         }
//     }
// }
    newrd->Print("V");

    RooRealVar parSW_data("nsig_sw", "", -1000, 1000);
    RooDataSet* wds_sig = new RooDataSet("fitDataSWeighted_sig", "", newrd, *newrd->get(), nullptr, parSW_data.GetName());
    // RooDataSet* wds_bkg = new RooDataSet("fitDataSWeighted_bkg", "", newrd, *newrd->get(), nullptr, "nbkg_sw");

    double corr2 = 0.0, count2 = 0.0;
    for (auto idx : ROOT::TSeqI(wds_sig->numEntries())) {
        const RooArgSet* row = wds_sig->get(idx);
        RooRealVar* mass = (RooRealVar*)row->find("mass");
        RooRealVar* pt = (RooRealVar*)row->find("pT");
        RooRealVar* y = (RooRealVar*)row->find("y");
        if (!mass || !pt || !y) continue;

        double sWeight = wds_sig->weight();
        if (sWeight == 0.0) continue;

        double corr_ = hist_eff->GetBinContent(hist_eff->FindBin(pt->getVal(), y->getVal(), mvaCut + 1e-9));
        if (corr_ == 0.0) continue;

        corr_ /= sWeight;
        corr2 += 1.0 / corr_;
        count2 += sWeight;
    }

    // delete wds_sig;
    // delete wds_bkg;
    // delete newrd;
    FitResultFile->Close();

    return (count2 > 0) ? corr2 / count2 : -1;
}

void DrawSplotHist_copy() {
    // 입력 파일 로드
    TFile* effFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output.root", "READ");
    TH3D* effmap = effFile ? (TH3D*)effFile->Get("pt_y_mva_pr_ratio") : nullptr;
    TFile* yieldFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/D0_Yield_mva.root", "READ");
    TH1D* h1 = yieldFile ? (TH1D*)yieldFile->Get("D0_Yield_mva") : nullptr;

    if (!effmap || !h1) {
        std::cerr << "Error: Could not load input histograms." << std::endl;
        if (effFile) effFile->Close();
        if (yieldFile) yieldFile->Close();
        return;
    }

    // MVA 구간 설정
    std::vector<double> mvaBin = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    TH1D* h2 = new TH1D("h1", "Corrected D0 Yield vs MVA Cut", mvaBin.size() - 1, mvaBin.data());
    TH1D* h3 = new TH1D("weightHisto", "Efficiency Correction Factor vs MVA Cut", mvaBin.size() - 1, mvaBin.data());

    // pT, y 분포를 위한 히스토그램 (신호와 배경)
    TH1D* h_pt_sig = new TH1D("h_pt_sig", "Signal pT Distribution;M_{T} (GeV/c);Entries", 50, 0, 50);
    TH1D* h_pt_bkg = new TH1D("h_pt_bkg", "Background pT Distribution;M_{T} (GeV/c);Entries", 50, 0, 50);
    TH1D* h_y_sig = new TH1D("h_y_sig", "Signal y Distribution;y;Entries", 50, -2.5, 2.5);
    TH1D* h_y_bkg = new TH1D("h_y_bkg", "Background y Distribution;y;Entries", 50, -2.5, 2.5);
    TH2D* h2d_sig = new TH2D("h2d_sig", "Signal pT vs y;M_{T} (GeV/c);y", 50, 0, 50, 50, -2.5, 2.5);
    TH2D* h2d_bkg = new TH2D("h2d_bkg", "Background pT vs y;M_{T} (GeV/c);y", 50, 0, 50, 50, -2.5, 2.5);

    FitOpt D0opt;
    std::vector<std::pair<double, double>> ptBins = {{0, 50}};
    std::vector<std::pair<double, double>> centBins = {{0, 100}};

    // 특정 MVA 컷에 대해 SPlot 수행 (예: 첫 번째 MVA 컷 사용)
    double mvaCut = mvaBin[9]; // 첫 번째 MVA 컷으로 고정 (필요 시 루프 추가)
    for (auto ptbin : ptBins) {
        for (auto centbin : centBins) {
            D0opt.pTMin = ptbin.first;
            D0opt.pTMax = ptbin.second;
            D0opt.centMin = centbin.first;
            D0opt.centMax = centbin.second;
            D0opt.mvaMin = mvaCut;
            D0opt.D0MCDefault();

            // SPlot 데이터셋 생성
            TFile* FitResultFile = TFile::Open((D0opt.outputDir + "/" + D0opt.outputFile).c_str());
            if (!FitResultFile || FitResultFile->IsZombie()) continue;

            RooWorkspace* ws = (RooWorkspace*)FitResultFile->Get("ws_D0");
            if (!ws) {
                FitResultFile->Close();
                continue;
            }

            RooAddPdf* total_pdf = (RooAddPdf*)ws->pdf("total_pdf");
            RooDataSet* data = (RooDataSet*)ws->data("dataset");
            RooRealVar* nSig = (RooRealVar*)ws->var("nsig");
            RooRealVar* nBkg = (RooRealVar*)ws->var("nbkg");
            RooRealVar* mass = (RooRealVar*)ws->var("mass");

            if (!total_pdf || !data || !nSig || !nBkg || !mass) {
                FitResultFile->Close();
                continue;
            }

            mass->setRange("analysis", D0opt.massMin, D0opt.massMax);
            RooStats::SPlot splot("splot", "", *data, total_pdf, RooArgList(*nSig, *nBkg), 
                                  RooArgSet(), false, true, "rd_total", 
                                  RooFit::NumCPU(8, 0), RooFit::Range("analysis"));

            RooDataSet* newrd = splot.GetSDataSet();
            if (!newrd) {
                FitResultFile->Close();
                continue;
            }

            RooDataSet* wds_sig = new RooDataSet("fitDataSWeighted_sig", "", newrd, *newrd->get(), nullptr, "nsig_sw");
            RooDataSet* wds_bkg = new RooDataSet("fitDataSWeighted_bkg", "", newrd, *newrd->get(), nullptr, "nbkg_sw");

            // pT, y 분포 채우기
            for (auto idx : ROOT::TSeqI(wds_sig->numEntries())) {
                const RooArgSet* row = wds_sig->get(idx);
                RooRealVar* pt = (RooRealVar*)row->find("pT");
                RooRealVar* y = (RooRealVar*)row->find("y");
                if (!pt || !y) continue;

                double sWeight = wds_sig->weight();
                if (sWeight == 0.0) continue;
                h_pt_sig->Fill(pt->getVal(), sWeight);
                h_y_sig->Fill(y->getVal(), sWeight);
                h2d_sig->Fill(pt->getVal(), y->getVal(), sWeight);
            }

            for (auto idx : ROOT::TSeqI(wds_bkg->numEntries())) {
                const RooArgSet* row = wds_bkg->get(idx);
                RooRealVar* pt = (RooRealVar*)row->find("pT");
                RooRealVar* y = (RooRealVar*)row->find("y");
                if (!pt || !y) continue;

                double bWeight = wds_bkg->weight();
                if (bWeight == 0.0) continue;
                h_pt_bkg->Fill(pt->getVal(), bWeight);
                h_y_bkg->Fill(y->getVal(), bWeight);
                h2d_bkg->Fill(pt->getVal(), y->getVal(), bWeight);
            }

            // delete wds_sig;
            // delete wds_bkg;
            // delete newrd;
            // FitResultFile->Close();
            cout << 123123 << endl;
        }
    }

    // 기존 MVA 루프 (효율 보정 계산)
    int imva = 1;
    // double mvaCut = mvaBin[0];
        for (auto ptbin : ptBins) {
            for (auto centbin : centBins) {
                D0opt.pTMin = ptbin.first;
                D0opt.pTMax = ptbin.second;
                D0opt.centMin = centbin.first;
                D0opt.centMax = centbin.second;
                D0opt.mvaMin = mvaCut;
                D0opt.D0MCDefault();

                double corr = CalSplotweight(D0opt, effmap, mvaCut);
                std::cout << "MVA: " << mvaCut << ", Original Yield: " << h1->GetBinContent(imva) 
                          << ", Correction: " << corr << std::endl;

                h2->SetBinContent(imva, h1->GetBinContent(imva) * corr);
                h3->SetBinContent(imva, corr);
                imva++;
            }
        }
    

    // 시각화
    TCanvas* c1 = new TCanvas("c1", "Corrected Yield", 800, 600);
    h2->SetLineColor(kBlue);
    h2->GetXaxis()->SetTitle("MVA Cut");
    h2->GetYaxis()->SetTitle("Corrected Yield");
    h2->Draw();
    c1->SaveAs("splot_hist.png");

    TCanvas* c2 = new TCanvas("c2", "Correction Factor", 800, 600);
    h3->SetLineColor(kRed);
    h3->GetXaxis()->SetTitle("MVA Cut");
    h3->GetYaxis()->SetTitle("Correction Factor");
    h3->Draw();
    c2->SaveAs("splot_hist_weight.png");

    // pT, y 분포 시각화
    TCanvas* c3 = new TCanvas("c3", "pT Distributions", 800, 600);
    h_pt_sig->SetLineColor(kBlue);
    h_pt_bkg->SetLineColor(kRed);
    h_pt_sig->Draw();
    h_pt_bkg->Draw("SAME");
    c3->SaveAs("splot_pt_dist.png");

    TCanvas* c4 = new TCanvas("c4", "y Distributions", 800, 600);
    h_y_sig->SetLineColor(kBlue);
    h_y_bkg->SetLineColor(kRed);
    h_y_sig->Draw();
    h_y_bkg->Draw("SAME");
    c4->SaveAs("splot_y_dist.png");

    TCanvas* c5 = new TCanvas("c5", "2D Distributions", 1200, 600);
    c5->Divide(2, 1);
    c5->cd(1);
    h2d_sig->Draw("COLZ");
    c5->cd(2);
    h2d_bkg->Draw("COLZ");
    c5->SaveAs("splot_2d_dist.png");

    // 메모리 정리
    // delete h2;
    // delete h3;
    // delete h_pt_sig;
    // delete h_pt_bkg;
    // delete h_y_sig;
    // delete h_y_bkg;
    // delete h2d_sig;
    // delete h2d_bkg;
    // effFile->Close();
    // yieldFile->Close();
}