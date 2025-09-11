#!/usr/bin/env python3
import json
import argparse
from typing import List, Tuple

def param(value, min_v, max_v, fixed=False):
    return {"value": value, "min": min_v, "max": max_v, "fixed": bool(fixed)}

def parse_ranges(items: List[str], cast=float) -> List[Tuple[float, float]]:
    out = []
    for it in items:
        a, b = it.split(":")
        out.append((cast(a), cast(b)))
    return out

def make_signal_params_dbcb():
    # Defaults tuned for D* Δm fits in your env
    return {
        "mean":  param(0.1455, 0.1450, 0.1460, True),
        "sigma": param(0.0005, 0.0001, 0.0100, True),
        "alphaL": param(1.10, 0.10, 2.00, False),
        "alphaR": param(1.10, 0.10, 2.00, False),
        "nL":    param(2.00, 1.00, 50.0, False),
        "nR":    param(2.50, 1.00, 50.0, False),
    }

def make_background_params_phenom2():
    # (x - m_pi)^m * exp(lambda * (x - m_pi))
    # lambda는 loader에서 lambda/p0/tau 모두 허용
    return {
        "m":      param(2.0,  -10.0,  10.0, False),
        "lambda": param(0.1,   -2.0,   2.0, False),
        "m_pi":   param(0.13957, 0.13957, 0.13957, True),  # 고정
    }

def make_background_params_phenom():
    # (x - m_pi)^m * exp(p0*(x - m_pi) + p1*(x - m_pi)^2 + p2*(x - m_pi)^3)
    return {
        "m":   param(2.0,  -10.0,  10.0, False),
        "p0":  param(0.1, -1000.0, 1000.0, False),
        "p1":  param(0.1, -1000.0, 1000.0, False),
        "p2":  param(0.1, -1000.0, 1000.0, False),
        "m_pi": param(0.13957, 0.13957, 0.13957, False),
    }

def make_yields_defaults():
    # 적절히 수정 가능
    return {
        "nsig_ratio": param(0.5, 0.3, 0.8, False),
        "nbkg_ratio": param(0.1, 0.1, 0.5, False),
    }

def make_bin_entry(pt_min, pt_max, cos_min, cos_max, cent_min, cent_max,
                   signal_type="DBCrystalBall", background_type="Phenomenological2"):
    if signal_type != "DBCrystalBall":
        raise ValueError("This generator currently implements DBCrystalBall for signal.")
    if background_type not in ("Phenomenological2", "Phenomenological"):
        raise ValueError("background_type must be Phenomenological2 or Phenomenological")

    signal_params = make_signal_params_dbcb()
    if background_type == "Phenomenological2":
        bkg_params = make_background_params_phenom2()
    else:
        bkg_params = make_background_params_phenom()

    return {
        "bin_info": {
            "pt_min": pt_min, "pt_max": pt_max,
            "cos_min": cos_min, "cos_max": cos_max,
            "cent_min": cent_min, "cent_max": cent_max,
            "description": f"pT [{pt_min}, {pt_max}), cos(θ*) [{cos_min}, {cos_max}), cent [{cent_min}, {cent_max})",
        },
        "signal_pdf": {
            "type": signal_type,
            "parameters": signal_params
        },
        "background_pdf": {
            "type": background_type,
            "parameters": bkg_params
        },
        "yields": make_yields_defaults()
    }

def main():
    ap = argparse.ArgumentParser(description="Generate dstar_parameters_grid_unified_pp_v2.json")
    ap.add_argument("--output", required=True, help="Output JSON path")
    ap.add_argument("--signal", default="DBCrystalBall",
                    help="Signal PDF type (default: DBCrystalBall)")
    ap.add_argument("--background", default="Phenomenological2",
                    help="Background PDF: Phenomenological2 or Phenomenological")
    ap.add_argument("--pt-bins", nargs="+", default=["5:7","7:10","10:20","20:50"],
                    help="pt bins as min:max (floats)")
    ap.add_argument("--cos-bins", nargs="+", default=["0:0.2","0.2:0.4","0.4:0.6","0.6:0.8","0.8:1.0"],
                    help="cos bins as min:max (floats)")
    ap.add_argument("--cent-bins", nargs="+", default=["0:10"],
                    help="centrality bins as min:max (ints). For pp this is only for bin-key matching.")
    ap.add_argument("--fit-strategy", type=int, default=2)
    ap.add_argument("--use-minos", action="store_true", default=True)
    ap.add_argument("--no-use-hesse", dest="use_hesse", action="store_false", default=True)
    ap.add_argument("--use-cuda", action="store_true", default=False)
    args = ap.parse_args()

    pt_bins = parse_ranges(args.pt_bins, float)
    cos_bins = parse_ranges(args.cos_bins, float)
    cent_bins = parse_ranges(args.cent_bins, int)

    bins = {}
    bin_idx = 1
    for (pt_min, pt_max) in pt_bins:
        for (cos_min, cos_max) in cos_bins:
            for (cent_min, cent_max) in cent_bins:
                entry = make_bin_entry(pt_min, pt_max, cos_min, cos_max, cent_min, cent_max,
                                       signal_type=args.signal, background_type=args.background)
                bins[f"bin_{bin_idx}"] = entry
                bin_idx += 1

    data = {
        "dstar_parameters": {
            "global_settings": {
                "fit_strategy": args.fit_strategy,
                "use_minos": bool(args.use_minos),
                "use_hesse": bool(args.use_hesse),
                "use_cuda": bool(args.use_cuda),
            },
            "bins": bins
        }
    }
    with open(args.output, "w") as f:
        json.dump(data, f, indent=2)
    print(f"Wrote JSON with {len(bins)} bins to {args.output}")

if __name__ == "__main__":
    main()
