#ifndef MATERIAL_H
#define MATERIAL_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "Lib/common.h"
#include <QString>

enum Reflectiveness{Reflective, Transparent, Lambertian};

/*
 * This class represents the material of the surface of the spacecraft.
 */
class Material
{
public:
	QString namemat;
	precision::value_type ps, pd, refIdx;
	Reflectiveness r;

	Material();
	Material( QString aMaterialName );
};

#endif // MATERIAL_H
