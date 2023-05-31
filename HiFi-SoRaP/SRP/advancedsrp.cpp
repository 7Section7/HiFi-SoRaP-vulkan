#include "advancedsrp.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include<QElapsedTimer>
#include<iostream>
#include<fstream>
#include <iomanip>

AdvancedSRP::AdvancedSRP()
{
}

void AdvancedSRP::computeSRP(Grid *results)
{
	int NEL, NAZ, AZstep, ELstep;
	precision::value_type el, az, elrad, azrad;
	vector3 RS, V1, V2, XS, cm;

	vector3 force;

	/* Right now we are not allowing the user to set the CM.
	// MOVE the stacecraft so tht CM is at the Origin
	cm = vector3{this->cm.x(),this->cm.y(),this->cm.z()};
	TriangleMesh *mesh = satellite->getMesh();
	for(r = 0; r < (int)mesh->vertices.size(); r++)
	{
		float x = mesh->vertices[r].x() - cm[0];
		float y = mesh->vertices[r].y() - cm[1];
		float z = mesh->vertices[r].z() - cm[2];
		mesh->vertices[r] = QVector3D(x,y,z);
	}*/

	AZstep = step_AZ;
	ELstep = step_EL;

	NEL = 180/ELstep+1;
	NAZ = 360/AZstep+1;

	for(int j = 0; j < NAZ; j++)
	{
		az = -180 + j*AZstep;
		azrad = az*M_PI/180.;
		/* Scan EL */
		for(int i =0; i < NEL; i++)
		{
			el = (-90. + i*ELstep);
			elrad = el*M_PI/180.;

			/*	RS is sun position wrt CM of spacecraft and <V1,V2> generate the Pixel Array */

			RS[0] =  cos(elrad)*cos(azrad); RS[1] = cos(elrad)*sin(azrad);  RS[2] = sin(elrad);
			V1[0] = -sin(elrad)*cos(azrad); V1[1] = -sin(elrad)*sin(azrad); V1[2] = cos(elrad);
			V2[0] =  sin(azrad); 	 	 	V2[1] = -cos(azrad); 	 	 	V2[2] = 0;
			XS[0] = -RS[0]; 	 	 	 	XS[1] = -RS[1];   				XS[2] = -RS[2];

			computeStepSRP(XS,force,V1,V2);

			(*results)(j,i)=Output(az, el, force.x, force.y, force.z);

			if(progressBar && progressBar->value()<NEL*NAZ) progressBar->setValue(progressBar->value()+1);
			QCoreApplication::processEvents();
			if(stopExecution) return;
		}

	}

	/* MOVE the stacecraft to its initial origin
	for(r = 0; r < (int)mesh->vertices.size(); r++)
	{
		float x = mesh->vertices[r].x() + cm[0];
		float y = mesh->vertices[r].y() + cm[1];
		float z = mesh->vertices[r].z() + cm[2];
		mesh->vertices[r] = QVector3D(x,y,z);
	}*/
}

vector3 AdvancedSRP::computeSRP(const vector3& XS,float angleX, float angleY, float angleZ)
{
	vector3 force;
	const Eigen::Affine3f rotationX(Eigen::AngleAxisf(-angleX,Eigen::Vector3f(1,0,0)));
	const Eigen::Affine3f rotationY(Eigen::AngleAxisf(-angleY,Eigen::Vector3f(0,1,0)));
	const Eigen::Affine3f rotationZ(Eigen::AngleAxisf(-angleZ,Eigen::Vector3f(0,0,1)));

	const auto model = rotationX.matrix()*rotationY.matrix()*rotationZ.matrix();
	const auto output = model * Eigen::Vector4f(XS.x,XS.y,XS.z,0);
	const auto outputV1 = model * Eigen::Vector4f(0,1,0,0);
	const auto outputV2 = model * Eigen::Vector4f(0,0,1,0);

	const vector3 rotatedXS{output[0],output[1],output[2]};
	const vector3 rotatedV1{outputV1[0],outputV1[1],outputV1[2]};
	const vector3 rotatedV2{outputV2[0],outputV2[1],outputV2[2]};

	computeStepSRP(rotatedXS,force,rotatedV1,rotatedV2);

	return force;
}

vector3 AdvancedSRP::computeSRP(const vector3& XS, Eigen::Matrix4f &satelliteRotation)
{
	vector3 force;

	// Do not use 'auto' in the next variables, it is important to specify their types explicitly.
	const Eigen::Matrix4f model = satelliteRotation.inverse();
	const Eigen::Vector4f output = model * Eigen::Vector4f(XS.x,XS.y,XS.z,0);
	const Eigen::Vector4f outputV1 = model * Eigen::Vector4f(0,1,0,0);
	const Eigen::Vector4f outputV2 = model * Eigen::Vector4f(0,0,1,0);

	const vector3 rotatedXS{output[0],output[1],output[2]};
	const vector3 rotatedV1{outputV1[0],outputV1[1],outputV1[2]};
	const vector3 rotatedV2{outputV2[0],outputV2[1],outputV2[2]};

	computeStepSRP(rotatedXS,force,rotatedV1,rotatedV2);

	return force;
}



void AdvancedSRP::saveResults(Grid *results)
{
	saveResultsToFile(nx,cm,results);
}
