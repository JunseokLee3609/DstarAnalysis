// Plot grids of massPion histograms for combined kinematic bins
// One canvas: 3 (centrality) x 2 (mva) panels; each panel contains a grid (rows: |y| bins, cols: pT bins)
// Usage:
//   root -l -b -q 'PlotKinematicGrid.cpp("PbPb_Data_EPtransformation")'
//   root -l -b -q 'PlotKinematicGrid.cpp("PbPb_Data_EPtransformation", 0)'
// The second argument controls normalization (1: area normalize per cell, 0: raw counts)

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSystem.h"

static const char* OUTDIR = "results/compare_grids";

static std::string fmtVal(double v, int prec) {
  std::ostringstream oss; oss << std::fixed << std::setprecision(prec) << v; return oss.str();
}
static std::string fmtValP(double v, int prec) {
  std::string s = fmtVal(v, prec); std::replace(s.begin(), s.end(), '.', 'p'); return s;
}

static std::string buildPath(const std::string& datasetKey,
                             double ptMin, double ptMax,
                             double yMin, double yMax,
                             double cMin, double cMax,
                             double mva) {
  std::ostringstream on;
  on << "results/reduced_hist/hist_" << datasetKey
     << "_pt"   << fmtValP(ptMin,1)  << "_" << fmtValP(ptMax,1)
     << "_y"    << fmtValP(yMin,1)   << "_" << fmtValP(yMax,1)
     << "_cent" << fmtValP(cMin,0)   << "_" << fmtValP(cMax,0)
     << "_mva"  << fmtValP(mva,3)    << ".root";
  return on.str();
}

static TH1D* loadHist(const std::string& path) {
  TFile* f = TFile::Open(path.c_str(), "READ");
  if (!f || f->IsZombie()) return nullptr;
  TH1D* h = (TH1D*)f->Get("hist_massPion");
  if (!h) { f->Close(); return nullptr; }
  TH1D* hc = (TH1D*)h->Clone();
  hc->SetDirectory(0);
  f->Close();
  return hc;
}

void PlotKinematicGrid(const char* datasetKey = "PbPb_Data_EPtransformation", int normalize = 0) {
  gStyle->SetOptStat(0);
  gSystem->mkdir(OUTDIR, true);

  // Define bins
  const double ptEdges[] = {5,7,10,20,30,40,50,60,70,80,90,100,110,120};
  const int nPt = sizeof(ptEdges)/sizeof(double) - 1; // 13

  const double yEdges[]  = {0.0, 0.3, 0.8, 1.2};
  const int nY  = sizeof(yEdges)/sizeof(double) - 1;  // 3 rows

  const double centMins[] = {0, 10, 30};
  const double centMaxs[] = {10, 30, 50};
  const int nCent = 3;

  const double mvas[] = {0.990, 0.999};
  const int nMVA = 2;

  // Loop over all (MVA, Centrality) combinations to create 6 independent canvases
  for (int im = 0; im < nMVA; ++im) {
    for (int ic = 0; ic < nCent; ++ic) {
      // Create one canvas per MVA x Centrality combination
      int W = 4000, H = 1200;  // Wider for 13 columns, narrower for 3 rows
      std::string canvasTitle = "grid_mva_" + fmtValP(mvas[im], 3) + "_cent_" + 
                                fmtValP(centMins[ic], 0) + "_" + fmtValP(centMaxs[ic], 0);
      TCanvas* c = new TCanvas(canvasTitle.c_str(), canvasTitle.c_str(), W, H);
      
      // Divide canvas into nPt (cols) x nY (rows) subpads
      c->Divide(nPt, nY, 0.001, 0.001);

      // Preload all histograms for this panel and find max
      std::vector<TH1D*> panelHists(nY*nPt, nullptr);
      double ymax = 0.0;
      for (int iy = 0; iy < nY; ++iy) {
        double yMin = yEdges[iy], yMax = yEdges[iy+1];
        for (int ip = 0; ip < nPt; ++ip) {
          double ptMin = ptEdges[ip], ptMax = ptEdges[ip+1];
          std::string path = buildPath(datasetKey, ptMin, ptMax, yMin, yMax, centMins[ic], centMaxs[ic], mvas[im]);
          TH1D* h = loadHist(path);
          panelHists[iy*nPt + ip] = h;
          if (h) {
            if (normalize && h->Integral() > 0) h->Scale(1.0 / h->Integral());
            ymax = std::max(ymax, h->GetMaximum());
          }
        }
      }
      if (ymax <= 0) ymax = 1.0;

      // Draw subpads: row=|y|, col=pT
      for (int iy = 0; iy < nY; ++iy) {
        for (int ip = 0; ip < nPt; ++ip) {
          int sub = iy*nPt + ip + 1;  // 1-indexed pad number
          c->cd(sub);
          gPad->SetFillColor(0);
          gPad->SetMargin(0.15, 0.05, 0.12, 0.05);  // left, right, bottom, top

          TH1D* h = panelHists[iy*nPt + ip];
          if (h) {
            h->SetLineColor(kBlack);
            h->SetLineWidth(1);
            h->SetTitle("");
            h->GetXaxis()->SetLabelSize(0.10);
            h->GetYaxis()->SetLabelSize(0.10);
            h->GetXaxis()->SetTitleSize(0.08);
            h->GetYaxis()->SetTitleSize(0.08);
            // Set each histogram's maximum to 1.2 times its own maximum
            h->SetMaximum(h->GetMaximum() * 1.2);
            h->Draw("hist");
          } else {
            // draw empty frame
            TH1D* frame = new TH1D("frame","",10,0.14,0.155);
            frame->SetMaximum(1.0);
            frame->GetXaxis()->SetLabelSize(0.10);
            frame->GetYaxis()->SetLabelSize(0.10);
            frame->Draw();
          }

          // Add bin labels
          // pT label on top row
          if (iy == 0) {
            TLatex tx; tx.SetNDC(); tx.SetTextFont(42); tx.SetTextSize(0.12);
            tx.DrawLatex(0.15, 0.85, ("pT=" + fmtVal(ptEdges[ip],0) + "-" + fmtVal(ptEdges[ip+1],0)).c_str());
          }
          // |y| label on left column
          if (ip == 0) {
            TLatex ty; ty.SetNDC(); ty.SetTextFont(42); ty.SetTextSize(0.12);
            ty.DrawLatex(0.05, 0.50, ("|y|=" + fmtVal(yEdges[iy],1) + "-" + fmtVal(yEdges[iy+1],1)).c_str());
          }
        }
      }

      // Add canvas-level title
      c->cd();
      TLatex title; title.SetNDC(); title.SetTextFont(62); title.SetTextSize(0.03);
      title.DrawLatex(0.02, 0.98, (std::string("MVA > ") + fmtVal(mvas[im],3) + "  |  Centrality [" + 
                                   fmtVal(centMins[ic],0) + "," + fmtVal(centMaxs[ic],0) + "]").c_str());

      // Save
      std::string savePath = std::string(OUTDIR) + "/grid_mva_" + fmtValP(mvas[im],3) + 
                             "_cent_" + fmtValP(centMins[ic],0) + "_" + fmtValP(centMaxs[ic],0);
      c->SaveAs((savePath + ".png").c_str());
      c->SaveAs((savePath + ".pdf").c_str());
      c->SaveAs((savePath + ".root").c_str());
      std::cout << "Saved: " << savePath << ".{png,pdf,root}" << std::endl;

      delete c;
    }
  }
}
