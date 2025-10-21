# D* Analysis Framework - Core Components

이 문서는 `/home/jun502s/DstarAna/DStarAnalysis/Analysis/Common/Core` 디렉토리에 있는 핵심 컴포넌트들을 설명합니다.

## 📁 **디렉토리 구조**

```
Core/
├── DataLoader.h          # 데이터 로딩 관리
├── DCAFitter.h           # DCA (Distance of Closest Approach) 피팅
├── MassFitterV2.h        # 핵심 질량 피팅 엔진
├── PDFFactory.h          # PDF 생성 팩토리
└── PDFFactoryImpl.h      # PDF 팩토리 구현체
```

---

## 🔧 **1. DataLoader.h**

### **목적**
ROOT 파일에서 데이터를 로딩하고 관리하는 클래스

### **핵심 기능**
- ROOT 파일 열기 및 관리
- TTree 및 RooDataSet 로딩
- Workspace에서 데이터셋 추출
- 메모리 안전한 데이터 관리

### **주요 메서드**

#### **생성자/소멸자**
```cpp
DataLoader();                                    // 기본 생성자
explicit DataLoader(const std::string& filename); // 파일명으로 생성
~DataLoader();                                   // 소멸자
```

#### **데이터 로딩**
```cpp
bool loadFile(const std::string& filename);                    // ROOT 파일 로딩
bool loadTree(const std::string& treename);                    // TTree 로딩
bool loadRooDataSet(const std::string& datasetname,            // RooDataSet 로딩
                   const std::string& workspacename = "");     // Workspace에서 로딩
```

#### **데이터 접근**
```cpp
TTree* getTree(const std::string& name = "") const;           // TTree 포인터 반환
RooDataSet* getDataSet(const std::string& name = "") const;   // RooDataSet 포인터 반환
```

#### **유틸리티**
```cpp
void clear();                    // 모든 데이터 정리
bool isLoaded() const;           // 로딩 상태 확인
void print() const;              // 현재 상태 출력
```

### **사용 예시**
```cpp
DataLoader loader("data.root");
if (loader.loadRooDataSet("datasetHX", "workspace")) {
    RooDataSet* data = loader.getDataSet();
    // 데이터 사용
}
```

---

## 🎯 **2. MassFitterV2.h**

### **목적**
D* 분석의 핵심 질량 피팅 엔진으로, 다양한 피팅 전략을 지원하는 모듈화된 클래스

### **핵심 특징**
- **Dependency Injection**: 외부 의존성 주입으로 테스트 가능
- **Template-based**: 타입 안전성과 코드 재사용성
- **Strategy Pattern**: 다양한 피팅 전략 지원
- **Smart Pointers**: 자동 메모리 관리

### **주요 메서드**

#### **생성자**
```cpp
// Dependency Injection 생성자
MassFitterV2(const std::string& name, 
             const std::string& massVar,
             double massMin, double massMax,
             std::unique_ptr<PDFFactory> pdfFactory = nullptr,
             std::unique_ptr<FitStrategy> fitStrategy = nullptr,
             std::unique_ptr<ResultManager> resultManager = nullptr,
             YieldMode yieldMode = YieldMode::Fraction);

// 전통적 생성자
MassFitterV2(const std::string& name,
             const std::string& massVar, 
             double massMin, double massMax,
             double nsigRatio = 0.1,
             double nsigMinRatio = 0.0, double nsigMaxRatio = 1.0,
             double nbkgRatio = 0.0,
             double nbkgMinRatio = 0.0, double nbkgMaxRatio = 1.0,
             YieldMode yieldMode = YieldMode::Fraction);
```

#### **데이터 관리**
```cpp
void SetData(RooDataSet* dataset);                    // 데이터 설정
void ApplyCut(const std::string& cutExpr);            // Cut 적용
RooDataSet* GetData() const;                          // 데이터 반환
int GetDataSize() const;                              // 데이터 크기 반환
```

