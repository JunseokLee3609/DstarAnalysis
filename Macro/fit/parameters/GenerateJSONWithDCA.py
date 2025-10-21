#!/usr/bin/env python3
"""Generate D* JSON parameter grids with optional DCA-bin overrides."""

import argparse
import json
from copy import deepcopy
from typing import Dict, List, Tuple


def param(value: float, min_v: float, max_v: float, fixed: bool = False) -> Dict[str, object]:
    return {"value": value, "min": min_v, "max": max_v, "fixed": bool(fixed)}


def parse_ranges(items: List[str], cast=float) -> List[Tuple[float, float]]:
    out: List[Tuple[float, float]] = []
    for it in items:
        left, right = it.split(":")
        out.append((cast(left), cast(right)))
    return out


def make_signal_params_dbcb() -> Dict[str, Dict[str, object]]:
    return {
        "mean": param(0.1455, 0.1450, 0.1460),
        "sigma": param(0.0005, 0.0001, 0.0100),
        "alphaL": param(1.10, 0.10, 2.00),
        "alphaR": param(1.10, 0.10, 2.00),
        "nL": param(2.00, 1.00, 100.0, True),
        "nR": param(2.50, 1.00, 100.0, True),
    }


def make_signal_params_doublegauss() -> Dict[str, Dict[str, object]]:
    return {
        "mean": param(0.1455, 0.1452, 0.1458),
        "sigma1": param(0.0005, 0.0001, 0.0100),
        "sigma2": param(0.0010, 0.0001, 0.0100),
        "fraction": param(0.70, 0.00, 1.00),
    }


def make_background_params_phenom2() -> Dict[str, Dict[str, object]]:
    return {
        "m": param(0.5, 0.0, 2.0, True),
        "lambda": param(0.1, -500.0, 2.0, True),
        "m_pi": param(0.13957, 0.13957, 0.13957, True),
    }


def make_background_params_phenom() -> Dict[str, Dict[str, object]]:
    return {
        "m": param(0.5, 0.0, 2.0),
        "p0": param(0.1, -1000.0, 2.0),
        "p1": param(0.1, -1000.0, 1000.0),
        "p2": param(0.1, -1000.0, 1000.0),
        "m_pi": param(0.13957, 0.13957, 0.13957),
    }


def make_background_params_dstd0() -> Dict[str, Dict[str, object]]:
    return {
        "p0": param(0.1, 0.0, 1.0),
        "p1": param(0.1, -1000.0, 1000.0),
        "p2": param(0.1, -1000.0, 1000.0),
    }


def make_yields_defaults() -> Dict[str, Dict[str, object]]:
    return {
        "nsig_ratio": param(0.5, 0.2, 1.0),
        "nbkg_ratio": param(0.1, 0.0, 0.8),
    }


def _parse_bool(value: str) -> bool:
    token = value.strip().lower()
    if token in {"1", "true", "t", "yes", "y"}:
        return True
    if token in {"0", "false", "f", "no", "n"}:
        return False
    raise ValueError(f"Cannot interpret '{value}' as boolean")


def parse_param_overrides(spec: str, base: Dict[str, Dict[str, object]]) -> Dict[str, Dict[str, object]]:
    overrides: Dict[str, Dict[str, object]] = {}
    if not spec:
        return overrides

    tokens = [tok.strip() for tok in spec.split(',') if tok.strip()]
    if not tokens:
        return overrides

    for token in tokens:
        if '=' not in token:
            raise ValueError(f"Override '{token}' must contain '='")
        name, payload = token.split('=', 1)
        name = name.strip()
        parts = [p.strip() for p in payload.split(':') if p.strip()]
        if not parts:
            raise ValueError(f"Override '{token}' is missing value content")

        def base_attr(key: str, default):
            if base and name in base and key in base[name]:
                return base[name][key]
            return default

        value = float(parts[0])
        min_v = base_attr('min', value)
        max_v = base_attr('max', value)
        fixed = base_attr('fixed', False)

        if len(parts) >= 2:
            min_v = float(parts[1])
        if len(parts) >= 3:
            max_v = float(parts[2])
        if len(parts) >= 4:
            fixed = _parse_bool(parts[3])

        overrides[name] = param(value, min_v, max_v, fixed)

    return overrides


