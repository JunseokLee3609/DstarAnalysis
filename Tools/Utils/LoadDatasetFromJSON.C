#include "TBufferJSON.h"
#include "TError.h"
#include "TFile.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace {

std::string ExtractObject(const std::string &json, const std::string &key)
{
   const std::string quoted = "\"" + key + "\"";
   auto keyPos = json.find(quoted);
   if (keyPos == std::string::npos)
      return {};

   auto colonPos = json.find(':', keyPos + quoted.size());
   if (colonPos == std::string::npos)
      return {};

   auto startPos = json.find_first_not_of(" \t\n\r", colonPos + 1);
   if (startPos == std::string::npos || json[startPos] != '{')
      return {};

   int depth = 0;
   bool inString = false;
   bool escape = false;
   size_t pos = startPos;
   for (; pos < json.size(); ++pos) {
      const char c = json[pos];
      if (escape) {
         escape = false;
         continue;
      }
      if (c == '\\') {
         escape = true;
         continue;
      }
      if (c == '"') {
         inString = !inString;
         continue;
      }
      if (inString)
         continue;
      if (c == '{')
         ++depth;
      else if (c == '}') {
         --depth;
         if (depth == 0) {
            ++pos;
            break;
         }
      }
   }

   if (depth != 0)
      return {};

   return json.substr(startPos, pos - startPos);
}

std::string ExtractObjectInObject(const std::string &objectJson, const std::string &key)
{
   const std::string quoted = "\"" + key + "\"";
   size_t searchStart = 0;

   while (true) {
      auto keyPos = objectJson.find(quoted, searchStart);
      if (keyPos == std::string::npos)
         return {};

      if (keyPos > 0) {
         const size_t prev = objectJson.find_last_not_of(" \t\n\r", keyPos - 1);
         if (prev != std::string::npos && objectJson[prev] != '{' && objectJson[prev] != ',') {
            searchStart = keyPos + quoted.size();
            continue;
         }
      }

      auto colonPos = objectJson.find(':', keyPos + quoted.size());
      if (colonPos == std::string::npos)
         return {};

      auto startPos = objectJson.find_first_not_of(" \t\n\r", colonPos + 1);
      if (startPos == std::string::npos)
         return {};

      if (objectJson[startPos] != '{') {
         searchStart = keyPos + quoted.size();
         continue;
      }

      int depth = 0;
      bool inString = false;
      bool escape = false;
      size_t pos = startPos;
      for (; pos < objectJson.size(); ++pos) {
         const char c = objectJson[pos];
         if (escape) {
            escape = false;
            continue;
         }
         if (c == '\\') {
            escape = true;
            continue;
         }
         if (c == '"') {
            inString = !inString;
            continue;
         }
         if (inString)
            continue;
         if (c == '{')
            ++depth;
         else if (c == '}') {
            --depth;
            if (depth == 0) {
               ++pos;
               break;
            }
         }
      }

      if (depth != 0)
         return {};

      return objectJson.substr(startPos, pos - startPos);
   }
}

} // namespace

TFile *LoadDatasetFromJSON(const char *datasetKey = "PbPb_Data",
                           const char *jsonPath = "Fit/Common/Analysis/datasets.json",
                           const char *mode = "READ")
{
   std::ifstream in(jsonPath);
   if (!in.is_open()) {
      Error("LoadDatasetFromJSON", "Failed to open JSON catalog at %s", jsonPath);
      return nullptr;
   }

   std::ostringstream buffer;
   buffer << in.rdbuf();
   const std::string content = buffer.str();

   const std::string datasetsBlock = ExtractObject(content, "datasets");
   if (datasetsBlock.empty()) {
      Error("LoadDatasetFromJSON", "Could not find \"datasets\" object in %s", jsonPath);
      return nullptr;
   }

   const std::string datasetJson = ExtractObjectInObject(datasetsBlock, datasetKey);
   if (datasetJson.empty()) {
      Error("LoadDatasetFromJSON", "Dataset key \"%s\" not found in catalog %s", datasetKey, jsonPath);
      return nullptr;
   }

   auto datasetInfo = TBufferJSON::FromJSON<std::map<std::string, std::string>>(datasetJson);
   if (!datasetInfo) {
      Error("LoadDatasetFromJSON", "Failed to parse dataset entry \"%s\"", datasetKey);
      return nullptr;
   }

   const auto fileIt = datasetInfo->find("file");
   if (fileIt == datasetInfo->end()) {
      Error("LoadDatasetFromJSON", "Dataset \"%s\" is missing required \"file\" field", datasetKey);
      return nullptr;
   }

   TFile *f = TFile::Open(fileIt->second.c_str(), mode);
   if (!f || f->IsZombie()) {
      Error("LoadDatasetFromJSON", "Failed to open ROOT file %s", fileIt->second.c_str());
      delete f;
      return nullptr;
   }

   const auto descriptionIt = datasetInfo->find("description");
   if (descriptionIt != datasetInfo->end()) {
      Info("LoadDatasetFromJSON", "Loaded dataset %s (%s)", datasetKey, descriptionIt->second.c_str());
   } else {
      Info("LoadDatasetFromJSON", "Loaded dataset %s", datasetKey);
   }

   return f;
}
