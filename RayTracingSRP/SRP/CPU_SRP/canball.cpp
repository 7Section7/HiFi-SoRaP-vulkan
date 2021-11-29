#include "canball.h"

Canball::Canball()
{
}

void Canball::computeStepSRP(double xs[], QVector3D &force, double RS[], double V1[], double V2[])
{
    double PS = (ctt)*cr*area/msat;
    force = -PS*QVector3D(xs[0],xs[1],xs[2]);
}
