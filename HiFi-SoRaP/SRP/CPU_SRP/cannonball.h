#ifndef CANBALL_H
#define CANBALL_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "SRP/basicsrp.h"

/*
 * This class implements the Cannonball method.
 */
class CannonBall: public BasicSRP
{
public:
	precision::value_type cr, area;

	CannonBall();
	void computeStepSRP(const vector3& XS, vector3& force, const vector3& V1 = DEFAULT_VEC3,
			const vector3& V2 = DEFAULT_VEC3);
};

#endif // CANBALL_H
