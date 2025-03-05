#include <TFile.h>
#include <TTree.h>
#include <ROOT/TThreadExecutor.hxx>
#include <vector>
#include <iostream>
#include <mutex>

std::mutex mtx;  // 동기화를 위한 뮤텍스

void mixAndSaveData(const std::string& dataFile, const std::string& mcFile, int startIndex, int endIndex, TFile* outputFile) {
    // ROOT 파일 열기
    TFile dataTFile(dataFile.c_str(), "READ");
    TFile mcTFile(mcFile.c_str(), "READ");
    
    // TTree 객체 가져오기
    TTree* dataTree = (TTree*)dataTFile.Get("Tree");
    TTree* mcTree = (TTree*)mcTFile.Get("Tree");

    // 믹싱된 데이터 저장할 새로운 TTree 생성
    TTree* mixedTree = new TTree("MixedTree", "Mixed Data");

    // 데이터와 MC의 변수를 정의 (예시)
    float dataVar, mcVar;
    dataTree->SetBranchAddress("dataVar", &dataVar);
    mcTree->SetBranchAddress("mcVar", &mcVar);

    // 데이터를 읽고 믹싱하여 새 TTree에 저장
    for (int i = startIndex; i < endIndex; ++i) {
        dataTree->GetEntry(i);
        mcTree->GetEntry(i);

        // 믹싱된 데이터 처리 (여기서는 간단히 두 값을 더하는 예시)
        float mixedValue = dataVar + mcVar;

        // 믹싱된 값을 새로운 TTree에 저장
        mixedTree->Branch("mixedVar", &mixedValue, "mixedVar/F");
        mixedTree->Fill();
    }

    // 동기화된 방식으로 결과를 한 TFile에 저장
    {
        std::lock_guard<std::mutex> lock(mtx);  // 스레드 간 동기화
        outputFile->cd();
        mixedTree->Write("", TObject::kOverwrite);  // 기존 파일에 덮어쓰기
    }

    // TTree와 파일 닫기
    delete mixedTree;
    dataTFile.Close();
    mcTFile.Close();
}

void runMultithreadedMixing(const std::string& dataFile, const std::string& mcFile, const std::string& outputFileName, int numThreads) {
    // 결과를 저장할 TFile 열기
    TFile* outputFile = new TFile(outputFileName.c_str(), "RECREATE");

    // 총 처리할 이벤트 개수 (예시로 1000개 이벤트 처리한다고 가정)
    int totalEvents = 1000;
    int eventsPerThread = totalEvents / numThreads;
    
    // 멀티스레딩을 위한 ThreadExecutor 사용
    ROOT::TThreadExecutor executor;
    
    // 각 스레드에 작업을 할당
    for (int i = 0; i < numThreads; ++i) {
        int startIndex = i * eventsPerThread;
        int endIndex = (i == numThreads - 1) ? totalEvents : (i + 1) * eventsPerThread;

        // 각 스레드에 작업 할당
        executor.AddTask([=]() { mixAndSaveData(dataFile, mcFile, startIndex, endIndex, outputFile); });
    }

    // 모든 스레드 작업이 완료될 때까지 기다림
    executor.Wait();

    // TFile 닫기
    outputFile->Close();
}

int main() {
    // 데이터 파일, MC 파일, 최종 결과 파일 경로 설정
    std::string dataFile = "data.root";
    std::string mcFile = "mc.root";
    std::string outputFileName = "mixed_output.root";

    // 사용할 스레드 수 설정
    int numThreads = 4;  // 예시로 4개의 스레드 사용

    // 멀티스레딩을 통한 데이터 믹싱 실행
    runMultithreadedMixing(dataFile, mcFile, outputFileName, numThreads);

    return 0;
}