#### **PDF 설정**
```cpp
template<typename SignalParams>
void SetSignalPDF(const SignalParams& params, const std::string& name = "signal");

template<typename BackgroundParams>
void SetBackgroundPDF(const BackgroundParams& params, const std::string& name = "background");
```

#### **피팅 실행**
```cpp
// 기본 피팅
bool Fit(const std::string& strategyName = "Robust", const std::string& resultName = "default");
bool FitWithConfig(const FitOpt& config, const std::string& resultName = "configured");

// 템플릿 기반 피팅
template<typename SignalParams, typename BackgroundParams>
bool PerformFit(const FitOpt& options, RooDataSet* dataset, 
               const SignalParams& signalParams, const BackgroundParams& backgroundParams,
               const std::string& resultName = "");

// MC 피팅
template<typename SignalParams>
bool PerformMCFit(const FitOpt& options, RooDataSet* mcDataset,
                 const SignalParams& signalParams,
                 const std::string& resultName = "");

// Constraint 피팅
template<typename SignalParams, typename BackgroundParams>
bool PerformGaussianConstraintFitWithMC(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                       const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                       const std::vector<std::string>& paramsToConstrain,
                                       const std::string& resultName = "");

// Sideband Background Constraint 피팅
template<typename SignalParams, typename BackgroundParams>
bool PerformSidebandPrefitBackgroundConstraintFit(const FitOpt& options, RooDataSet* dataset,
                                                  const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                  double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
                                                  double sigmaScale = 1.0,
                                                  const std::string& resultName = "");

// Combined Constraint 피팅
template<typename SignalParams, typename BackgroundParams>
bool PerformGaussianConstraintFitWithMCAndBkgSB(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                                const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                                const std::vector<std::string>& signalParamsToConstrain,
                                                double sbLoMin, double sbLoMax, double sbHiMin, double sbHiMax,
                                                double sigmaScaleSignal = 1.0, double sigmaScaleBkg = 2.0,
                                                const std::string& resultName = "");

// Fixed Parameter 피팅
template<typename SignalParams, typename BackgroundParams>
bool PerformFixedParameterFitWithMC(const FitOpt& options, RooDataSet* dataset, RooDataSet* mcDataset,
                                    const SignalParams& signalParams, const BackgroundParams& backgroundParams,
                                    const std::vector<std::string>& paramPrefixesToFix,
                                    const std::string& resultName = "");
```

#### **결과 접근**
```cpp
double GetSignalYield(const std::string& resultName = "default") const;
double GetSignalYieldError(const std::string& resultName = "default") const;
double GetBackgroundYield(const std::string& resultName = "default") const;
double GetBackgroundYieldError(const std::string& resultName = "default") const;
double GetTotalYield(const std::string& resultName = "default") const;
double GetChiSquare(const std::string& resultName = "default") const;
double GetReducedChiSquare(const std::string& resultName = "default") const;
double GetNDF(const std::string& resultName = "default") const;
bool IsGoodFit(const std::string& resultName = "default") const;
```

#### **고급 결과 접근**
```cpp
FitResults* GetFitResults(const std::string& resultName = "default");
RooFitResult* GetRooFitResult(const std::string& resultName = "default");
RooWorkspace* GetWorkspace(const std::string& resultName = "default");
std::vector<std::string> GetResultNames() const;
```

#### **시각화**
```cpp
std::unique_ptr<RooPlot> CreatePlot(const std::string& resultName = "default",
                                   const PlotOptions& options = PlotOptions{});
std::unique_ptr<TCanvas> CreateCanvas(const std::string& resultName = "default",
                                     const PlotOptions& options = PlotOptions{});
```

#### **파일 I/O**
```cpp
void SaveResults(const std::string& filePath, const std::string& fileName,
                bool saveWorkspaces = true);
void SaveResult(const std::string& resultName, const std::string& filePath, 
               const std::string& fileName, bool saveWorkspace = true);
void LoadResults(const std::string& filePath, const std::string& fileName);
```

