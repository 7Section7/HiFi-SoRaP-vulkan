#include "basicsrp.h"
#include <QElapsedTimer>

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

BasicSRP::BasicSRP()
{
}

void BasicSRP::computeSRP(Grid *results)
{
	int  i, j;
	int  NEL, NAZ, AZstep, ELstep;
	double el, az, elrad, azrad;
	vector3 XS, force;

	AZstep = step_AZ;
	ELstep = step_EL;

	NEL = 180/ELstep+1;
	NAZ = 360/AZstep+1;

	for(j = 0; j < NAZ; j++)
	{
		az = -180 + j*AZstep;
		azrad = az*M_PI/180.;
		for(i = 0; i < NEL; i++)
		{
			el = (-90. + i*ELstep);
			elrad = el*M_PI/180.;

			/* direction of the sunray shotted (xs = -rs) */
			XS = vector3(-cos(elrad)*cos(azrad), -cos(elrad)*sin(azrad), -sin(elrad));

			computeStepSRP(XS,force);

			(*results)(j,i)=Output(az, el, force.x, force.y, force.z);
		}
	}
}

vector3 BasicSRP::computeSRP(const vector3& XS, float angleX, float angleY, float angleZ)
{
	vector3 force;
	const Eigen::Affine3f rotationX(Eigen::AngleAxisf(-angleX,Eigen::Vector3f(1,0,0)));
	const Eigen::Affine3f rotationY(Eigen::AngleAxisf(-angleY,Eigen::Vector3f(0,1,0)));
	const Eigen::Affine3f rotationZ(Eigen::AngleAxisf(-angleZ,Eigen::Vector3f(0,0,1)));

	const auto model = rotationX.matrix()*rotationY.matrix()*rotationZ.matrix();
	const auto output = model * Eigen::Vector4f(XS.x,XS.y,XS.z,0);

	const auto rotatedXS = vector3{output[0],output[1],output[2]};
	computeStepSRP(rotatedXS, force);

	return force;
}

vector3 BasicSRP::computeSRP(const vector3& XS, Eigen::Matrix4f& satelliteRotation)
{
	vector3 force;

	const auto model = satelliteRotation.inverse();
	const auto output = model * Eigen::Vector4f(XS.x,XS.y,XS.z,0);

	const auto rotatedXS = vector3{output[0],output[1],output[2]};
	computeStepSRP(rotatedXS, force);

	return force;
}
