#ifndef NPLATE_H
#define NPLATE_H
#include "SRP/basicsrp.h"

class NPlate: public BasicSRP
{
    void fsrp_nplate(int N, double xs[], double fs[]);

    std::vector<double> A,ps,pd;
    std::vector<QVector3D> n;
    int np;
public:
    QString satelliteInfoFile;
    NPlate();
    void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY);

    bool isSatelliteInfoLoaded();
    void loadSatelliteInfo();
};

#endif // NTRACESRP_H
