#include "../interface/simpleDMC.h"
#include "../fit/Opt.h"
float getEfficiencyVal(double pt, double y, double mva, TH3D *effmap){
	float eff = 0.001;
	int bin = effmap->FindBin(pt,y,mva);
	// cout << "bin: " << bin << endl;
	eff = effmap->GetBinContent(bin);
	// cout << 1/eff << endl;
	return 1/eff;


}
void MVAEfficiency(){
	
	// TFile *fout = new TFile("","RECREATE");
	TFile *feff = TFile::Open("output.root");
	TFile *fin = TFile::Open("/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppData/D0/flatSkimForBDT_D0_PbPb_Data_ONNX_withCent_16Apr.root");
	
	TH3D *effmap = (TH3D*)feff->Get("pt_y_mva_pr_ratio");
	DataFormat::simpleDStarMCTreeflat evtRecoFlat;
	TTree *t = (TTree*)fin->Get("skimTreeFlat");
	evtRecoFlat.setTree(t);
	int entry = t->GetEntries();
	double normalizedMVAEff =0;
	std::vector<double> mvaCutBin = {0.990,0.991,0.992,0.993,0.994,0.995,0.996,0.997,0.998,0.999};
	std::map <double, std::pair<int,double>> mvaCutMap;
	for(int i=0; i<mvaCutBin.size(); i++){
		mvaCutMap[mvaCutBin[i]] = {0,0.};
	}
	auto FillMva = [&](double mvaCut, TH3D *effmap){
		if(evtRecoFlat.mass >1.84 && evtRecoFlat.mass < 1.88 && evtRecoFlat.mva > mvaCut && evtRecoFlat.pT < 100 && evtRecoFlat.pT >2 && abs(evtRecoFlat.y) < 1.0 && evtRecoFlat.pTD1>1 && evtRecoFlat.pTD2>1){
			float eff = getEfficiencyVal(evtRecoFlat.pT,evtRecoFlat.y,mvaCut+0.0001,effmap);
			mvaCutMap[mvaCut].first += 1;
			mvaCutMap[mvaCut].second += eff;

		}
	};
	for(int i=0; i<entry; i++){
		if(entry%100000==0) cout << "processing events : " << entry << endl;
		t->GetEntry(i);
		if(evtRecoFlat.mass >1.84 && evtRecoFlat.mass < 1.88){
			for(auto mvaCut : mvaCutBin){
				FillMva(mvaCut, effmap);
			}
		}
	}
	TFile *fh1 = new TFile("/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/testRun/D0_Yield_mva.root","READ");
	TH1D* h1 = (TH1D*)fh1->Get("D0_Yield_mva");
	TH1D* hCorrect = new TH1D("hCorrect","hCorrect",mvaCutBin.size()-1,mvaCutBin.data());
	TH1D* eff = new TH1D("heff","heff",mvaCutBin.size()-1,mvaCutBin.data());
	for(int i =0; i<h1->GetNbinsX(); i++){
		hCorrect->SetBinContent(i, h1->GetBinContent(i)*mvaCutMap[mvaCutBin[i]].second/mvaCutMap[mvaCutBin[i]].first);
		eff->SetBinContent(i, mvaCutMap[mvaCutBin[i]].second/mvaCutMap[mvaCutBin[i]].first);
		cout << h1->GetBinContent(i) << " " << mvaCutMap[mvaCutBin[i]].second/mvaCutMap[mvaCutBin[i]].first << endl;
	}
	
	TCanvas *c1 = new TCanvas("c1","c1",800,600);
	c1->SaveAs("hCorrect.png");
	hCorrect->Draw();
	TFile *fout = new TFile("efficiency.root","recreate");
	fout->cd();
	eff->Write();
	fout->Close();
	
	
	
	







}
