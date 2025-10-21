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
void Draw1DPlot_cp(){
	gStyle->SetOptStat(0);
    TFile *f = new TFile("1Dhistogram_test.root","read");
    vector<std::string> vars={"3DPointingAngle","pTD1","pTD2","EtaD1","EtaD2","VtxProb","3DDecayLength","3DDecayLengthSignificance","pT","y","NHitD1","NHitD2","zDCASignificanceDaugther2","zDCASignificanceDaugther1","xyDCASignificanceDaugther1","xyDCASignificanceDaugther2","pTSignificanceDaughter2","pTSignificanceDaughter1","normalizedChi2"};

    size_t nVars = vars.size();
    size_t nHist = nVars * (nVars - 1) / 2;  // 2D 조합 개수

    TH1D *h1[nVars];
    TH1D *hgen[nVars];
    TLegend *leg[nVars];
    TCanvas *cvs[nVars];
	double xlow=0.7;
	double ylow=0.7;
	double xup=0.89;
	double yup=0.89;

    // 변수별 범위 저장하는 map
    std::map<std::string, std::tuple<int,float, float>> rangemap;

    // rangemap 초기화
    for (const auto &var : vars) {
	float down=0.8;
	float up =10;
	int nbin =40;
	
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
	up=TMath::Pi();
	}
	if(var.find("Vtx")!=std::string::npos){
	down=0;
	up=1;
	}
	if(var.find("pTSignificanceDaughter")!=std::string::npos){
	nbin=300;
	down=0;
	up=300;
	}	
	if(var.find("normalizedChi2")!=std::string::npos){
	nbin=30;
	down=0;
	up=10;
	}
    rangemap.insert({var, {nbin, down,up}});  // 예제: (0.0, 1.0) 범위 설정
    }

    // 확인 출력

for(auto var : vars){
std::cout << var << " -> nbin: " << std::get<0>(rangemap[var])
          << ", down: " << std::get<1>(rangemap[var])
         << ", up: " << std::get<2>(rangemap[var]) << std::endl;}
 std::vector<bool> select(vars.size(), false);
    select[0] = true;

    int i =0;
    f->cd();
    do {
	std::vector<std::string> chosen;
        for (size_t i = 0; i < vars.size(); ++i) {
            if (select[i]) chosen.push_back(vars[i]);
        }
//	if(chosen[0].find("zDCASignificanceDaugther1")==std::string::npos) continue;

 		std::string x_var = chosen[0];
		// if(chosen[0].find("pTSignificanceDaughter")==std::string::npos ) continue;
		if(chosen[0].find("NHit")!=std::string::npos){
		xlow=0.11;
		ylow=0.7;
		xup=0.3;
		yup=0.89;
		}
		cout << "x_var: " << x_var << endl;
		leg[i]=new TLegend(xlow,ylow,xup,yup);
		leg[i]->SetBorderSize(0);

        std::string hist_name = "h1_" + x_var + "_";
        h1[i] = (TH1D*)f->Get(hist_name.c_str());
        hgen[i] = (TH1D*)f->Get(Form("hgen_%s",hist_name.c_str()));
		
// Create cumulative histograms
TH1D* h1_cumulative = (TH1D*)h1[i]->GetCumulative();
TH1D* hgen_cumulative = (TH1D*)hgen[i]->GetCumulative();

// Create a new canvas with two pads
TCanvas* c_combined = new TCanvas(Form("combined_%s", hist_name.c_str()), Form("Combined %s", hist_name.c_str()), 800, 600);
TPad* pad1 = new TPad("pad1", "Pad for original histograms", 0.0, 0.3, 1.0, 1.0);
TPad* pad2 = new TPad("pad2", "Pad for cumulative histograms", 0.0, 0.0, 1.0, 0.3);
pad1->SetTopMargin(0.01);
pad1->SetBottomMargin(0.01);
pad2->SetTopMargin(0.01);
pad2->SetBottomMargin(0.15);
pad1->Draw();
pad2->Draw();

// Draw the original histograms on the first pad
pad1->cd();
h1[i]->SetLineColor(kGreen);
hgen[i]->SetLineColor(kRed);
h1[i]->SetTitle("");
h1[i]->GetXaxis()->SetTitle("");
h1[i]->GetYaxis()->SetNdivisions(505);
h1[i]->GetYaxis()->SetLabelSize(0.03);
hgen[i]->SetTitle("");
        if(h1[i]->GetMaximum()>hgen[i]->GetMaximum() ){
        h1[i]->Draw("HIST");
        hgen[i]->Draw("HIST same");}
        else{
        hgen[i]->Draw("HIST");
        h1[i]->Draw("HIST same");
        }
leg[i]->Draw("same");

// Draw the cumulative histograms on the second pad
pad2->cd();
h1_cumulative->SetLineColor(kGreen);
hgen_cumulative->SetLineColor(kRed);
h1_cumulative->SetTitle("");
h1_cumulative->GetYaxis()->SetNdivisions(505);
h1_cumulative->GetYaxis()->SetLabelSize(0.07);
h1_cumulative->GetXaxis()->SetRangeUser(0,1);
h1_cumulative->GetXaxis()->SetLabelSize(0.07);
h1_cumulative->GetXaxis()->SetTitleSize(0.06);
hgen_cumulative->SetTitle("");
h1_cumulative->Draw("HIST");
hgen_cumulative->Draw("HIST same");

// Save the combined canvas
c_combined->SaveAs(Form("1DHisto/CulmHist_%s_test.pdf", hist_name.c_str()));

// Calculate and print the signal-to-background ratio
double signal = hgen_cumulative->GetBinContent(hgen_cumulative->GetNbinsX());
double background = h1_cumulative->GetBinContent(h1_cumulative->GetNbinsX());
double ratio = signal / background;
std::cout << "Signal: " << signal << ", Background: " << background << ", Signal-to-Background Ratio: " << ratio << std::endl;

// Write the cumulative histograms to the file
// h1_cumulative->Write();
// hgen_cumulative->Write();

i++;

    } while (std::prev_permutation(select.begin(), select.end()));
    f->Close();

    std::cout << "Generated  histograms.\n";

}

