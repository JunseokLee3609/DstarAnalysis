#!/bin/bash

if [ -z "$ROOTSYS" ]; then
    source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
fi
root -l -b -q DCAResolutionCosScanStandalone.cpp\(5.,7.,0,100,0.70,1.40,0.03,true\);
wait
root -l -b -q DCAResolutionCosScanStandalone.cpp\(7.,10.,0,100,0.70,1.40,0.03,true\);
wait
root -l -b -q DCAResolutionCosScanStandalone.cpp\(10.,20.,0,100,0.70,1.40,0.03,true\);
wait
root -l -b -q DCAResolutionCosScanStandalone.cpp\(20.,50.,0,100,0.70,1.40,0.03,true\);
wait

