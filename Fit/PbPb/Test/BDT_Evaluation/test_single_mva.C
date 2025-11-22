void test_single_mva(){
  gROOT->ProcessLine(".L CorrectYieldsWithMVA_Stage1_Independent.cpp");
  CorrectYieldsWithMVA_Stage1_Independent(0, "results/data_fits", "prompt");
}
