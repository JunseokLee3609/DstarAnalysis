#include ../Tools/Helper.h
void ScaleHistogram(TH1D* hist) {
    if (hist) {
        hist->Scale(1.0 / hist->Integral());
    } else {
        std::cerr << "Error: Histogram is null." << std::endl;
    }
}
void DrawMVADistributionfromMCAndData(){
    createDir("./output/.");
    TFile * fout = createFileInDir("./output/.", "MVADistribution.root");
    TChain *chainMC = new TChain("skimTreeFlat");
    TChain *chainData = new TChain("skimTreeFlat");
    TH1D *h1Data = new TH1D("MVA Dsitribution","",100,0.9,1.0)
    TH1D *h1MC = new TH1D("MVA Dsitribution","",100,0.9,1.0)
    std::string inputMCFile = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/*.root";
    std::string inputDataFile = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/*.root";
    chainMC->Add(inputMCFile.c_str());
    chainData->Add(inputDataFile.c_str());
    TString cut = "abs(y)<1 && pT>2 && pT<50 && EtaD1< 2.4 && EtaD2< 2.4";
    chainMC->Draw("mva>>h1MC",cut.Data())
    chainData->Draw("mva>>h1Data",cut.Data())

    ScaleHistogram(h1Data);
    ScaleHistogram(h1MC);
    TCanvas *c1 = new TCanvas("c1", "MVA Distribution", 800, 600);
    h1Data->SetLineColor(kRed);
    h1MC->SetLineColor(kBlue);
    h1Data->SetLineWidth(2);
    h1MC->SetLineWidth(2);
    h1Data->Draw("hist");
    h1MC->Draw("hist same");
    TLegend *leg = new TLegend(0.6, 0.7, 0.9, 0.9);
    leg->AddEntry(h1Data, "Data", "l");
    leg->AddEntry(h1MC, "MC", "l");
    leg->Draw();
    c1->SaveAs("./output/MVA_Distribution.png");
    h1Data->Write();
    h1MC->Write();
    fout->Close();

    



}