#include "../Tools/BasicHeaders.h"

#include "../Tools/Parameters/AnalysisParameters.h"

TH2* DrawCosThetaPhiDistribution(RooDataSet* dataset, RooWorkspace& wspace, const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
	Double_t massMin = 1.9, massMax = 2.1;

	Int_t nCosThetaBins = 20;
	Float_t cosThetaMin = -1, cosThetaMax = 1;

	Int_t nPhiBins = 25;
	Float_t phiMin = -200, phiMax = 300;

	const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

	wspace.import(*dataset);
	RooRealVar cosThetaVar = *wspace.var(Form("cosTheta%sVar", frameAcronym));
	RooRealVar phiVar = *wspace.var(Form("phi%sVar", frameAcronym));

	// reduce the whole dataset (N dimensions) to 2D (cos theta, phi)
	const char* kinematicCut = Form("(mass > %f && mass < %f) && (y > %f && y < %f) && (pT > %d) && (RapD1<1 && RapD1>-1) && (abs(EtaD2) < 1.5) &&", massMin, massMax, -1., 1., ptMin);

	RooDataSet* reducedDataset = (RooDataSet*)dataset->reduce(RooArgSet(cosThetaVar, phiVar), kinematicCut);

	/// Draw and save the number of events(signal+background) plots in the 2D (costheta, phi) space

	TCanvas* canvas = new TCanvas(Form("canvas%s", frameAcronym), "canvas", 700, 600);

	TH2* histo = dynamic_cast<TH2*>(reducedDataset->createHistogram(histoName, cosThetaVar, RooFit::Binning(nCosThetaBins, cosThetaMin, cosThetaMax), RooFit::YVar(phiVar, RooFit::Binning(nPhiBins, phiMin, phiMax))));

	histo->SetTitle(Form(";cos #theta_{%s}; #varphi_{%s} (#circ)", frameAcronym, frameAcronym));
	histo->Draw("COLZ");

	TLatex legend;
	legend.SetTextAlign(22);
	legend.SetTextSize(0.05);
	// legend.DrawLatexNDC(.48, .86, Form("centrality %d-%d%%, %d < p_{T}^{#mu#mu} < %d GeV/c", gCentralityBinMin, gCentralityBinMax, ptMin, ptMax));
	legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

	histo->GetXaxis()->CenterTitle();
	histo->GetYaxis()->SetRangeUser(-200, 300);
	histo->GetYaxis()->CenterTitle();
	histo->GetZaxis()->SetMaxDigits(3);

	gPad->Update();

	CMS_lumi(canvas, gCMSLumiText);
	canvas->SaveAs(Form("frame_distrib/%s.png", histoName), "RECREATE");

	return histo;
}

TH1* DrawCosThetaDistribution(RooDataSet* dataset, RooWorkspace& wspace, const char* frameAcronym = "CS", Int_t ptMin = 0, Int_t ptMax = 30, const char* extraString = "") {
	Double_t massMin = 1.9, massMax = 2.1;

	Int_t nCosThetaBins = 20;
	Float_t cosThetaMin = -1, cosThetaMax = 1;

	const char* histoName = Form("%s_pt%dto%dGeV%s", frameAcronym, ptMin, ptMax, extraString);

	// wspace.import(*dataset);
	RooRealVar cosThetaVar = *wspace.var(Form("cosTheta%sVar", frameAcronym));

	// reduce the whole dataset (N dimensions) to 2D (cos theta, phi)
	const char* kinematicCut = Form("(mass > %f && mass < %f) && (y > %f && y < %f) && (pT > %d)", massMin, massMax, -1., 1., ptMin);

	RooDataSet* reducedDataset = (RooDataSet*)dataset->reduce(RooArgSet(cosThetaVar), kinematicCut);

	/// Draw and save the number of events(signal+background) plots in the 2D (costheta, phi) space

	TCanvas* canvas = new TCanvas(Form("canvas1D%s", frameAcronym), "canvas", 700, 600);

	TH1* histo = dynamic_cast<TH1*>(reducedDataset->createHistogram(histoName, cosThetaVar, RooFit::Binning(nCosThetaBins, cosThetaMin, cosThetaMax)));

	histo->SetTitle(Form(";cos (#theta_{%s})", frameAcronym));
	histo->Draw("COLZ");
	histo->GetYaxis()->SetRangeUser(0,histo->GetMaximum()*1.2);

	TLatex legend;
	legend.SetTextAlign(22);
	legend.SetTextSize(0.05);
	// legend.DrawLatexNDC(.48, .86, Form("centrality %d-%d%%, %d < p_{T}^{#mu#mu} < %d GeV/c", gCentralityBinMin, gCentralityBinMax, ptMin, ptMax));
	legend.DrawLatexNDC(.48, .78, Form("%.0f < m_{K#pi#pi} < %.1f GeV/c^{2}", massMin, massMax));

	histo->GetXaxis()->CenterTitle();
	// histo->GetYaxis()->SetRangeUser(-200, 200);
	// histo->GetYaxis()->CenterTitle();
	// histo->GetZaxis()->SetMaxDigits(3);

	gPad->Update();

	CMS_lumi(canvas, gCMSLumiText);
	canvas->SaveAs(Form("frame_distrib/%s_1D.png", histoName), "RECREATE");

	return histo;
}

