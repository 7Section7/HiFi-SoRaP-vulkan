#ifndef QUAD_H
#define QUAD_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "object.h"

/*
 * This class is a mesh with 2 triangles.
 */
class Quad : public Object
{
public:
	Quad();

	void initializeBuffers();
};

#endif // QUAD_H
