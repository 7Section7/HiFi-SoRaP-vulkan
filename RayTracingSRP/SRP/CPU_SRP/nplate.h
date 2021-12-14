#ifndef NPLATE_H
#define NPLATE_H

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
 * This class implements the NPlate method.
 */
class NPlate: public BasicSRP
{
	void fsrp_nplate(int N, double xs[], double fs[]);

	std::vector<double> A,ps,pd;
	std::vector<QVector3D> n;
	int np;
public:
	QString satelliteInfoFile;
	NPlate();
	void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY);

	bool isSatelliteInfoLoaded();
	void loadSatelliteInfo();
};

#endif // NTRACESRP_H
