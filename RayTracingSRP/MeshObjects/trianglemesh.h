#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <vector>
#include <QVector3D>
#include <math.h>
#include <stdio.h>
#include <Lib/eigen3/Eigen/Geometry>
#include <unordered_map>

#include <MeshObjects/material.h>
#include <MeshObjects/triangle.h>
#include "Lib/common.h"

#include <QGLShaderProgram>
#include <memory>

#define MTLMAX 20
#define VMAX  10000
#define FMAX  10000
#define TMAX  100

/*
 * This class defines the properties of a mesh modelized as triangles.
 */
class TriangleMesh
{
public:

    std::vector<QVector3D> faceNormals;
    std::vector<QVector3D> vertices;
    std::vector<Triangle> faces;

    std::vector<vec4> replicatedVertices, replicatedNormals;
    std::vector<int> indexedFaces;

    Eigen::Vector3f min_, max_;

    TriangleMesh();
    bool hitTriangle(vec3 point, vec3 L, int triangleIdx, vec3& hitPoint);
    int hitTriangle(double pix[], double ray[], Triangle to,double Pint[]);

    void computeVertexNormals();
    void prepareDataToGPU();

    void computeBoundingBox();

    void sendMeshToGPU(std::unique_ptr<QGLShaderProgram> &program);
private:
    int solveCramer(double A[][3], double b[], double x[]);
    double det3(double A[][3]);
};

#endif // TRIANGLEMESH_H
