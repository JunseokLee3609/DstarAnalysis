void DrawSelectionCut(){
    // TFile *f = TFile::Open("2Dhistogram.root");
    float etacut = 1.0;
    TFile *f1 = TFile::Open("2Dhistogram_test.root");
    TH2D *h2dEta = (TH2D*)f1->Get("h2_EtaD1_EtaD2");
    TH2D *h2dpTD = (TH2D*)f1->Get("h2_pTD1_pTD2");
    TH2D *h2dEtaGen = (TH2D*)f1->Get("hgen_h2_EtaD1_EtaD2");
    TH2D *h2dpTDGen = (TH2D*)f1->Get("hgen_h2_pTD1_pTD2");
    TCanvas *cvs = new TCanvas("cvs","cvs",600,600);
    h2dEta->SetTitle("");
    h2dEta->Draw("colz");
    TF1 *curve = new TF1("curve", "[0]*x + [1]", -10, 10);
    curve->SetParameters(1, -1*etacut); 
    curve->SetLineColor(kGreen);
    curve->Draw("same");
    TF1 *curve1 = new TF1("curve1", "[0]*x + [1]", -10, 10);
    curve1->SetParameters(1, etacut); 
    curve1->SetLineColor(kGreen);
    curve1->Draw("same");
    TF1 *curve2 = new TF1("curve2", "[0]*x + [1]", -10, 10);
    curve2->SetParameters(-1, 5); 
    curve2->SetLineColor(kGreen);
    // curve2->Draw("same");
    TF1 *curve3 = new TF1("curve3", "[0]*x + [1]", -10, 10);
    curve3->SetParameters(-1, 1.6); 
    curve3->SetLineColor(kGreen);
    // curve3->Draw("same");
    TCanvas *cvs1 = new TCanvas("cvs1","cvs1",600,600);
    h2dpTD->SetTitle("");
    h2dpTD->Draw("colz");
    curve2->Draw("same");
    curve3->Draw("same");
    TCanvas *cvsGen = new TCanvas("cvsGen","cvsGen",600,600);
    h2dEtaGen->SetTitle("");
    h2dEtaGen->Draw("colz");
    curve->SetParameters(1, -1*etacut); 
    curve->SetLineColor(kGreen);
    curve->Draw("same");
    curve1->SetParameters(1, etacut); 
    curve1->SetLineColor(kGreen);
    curve1->Draw("same");
    curve2->SetParameters(-1, 5); 
    curve2->SetLineColor(kGreen);
    // curve2->Draw("same");
    curve3->SetParameters(-1, 1.6); 
    curve3->SetLineColor(kGreen);
    // curve3->Draw("same");
    TCanvas *cvsGen1 = new TCanvas("cvsGen1","cvsGen1",600,600);
    h2dpTDGen->SetTitle("");
    h2dpTDGen->Draw("colz");
    curve2->Draw("same");
    curve3->Draw("same");

    cvs->SaveAs("1DHisto/EtaDiff.pdf");
    cvs1->SaveAs("1DHisto/pTSum.pdf");
    cvsGen->SaveAs("1DHisto/EtaDiffGen.pdf");
    cvsGen1->SaveAs("1DHisto/pTSumGen.pdf");
    
    
     

}