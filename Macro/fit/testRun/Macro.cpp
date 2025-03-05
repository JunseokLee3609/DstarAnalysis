#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
void Macro(){
    FitOpt D0opt;
    D0opt.D0Default();
    std::string particleType = "D0";
    std::vector<double> ptBins = {0.0, 1.0, 2.0, 3.0};
    std::vector<double> etaBins = {0.0, 0.5, 1.0};
    std::string filepath = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_D0MC.root";
    std::string datasetName = "dataset";
    MassFitter fitter(D0opt.name,D0opt.massMin,D0opt.massMax);
    ParamMap params = ParamMaker(ptBins,etaBins);
    // RooDataSet* dataset = (RooDataSet*)TFile::Open(filepath.c_str())->Get(datasetName.c_str());
    // fitter.SetData(dataset);
    // fitter.ApplyCut(D0opt.cutExpr);
    // fitter.init(D0opt, filepath, true, "","", params[{0.0,0.0}].first,params[{0.0,0.0}].second);
    PlotManager plotManager(D0opt);
    plotManager.DrawRawDistribution();
    plotManager.DrawFittedModel(true);
    

    




}
