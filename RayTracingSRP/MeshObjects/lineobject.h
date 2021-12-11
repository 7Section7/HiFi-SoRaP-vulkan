#ifndef LINEOBJECT_H
#define LINEOBJECT_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "object.h"

/*
 * This class represents a 3D line mesh.
 */
class LineObject : public Object
{
public:
	LineObject();

	void initializeBuffers();
	void draw(std::unique_ptr<QGLShaderProgram> &program);
};

#endif // LINEOBJECT_H
