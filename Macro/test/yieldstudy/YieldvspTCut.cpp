void YieldvspTCut() {
    TFile* fin  = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/flatSkimForBDT_DStar_MixDStar_MC_25Jul_0_.root");
    if (!fin || !fin->IsOpen()) {
        std::cerr << "Error: Could not open input file!" << std::endl;
        return;
    }
    TH1D* hist = new TH1D("yield", "Yield vs pT Cut", 10, 0, 10);
    TTree* tree = (TTree*)fin->Get("skimTreeFlat");
    if (!tree) {
        std::cerr << "Error: Could not find tree in input file!" << std::endl;
        return;
    }
    hist->SetBinContent(1, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >0.5 && pTGrandD2 > 0.5 && pT>6 && pT <7"));
    hist->SetBinContent(2, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >0.7 && pTGrandD2 > 0.7 && pT>6 && pT <7"));
    hist->SetBinContent(3, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >0.9 && pTGrandD2 > 0.9 && pT>6 && pT <7"));
    hist->SetBinContent(4, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >1.0 && pTGrandD2 > 1.0 && pT>6 && pT <7"));
    hist->SetBinContent(5, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >1.3 && pTGrandD2 > 1.3 && pT>6 && pT <7"));
    hist->SetBinContent(6, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >1.5 && pTGrandD2 > 1.5 && pT>6 && pT <7"));
    hist->SetBinContent(7, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >1.7 && pTGrandD2 > 1.7 && pT>6 && pT <7"));
    hist->SetBinContent(8, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >1.9 && pTGrandD2 > 1.9 && pT>6 && pT <7"));
    hist->SetBinContent(9, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >2.0 && pTGrandD2 > 2.0 && pT>6 && pT <7"));
    hist->SetBinContent(10, tree->GetEntries("matchGEN==1 && pTD2>0.4 && pTGrandD1 >2.3 && pTGrandD2 > 2.3 && pT>6 && pT <7"));
    TAxis* xAxis = hist->GetXaxis();

    for (int i = 0; i < 10; ++i) {
        TString label;
        label.Form("%f", 0.5+0.2*i); // bin 번호 i에 대해 i*10 값을 라벨로 설정

        
        // i번째 bin에 라벨 설정
        xAxis->SetBinLabel(i+1, label.Data());
    }
    TCanvas* c1 = new TCanvas("c1", "Yield vs pT Cut", 800, 600);
    hist->SetTitle("Yield vs pT Cut; pT Cut; Yield");
    hist->SetLineColor(kBlue);
    hist->SetMarkerStyle(20);
    hist->SetMarkerColor(kBlue);
    hist->Draw("E");
    c1->SaveAs("yield_vs_pT_cut_6_7_pTD2_0p4.png");


}