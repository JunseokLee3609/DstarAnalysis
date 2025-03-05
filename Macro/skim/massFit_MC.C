#include "RooGlobalFunc.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooPlot.h"
#include "TFile.h"
#include "TCanvas.h"
#include <iostream>
#include "RooRealVar.h"

using namespace RooFit;
class modelpdf{
	private:
		RooWorkspace *ws;
		RooRealVar *mass;
	public:
	modelpdf(RooDataSet* dataset){
		ws=new RooWorkspace("workspace");
		mass=new RooRealVar("mass","mass",1.9,2.1,"GeV/c^{2}");
		ws->import(*dataset);
		ws->import(*mass);
	};
<<<<<<< HEAD
	// RooAddPdf combinedmodel;
	// int NBkg;
	// int NSig;
=======
	RooAddPdf *combinedmodel;
	RooRealVar *Nbkg;
	RooRealVar *Nsig;
	// RooDataSet *a;
	int counter;
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
	map<TString,RooAbsArg*> models;
	void ModelConstructor(TString pdf){
		if(pdf.Contains("Guass")){
		models.insert(make_pair(pdf,ws->factory(Form("Gaussian::%s[mass,mean_%s[0],sigma_%s[2,0.1,5]]",pdf.Data(),pdf.Data(),pdf.Data()))));
		cout << "Sucessfully constrcted Gaussian PDF" << endl;}
		else if(pdf.Contains("CB")){
		models.insert(make_pair(pdf,ws->factory(Form("CBShape::%s[mass,mean_%s,sigma_%s[2,0.1,5]]",pdf.Data(),pdf.Data(),pdf.Data()))));
		cout << "Sucessfully constrcted CrystallBall PDF" << endl;}
		else if(pdf.Contains("Expo")){
		models.insert(make_pair(pdf,ws->factory(Form("Exponential::%s[mass,mean_%s,sigma_%s[2,0.1,5]]",pdf.Data(),pdf.Data(),pdf.Data()))));
		cout << "Sucessfully constrcted Exponential PDF" << endl;
		}
		else{
		cout << Form("%s is not available pdf candidates",pdf.Data()) <<endl;
		}
	};
	void Print(){
		ws->Print();
	};
	void Getparameter(TString Var){
	if(ws->var(Var.Data())!=nullptr){
		double initial = ws->var(Var.Data())->getVal();
		double initialmin = ws->var(Var.Data())->getMin();
		double initialmax = ws->var(Var.Data())->getMax();
		cout << Form("Parameter %s : [%f, %f , %f]",Var.Data(),initial,initialmin,initialmax) << endl;}
	else {
		cout << "No prameter exist" << endl;
	}
		
		
	};
	void Modify(TString Var, double init, double min,double max ){
	if(ws->var(Var.Data())!=nullptr){
	double initial = ws->var(Var.Data())->getVal();
	double initialmin = ws->var(Var.Data())->getMin();
	double initialmax = ws->var(Var.Data())->getMax();
	ws->var(Var.Data())->setVal(init);
	ws->var(Var.Data())->setMax(max);
	ws->var(Var.Data())->setMin(min);
	cout << Form("Parameter has been changed from [%f, %f , %f] to [%f, %f, %f]",initial,initialmin,initialmax, ws->var(Var.Data())->getVal(), ws->var(Var.Data())->getMin(), ws->var(Var.Data())->getMax()) << endl;}
	else {
		cout << "No prameter exist" << endl;
		}
	};
<<<<<<< HEAD
// void Combiner(){
// 	RooAddPdf model("model","sig+background",RooArgList(gauss1,background),RooArgList(Nsig,Nbkg));
// 	combinedmodel = model;


// }
=======
void Combiner( RooAbsArg* sig,RooAbsArg* back, RooRealVar *Nsig, RooRealVar *Nbkg){
	this->Nsig=Nsig;
	this->Nbkg=Nbkg;
	RooAddPdf *model=new RooAddPdf("model","sig+background",RooArgList(sig,back),RooArgList(Nsig,Nbkg));
	combinedmodel = model;
	};
void DrawPlot(RooDataSet *a){
	TCanvas *cvs = new TCanvas(Form("cvs%d",counter),Form("cvs%d",counter),800,400);
	cvs->cd();
	RooPlot *frame =mass->frame();
	// a = (RooDataSet*)ws->data("dataset");
	a->plotOn(frame);
	frame->Draw();
};
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
void Reducedata(string str){
	double ptMin=1;
	double ptMax=40;
	double rapMin=-2.4;
	double rapMax=2.4;
    bool matchGEN=true;
	// ws->import(*dataset);
	ws->data("dataset")->Print();

    RooDataSet* initialDS = (RooDataSet*)ws->data("dataset"); 
    // initialDS->reduce(RooArgSet(*(ws->var("mass")), *(ws->var("pT")), *(ws->var("y")),*(ws->var("eta")),*(ws->var("phi")),*(ws->var("matchGEN"))));
	initialDS->SetName(Form("initialDS%s",str.c_str()));
    // RooDataSet* weightedDS = new RooDataSet("weightedDS", "weight dataset", *initialDS->get(), Import(*initialDS), WeightVar(*ws->var("weight")));	
    RooDataSet* reducedDS = (RooDataSet*) initialDS->reduce((Form("(pT>=%.1f&&pT<%.1f)&&(y>=%.2f&&y<%.2f)&&matchGEN==1", ptMin, ptMax, rapMin, rapMax)));
    reducedDS->SetName(Form("reducedDS%s",str.c_str()));
	ws->import(*reducedDS);
<<<<<<< HEAD


=======
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
	};
};



