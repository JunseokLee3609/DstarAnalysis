#!/bin/bash

source /software/ROOT/ROOT-v6.24.06/root-6.24.06-install/bin/thisroot.sh
cd /home/vince402/Analysis/DDbarpPb/bin
source setup.sh
cd ../macros/fit
# "DCandBasicMVATighter DCandDCATighter" "DCandBasicMVATighter DCandDCATight"  "DCandBasicMVATighter DCandDCALoose" "DCandBasicMVATighter DCandDCANone"

# for I in   "DCandBasicMVATight DCandDCATighter" "DCandBasicMVATight DCandDCATight" "DCandBasicMVATight DCandDCALoose" "DCandBasicMVATight DCandDCANone" "DCandBasicMVANominal DCandDCATighter"  "DCandBasicMVANominal DCandDCATight" "DCandBasicMVANominal DCandDCALoose" "DCandBasicMVANominal DCandDCANone" "DCandBasicMVALoose DCandDCATighter" "DCandBasicMVALoose DCandDCATight" "DCandBasicMVALoose DCandDCALoose" "DCandBasicMVALoose DCandDCANone" "DCandBasicMVALooser DCandDCATighter" "DCandBasicMVALooser DCandDCATight" "DCandBasicMVALooser DCandDCALoose" "DCandBasicMVALooser DCandDCANone"
# for I in "DCandBasicMVATight DCandDCANone" "DCandBasicMVANominal DCandDCATighter"  "DCandBasicMVANominal DCandDCATight" "DCandBasicMVANominal DCandDCALoose" "DCandBasicMVANominal DCandDCANone" 
# for I in "DCandBasicMVALoose DCandDCATight" "DCandBasicMVANominal DCandDCATight" "DCandBasicMVATight DCandDCATight" "DCandBasicMVALoose DCandDCANone" "DCandBasicMVALoose DCandDCALoose" "DCandBasicMVALoose DCandDCATighter"
# for I in "DCandBasicMVALooser DCandDCATighter" "DCandBasicMVALooser DCandDCATight" "DCandBasicMVALooser DCandDCALoose" "DCandBasicMVALooser DCandDCANone"
# for I in "DCandBasicMVALooser DCandDCANone"
# for I in "DCandBasicMVALooser DCandDCALoose"
# for I in "DCandBasicMVALooser DCandDCATighter"
# for I in "DCandBasicMVALoose DCandDCANone" "DCandBasicMVANominal DCandDCANone" "DCandBasicMVATight DCandDCANone" "DCandBasicMVATighter DCandDCANone"  

#  for I in "DCandBasicMVATight DCandDCATight" 
# for I in "DCandBasicMVATight DCandDCATight"  
# for I in "DCandBasicMVALoose DCandDCATight" 
# for I in "DCandBasicMVANominal DCandDCALoose" 
# for I in "DCandBasicMVALoose DCandDCANone" "DCandBasicMVALoose DCandDCATight" 
# for I in "DCandBasicMVANominal DCandDCATight" "DCandBasicMVALoose DCandDCATight" "DCandBasicMVATight DCandDCATight" "DCandBasicMVANominal DCandDCATighter" "DCandBasicMVANominal DCandDCALoose" "DCandBasicMVANominal DCandDCANone" "DCandBasicMVALoose DCandDCANone" ########## ALL  ############
# for I in "DCandBasicMVANominal DCandDCATight" "DCandBasicMVALoose DCandDCATight" "DCandBasicMVANominal DCandDCANone" "DCandBasicMVALoose DCandDCANone" ########## Closure ALL  ############
# for I in "DCandBasicMVANominal DCandDCATight" "DCandBasicMVALoose DCandDCATight" 
# for I in "DCandBasicMVANominal DCandDCATighter" "DCandBasicMVANominal DCandDCALoose" 
# for I in "DCandBasicMVATight DCandDCATight" "DCandBasicMVANominal DCandDCANone" ########## ALL  ############
# for I in "DCandBasicMVALoose DCandDCANone"
for I in  "DCandBasicMVANone DCandDCANone" "DCandBasicMVANone DCandDCATight" "DCandBasicMVANone DCandDCALoose" "DCandBasicMVANone DCandDCATighter"  
# for I in "DCandBasicMVALoose DCandDCANone" 

# for I in "DCandBasicMVANone DCandDCANone" 
# for I in "DCandBasicMVANone DCandDCATight" 
# for I in "DCandBasicMVANone DCandDCALoose" 
# for I in "DCandBasicMVANone DCandDCATighter"  

