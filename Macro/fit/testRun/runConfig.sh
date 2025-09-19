#!/bin/bash



# root -l -b -q DStarAnalysisV2forpp.cpp\(1,0,1,1,5,7,0,0.2,0,10,\"/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/parameters/dstar_parameters_grid_unified_pp_v2.json\"\)
# root -l -b -q DStarAnalysisV2forpp.cpp\(0,1,1,1,5,7,0,0.2,0,10,\"/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/parameters/dstar_parameters_DBCrystalBall_Phenomenological2.json\",0,1\)
# root -l -b -q DStarAnalysisV2forpp.cpp\(1,1,1,1,5,7,0,1,0,10,\"/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/parameters/dstar_parameters_DBCrystalBall_Phenomenological2.json\",0,0\)
root -l -b -q DStarAnalysisV2forpp.cpp\(1,0,1,1,5,7,0.8,1,0,100,\"/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/parameters/dstar_parameters_DBCrystalBall_Phenomenological2.json\",0,0\)
# root -l -b -q DStarAnalysisV2forpp.cpp\(1,1,1,1,5,7,0,0.2,0,10,\"/home/jun502s/DstarAna/DStarAnalysis/Macro/fit/parameters/dstar_parameters_DoubleGaussian_Phenomenological.json\"\)