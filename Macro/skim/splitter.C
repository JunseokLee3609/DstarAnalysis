#include "src/common/interface/commonHeader.hxx"
#include "src/common/interface/simpleAlgos.hxx"

#include <fstream>
#include <type_traits>

#include "src/dataformat/interface/simpleD.hxx"
#include "src/dataformat/interface/simpleDMC.hxx"

void splitter(){
    //std::string date="2411"
    TFile *file = TFile::Open("../../data/flatSkimForBDT_isMC1_0_241109.root");
    TTree *tree = (TTree*)file->Get("skimTreeFlat");  // Replace 'tree_name' with the actual name
    TFile *testFile = new TFile("../../data/flatSkimForBDT_Validation_241109.root", "RECREATE");
    TTree *testTree = tree->CloneTree(0);   // Create an empty clone of the original tree for testing data
    TFile *trainFile = new TFile("../../data/flatSkimForBDT_Training_241109.root", "RECREATE");
    TTree *trainTree = tree->CloneTree(0);  // Create an empty clone of the original tree for training data
    using namespace DataFormat;
    simpleDStarMCTreeflat dinMC;
    dinMC.setTree<TTree>(tree);
    // cout <<trainTree << endl;
    // tree->CopyAddresses(trainTree);
    // cout <<testTree << endl;
    // dinMC.setTree<TTree>(testTree);
    // dinMC.setTree<TTree>(trainTree);
    
    // simpleDStarMCTreeflat doutMC;
    // simpleDStarMCTreeevt dinData;
    // simpleDStarMCTreeflat dout;
    // std::cout << tfinderMC->GetEntries() << std::endl;
    // dinData.setTree<TTree>(tfinderData);



TRandom3 randGen;  // Random number generator

Long64_t nentries = tree->GetEntries();
// Long64_t nentries = 10000;
for (Long64_t i = 0; i < nentries; i++) {
    tree->GetEntry(i);
    // cout<< dinMC.mass << endl;

    if (randGen.Uniform() < 0.7) {  // 70% for training set
        trainTree->Fill();
    } else {  // 30% for test set
        testTree->Fill();
    }
}
trainFile->cd();
trainTree->Write();
// trainFile->Write();

testFile->cd();
testTree->Write();
// testFile->Write();
testFile->Close();
trainFile->Close();

cout << "done" << endl;

}
