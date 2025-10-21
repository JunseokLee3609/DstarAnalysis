# Plot Output Pipeline Guidelines

- 모든 플롯 파일명 생성은 `Common/Managers/PlotFileHelper.h`의 `BuildFilename()`을 통해 수행한다.
  - 기본 확장자는 `.pdf`; 다른 확장자가 필요한 경우 호출부에서 지정한다.
  - 디렉터리가 존재하지 않으면 `EnsureDirectory()`를 호출해 생성한다.
- `EnhancedPlotManager`는 PlotFileHelper를 이용해 디렉터리 생성 및 파일명 생성을 처리한다.
  - 기존 `GetOutputFileName()`은 내부적으로 PlotFileHelper를 호출하도록 단순화되었다.
- 새로운 플롯 또는 DCA 관련 모듈에서 파일을 저장할 때는 PlotFileHelper를 재사용해 경로 규칙을 통일한다.
- 문서/스크립트에서 플롯 경로를 안내할 때는 `results/<subdir>/...` 구조(PlotFileHelper가 생성)와 일치하도록 한다.
