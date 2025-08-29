#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
#include "../../interface/simpleDMC.hxx"
#include "../../interface/simpleAlgos.hxx"
#include "../../Tools/Transformations.h"
// #include "../../Tools/BasicHeaders.h"
// #include "../../Tools/Parameters/AnalysisParameters.h"
#include "../../Tools/Parameters/PhaseSpace.h"
#include "../../Correction/commonSelectionVar.h"

void Draw2DHisto(){
    std::vector<std::pair<double,double>> ptBins = {
        {5,7},
        {7,10},
        {10,15},
        {15,20},
        {20,30}
    };
    std::vector<double> pTBin1D = {
        5,
        7,
        10,
        15,
        20,
        30
    };

    std::vector<std::pair<double,double>> cosBins = {
        {-1,-0.75},
        {-0.75,-0.5},
        {-0.5,-0.25},
        {-0.25,0},
        {0,0.25},
        {0.25,0.5},
        {0.5,0.75},
        {0.75,1}
    };
    std::vector<double> cosBin1D = {
        -1,
        -0.75,
        -0.5,
        -0.25,
        0,
        0.25,
        0.5,
        0.75,
        1
    };
    string fileName="/home/jun502s/DstarAna/DStarAnalysis/Data/MC/Dstar2024ppRef/Mar30NonSwap/d0ana_tree_nonswapsample_ppref_30Mar.root";
    // string fileName="/home/jun502s/DstarAna/DStarAnalysis/Macro/Correction/d0ana_tree_nokinematic_Pi0p4pT.root"
    TFile* f =  TFile::Open(fileName.c_str());
    // TFile* feffMap = TFile::Open("output.root");

    string name_tree = "dStarana_mc/PATCompositeNtuple";
    /* Get tree*/
    auto t = (TTree*) f->Get(name_tree.c_str());
    DataFormat::simpleDStarMCTreeevt evtMC;
    // DataFormat::simpleDStarDataTreeflat evtData;
    evtMC.setTree(t);
    evtMC.setGENTree(t);
    int nEvts;
    nEvts = t->GetEntries();
    TH2D* h2D = new TH2D("h2D","D* Yield;p_{T} (GeV/c);cos#theta",
                          cosBin1D.size()-1, &cosBin1D[0],pTBin1D.size()-1, &pTBin1D[0]);
    TH2D* h2DGEN = new TH2D("h2DGEN","D* Yield;p_{T} (GeV/c);cos#theta",
                          cosBin1D.size()-1, &cosBin1D[0],pTBin1D.size()-1, &pTBin1D[0]);
    auto fill = [&](int i){
        auto D0y= algo::rapidity(evtMC.pTD1[i], evtMC.EtaD1[i], evtMC.phi[i], evtMC.massDaugther1[i]);
    if(evtMC.matchGEN[i]  && !evtMC.isSwap[i] &&
        fabs(evtMC.y[i]) < DSGLABSY &&
        evtMC.pT[i] > DSGLPTLO &&
        fabs(D0y) < DSGLABSY_D0 &&
        fabs(evtMC.EtaD2[i]) < DSGLABSETA_D1 && 
        evtMC.pTD2[i] > DSGLPT_D1 && 
        fabs(evtMC.EtaGrandD1[i]) < DSGLABSETA_D0DAU1 &&
        fabs(evtMC.EtaGrandD2[i]) < DSGLABSETA_D0DAU2 &&
        evtMC.pTGrandD1[i] > DSGLPT_D0DAU1 &&
        evtMC.pTGrandD2[i] > DSGLPT_D0DAU2 &&
        true)
    {
        TLorentzVector Dstar, D0;
        Dstar.SetPtEtaPhiM(evtMC.pT[i], evtMC.eta[i], evtMC.phi[i], evtMC.mass[i]);
        D0.SetPtEtaPhiM(evtMC.pTD1[i], evtMC.EtaD1[i], evtMC.PhiD1[i], evtMC.massDaugther1[i]);
        TVector3 vect = DstarDau1Vector_Helicity(Dstar,D0);

        // double weight = findWeight(evtMC.pT[i], evtMC.y[i], evtMC.phi[i], effMap);
        h2DGEN->Fill(vect.CosTheta(),evtMC.pT[i]);
        // h->Fill(vect.CosTheta(), weight);
     }
    };
    for( auto idx : ROOT::TSeqU(nEvts)){
        t->GetEntry(idx);
        for( auto i : ROOT::TSeqI(evtMC.candSize)){
        fill(i);
    }
}
    TCanvas * cGEN = new TCanvas("c1", "D* Yields", 800, 600);
    gPad->SetRightMargin(0.15);
    h2DGEN->SetStats(0);
    h2DGEN->Draw("LEGO2Z");
    cGEN->SaveAs("DStar_Yield_2D_GEN.pdf");

    
    
    FitOpt opt;
    
    for(int ipt = 0; ipt < ptBins.size(); ipt++){
        auto ptbin = ptBins[ipt];
        for(int icos = 0; icos < cosBins.size(); icos++){
            auto cosbin = cosBins[icos];
            
            // 각 빈에 맞는 옵션 설정
            opt.pTMin = ptbin.first;
            opt.pTMax = ptbin.second;
            opt.cosMin = cosbin.first;
            opt.cosMax = cosbin.second;
            opt.DStarMCDefault(); // 기본 옵션 설정
            
            // 파일 경로 생성
            std::string fullPath = opt.outputDir;
            
            if (!fullPath.empty() && fullPath.back() != '/') fullPath += '/';
            
            fullPath += opt.outputFile;
            cout << "Full path: " << fullPath << endl;
            
            TFile *file = TFile::Open(fullPath.c_str());
            if (file && !file->IsZombie()) {
                // Try to get the fit result object
                RooFitResult* fitResult = (RooFitResult*)file->Get("fitResult");
                
                if (fitResult) {
                    // 올바르게 파라미터를 찾는 방법 1: RooArgList에서 반복문으로 찾기
                    const RooArgList& finalPars = fitResult->floatParsFinal();
                    for(int i = 0; i < finalPars.getSize(); i++) {
                        RooRealVar* par = dynamic_cast<RooRealVar*>(finalPars.at(i));
                        if(par && std::string(par->GetName()) == "nsig") {
                            double nsig = par->getVal();
                            double nsigError = par->getError();
                            
                            
                            h2D->SetBinContent(icos+1,ipt+1,  nsig);
                            h2D->SetBinError(icos+1,ipt+1, nsigError);
                            
                            std::cout << "pT: [" << ptbin.first << "-" << ptbin.second << "], cos: [" 
                                     << cosbin.first << "-" << cosbin.second << "], nsig: " 
                                     << nsig << " ± " << nsigError << std::endl;
                            break;
                        }
                    }
                    
                    /* 
                    // 방법 2: 직접 다운캐스팅 (위의 방법이 더 안전함)
                    RooAbsArg* nsigArg = fitResult->floatParsFinal().find("nsig");
                    if(nsigArg) {
                        RooRealVar* nsigVar = dynamic_cast<RooRealVar*>(nsigArg);
                        if(nsigVar) {
                            double nsig = nsigVar->getVal();
                            double nsigError = nsigVar->getError();
                            
                            h2D->SetBinContent(ipt+1, icos+1, nsig);
                            h2D->SetBinError(ipt+1, icos+1, nsigError);
                        }
                    }
                    */
                } else {
                    std::cout << "FitResult not found in file: " << fullPath << std::endl;
                }
                
                file->Close();
                delete file;
            } else {
                std::cout << "Could not open file: " << fullPath << std::endl;
            }
        }
    }
    
    // 결과 시각화
    TCanvas* c1 = new TCanvas("c1", "D* Yields", 800, 600);
    gPad->SetRightMargin(0.15);
    h2D->SetStats(0);
    h2D->Draw("LEGO2Z");
    
    // Y 축 통합 plot - pT 의존성
    TCanvas* c2 = new TCanvas("c2", "D* Yield vs pT", 800, 600);
    TH1D* h1D_pt = h2D->ProjectionX("h1D_pt", 1, h2D->GetNbinsY());
    h1D_pt->SetTitle("D* Yield vs p_{T};p_{T} (GeV/c);Yield");
    h1D_pt->SetMarkerStyle(20);
    h1D_pt->SetMarkerColor(kBlue);
    h1D_pt->Draw("EP");
    
    // X 축 통합 plot - cos θ 의존성
    TCanvas* c3 = new TCanvas("c3", "D* Yield vs cos#theta", 800, 600);
    TH1D* h1D_cos = h2D->ProjectionY("h1D_cos", 1, h2D->GetNbinsX());
    h1D_cos->SetTitle("D* Yield vs cos#theta;cos#theta;Yield");
    h1D_cos->SetMarkerStyle(20);
    h1D_cos->SetMarkerColor(kRed);
    h1D_cos->Draw("EP");
    
    // 결과 저장
    c1->SaveAs("DStar_Yield_2D.pdf");
    c2->SaveAs("DStar_Yield_vs_pT.pdf");
    c3->SaveAs("DStar_Yield_vs_costheta.pdf");
    
    // ROOT 파일로 저장
    TFile* outFile = new TFile("DStar_2D_Results.root", "RECREATE");
    h2D->Write();
    h1D_pt->Write();
    h1D_cos->Write();
    outFile->Close();
    
    std::cout << "Analysis completed. Results saved." << std::endl;
}