bool DirChecker(const std::string& dirPath) {
    if (gSystem->AccessPathName(dirPath.c_str())) {
        if (gSystem->MakeDirectory(dirPath.c_str()) != 0) {
            std::cerr << "Error creating directory: " << dirPath << std::endl;
            return false;
        }
        std::cout << "Created directory: " << dirPath << std::endl;
    }
    return true;
}

// Open TFile with directory creation if needed
TFile* OpenFile(std::string& filePath, const std::string& mode = "READ") {
    // Extract the directory path
    std::string dirPath = "";
    size_t pos = filePath.find_last_of("/\\");
    if (pos != std::string::npos) {
        dirPath = filePath.substr(0, pos);
        if (!DirChecker(dirPath)) {
            return nullptr;
        }
    }
    
    // Open the file
    TFile* file = new TFile(filePath.c_str(), mode.c_str());
    if (file->IsZombie()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        delete file;
        return nullptr;
    }
    
    return file;
}
void DrawMvaCorrelation(){
	gStyle->SetOptStat(0);
    std::string outputDir = "/home/jun502s/DstarAna/DStarAnalysis/Macro/plot/MvaCorrelation";
    DirChecker(outputDir);
    std::string filePath = "/home/jun502s/DstarAna/DStarAnalysis/Macro/plot/MvaCorrelation/MvaCorrelation.root";
    TFile *f = OpenFile(filePath, "RECREATE");
    // TFile *f = new TFile("2Dhistogram_DataVSMC.root","recreate");
    TChain* chMC = new TChain("d0ana_mc_newreduced/PATCompositeNtuple");
    // TChain* chData = new TChain("d0ana_newreduced/PATCompositeNtuple");
    // chData->Add("/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/d0ana_tree_1.root");
    // chData->Add("/home/CMS/Run3_2023/Data/SkimMVA/D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/HIPhysicsRawPrime0/crab_D0tarAna_Data_Run375513_HIPhysicsRawPrime0_CMSSW_13_2_13_MVA_25Feb2025_v1/250225_080651/0000/d0ana_tree_10.root");
        //loadRootFilesToTChain(ch1,".");
    chMC->Add("/home/CMS/Run3_2023/MC/SkimMVA/Dstar_D0Ana_Official_pT0_MC_Step1MVA_ForLowpT_CMSSW_13_2_13_MVA_09Mar2025_v1/promptD0ToKPi_PT-0_TuneCP5_5p36TeV_pythia8-evtgen/crab_Dstar_D0Ana_Official_pT0_MC_Step1MVA_ForLowpT_CMSSW_13_2_13_MVA_09Mar2025_v1/250309_133616/0000/*.root");
    // chMC->Add("d0ana_tree_115.root");
    // chMC->Add("d0ana_tree_128.root");
    // chMC->Add("d0ana_tree_111.root");
    // chMC->Add("d0ana_tree_132.root");

    // chMC->Add("d0ana_tree_11.root");
	// ch1->Add("d0ana_tree_129.root");
	// ch1->Add("d0ana_tree_6.root");
	// ch1->Add("d0ana_tree_150.root");
    vector<std::string> vars={"3DPointingAngle","3DCosPointingAngle","2DPointingAngle","2DCosPointingAngle","pTD1","pTD2","pTerrD2","pTerrD1","EtaD1","EtaD2","VtxProb","3DDecayLength","3DDecayLengthSignificance","2DDecayLength","2DDecayLengthSignificance","mva"};

    size_t nVars = vars.size();
    size_t nHist = nVars * (nVars - 1) / 2;  // 2D 조합 개수

    TH2D *h1[nHist];
    TH2D *hgen[nHist];
    TLegend *leg[nHist];
    TCanvas *cvs[nHist];
    TCanvas *cvs1[nHist];

    // 변수별 범위 저장하는 map
    std::map<std::string, std::tuple<int,float, float>> rangemap;

    // rangemap 초기화
    for (const auto &var : vars) {
	float down=0.8;
	float up =10;
	int nbin =60;
	if(var.compare("y")==0){
	down=-1*1.6;
	up=1.6;
	}
	if(var.find("NHit")!=std::string::npos){
	nbin=20;
	down=0;
	up=20;
	}
	if(var.find("pTD")!=std::string::npos){
    nbin=60;
	down=0.;
	up=3;
	}
	if(var.find("Eta")!=std::string::npos){
	down=-1*TMath::Pi();
	up=TMath::Pi();
	}
	if(var.compare("3DDecayLength")==0){
	nbin=50;
	down=0;
	up=0.2;
	}	if(var.compare("2DDecayLength")==0){
	nbin=50;
	down=0;
	up=0.2;
	}
	if(var.find("3DDecayLengthSignificance")!=std::string::npos){
	down=0;
	up=5;
	}	
    if(var.find("2DDecayLengthSignificance")!=std::string::npos){
	down=0;
	up=5;
	}
	if(var.find("Pointing")!=std::string::npos){
	down=0;
	up=1;
	}	
    if(var.find("Cos")!=std::string::npos){
	down=-1;
	up=1;
	}
	if(var.find("Vtx")!=std::string::npos){
	down=0;
	up=1;
	}
    if(var.find("mva")!=std::string::npos){
    down=-1;
	up=1;
    }    
    if(var.find("pTerr")!=std::string::npos){
    down=0;
	up=0.05;
    }

        rangemap.insert({var, {nbin, down,up}});  // 예제: (0.0, 1.0) 범위 설정
    }

    // 확인 출력

for(auto var : vars){
std::cout << var << " -> nbin: " << std::get<0>(rangemap[var])
          << ", down: " << std::get<1>(rangemap[var])
         << ", up: " << std::get<2>(rangemap[var]) << std::endl;}
 std::vector<bool> select(vars.size(), false);
    // select[0] = select[1] = true;
    select[0] =true;

    int i =0;
    f->cd();
    do {
	std::vector<std::string> chosen;
        for (size_t i = 0; i < vars.size(); ++i) {
            if (select[i]) chosen.push_back(vars[i]);
        }

        std::string x_var = chosen[0];
        // std::string y_var = chosen[1];
	// cout << chosen[0] << chosen[1] << endl;
	cout << chosen[0] << endl;
	// if((chosen[0].find("pTD1")!=std::string::npos&& chosen[1].find("pTD2")!=std::string::npos )|| ((chosen[0].find("EtaD1")!=std::string::npos && chosen[1].find("EtaD2")!=std::string::npos))){

        std::string hist_name = "h2_" + x_var + "_" + "mva";
		cout << hist_name << endl;
        hgen[i] = new TH2D(Form("hgen_%s",hist_name.c_str()), Form("%s;mva;%s",hist_name.c_str(),x_var.c_str()),60,-1,1,std::get<0>(rangemap[chosen[0]]),std::get<1>(rangemap[chosen[0]]),std::get<2>(rangemap[chosen[0]]));//std::get<0>(rangemap[chosen[1]]),std::get<1>(rangemap[chosen[1]]),std::get<2>(rangemap[chosen[1]]));
        h1[i] = new TH2D(Form("hgenMatch_%s",hist_name.c_str()), Form("%s;mva;%s",hist_name.c_str(),x_var.c_str()),60,-1,1,std::get<0>(rangemap[chosen[0]]),std::get<1>(rangemap[chosen[0]]),std::get<2>(rangemap[chosen[0]]));//std::get<0>(rangemap[chosen[1]]),std::get<1>(rangemap[chosen[1]]),std::get<2>(rangemap[chosen[1]]));
        // h1[i] = new TH2D(hist_name.c_str(), Form("%s;%s;%s",hist_name.c_str(),x_var.c_str(),y_var.c_str()),std::get<0>(rangemap[chosen[0]]),std::get<1>(rangemap[chosen[0]]),std::get<2>(rangemap[chosen[0]]),std::get<0>(rangemap[chosen[1]]),std::get<1>(rangemap[chosen[1]]),std::get<2>(rangemap[chosen[1]]));
		chMC->Draw(Form("%s:mva>>%s",chosen[0].c_str(),Form("hgen_%s",hist_name.c_str())));
		chMC->Draw(Form("%s:mva>>%s",chosen[0].c_str(),Form("hgenMatch_%s",hist_name.c_str())),"matchGEN==1");
		// chData->Draw(Form("%s:%s>>%s",chosen[1].c_str(),chosen[0].c_str(),hist_name.c_str()));
		cout << "Drawing" << endl;
        h1[i]->SetTitle(Form("%.3f",h1[i]->GetCorrelationFactor()));
        hgen[i]->SetTitle(Form("%.3f",hgen[i]->GetCorrelationFactor()));

        cvs[i] = new TCanvas(hist_name.c_str(), hist_name.c_str(), 1600, 600);
        cvs[i]->Divide(2, 1);
		cvs[i]->cd(1);
        h1[i]->Draw("COLZ");
        // cvs1[i] = new TCanvas(Form("hgen_%s",hist_name.c_str()), hist_name.c_str(), 800, 600);
		cvs[i]->cd(2);
        hgen[i]->Draw("COLZ");
        cout << Form("correlation of %s : ",hist_name.c_str()) << Form("%.3f",hgen[i]->GetCorrelationFactor()) << Form(" Correlation of GenMatch_%s : ",hist_name.c_str()) << Form("%.3f",h1[i]->GetCorrelationFactor()) << endl;
        cvs[i]->SaveAs(Form("2DGENMvaHist_%s.pdf",hist_name.c_str()));
		// cvs1[i]->SaveAs(Form("2DGENMvaHist_%s.pdf",hist_name.c_str()));
        // h1[i]->Write();
        hgen[i]->Write(); 
        h1[i]->Write(); 
		
		i++;
    // }
    } while (std::prev_permutation(select.begin(), select.end()));
    f->Close();

    std::cout << "Generated  histograms.\n";

}

