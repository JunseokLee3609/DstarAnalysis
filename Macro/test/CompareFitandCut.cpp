#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <vector>
#include <iostream>

void CompareFitandCut() {
    // 입력 파일 열기
    TFile *file = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/D0/flatSkimForBDT_D0_PbPb_MC_ONNX_withOfficialCent_DpT018_30Apr25.root");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open input file!" << std::endl;
        return;
    }

    // TTree 가져오기 (트리 이름은 예시로 "tree"로 가정, 실제 이름으로 변경 필요)
    TTree *tree = (TTree*)file->Get("skimTreeFlat");
    if (!tree) {
        std::cerr << "Error: Cannot find TTree!" << std::endl;
        file->Close();
        return;
    }

    // mvabin 설정
    std::vector<double> mvabins = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    std::vector<double> mvabins1 = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999,1.000};
    int nBins = mvabins.size();

    // 히스토그램 생성
    TH1F *hist = new TH1F("hist", "Entry Counts per MVA Score; MVA Score; Entries", mvabins1.size()-1, mvabins1.data());
    hist->SetStats(0);
    hist->SetLineColor(kBlue);
    hist->SetLineWidth(2);

    // TTree 변수 설정
    float mva, y, pT;
    bool matchGEN, isSwap;
    tree->SetBranchAddress("mva", &mva);
    tree->SetBranchAddress("y", &y);
    tree->SetBranchAddress("pT", &pT);
    tree->SetBranchAddress("matchGEN", &matchGEN);
    tree->SetBranchAddress("isSwap", &isSwap);

    // 각 mvascore에 대해 엔트리 수 계산
    for (int i = 0; i < nBins; ++i) {
        double mvascore = mvabins[i];
        Long64_t nEntries = tree->GetEntries(Form("matchGEN == 1 && !isSwap && mva > %f && std::abs(y) < 1 && pT > 0 && pT < 50 && centrality < 180",mvascore));
        cout << nEntries << endl;

        

        // 히스토그램에 엔트리 수 저장
        hist->SetBinContent(i+1, nEntries);
    }
    // if (hist->Integral() > 0) {
    //     hist->Scale(1.0 / hist->Integral());
    // } else {
    //     std::cerr << "Warning: hist has zero integral, skipping normalization." << std::endl;
    // }

    // 다른 ROOT 파일에서 히스토그램 가져오기 (예시 파일 경로와 히스토그램 이름)
    TFile *otherFile = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/D0_Yield_mva.root"); // 실제 파일 경로로 변경
    if (!otherFile || otherFile->IsZombie()) {
        std::cerr << "Error: Cannot open other ROOT file!" << std::endl;
        file->Close();
        delete hist;
        return;
    }


    TH2F *histOther = (TH2F*)otherFile->Get("D0_Yield_mva");
    auto *h2f = histOther->ProjectionX(); // 실제 히스토그램 이름으로 변경
    if (!histOther) {
        std::cerr << "Error: Cannot find histogram in other ROOT file!" << std::endl;
        otherFile->Close();
        file->Close();
        delete hist;
        return;
    }
    // histOther->SetLineColor(kRed);
    // histOther->SetLineWidth(2);
    h2f->SetStats(0);
    h2f->SetLineColor(kRed);
    h2f->SetLineWidth(2);
    // if (histOther->Integral() > 0) {
    //     histOther->Scale(1.0 / histOther->Integral());
    // } else {
    //     std::cerr << "Warning: histOther has zero integral, skipping normalization." << std::endl;
    // }

    // 캔버스 생성 및 그리기
    TCanvas *canvas = new TCanvas("canvas", "Histogram Comparison", 800, 600);
    // h2f->Draw("HIST");
    // // histOther->Draw("HIST");
    // hist->Draw("HIST SAME");

    // // 범례 추가
    TLegend *legend = new TLegend(0.78, 0.78, 0.88, 0.88);
    legend->SetBorderSize(0);
    legend->AddEntry(hist, "Condition", "l");
    legend->AddEntry(h2f, "Fitting", "l");
    // legend->Draw();

    // // 캔버스 저장 (선택 사항)
    // canvas->SaveAs("histogram_comparison.png");
    // Create pads
    TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    pad1->SetBottomMargin(0.02); // Upper pad bottom margin
    pad1->Draw();
    pad1->cd(); // Go into the upper pad

    // Draw main histograms in the upper pad
    h2f->GetXaxis()->SetLabelSize(0); // Hide x-axis labels on the main plot
    h2f->Draw("HIST");
    hist->Draw("HIST SAME");
    legend->Draw(); // Draw legend in the upper pad

    // Go back to the main canvas before creating the second pad
    canvas->cd();

    // Create the lower pad for the ratio plot
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0.02); // Lower pad top margin
    pad2->SetBottomMargin(0.2); // Lower pad bottom margin
    pad2->SetGridy(); // Add a grid for the ratio
    pad2->Draw();
    pad2->cd(); // Go into the lower pad

    // Create the ratio histogram
    TH1F *hRatio = (TH1F*)hist->Clone("hRatio");
    hRatio->SetTitle(""); // Remove the title
    hRatio->SetStats(0);
    hRatio->Divide(h2f); // Divide hist by h2f
    hRatio->SetLineColor(kBlack);
    hRatio->SetMarkerStyle(20);
    hRatio->SetMarkerSize(0.8);

    // Adjust ratio plot appearance
    hRatio->GetYaxis()->SetTitle("MC / Data");
    hRatio->GetYaxis()->SetNdivisions(505); // Nicer divisions
    hRatio->GetYaxis()->SetTitleSize(20);
    hRatio->GetYaxis()->SetTitleFont(43);
    hRatio->GetYaxis()->SetTitleOffset(1.55);
    hRatio->GetYaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    hRatio->GetYaxis()->SetLabelSize(15);

    hRatio->GetXaxis()->SetTitle("MVA Score");
    hRatio->GetXaxis()->SetTitleSize(20);
    hRatio->GetXaxis()->SetTitleFont(43);
    hRatio->GetXaxis()->SetTitleOffset(3.0); // Increased offset
    hRatio->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    hRatio->GetXaxis()->SetLabelSize(15);

    // Set Y-axis range for ratio plot (optional, adjust as needed)
    hRatio->GetYaxis()->SetRangeUser(0.5, 1.5); // Example range

    hRatio->Draw("ep"); // Draw ratio plot with error bars

    // Draw a line at y=1 for reference
    TLine *line = new TLine(mvabins1.front(), 1.0, mvabins1.back(), 1.0);
    line->SetLineStyle(2); // Dashed line
    line->SetLineColor(kGray+2);
    line->Draw();

    // Update the canvas to show everything
    canvas->Update();
    canvas->SaveAs("histogram_comparison_with_ratio.png");
    // 정리
    // otherFile->Close();
    // file->Close();
    // delete hist;
    // delete histOther;
    // delete canvas;
}

