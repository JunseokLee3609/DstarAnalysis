// Plot grids of massPion fit plots for combined kinematic bins with DCA mode comparison
// Creates separate canvases for each DCA mode (inclusive, prompt_rich, nonprompt_rich)
// Usage:
//   root -l -b -q 'PlotKinematicGrid.cpp("PbPb_Data_EPtransformation")'

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

#include "TFile.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TSystem.h"
#include "RooPlot.h"

#include "KinematicBinConfig.h"

using namespace KinematicBinConfig;

static const char* OUTDIR = "results/compare_grids";

// Use DCAMode from KinematicBinConfig - no need for local enum
// DCAMode values: INCLUSIVE=0, PROMPT_RICH=1, NONPROMPT_RICH=2

// Wrapper for compatibility with existing DCAModeStr calls
static std::string DCAModeStr(int mode) {
  return KinematicBinConfig::DCAModeToString(mode);
}

static std::string fmtVal(double v, int prec) {
  std::ostringstream oss; oss << std::fixed << std::setprecision(prec) << v; return oss.str();
}

static std::string fmtValP(double v, int prec) {
  std::string s = fmtVal(v, prec); std::replace(s.begin(), s.end(), '.', 'p'); return s;
}

static std::string buildFitRootPath(const std::string& datasetKey,
                                     double ptMin, double ptMax,
                                     double yMin, double yMax,
                                     double cMin, double cMax,
                                     double mva,
                                     int dcaMode,  // Changed from DCAMode to int
                                     double cosMin, double cosMax) {
  std::ostringstream on;
  on << "results/reduced_hist/hist_" << datasetKey
     << "_pt"   << fmtValP(ptMin,1)  << "_" << fmtValP(ptMax,1)
     << "_y"    << fmtValP(yMin,1)   << "_" << fmtValP(yMax,1)
     << "_cent" << fmtValP(cMin,0)   << "_" << fmtValP(cMax,0)
     << "_cosEP" << fmtValP(cosMin,2) << "_" << fmtValP(cosMax,2)
     << "_mva"  << fmtValP(mva,3)    
     << "_" << DCAModeStr(dcaMode)
     << "_fitResult.root";
  return on.str();
}

// Global container to keep TFile objects alive
static std::vector<TFile*> g_openFiles;

static RooPlot* loadFitFrame(const std::string& rootPath) {
  TFile* f = TFile::Open(rootPath.c_str(), "READ");
  if (!f || f->IsZombie()) {
    return nullptr;
  }
  
  RooPlot* frame = (RooPlot*)f->Get("massFrame");
  if (!frame) {
    f->Close();
    return nullptr;
  }
  
  // Keep file open
  g_openFiles.push_back(f);
  return frame;
}