#### **분석 및 비교**
```cpp
void CompareResults(const std::vector<std::string>& resultNames, const std::string& outputFile = "");
void PrintSummary(const std::string& resultName = "") const;
void ExportResults(const std::string& format = "json", const std::string& fileName = "");
```

#### **전략 관리**
```cpp
void SetFitStrategy(std::unique_ptr<FitStrategy> strategy);
FitStrategy* GetFitStrategy() const;
std::vector<std::string> GetAvailableStrategies() const;
```

#### **팩토리 관리**
```cpp
PDFFactory* GetPDFFactory() const;
void SetPDFFactory(std::unique_ptr<PDFFactory> factory);
ResultManager* GetResultManager() const;
void SetResultManager(std::unique_ptr<ResultManager> manager);
```

#### **검증 및 진단**
```cpp
bool ValidateSetup() const;
std::vector<std::string> GetValidationErrors() const;
void RunDiagnostics(const std::string& outputFile = "") const;
```

#### **유틸리티**
```cpp
void Clear();
void Reset();
std::string GetInfo() const;
std::string GetName() const;
```

#### **Yield 모드 제어**
```cpp
void UseIndependentYields(bool enable = true);
void UseFractionCoefficientModel(bool enable = true);
void ConfigureYieldRatios(double nsigRatio, double nsigMinRatio, double nsigMaxRatio,
                          double nbkgRatio, double nbkgMinRatio, double nbkgMaxRatio);
```

#### **통계 유틸리티**
```cpp
double CalculateSignificance(const std::string& resultName = "default") const;
double CalculatePurity(const std::string& resultName = "default") const;
std::pair<double, double> GetSignalToBackgroundRatio(const std::string& resultName = "default") const;
```

### **Yield 모드**

#### **Fraction Mode (분수 모드)**
```cpp
// nsig = fsig * ntot, nbkg = (1-fsig) * ntot
std::unique_ptr<RooRealVar> fsig_;     // signal fraction [0,1]
std::unique_ptr<RooRealVar> ntot_;     // total expected yield
std::unique_ptr<RooAbsReal> nsig_;     // nsig = fsig * ntot
std::unique_ptr<RooAbsReal> nbkg_;     // nbkg = (1-fsig) * ntot
```

#### **Independent Mode (독립 모드)**
```cpp
// nsig, nbkg are independent
RooRealVar* nsigVar_ = nullptr;  // Direct signal yield
RooRealVar* nbkgVar_ = nullptr;  // Direct background yield
```

### **사용 예시**
```cpp
// 1. MassFitterV2 생성
MassFitterV2 fitter("DStar", "mass", 0.14, 0.16);

// 2. 데이터 설정
fitter.SetData(dataset);

// 3. PDF 설정
fitter.SetSignalPDF(signalParams);
fitter.SetBackgroundPDF(backgroundParams);

// 4. 피팅 실행
bool success = fitter.PerformFit(fitOpt, dataset, signalParams, backgroundParams);

// 5. 결과 접근
double signalYield = fitter.GetSignalYield();
double chi2 = fitter.GetChiSquare();
```

---

## 🏭 **3. PDFFactory.h**

### **목적**
다양한 PDF 타입을 생성하는 팩토리 클래스

### **핵심 기능**
- Signal PDF 생성 (Gaussian, Crystal Ball, Voigtian 등)
- Background PDF 생성 (Exponential, Chebychev, Phenomenological 등)
- 파라미터 관리 및 고정 설정
- 타입 안전한 PDF 생성

### **주요 메서드**

