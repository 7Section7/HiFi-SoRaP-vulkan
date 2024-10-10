#ifndef ADVANCEDSRP_H
#define ADVANCEDSRP_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
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
	vector3 cm;
	int nx,ny;
	precision::value_type safeDistance;
	long int counter;

	AdvancedSRP();
	void computeSRP(Grid *results);
	vector3 computeSRP(const vector3& XS, float angleX, float angleY, float angleZ);
	vector3 computeSRP(const vector3& XS, Eigen::Matrix4f& satelliteRotation);
	virtual void computeStepSRP(const vector3& XS, vector3& force, const vector3& V1 = DEFAULT_VEC3,
			const vector3& V2 = DEFAULT_VEC3) = 0;

	void saveResults(Grid *results);

	static const float* getNormalNoiseTexture();
	static const float* getUniformNoiseTexture();
	static const float* getFixedUniformNoiseTexture();
};

#endif // ADVANCEDSRP_H
