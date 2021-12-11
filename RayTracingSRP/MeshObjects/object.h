#ifndef OBJECT_H
#define OBJECT_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "Lib/common.h"

#include <QObject>
#include <MeshObjects/trianglemesh.h>

#include <algorithm>

/*
 * Basic class to send the information of an object (satellite) to the gpu.
 */
class Object: public QObject
{
	Q_OBJECT

	std::vector<Material> materials;
	std::vector<float> replicatedMaterialsPS,replicatedMaterialsPD;
	std::vector<float> replicatedMaterialsRefIdx;
	std::vector<int> replicatedMaterialsReflectiveness;

	void sendMaterialsToGPU(std::unique_ptr<QGLShaderProgram> &program);

protected:
	TriangleMesh* mesh;

	GLuint buffer;
	GLuint vao;

	QVector3D diffuseColor;

public:
	Object();
	virtual void initializeBuffers();
	virtual void draw(std::unique_ptr<QGLShaderProgram> &program);
	void setMesh(TriangleMesh * mesh);
	void prepareMaterialsToGPU();
	Box3D computeBoundingBox();
	bool isLoaded();
	int loadOBJ(char *nameobj, char *namemtl);
	TriangleMesh* getMesh();
	Material& getMaterial(int i);
	void setReflectivenessInMaterials(Reflectiveness r);
	void setRefractiveIndexInMaterials(float refIdx);

	void sendObjectToGPU(std::unique_ptr<QGLShaderProgram> &program);

	QVector3D getDiffuseColor() const;
	void setDiffuseColor(const QVector3D &value);

	int getNumMaterials();
};

#endif // OBJECT_H
