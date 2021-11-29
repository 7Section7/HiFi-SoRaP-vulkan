#ifndef CANBALL_H
#define CANBALL_H
#include "SRP/basicsrp.h"



class Canball: public BasicSRP
{
public:
    double ps;
    float cr, area;
    Canball();
    void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY);

};

#endif // CANBALL_H
