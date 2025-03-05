/// Helping functions returning the positive muon's coordinates in a given reference frame based on the TLorentzVectors in the lab frame
#include "TVector3.h"
#include "TLorentzVector.h"

// Lab to Helicity
TVector3 DstarDau1Vector_Helicity(const TLorentzVector DstarLV_Lab, const TLorentzVector DstarDau1LV_Lab) {
	// ******** Transform variables of muons from the lab frame to the upsilon's rest frame ******** //
	TVector3 DstarVec_Lab = DstarLV_Lab.Vect();
	TLorentzVector Dau1LV_DstarRestFrame(DstarDau1LV_Lab);

	//(Note. TLorentzVector.BoostVector() gives beta(=px/E,py/E,pz/E) of the parents)
	//(TLorentzVector.Boost() boosts from the rod frame to the lab frame, so plug in -beta to get lab to rod)
	Dau1LV_DstarRestFrame.Boost(-DstarLV_Lab.BoostVector());

	// ******** Rotate the coordinates ******** //
	TVector3 Dau1Vec_Boosted = Dau1LV_DstarRestFrame.Vect();

	//Note: TVector3.Rotate() rotates the vectors, not the coordinates, so should rotate -phi and -theta

	Dau1Vec_Boosted.RotateZ(-DstarVec_Lab.Phi());

	Dau1Vec_Boosted.RotateY(-DstarVec_Lab.Theta());

	return Dau1Vec_Boosted;
}

// Lab to Collins-Soper
// requires the beam parameters
TVector3 DstarDau1Vector_CollinsSoper(const TLorentzVector DstarLV_Lab, const TLorentzVector DstarDau1LV_Lab) {
    // ******** Set beam energy for the Collins-Soper reference frame ******** //
    double sqrt_S_NN = 5.36;                    //(Center of mass Energy per nucleon pair in TeV)
    double beamEnergy = sqrt_S_NN * 1000. / 2.; //(in GeV) (Note. sqrt_S_NN = sqrt(2*E1*E2+2*p1*p2) = 2E1 when two beams have the same E)

    // ******** HX to CS (rotation from HX frame to CS frame) ******** //
    // (1. Boost two beams to upsilon's rest frame)
    // (2. Rotate the coordinates)
    // (3. Get angle between two beams(b1 and -b2), and between b1 and ZHX in the upsilon's rest frame)
    // (4. Calculate delta (angle btw ZHX and ZCS))

    // ******** Transform variables of beams from the lab frame to the upsilon's rest frame ******** //
    TLorentzVector Beam1LV_Boosted(0, 0, beamEnergy, beamEnergy);
    TLorentzVector Beam2LV_Boosted(0, 0, -beamEnergy, beamEnergy); // mind the sign!!

    Beam1LV_Boosted.Boost(-DstarLV_Lab.BoostVector());
    Beam2LV_Boosted.Boost(-DstarLV_Lab.BoostVector());

    // ******** Rotate the coordinates ******** //
    TVector3 Beam1Vector_DstarRestFrame(Beam1LV_Boosted.Vect());
    TVector3 Beam2Vector_DstarRestFrame(Beam2LV_Boosted.Vect());

    TVector3 DstarVector_Lab = DstarLV_Lab.Vect();
    Beam1Vector_DstarRestFrame.RotateZ(-DstarVector_Lab.Phi());
    Beam1Vector_DstarRestFrame.RotateY(-DstarVector_Lab.Theta());

    Beam2Vector_DstarRestFrame.RotateZ(-DstarVector_Lab.Phi());
    Beam2Vector_DstarRestFrame.RotateY(-DstarVector_Lab.Theta());

    // ******** Calculate the angle between z_HX and z_CS ******** //
    TVector3 ZHXunitVec(0, 0, 1);                                                  //(define z_HX unit vector)
    double Angle_B1ZHX = Beam1Vector_DstarRestFrame.Angle(ZHXunitVec);                //(angle between beam1 and z_HX)
    double Angle_B2ZHX = Beam2Vector_DstarRestFrame.Angle(-ZHXunitVec);               //(angle between beam2 and -z_HX =(-beam2 and z_HX) )
    double Angle_B1miB2 = Beam1Vector_DstarRestFrame.Angle(-Beam2Vector_DstarRestFrame); //(angle between beam1 and -beam2)

    double delta = 0; //(define and initialize the angle between z_HX and z_CS)

    // Maths for calculating the angle between z_HX and z_CS is different depending on the sign of the beam1's z-coordinate)
    if (Angle_B1ZHX > Angle_B2ZHX)
        delta = Angle_B2ZHX + Angle_B1miB2 / 2.;
    else if (Angle_B1ZHX < Angle_B2ZHX)
        delta = Angle_B1ZHX + Angle_B1miB2 / 2.;
    else
        std::cout << "beam1PvecBoosted.Pz() = 0?" << std::endl;

    // ******** Rotate the coordinates along the y-axis by the angle between z_HX and z_CS ******** //
    TVector3 DstarDau1Vec_CS = DstarDau1Vector_Helicity(DstarLV_Lab, DstarDau1LV_Lab);

    DstarDau1Vec_CS.RotateY(delta);

    return DstarDau1Vec_CS;
}