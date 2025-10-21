#include "../../Correction/EffHead.h"
#include "../../Correction/commonSelectionVar.h"
#include "../../interface/simpleDMC.h"
#include "../../interface/simpleAlgos.hxx"
#include "../../Tools/Transformations.h"
#include "../../Tools/BasicHeaders.h"
#include "../../Tools/Parameters/AnalysisParameters.h"
#include "../../Tools/Parameters/PhaseSpace.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TLine.h"


TH2* DrawCosThetaPhiDistribution(TH2D *histo, TCanvas *canvas,  const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
    Double_t massMin = 1.9, massMax = 2.1;

    Int_t nCosThetaBins = 20;
    Float_t cosThetaMin = -1, cosThetaMax = 1;

    Int_t nPhiBins = 25;
    Float_t phiMin = -200, phiMax = 300;
    /// Draw and save the number of events(signal+background) plots in the 2D (costheta, phi) space
    const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

    histo->SetTitle(Form(";cos #theta_{%s}; #varphi_{%s} (#circ)", frameAcronym, frameAcronym));
    histo->Draw("COLZ");

    TLatex legend;
    legend.SetTextAlign(22);
    legend.SetTextSize(0.05);
    legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

    histo->GetXaxis()->CenterTitle();
    histo->GetYaxis()->SetRangeUser(-200, 300);
    histo->GetYaxis()->CenterTitle();
    histo->GetZaxis()->SetMaxDigits(3);

    gPad->Update();

    return histo;
}

TH1* DrawCosThetaDistribution(TH1D *histo, TCanvas *canvas, TString drawoption="", const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
    Double_t massMin = 1.9, massMax = 2.1;

    Int_t nCosThetaBins = 20;
    Float_t cosThetaMin = -1, cosThetaMax = 1;

    const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

    histo->SetTitle(Form(";cos (#theta_{%s})", frameAcronym));
    histo->Draw(drawoption.Data());
    histo->GetYaxis()->SetRangeUser(0,histo->GetMaximum()*1.2);

    TLatex legend;
    legend.SetTextAlign(22);
    legend.SetTextSize(0.05);
    legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

    histo->GetXaxis()->CenterTitle();
    histo->GetYaxis()->SetRangeUser(0, histo->GetMaximum()*1.2);

    gPad->Update();

    return histo;
}

double findWeight(Float_t pt, Float_t y, Float_t phi, TH3D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(pt, y, phi);
        double eff = effMap->GetBinContent(bin);
        weight = 1.0/eff;
    }
    return weight;
}

double findWeight(Float_t x, Float_t y, TH2D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(x, y);
        double eff = effMap->GetBinContent(bin);
        if (eff > 0){
            weight = 1.0/eff;
        }
    }
    return weight;
}

double findWeight(Float_t x, TH1D* effMap){
    double weight = 1.0;
    if (effMap){
        Int_t bin = effMap->FindBin(x);
        double eff = effMap->GetBinContent(bin);
        weight = 1.0/eff;
    }
    return weight;
}

