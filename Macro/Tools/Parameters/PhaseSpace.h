Bool_t MuonWithin2018PbPbAcc(const TLorentzVector& muonLV) {
	float eta = fabs(muonLV.Eta());
	float pt = muonLV.Pt();

	return (fabs(eta) < 2.4 &&
	        ((fabs(eta) < 1.2 && pt >= 3.5) ||
	         (1.2 <= fabs(eta) && fabs(eta) < 2.1 && pt >= 5.47 - 1.89 * fabs(eta)) ||
	         (2.1 <= fabs(eta) && pt >= 1.5)));
}

Bool_t DstarDauSimpleAcc(const TLorentzVector& D0LV /* DstarLV */, const TLorentzVector& PionLV) {
	float D0Eta = fabs(D0LV.Eta());
	float PionEta = fabs(PionLV.Eta());
	float D0Rapidity = fabs(D0LV.Rapidity());
	float PionRapidity = fabs(PionLV.Rapidity());
	float Pionpt = PionLV.Pt();
	float D0pt = D0LV.Pt();
	return (D0Rapidity < 1 && Pionpt > 0.4 && PionEta < 1.5);
}
Bool_t D0DauSimpleAcc(const TLorentzVector& D1 /* D0 */){
	float D1Eta = fabs(D1.Eta());
	float D1Rapidity = fabs(D1.Rapidity());
	float D1pt = D1.Pt();
	return (D1pt>1 && D1Eta < 2.4);
}
Bool_t D0DauAcc(const TLorentzVector& D1, const TLorentzVector& D2){
	return D0DauSimpleAcc(D1) && D0DauSimpleAcc(D2);
}
Bool_t DstarAcc(const TLorentzVector& DstarLV) {
	float DstarRapidity = fabs(DstarLV.Rapidity());
	float Dstarpt = DstarLV.Pt();
	return DstarRapidity < 1 && Dstarpt >4;
}