# for I in "DCandBasicMVATight DCandDCATight" 
# for I in "DCandBasicMVALoose DCandDCATight" 
# for I in "DCandBasicMVANominal DCandDCATight" 
# for I in "DCandBasicMVANominal DCandDCANone" 

# for I in "DCandBasicMVALooser DCandDCATight" 
# for I in "DCandBasicMVALoose DCandDCANone" 
# for I in "DCandBasicMVANominal DCandDCANone" 
# for I in "DCandBasicMVANominal DCandDCALoose" 
# for I in "DCandBasicMVALooser DCandDCATight" 
do
    SE=( $I )
    SEMVA="${SE[0]}"
    SEDCA="${SE[1]}"
    echo $SEMVA

# MC
# for PT in {0..2}
# do
#     for idx in {191..192}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_A"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_A"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done

# # MIX
# for PT in {0..0}
# do
#     # for idx in {214..275}
#     for idx in {238..244}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_B"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_B"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {238..244}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_B"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_B"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done

# # MIX
# for PT in {0..0}
# do
#     for idx in {214..275}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_C"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_C"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done

# DTYPE=dDATAMBDD
# DTYPE=dDATAMBMIX
#   DTYPE=dDATAMB
#   DTYPE=dDATAHMDD
DTYPE=dMCPRDD
# DTYPE=dMCPRDDAUG

########### FOR VERSION 2 #####################

###############################################
################# No Cand is ON ###############
###############################################
# for PT in {0..0}
# do
#      for idx in {177..182}
#      do
#          sleep 1
#          ts=`date +%s`;
#          fname=TMP_FIT_A"$ts$SE".C;
#          cp orderFit1D_MVA_FORBASH2.C  $fname;
#          sed -i "121s/orderFit1D_MVA/TMP_FIT_A"$ts$SE"/" $fname
#          PTT=`expr $PT + 1`
#          sed -i "131s/XX/"$SEMVA"/" $fname
#          sed -i "132s/XX/"$SEDCA"/" $fname
#          sed -i "134s/XX/"$PT"/" $fname
#          sed -i "134s/YY/"$PTT"/" $fname
#          sed -i ""$idx"s/\/\///" $fname
#          sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         #  sed -i ""$idx"s/Simultaneous/DCandNoSign/" $fname
#          root -l -b -q $fname
#          rm -f $fname
#      done
#      for idx in {185..191}
#      do
#          sleep 1
#          ts=`date +%s`;
#          fname=TMP_FIT_B"$ts$SE".C;
#          cp orderFit1D_MVA_FORBASH2.C  $fname;
#          sed -i "121s/orderFit1D_MVA/TMP_FIT_B"$ts$SE"/" $fname
#          PTT=`expr $PT + 1`
#          sed -i "131s/XX/"$SEMVA"/" $fname
#          sed -i "132s/XX/"$SEDCA"/" $fname
#          sed -i "134s/XX/"$PT"/" $fname
#          sed -i "134s/YY/"$PTT"/" $fname
#          sed -i ""$idx"s/\/\///" $fname
#          sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         #  sed -i ""$idx"s/Simultaneous/DCandNoSign/" $fname
#          root -l -b -q $fname
#          rm -f $fname
#      done
#      for idx in {195..196}
#      do
#          sleep 1
#          ts=`date +%s`;
#          fname=TMP_FIT_C"$ts$SE".C;
#          cp orderFit1D_MVA_FORBASH2.C  $fname;
#          sed -i "121s/orderFit1D_MVA/TMP_FIT_C"$ts$SE"/" $fname
#          PTT=`expr $PT + 1`
#          sed -i "131s/XX/"$SEMVA"/" $fname
#          sed -i "132s/XX/"$SEDCA"/" $fname
#          sed -i "134s/XX/"$PT"/" $fname
#          sed -i "134s/YY/"$PTT"/" $fname
#          sed -i ""$idx"s/\/\///" $fname
#          sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         #  sed -i ""$idx"s/Simultaneous/DCandNoSign/" $fname
#          root -l -b -q $fname
#          rm -f $fname
#      done
#      for idx in {208..212}
#      do
#          sleep 1
#          ts=`date +%s`;
#          fname=TMP_FIT_D"$ts$SE".C;
#          cp orderFit1D_MVA_FORBASH2.C  $fname;
#          sed -i "121s/orderFit1D_MVA/TMP_FIT_D"$ts$SE"/" $fname
#          PTT=`expr $PT + 1`
#          sed -i "131s/XX/"$SEMVA"/" $fname
#          sed -i "132s/XX/"$SEDCA"/" $fname
#          sed -i "134s/XX/"$PT"/" $fname
#          sed -i "134s/YY/"$PTT"/" $fname
#          sed -i ""$idx"s/\/\///" $fname
#          sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         #  sed -i ""$idx"s/Simultaneous/DCandNoSign/" $fname
#          root -l -b -q $fname
#          rm -f $fname
#      done
#      for idx in {252..266}
#      do
#          sleep 1
#          ts=`date +%s`;
#          fname=TMP_FIT_E"$ts$SE".C;
#          cp orderFit1D_MVA_FORBASH2.C  $fname;
#          sed -i "121s/orderFit1D_MVA/TMP_FIT_E"$ts$SE"/" $fname
#          PTT=`expr $PT + 1`
#          sed -i "131s/XX/"$SEMVA"/" $fname
#          sed -i "132s/XX/"$SEDCA"/" $fname
#          sed -i "134s/XX/"$PT"/" $fname
#          sed -i "134s/YY/"$PTT"/" $fname
#          sed -i ""$idx"s/\/\///" $fname
#          sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         #  sed -i ""$idx"s/Simultaneous/DCandNoSign/" $fname
#          root -l -b -q $fname
#          rm -f $fname
#      done
#      echo "DD pT"    ########### DD pT Bin #############                 
#      for idx in {269..276}
#      do
#          sleep 1
#          ts=`date +%s`;
#          fname=TMP_FIT_E"$ts$SE".C;
#          cp orderFit1D_MVA_FORBASH2.C  $fname;
#          sed -i "121s/orderFit1D_MVA/TMP_FIT_E"$ts$SE"/" $fname
#          PTT=`expr $PT + 1`
#          sed -i "131s/XX/"$SEMVA"/" $fname
#          sed -i "132s/XX/"$SEDCA"/" $fname
#          sed -i "134s/XX/"$PT"/" $fname
#          sed -i "134s/YY/"$PTT"/" $fname
#          sed -i ""$idx"s/\/\///" $fname
#          sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         #  sed -i ""$idx"s/Simultaneous/DCandNoSign/" $fname
#          root -l -b -q $fname
#          rm -f $fname
#      done
# done
########### END FOR VERSION 2 #####################

