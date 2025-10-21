void drawplot(){
	gStyle->SetOptStat(0);
	TFile* f1= TFile::Open("d0ana_tree.root");
	TTree* t1 = (TTree*)f1->Get("dStarana_mc/PATCompositeNtuple");
	TFile* f2= TFile::Open("d0ana_tree_brich.root");
	TTree* t2 = (TTree*)f2->Get("dStarana_mc/PATCompositeNtuple");
	int promptID, nonpromptID;
	TH1D *h1 = new TH1D("prompt","gen_D0ancestorId_",600,1,600);
	TH1D *h2 = new TH1D("nonprompt","nonprompt",600,1,600);
	h1->SetLineColor(kRed);
	h2->SetLineColor(kBlue);
//	t1->SetBranchAddress("gen_D0ancestorId_",&promptID);
//	t2->SetBranchAddress("gen_D0ancestorId_",&nonpromptID);
	t1->Draw("gen_D0ancestorId_>>prompt");
	t2->Draw("gen_D0ancestorId_>>nonprompt");
	TCanvas *cvs = new TCanvas("c1","c1",600,800);
	cvs->SetTitle("gen_DstarancestorID");
	TLegend *leg = new TLegend(0.7,0.7,0.9,0.9);
	leg->AddEntry(h1,"prompt");
	leg->AddEntry(h2,"nonprompt");
	cvs->cd();
	h1->Draw();
	h2->Draw("same");
	leg->Draw("same");

	
	

	
	
}
