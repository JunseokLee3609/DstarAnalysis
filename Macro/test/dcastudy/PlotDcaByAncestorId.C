// PlotDcaByAncestorId.C
// - Scans a ROOT file for a TTree containing branches "matchGen_D1ancestorId_" and "dca3D"
// - Counts unique ancestor IDs in [500,600]
// - Draws and saves dca3D distributions per ancestor ID

#include "TFile.h"
#include "TTree.h"
#include "TKey.h"
#include "TClass.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TString.h"
#include <map>
#include <memory>
#include <iostream>

static TTree* FindTreeWithBranches(TFile* f, const char* br1, const char* br2) {
    if (!f) return nullptr;
    TIter nextkey(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)nextkey())) {
        TObject* obj = key->ReadObj();
        if (!obj) continue;
        if (obj->InheritsFrom(TTree::Class())) {
            TTree* t = dynamic_cast<TTree*>(obj);
            if (!t) continue;
            if (t->GetBranch(br1) && t->GetBranch(br2)) {
                return t;
            }
        }
    }
    return nullptr;
}

void PlotDcaByAncestorId(const char* infile = "../../skim/Data/FlatSample/ppMC/DStar/flatSkimForBDT_DStar_MixDStar_MC_25Jul_0_.root",
                         const char* outdir = "plots_dca_by_ancestor")
{
    // Open file
    std::unique_ptr<TFile> fin(TFile::Open(infile, "READ"));
    if (!fin || fin->IsZombie()) {
        std::cerr << "[Error] Cannot open file: " << infile << std::endl;
        return;
    }

    // Try to find a suitable tree
    TTree* tree = FindTreeWithBranches(fin.get(), "matchGen_D1ancestorId_", "dca3D");
    if (!tree) {
        // Fallback: try common flat tree names
        tree = dynamic_cast<TTree*>(fin->Get("skimTreeFlat"));
        if (tree && !(tree->GetBranch("matchGen_D1ancestorId_") && tree->GetBranch("dca3D"))) tree = nullptr;
    }
    if (!tree) {
        std::cerr << "[Error] No TTree in file with branches 'matchGen_D1ancestorId_' and 'dca3D'." << std::endl;
        fin->ls();
        return;
    }

    std::cout << "[Info] Using tree: " << tree->GetName() << " with " << tree->GetEntries() << " entries" << std::endl;

    // Set up branches (flat per-candidate form expected)
    Int_t   ancId = -999;
    Float_t dca   = -99.f;
    tree->SetBranchAddress("matchGen_D1ancestorId_", &ancId);
    tree->SetBranchAddress("dca3D", &dca);

    // Prepare output dir
    if (gSystem->AccessPathName(outdir)) {
        gSystem->MakeDirectory(outdir);
    }

    // Histograms (lazy-created per ancestor ID)
    std::map<int, std::unique_ptr<TH1F>> hists;
    std::map<int, Long64_t> counts;

    const int nbins = 50;     // binning for dca3D
    const double xmin = 0.0;  // expected typical range
    const double xmax = 0.2;

    const Long64_t n = tree->GetEntries();
    for (Long64_t i = 0; i < n; ++i) {
        tree->GetEntry(i);
        if (ancId >= 500 && ancId <= 600) {
            // Create histogram on first sight
            if (!hists.count(ancId)) {
                TString hname; hname.Form("h_dca3D_id%d", ancId);
                TString htitle; htitle.Form("dca3D for matchGen_D1ancestorId_=%d;dca3D;Counts", ancId);
                hists[ancId] = std::unique_ptr<TH1F>(new TH1F(hname, htitle, nbins, xmin, xmax));
                hists[ancId]->SetLineWidth(2);
            }
            hists[ancId]->Fill(dca);
            counts[ancId]++;
        }
    }

    // Summary and plotting
    std::cout << "[Result] Ancestor ID counts in [500,600]:" << std::endl;
    for (const auto& kv : counts) {
        std::cout << "  ID " << kv.first << ": " << kv.second << std::endl;
    }

    // Save per-ID plots
    std::unique_ptr<TCanvas> c(new TCanvas("c", "c", 800, 600));
    for (auto& kv : hists) {
        int id = kv.first;
        TH1F* h = kv.second.get();
        h->SetLineColor(kAzure + (id % 9));
        h->Draw("HIST");
        TString out; out.Form("%s/dca3D_id%d.pdf", outdir, id);
        c->SaveAs(out);
    }
    TCanvas *cptr = new TCanvas("cptr", "cptr", 800, 600);
    for (auto& kv : hists) {
        int id = kv.first;
        TH1F* h = kv.second.get();
        h->SetLineColor(kAzure + (id % 9));
        h->Scale(1.0 / h->Integral(), "width"); // Normalize to unit area
        if (id == hists.begin()->first) {
            h->Draw("HIST");
        } else {
            h->Draw("HIST SAME");
        }
    }
    cptr->BuildLegend(0.7, 0.7, 0.9, 0.9);
    TString out; out.Form("%s/dca3D_all_normalized.pdf", outdir);
    cptr->SaveAs(out);

    // Also save a combined ROOT file with all histograms
    TString rootOut; rootOut.Form("%s/dca3D_by_ancestor.root", outdir);
    std::unique_ptr<TFile> fout(TFile::Open(rootOut, "RECREATE"));
    if (fout && fout->IsOpen()) {
        for (auto& kv : hists) {
            kv.second->Write();
        }
        fout->Write();
        fout->Close();
    }

    std::cout << "[Done] Wrote per-ID plots to: " << outdir << std::endl;
}