# for PT in {0..0}
# do
#     # for idx in {303..438}
#     for idx in {336..337}
#     # for idx in {336..337}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_D"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_D"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {367..371}
#     # for idx in {336..337}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_D"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_D"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {318..324}
#     # for idx in {336..337}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_D"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_D"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done
# for PT in {0..2}
# do
#     # for idx in {303..438}
#     for idx in {422..424}
#     # for idx in {336..337}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_D"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_D"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done

# for PT in {0..0}
# do
#     # for idx in {303..438}
#     for idx in {359..377}
#     # for idx in {336..337}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_D"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_D"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done

# for PT in {1..1}
# do
#     # for idx in {303..438}
#     for idx in {359..377}
#     # for idx in {336..337}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_E"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_E"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#         sed -i "159s/XX/"$SEMVA"/" $fname
#         sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done #  DONE MIX PT

# MIN BIAS
# for PT in {0..2}
# do
#     ############ for idx in {303..438}
#     for idx in {282..284}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {303..315}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {318..332}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {336..337}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {347..348}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {359..377}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done

#     for idx in {385..391}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
#     for idx in {405..438}
#     do
#         sleep 1
#         ts=`date +%s`;
#         fname=TMP_FIT_F"$ts$SE".C;
#         cp orderFit1D_MVA_FORBASH.C  $fname;
#         sed -i "149s/orderFit1D_MVA/TMP_FIT_F"$ts$SE"/" $fname
#         PTT=`expr $PT + 1`
#           sed -i "159s/XX/"$SEMVA"/" $fname
#           sed -i "160s/XX/"$SEDCA"/" $fname
#         sed -i "188s/XX/"$PT"/" $fname
#         sed -i "188s/YY/"$PTT"/" $fname
#         sed -i ""$idx"s/\/\///" $fname
#         sed -i ""$idx"s/dDATAMB/"$DTYPE"/" $fname
#         root -l -b -q $fname
#         rm -f $fname
#     done
# done # DONE PT LOOP
done 
# DONE QUALITY
