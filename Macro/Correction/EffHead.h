// #include "src/dataformat/interface/simpleD.hxx"
// #include "src/dataformat/interface/simpleDMC.hxx"
#include "../interface/correctionmap.hxx"
// #include "src/dataformat/data/DmesonSelection.hxx"

static const int npt = corrParams::npt; 
static const int ny = corrParams::ny; 
static const int nphi = corrParams::nphi;
static const int ncos = corrParams::ncos;
static const int ncent = corrParams::ncent;
static const double pl = corrParams::pl; 
static const double ph = corrParams::ph;
static const double yl = corrParams::yl; 
static const double yh = corrParams::yh;
static const double phil = corrParams::phil; 
static const double phih = corrParams::phih;
static const double coslo = corrParams::cosl; 
static const double coshi = corrParams::cosh;

enum class MC{
    kPR = 1,
    kNP = 2
};