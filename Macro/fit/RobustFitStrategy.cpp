// Header-only conversion note:
// Implementations for SmartFitStrategyManager have been moved to
// RobustParameterManager.h so that ROOT macros (Cling) can load them without
// requiring compilation/linking of separate .cpp files.
// This file is kept as a lightweight stub to avoid duplicate symbol definitions
// when building outside ROOT. Simply include the header.

#include "RobustParameterManager.h"