#ifndef LIGHT_H
#define LIGHT_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "Lib/common.h"
#include <qvector3d.h>

#undef GL_GLEXT_VERSION
#include <QGLShaderProgram>
#include <memory>

/*
 * This class contains the information of a light source.
 */
class Light
{
	vector3 lightDir;
	vector3 rightDir,upDir;
public:
	Light();

	void toGPU(std::unique_ptr<QGLShaderProgram> &program);

	const vector3& getLightDir() const;
	void setLightDir(const vector3& value);
	const vector3& getRightDir() const;
	void setRightDir(const vector3& value);
	const vector3& getUpDir() const;
	void setUpDir(const vector3& value);
};

#endif // LIGHT_H
