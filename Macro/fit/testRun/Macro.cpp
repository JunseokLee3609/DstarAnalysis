#include "../MassFitter.h"
#include "../Opt.h"
#include "../Params.h"
#include "../Helper.h"
#include "../PlotManager.h"
#include "../DataLoader.h"
void Macro(bool doFit = false,bool useCUDA=true){
    FitOpt D0opt;
    D0opt.useCUDA = useCUDA;
    D0opt.doFit = doFit;
    cout << D0opt.mvaMin << endl; 
    std::string particleType = "D0";
    std::vector<double> ptBins = {0.0, 1.0, 2.0, 3.0};
    std::vector<double> etaBins = {0.0, 0.5, 1.0};
    std::vector<double> mvabin = {-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
    // std::string filepath = "/home/jun502s/DstarAna/DStarAnalysis/Data/RDS_MC/RDS_D0DATAMVA.root";
    std::string filepath = "/home/jun502s/DstarAna/DStarAna/data/DstarData_Rds_CUDA_pp.root";
    std::string datasetName = "dataset";
    DStarParamMap params = D0ParamMaker(ptBins,etaBins);
    DataLoader loader(filepath);
    loader.loadRooDataSet(datasetName,"workspace");
    // RooDataSet* dataset = (RooDataSet*)TFile::Open(filepath.c_str())->Get(datasetName.c_str());
    // fitter.SetData(dataset);
    // fitter.ApplyCut(D0opt.cutExpr);
    // for(auto mva : mvabin){
    // D0opt.mvaMin = mva;
    D0opt.DStarDataDefault();
    MassFitter fitter(D0opt.name,loader.getDataSet(),D0opt.massVar,D0opt.massMin,D0opt.massMax);
    fitter.PerformFit(D0opt, true, "","", params[{0.0,0.0}].first,params[{0.0,0.0}].second);
    // fitter.PlotResult(true,D0opt.outputFile);
    PlotManager plotManager(D0opt);
    if(!doFit){
        plotManager.DrawRawDistribution();
    }
    plotManager.DrawFittedModel(true);
    // }
}
    

    





