#ifndef BASICSRP_H
#define BASICSRP_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "srp.h"
#include <QTime>

/*
 * This class contains the shared information between the basic models: cannonball & nplate.
 */
class BasicSRP: public SRP
{
public:
	BasicSRP();
	void computeSRP(Grid *results);
	vector3 computeSRP(const vector3& XS, float angleX, float angleY, float angleZ);
	vector3 computeSRP(const vector3& XS, Eigen::Matrix4f& satelliteRotation);
	virtual void computeStepSRP(const vector3& XS, vector3& force, const vector3& V1 = DEFAULT_VEC3,
			const vector3& V2 = DEFAULT_VEC3) = 0;
};

#endif // BASICSRP_H
