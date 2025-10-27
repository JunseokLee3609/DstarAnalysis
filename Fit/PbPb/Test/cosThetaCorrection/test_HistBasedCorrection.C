// ROOT 매크로: HistBasedCorrection 테스트 (Centrality 0-10%)
// 사용법: 
//   1) ROOT 대화형 모드:
//      root [0] .L HistBasedCorrection.cpp+
//      root [1] .L test_HistBasedCorrection.C
//      root [2] test_HistBasedCorrection()
//
//   2) 배치 모드:
//      root -l -b -q test_HistBasedCorrection.C

#include <iostream>
#include <string>

void test_HistBasedCorrection() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "HistBasedCorrection 테스트 시작" << std::endl;
    std::cout << "Centrality: 0-10%" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 파라미터 설정
    bool isMC = false;              // Data 분석
    bool useAbsTheta = true;        // |cos theta| 사용
    int centralityMin = 0;          // Centrality 최소값 0%
    int centralityMax = 10;         // Centrality 최대값 10%
    std::string mcDatasetKey = "PbPb_MC_Oct22";  // JSON catalog의 MC 데이터셋 키
    
    std::cout << "[PARAMS] 분석 설정:" << std::endl;
    std::cout << "  - isMC: " << (isMC ? "true" : "false") << std::endl;
    std::cout << "  - useAbsTheta: " << (useAbsTheta ? "true" : "false") << std::endl;
    std::cout << "  - Centrality: " << centralityMin << "-" << centralityMax << "%" << std::endl;
    std::cout << "  - MC Dataset: " << mcDatasetKey << std::endl;
    std::cout << std::endl;
    
    std::cout << "[START] HistBasedCorrectionWithCentrality 실행 중...\n" << std::endl;
    
    try {
        // HistBasedCorrectionWithCentrality 함수 호출
        HistBasedCorrectionWithCentrality(isMC, useAbsTheta, centralityMin, centralityMax, mcDatasetKey);
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "✓ 테스트 완료!" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        std::cout << "[OUTPUT] 결과 파일 위치:" << std::endl;
        std::cout << "  - splot/<subDir>_cent0to10/" << std::endl;
        std::cout << "  - efficiency_map_*.png" << std::endl;
        std::cout << "  - splot_*_corrected_yield_pt_*.png" << std::endl;
        std::cout << "  - rho00_vs_pT_prompt_nonprompt.png" << std::endl;
        std::cout << "  - yield_summary.csv" << std::endl;
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] 예외 발생: " << e.what() << std::endl;
        return;
    } catch (...) {
        std::cerr << "\n[ERROR] 알 수 없는 예외 발생!" << std::endl;
        return;
    }
}