void drawCosThetaPhiMap(Int_t ptMin = 0, Int_t ptMax = 30, const char* filename = "/home/jun502s/DstarAna/DStarAnalysis/Data/DstarMC_Rds_pp.root", const char* extraString = "") {
	TFile* f = TFile::Open(filename, "READ");
	if (!f) {
		cout << "File " << filename << " not found. Check the directory of the file." << endl;
		return;
	}

	cout << "File " << filename << " opened" << endl;

	writeExtraText = false; // if extra text
	extraText = "       Internal";

	gStyle->SetPadLeftMargin(.15);
	gStyle->SetTitleYOffset(1.2);
	gStyle->SetPadRightMargin(0.18);
	gStyle->SetTitleOffset(1.2, "z");
	gStyle->SetPalette(kRainBow);
	gStyle->SetNumberContours(256);
	gStyle->SetOptStat(0);

	using namespace RooFit;
	RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

	/// Draw and save the number of events(signal+background) plots in the 2D (costheta, phi) space
	RooWorkspace wspace("wspace");
    RooWorkspace* ws = (RooWorkspace*)f->Get("workspace");
	// cout << ws << endl;
	// cout << (RooDataSet*)ws->data("datasetCS") << endl;

	TH2* histoCS = DrawCosThetaPhiDistribution((RooDataSet*)ws->data("datasetCS"), wspace, "CS", ptMin, ptMax, extraString);

	TH2* histoHX = DrawCosThetaPhiDistribution((RooDataSet*)ws->data("datasetHX"), wspace, "HX", ptMin, ptMax, extraString);
	TH1* histoHX1D = DrawCosThetaDistribution((RooDataSet*)ws->data("datasetHX"), wspace, "HX", ptMin, ptMax, extraString);
	// canvasProjectX_HX->cd();
	// projectionX->Draw();
	// projectionX->SetTitle(";cos #theta_{HX};Counts");
	// projectionX->GetYaxis()->SetRangeUser(0,100000);
	// canvasProjectX_HX->SaveAs(Form("frame_distrib/HX_projectionX_pt%dto%dGeV%s.png", ptMin, ptMax, extraString), "RECREATE");
	// canvasProjectY_HX->cd();
	// projectionY->Draw();
	// canvasProjectY_HX->SaveAs(Form("frame_distrib/HX_projectionY_pt%dto%dGeV%s.png", ptMin, ptMax, extraString), "RECREATE");

	/// save the results in a file for later usage
	gSystem->mkdir("frame_distrib", kTRUE);
	TFile outputFile("CosThetaPhiMaps.root", "UPDATE");

	histoHX->Write();
	histoHX1D->Write();
	histoCS->Write();

	outputFile.Close();

	cout << endl
	     << "CosTheta-Phi maps saved in " << outputFile.GetName() << endl;
}

void scanDrawCosThetaPhiMap(const char* filename = "../../../../data/DstarMC_Rds_pp.root", const char* extraString = "") {
	for (Int_t idx = 0; idx < NPtBins; idx++) {
		drawCosThetaPhiMap(gPtBinning[idx], gPtBinning[idx + 1], filename, extraString);
	}
}