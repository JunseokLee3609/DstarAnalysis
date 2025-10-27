#!/usr/bin/env python3
"""Generate D* JSON parameter grids for PbPb analysis."""

import argparse
import json
from copy import deepcopy
from typing import Dict, List, Tuple


def param(value: float, min_v: float, max_v: float, fixed: bool = False) -> Dict[str, object]:
    return {"value": value, "min": min_v, "max": max_v, "fixed": bool(fixed)}


def make_signal_params_dbcb() -> Dict[str, Dict[str, object]]:
    """PbPb signal parameters - Double-sided Crystal Ball"""
    return {
        "mean": param(0.1455, 0.1450, 0.1460),
        "sigma": param(0.0005, 0.0001, 0.0100),
        "alphaL": param(1.10, 0.10, 4.00, True),
        "alphaR": param(1.10, 0.10, 4.00, True),
        "nL": param(2.00, 1.00, 100.0, True),
        "nR": param(2.50, 1.00, 100.0, True),
    }


def make_background_params_phenom2() -> Dict[str, Dict[str, object]]:
    """PbPb background parameters - Phenomenological2"""
    return {
        "m": param(0.5, 0.0, 2.0, True),
        "lambda": param(0.1, -500.0, 2.0, True),
        "m_pi": param(0.13957, 0.13957, 0.13957, True),
    }


def make_yields_defaults() -> Dict[str, Dict[str, object]]:
    return {
        "nsig_ratio": param(0.1, 0.0, 1.0),
        "nbkg_ratio": param(0.8, 0.0, 1.0),
    }


def build_pbpb_grid(
    pt_ranges: List[Tuple[float, float]],
    cent_ranges: List[Tuple[float, float]]
) -> Dict[str, dict]:
    """Build pT×Centrality grid for PbPb analysis"""
    grid = {}
    
    signal_params = make_signal_params_dbcb()
    background_params = make_background_params_phenom2()
    yield_params = make_yields_defaults()
    
    for pt_min, pt_max in pt_ranges:
        for cent_min, cent_max in cent_ranges:
            bin_id = f"pT_{pt_min:.1f}_{pt_max:.1f}_cent_{int(cent_min)}_{int(cent_max)}"
            grid[bin_id] = {
                "kinematic": {
                    "pT_min": pt_min,
                    "pT_max": pt_max,
                    "centrality_min": cent_min,
                    "centrality_max": cent_max,
                },
                "signal": deepcopy(signal_params),
                "background": deepcopy(background_params),
                "background_type": "Phenomenological2",
                "yields": deepcopy(yield_params),
            }
    
    return grid


def main():
    parser = argparse.ArgumentParser(description="Generate PbPb parameter grid")
    parser.add_argument(
        "--pt-bins",
        default="5:7,7:10,10:15,15:20",
        help="pT bins as 'min:max' (comma-separated)"
    )
    parser.add_argument(
        "--cent-bins",
        default="0:10,10:30,30:50,50:90",
        help="Centrality bins as 'min:max' (comma-separated)"
    )
    parser.add_argument(
        "-o", "--output",
        default="Parameters/dstar_parameters_DBCrystalBall_Phenomenological2_PbPb.json",
        help="Output JSON file"
    )
    
    args = parser.parse_args()
    
    # Parse pT ranges
    pt_ranges = []
    for pt_range in args.pt_bins.split(","):
        pt_min, pt_max = map(float, pt_range.split(":"))
        pt_ranges.append((pt_min, pt_max))
    
    # Parse centrality ranges
    cent_ranges = []
    for cent_range in args.cent_bins.split(","):
        cent_min, cent_max = map(float, cent_range.split(":"))
        cent_ranges.append((cent_min, cent_max))
    
    # Build grid
    grid = build_pbpb_grid(pt_ranges, cent_ranges)
    
    # Save to JSON
    with open(args.output, "w") as f:
        json.dump(grid, f, indent=2)
    
    print(f"✓ Generated PbPb parameters: {args.output}")
    print(f"  pT bins: {args.pt_bins}")
    print(f"  Centrality bins: {args.cent_bins}")
    print(f"  Total bins: {len(grid)}")


if __name__ == "__main__":
    main()
