#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <MeshObjects/material.h>
#include <MeshObjects/triangle.h>

#include "Lib/common.h"
#undef GL_GLEXT_VERSION // Lib/common and QGLShaderProgram both defines this macro.
//We do this to avoid redefinition warning.
#include <QGLShaderProgram>

MODIFY_WARNINGS( ignored )
	#include <Lib/eigen3/Eigen/Geometry>
MODIFY_WARNINGS( warning )

#include <unordered_map>
#include <vector>
#include <QVector3D>
#include <math.h>
#include <stdio.h>

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
	std::vector<vector3> faceNormals;
	std::vector<vector3> vertices;
	std::vector<Triangle> faces;

	//For now, we pass the info of the object to the GPU as floats.
	std::vector<vector4> replicatedVertices, replicatedNormals;
	std::vector<int> indexedFaces;

	Eigen::Vector3f min_, max_;

	TriangleMesh();
	bool hitTriangle(const vector3& point, const vector3& L, const int triangleIdx, vector3& hitPoint);

	void computeVertexNormals();
	void prepareDataToGPU();

	void computeBoundingBox();

	void sendMeshToGPU(std::unique_ptr<QGLShaderProgram> &program);

    std::vector<Triangle> getTriangles();
};

#endif // TRIANGLEMESH_H
