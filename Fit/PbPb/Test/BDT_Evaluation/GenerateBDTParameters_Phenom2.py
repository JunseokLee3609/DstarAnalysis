#!/usr/bin/env python3
"""Generate BDT evaluation parameter JSON with Phenomenological2 background."""

import json

def param(value, min_v, max_v, fixed=False):
    return {"value": value, "min": min_v, "max": max_v, "fixed": fixed}

def generate_bdt_json(output_file="bdt_fit_parameters_phenom2.json"):
    """Generate JSON with Phenomenological2 background PDF."""
    
    # Define pT bins: 5-20 with 1 GeV interval, 20-50 with 2 GeV interval
    pt_bins = []
    # 5-20: interval of 1
    for i in range(5, 20):
        pt_bins.append((float(i), float(i+1)))
    # 20-50: interval of 2
    for i in range(20, 50, 2):
        pt_bins.append((float(i), float(i+2)))
    
    data = {
        "description": "BDT Evaluation Fit Parameters for D* PbPb with Phenomenological2",
        "global_settings": {
            "fit_strategy": 2,
            "use_minos": False,
            "use_hesse": True,
            "mva_scan": {
                "min": 0.99,
                "max": 0.999,
                "n_points": 10
            }
        },
        "bins": {}
    }
    
    # Generate MVA bins
    mva_min = 0.99
    mva_max = 0.999
    n_points = 10
    
    mva_thresholds = [mva_min + i * (mva_max - mva_min) / (n_points - 1) 
                     for i in range(n_points)]
    
    # Generate pT bins with MVA sub-bins
    for pt_min, pt_max in pt_bins:
        bin_key = f"bin_pT_{pt_min:.1f}_{pt_max:.1f}".replace('.', '_').replace('_0_', '_0p')
        bin_key = f"bin_pT_{pt_min:.2f}_{pt_max:.2f}".replace('.', 'p')
        
        data["bins"][bin_key] = {
            "bin_info": {
                "var_type": "pT",
                "var_min": pt_min,
                "var_max": pt_max,
                "description": f"pT bin [{pt_min}, {pt_max}]"
            },
            "mva_bins": {}
        }
        
        for mva in mva_thresholds:
            mva_key = f"mva_{mva:.3f}".replace('.', '_').replace('_', '_', 1).replace('_', '_')
            # Clean up: "mva_0_990" format
            parts = f"{mva:.3f}".split('.')
            mva_key = f"mva_{parts[0]}_{parts[1]}"
            
            data["bins"][bin_key]["mva_bins"][mva_key] = {
                "mva_threshold": round(mva, 3),
                "yield_ratios": {
                    "nsig_ratio": 0.05 + (mva - mva_min) / (mva_max - mva_min) * 0.25,  # 0.05 to 0.30
                    "nbkg_ratio": 0.95 - (mva - mva_min) / (mva_max - mva_min) * 0.25   # 0.95 to 0.70
                },
                "signal_pdf": {
                    "type": "DBCrystalBall",
                    "parameters": {
                        "mean": param(0.1455, 0.145, 0.146, False),
                        "sigma": param(0.0005, 0.0001, 0.01, False),
                        "alphaL": param(1.1, 0.1, 5.0, True),
                        "alphaR": param(1.1, 0.1, 5.0, True),
                        "nL": param(2.0, 1.0, 100.0, True),
                        "nR": param(2.5, 1.0, 100.0, True)
                    }
                },
                "background_pdf": {
                    "type": "Phenomenological2",
                    "parameters": {
                        "m": param(1.0, 0.0, 10.0, False),
                        "lambda": param(-2.0, -50.0, 50.0, False),
                        "m_pi": param(0.13957, 0.13957, 0.13957, True),
                        "p0": param(0.0, -1000.0, 1000.0, True),
                        "p1": param(0.0, -1000.0, 1000.0, True)
                    }
                }
            }
    
    # Add rapidity (y) bins
    y_bins = [(0.0,0.2),(0.2,0.4),(0.4,0.6),(0.6,0.8),(0.8,1.0)]
    for y_min, y_max in y_bins:
        bin_key = f"bin_y_{str(y_min).replace('.', '_')}_{str(y_max).replace('.', '_')}"
        data["bins"][bin_key] = {"bin_info": {"var_type": "y", "var_min": y_min, "var_max": y_max, "description": f"y bin [{y_min}, {y_max}]"}, "mva_bins": {}}
        for mva in mva_thresholds:
            parts = f"{mva:.3f}".split('.')
            mva_key = f"mva_{parts[0]}_{parts[1]}"
            # Use the first pT bin as reference for signal/background PDFs
            first_pt_key = list(data["bins"].keys())[0]
            first_mva_key = list(data["bins"][first_pt_key]["mva_bins"].keys())[0]
            data["bins"][bin_key]["mva_bins"][mva_key] = {
                "mva_threshold": round(mva,3),
                "yield_ratios": {"nsig_ratio": 0.2, "nbkg_ratio": 0.8},
                "signal_pdf": data["bins"][first_pt_key]["mva_bins"][first_mva_key]["signal_pdf"],
                "background_pdf": data["bins"][first_pt_key]["mva_bins"][first_mva_key]["background_pdf"]
            }
    
    # Add cos bins
    cos_bins = [(-1.0,-0.5),(-0.5,0.0),(0.0,0.5),(0.5,1.0)]
    for cmin, cmax in cos_bins:
        bin_key = f"bin_cos_{str(cmin).replace('.', '_')}_{str(cmax).replace('.', '_')}"
        data["bins"][bin_key] = {"bin_info": {"var_type": "cos", "var_min": cmin, "var_max": cmax, "description": f"cos bin [{cmin}, {cmax}]"}, "mva_bins": {}}
        for mva in mva_thresholds:
            parts = f"{mva:.3f}".split('.')
            mva_key = f"mva_{parts[0]}_{parts[1]}"
            # Use the first pT bin as reference for signal/background PDFs
            first_pt_key = list(data["bins"].keys())[0]
            first_mva_key = list(data["bins"][first_pt_key]["mva_bins"].keys())[0]
            data["bins"][bin_key]["mva_bins"][mva_key] = {
                "mva_threshold": round(mva,3),
                "yield_ratios": {"nsig_ratio": 0.2, "nbkg_ratio": 0.8},
                "signal_pdf": data["bins"][first_pt_key]["mva_bins"][first_mva_key]["signal_pdf"],
                "background_pdf": data["bins"][first_pt_key]["mva_bins"][first_mva_key]["background_pdf"]
            }
    
    # Write to file
    with open(output_file, 'w') as f:
        json.dump(data, f, indent=2)
    
    print(f"Generated {output_file}")
    print(f"  - pT bins: {pt_bins}")
    print(f"  - {n_points} MVA bins from {mva_min} to {mva_max}")
    print(f"  - Signal PDF: DBCrystalBall (alpha,n fixed)")
    print(f"  - Background PDF: Phenomenological2 (thresholded)")

if __name__ == "__main__":
    generate_bdt_json()
