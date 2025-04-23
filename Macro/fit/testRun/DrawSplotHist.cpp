#include "../Opt.h"
#include <TFile.h>
double CalSplotweight(FitOpt D0Opt, TH3D* hist_eff, double mvaCut){
    // TFile *effmapFile = TFile::Open(effmapPath.c_str());
    TFile *FitResultFile = TFile::Open((D0Opt.outputDir+"/"+D0Opt.outputFile).c_str());
    
    RooAddPdf *total_pdf = (RooAddPdf*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->pdf("total_pdf");
    RooDataSet *data = (RooDataSet*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->data("dataset");
    RooRealVar *nSig = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nsig");
    RooRealVar *nBkg = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("nbkg");
    RooRealVar *mass = (RooRealVar*)((RooWorkspace*)FitResultFile->Get("ws_D0"))->var("mass");
    // RooRealVar *nSig = new RooRealVar("nsig", "nsig", 0, 0, data->numEntries());
    // RooRealVar *nBkg = new RooRealVar("nbkg", "nbkg", 0, 0, data->numEntries());
    nSig->setMin(0);
    nSig->setMax(data->numEntries());
    nBkg->setMin(0);
    nBkg->setMax(data->numEntries());
    if(total_pdf == nullptr || data == nullptr || nSig == nullptr || nBkg == nullptr){
        std::cerr << "Error: Could not retrieve objects from workspace." << std::endl;
        return -1;
    }
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
    //if(!(pt->getVal() >2 && pt->getVal()<50)) continue;
    double corr_ = hist_eff->GetBinContent(hist_eff->FindBin(pt->getVal(), y->getVal(), mvaCut+0.000000001));
    sWeight = wds_sig->weight();
    //if(sWeight==0.0) continue;
    //if(corr_==0.0) continue;
    // cout << sWeight << endl;
    corr_ /= sWeight;
    corr2 += 1.0/corr_; 
    count2+= sWeight;
     //cout << "corr_ : " << corr_ << " corr2 : " << corr2 << " count2 : " << count2 << endl;
    }
    return corr2/count2;

}
void DrawSplotHist(){
    TH3D *effmap = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/output.root","read")->Get<TH3D>("pt_y_mva_pr_ratio");
    TH1D *h1 = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/D0_Yield_mva.root","read")->Get<TH1D>("D0_Yield_mva");
    FitOpt D0opt;
    std::string particleType = "D0";
    std::vector<std::pair<double,double>> ptBins = {
        // {2,3},
        // {3,5},
        // {5,7},
        // {7,10},
        // {10,12.5},
        // {12.5,15},
        // {15,20},
        // {20,25},
        // {25,30},
        // {30,40},
        // {40,100}
        {0,50}
    };
    std::vector<std::pair<double,double>> centBins = {
        // {0,10},
        // {10,30},
        // {30,50},
        // {50,100}
        {0,100}
    };
    std::vector<double> mvaBin = {0.990, 0.991, 0.992, 0.993, 0.994, 0.995, 0.996, 0.997, 0.998, 0.999};
    TH1D *h2 = new TH1D("h1", "h1", mvaBin.size()-1,mvaBin.data());
    TH1D *h3 = new TH1D("weightHisto", "", mvaBin.size()-1,mvaBin.data());
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
                double corr = CalSplotweight(D0opt, effmap, mva);
                cout << h1->GetBinContent(imva+1) << endl;
                cout << corr << endl;

                h2->SetBinContent(imva,h1->GetBinContent(imva)*corr);
                h3->SetBinContent(imva,corr);
                imva++;

            }
        }
    }
    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    h2->Draw();
    c1->SaveAs("splot_hist.png");
    TCanvas *c2 = new TCanvas("c2", "c2", 800, 600);
    h3->Draw();
    c2->SaveAs("splot_hist_weight.png");

    // std::cout << "Correlation: " << corr << std::endl;

}
