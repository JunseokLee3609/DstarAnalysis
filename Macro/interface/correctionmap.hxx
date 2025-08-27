#ifndef __CORRECTION_MAP__
#define __CORRECTION_MAP__

#include "TMath.h"

struct corrParams{
    public: 

        constexpr static int nptPF = 7 ;
        static std::vector<double> PFpvec(){ return std::vector<double>{ 2.0, 4.0, 6.0, 8.0, 10.0, 15.0, 20.0, 30.0 };} // 8
        constexpr static int nptBDT = 14 ;
        static std::vector<double> BDTpvec(){ return std::vector<double>{2.0, 2.4, 2.8, 3.2, 3.6, 4.0, 4.5, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 15.0, 30.0};} // 15
        constexpr static int npt = 12;
        constexpr static int ny = 4;
        constexpr static int nphi = 9;
        constexpr static int ncos = 20;
        constexpr static int ncent = 4;
        // constexpr static int ny = 2;
        // constexpr static int nphi = 2;
        constexpr static double pl = 4;
        constexpr static double ph = 50;
        constexpr static double yl = -1.0;
        constexpr static double yh = 1.0;
        constexpr static double phil = -1*(TMath::Pi()); 
        constexpr static double phih = TMath::Pi();
        constexpr static double cosl = -1.0;
        constexpr static double cosh = 1.0;

        // static std::vector<double> pvec(){ return std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 15.0, 30.0};}
        // static std::vector<double> pvec(){ return std::vector<double>{2.0, 2.4, 2.8, 3.2, 3.6, 4.0, 4.5, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 15.0, 30.0};} // 15
        //static std::vector<double> pvec(){ return std::vector<double>{2.0, 3.0, 4.0, 5.0,  7.0, 10.0, 15.0, 30.0,50,100};} // 15
        // static std::vector<double> pvec(){ return std::vector<double>{0.0,2.0, 4.0, 6.0, 8.0, 10.0, 15.0,50.0};} // 7
        static std::vector<double> pvec(){ return std::vector<double>{0.0,2.0,3,4, 5.0, 7.0,8, 10.0,12, 15.0,20,30,50.0};} // 7
        // static std::vector<double> pvec(){ return std::vector<double>{ 2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 4.0, 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 5.0, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.0, 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0, 11.0, 12.5, 15.0, 30.0};} // 61
        // static std::vector<double> pvec(){ return std::vector<double>{ 2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2, 5.4, 5.6, 5.8, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0, 11.0, 12.5, 15.0, 30.0};} // 37
        // static std::vector<double> pvec(){ return std::vector<double>{ 2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0, 5.2, 5.4, 5.6, 5.8, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0, 11.0, 12.5, 15.0, 30.0};} // 37
        // static std::vector<double> pvec(){ return std::vector<double>{ 2.0, 2.25, 2.5, 2.75, 3.0, 3.25, 3.5, 3.75, 4.0, 4.25, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0, 11.0, 12.5, 17.0, 30.0};} // 26
        // static std::vector<double> pvec(){ return std::vector<double>{ 2.0, 2.25, 2.5, 2.75, 3.0, 3.25, 3.5, 3.75, 4.0, 4.25, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0, 11.0, 12.5, 15.0, 18.0, 30.0};} // 26
        static std::vector<double> yvec(){ return std::vector<double>{-1.0, -0.5, 0.0,  0.5, 1.0};} // 4
         //static std::vector<double> yvec(){ return std::vector<double>{-1.0, 1.0};} // 4
         static std::vector<double> centvec(){ return std::vector<double>{0., 20., 60.,100.,180.};} // 5
        // static std::vector<double> yvec(){ return std::vector<double>{-1.0, -0.6, -0.3, 0.3, 0.6, 1.0};} // 6
        // static std::vector<double> yvec(){ return std::vector<double>{-1.0, 1.0};} // 2
        // static std::vector<double> yvec(){ return std::vector<double>{-1.0, -0.4, 0.4, 1.0};} // 4
        // static std::vector<double> yvec(){ return std::vector<double>{-1.0, -0.7, -0.4, -0.2, 0.0, 0.2, 0.4, 0.7, 1.0};} // 9
        //static std::vector<double> yvec(){ return std::vector<double>{-1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0};} // 11 
        // static std::vector<double> yvec(){ return std::vector<double>{-1.0, -0.9,-0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, 0.0, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};} // 11 

        // TEST 1 ONE BIN
        // constexpr static int npt = 2 ;
        // constexpr static int ny = 1;
        // constexpr static int nphi = 1;
        // constexpr static double pl = 2;
        // constexpr static double ph = 30;
        // constexpr static double yl = -1.0;
        // constexpr static double yh =  1.0;
        // constexpr static double phil = -1*(TMath::Pi()); 
        // constexpr static double phih = TMath::Pi();
        // // static std::vector<double> pvec(){ return std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 15.0, 30.0};}
        // // static std::vector<double> pvec(){ return std::vector<double>{2.0, 4.0, 6.0, 9.0, 12.0, 20.0, 30.0};}
        // // static std::vector<double> pvec(){ return std::vector<double>{0.0, 2.0, 2.4, 2.8, 3.2, 3.6, 4.0, 4.5, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 15.0, 20.0, 30.0};}
        // static std::vector<double> pvec(){ return std::vector<double>{2.0, 10.0, 30.0};}
        // // static std::vector<double> pvec(){ return std::vector<double>{2.0, 30.0};}
        // // static std::vector<double> yvec(){ return std::vector<double>{-1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0};}
        // static std::vector<double> yvec(){ return std::vector<double>{-1.0, 1.0};}

        // // TEST 2 FINE FINE BIN PT Y
        // constexpr static int npt = 60 ;
        // constexpr static int ny = 10;
        // constexpr static int nphi = 1;
        // constexpr static double pl = 2;
        // constexpr static double ph = 30;
        // constexpr static double yl = -1.0;
        // constexpr static double yh = 1.0;
        // constexpr static double phil = -1*(TMath::Pi()); 
        // constexpr static double phih = TMath::Pi();
        // // static std::vector<double> pvec(){ return std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 15.0, 30.0};}
        // static std::vector<double> pvec(){ return algo::getEqualRangeV<double>(pl, ph, npt);}
        // static std::vector<double> yvec(){ return algo::getEqualRangeV<double>(yl, yh, ny);}
};

#endif
