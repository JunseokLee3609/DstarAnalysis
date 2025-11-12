// Wrapper macro to run FlexibleFlattener with a file list (chunk) using TChain
// Usage in ROOT:
//   .x Preprocessing/Common/FlexibleFlattenerForCondor.cpp("file_list.txt", 0, "Prefix", 1, "DStar", "PbPb")
// Arguments:
//   fileList: path to a text file containing ROOT file paths (one per line)
//   isMC:     0=data, 1=MC
//   suffix:   output suffix (e.g., condor chunk label)
//   jobIdx:   job index for output naming
//   particle: "D0" or "DStar"
//   collision:"PbPb" or "pp"

#include <string>

// Reuse the full implementation; it supports file-list input for the 'path' argument
#include "FlexibleFlattener.cpp"

int FlexibleFlattenerForCondor(const char* fileList,
                               int isMC = 0,
                               const char* suffix = "",
                               int jobIdx = 0,
                               const char* particle = "DStar",
                               const char* collision = "PbPb") {
  const int type = isMC ? 1 : 0; // 0=Data, 1=MC
  // start=0, end=-1 to process all; pass fileList as 'path' so internals build a TChain from the list
  return FlexibleFlattener(type, particle, collision, jobIdx, 0, -1,
                           std::string(fileList), std::string(suffix));
}
