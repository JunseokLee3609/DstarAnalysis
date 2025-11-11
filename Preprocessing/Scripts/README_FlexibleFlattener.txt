========================================
FlexibleFlattener.cpp Execution Guide
========================================

Overview:
---------
The run_flexible_flattener.sh script provides a convenient way to run FlexibleFlattener
with proper path handling for different collision systems (PbPb/pp) and data types (Data/MC/Mix).

Output Directory Structure:
---------------------------
/home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/
├── PbPbData/
│   ├── D0/
│   └── DStar/
├── PbPbMC/
│   ├── D0/
│   └── DStar/
├── PbPbMix/
├── ppData/
│   ├── D0/
│   └── DStar/
├── ppMC/
│   ├── D0/
│   └── DStar/
└── ppMix/

(Uses absolute path: /home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/)

Usage:
------
./run_flexible_flattener.sh [type] [particle] [collision] [jobIdx] [inputfile] [suffix]

Parameters:
-----------
  type:       0 = Data, 1 = MC, 2 = Mix (required, default: 0)
  particle:   D0 or DStar (required, default: DStar)
  collision:  PbPb or pp (required, default: PbPb)
  jobIdx:     Job index for parallelization (optional, default: 0)
  inputfile:  Custom input file path (optional, default: use preset paths from code)
  suffix:     Output suffix for file naming (optional, default: empty)

Examples:
---------

1. Run PbPb Data (DStar) with defaults:
   $ ./run_flexible_flattener.sh 0 DStar PbPb 0

2. Run PbPb MC (DStar) with job index:
   $ ./run_flexible_flattener.sh 1 DStar PbPb 0

3. Run pp Data (D0):
   $ ./run_flexible_flattener.sh 0 D0 pp 0

4. Run pp MC (DStar):
   $ ./run_flexible_flattener.sh 1 DStar pp 0

5. Run PbPb Mix:
   $ ./run_flexible_flattener.sh 2 DStar PbPb 0

6. Run with custom input file:
   $ ./run_flexible_flattener.sh 1 DStar PbPb 0 "/path/to/custom/MC/*.root"

7. Run with custom input file and suffix:
   $ ./run_flexible_flattener.sh 0 DStar PbPb 0 "/path/to/data/*.root" "v2_mva0p9"

8. Run with suffix only (uses default input file):
   $ ./run_flexible_flattener.sh 1 DStar PbPb 0 "" "custom_tag"

9. Run with job index for parallel processing:
   $ ./run_flexible_flattener.sh 1 DStar PbPb 1
   $ ./run_flexible_flattener.sh 1 DStar PbPb 2
   $ ./run_flexible_flattener.sh 1 DStar PbPb 3

10. Run with all parameters:
    $ ./run_flexible_flattener.sh 1 DStar PbPb 5 "/custom/data/path/*.root" "v1_mva0p9_job5"

Default Behavior (minimal parameters):
--------------------------------------
./run_flexible_flattener.sh
  → Runs PbPb Data (DStar) with jobIdx=0, default input files, no suffix

./run_flexible_flattener.sh 1 DStar pp
  → Runs pp MC (DStar) with jobIdx=0, default input files, no suffix

Output Log Files:
-----------------
Job logs are saved to: /home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/{collision}{type}/{particle}/job_{jobIdx}.log

Example output paths:
  PbPb Data (DStar): /home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/PbPbData/DStar/job_0.log
  pp MC (D0):        /home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/ppMC/D0/job_0.log
  PbPb Mix:          /home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/PbPbMix/job_0.log

MVA Cut Applied:
----------------
✓ MVA threshold: 0.9 (applied via CandidateSelection.h)

Key Changes:
------------
1. MVA cut (0.9) moved to CandidateSelection.h (centralized)
2. FlexibleFlattener.cpp now accepts:
   - type (0=Data, 1=MC, 2=Mix)
   - particle_type ("D0" or "DStar")
   - collision_type ("PbPb" or "pp")
   - jobIdx
   - path (custom input file path)
   - suffix (output suffix)
3. Output paths automatically determined based on collision_type
4. Optional parameters for flexibility in data source and naming

Execution Notes:
----------------
- Can be run from any directory (script automatically finds DStarAnalysis directory)
- ROOT environment is automatically sourced
- Log files include both stdout and stderr
- Success/failure status reported at completion
- Parameters can be passed positionally or with defaults
- Input paths are glob-enabled (use *.root wildcards)
- Output paths use absolute paths: /home/jun502s/DstarAna/DStarAnalysis/Data/FlatSample/

Suffix Usage Examples:
----------------------
- Version tagging:     "v1", "v2_final", "20250101"
- MVA reference:       "mva0p9", "mva0p99_test"
- Run-specific info:   "job5", "run1_test"
- Dataset indicators:  "prompt", "nonprompt", "inclusive"

Combined examples:
  $ ./run_flexible_flattener.sh 1 DStar pp 3 "" "v2_final"
    → pp MC, job 3, default inputs, suffix="v2_final"

  $ ./run_flexible_flattener.sh 0 D0 PbPb 0 "/custom/pbpb/data/*.root" "test_run"
    → PbPb Data, job 0, custom input path, suffix="test_run"
