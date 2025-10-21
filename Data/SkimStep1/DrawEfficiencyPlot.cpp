void loadRootFilesToTChain(TChain* chain, const std::string& folderPath) {

    TSystemDirectory dir(folderPath.c_str(), folderPath.c_str());
    TList* files = dir.GetListOfFiles();

    if (files) {
        TSystemFile* file;
        TIter next(files);

        while ((file = (TSystemFile*)next())) {
            std::string fileName = file->GetName();
            if (!(file->IsZombie()) &&fileName.find(".root") != std::string::npos) {
                std::string filePath = folderPath + "/" + fileName;
                chain->Add(filePath.c_str());
            }
        }
    }

    if (chain->GetEntries() > 0) {
    } else {
    }
}
void DrawEfficiencyPlot(){
	gStyle->SetOptStat(0);

        TChain* ch1 = new TChain("d0ana_newreduced/PATCompositeNtuple");
        //loadRootFilesToTChain(ch1,".");
        ch1->Add("d0ana_tree_12.root");
        ch1->Add("d0ana_tree_11.root");
        ch1->Add("d0ana_tree_129.root");
        ch1->Add("d0ana_tree_150.root");
        ch1->Add("d0ana_tree_146.root");
        ch1->Add("d0ana_tree_147.root");
        ch1->Add("d0ana_tree_149.root");
        cout << "add" << endl;
        vector<std::string> vars={"3DPointingAngle","pTD1","pTD2","EtaD1","EtaD2","VtxProb","3DDecayLength","3DDecayLengthSignificance","pT","y"};
        TH1D *h1[vars.size()];
        TH1D *hmatchGEN[vars.size()];
        TLegend *leg[vars.size()];
        TCanvas *cvs[vars.size()];
        for(int i =0;i < vars.size(); i ++){
	if(!(vars[i].compare("y")==0)) continue;
        //for(int i =0;i < 2; i ++){
        leg[i]=new TLegend(0.7,0.7,0.9,0.9);
        cvs[i]=new TCanvas(Form("cvs_%d",i),Form("cvs_%d",i),800,600);
        if(vars[i].find("Pointing")!=std::string::npos){
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,0,4);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,0,4);
        }
        else if(vars[i].compare("y")==0){
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,-1.5,1.5);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,-1.5,1.5);
        }
        else if(vars[i].find("Eta")!=std::string::npos){
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,-4,4);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,-4,4);
        }
        else if(vars[i].find("VtxProb")!=std::string::npos){
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),20,0,1);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),20,0,1);
        }
        else if(vars[i].find("Hit")!=std::string::npos){
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),20,0,20);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,0,20);
        h1[i]->GetXaxis()->SetRangeUser(0,20);
        hmatchGEN[i]->GetXaxis()->SetRangeUser(0,20);
        }        
        else if(vars[i].find("pTD1")!=std::string::npos || vars[i].find("pTD2")!=std::string::npos){
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,0,10);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),30,0,10);
        h1[i]->GetXaxis()->SetRangeUser(0,10);
        hmatchGEN[i]->GetXaxis()->SetRangeUser(0,10);
        }
        else if(vars[i].find("Decay")!=std::string::npos){
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),20,0,10);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),20,0,10);
        h1[i]->GetXaxis()->SetRangeUser(0,10);
        hmatchGEN[i]->GetXaxis()->SetRangeUser(0,10);
        }
	else{
        h1[i]= new TH1D(Form("h_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),100,0,30);
        hmatchGEN[i]= new TH1D(Form("hmatchGEN_%s",vars[i].c_str()),Form("h_%s",vars[i].c_str()),100,0,30);
}
        ch1->Draw(Form("%s>>h_%s",vars[i].c_str(),vars[i].c_str()));
        ch1->Draw(Form("%s>>hmatchGEN_%s",vars[i].c_str(),vars[i].c_str()),"matchGEN==1");
        if (h1[i]->Integral() > 0) {
            h1[i]->Scale(1.0 / h1[i]->Integral());
        }
        if (hmatchGEN[i]->Integral() > 0) {
            hmatchGEN[i]->Scale(1.0 / hmatchGEN[i]->Integral());
        }
        leg[i]->AddEntry(h1[i],Form("h_%s",vars[i].c_str()));
        leg[i]->AddEntry(hmatchGEN[i],Form("hmatchGEN_%s",vars[i].c_str()));
	leg[i]->SetBorderSize(0);
        // h1[i]->GetYaxis()->SetRangeUser(0,1);
        h1[i]->SetLineColor(kRed);
        hgen[i]->SetLineColor(kBlue);
        cvs[i]->cd();
        if(h1[i]->GetMaximum()>hgen[i]->GetMaximum() ){
        h1[i]->Draw();
        hgen[i]->Draw("same");}
        else{
        hgen[i]->Draw();
        h1[i]->Draw("same");
        }
	leg[i]->Draw("same");
	cvs[i]->SaveAs(Form("%sdistribution.pdf",vars[i].c_str()));
        }
}
