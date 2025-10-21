#include "../fit/Opt.h"
#include <TFile.h>
double CalSplotweight(FitOpt D0Opt, TH3D* hist_eff, double mvaCut, std::string subDir){
    // TFile *effmapFile = TFile::Open(effmapPath.c_str());
    D0Opt.outputDir = D0Opt.outputDir + subDir + "/";

    gStyle->SetOptStat(0);
    TFile *FitResultFile = TFile::Open((D0Opt.outputDir+"/"+D0Opt.outputFile).c_str());
    
    RooAddPdf *total_pdf = (RooAddPdf*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->pdf("total_pdf");
    RooDataSet *data = (RooDataSet*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->data("dataset");
    RooRealVar *nSig = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nsig");
    RooRealVar *nBkg = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nbkg");
    RooRealVar *mass = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("mass");
    // RooRealVar *nSig = new RooRealVar("nsig", "nsig", 0, 0, data->numEntries());
    // RooRealVar *nBkg = new RooRealVar("nbkg", "nbkg", 0, 0, data->numEntries());
    // nSig->setMin(0);
    // nSig->setMax(data->numEntries());
    // nBkg->setMin(0);
    // nBkg->setMax(data->numEntries());
    // if(total_pdf == nullptr || data == nullptr || nSig == nullptr || nBkg == nullptr){
    //     std::cerr << "Error: Could not retrieve objects from workspace." << std::endl;
    //     return -1;
    // }
    // RooFitResult* fitResult = total_pdf->fitTo(*data, RooFit::Save(true), RooFit::PrintLevel(1));
    // fitResult->Print("V");
    // std::cout << "Fit status: " << fitResult->status() << ", Covariance matrix quality: " 
    //           << fitResult->covQual() << std::endl;

    // // 공분산 행렬 출력
    // const TMatrixDSym& covMatrix = fitResult->covarianceMatrix();
    // std::cout << "Covariance Matrix:" << std::endl;
    // covMatrix.Print();

    // // 상관관계 행렬 출력
    // const TMatrixDSym& corMatrix = fitResult->correlationMatrix();
    // std::cout << "Correlation Matrix:" << std::endl;
    // corMatrix.Print(); 
RooArgSet* params = total_pdf->getParameters(*data);
params->Print("V");
    mass->setRange("analysis", D0Opt.massMin, D0Opt.massMax);
    mass->setMin(D0Opt.massMin);
    mass->setMax(D0Opt.massMax);
    total_pdf->Print("V");
    
    std::cout << "nsig range: [" << nSig->getMin() << ", " << nSig->getMax() << "]" << std::endl;
std::cout << "nbkg range: [" << nBkg->getMin() << ", " << nBkg->getMax() << "]" << std::endl;

RooStats::SPlot splot("splot", "", *data, 
    total_pdf, 
    RooArgList(*nSig, *nBkg), 
    RooArgSet(),
    false,
    true,
    "rd_total",
    RooFit::NumCPU(8, 0),
    RooFit::Range("analysis")
);
RooRealVar parSW_data("nsig_sw", "", -1000, 1000);
auto newrd = splot.GetSDataSet();
int mismatch_count = 0;
for (auto idx : ROOT::TSeqI(newrd->numEntries())) {
    const RooArgSet* row = newrd->get(idx);
    RooRealVar* nsig_sw = (RooRealVar*)row->find("nsig_sw");
    RooRealVar* nbkg_sw = (RooRealVar*)row->find("nbkg_sw");
    if (!nsig_sw || !nbkg_sw) {
        std::cerr << "Event " << idx << ": Missing nsig_sw or nbkg_sw!" << std::endl;
        continue;
    }
    double sum_sw = nsig_sw->getVal() + nbkg_sw->getVal();
    if (fabs(sum_sw - 1.0) > 1e-6) { // 부동소수점 정밀도 고려
        // std::cout << "Event " << idx << ": nsig_sw = " << nsig_sw->getVal()
        //           << ", nbkg_sw = " << nbkg_sw->getVal() 
        //           << ", Sum = " << sum_sw << std::endl;
        mismatch_count++;
    }
}
std::cout << "Total events with sWeight sum != 1: " << mismatch_count 
          << " / " << newrd->numEntries() << std::endl;
// RooDataSet* newrd = splot.GetSDataSet();
newrd->Print("V");
RooDataSet* wds_sig = new RooDataSet("fitDataSWeighted_sig", "", newrd,
        *newrd->get(),
        nullptr, parSW_data.GetName()
);
// corr = corr/((double) count);
double sWeight = 0.0;
double count2 = 0.0;
double corr2 = 0.0;
for( auto idx: ROOT::TSeqI(wds_sig->numEntries())){
    const RooArgSet* row = wds_sig->get(idx);
    RooRealVar* mass = (RooRealVar*) row->find("mass");
    RooRealVar* pt = (RooRealVar*) row->find("pT");
    RooRealVar* y = (RooRealVar*) row->find("y");
    //if(!(abs(y->getVal())<1)) continue;
    if(!(pt->getVal() >2 && pt->getVal()<50)) continue;
    double corr_ = hist_eff->GetBinContent(hist_eff->FindBin(pt->getVal(), y->getVal(), mvaCut+0.000000001));
    // cout << "pT : " << pt->getVal() << " y : "<< y->getVal() << " corr_: " << corr_ << endl;
    sWeight = wds_sig->weight();
    // if (sWeight <= 0.0) continue;
    //if(sWeight==0.0) continue;
    //if(corr_==0.0) continue;
    // cout << sWeight << endl;
    corr_ /= sWeight;
    corr2 += 1.0/corr_; 
    count2+= sWeight;
    //  cout << "corr_ : " << corr_ << " corr2 : " << corr2 << " count2 : " << count2 << endl;
    }
    cout << "count2 : " << count2 << " nsig" << nSig->getVal() << " nbkg " << nBkg->getVal() << endl;
    delete FitResultFile;
    delete data;
    delete nSig;
    delete nBkg;
    delete mass;
    return corr2/count2;

}
void DrawSplotHist(){
    TH3D *effmap = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output.root","read")->Get<TH3D>("pt_y_mva_pr_ratio");
    TH1D *h1 = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/D0_Yield_mva.root","read")->Get<TH1D>("D0_Yield_mva");
    FitOpt D0opt;
    std::string particleType = "D0";
    std::string subDir = "D0_Apr30/";
    std::vector<std::pair<double,double>> ptBins = {
        {2,4},
        {4,9},
        {9,50}
        // {7,10},
        // {10,12.5},
        // {12.5,15},
        // {15,20},
        // {20,25},
        // {25,30},
        // {30,40},
        // {40,100}
        // {0,50}
    };
    std::vector<std::pair<double,double>> centBins = {
        // {0,10},
        // {10,30},
        // {30,50},
        // {50,100}
        {0,90}
    };
    std::vector<double> mvaBin = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    // std::vector<double> mvaBin = {0.990};
    std::vector<double> mvaBin1 = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999,1};
    TH1D *h2 = new TH1D("h1", "h1", mvaBin1.size()-1,mvaBin1.data());
    TH1D *h3 = new TH1D("weightHisto", "", mvaBin1.size()-1,mvaBin1.data());
    int imva =0; 
    for(auto mva : mvaBin){
        for(auto ptbin : ptBins){
            for(auto centbin : centBins){
                D0opt.pTMin = ptbin.first;
                D0opt.pTMax = ptbin.second;
                D0opt.centMin = centbin.first;
                D0opt.centMax = centbin.second;
                D0opt.mvaMin = mva;
                D0opt.D0MCDefault();
                D0opt.outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/"+D0opt.outputDir;
                double corr = CalSplotweight(D0opt, effmap, mva, subDir);
                cout << h1->GetBinContent(imva+1) << endl;
                cout << corr << endl;

                h2->SetBinContent(imva+1,h1->GetBinContent(imva+1)*corr);
                h3->SetBinContent(imva+1,corr);
                imva++;

            }
        }
    }
    TCanvas *c0 = new TCanvas("c0", "Corrected Yield", 800, 600);
    TLegend *leg = new TLegend(0.7, 0.8, 0.9, 0.9);
    // leg->SetBorderSize(0);
    h2->GetYaxis()->SetRangeUser(h1->GetMinimum()*0,h2->GetBinContent(1) * 1.2); // Adjust max for visibility
    h2->Draw("E1"); // Draw with error bars
    h1->SetLineColor(kRed);
    h1->Scale(h2->GetBinContent(1)/h1->GetBinContent(1));
    h1->Draw("E1 same");
    leg->AddEntry(h2, "Corrected Yield", "lep");
    leg->AddEntry(h1, "Raw Yield (scaled)", "l");
    leg->Draw();
    
    c0->SaveAs("splot_hist_correctedvsraw.png");
    
    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    h2->GetYaxis()->SetRangeUser(h2->GetMinimum()*0,h2->GetMaximum() * 1.2); // Adjust max for visibility
    h2->SetTitle("Corrected Yield vs MVA Cut;MVA Cut Lower Edge;Corrected Yield");
    h2->Draw("E1"); // Draw with error bars
    // h1->SetLineColor(kRed);
    // h1->Scale(h2->GetMaximum()/h1->GetMaximum());
    // h1->Draw("same");
    c1->SaveAs("splot_hist.png");
    TCanvas *c2 = new TCanvas("c2", "c2", 800, 600);
    h3->SetTitle("weights vs MVA Cut");
    h3->Draw();
    c2->SaveAs("splot_hist_weight.png");

    // std::cout << "Correlation: " << corr << std::endl;

}
