#ifndef CENTRALITYUTILS_H
#define CENTRALITYUTILS_H

#include <iostream>

class CentralityUtils {
public:
    static float findNcoll(int hiBin);
    
    static bool isValidCentrality(short centrality);
    
    static float getCentralityPercentage(short centrality);
    
    static int getCentralityBin(float centralityPercent);
    
    static void printCentralityInfo(short centrality);

private:
    static const int NBINS = 200;
    static const float NCOLL_TABLE[200];
};

const float CentralityUtils::NCOLL_TABLE[200] = {
    1893.13, 1867.0, 1834.16, 1805.64, 1770.84, 1744.49, 1699.76, 1661.52, 1615.89, 1579.59, 
    1540.62, 1499.14, 1469.01, 1432.18, 1402.8, 1368.39, 1338.12, 1302.26, 1274.91, 1245.56, 
    1215.28, 1183.76, 1160.61, 1131.12, 1107.67, 1078.54, 1055.72, 1026.72, 1000.57, 980.728, 
    958.777, 936.515, 911.397, 889.182, 869.677, 853.33, 826.999, 808.145, 792.14, 769.639, 
    753.513, 732.883, 716.817, 697.168, 679.091, 668.056, 650.114, 631.024, 616.203, 597.835, 
    583.435, 571.454, 555.478, 543.589, 526.328, 511.657, 497.023, 489.255, 471.52, 461.133, 
    447.767, 436.993, 426.106, 412.626, 403.224, 389.71, 382.595, 371.48, 358.899, 349.179, 
    339.387, 330.523, 320.094, 313.254, 302.339, 292.421, 282.594, 274.834, 268.847, 259.463, 
    252.027, 244.561, 236.738, 229.574, 222.898, 215.138, 207.328, 200.879, 196.592, 190.921, 
    183.942, 176.685, 170.919, 166.96, 161.057, 154.421, 148.816, 144.84, 139.087, 134.448, 
    128.72, 124.905, 121.166, 116.648, 112.367, 109.012, 104.33, 100.736, 97.3484, 93.2283, 
    89.3299, 85.9068, 83.6446, 80.2019, 77.5299, 73.9647, 70.7606, 68.2284, 65.793, 63.4532, 
    60.4738, 58.2406, 55.063, 53.7287, 51.4638, 49.241, 47.0111, 45.5443, 43.1729, 41.5041, 
    39.5449, 37.9282, 36.8918, 34.9287, 33.1886, 31.9177, 30.756, 29.0803, 27.6721, 26.42, 
    25.2678, 24.2585, 23.1429, 22.0138, 21.0169, 19.8203, 19.1043, 18.1478, 17.1715, 16.3605, 
    15.4763, 14.7973, 14.1594, 13.3927, 12.795, 12.1059, 11.5921, 10.9751, 10.3213, 9.94434, 
    9.3518, 8.94274, 8.37618, 7.94437, 7.48868, 7.06923, 6.71137, 6.31856, 6.03184, 5.67048, 
    5.43369, 5.13727, 4.83292, 4.58846, 4.37208, 4.15225, 3.84385, 3.63752, 3.45214, 3.24892, 
    3.02845, 2.81715, 2.66395, 2.5053, 2.29512, 2.13703, 1.93591, 1.79771, 1.64165, 1.54375, 
    1.45878, 1.36718, 1.2942, 1.23934, 1.18423, 1.14467, 1.11826, 1.0863, 1.06149, 1.04497
};

float CentralityUtils::findNcoll(int hiBin) {
    if (hiBin < 0 || hiBin >= NBINS) {
        std::cerr << "Warning: Invalid centrality bin " << hiBin 
                  << ". Using bin 0 (most central)." << std::endl;
        return NCOLL_TABLE[0];
    }
    return NCOLL_TABLE[hiBin];
}

bool CentralityUtils::isValidCentrality(short centrality) {
    return (centrality >= 0 && centrality < NBINS);
}

float CentralityUtils::getCentralityPercentage(short centrality) {
    if (!isValidCentrality(centrality)) {
        return -1.0f;
    }
    return centrality * 0.5f; // Convert bin to percentage (each bin = 0.5%)
}

int CentralityUtils::getCentralityBin(float centralityPercent) {
    if (centralityPercent < 0 || centralityPercent > 100) {
        return -1;
    }
    return static_cast<int>(centralityPercent * 2.0f); // Convert percentage to bin
}

void CentralityUtils::printCentralityInfo(short centrality) {
    if (!isValidCentrality(centrality)) {
        std::cout << "Invalid centrality: " << centrality << std::endl;
        return;
    }
    
    float percentage = getCentralityPercentage(centrality);
    float ncoll = findNcoll(centrality);
    
    std::cout << "Centrality bin: " << centrality 
              << " (" << percentage << "%) "
              << "Ncoll: " << ncoll << std::endl;
}

#endif // CENTRALITYUTILS_H