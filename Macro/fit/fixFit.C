#include "Fit1D.C"
#include "Fit1D_VertexNtuple.C"
#include "Fit1D_VertexNtuple_DPP.C"
#include "Fit1D_VertexNtuple_DKK.C"
// #include "Fit2D.C"
#include "Fit2D_CrossUpdate.C"
#include "Fit2D_CrossUpdate_SeparateSwp.C"
#include "Fit2D_CrossUpdate_ConstSwp.C"
#include "Fit2D_CrossUpdate_SeparateSwpNoSigBkg.C"
#include "Fit2D_CrossUpdate_ConstSwpNoSigBkg.C"
#include "simpleDDMassFit_v1.C"
#include "src/plot/src/fitresultplotter.cxx"
// #include "Fit1DDCA.C"


using namespace ANA;

void fixFit( string fname = "/home/vince402/Analysis/DDbarpPb/macros/output/root/fit/MinBias_ResFit2D_pT2_y0_ntrk0_isMC0_sel2D_AD0sig_MVALoose_DCA_Tight_dphi1_FineBin_DD_OS_ConstSwp.root" ){

    const int nCore = 20;
    const float fitRangeDo = 1.74;
    const float fitRangeUp = 2.00;
    TFile* fIn = TFile::Open(fname.c_str(), "update");

    RooWorkspace* wksp = (RooWorkspace*) fIn->Get("wksp2d");
    RooFitResult* _prim_fitRes = (RooFitResult*) fIn->Get("fitresult_model_myDataSet_reduced");
    RooAddPdf* model = (RooAddPdf*) fIn->Get("model");
    RooDataSet* dataset = (RooDataSet*) fIn->Get("myDataSet_reduced");
    TNamed* bkgOrder = (TNamed*) fIn->Get("idx");
    int bkgOrderVal = stoi(bkgOrder->GetTitle());

    RooFitResult* _prim_fitRes_sigShape = (RooFitResult*) fIn->Get("fitresult_model_myDataSet_reduced_step1");
    RooFitResult* _prim_fitRes_bkgShape = (RooFitResult*) fIn->Get(Form("fitresult_model_myDataSet_bkgX%dY%d", bkgOrderVal, bkgOrderVal));
    RooFitResult* fitResCol[3];
    bool needFinalFitRefit = ((_prim_fitRes->statusCodeHistory(0)!=0) || (_prim_fitRes->statusCodeHistory(1)!=0)|| (_prim_fitRes->statusCodeHistory(2)!=0 && strcmp(_prim_fitRes->statusLabelHistory(2), "MINOS")!=0));
    bool needSigShapeRefit = ((_prim_fitRes_sigShape->statusCodeHistory(0)!=0) || (_prim_fitRes_sigShape->statusCodeHistory(1)!=0)|| (_prim_fitRes_sigShape->statusCodeHistory(2)!=0&& strcmp(_prim_fitRes_sigShape->statusLabelHistory(2), "MINOS")!=0));
    bool needBkgShapeRefit = ((_prim_fitRes_bkgShape->statusCodeHistory(0)!=0) || (_prim_fitRes_bkgShape->statusCodeHistory(1)!=0)|| (_prim_fitRes_bkgShape->statusCodeHistory(2)!=0&& strcmp(_prim_fitRes_bkgShape->statusLabelHistory(2), "MINOS")!=0));
    if( needSigShapeRefit){
        needBkgShapeRefit = true;
        needFinalFitRefit = true; 
        auto fpf  = _prim_fitRes_sigShape->floatParsFinal();
        auto cpf  = _prim_fitRes_sigShape->constPars();
        auto modelVars = *model->getVariables();
        modelVars.Print();
        for( int idx=0;idx<cpf.size();idx++ ){
            auto var = (TObject*) &cpf[idx];
            string _vname_ = string(var->GetName());
            auto pdfVar = (RooRealVar*)  modelVars.find(_vname_.c_str());
            pdfVar->setConstant(kTRUE);
        }
        for( int idx=0;idx<fpf.size();idx++ ){
            auto var = (RooRealVar*) &fpf[idx];
            string _vname_ = string(var->GetName());
            auto pdfVar = (RooRealVar*)  modelVars.find(_vname_.c_str());
            // Don't touch background params
            if((_vname_.find("ax")==0 ||_vname_.find("ax")==0) && _vname_.size()==3) continue; 
            double _val_ = var->getVal();
            double _limup_ = var->getMax();
            double _limdown_ = var->getMin();
            double _errup_ = var->getErrorHi();
            double _errdo_ = var->getErrorLo();
            double _errsym_ = var->getError();
            if( _val_ + 3* _errup_ > _limup_ ){
                if(_vname_.find("frac")!=std::string::npos ) continue;
                pdfVar->setMax( _val_ + _errup_*9.0);
            }
            if(_val_ + 3*_errdo_ < _limdown_ ){
                // Don't touch low limit when fraction or sigma
                if( _vname_.find("sigma")!=std::string::npos || _vname_.find("frac")!=std::string::npos ) continue;
                pdfVar->setMin( _val_ + _errdo_*9.0);

            }
        }

        RooLinkedList fitOpts;

        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(nCore, 0)));
        fitOpts.Add(new RooCmdArg(RooFit::Save()));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit","minimize")));
        fitOpts.Add(new RooCmdArg(RooFit::Extended(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::BatchMode(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::Hesse(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::Range(fitRangeDo, fitRangeUp)));
        fitOpts.Add(new RooCmdArg(RooFit::Minos(kTRUE)));

        fitResCol[0] = model->fitTo(*dataset, fitOpts);
    }
    if( needBkgShapeRefit){
        needFinalFitRefit = true; 
        auto fpf  = _prim_fitRes_bkgShape->floatParsFinal();
        auto cpf  = _prim_fitRes_bkgShape->constPars();
        auto modelVars = *model->getVariables();
        modelVars.Print();
        for( int idx=0;idx<cpf.size();idx++ ){
            auto var = (TObject*) &cpf[idx];
            string _vname_ = string(var->GetName());
            auto pdfVar = (RooRealVar*)  modelVars.find(_vname_.c_str());
            pdfVar->setConstant(kTRUE);
        }
        for( int idx=0;idx<fpf.size();idx++ ){
            auto var = (RooRealVar*) &fpf[idx];
            string _vname_ = string(var->GetName());
            auto pdfVar = (RooRealVar*)  modelVars.find(_vname_.c_str());
            // Don't touch background params
            if((_vname_.find("ax")==0 ||_vname_.find("ax")==0) && _vname_.size()==3) continue; 
            double _val_ = var->getVal();
            double _limup_ = var->getMax();
            double _limdown_ = var->getMin();
            double _errup_ = var->getErrorHi();
            double _errdo_ = var->getErrorLo();
            double _errsym_ = var->getError();
            if( _val_ + 3* _errup_ > _limup_ ){
                if(_vname_.find("frac")!=std::string::npos ) continue;
                pdfVar->setMax( _val_ + _errup_*9.0);
            }
            if(_val_ + 3*_errdo_ < _limdown_ ){
                // Don't touch low limit when fraction or sigma
                if( _vname_.find("sigma")!=std::string::npos || _vname_.find("frac")!=std::string::npos ) continue;
                pdfVar->setMin( _val_ + _errdo_*9.0);

            }
        }

        RooLinkedList fitOpts;

        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(nCore, 0)));
        fitOpts.Add(new RooCmdArg(RooFit::Save()));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit","minimize")));
        fitOpts.Add(new RooCmdArg(RooFit::Extended(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::BatchMode(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::Hesse(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::Range(fitRangeDo, fitRangeUp)));
        fitOpts.Add(new RooCmdArg(RooFit::Minos(kTRUE)));

        fitResCol[1] = model->fitTo(*dataset, fitOpts);
    }
    if( needFinalFitRefit){
        auto fpf  = _prim_fitRes->floatParsFinal();
        auto cpf  = _prim_fitRes->constPars();
        auto modelVars = *model->getVariables();
        modelVars.Print();
        for( int idx=0;idx<cpf.size();idx++ ){
            auto var = (TObject*) &cpf[idx];
            string _vname_ = string(var->GetName());
            auto pdfVar = (RooRealVar*)  modelVars.find(_vname_.c_str());
            pdfVar->setConstant(kTRUE);
        }
        for( int idx=0;idx<fpf.size();idx++ ){
            auto var = (RooRealVar*) &fpf[idx];
            string _vname_ = string(var->GetName());
            auto pdfVar = (RooRealVar*)  modelVars.find(_vname_.c_str());
            // Don't touch background params
            if((_vname_.find("ax")==0 ||_vname_.find("ax")==0) && _vname_.size()==3) continue; 
            double _val_ = var->getVal();
            double _limup_ = var->getMax();
            double _limdown_ = var->getMin();
            double _errup_ = var->getErrorHi();
            double _errdo_ = var->getErrorLo();
            double _errsym_ = var->getError();
            if( _val_ + 3* _errup_ > _limup_ || (_val_ >0 && 1.2*_val_ > _limup_ )|| (_val_ <0 && 0.8*_val_ > _limup_ )){
                if(_vname_.find("frac")!=std::string::npos ) continue;
                pdfVar->setMax( _val_ + _errup_*9.0);
            }
            if(_val_ + 3*_errdo_ < _limdown_ || (_val_ >0 && 0.8*_val_ < _limdown_ )|| (_val_ <0 && 1.2*_val_ < _limdown_ )){
                // Don't modify toom uch the low limit when fraction or sigma
                if( _vname_.find("sigma")!=std::string::npos ){pdfVar->setMin(0.01*pdfVar->getMin()); continue;}
                if( _vname_.find("frac")!=std::string::npos ) continue;
                pdfVar->setMin( _val_ + _errdo_*9.0);

            }
        }

        RooLinkedList fitOpts;

        fitOpts.Add(new RooCmdArg(RooFit::NumCPU(nCore, 0)));
        fitOpts.Add(new RooCmdArg(RooFit::Save()));
        fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit","minimize")));
        fitOpts.Add(new RooCmdArg(RooFit::Extended(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::BatchMode(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::Hesse(kTRUE)));
        fitOpts.Add(new RooCmdArg(RooFit::Range(fitRangeDo, fitRangeUp)));
        fitOpts.Add(new RooCmdArg(RooFit::Minos(kTRUE)));

        fitResCol[2] = model->fitTo(*dataset, fitOpts);
    }

    // auto fpf  = _prim_fitRes->floatParsFinal();
    // auto modelVars = *model->getVariables();
    // for( int idx=0;idx<fpf.size();idx++ ){
    //     auto var = (RooRealVar*) &fpf[idx];
    //     string _vname_ = string(var->GetName());
    //     auto pdfVar = (RooRealVar*)  modelVars.find(_vname_.c_str());
    //     // Don't touch background params
    //     if((_vname_.find("ax")==0 ||_vname_.find("ax")==0) && _vname_.size()==3) continue; 
    //     double _val_ = var->getVal();
    //     double _limup_ = var->getMax();
    //     double _limdown_ = var->getMin();
    //     double _errup_ = var->getErrorHi();
    //     double _errdo_ = var->getErrorLo();
    //     double _errsym_ = var->getError();
    //     if( _val_ + 3* _errup_ > _limup_ ){
    //         if(_vname_.find("frac")!=std::string::npos ) continue;
    //         pdfVar->setMax( _val_ + _errup_*9.0);
    //     }
    //     if(_val_ + 3*_errdo_ < _limdown_ ){
    //         // Don't touch low limit when fraction or sigma
    //         if( _vname_.find("sigma")!=std::string::npos || _vname_.find("frac")!=std::string::npos ) continue;
    //         pdfVar->setMin( _val_ + _errdo_*9.0);
            
    //     }
    // }



    // RooLinkedList fitOpts;

    // fitOpts.Add(new RooCmdArg(RooFit::NumCPU(nCore, 0)));
    // fitOpts.Add(new RooCmdArg(RooFit::Save()));
    // fitOpts.Add(new RooCmdArg(RooFit::Minimizer("Minuit","minimize")));
    // fitOpts.Add(new RooCmdArg(RooFit::Extended(kTRUE)));
    // fitOpts.Add(new RooCmdArg(RooFit::BatchMode(kTRUE)));
    // fitOpts.Add(new RooCmdArg(RooFit::Hesse(kTRUE)));
    // fitOpts.Add(new RooCmdArg(RooFit::Range(fitRangeDo, fitRangeUp)));
    // fitOpts.Add(new RooCmdArg(RooFit::Minos(kTRUE)));

    // RooFitResult* fitRes = model->fitTo(*dataset, fitOpts);

    _prim_fitRes->Print("v");
    std::cout << "\n\n\n POST CORRECTION RESULT 1 \n\n\n" << std::endl;
    std::cout << "\n\n\n POST CORRECTION RESULT 2 \n\n\n" << std::endl;
    std::cout << "\n\n\n POST CORRECTION RESULT 3 \n\n\n" << std::endl;
    // fitRes->Print("v");
    if(needSigShapeRefit) {fitResCol[0]->SetName("fitresult_model_myDataSet_reduced_step1");fitResCol[0]->Write();fitResCol[0]->Print("v");};
    if(needBkgShapeRefit) {fitResCol[1]->SetName(Form("fitresult_model_myDataSet_bkgX%dY%d", bkgOrderVal, bkgOrderVal));fitResCol[1]->Write();fitResCol[1]->Print("v");};
    if(needFinalFitRefit) {fitResCol[2]->SetName("fitresult_model_myDataSet_reduced");fitResCol[2]->Write();fitResCol[2]->Print("v");};

    fIn->Close();

}