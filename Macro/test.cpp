// File: plotGaussian.C

#include "TF1.h"
#include "TCanvas.h"
#include "TAxis.h"

// The main function that will be executed by ROOT
void test() {
    // 1. Create a canvas to draw on
    // TCanvas(name, title, width, height)
    TCanvas *c1 = new TCanvas("c1", "Gaussian Plot", 800, 600);

    // 2. Define the Gaussian function
    // TF1(name, formula, x_min, x_max)
    // "gaus" is a predefined ROOT formula: [0]*exp(-0.5*((x-[1])/[2])^2)
    TF1 *gausFunc = new TF1("gausFunc", "gaus", -10, 10);

    // 3. Set the parameters for the Gaussian
    // Parameter 0: Constant / Height
    // Parameter 1: Mean (μ)
    // Parameter 2: Standard Deviation (σ)
    double height = 1.0;
    double mean = 2.0;
    double sigma = 1.5;
    gausFunc->SetParameters(height, mean, sigma);

    // 4. Customize the plot's appearance (optional)
    gausFunc->SetLineColor(kBlue + 1); // Set line color
    gausFunc->SetLineWidth(3);         // Set line width
    gausFunc->SetTitle("Gaussian Distribution;X Value;Probability Density"); // Title;X-axis;Y-axis

    // 5. Draw the function on the canvas
    gausFunc->Draw();

    // To save the plot automatically, uncomment the line below
    c1->SaveAs("gaussian_plot.png");
}