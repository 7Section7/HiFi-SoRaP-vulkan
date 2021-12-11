#ifndef MATERIAL_H
#define MATERIAL_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <QString>

enum Reflectiveness{Reflective, Transparent, Lambertian};

/*
 * This class represents the material of the surface of the spacecraft.
 */
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
