#ifndef NPLATE_H
#define NPLATE_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "SRP/basicsrp.h"

/*
 * This class implements the NPlate method.
 */
class NPlate: public BasicSRP
{
	void fsrp_nplate(const vector3& XS, vector3 &force);

	std::vector<precision::value_type> A,ps,pd;
	std::vector<vector3> n;
	int np;
public:
	QString satelliteInfoFile;
	NPlate();
	void computeStepSRP(const vector3& XS, vector3& force, const vector3& V1 = DEFAULT_VEC3,
			const vector3& V2 = DEFAULT_VEC3);

	bool isSatelliteInfoLoaded();
	void loadSatelliteInfo();
};

#endif // NTRACESRP_H