#### **Signal PDF 생성**
```cpp
std::unique_ptr<RooAbsPdf> CreateGaussian(const PDFParams::GaussianParams& params, 
                                         const std::string& name = "gauss");
std::unique_ptr<RooAbsPdf> CreateDoubleGaussian(const PDFParams::DoubleGaussianParams& params, 
                                               const std::string& name = "doublegauss");
std::unique_ptr<RooAbsPdf> CreateCrystalBall(const PDFParams::CrystalBallParams& params, 
                                            const std::string& name = "cb");
std::unique_ptr<RooAbsPdf> CreateDBCrystalBall(const PDFParams::DBCrystalBallParams& params, 
                                              const std::string& name = "dbcb");
std::unique_ptr<RooAbsPdf> CreateDoubleDBCrystalBall(const PDFParams::DoubleDBCrystalBallParams& params, 
                                                    const std::string& name = "ddbcb");
std::unique_ptr<RooAbsPdf> CreateVoigtian(const PDFParams::VoigtianParams& params, 
                                         const std::string& name = "voigt");
```

#### **Background PDF 생성**
```cpp
std::unique_ptr<RooAbsPdf> CreateExponential(const PDFParams::ExponentialBkgParams& params, 
                                            const std::string& name = "exp");
std::unique_ptr<RooAbsPdf> CreateChebychev(const PDFParams::ChebychevBkgParams& params, 
                                          const std::string& name = "cheb");
std::unique_ptr<RooAbsPdf> CreatePolynomial(const PDFParams::PolynomialBkgParams& params, 
                                           const std::string& name = "poly");
std::unique_ptr<RooAbsPdf> CreatePhenomenological(const PDFParams::PhenomenologicalParams& params, 
                                                  const std::string& name = "phenom");
std::unique_ptr<RooAbsPdf> CreatePhenomenological2(const PDFParams::Phenomenological2Params& params, 
                                                   const std::string& name = "phenom2");
std::unique_ptr<RooAbsPdf> CreateDstD0Background(const PDFParams::DstD0Params& params, 
                                                 const std::string& name = "dstd0");
std::unique_ptr<RooAbsPdf> CreateExpErf(const PDFParams::ExpErfBkgParams& params, 
                                       const std::string& name = "experf");
std::unique_ptr<RooAbsPdf> CreateDstBg(const PDFParams::DstBkgParams& params, 
                                       const std::string& name = "dstbg");
std::unique_ptr<RooAbsPdf> CreateThresholdFunction(const PDFParams::ThresholdFuncParams& params, 
                                                   const std::string& name = "threshold");
```

#### **파라미터 관리**
```cpp
const std::vector<std::unique_ptr<RooRealVar>>& GetParameters() const;  // 파라미터 목록 반환
void ClearParameters();                                                  // 파라미터 정리
void ApplyFixedSettings(const std::map<std::string, bool>& fixedFlags,  // 파라미터 고정 설정
                       const std::string& pdfName = "");
```

### **사용 예시**
```cpp
PDFFactory factory(massVar);

// Signal PDF 생성
auto signalPDF = factory.CreateDBCrystalBall(signalParams);

// Background PDF 생성
auto bkgPDF = factory.CreatePhenomenological2(bkgParams);

// 파라미터 고정 설정
std::map<std::string, bool> fixedFlags = {{"mean", true}, {"sigma", false}};
factory.ApplyFixedSettings(fixedFlags, "signal");
```

---

## 🔧 **4. PDFFactoryImpl.h**

### **목적**
PDFFactory를 IPDFFactory 인터페이스로 래핑하는 어댑터 클래스

### **핵심 기능**
- 의존성 주입 프레임워크와의 호환성
- 타입 안전한 PDF 생성
- 인터페이스 기반 설계

### **주요 메서드**

#### **인터페이스 구현**
```cpp
std::unique_ptr<RooAbsPdf> CreateSignalPDF(const std::string& type, const void* params) override;
std::unique_ptr<RooAbsPdf> CreateBackgroundPDF(const std::string& type, const void* params) override;
const std::vector<std::unique_ptr<RooRealVar>>& GetParameters() const override;
void ClearParameters() override;
```

#### **팩토리 함수**
```cpp
std::unique_ptr<IPDFFactory> CreatePDFFactory(RooRealVar* massVar);
```

### **지원하는 PDF 타입**