using namespace RooFit;
void massFit_MC(){
<<<<<<< HEAD
	TFile* f1 = TFile::Open("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds.root");
=======
	TFile* f1 = TFile::Open("/home/jun502s/DstarAna/DStarAna/data/DstarMC_Rds.root");
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
    auto ws = (RooWorkspace*)f1->Get("workspace");
	auto MCPR1= (RooDataSet*)ws->data("dataset");
	// auto RD= (RooDataSet*)f2->Get("dataset");
	// reducedata(ws,"1");
	// reducedata(ws2,RD,"2");
<<<<<<< HEAD
//	modelpdf modelpdf(MCPR1);
//	modelpdf.Reducedata("1");
//	modelpdf.ModelConstructor("gauss1");
//	modelpdf.ModelConstructor("CB1");
//	RooAddPdf model("model","sig+background",RooArgList(model.ModelConstructor.models["gauss1"],model.ModelConstructor.models["CB1"]),RooArgList(Nsig,Nbkg));
=======
	modelpdf model(MCPR1);
	model.Reducedata("1");
	model.ModelConstructor("gauss1");
	model.ModelConstructor("CB1");
	model.DrawPlot(MCPR1);
	// RooAddPdf model("model","sig+background",RooArgList(model.ModelConstructor.models["gauss1"],model.ModelConstructor.models["CB1"]),RooArgList(Nsig,Nbkg));
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3

	

//	TFile* f3 = TFile::Open("Skim_OniaTree_RD_PADoubleMuon_AccLoose_FullDataSet.root");
	// TTree *treeNPMC = (TTree*)f1->Get("DiMuonTree");
//	auto MCNP= (RooDataSet*)f2->Get("dataset");
//	auto data= (RooDataSet*)f3->Get("dataset");
	//treeNPMC->SetBranchAddress("weight", &weight, &b_weight);
<<<<<<< HEAD
 	RooRealVar mass("mass","mass",1.9,2.1,"GeV/c^{2}");
=======
// 	RooRealVar mass("mass","mass",1.9,2.1,"GeV/c^{2}");
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
// //	RooRealVar mass2("mass2","mass",3.0,3.2,"GeV/c^{2}");
// 	RooRealVar mean("mean", "mean of gaussian", 2, 2.0, 2.1);
// 	RooRealVar sigma1("sigma1", "width of gaussian1", 0.05, 0.01, 0.1);
// 	RooRealVar sigma2("sigma2", "width of gaussian1", 0.05, 0.01, 0.1);
//    // Define the parameters for the Crystal Ball function
//     // RooRealVar mean_cb("mean_cb", "Mean of Crystal Ball", 2, 2.0, 2.1);
//     // RooRealVar sigma_cb("sigma_cb", "Width of Crystal Ball", 0.005, 0.001, 0.03);
//     // RooRealVar alpha_cb("alpha_cb", "Alpha (tail parameter)", 0.5, 0.1, 4.0);
//     // RooRealVar n_cb("n_cb", "n (exponent)", 5.0, 0.001, 10.0);

//     // Define the Crystal Ball function
//     // RooCBShape crystalBall("crystalBall", "Crystal Ball Component", mass, mean_cb, sigma_cb, alpha_cb, n_cb);
// 	RooGaussian gauss1("gauss", "signal gaussian1", mass, mean, sigma1);
// 	RooRealVar gauss_fraction("gauss_fraction", "Fraction of Gaussian", 0.5, 0.0, 1.0);
	// RooAddPdf signal("signal", "Gaussian + Crystal Ball Signal", RooArgList(gauss, crystalBall), RooArgList(gauss_fraction));

// 	// RooRealVar sigma2("sigma2", "width of gaussian2", 0.05, 0.001, 0.10);
// 	RooGaussian gauss2("gauss2", "signal gaussian2", mass, mean, sigma2);

// 	RooAddPdf signal("signal", "double gaussian", RooArgList(gauss1,gauss2),gauss_fraction);
// 	auto reduceDS1=(RooDataSet*)ws->data("reducedDS1");
// 	// auto reduceDS2=(RooDataSet*)ws2->data("reducedDS2");
// 	// auto *fitresultMC = signal.fitTo(*reduceDS1,RooFit::Minimizer("Minuit","Minizer"),Extended(false),RooFit::Save(),RooFit::NumCPU(8));
// 	// fitresultMC->Print("v");



<<<<<<< HEAD
 	 RooPlot *frame = mass.frame();
 	 MCPR1->plotOn(frame,RooFit::MarkerColor(kBlue));
=======
// 	// RooPlot *frame = mass.frame();
// 	// reduceDS1->plotOn(frame,RooFit::MarkerColor(kBlue));
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
// 	// reduceDS2->plotOn(frame,RooFit::MarkerColor(kRed));
// 	// signal.plotOn(frame);
// 	// signal.plotOn(frame,RooFit::Components("gauss"),RooFit::LineColor(kGreen));
// 	// signal.plotOn(frame,RooFit::Components("crystalBall"),RooFit::LineColor(kRed));
// 	// TCanvas *cvs = new TCanvas("cvs","cvs",800,600);
<<<<<<< HEAD
	 frame->Draw();
=======
// 	// frame->Draw();
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
// 	// cvs->SaveAs("asd2.png");
// 	// auto modelVars = signal.getVariables();
// 	// auto fpf = fitresultMC->floatParsFinal();
// 	// for(int idx =0; idx <fpf.getSize(); idx++){
//     //         auto var = (TObject*) &fpf[idx];
//     //         string _vname_ = string(var->GetName());
//     //         auto pdfVar = (RooRealVar*)  modelVars->find(_vname_.c_str());
//     //         pdfVar->setConstant(kTRUE);
// 	// 		pdfVar->Print();
//     //  }
//     //RooRealVar lambda1("lambda1", "slope", -0.5, -1, 0.1);
//     //RooRealVar lambda2("lambda2", "slope", -0.01, -0.5, 0.5);
// 	RooRealVar Nsig("Nsig","sig yield",3000,1000,5000000);
// 	RooRealVar Nbkg("Nbkg","sig yield",2000,1000,1000000);
// 	RooRealVar p0("p0", "Coefficient for x^1", 0, -1, 1.0);
// 	RooRealVar p1("p1", "Coefficient for x^0", -0.4, -1.0, 0.0);
// 	RooRealVar p2("p2", "Coefficient for x^0", 0.01, -1.0, 1.0);

// 	// Define the polynomial background function
// 	RooPolynomial background("background", "Polynomial Background", mass, RooArgList(p0, p1));
// 	// RooRealVar frac("frac", "fraction of gaussian1", 0.2, 0, 1);

	
//     //RooChebychev Cheb("Cheb", "background", mass, RooArgList(lambda1));
// 	auto *fitresult = model.fitTo(*reduceDS1,RooFit::Minimizer("Minuit","Minimizer"),Extended(true),RooFit::Save(),RooFit::NumCPU(8));
// 	// auto *fitresult = model.fitTo(*reduceDS2,Extended(false),RooFit::Save(),RooFit::NumCPU(8));
// 	fitresult->Print("v");
// 	TCanvas *cvs2 = new TCanvas("cvs2","cvs2",600,800);
// 	cvs2->cd();
// 	RooPlot* frame2 = mass.frame();
// 	//reduceDS2->plotOn(frame2,RooFit::Binning(50));
// 	reduceDS1->plotOn(frame2);
// 	model.plotOn(frame2);
// 	model.plotOn(frame2,RooFit::Components("gauss1"),RooFit::LineColor(kRed));
// 	model.plotOn(frame2,RooFit::Components("background"),RooFit::LineColor(kBlue));
// 	frame2->Draw();
// 	cvs2->SaveAs("asd3.png");
	//int nbins = 100;
	//double xmin =-10;
	//double xmax =100;
	//TH1F *h = new TH1F("h","Histogram",nbins,xmin,xmax);
	//RooRealVar ctau("ctauRes","ctauRes",-10,100,"l_{ctau}");
	//RooDataHist dh("dh","dh",ctau,h);
	//RooHistPdf pdf("pdf", "pdf", ctau, dh);
	//TCanvas *cvs3 = new TCanvas("cvs3","cvs3",800,600);
	//cvs3->cd();
	//RooPlot *frame3 = ctau.frame();
	//MCPR->plotOn(frame3,RooFit::Normalization(3.0),RooFit::MarkerColor(kBlue));
	////MCNP->plotOn(frame3,RooFit::LineColor(kBlue),RooFit::Normalization(1.0, RooAbsReal::RelativeExpected));
	////dh.plotOn(frame3,RooFit::LineColor(kRed));
	//frame3->Draw();
	//cvs3->SetLogy();
	
	
	
<<<<<<< HEAD
	}
=======
	}
>>>>>>> dd9ba2be2bc9a67fbcce4205d3bac28f0806f0d3
