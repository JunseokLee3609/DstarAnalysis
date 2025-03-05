void D0pTEtaMap(){
    const char* filename = "/home/jun502s/DstarAna/DStarAnalysis/Data/flatSkimForBDT_isMC1_noFilter_wOnlyGEN_0_250108.root";

	TFile* file = TFile::Open(filename, "READ");
	if (!file) {
		cout << "File " << filename << " not found. Check the directory of the file." << endl;
		return;
    }
    TTree *GenTree = (TTree*)file->Get("skimGENTreeFlat");
    Float_t Dstar_pT, Dstar_y, Dstar_eta, Dstar_phi, Dstar_mass;
    Float_t Dstar_Dau1_pT, Dstar_Dau1_y, Dstar_Dau1_eta, Dstar_Dau1_phi, Dstar_Dau1_mass;
    Float_t Dstar_Dau2_pT, Dstar_Dau2_y, Dstar_Dau2_eta, Dstar_Dau2_phi, Dstar_Dau2_mass;

    // Set branch addresses
    GenTree->SetBranchAddress("gen_mass", &Dstar_mass);
    GenTree->SetBranchAddress("gen_y", &Dstar_y);
    GenTree->SetBranchAddress("gen_pT", &Dstar_pT);
    GenTree->SetBranchAddress("gen_eta", &Dstar_eta);
    GenTree->SetBranchAddress("gen_phi", &Dstar_phi);
    GenTree->SetBranchAddress("gen_D0pT", &Dstar_Dau1_pT);
    GenTree->SetBranchAddress("gen_D0eta", &Dstar_Dau1_eta);
    GenTree->SetBranchAddress("gen_D0phi", &Dstar_Dau1_phi);
    GenTree->SetBranchAddress("gen_D0mass", &Dstar_Dau1_mass);
    GenTree->SetBranchAddress("gen_D1pT", &Dstar_Dau2_pT);
    GenTree->SetBranchAddress("gen_D1eta", &Dstar_Dau2_eta);
    GenTree->SetBranchAddress("gen_D1phi", &Dstar_Dau2_phi);
    GenTree->SetBranchAddress("gen_D1mass", &Dstar_Dau2_mass);
    TH2D *accMapDau1 = new TH2D("accMapD0", "Acceptance Map", 50, 0, 4, 50, -2.4, 2.4);
    TH2D *accMapDau2 = new TH2D("accMapPion", "Acceptance Map", 50, 0, 10, 50, -2.4, 2.4);
    TH2D *accMap= new TH2D("accMap", "Acceptance Map", 50, -2.4,2.4, 50, -2.4, 2.4);
    TCanvas* canvas = new TCanvas("Acceptance Map", "", 2100, 600);
    canvas->Divide(3);
    for (int i = 0; i < GenTree->GetEntries(); i++) {
        GenTree->GetEntry(i);
        if(abs(Dstar_eta) <1.&& Dstar_pT > 4){
        accMapDau1->Fill(Dstar_Dau1_pT, Dstar_Dau1_eta);
        accMapDau2->Fill(Dstar_Dau2_pT, Dstar_Dau2_eta);
        }
        accMap->Fill(Dstar_eta, Dstar_Dau2_eta);


    }
    canvas->cd(1);  
    accMapDau1->Draw("colz");
    canvas->cd(2);
    accMapDau2->Draw("colz");
    canvas->cd(3);
    accMap->Draw("colz");
    canvas->SaveAs("D0pTEtaMap.png");
    file->Close();
    // delete file;
    // delete canvas;
    // delete accMapDau1;
    // delete accMapDau2;
    
	}