def build_dca_bins(dca_ranges: List[Tuple[float, float]],
                   signal_type: str,
                   background_type: str,
                   sig_override_spec: str,
                   bkg_override_spec: str,
                   yield_override_spec: str) -> Dict[str, dict]:
    if not dca_ranges:
        return {}

    signal_base = make_signal_params_dbcb() if signal_type == "DBCrystalBall" else make_signal_params_doublegauss()
    if background_type == "Phenomenological2":
        background_base = make_background_params_phenom2()
    elif background_type == "Phenomenological":
        background_base = make_background_params_phenom()
    elif background_type == "DstD0":
        background_base = make_background_params_dstd0()
    else:
        raise ValueError(f"Unsupported background type for DCA bins: {background_type}")
    yield_base = make_yields_defaults()

    sig_overrides = parse_param_overrides(sig_override_spec, signal_base)
    bkg_overrides = parse_param_overrides(bkg_override_spec, background_base)
    yield_overrides = parse_param_overrides(yield_override_spec, yield_base)

    dca_bins: Dict[str, dict] = {}
    for idx, (dca_min, dca_max) in enumerate(dca_ranges, start=1):
        signal_params = deepcopy(signal_base)
        signal_params.update(sig_overrides)

        background_params = deepcopy(background_base)
        background_params.update(bkg_overrides)

        yields = deepcopy(yield_base)
        yields.update(yield_overrides)

        dca_bins[f"dca_bin_{idx}"] = {
            "dca_min": dca_min,
            "dca_max": dca_max,
            "signal_pdf": {"parameters": signal_params},
            "background_pdf": {"parameters": background_params},
            "yields": yields,
        }

    return dca_bins


def make_bin_entry(pt_min: float, pt_max: float,
                   cos_min: float, cos_max: float,
                   cent_min: float, cent_max: float,
                   signal_type: str,
                   background_type: str,
                   dca_ranges: List[Tuple[float, float]],
                   dca_signal_override: str,
                   dca_background_override: str,
                   dca_yield_override: str) -> Dict[str, object]:
    if signal_type not in {"DBCrystalBall", "DoubleGaussian"}:
        raise ValueError("signal_type must be DBCrystalBall or DoubleGaussian")
    if background_type not in {"Phenomenological2", "Phenomenological", "DstD0"}:
        raise ValueError("background_type must be Phenomenological2, Phenomenological, or DstD0")

    if signal_type == "DBCrystalBall":
        signal_params = make_signal_params_dbcb()
    else:
        signal_params = make_signal_params_doublegauss()

    if background_type == "Phenomenological2":
        background_params = make_background_params_phenom2()
    elif background_type == "Phenomenological":
        background_params = make_background_params_phenom()
    elif background_type == "DstD0":
        background_params = make_background_params_dstd0()
    else:
        raise ValueError(f"Unsupported background type: {background_type}")

    entry: Dict[str, object] = {
        "bin_info": {
            "pt_min": pt_min,
            "pt_max": pt_max,
            "cos_min": cos_min,
            "cos_max": cos_max,
            "cent_min": cent_min,
            "cent_max": cent_max,
            "description": f"pT [{pt_min}, {pt_max}), cos(θ*) [{cos_min}, {cos_max}), cent [{cent_min}, {cent_max})",
        },
        "signal_pdf": {
            "type": signal_type,
            "parameters": signal_params,
        },
        "background_pdf": {
            "type": background_type,
            "parameters": background_params,
        },
        "yields": make_yields_defaults(),
    }

    if dca_ranges:
        entry["dca_bins"] = build_dca_bins(dca_ranges,
                                            signal_type,
                                            background_type,
                                            dca_signal_override,
                                            dca_background_override,
                                            dca_yield_override)
    return entry


def canon_signal(name: str) -> str:
    token = name.strip().lower().replace(" ", "").replace("_", "")
    if token in {"dbcb", "dbcrystalball", "doublecrystalball", "doublesidedcrystalball"}:
        return "DBCrystalBall"
    if token in {"doublegaussian", "doublegauss", "dgauss"}:
        return "DoubleGaussian"
    if name in {"DBCrystalBall", "DoubleGaussian"}:
        return name
    raise ValueError(f"Unsupported signal type: {name}")


