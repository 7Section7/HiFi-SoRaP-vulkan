#ifndef LIGHT_H
#define LIGHT_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "Lib/common.h"
#include <qvector3d.h>
#include <QGLShaderProgram>
#include <memory>

/*
 * This class contains the information of a light source.
 */
class Light
{
    QVector3D lightDir;
    QVector3D rightDir,upDir;
public:
    Light();

    void toGPU(std::unique_ptr<QGLShaderProgram> &program);

    QVector3D getLightDir() const;
    void setLightDir(const QVector3D &value);
    QVector3D getRightDir() const;
    void setRightDir(const QVector3D &value);
    QVector3D getUpDir() const;
    void setUpDir(const QVector3D &value);
};

#endif // LIGHT_H