#### **Signal PDFs**
- `"Gaussian"`
- `"DoubleGaussian"`
- `"CrystalBall"`
- `"DBCrystalBall"`
- `"DoubleDBCrystalBall"`
- `"Voigtian"`

#### **Background PDFs**
- `"Exponential"`
- `"DstD0"`
- `"Chebychev"`
- `"Polynomial"`
- `"Phenomenological"`
- `"ExpErf"`
- `"DstBg"`
- `"ThresholdFunction"`

### **사용 예시**
```cpp
auto pdfFactory = CreatePDFFactory(massVar);
auto signalPDF = pdfFactory->CreateSignalPDF("DBCrystalBall", &signalParams);
auto bkgPDF = pdfFactory->CreateBackgroundPDF("Phenomenological2", &bkgParams);
```

---

## 🎯 **5. DCAFitter.h**

### **목적**
DCA (Distance of Closest Approach) 분석을 위한 전용 피팅 클래스

### **핵심 기능**
- DCA 분포 피팅
- Prompt/Non-prompt 분리
- MC 템플릿 기반 피팅
- 향상된 에러 처리 및 로깅

### **주요 메서드**

#### **생성자**
```cpp
DCAFitter(const FitOpt& opt, 
          const std::string& name, 
          const std::string& massVarName, 
          double dcaMin, double dcaMax, 
          int nBins,
          std::unique_ptr<ConfigManager> configManager = nullptr,
          std::unique_ptr<class DCAErrorHandler> errorHandler = nullptr);
```

#### **핵심 피팅 메서드**
```cpp
bool FitDCA(RooDataSet* data, const std::string& outputDir = "");
bool FitDCATemplate(RooDataSet* data, RooDataSet* promptMC, RooDataSet* nonPromptMC, 
                   const std::string& outputDir = "");
```

#### **유틸리티**
```cpp
std::string LogContext() const;
template<typename... Args>
std::string BuildLogMessage(Args&&... args) const;
template<typename... Args>
void LogInfoMsg(Args&&... args) const;
template<typename... Args>
void LogWarningMsg(Args&&... args) const;
template<typename... Args>
void LogErrorMsg(Args&&... args) const;
```

### **사용 예시**
```cpp
DCAFitter dcaFitter(fitOpt, "DCA_Analysis", "mass", 0.0, 0.07, 50);
bool success = dcaFitter.FitDCA(data, "output/");
```

---

## 🔄 **데이터 흐름**

```
📁 DataLoader → 📊 MassFitterV2 → 🎯 PDFFactory → 📈 ResultManager
     ↓              ↓                ↓                ↓
  ROOT 파일      피팅 실행        PDF 생성        결과 저장
  데이터셋       전략 선택        파라미터 관리    시각화
```

## 🎯 **핵심 설계 원칙**

### **1. 모듈화**
- 각 클래스는 단일 책임 원칙을 따름
- 의존성 주입을 통한 느슨한 결합
- 인터페이스 기반 설계

### **2. 타입 안전성**
- 템플릿을 통한 컴파일 타임 타입 검사
- Smart pointer를 통한 메모리 안전성
- 예외 안전성 보장

### **3. 확장성**
- 새로운 PDF 타입 추가 용이
- 새로운 피팅 전략 추가 가능
- 플러그인 아키텍처 지원

### **4. 테스트 가능성**
- 의존성 주입을 통한 Mock 객체 사용 가능
- 단위 테스트 지원
- 통합 테스트 프레임워크

## 📚 **사용 가이드**

### **기본 사용법**
1. `DataLoader`로 데이터 로딩
2. `PDFFactory`로 PDF 생성
3. `MassFitterV2`로 피팅 실행
4. 결과 분석 및 시각화

### **고급 사용법**
1. 커스텀 피팅 전략 구현
2. 새로운 PDF 타입 추가
3. 의존성 주입을 통한 테스트
4. 배치 처리 및 자동화

이 Core 컴포넌트들은 D* 분석 프레임워크의 핵심을 이루며, 안전하고 확장 가능한 분석 환경을 제공합니다.
