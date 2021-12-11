#ifndef SRP_H
#define SRP_H

/***********************************************************************
 +
 * Project: RayTracingSRP
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

#define ctt 4.57e-6
static double DEFAULT_DOUBLE_ARRAY[] = {0,0,0};

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
    float msat;

    Object *satellite;
    void saveResultsToFile(float xpix, QVector3D& cm,Grid *results);

public:
    SRP();
    virtual void computeSRP(Grid *results) = 0;
    virtual QVector3D computeSRP(QVector3D lightDir,float angleX, float angleY, float angleZ) = 0;
    virtual QVector3D computeSRP(QVector3D lightDir,Eigen::Matrix4f& satelliteRotation) = 0;
    virtual void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY) = 0;
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
