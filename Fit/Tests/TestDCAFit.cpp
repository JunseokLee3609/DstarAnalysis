#include "../DCAFitter.h"

// void TestDCAFit(){
//     std::string MCPath ="/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/flatSkimForBDT_DStar_PPRef_PromptDStar_02Jun_0.root";
//     // std::string DataPath=""
//     DCAFitter fitter("dca",0,0.1,100);
//     fitter.setMCFile(MCPath,"skimTreeFlat");
//     fitter.setDCABranchName("dca3D");
//     //fitter.setMotherPdgIdBranchName("idBAnc_reco");
//     fitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_");
//     fitter.setPromptPdgIds({4});
//     fitter.setNonPromptPdgIds({5});
//     fitter.createTemplatesFromMC();
//     fitter.plotRawDataDistribution();
// }

void TestDCAFit() {

    std::string MCPath = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppMC/DStar/flatSkimForBDT_DStar_PPRef_PromptDStar_02Jun_0.root";
    std::string DataPath = "/home/jun502s/DstarAna/DStarAnalysis/Macro/skim/Data/FlatSample/ppData/DStar/flatSkimForBDT_DStar_PPRef_DStar_Data_09Jun.root"; // <<< 실제 데이터 파일 경로를 입력하세요.

    // --- DCAFitter 객체 생성 ---
    // DCAFitter(이름, DCA 최소값, DCA 최대값, 히스토그램 빈 개수)
    DCAFitter fitter("dcaFitter", 0.0, 0.1, 100);

    // --- 파일 및 트리/브랜치 이름 설정 ---
    fitter.setMCFile(MCPath, "skimTreeFlat");
    fitter.setDataFile(DataPath, "skimTreeFlat"); // 데이터 파일과 트리 이름 설정
    fitter.setDCABranchName("dca3D");
    fitter.setMotherPdgIdBranchName("matchGen_D1ancestorFlavor_"); // MC에서만 사용됨

    // --- 즉발/비-즉발 입자 정의 ---
    // c-quark(4)에서 유래한 D*는 즉발, b-quark(5)에서 유래한 D*는 비-즉발로 정의
    fitter.setPromptPdgIds({4,2});
    fitter.setNonPromptPdgIds({5});

    // std::string commonCuts = "abs(y) < 1.0";
    // fitter.setMCCuts(commonCuts);
    // fitter.setDataCuts(commonCuts);

    // --- 가중치(Weight) 브랜치 설정 (선택 사항) ---
    // 만약 이벤트별 가중치(예: 효율 보정)가 있다면 브랜치 이름을 설정합니다.
    // fitter.setWeightBranchName("yourWeightBranch");

    // --- 결과 저장 파일 설정 ---
    fitter.setOutputFile("DCAFit_Results.root");
    std::cout << "Step 1: Creating templates from MC..." << std::endl;
    if (fitter.createTemplatesFromMC()) {
        fitter.plotRawDataDistribution("MC_Templates_Normalized");

        std::cout << "\nStep 2: Loading data..." << std::endl;
        if (fitter.loadData()) {

            std::cout << "\nStep 3: Building the fit model..." << std::endl;
            if (fitter.buildModel()) {

                std::cout << "\nStep 4: Performing the fit..." << std::endl;
                // fitTo()가 RooFitResult 포인터를 반환하므로, 사용 후 메모리를 해제해야 합니다.
                RooFitResult* fitResult = fitter.performFit(true); // true: MINOS 에러 계산 사용

                if (fitResult) {
                    std::cout << "\nStep 5: Plotting and saving results..." << std::endl;
                    // 피팅 결과를 시각화
                    fitter.plotResults(fitResult, "DCA_Fit_Result_Plot");

                    // 워크스페이스, 피팅 결과, 그림 등을 ROOT 파일에 저장
                    fitter.saveResults(fitResult);

                    // performFit()에서 동적으로 할당된 RooFitResult 객체의 메모리 해제
                    delete fitResult;
                    
                    std::cout << "\nAnalysis finished successfully!" << std::endl;
                } else {
                    std::cerr << "Fit failed to produce a result." << std::endl;
                }
            } else {
                std::cerr << "Failed to build the model." << std::endl;
            }
        } else {
            std::cerr << "Failed to load data." << std::endl;
        }
    } else {
        std::cerr << "Failed to create templates from MC." << std::endl;
    }
}