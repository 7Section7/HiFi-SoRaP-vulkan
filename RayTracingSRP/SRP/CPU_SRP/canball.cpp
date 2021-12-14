#include "canball.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

Canball::Canball()
{
}

void Canball::computeStepSRP(double xs[], QVector3D &force, double RS[], double V1[], double V2[])
{
	double PS = (ctt)*cr*area/msat;
	force = -PS*QVector3D(xs[0],xs[1],xs[2]);
}
