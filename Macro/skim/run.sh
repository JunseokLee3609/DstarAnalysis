#!/bin/bash


# root -l -b -q 'FlatMixWithPromptNP.cpp(0, 100000, 0)' &
# root -l -b -q 'FlatMixWithPromptNP.cpp(100000, 200000, 1)' &
# root -l -b -q 'FlatMixWithPromptNP.cpp(200000, 300000, 2)'&
# root -l -b -q 'FlatMixWithPromptNP.cpp(300000, 400000, 3)'&

# root -l -b -q 'Flat.cpp(0, 4000000, 0)'&
# root -l -b -q 'Flat.cpp(4000000, 8000000, 1)' &
# root -l -b -q 'Flat.cpp(8000000, 12000000, 2)' &
# root -l -b -q 'Flat.cpp(12000000, 16000000, 3)' &
# root -l -b -q 'Flat.cpp(16000000, 30000000, 4)' &
# root -l -b -q 'Flat.cpp(30000000, 40000000, 5)' &
# root -l -b -q 'Flat.cpp(40000000, 50000000, 6)' &
# root -l -b -q 'Flat.cpp(50000000, 60000000, 7)' &
# root -l -b -q 'Flat.cpp(60000000, 70000000, 8)' &
# root -l -b -q 'Flat.cpp(70000000, 80000000, 9)' &
# root -l -b -q 'Flat.cpp(80000000, 90000000, 10)' &
# root -l -b -q 'Flat.cpp(90000000, 100000000, 11)' &

root -l -b -q 'Flat.cpp(100000000, 110000000, 12)' &
root -l -b -q 'Flat.cpp(110000000, 120000000, 13)' &
root -l -b -q 'Flat.cpp(120000000, 130000000, 14)' &
root -l -b -q 'Flat.cpp(130000000, 140000000, 15)' &

wait
#root -l -b -q 'FlatMixWithPromptNP.cpp(0, 300000, 0)' &
#root -l -b -q 'FlatMixWithPromptNP.cpp(300000, 600000, 1)' &
#root -l -b -q 'FlatMixWithPromptNP.cpp(600000, 900000, 2)'&
#wait
#root -l -b -q 'FlatMixWithPromptNP.cpp(900000, 1200000, 3)'&
#root -l -b -q 'FlatMixWithPromptNP.cpp(1200000, 1500000, 4)'&
#root -l -b -q 'FlatMixWithPromptNP.cpp(1500000, 1800000, 5)'&
#wait
#root -l -b -q 'FlatMixWithPromptNP.cpp(1800000, 2100000, 6)'&
#root -l -b -q 'FlatMixWithPromptNP.cpp(2100000, 2400000, 7)'&
#root -l -b -q 'FlatMixWithPromptNP.cpp(2400000, 2700000, 8)'&
#wait
#root -l -b -q 'FlatMixWithPromptNP.cpp(2700000, 2000000, 9)'&
#root -l -b -q 'FlatMixWithPromptNP.cpp(3000000, 3300000, 10)'&
#wait
