#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <vector>
#include <iostream>

void testdraw() {
    // 입력 파일 열기
    TFile *file = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/D0/flatSkimForBDT_D0_PbPb_MC_ONNX_withCent_0_18Apr25.root");
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
        Long64_t nEntries = tree->GetEntries(Form("matchGEN == 1 && !isSwap && mva > %f && std::abs(y) < 1 && pT > 2 && pT < 50",mvascore));
        cout << nEntries << endl;

        

        // 히스토그램에 엔트리 수 저장
        hist->SetBinContent(i+1, nEntries);
    }
    if (hist->Integral() > 0) {
        hist->Scale(1.0 / hist->Integral());
    } else {
        std::cerr << "Warning: hist has zero integral, skipping normalization." << std::endl;
    }

    // 다른 ROOT 파일에서 히스토그램 가져오기 (예시 파일 경로와 히스토그램 이름)
    TFile *otherFile = TFile::Open("D0_Yield_mva.root"); // 실제 파일 경로로 변경
    if (!otherFile || otherFile->IsZombie()) {
        std::cerr << "Error: Cannot open other ROOT file!" << std::endl;
        file->Close();
        delete hist;
        return;
    }

    TH1F *histOther = (TH1F*)otherFile->Get("D0_Yield_mva"); // 실제 히스토그램 이름으로 변경
    if (!histOther) {
        std::cerr << "Error: Cannot find histogram in other ROOT file!" << std::endl;
        otherFile->Close();
        file->Close();
        delete hist;
        return;
    }
    histOther->SetLineColor(kRed);
    histOther->SetLineWidth(2);
    if (histOther->Integral() > 0) {
        histOther->Scale(1.0 / histOther->Integral());
    } else {
        std::cerr << "Warning: histOther has zero integral, skipping normalization." << std::endl;
    }

    // 캔버스 생성 및 그리기
    TCanvas *canvas = new TCanvas("canvas", "Histogram Comparison", 800, 600);
    histOther->Draw("HIST");
    hist->Draw("HIST SAME");

    // 범례 추가
    TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->AddEntry(hist, "MC", "l");
    legend->AddEntry(histOther, "Data", "l");
    legend->Draw();

    // 캔버스 저장 (선택 사항)
    canvas->SaveAs("histogram_comparison.png");

    // 정리
    // otherFile->Close();
    // file->Close();
    // delete hist;
    // delete histOther;
    // delete canvas;
}

