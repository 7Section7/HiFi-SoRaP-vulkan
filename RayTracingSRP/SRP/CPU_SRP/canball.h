#ifndef CANBALL_H
#define CANBALL_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

#include "SRP/basicsrp.h"

/*
 * This class implements the Cannonball method.
 */
class Canball: public BasicSRP
{
public:
	double ps;
	float cr, area;

	Canball();
	void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY);
};

#endif // CANBALL_H
