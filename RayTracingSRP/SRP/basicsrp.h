#ifndef BASICSRP_H
#define BASICSRP_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "srp.h"
#include <QTime>

/*
 * This class contains the shared information between the basic models: cannonball & nplate.
 */
class BasicSRP: public SRP
{
public:
    BasicSRP();
    void computeSRP(Grid *results);
    QVector3D computeSRP(QVector3D lightDir,float angleX, float angleY, float angleZ);
    QVector3D computeSRP(QVector3D lightDir,Eigen::Matrix4f& satelliteRotation);
    virtual void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY)=0;
};

#endif // BASICSRP_H