void FlatTest(
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbMC/DStar/flatSkimForBDT_DStar_dstar_PbPb_MC_Aug21.root"
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbMC/DStar/0822/flatSkimForBDT_DStar_dstar_PbPb_MC_wEvtplane_Aug22.root"
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/PbPbMC/DStar/0822/flatSkimForBDT_DStar_dstar_PbPb_MC_wEvtplane_Aug22.root"
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/0908/flatSkimForBDT_DStar_Dstar_NonPrompt_MC_08Sep25_0_.root"
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/0908/flatSkimForBDT_DStar_Dstar_Prompt_MC_08Sep25_0_.root"
    // string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/0916/flatSkimForBDT_DStar_DStar_Prompt_ppRef_16Sep25_1_.root"
    string fileMCPR="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/0916/flatSkimForBDT_DStar_DStar_MixPU_ppRef_MC_Sep16_v1_1_.root"
){
    // Open MC file
    TFile* MCPR = TFile::Open(fileMCPR.c_str());
    if (!MCPR) {
        std::cerr << "Error opening input file" << std::endl;
        return;
    }
    
    // Get the flat trees
    TTree* tGen = (TTree*)MCPR->Get("skimGENTreeFlat");
    
    if (!tGen) {
        std::cerr << "Error getting GEN tree from file" << std::endl;
        return;
    }
    
    // Set up data structures for flat trees
    DataFormat::simpleDStarMCTreeflat evtGenFlat;
    
    // Set branch addresses
    evtGenFlat.setGENTree(tGen);
    
    int nGenEntries = tGen->GetEntries();
    std::cout << "Gen entries: " << nGenEntries << std::endl;
    
    // Define pT bins
    vector<double> ptBins = {5, 7, 10, 20, 50};
    int nPtBins = ptBins.size() - 1;
    
    // Create histograms for each pT bin and prompt/non-prompt
    vector<TH1D*> h_prompt, h_nonprompt;
    vector<TF1*> fitFunc_prompt, fitFunc_nonprompt;
    
    // For storing fit results
    vector<double> ptCenter, ptError;
    vector<double> rho00_prompt_values, rho00_prompt_errors;
    vector<double> rho00_nonprompt_values, rho00_nonprompt_errors;
    
    for (int i = 0; i < nPtBins; i++) {
        h_prompt.push_back(new TH1D(Form("h_prompt_pt%.0fto%.0f", ptBins[i], ptBins[i+1]), 
                                   Form("Prompt D* (%.0f < p_{T} < %.0f GeV/c);cos(#theta_{HX});Entries", ptBins[i], ptBins[i+1]), 
                                   20, -1, 1));
        h_nonprompt.push_back(new TH1D(Form("h_nonprompt_pt%.0fto%.0f", ptBins[i], ptBins[i+1]), 
                                      Form("Non-prompt D* (%.0f < p_{T} < %.0f GeV/c);cos(#theta_{HX});Entries", ptBins[i], ptBins[i+1]), 
                                      20, -1, 1));
        
        h_prompt[i]->Sumw2(true);
        h_nonprompt[i]->Sumw2(true);
        
        // Create fit functions: N0 * (1 - rho00 + (3*rho00 - 1)*cos^2(theta))
        fitFunc_prompt.push_back(new TF1(Form("fitFunc_prompt_%d", i), "[0]*(1 - [1] + (3*[1] - 1)*x*x)", -1, 1));
        fitFunc_nonprompt.push_back(new TF1(Form("fitFunc_nonprompt_%d", i), "[0]*(1 - [1] + (3*[1] - 1)*x*x)", -1, 1));
        
        // Set parameter names and initial values
        fitFunc_prompt[i]->SetParName(0, "N_{0}");
        fitFunc_prompt[i]->SetParName(1, "#rho_{00}");
        fitFunc_prompt[i]->SetParameter(0, 1000);
        fitFunc_prompt[i]->SetParameter(1, 1.0/3.0);
        fitFunc_prompt[i]->SetParLimits(1, 0.0, 1.0);
        
        fitFunc_nonprompt[i]->SetParName(0, "N_{0}");
        fitFunc_nonprompt[i]->SetParName(1, "#rho_{00}");
        fitFunc_nonprompt[i]->SetParameter(0, 1000);
        fitFunc_nonprompt[i]->SetParameter(1, 1.0/3.0);
        fitFunc_nonprompt[i]->SetParLimits(1, 0.0, 1.0);
        
        // Calculate pT center and error
        double pt_center = (ptBins[i] + ptBins[i+1]) / 2.0;
        double pt_width = (ptBins[i+1] - ptBins[i]) / 2.0;
        ptCenter.push_back(pt_center);
        ptError.push_back(pt_width);
    }

    // Process GEN tree entries
    for (int iEntry = 0; iEntry < nGenEntries; iEntry++) {
        tGen->GetEntry(iEntry);
        
        // Apply basic selection cuts
        if (fabs(evtGenFlat.gen_y) >= 1.0) continue;
        
        // Create 4-vectors
        TLorentzVector Dstar, D0, D1, D0Dau1, D0Dau2;
        Dstar.SetPtEtaPhiM(evtGenFlat.gen_pT, evtGenFlat.gen_eta, evtGenFlat.gen_phi, evtGenFlat.gen_mass);
        D0.SetPtEtaPhiM(evtGenFlat.gen_D0pT, evtGenFlat.gen_D0eta, evtGenFlat.gen_D0phi, evtGenFlat.gen_D0mass);
        D1.SetPtEtaPhiM(evtGenFlat.gen_D1pT, evtGenFlat.gen_D1eta, evtGenFlat.gen_D1phi, evtGenFlat.gen_D1mass);
        D0Dau1.SetPtEtaPhiM(evtGenFlat.gen_D0Dau1_pT, evtGenFlat.gen_D0Dau1_eta, evtGenFlat.gen_D0Dau1_phi, evtGenFlat.gen_D0Dau1_mass);
        D0Dau2.SetPtEtaPhiM(evtGenFlat.gen_D0Dau2_pT, evtGenFlat.gen_D0Dau2_eta, evtGenFlat.gen_D0Dau2_phi, evtGenFlat.gen_D0Dau2_mass);
        
        // Apply acceptance cuts
        // if (!DstarDauSimpleAcc(D0, D1) || !D0DauAcc(D0Dau1, D0Dau2)) continue;
 
        // Calculate helicity cos(theta)
        TVector3 vect = DstarDau1Vector_Helicity(Dstar, D0);
        // TVector3 vect = DstarDau1Vector_EventPlane(Dstar, D0, evtGenFlat.Psi2Raw_Trk );
        double cosTheta = vect.CosTheta();
        
        // Determine prompt/non-prompt
        bool isPrompt = (evtGenFlat.gen_D0ancestorFlavor_ != 5);
        
        // Fill histograms for each pT bin
        for (int i = 0; i < nPtBins; i++) {
            if (evtGenFlat.gen_pT >= ptBins[i] && evtGenFlat.gen_pT < ptBins[i+1]) {
                if (isPrompt) {
                    h_prompt[i]->Fill(cosTheta);
                } else {
                    h_nonprompt[i]->Fill(cosTheta);
                }
                break;
            }
        }
    }
    
    // Create canvas and draw histograms
    TCanvas* c = new TCanvas("c", "Cos(theta) distributions by pT bins", 1600, 1200);
    c->Divide(2, nPtBins);
    
    gStyle->SetOptStat(0);
    
    for (int i = 0; i < nPtBins; i++) {
        // Draw prompt histograms
        c->cd(2*i + 1);
        h_prompt[i]->SetLineColor(kRed);
        h_prompt[i]->SetMarkerColor(kRed);
        h_prompt[i]->SetMarkerStyle(20);
        h_prompt[i]->SetMarkerSize(0.8);
        h_prompt[i]->GetYaxis()->SetRangeUser(0, h_prompt[i]->GetMaximum() * 1.2);
        h_prompt[i]->Draw("E");
        
        TLatex* tex1 = new TLatex(0.15, 0.85, "Prompt D*");
        tex1->SetNDC();
        tex1->SetTextSize(0.06);
        tex1->SetTextColor(kRed);
        tex1->Draw();
        
        // Draw non-prompt histograms
        c->cd(2*i + 2);
        h_nonprompt[i]->SetLineColor(kBlue);
        h_nonprompt[i]->SetMarkerColor(kBlue);
        h_nonprompt[i]->SetMarkerStyle(21);
        h_nonprompt[i]->SetMarkerSize(0.8);
        h_nonprompt[i]->GetYaxis()->SetRangeUser(0, h_nonprompt[i]->GetMaximum() * 1.2);
        h_nonprompt[i]->Draw("E");
        
        TLatex* tex2 = new TLatex(0.15, 0.85, "Non-prompt D*");
        tex2->SetNDC();
        tex2->SetTextSize(0.06);
        tex2->SetTextColor(kBlue);
        tex2->Draw();
    }
    
    c->SaveAs("cos_theta_pT_bins.png");
    c->SaveAs("cos_theta_pT_bins.pdf");
    
    // Perform fits for each pT bin
    gStyle->SetOptStat(0);
    std::cout << "\n=== Performing fits ===" << std::endl;
    
    for (int i = 0; i < nPtBins; i++) {
        std::cout << Form("Fitting pT bin %.0f-%.0f GeV/c", ptBins[i], ptBins[i+1]) << std::endl;
        
        // Fit prompt histogram
        if (h_prompt[i]->GetEntries() > 50) {
            fitFunc_prompt[i]->SetParLimits(0, 0, h_prompt[i]->GetMaximum() * 1.5);
            TFitResultPtr fitResult_prompt = h_prompt[i]->Fit(fitFunc_prompt[i], "SER", "", -0.9, 0.9);
            
            if (fitResult_prompt->IsValid()) {
                double rho00 = fitFunc_prompt[i]->GetParameter(1);
                double rho00_err = fitFunc_prompt[i]->GetParError(1);
                rho00_prompt_values.push_back(rho00);
                rho00_prompt_errors.push_back(rho00_err);
                std::cout << "  Prompt: ρ₀₀ = " << rho00 << " ± " << rho00_err << std::endl;
            } else {
                rho00_prompt_values.push_back(1.0/3.0);
                rho00_prompt_errors.push_back(0.1);
                std::cout << "  Prompt: Fit failed, using default value" << std::endl;
            }
        } else {
            rho00_prompt_values.push_back(1.0/3.0);
            rho00_prompt_errors.push_back(0.1);
            std::cout << "  Prompt: Too few entries for fit" << std::endl;
        }
        
        // Fit nonprompt histogram
        if (h_nonprompt[i]->GetEntries() > 50) {
            fitFunc_nonprompt[i]->SetParLimits(0, 0, h_nonprompt[i]->GetMaximum() * 1.5);
            TFitResultPtr fitResult_nonprompt = h_nonprompt[i]->Fit(fitFunc_nonprompt[i], "SER", "", -0.9, 0.9);
            
            if (fitResult_nonprompt->IsValid()) {
                double rho00 = fitFunc_nonprompt[i]->GetParameter(1);
                double rho00_err = fitFunc_nonprompt[i]->GetParError(1);
                rho00_nonprompt_values.push_back(rho00);
                rho00_nonprompt_errors.push_back(rho00_err);
                std::cout << "  Non-prompt: ρ₀₀ = " << rho00 << " ± " << rho00_err << std::endl;
            } else {
                rho00_nonprompt_values.push_back(1.0/3.0);
                rho00_nonprompt_errors.push_back(0.1);
                std::cout << "  Non-prompt: Fit failed, using default value" << std::endl;
            }
        } else {
            rho00_nonprompt_values.push_back(1.0/3.0);
            rho00_nonprompt_errors.push_back(0.1);
            std::cout << "  Non-prompt: Too few entries for fit" << std::endl;
        }
    }
    
    // Create overlay plots
    TCanvas* c2 = new TCanvas("c2", "Cos(theta) distributions overlay", 1200, 800);
    c2->Divide(3, 2);
    
    for (int i = 0; i < nPtBins; i++) {
        c2->cd(i + 1);
        
        // Normalize histograms for comparison
        TH1D* h_prompt_norm = (TH1D*)h_prompt[i]->Clone(Form("h_prompt_norm_%d", i));
        TH1D* h_nonprompt_norm = (TH1D*)h_nonprompt[i]->Clone(Form("h_nonprompt_norm_%d", i));
        
        // if (h_prompt_norm->Integral() > 0) h_prompt_norm->Scale(1.0 / h_prompt_norm->Integral());
        // if (h_nonprompt_norm->Integral() > 0) h_nonprompt_norm->Scale(1.0 / h_nonprompt_norm->Integral());
        
        double maxY = TMath::Max(h_prompt_norm->GetMaximum(), h_nonprompt_norm->GetMaximum()) * 1.2;
        
        h_prompt_norm->SetLineColor(kRed);
        h_prompt_norm->SetMarkerColor(kRed);
        h_prompt_norm->SetMarkerStyle(20);
        h_prompt_norm->GetYaxis()->SetRangeUser(0, maxY);
        h_prompt_norm->SetTitle(Form("%.0f < p_{T} < %.0f GeV/c;cos(#theta_{HX});Normalized Entries", ptBins[i], ptBins[i+1]));
        h_prompt_norm->Draw("E");
        
        h_nonprompt_norm->SetLineColor(kBlue);
        h_nonprompt_norm->SetMarkerColor(kBlue);
        h_nonprompt_norm->SetMarkerStyle(21);
        h_nonprompt_norm->Draw("E same");
        
        TLegend* leg = new TLegend(0.65, 0.75, 0.88, 0.88);
        leg->AddEntry(h_prompt_norm, "Prompt", "lp");
        leg->AddEntry(h_nonprompt_norm, "Non-prompt", "lp");
        leg->SetBorderSize(0);
        leg->Draw();
    }
    
    c2->SaveAs("cos_theta_pT_bins_overlay.png");
    c2->SaveAs("cos_theta_pT_bins_overlay.pdf");
    
    // Create individual fit plots for each pT bin
    for (int i = 0; i < nPtBins; i++) {
        TCanvas* cFit = new TCanvas(Form("cFit_%d", i), 
                                   Form("Fits for pT = %.0f-%.0f GeV/c", ptBins[i], ptBins[i+1]), 
                                   1200, 600);
        cFit->Divide(2, 1);
        
        // Prompt fit plot
        cFit->cd(1);
        cFit->cd(1)->SetLeftMargin(0.15);
        cFit->cd(1)->SetBottomMargin(0.15);
        
        h_prompt[i]->SetMarkerStyle(20);
        h_prompt[i]->SetMarkerSize(1.0);
        h_prompt[i]->SetMarkerColor(kRed);
        h_prompt[i]->SetLineColor(kRed);
        h_prompt[i]->GetXaxis()->SetTitle("cos#theta_{HX}");
        h_prompt[i]->GetYaxis()->SetTitle("dN/dcos#theta");
        h_prompt[i]->GetYaxis()->SetTitleOffset(1.4);
        h_prompt[i]->Draw("EP");
        
        if (h_prompt[i]->GetEntries() > 50) {
            fitFunc_prompt[i]->SetLineColor(kRed);
            fitFunc_prompt[i]->SetLineWidth(2);
            fitFunc_prompt[i]->Draw("same");
            
            // Add fit info
            TLatex* texPrompt = new TLatex(0.2, 0.85, "Prompt D*");
            texPrompt->SetNDC();
            texPrompt->SetTextSize(0.05);
            texPrompt->SetTextColor(kRed);
            texPrompt->Draw();
            
            double rho00_val = fitFunc_prompt[i]->GetParameter(1);
            double rho00_err = fitFunc_prompt[i]->GetParError(1);
            TLatex* texRho = new TLatex(0.2, 0.75, Form("#rho_{00} = %.3f #pm %.3f", rho00_val, rho00_err));
            texRho->SetNDC();
            texRho->SetTextSize(0.04);
            texRho->Draw();
            
            TLatex* texEq = new TLatex(0.2, 0.65, "N_{0}(1-#rho_{00}+(3#rho_{00}-1)cos^{2}#theta)");
            texEq->SetNDC();
            texEq->SetTextSize(0.035);
            texEq->Draw();
        }
        
        // Non-prompt fit plot
        cFit->cd(2);
        cFit->cd(2)->SetLeftMargin(0.15);
        cFit->cd(2)->SetBottomMargin(0.15);
        
        h_nonprompt[i]->SetMarkerStyle(21);
        h_nonprompt[i]->SetMarkerSize(1.0);
        h_nonprompt[i]->SetMarkerColor(kBlue);
        h_nonprompt[i]->SetLineColor(kBlue);
        h_nonprompt[i]->GetXaxis()->SetTitle("cos#theta_{HX}");
        h_nonprompt[i]->GetYaxis()->SetTitle("dN/dcos#theta");
        h_nonprompt[i]->GetYaxis()->SetTitleOffset(1.4);
        h_nonprompt[i]->Draw("EP");
        
        if (h_nonprompt[i]->GetEntries() > 50) {
            fitFunc_nonprompt[i]->SetLineColor(kBlue);
            fitFunc_nonprompt[i]->SetLineWidth(2);
            fitFunc_nonprompt[i]->Draw("same");
            
            // Add fit info
            TLatex* texNonprompt = new TLatex(0.2, 0.85, "Non-prompt D*");
            texNonprompt->SetNDC();
            texNonprompt->SetTextSize(0.05);
            texNonprompt->SetTextColor(kBlue);
            texNonprompt->Draw();
            
            double rho00_val = fitFunc_nonprompt[i]->GetParameter(1);
            double rho00_err = fitFunc_nonprompt[i]->GetParError(1);
            TLatex* texRho = new TLatex(0.2, 0.75, Form("#rho_{00} = %.3f #pm %.3f", rho00_val, rho00_err));
            texRho->SetNDC();
            texRho->SetTextSize(0.04);
            texRho->Draw();
            
            TLatex* texEq = new TLatex(0.2, 0.65, "N_{0}(1-#rho_{00}+(3#rho_{00}-1)cos^{2}#theta)");
            texEq->SetNDC();
            texEq->SetTextSize(0.035);
            texEq->Draw();
        }
        
        cFit->SaveAs(Form("cosTheta_fits_pt%.0f_%.0f.png", ptBins[i], ptBins[i+1]));
        cFit->SaveAs(Form("cosTheta_fits_pt%.0f_%.0f.pdf", ptBins[i], ptBins[i+1]));
        delete cFit;
    }
    
    // Create rho00 vs pT plots
    TCanvas* c3 = new TCanvas("c3", "rho00 vs pT", 1000, 800);
    c3->SetLeftMargin(0.12);
    c3->SetBottomMargin(0.12);
    c3->SetTopMargin(0.08);
    c3->SetRightMargin(0.05);
    
    // Create TGraphErrors for prompt and nonprompt
    TGraphErrors* gr_rho00_prompt = new TGraphErrors(ptCenter.size(), 
                                                     &ptCenter[0], &rho00_prompt_values[0],
                                                     &ptError[0], &rho00_prompt_errors[0]);
    TGraphErrors* gr_rho00_nonprompt = new TGraphErrors(ptCenter.size(), 
                                                        &ptCenter[0], &rho00_nonprompt_values[0],
                                                        &ptError[0], &rho00_nonprompt_errors[0]);
    
    // Style prompt graph
    gr_rho00_prompt->SetMarkerStyle(20);
    gr_rho00_prompt->SetMarkerSize(1.2);
    gr_rho00_prompt->SetMarkerColor(kRed);
    gr_rho00_prompt->SetLineColor(kRed);
    gr_rho00_prompt->SetLineWidth(2);
    
    // Style nonprompt graph
    gr_rho00_nonprompt->SetMarkerStyle(21);
    gr_rho00_nonprompt->SetMarkerSize(1.2);
    gr_rho00_nonprompt->SetMarkerColor(kBlue);
    gr_rho00_nonprompt->SetLineColor(kBlue);
    gr_rho00_nonprompt->SetLineWidth(2);
    
    // Axis setup
    gr_rho00_prompt->GetXaxis()->SetTitle("p_{T} (GeV/c)");
    gr_rho00_prompt->GetYaxis()->SetTitle("#rho_{00}");
    gr_rho00_prompt->GetXaxis()->SetRangeUser(0, 50);
    gr_rho00_prompt->GetYaxis()->SetRangeUser(0, 0.7);
    
    gr_rho00_prompt->GetXaxis()->SetTitleSize(0.05);
    gr_rho00_prompt->GetXaxis()->SetTitleOffset(1.0);
    gr_rho00_prompt->GetXaxis()->SetLabelSize(0.04);
    gr_rho00_prompt->GetXaxis()->SetLabelFont(42);
    gr_rho00_prompt->GetXaxis()->SetTitleFont(42);
    
    gr_rho00_prompt->GetYaxis()->SetTitleSize(0.05);
    gr_rho00_prompt->GetYaxis()->SetTitleOffset(1.2);
    gr_rho00_prompt->GetYaxis()->SetLabelSize(0.04);
    gr_rho00_prompt->GetYaxis()->SetLabelFont(42);
    gr_rho00_prompt->GetYaxis()->SetTitleFont(42);
    
    gr_rho00_prompt->Draw("AP");
    gr_rho00_nonprompt->Draw("P same");
    
    // Add 1/3 line (unpolarized limit)
    double xmin = gr_rho00_prompt->GetXaxis()->GetXmin();
    double xmax = gr_rho00_prompt->GetXaxis()->GetXmax();
    TLine* line_unpol = new TLine(xmin, 1./3., xmax, 1./3.);
    line_unpol->SetLineStyle(2);
    line_unpol->SetLineWidth(2);
    line_unpol->SetLineColor(kGray+2);
    line_unpol->Draw();
    
    // Legend
    TLegend* leg3 = new TLegend(0.15, 0.65, 0.55, 0.92);
    leg3->SetBorderSize(0);
    leg3->SetFillStyle(0);
    leg3->SetTextSize(0.04);
    leg3->SetTextFont(42);
    
    leg3->AddEntry(gr_rho00_prompt, "Prompt D*", "lp");
    leg3->AddEntry(gr_rho00_nonprompt, "Non-prompt D*", "lp");
    leg3->AddEntry(line_unpol, "No spin alignment (#rho_{00} = 1/3)", "l");
    leg3->AddEntry((TObject*)0, "", "");
    leg3->AddEntry((TObject*)0, "Fit function:", "");
    leg3->AddEntry((TObject*)0, "N_{0}[1-#rho_{00}+(3#rho_{00}-1)cos^{2}#theta_{HX}]", "");
    leg3->Draw();
    
    // Unpolarized limit label
    TLatex* unpol_label = new TLatex(30, 0.36, "no spin alignment");
    unpol_label->SetTextSize(0.03);
    unpol_label->SetTextFont(42);
    unpol_label->SetTextColor(kGray+2);
    unpol_label->Draw();
    
    c3->SaveAs("rho00_vs_pT_comparison.png");
    c3->SaveAs("rho00_vs_pT_comparison.pdf");
    
    // Print statistics and fit results
    std::cout << "\n=== Statistics and Fit Results ===" << std::endl;
    for (int i = 0; i < nPtBins; i++) {
        std::cout << Form("pT bin %.0f-%.0f GeV/c:", ptBins[i], ptBins[i+1]) << std::endl;
        std::cout << Form("  Entries: Prompt = %.0f, Non-prompt = %.0f", 
                         h_prompt[i]->GetEntries(), 
                         h_nonprompt[i]->GetEntries()) << std::endl;
        if (i < rho00_prompt_values.size()) {
            std::cout << Form("  Prompt ρ₀₀: %.4f ± %.4f", rho00_prompt_values[i], rho00_prompt_errors[i]) << std::endl;
        }
        if (i < rho00_nonprompt_values.size()) {
            std::cout << Form("  Non-prompt ρ₀₀: %.4f ± %.4f", rho00_nonprompt_values[i], rho00_nonprompt_errors[i]) << std::endl;
        }
    }
    
    std::cout << "\n=== Fit Function ===" << std::endl;
    std::cout << "dN/dcos(θ) = N₀ × [1 - ρ₀₀ + (3ρ₀₀ - 1)cos²(θ)]" << std::endl;
    std::cout << "where ρ₀₀ is the spin density matrix element (00 component)" << std::endl;
    std::cout << "ρ₀₀ = 1/3 corresponds to no spin alignment (unpolarized)" << std::endl;
    std::cout << "ρ₀₀ < 1/3 indicates transverse polarization" << std::endl;
    std::cout << "ρ₀₀ > 1/3 indicates longitudinal polarization" << std::endl;
    
    // Clean up graphs and canvas
    delete c3;
    delete gr_rho00_prompt;
    delete gr_rho00_nonprompt;
    delete leg3;
    delete line_unpol;
    delete unpol_label;
    
    // Clean up
    MCPR->Close();
}
