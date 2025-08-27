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

// Lab to Event Plane frame
// Event plane angle (Psi2Raw_Trk) is used as the quantization axis
TVector3 DstarDau1Vector_EventPlane(const TLorentzVector DstarLV_Lab, const TLorentzVector DstarDau1LV_Lab, double eventPlaneAngle) {
    // ******** Transform daughter to D* rest frame ******** //
    TLorentzVector Dau1LV_DstarRestFrame(DstarDau1LV_Lab);
    Dau1LV_DstarRestFrame.Boost(-DstarLV_Lab.BoostVector());

    TVector3 Dau1Vec_Boosted = Dau1LV_DstarRestFrame.Vect();
    // TVector3 DstarVec_Lab = DstarLV_Lab.Vect();

    // ******** Rotate to align with standard coordinate system ******** //
    // First rotate around Z to align with x-axis
    // Dau1Vec_Boosted.RotateZ(-DstarVec_Lab.Phi());
    // Then rotate around Y to align with z-axis  
    // Dau1Vec_Boosted.RotateY(-DstarVec_Lab.Theta());

    // ******** Rotate around z-axis by event plane angle ******** //
    // The event plane angle defines the quantization axis in the lab frame
    // We rotate the daughter momentum by this angle to use event plane as reference
    Dau1Vec_Boosted.RotateZ(-eventPlaneAngle);

    return Dau1Vec_Boosted;
}
// TVector3 DstarDau1Vector_EventPlane(const TLorentzVector DstarLV_Lab, const TLorentzVector DstarDau1LV_Lab, double eventPlaneAngle) {
    
//     // 원본 TLorentzVector를 수정하지 않기 위해 복사본을 생성합니다.
//     TLorentzVector DstarLV_rotated = DstarLV_Lab;
//     TLorentzVector Dau1LV_rotated = DstarDau1LV_Lab;

//     // ******** 1. 시스템 전체를 회전시켜 이벤트 평면을 X-Z 평면으로 정렬 ******** //
//     // Z축을 중심으로 (-eventPlaneAngle) 만큼 회전시키면, 
//     // 이벤트 평면의 방향 벡터 (cos(Psi), sin(Psi), 0)가 새로운 X축 (1, 0, 0) 방향을 향하게 됩니다.
//     DstarLV_rotated.RotateZ(-eventPlaneAngle);
//     Dau1LV_rotated.RotateZ(-eventPlaneAngle);

//     // ******** 2. 회전된 D*의 정지 좌표계로 딸 입자를 변환(Boost) ******** //
//     // 이제 D*와 딸 입자는 이벤트 평면이 X-Z 평면인 새로운 좌표계에 놓여 있습니다.
//     // 이 상태에서 D* 정지 좌표계로 변환을 수행합니다.
//     Dau1LV_rotated.Boost(-DstarLV_rotated.BoostVector());

//     // ******** 3. 최종 3-Vector 반환 ******** //
//     // 반환된 벡터의 각 좌표축은 다음과 같은 의미를 가집니다.
//     // Py() : 이벤트 평면에 수직인(out-of-plane) 방향의 운동량 성분
//     // Px() : 이벤트 평면 내에 있으면서 D* 운동량과 수직인(in-plane transverse) 방향의 성분
//     // Pz() : D* 운동량 방향(in-plane longitudinal)과 관련된 성분 (주의: 헬리시티 좌표계는 아님)
//     return Dau1LV_rotated.Vect();
// }