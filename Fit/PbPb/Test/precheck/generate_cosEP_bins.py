#!/usr/bin/env python3
"""
Generate kinematic bins queue with cosThetaEP binning
Format: ptMin ptMax yAbsMin yAbsMax centMin centMax dcaModeInt cosEPMin cosEPMax
"""

# Kinematic bins
pt_bins = [(30, 50)]  # Merged pT bin 30-50 GeV/c
y_bins = [(0.0, 0.3), (0.3, 0.8), (0.8, 1.2)]
cent_bins = [(0, 10), (10, 30), (30, 50)]
dca_modes = [0, 1, 2]  # 0=inclusive, 1=prompt_rich, 2=nonprompt_rich

# cosThetaEP bins: [-1, -0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4, 0.6, 0.8, 1]
cosEP_edges = [-1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0]
cosEP_bins = [(cosEP_edges[i], cosEP_edges[i+1]) for i in range(len(cosEP_edges)-1)]

output_file = "kinematic_bins_queue_cosEP.txt"

with open(output_file, 'w') as f:
    f.write("# Combined kinematic bins for Condor queue with cosThetaEP binning\n")
    f.write("# Format: ptMin ptMax yAbsMin yAbsMax centMin centMax dcaModeInt cosEPMin cosEPMax\n")
    f.write("# |y| is absolute rapidity, Centrality variable name is 'Centrality'\n")
    f.write("# dcaModeInt: 0=inclusive, 1=prompt_rich, 2=nonprompt_rich\n")
    f.write("# cosThetaEP bins: [-1, -0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4, 0.6, 0.8, 1]\n")
    f.write("# pT merged: [30-50] GeV/c\n")
    f.write("# MC always uses cosThetaHX inclusive [-1, 1]\n")
    f.write("\n")
    
    count = 0
    for pt_min, pt_max in pt_bins:
        for y_min, y_max in y_bins:
            for cent_min, cent_max in cent_bins:
                for dca_mode in dca_modes:
                    for cosEP_min, cosEP_max in cosEP_bins:
                        f.write(f"{pt_min} {pt_max} {y_min} {y_max} {cent_min} {cent_max} {dca_mode} {cosEP_min} {cosEP_max}\n")
                        count += 1

print(f"Generated {count} bins in {output_file}")
print(f"  pT bins: {len(pt_bins)}")
print(f"  y bins: {len(y_bins)}")
print(f"  centrality bins: {len(cent_bins)}")
print(f"  DCA modes: {len(dca_modes)}")
print(f"  cosThetaEP bins: {len(cosEP_bins)}")
print(f"  Total: {len(pt_bins)} × {len(y_bins)} × {len(cent_bins)} × {len(dca_modes)} × {len(cosEP_bins)} = {count}")
