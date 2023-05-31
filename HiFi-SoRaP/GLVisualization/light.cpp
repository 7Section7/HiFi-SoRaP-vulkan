#include "light.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

Light::Light()
{
	lightDir = vector3(1,0,0);
	rightDir = vector3(0,0,1);
	upDir = vector3(0,1,0);
}

void Light::toGPU(std::unique_ptr<QGLShaderProgram> &program)
{
	program->bind();
	GLuint lightLocation = program->uniformLocation("lightDirection");
	glUniform3f(lightLocation,lightDir.x,lightDir.y,lightDir.z);

	GLuint rightLightLocation = program->uniformLocation("V1");
	glUniform3f(rightLightLocation,rightDir.x,rightDir.y,rightDir.z);

	GLuint upLightLocation = program->uniformLocation("V2");
	glUniform3f(upLightLocation,upDir.x,upDir.y,upDir.z);
}

/*
 * Getters and setters.
 */
const vector3& Light::getLightDir() const
{
	return lightDir;
}

void Light::setLightDir(const vector3& value)
{
	lightDir = value;
}

const vector3& Light::getRightDir() const
{
	return rightDir;
}

void Light::setRightDir(const vector3& value)
{
	rightDir = value;
}

const vector3& Light::getUpDir() const
{
	return upDir;
}

void Light::setUpDir(const vector3& value)
{
	upDir = value;
}