void PlotKinematicGrid(const char* datasetKey = "PbPb_Data_Oct29_mva0p9") {
  gStyle->SetOptStat(0);
  gSystem->mkdir(OUTDIR, true);

  // Use global kinematic bin configuration
  const int nPt = N_PT_BINS;
  const int nY  = N_Y_BINS;
  const int nCent = N_CENT_BINS;
  const int nMVA = N_MVA;
  const int nDCA = N_DCA_MODES;
  const int nCos = N_COSTHETA_BINS;  // Get cos bins from config

  const DCAMode dcaModes[] = {INCLUSIVE, PROMPT_RICH, NONPROMPT_RICH};
  
  // Loop over all DCA modes
  for (int id = 0; id < nDCA; ++id) {
    DCAMode dcaMode = dcaModes[id];
    
    // Loop over all (MVA, Centrality, Cos) combinations
    for (int im = 0; im < nMVA; ++im) {
      for (int ic = 0; ic < nCent; ++ic) {
        for (int icos = 0; icos < nCos; ++icos) {  // Loop over cos bins
          double cosMin = COSTHETA_EDGES[icos];
          double cosMax = COSTHETA_EDGES[icos+1];
          
          // Create one canvas per MVA x Centrality x Cos x DCA combination
          int W = 3200, H = 1200;  // 5 columns x 3 rows
          std::string canvasTitle = "grid_" + DCAModeStr(dcaMode) + "_mva_" + fmtValP(MVA_THRESHOLDS[im], 3) + 
                                    "_cent_" + fmtValP(CENT_EDGES[ic], 0) + "_" + fmtValP(CENT_EDGES[ic+1], 0) +
                                    "_cos_" + fmtValP(cosMin, 2) + "_" + fmtValP(cosMax, 2);
          TCanvas* c = new TCanvas(canvasTitle.c_str(), canvasTitle.c_str(), W, H);
          
          // Divide canvas into nPt (cols) x nY (rows) subpads
          c->Divide(nPt, nY, 0.001, 0.001);

          // Preload all fit frames for this panel
          std::vector<RooPlot*> panelFrames(nY*nPt, nullptr);
          int loadedCount = 0;
          
          for (int iy = 0; iy < nY; ++iy) {
            double yMin = Y_EDGES[iy], yMax = Y_EDGES[iy+1];
            for (int ip = 0; ip < nPt; ++ip) {
              double ptMin = PT_EDGES[ip], ptMax = PT_EDGES[ip+1];
              std::string rootPath = buildFitRootPath(datasetKey, ptMin, ptMax, yMin, yMax, 
                                                      CENT_EDGES[ic], CENT_EDGES[ic+1], 
                                                      MVA_THRESHOLDS[im], (int)dcaMode,  // Cast to int
                                                      cosMin, cosMax);
              
              RooPlot* frame = loadFitFrame(rootPath);
              panelFrames[iy*nPt + ip] = frame;
              if (frame) loadedCount++;
            }
          }
          
          std::cout << "Canvas " << canvasTitle << ": loaded " << loadedCount << "/" << (nY*nPt) << " plots" << std::endl;

          // Draw subpads: row=|y|, col=pT
          for (int iy = 0; iy < nY; ++iy) {
            for (int ip = 0; ip < nPt; ++ip) {
              int sub = iy*nPt + ip + 1;  // 1-indexed pad number
              c->cd(sub);
              gPad->SetFillColor(0);
              gPad->SetMargin(0.15, 0.05, 0.12, 0.05);  // left, right, bottom, top

              RooPlot* frame = panelFrames[iy*nPt + ip];
              if (frame) {
                // Draw the RooPlot directly
                frame->Draw("");
              } else {
                // Draw empty frame with message
                TLatex noData;
                noData.SetNDC();
                noData.SetTextFont(42);
                noData.SetTextSize(0.15);
                noData.SetTextColor(kRed);
                noData.DrawLatex(0.1, 0.5, "No data");
              }

              // Add bin labels
              // pT label on top row
              if (iy == 0) {
                TLatex tx; tx.SetNDC(); tx.SetTextFont(42); tx.SetTextSize(0.12);
                tx.DrawLatex(0.15, 0.85, ("pT=" + fmtVal(PT_EDGES[ip],0) + "-" + fmtVal(PT_EDGES[ip+1],0)).c_str());
              }
              // |y| label on left column
              if (ip == 0) {
                TLatex ty; ty.SetNDC(); ty.SetTextFont(42); ty.SetTextSize(0.12);
                ty.DrawLatex(0.05, 0.50, ("|y|=" + fmtVal(Y_EDGES[iy],1) + "-" + fmtVal(Y_EDGES[iy+1],1)).c_str());
              }
            }
          }

          // Add canvas-level title
          c->cd();
          TLatex title; title.SetNDC(); title.SetTextFont(62); title.SetTextSize(0.03);
          title.DrawLatex(0.02, 0.98, (std::string("DCA: ") + DCAModeStr(dcaMode) + 
                                       "  |  MVA > " + fmtVal(MVA_THRESHOLDS[im],3) + 
                                       "  |  Centrality [" + fmtVal(CENT_EDGES[ic],0) + 
                                       "," + fmtVal(CENT_EDGES[ic+1],0) + "]" +
                                       "  |  cos(EP) [" + fmtVal(cosMin,2) + "," + fmtVal(cosMax,2) + "]").c_str());

          // Save
          std::string savePath = std::string(OUTDIR) + "/grid_" + DCAModeStr(dcaMode) + 
                                 "_mva_" + fmtValP(MVA_THRESHOLDS[im],3) + 
                                 "_cent_" + fmtValP(CENT_EDGES[ic],0) + "_" + fmtValP(CENT_EDGES[ic+1],0) +
                                 "_cos_" + fmtValP(cosMin,2) + "_" + fmtValP(cosMax,2);
          c->SaveAs((savePath + ".png").c_str());
          c->SaveAs((savePath + ".pdf").c_str());
          c->SaveAs((savePath + ".root").c_str());
          std::cout << "Saved: " << savePath << ".{png,pdf,root}" << std::endl;

          delete c;
        }
      }
    }
  }
  
  // Close all files
  for (auto f : g_openFiles) {
    if (f) f->Close();
  }
  g_openFiles.clear();
  
  std::cout << "\n=== Summary ===" << std::endl;
  std::cout << "Processed " << nDCA << " DCA modes x " << nMVA << " MVA thresholds x " << nCent 
            << " centrality ranges x " << nCos << " cos bins = " 
            << (nDCA*nMVA*nCent*nCos) << " canvases" << std::endl;
  std::cout << "Each canvas: " << nPt << " pT bins x " << nY << " |y| bins = " << (nPt*nY) << " subpads" << std::endl;
}
