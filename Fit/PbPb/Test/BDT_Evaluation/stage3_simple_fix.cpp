// Just add GetEdges filtering and keep the file scanning approach that worked

// After line 72, change to use GetEdges:
    // Build set of valid analytic bin ranges for filtering  
    const auto& edges = GetEdges(vtype);
    std::set<std::pair<double,double>> validBins;
    for(size_t i=0; i+1<edges.size(); ++i){
      validBins.insert({edges[i], edges[i+1]});
    }

// In the file scanning loop, check validBins instead of validFineBins
        bool isValidBin = (validBins.find({vmin, vmax}) != validBins.end());
        if(!isValidBin) continue;

