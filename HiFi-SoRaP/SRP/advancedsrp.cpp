#include "advancedsrp.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

AdvancedSRP::AdvancedSRP()
{
}

void AdvancedSRP::computeSRP(Grid *results)
{
	int i, j,  r;
	int nx, ny;
	int  NEL, NAZ, AZstep, ELstep;
	double d;
	double el, az, elrad, azrad;
	double RS[3], V1[3], V2[3], XS[3], cm[3]; //pixel[3], Pint[3]
	double ffx,ffy,ffz;

	QVector3D force;

	TriangleMesh *mesh = satellite->getMesh();

	d=safeDistance;
	nx=this->nx;
	ny=this->ny;
	cm[0]=this->cm.x();
	cm[1]=this->cm.y();
	cm[2]=this->cm.z();

	/* MOVE the stacecraft so tht CM is at the Origin */
	for(r = 0; r < (int)mesh->vertices.size(); r++)
	{
		float x = mesh->vertices[r].x() - cm[0];
		float y = mesh->vertices[r].y() - cm[1];
		float z = mesh->vertices[r].z() - cm[2];
		mesh->vertices[r] = QVector3D(x,y,z);

	}

	AZstep = step_AZ;//atoi(argv[5]);
	ELstep = step_EL;//atoi(argv[6]);

	if( ((360 % AZstep)!=0) || ((180 % ELstep)!=0) )
	{
		printf("Step Sizes determined for Azimuth and Elevation not good\n");
		return;
	}

	/* (3rd) SCAN different Attitudes the SRP value and save in file */
	NEL = 180/ELstep+1;
	NAZ = 360/AZstep+1;

	QTime myTimer;
	myTimer.start();

	for(j = 0; j < NAZ; j++){
		az = -180 + j*AZstep;
		azrad = az*M_PI/180.;
		/* Scan EL */
		for(i =0; i < NEL; i++){
			el = (-90. + i*ELstep);
			elrad = el*M_PI/180.;

			/*	RS is sun position wrt CM of spacecraft and <V1,V2> generate the Pixel Array */
			RS[0] =  cos(elrad)*cos(azrad); RS[1] =  cos(elrad)*sin(azrad); RS[2] = sin(elrad);
			V1[0] = -sin(elrad)*cos(azrad); V1[1] = -sin(elrad)*sin(azrad); V1[2] = cos(elrad);
			V2[0] =  sin(azrad); 	 	 	V2[1] = -cos(azrad); 	 	 	V2[2] = 0;
			XS[0] = -RS[0]; 	 	 	 	XS[1] = -RS[1];   				XS[2] = -RS[2];

			computeStepSRP(XS,force,RS,V1,V2);
			ffx=force.x();  ffy=force.y();  ffz=force.z();

			(*results)(j,i)=Output(az, el, ffx,ffy,ffz);

			if(progressBar) progressBar->setValue(progressBar->value()+1);
			QCoreApplication::processEvents();
			if(stopExecution) return;
		}
	}

	int nMilliseconds = myTimer.elapsed();
}

QVector3D AdvancedSRP::computeSRP(QVector3D lightDir,float angleX, float angleY, float angleZ)
{
	QVector3D force;
	Eigen::Affine3f rotationX(Eigen::AngleAxisf(-angleX,Eigen::Vector3f(1,0,0)));
	Eigen::Affine3f rotationY(Eigen::AngleAxisf(-angleY,Eigen::Vector3f(0,1,0)));
	Eigen::Affine3f rotationZ(Eigen::AngleAxisf(-angleZ,Eigen::Vector3f(0,0,1)));

	Eigen::Matrix4f model = rotationX.matrix()*rotationY.matrix()*rotationZ.matrix();
	Eigen::Vector4f output = model * Eigen::Vector4f(lightDir.x(),lightDir.y(),lightDir.z(),0);
	Eigen::Vector4f outputV1 = model * Eigen::Vector4f(0,1,0,0);
	Eigen::Vector4f outputV2 = model * Eigen::Vector4f(0,0,1,0);

	double xs[] = {output[0],output[1],output[2]};
	double rs[] = {-output[0],-output[1],-output[2]};
	double v1[] = {outputV1[0],outputV1[1],outputV1[2]};
	double v2[] = {outputV2[0],outputV2[1],outputV2[2]};

	computeStepSRP(xs,force,rs,v1,v2);

	return force;
}

QVector3D AdvancedSRP::computeSRP(QVector3D lightDir, Eigen::Matrix4f &satelliteRotation)
{
	QVector3D force;

	Eigen::Matrix4f model = satelliteRotation.inverse();
	Eigen::Vector4f output = model * Eigen::Vector4f(lightDir.x(),lightDir.y(),lightDir.z(),0);
	Eigen::Vector4f outputV1 = model * Eigen::Vector4f(0,1,0,0);
	Eigen::Vector4f outputV2 = model * Eigen::Vector4f(0,0,1,0);

	double xs[] = {output[0],output[1],output[2]};
	double rs[] = {-output[0],-output[1],-output[2]};
	double v1[] = {outputV1[0],outputV1[1],outputV1[2]};
	double v2[] = {outputV2[0],outputV2[1],outputV2[2]};

	computeStepSRP(xs,force,rs,v1,v2);

	return force;
}

//Returns rhit
int AdvancedSRP::hit(double pixel[], double XS[], double pointInt[])
{
	double Pint[3], depmin,dephit;
	int r, rhit;

	TriangleMesh *mesh = satellite->getMesh();
	int NT = mesh->faces.size();

	depmin = std::numeric_limits<double>::infinity();
	rhit = -1;

	vec3 point(pixel[0],pixel[1],pixel[2]);
	vec3 L(XS[0],XS[1],XS[2]);
	normalize3d(L);
	vec3 intPoint;

	for(r = 0; r < NT; r++)
	{
		if( mesh->hitTriangle(point, L, r, intPoint) )
		{
			/* if intersection keep point_ref and distance */
			Pint[0] = intPoint.x; Pint[1] = intPoint.y; Pint[2] = intPoint.z;

			dephit= ( (pixel[0]-Pint[0])*(pixel[0]-Pint[0])
					+ (pixel[1]-Pint[1])*(pixel[1]-Pint[1])
					+ (pixel[2]-Pint[2])*(pixel[2]-Pint[2]) );
			if(dephit<depmin){
				depmin=dephit;
				rhit = r;
				pointInt[0] = Pint[0]; pointInt[1] = Pint[1]; pointInt[2] = Pint[2];
			}
		}
	}
	return rhit;
}

void AdvancedSRP::saveResults(Grid *results)
{
	saveResultsToFile(nx,cm,results);
}
