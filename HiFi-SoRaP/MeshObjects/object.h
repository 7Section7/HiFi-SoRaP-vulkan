#ifndef OBJECT_H
#define OBJECT_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "Lib/common.h"

#include <QObject>
#include <MeshObjects/trianglemesh.h>

#include <QVulkanDeviceFunctions>


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

public:
    VkBuffer buf = VK_NULL_HANDLE;
    VkDeviceAddress bufMem = VK_NULL_HANDLE;

    VkDevice dev;
    QVulkanDeviceFunctions *devFuncs;

protected:
	TriangleMesh* mesh;

	GLuint buffer;
	GLuint vao;

	QVector3D diffuseColor;

public:
	Object();

    virtual void initializeBuffers();
	virtual void draw(std::unique_ptr<QGLShaderProgram> &program);

    virtual void initializeBuffers(VkDevice dev, QVulkanDeviceFunctions* devFuncs, uint32_t memType);
    virtual void vkdraw();

    virtual void destroyBuffers();


	void setMesh(TriangleMesh * mesh);
	void prepareMaterialsToGPU();
	Box3D computeBoundingBox();
	bool isLoaded();
	int loadOBJ(char *nameobj, char *namemtl);
	TriangleMesh* getMesh();
	Material& getMaterial(int i);
	void setReflectivenessInMaterials(Reflectiveness r);
	// In case we want to use the same refractive index for all the materials.
	void setRefractiveIndexInMaterials(const precision::value_type& refIdx);

	void sendObjectToGPU(std::unique_ptr<QGLShaderProgram> &program);

	QVector3D getDiffuseColor() const;
	void setDiffuseColor(const QVector3D &value);

	int getNumMaterials();
};

#endif // OBJECT_H
