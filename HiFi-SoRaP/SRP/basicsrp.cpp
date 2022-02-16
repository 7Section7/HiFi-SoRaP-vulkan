#include "basicsrp.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

BasicSRP::BasicSRP()
{
}

void BasicSRP::computeSRP(Grid *results)
{
	int  i, j;
	int  NEL, NAZ, AZstep, ELstep;
	double xs[3];
	double el, az, elrad, azrad, ffx, ffy, ffz;
	QVector3D force;

	AZstep = step_AZ;
	ELstep = step_EL;

	if( ((360 % AZstep)!=0) || ((180 % ELstep)!=0) )
	{
		printf("Step Sizes determined for Azimuth and Elevation not good\n");
		return ;//-1;
	}

	NEL = 180/ELstep+1;
	NAZ = 360/AZstep+1;

	QTime myTimer;
	myTimer.start();

	for(j = 0; j < NAZ; j++)
	{
		az = -180 + j*AZstep;
		azrad = az*M_PI/180.;
		for(i = 0; i < NEL; i++)
		{
			el = (-90. + i*ELstep);
			elrad = el*M_PI/180.;

			/* direction of the sunray shotted (xs = -rs) */
			xs[0] = -cos(elrad)*cos(azrad); xs[1] = -cos(elrad)*sin(azrad); xs[2] = -sin(elrad);

			computeStepSRP(xs,force);
			ffx=force.x();  ffy=force.y();  ffz=force.z();

			(*results)(j,i)=Output(az, el, ffx, ffy, ffz);

		}
	}
	int nMilliseconds = myTimer.elapsed();
}

QVector3D BasicSRP::computeSRP(QVector3D lightDir,float angleX, float angleY, float angleZ)
{
	QVector3D force;
	Eigen::Affine3f rotationX(Eigen::AngleAxisf(-angleX,Eigen::Vector3f(1,0,0)));
	Eigen::Affine3f rotationY(Eigen::AngleAxisf(-angleY,Eigen::Vector3f(0,1,0)));
	Eigen::Affine3f rotationZ(Eigen::AngleAxisf(-angleZ,Eigen::Vector3f(0,0,1)));

	Eigen::Matrix4f model = rotationX.matrix()*rotationY.matrix()*rotationZ.matrix();
	Eigen::Vector4f output = model * Eigen::Vector4f(lightDir.x(),lightDir.y(),lightDir.z(),0);

	double xs[] = {output[0],output[1],output[2]};
	computeStepSRP(xs,force);

	return force;
}

QVector3D BasicSRP::computeSRP(QVector3D lightDir, Eigen::Matrix4f &satelliteRotation)
{
	QVector3D force;

	Eigen::Matrix4f model = satelliteRotation.inverse();
	Eigen::Vector4f output = model * Eigen::Vector4f(lightDir.x(),lightDir.y(),lightDir.z(),0);

	double xs[] = {output[0],output[1],output[2]};
	computeStepSRP(xs,force);

	return force;
}
