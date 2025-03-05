#include "RooFit.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "TFile.h"
#include "TTree.h"
TFile* OpenTFile(TString filename){
  TFile* file = TFile::Open(filename.Data(), "READ");
  if(!file->IsOpen()){
    std::cout << "File " << filename << " not found or open" << std::endl;
    exit(1);
  }
  return file;

}

void RootToRDS(){
    TString date="250110";
    TFile *fin = TFile::Open("/home/jun502s/DstarAna/DstarAna_real/DDbarpPb/data/flatSkimForBDT_PbPb_isMC1_0_250110.root"); //Data
    TFile *fout = TFile::Open(Form("/home/jun502s/DstarAna/DstarAna_real/DDbarpPb/data/DstarMC_Rds_PbPb_%s.root",date.Data()),"RECREATE"); //Data
    // TFile *fin = TFile::Open("/home/jun502s/DstarAna/DStarAna/data/output_PbPb_Data.root"); //Data
    // TFile *fout = TFile::Open(Form("/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_PbPb_%s.root",date.Data()),"RECREATE"); //Data
    TTree *t = (TTree*)fin->Get("skimTreeFlat");
	fout->cd();

    RooRealVar mass("mass","Invariant mass",1.74,2.1);
    RooRealVar pt("pT","Transverse Momentum",0,100);
    RooRealVar eta("eta","",-2.4,2.4);
    RooRealVar phi("phi","",-3.14,3.14);
    RooRealVar massDaugther1("massDaugther1", "massDaugther1",1.74,2.00);
    RooFormulaVar massPion("massPion", "Parent mass - Daughter mass", "mass - massDaugther1", RooArgList(mass, massDaugther1));
    RooRealVar y("y","",-2.4,2.4);
    RooRealVar mva("mva","",-1,1);  
    // RooCategory matchGEN("matchGEN","matchGEN");
   RooRealVar matchGEN("matchGEN","matchGEN",-1,1);
    // matchGEN.defineType("true", 1);
    // matchGEN.defineType("false", 0);
    RooArgSet variables(mass,pt,eta,phi,y,mva,massDaugther1);
    RooDataSet::setDefaultStorageType(RooAbsData::Tree);
    // RooDataSet data("dataset", "dataset from tree", t, variables);
    const int chunkSize = 1000000; // Adjust chunk size as needed
    int nEntries = t->GetEntries();
    // int nEntries = 5000000;
    cout << nEntries << endl;
    RooDataSet data("dataset", "dataset from tree",variables);

    for (int i = 0; i < nEntries; i += chunkSize) {
        // t->GetEntry(i);
        int end = std::min(i + chunkSize, nEntries);
        TTree *chunk = t->CopyTree("", "", chunkSize, i);
        RooDataSet chunkData("chunkData", "chunk data", chunk, variables);
        data.append(chunkData);
        delete chunk;
    }
    data.addColumn(massPion);
    RooWorkspace *ws = new RooWorkspace("workspace");
    ws->import(mass);
    ws->import(pt);
    ws->import(eta);
    ws->import(y);
    ws->import(phi);
    ws->import(mva);
    ws->import(massPion);
//    ws->import(massDaugther1);
    ws->import(matchGEN);
    ws->import(data);
    ws->Write();
    fout->Close();

}
