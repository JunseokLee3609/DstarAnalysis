#!/usr/bin/env python3
"""Generate BDT evaluation fit parameter JSON files."""

import argparse
import json
from typing import Dict, List, Tuple


def param(value: float, min_v: float, max_v: float, fixed: bool = False) -> Dict[str, object]:
    """Create a parameter dictionary."""
    return {"value": value, "min": min_v, "max": max_v, "fixed": bool(fixed)}


def parse_ranges(items: List[str], cast=float) -> List[Tuple[float, float]]:
    """Parse range strings like '7:10' into tuples."""
    out: List[Tuple[float, float]] = []
    for it in items:
        left, right = it.split(":")
        out.append((cast(left), cast(right)))
    return out


def make_signal_params_dbcb() -> Dict[str, Dict[str, object]]:
    """Default DBCrystalBall signal parameters."""
    return {
        "mean": param(0.1455, 0.145, 0.146, False),
        "sigma": param(0.0005, 0.0001, 0.01, False),
        "alphaL": param(1.10, 0.10, 5.00, False),
        "alphaR": param(1.10, 0.10, 5.00, False),
        "nL": param(2.00, 1.00, 100.0, True),
        "nR": param(2.50, 1.00, 100.0, True),
    }


def make_background_params_roodstd0bg() -> Dict[str, Dict[str, object]]:
    """Default RooDstD0BG background parameters."""
    return {
        "m": param(0.5, 0.0, 2.0, False),
        "lambda": param(0.1, -500.0, 500.0, False),
        "p0": param(0.1, -1000.0, 1000.0, False),
        "p1": param(0.1, -1000.0, 1000.0, False),
    }


def make_bin_entry(var_type: str, var_min: float, var_max: float,
                   mva_thresholds: List[float],
                   signal_type: str = "DBCrystalBall",
                   background_type: str = "RooDstD0BG",
                   nsig_ratio: float = 0.5,
                   nbkg_ratio: float = 0.5) -> Tuple[str, Dict[str, object]]:
    """Create a single bin entry with MVA-specific parameters."""
    
    # Create bin name - kinematic variable only
    bin_name = f"bin_{var_type}_{var_min}_{var_max}".replace(".", "_")
    
    # Create bin info
    bin_info = {
        "var_type": var_type,
        "var_min": var_min,
        "var_max": var_max,
        "description": f"{var_type} bin [{var_min}, {var_max}]"
    }
    
    # Create MVA-specific parameters
    mva_bins = {}
    for mva in mva_thresholds:
        mva_key = f"mva_{str(mva).replace('.', '_')}"
        mva_bins[mva_key] = {
            "mva_threshold": mva,
            "yield_ratios": {
                "nsig_ratio": nsig_ratio,
                "nbkg_ratio": nbkg_ratio
            },
            "signal_pdf": {
                "type": signal_type,
                "parameters": make_signal_params_dbcb()
            },
            "background_pdf": {
                "type": background_type,
                "parameters": make_background_params_roodstd0bg()
            }
        }
    
    entry = {
        "bin_info": bin_info,
        "mva_bins": mva_bins
    }
    
    return bin_name, entry


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate BDT evaluation fit parameter JSON")
    parser.add_argument("--output", default="bdt_fit_parameters.json",
                        help="Output JSON path")
    parser.add_argument("--pt-bins", nargs="+", default=["5:7", "7:10", "10:15", "15:20", "20:30", "30:50"],
                        help="pT bins as min:max")
    parser.add_argument("--y-bins", nargs="+", default=["0.0:0.2", "0.2:0.4", "0.4:0.6", "0.6:0.8", "0.8:1.0"],
                        help="Rapidity bins as min:max")
    parser.add_argument("--cent-bins", nargs="+", default=["0:10", "10:30", "30:50", "50:90"],
                        help="Centrality bins as min:max")
    parser.add_argument("--mva-min", type=int, default=990,
                        help="Minimum MVA threshold (e.g., 990 for 0.990)")
    parser.add_argument("--mva-max", type=int, default=999,
                        help="Maximum MVA threshold (e.g., 999 for 0.999)")
    parser.add_argument("--signal", default="DBCrystalBall",
                        help="Signal PDF type")
    parser.add_argument("--background", default="RooDstD0BG",
                        help="Background PDF type")
    parser.add_argument("--nsig-ratio", type=float, default=0.5,
                        help="Default nsig initialization ratio (0.0-1.0)")
    parser.add_argument("--nbkg-ratio", type=float, default=0.5,
                        help="Default nbkg initialization ratio (0.0-1.0)")
    args = parser.parse_args()

    # Parse bin ranges
    pt_bins = parse_ranges(args.pt_bins, float)
    y_bins = parse_ranges(args.y_bins, float)
    cent_bins = parse_ranges(args.cent_bins, float)
    
    # Generate MVA threshold list
    mva_thresholds = [i / 1000.0 for i in range(args.mva_min, args.mva_max + 1)]
    
    # Create JSON structure
    json_data = {
        "description": "BDT Evaluation Fit Parameters for D* PbPb",
        "global_settings": {
            "fit_strategy": 2,
            "use_minos": False,
            "use_hesse": True,
            "mva_scan": {
                "min": args.mva_min / 1000.0,
                "max": args.mva_max / 1000.0,
                "n_points": len(mva_thresholds)
            }
        },
        "bins": {}
    }
    
    # Add pT bins
    print(f"Generating {len(pt_bins)} pT bins (MVA scan: {args.mva_min/1000.0:.3f} - {args.mva_max/1000.0:.3f})...")
    print(f"  Yield ratios: nsig={args.nsig_ratio}, nbkg={args.nbkg_ratio}")
    for pt_min, pt_max in pt_bins:
        bin_name, bin_entry = make_bin_entry("pT", pt_min, pt_max, mva_thresholds, 
                                             args.signal, args.background,
                                             args.nsig_ratio, args.nbkg_ratio)
        json_data["bins"][bin_name] = bin_entry
        print(f"  {bin_name}: pT [{pt_min}, {pt_max}] GeV/c, {len(mva_thresholds)} MVA points")
    
    # Add rapidity bins
    print(f"Generating {len(y_bins)} rapidity bins...")
    for y_min, y_max in y_bins:
        bin_name, bin_entry = make_bin_entry("y", y_min, y_max, mva_thresholds,
                                             args.signal, args.background,
                                             args.nsig_ratio, args.nbkg_ratio)
        json_data["bins"][bin_name] = bin_entry
        print(f"  {bin_name}: |y| [{y_min}, {y_max}], {len(mva_thresholds)} MVA points")
    
    # Add centrality bins
    print(f"Generating {len(cent_bins)} centrality bins...")
    for cent_min, cent_max in cent_bins:
        bin_name, bin_entry = make_bin_entry("centrality", cent_min, cent_max, mva_thresholds,
                                             args.signal, args.background,
                                             args.nsig_ratio, args.nbkg_ratio)
        json_data["bins"][bin_name] = bin_entry
        print(f"  {bin_name}: Centrality [{cent_min}, {cent_max}]%, {len(mva_thresholds)} MVA points")
    
    # Write to file
    with open(args.output, 'w') as f:
        json.dump(json_data, f, indent=2)
    
    print(f"\n✅ Generated {args.output}")
    print(f"   Total bins: {len(json_data['bins'])}")
    print(f"   pT bins: {len(pt_bins)}")
    print(f"   Rapidity bins: {len(y_bins)}")
    print(f"   Centrality bins: {len(cent_bins)}")
    print(f"   MVA thresholds per bin: {len(mva_thresholds)}")


if __name__ == "__main__":
    main()
