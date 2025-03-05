#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <ROOT/TThreadExecutor.hxx>
#include "TSystemDirectory.h"
#include "TSystemFile.h"
// #include "../interface/simpleDMC.hxx"  // For MC processing
#include "../interface/simpleDMC.h"  // For MC processing
int test(const std::string &filePath,unsigned int i, bool isMC=true) {
    cout << "AA " << endl;
    // return 0;
    std::cout << "Processing file: " << filePath << std::endl;

    // Open the input file
    TFile* fin = TFile::Open(filePath.c_str());
    if (!fin || fin->IsZombie()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return -1;
    }
    
    
    // Retrieve the TTree from the file
    // (Adjust the tree name as needed; here we assume an MC tree name)
    std::string treeName;
   
    treeName = isMC ? "dStarana/PATCompositeNtuple" : "d0ana_newreduced/PATCompositeNtuple";
    TTree* tree = (TTree*) fin->Get(treeName.c_str());
    if (!tree) {
        std::cerr << "Error: TTree '" << treeName << "' not found in file " << filePath << std::endl;
        fin->Close();
        return -1;
    }
    
    // Set up your processing objects (for example, for MC)
    using namespace DataFormat;
    // simpleDMCTreeevt dinMC;
    simpleDMCTreeflat doutMC;
    // // For data, you would use your Data variants instead

    // // Attach the TTree to your objects
    // if (isMC) {
    //     dinMC.setTree<TTree>(tree);
    //     dinMC.setGENTree<TTree>(tree);  // if needed for GEN info
    // } else {
    //     // For data processing:
    //     // simpleDStarDataTreeevt dinData;
    //     // simpleDStarDataTreeflat doutData;
    //     // dinData.setTree<TTree>(tree);
    // }
    
    // // Create a unique output file name (for instance, based on the input file name)
    // // Here we use a simple scheme: prefix "flatSkim_" + original file name.
    
    // // Create the output tree(s)
    // TString date = "0220";
    // std::string outFileName = Form("/home/jun502s/DstarAna/DStarAnalysis/Data/MC/SkimFile_%s/flatSkim_",date.Data()) + std::to_string(i) + ".root";
    // TFile* fout = new TFile(outFileName.c_str(), "recreate");
    // fout->cd();
    // TTree* tskim = new TTree("skimTreeFlat", "");
    // TTree* tGENskim = isMC ? new TTree("skimGENTreeFlat", "") : nullptr;
    
    // // Branch to record event number
    // int iEvent = 0;
    // tskim->Branch("evtNo", &iEvent);
    
    // // Set output branches on your processing objects
    // if (isMC) {
    //     doutMC.setOutputTree(tskim);
    //     if (tGENskim) doutMC.setGENOutputTree(tGENskim);
    // } else {
    //     // For data:
    //     // doutData.setOutputTree(tskim);
    // }
    
    // // Loop over events in the file
    // Long64_t totEvt = tree->GetEntries();
    // for (Long64_t i = 0; i < totEvt; ++i) {
    //     if (i % 10000 == 0)
    //         std::cout << "  Processing event: " << i << std::endl;
    //     tree->GetEntry(i);
    //     iEvent = i;  // update event number in the branch
        
    //     if (isMC) {
    //         // Loop over candidate entries and fill output tree for MC
    //         for (auto iD1 : ROOT::TSeqI(dinMC.candSize)) {
    //             doutMC.copyDn(dinMC, iD1);
    //             tskim->Fill();
    //         }
    //         // Also process GEN information if available
    //         for (auto iD1 : ROOT::TSeqI(dinMC.candSize_gen)) {
    //             doutMC.copyGENDn(dinMC, iD1);
    //             tGENskim->Fill();
    //         }
    //     } else {
    //         // For Data:
    //         // for (auto iD1 : ROOT::TSeqI(dinData.candSize)) {
    //         //     doutData.copyDn(dinData, iD1);
    //         //     tskim->Fill();
    //         // }
    //     }
    // }
    
    
    // // Write and close the output file
    // tskim->Write();
    // if (isMC && tGENskim) tGENskim->Write();
    // fout->Write();
    // fout->Close();
    
    // // Clean up
    // fin->Close();
    // delete fin;
    
    // std::cout << "Finished processing " << filePath << " (" << totEvt << " events)" << std::endl;
    // return totEvt;
}