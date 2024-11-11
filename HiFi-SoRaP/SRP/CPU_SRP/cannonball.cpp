#include "cannonball.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

CannonBall::CannonBall()
{
}

void CannonBall::computeStepSRP(const vector3& XS, vector3 &force, const vector3& V1, const vector3& V2)
{
	Q_UNUSED(V1);
	Q_UNUSED(V2);

	const precision::value_type PS = PRESSURE;
	force = (PS)*cr*area/msat*XS;
}

