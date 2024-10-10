#ifndef SRP_H
#define SRP_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <vector>
#include <QVector3D>
#include <stdio.h>
#include <stdlib.h>
#include <MeshObjects/object.h>
#include <QString>
#include <unordered_map>
#include "DataVisualization/grid.h"
#include <QProgressBar>

//Pressure
#define PRESSURE  2.2753587667030186e-07L // Before: 4.563157e-6L //4.57e-6
#define DEFAULT_VEC3 vector3{}


/*
 * This class is an abstract class that contains the basic information that a SRP method should have.
 */
class SRP
{
protected:
	QProgressBar* progressBar;
	bool stopExecution;
	int step_AZ, step_EL;
	std::string output;
	precision::value_type msat;

	Object *satellite;
	void saveResultsToFile(float xpix, const vector3& cm, Grid *results);

public:
	SRP();
	virtual void computeSRP(Grid *results) = 0;
	virtual vector3 computeSRP(const vector3& XS, float angleX, float angleY, float angleZ) = 0;
	virtual vector3 computeSRP(const vector3& XS, Eigen::Matrix4f& satelliteRotation) = 0;
	virtual void computeStepSRP(const vector3& XS, vector3& force, const vector3& V1 = DEFAULT_VEC3,
			const vector3& V2 = DEFAULT_VEC3) = 0;

	virtual void saveResults(Grid *results);

	int getAzimuthStep() const;
	void setAzimuthStep(int value);
	int getElevationStep() const;
	void setElevationStep(int value);
	std::string getOutput() const;
	void setOutput(const std::string &value);
	float getMsat() const;
	void setMsat(float value);

	Object *getSatellite() const;
	void setSatellite(Object *value);
	QProgressBar *getProgressBar() const;
	void setProgressBar(QProgressBar *value);
	bool getStopExecution() const;
	void setStopExecution(bool value);
};

#endif // SRP_H
