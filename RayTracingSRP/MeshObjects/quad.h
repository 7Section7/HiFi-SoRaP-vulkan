#ifndef QUAD_H
#define QUAD_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
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
