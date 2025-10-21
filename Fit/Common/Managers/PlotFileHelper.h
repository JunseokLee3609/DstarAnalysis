#ifndef PLOT_FILE_HELPER_H
#define PLOT_FILE_HELPER_H

#include <string>
#include "../Utils/Helper.h"

namespace PlotFileHelper {

inline bool EnsureDirectory(const std::string& dir) {
    return ensureDir(dir);
}

inline std::string BuildFilename(const std::string& dir,
                                 const std::string& baseName,
                                 const std::string& suffix = std::string(),
                                 const std::string& defaultExt = ".pdf") {
    std::string output = dir;
    if (!output.empty() && output.back() != '/') {
        output += "/";
    }
    output += baseName;
    if (!suffix.empty()) {
        output += "_" + suffix;
    }
    const auto lastSlash = output.find_last_of('/');
    const auto lastDot = output.find_last_of('.');
    if (lastDot == std::string::npos || (lastSlash != std::string::npos && lastDot < lastSlash)) {
        output += defaultExt;
    }
    return output;
}

} // namespace PlotFileHelper

#endif // PLOT_FILE_HELPER_H
