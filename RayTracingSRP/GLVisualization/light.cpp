#include "light.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

Light::Light()
{
    lightDir = QVector3D(1,0,0);
    rightDir = QVector3D(0,0,1);
    upDir = QVector3D(0,1,0);
}

void Light::toGPU(std::unique_ptr<QGLShaderProgram> &program)
{
    program->bind();
    GLuint lightLocation = program->uniformLocation("lightDirection");
    glUniform3f(lightLocation,lightDir.x(),lightDir.y(),lightDir.z());
}

/*
 * Getters and setters.
 */
QVector3D Light::getLightDir() const
{
    return lightDir;
}

void Light::setLightDir(const QVector3D &value)
{
    lightDir = value;
}

QVector3D Light::getRightDir() const
{
    return rightDir;
}

void Light::setRightDir(const QVector3D &value)
{
    rightDir = value;
}

QVector3D Light::getUpDir() const
{
    return upDir;
}

void Light::setUpDir(const QVector3D &value)
{
    upDir = value;
}
