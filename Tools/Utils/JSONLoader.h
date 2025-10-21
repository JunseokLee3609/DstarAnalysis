#ifndef JSON_LOADER_H
#define JSON_LOADER_H

#include "TBufferJSON.h"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace JsonLoader {

inline std::string ReadFile(const std::string &path)
{
   std::ifstream in(path);
   if (!in.is_open()) {
      throw std::runtime_error("JsonLoader::ReadFile - cannot open file: " + path);
   }
   std::ostringstream ss;
   ss << in.rdbuf();
   return ss.str();
}

inline std::string ExtractObject(const std::string &json, const std::string &key)
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

inline std::string ExtractNestedObject(const std::string &parentObjectJson, const std::string &key)
{
   const std::string quoted = "\"" + key + "\"";
   size_t searchStart = 0;

   while (true) {
      auto keyPos = parentObjectJson.find(quoted, searchStart);
      if (keyPos == std::string::npos)
         return {};

      if (keyPos > 0) {
         const size_t prev = parentObjectJson.find_last_not_of(" \t\n\r", keyPos - 1);
         if (prev != std::string::npos && parentObjectJson[prev] != '{' && parentObjectJson[prev] != ',') {
            searchStart = keyPos + quoted.size();
            continue;
         }
      }

      auto colonPos = parentObjectJson.find(':', keyPos + quoted.size());
      if (colonPos == std::string::npos)
         return {};

      auto startPos = parentObjectJson.find_first_not_of(" \t\n\r", colonPos + 1);
      if (startPos == std::string::npos)
         return {};

      if (parentObjectJson[startPos] != '{') {
         searchStart = keyPos + quoted.size();
         continue;
      }

      int depth = 0;
      bool inString = false;
      bool escape = false;
      size_t pos = startPos;
      for (; pos < parentObjectJson.size(); ++pos) {
         const char c = parentObjectJson[pos];
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

      return parentObjectJson.substr(startPos, pos - startPos);
   }
}

inline std::map<std::string, std::string>
LoadObjectMap(const std::string &jsonPath, const std::string &parentKey, const std::string &objectKey)
{
   const std::string rootJson = ReadFile(jsonPath);
   const std::string parentObject = ExtractObject(rootJson, parentKey);
   if (parentObject.empty()) {
      throw std::runtime_error("JsonLoader::LoadObjectMap - cannot find parent object: " + parentKey);
   }

   const std::string objectJson = ExtractNestedObject(parentObject, objectKey);
   if (objectJson.empty()) {
      throw std::runtime_error("JsonLoader::LoadObjectMap - cannot find entry: " + objectKey);
   }

   auto parsed = TBufferJSON::FromJSON<std::map<std::string, std::string>>(objectJson);
   if (!parsed) {
      throw std::runtime_error("JsonLoader::LoadObjectMap - failed to parse object for key: " + objectKey);
   }

   return *parsed;
}

} // namespace JsonLoader

#endif
