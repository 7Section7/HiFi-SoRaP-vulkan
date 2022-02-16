#ifndef ADVANCEDSRP_H
#define ADVANCEDSRP_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

#include "srp.h"
#include <QTime>
#include <QCoreApplication>

/*
 * This class contains the shared information between the complex models.
 */
class AdvancedSRP: public SRP
{
public:
	QVector3D cm;
	int nx,ny; //number of pixels in mesh
	float xtot,ytot; //pixel array size
	float safeDistance;

	AdvancedSRP();
	void computeSRP(Grid *results);
	QVector3D computeSRP(QVector3D lightDir,float angleX, float angleY, float angleZ);
	QVector3D computeSRP(QVector3D lightDir,Eigen::Matrix4f& satelliteRotation);
	virtual void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY) = 0;
	int hit(double pixel[], double XS[], double pointInt[]);
	void saveResults(Grid *results);
};

#endif // ADVANCEDSRP_H
