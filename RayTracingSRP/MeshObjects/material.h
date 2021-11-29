#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>

enum Reflectiveness{Reflective, Transparent, Lambertian};

class Material
{

public:
    QString namemat;
    double ps, pd;
    Reflectiveness r;
    float refIdx;

    Material();
    Material( QString aMaterialName );
};

#endif // MATERIAL_H
