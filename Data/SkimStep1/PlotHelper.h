class PlotHelper {
    private:
    std::string cut_;
    float topmargin_
    float bottommargin_
    float leftmargin_
    float rightmargin_
    float textsize_
    float labelsize_
    float titlesize_
    float xtitleoffset_
    float ytitleoffset_
    TH1D* h1d_;
    TH1D* hgen1d_;
    TH2D* h2d_;
    TH2D* hgen2d_;
    
    TPad* pad1_;
    TPad* pad2_;
    public:
    PlotHelper(Chain *tree);
    ~PlotHelper();
    vector<TCanvas*> canvases_; 
    std::map<std::string,<int,float,float> rangemap_;
    vector<leg*> legs_;
    void SetPads(){
    pad1 = new TPad("pad1", "Pad for original histograms", 0.0, 0.3, 1.0, 1.0);
    pad2 = new TPad("pad2", "Pad for cumulative histograms", 0.0, 0.0, 1.0, 0.3);
    pad1->SetTopMargin(0.01);
    pad1->SetBottomMargin(0.01);
    pad2->SetTopMargin(0.01);
    pad2->SetBottomMargin(0.15);
    };
    void ClearPad(){
        delete pad1_;
        delete pad2_;
    };
    void Draw1DPlot(){
        for (auto it = rangemap_.begin(); it != rangemap_.end(); ++it){
            Setpads();
            h1d_ = new TH1D(it->first.c_str(),it->first.c_str(),std::get<0>(it->second),std::get<1>(it->second),std::get<2>(it->second));
            hgen1d_ = new TH1D((it->first+"_gen").c_str(),(it->first+"_gen").c_str(),std::get<0>(it->second),std::get<1>(it->second),std::get<2>(it->second));
            tree->Draw((it->first+">>"+it->first).c_str(),cut_.c_str());
            tree->Draw((it->first+">>"+it->first+"_gen").c_str(),("matchGEN==1 &&"+cut_).c_str());
            h1d_->SetLineColor(kBlack);
            hgen1d_->SetLineColor(kRed);
            pad1_->cd();
            h1d_->Draw("HIST");
            hgen1d_->Draw("HIST same");
            legs_.push_back(new TLegend(0.7,0.7,0.9,0.9));
            legs_.back()->AddEntry(h1d_, "Reco", "l");
            legs_.back()->AddEntry(hgen1d_, "Gen", "l");
            legs_.back()->Draw();
            canvases_.push_back(new TCanvas());
            canvases_.back()->cd();
            pad1_->Draw();
            pad2_->Draw();
            pad1_->cd();
            h1d_->Draw();
            hgen1d_->Draw("same");
            legs_.back()->Draw();
            pad2_->cd();
            h1d_->Draw();
            hgen1d_->Draw("same");
            legs_.back()->Draw();
            canvases_.back()->SaveAs((it->first+".png").c_str());
            ClearPad();
        }

    };

    void Draw2DPlot();
    void AddVar(std::string varname, int nbins, float min, float max, float xlow, float ylow, float xhigh, float yhigh){
        rangemap_[varname] = std::make_tuple(nbins,min,max,xlow,ylow,xhigh,yhigh);
    };
    void SetCut(std::string cut){
        cut_ = cut;
    };
};
