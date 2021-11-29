#ifndef OUTPUT_H
#define OUTPUT_H

#include <QVector3D>
#include <limits>
#include <stdio.h>
#include <stdlib.h>

/*
 * This class stores the information of the computation of a SRP method.
 */
class Output
{

    double azimuth;
    double elevation;
    QVector3D forces;

public:
    Output();
    Output(double az, double el, double ffx, double ffy, double ffz);

    QVector3D getOutputWithMeanForces();
    QVector3D getOutputWithForceX();
    QVector3D getOutputWithForceY();
    QVector3D getOutputWithForceZ();

    void save(FILE* fo);

    QVector3D getForces() const;
    double getAzimuth() const;
    double getElevation() const;
};

#endif // OUTPUT_H
