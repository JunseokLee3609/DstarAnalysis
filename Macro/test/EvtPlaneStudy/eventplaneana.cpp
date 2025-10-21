#include "EventPlaneAnalyzer.h"
#include "EventPlaneAnalyzerFlat.h"
#include "EventPlaneAnalyzerFinal.h"
void eventplaneana(bool doRec = true, bool doFlat = true, bool doFinal = true) {

    TString inputPath= "/home/jun502s/DstarAna/DStarAnalysis/Data/test/evtplanestudy/output.root";
    if(doRec){
        cout << "EventPlaneAnalyzerRec" << endl;
    EventPlaneAnalyzer analyzer(
        "EvtPlane.MB.ForRec",
        10, // trigIdx
        100, 160 // centrality range
    );
    analyzer.Analyze(inputPath.Data());
}
if(doFlat){
    cout << "EventPlaneAnalyzerFlat" << endl;
    EventPlaneAnalyzerFlat analyzerFlat(
        "EvtPlane.MB.ForRec.root", // centering file
        "EvtPlane.MB.ForFlat", // output prefix
        10, // trigIdx
        100, 160 // centrality range
    );
    analyzerFlat.Analyze(inputPath.Data());
}
if(doFinal){
    cout << "EventPlaneAnalyzerFinal" << endl;
    EventPlaneAnalyzerFinal analyzerFinal(
        "EvtPlane.MB.ForRec.root",
        "EvtPlane.MB.ForFlat.root", // output prefix
        "EvtPlane.MB.Final",
        10, // trigIdx
        100, 160 // centrality range
    );
    analyzerFinal.Analyze(inputPath.Data());
}





    // return analyzer.Analyze(argv[1]);
}