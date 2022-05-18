#ifndef RAYTRACECPU_H
#define RAYTRACECPU_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "SRP/advancedsrp.h"

/*
 * This class implements the RayTrace (CPU) method.
 */
class RayTraceCPU: public AdvancedSRP
{
	int numSecondaryRays;
	int numDiffuseRays;
	int reflectionType;
	float seed;
	QVector3D pixel;

public:
	RayTraceCPU();
	void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY);

	int getNumSecondaryRays() const;
	void setNumSecondaryRays(int value);

	int getNumDiffuseRays() const;
	void setNumDiffuseRays(int value);

	int getReflectionType() const;
	void setReflectionType(int value);

private:
	QVector3D randomInSphere(QVector3D hitPoint);
	QVector3D randomInSphere();
	QVector3D computePixelForce(double xs[], double Apix, double pixel[]);

	QVector3D rayTrace( double Apix,double point[], double dir[], double importance[], int numSecondaryRays);
	QVector3D computeForce(int rhit, double xs[],double Apix);
	void scatter(double pointInt[],int rhit, double xs[], double importance[],Reflectiveness r );
};

#endif // RAYTRACECPU_H
