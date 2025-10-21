#!/bin/bash

if [ -z "$ROOTSYS" ]; then
    source /software/ROOT/ROOT-v6.24/root-6.24-install/bin/thisroot.sh
fi
root -l -b -q DCAResolutionScan.cpp\(5,7,0,0.2\)
wait
root -l -b -q DCAResolutionScan.cpp\(5,7,0.2,0.4\)
wait
root -l -b -q DCAResolutionScan.cpp\(5,7,0.4,0.6\)
wait
root -l -b -q DCAResolutionScan.cpp\(5,7,0.6,0.8\)
wait
root -l -b -q DCAResolutionScan.cpp\(5,7,0.8,1.0\)
wait
root -l -b -q DCAResolutionScan.cpp\(7,10,0,0.2\)
wait
root -l -b -q DCAResolutionScan.cpp\(7,10,0.2,0.4\)
wait
root -l -b -q DCAResolutionScan.cpp\(7,10,0.4,0.6\)
wait
root -l -b -q DCAResolutionScan.cpp\(7,10,0.6,0.8\)
wait
root -l -b -q DCAResolutionScan.cpp\(7,10,0.8,1.0\)
wait
root -l -b -q DCAResolutionScan.cpp\(10,20,0,0.2\)
wait
root -l -b -q DCAResolutionScan.cpp\(10,20,0.2,0.4\)
wait      
root -l -b -q DCAResolutionScan.cpp\(10,20,0.4,0.6\)
wait
root -l -b -q DCAResolutionScan.cpp\(10,20,0.6,0.8\)
wait
root -l -b -q DCAResolutionScan.cpp\(10,20,0.8,1.0\)
wait  
root -l -b -q DCAResolutionScan.cpp\(20,50,0,0.2\)
wait
root -l -b -q DCAResolutionScan.cpp\(20,50,0.2,0.4\)
wait
root -l -b -q DCAResolutionScan.cpp\(20,50,0.4,0.6\)
wait
root -l -b -q DCAResolutionScan.cpp\(20,50,0.6,0.8\)
wait
root -l -b -q DCAResolutionScan.cpp\(20,50,0.8,1.0\)
wait
echo "All done!"    