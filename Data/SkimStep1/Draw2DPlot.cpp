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
void Draw2DPlot(){
	gStyle->SetOptStat(0);
        TChain* ch1 = new TChain("d0ana_newreduced/PATCompositeNtuple");
        //loadRootFilesToTChain(ch1,".");
        ch1->Add("d0ana_tree_12.root");
        ch1->Add("d0ana_tree_11.root");
	ch1->Add("d0ana_tree_129.root");
	ch1->Add("d0ana_tree_6.root");
	ch1->Add("d0ana_tree_150.root");
        vector<std::string> vars={"3DPointingAngle","pTD1","pTD2","EtaD1","EtaD2","VtxProb","3DDecayLength","3DDecayLengthSignificance","pT","y"};

    size_t nVars = vars.size();
    size_t nHist = nVars * (nVars - 1) / 2;  // 2D 조합 개수

    TH2D *h2[nHist];
    TLegend *leg[nHist];
    TCanvas *cvs[nHist];

    // 변수별 범위 저장하는 map
    std::map<std::string, std::tuple<int,float, float>> rangemap;

    // rangemap 초기화
    for (const auto &var : vars) {
	float down=0.8;
	float up =10;
	int nbin =30;
	if(var.compare("y")==0){
	down=-1*1.6;
	up=1.6;
	}
	if(var.find("ptd")!=std::string::npos){
	down=0.7;
	up=2;
	}
	if(var.find("Eta")!=std::string::npos){
	down=-1*TMath::Pi();
	up=TMath::Pi();
	}
	if(var.compare("3DDecayLength")==0){
	nbin=20;
	down=0;
	up=0.2;
	}
	if(var.find("3DDecayLengthSignificance")!=std::string::npos){
	down=0;
	up=5;
	}
	if(var.find("Pointing")!=std::string::npos){
	down=0;
	up=1;
	}
	if(var.find("Vtx")!=std::string::npos){
	down=0;
	up=1;
	}
        rangemap.insert({var, {nbin, down,up}});  // 예제: (0.0, 1.0) 범위 설정
    }

    // 확인 출력

for(auto var : vars){
std::cout << var << " -> nbin: " << std::get<0>(rangemap[var])
          << ", down: " << std::get<1>(rangemap[var])
         << ", up: " << std::get<2>(rangemap[var]) << std::endl;}
 std::vector<bool> select(vars.size(), false);
    select[0] = select[1] = true;

    int i =0;
    do {
	std::vector<std::string> chosen;
        for (size_t i = 0; i < vars.size(); ++i) {
            if (select[i]) chosen.push_back(vars[i]);
        }

        std::string x_var = chosen[0];
        std::string y_var = chosen[1];
	cout << chosen[0] << chosen[1] << endl;

                std::string hist_name = "h2_" + x_var + "_" + y_var;
		cout << hist_name << endl;
                h2[i] = new TH2D(hist_name.c_str(), Form("%s;%s;%s",hist_name.c_str(),x_var.c_str(),y_var.c_str()),std::get<0>(rangemap[chosen[0]]),std::get<1>(rangemap[chosen[0]]),std::get<2>(rangemap[chosen[0]]),std::get<0>(rangemap[chosen[1]]),std::get<1>(rangemap[chosen[1]]),std::get<2>(rangemap[chosen[1]]));
		ch1->Draw(Form("%s:%s>>%s",chosen[1].c_str(),chosen[0].c_str(),hist_name.c_str()),"matchGEN==1");
		cout <<"Drawing" << endl;

                cvs[i] = new TCanvas(hist_name.c_str(), hist_name.c_str(), 800, 600);
		cvs[i]->cd();
                h2[i]->Draw("COLZ");
		cvs[i]->SaveAs(Form("2DHist_%s.pdf",hist_name.c_str()));
		
		i++;
            
    } while (std::prev_permutation(select.begin(), select.end()));

    std::cout << "Generated  histograms.\n";

}