def canon_background(name: str) -> str:
    token = name.strip().lower().replace(" ", "").replace("_", "")
    if token in {"phenomenological2", "phenom2", "p2"}:
        return "Phenomenological2"
    if token in {"phenomenological", "phenom", "p1"}:
        return "Phenomenological"
    if token in {"dstd0", "dst+d0", "dst_d0"}:
        return "DstD0"
    if name in {"Phenomenological2", "Phenomenological"}:
        return name
    if name == "DstD0":
        return name
    raise ValueError(f"Unsupported background type: {name}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate D* fit parameter JSON with optional DCA bins")
    parser.add_argument("--output", help="Output JSON path (default: dstar_parameters_<signal>_<background>.json)")
    parser.add_argument("--signal", default="DBCrystalBall",
                        help="Signal PDF type (DBCrystalBall or DoubleGaussian)")
    parser.add_argument("--background", default="Phenomenological2",
                        help="Background PDF type (Phenomenological2, Phenomenological, or DstD0)")
    parser.add_argument("--pt-bins", nargs="+", default=["5:7", "7:10", "10:20", "20:50"],
                        help="pT bins as min:max")
    parser.add_argument("--cos-bins", nargs="+", action="append", default=None,
                        help="cos bins as min:max")
    parser.add_argument("--cent-bins", nargs="+", default=["0:10"],
                        help="centrality bins as min:max (ints)")
    parser.add_argument("--dca-bins", nargs="*", default=None,
                        help="Optional DCA bins as min:max. Applies to all kinematic bins if provided.")
    parser.add_argument("--dca-signal", default="",
                        help="Comma-separated DCA signal overrides (name=value[:min][:max][:fixed])")
    parser.add_argument("--dca-background", default="",
                        help="Comma-separated DCA background overrides (name=value[:min][:max][:fixed])")
    parser.add_argument("--dca-yields", default="",
                        help="Comma-separated DCA yield overrides (name=value[:min][:max][:fixed])")
    parser.add_argument("--fit-strategy", type=int, default=2)
    parser.add_argument("--use-minos", action="store_true", default=True)
    parser.add_argument("--no-use-hesse", dest="use_hesse", action="store_false", default=True)
    parser.add_argument("--use-cuda", action="store_true", default=False)
    args = parser.parse_args()

    signal_type = canon_signal(args.signal)
    background_type_requested = canon_background(args.background)

    pt_bins = parse_ranges(args.pt_bins, float)
    if args.cos_bins is None:
        cos_bin_specs = ["0:0.2", "0.2:0.4", "0.4:0.6", "0.6:0.8", "0.8:1.0"]
    else:
        cos_bin_specs = [item for group in args.cos_bins for item in group]
    cos_bins = parse_ranges(cos_bin_specs, float)
    cent_bins = parse_ranges(args.cent_bins, int)

    if args.dca_bins is None:
        dca_bins: List[Tuple[float, float]] = []
    else:
        if not args.dca_bins:
            raise ValueError("--dca-bins specified but no ranges provided")
        dca_bins = parse_ranges(args.dca_bins, float)

    bins: Dict[str, Dict[str, object]] = {}
    bin_idx = 1
    for pt_min, pt_max in pt_bins:
        for cos_min, cos_max in cos_bins:
            for cent_min, cent_max in cent_bins:
                if float(pt_min) >= 10.0:
                    background_type = "Phenomenological2"
                else:
                    background_type = background_type_requested
                entry = make_bin_entry(pt_min, pt_max, cos_min, cos_max, cent_min, cent_max,
                                       signal_type, background_type,
                                       dca_bins,
                                       args.dca_signal,
                                       args.dca_background,
                                       args.dca_yields)
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
            "bins": bins,
        }
    }

    out_path = args.output if args.output else f"dstar_parameters_{signal_type}_{background_type_requested}.json"
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)
    print(f"Wrote JSON with {len(bins)} bins to {out_path}")


if __name__ == "__main__":
    main()
