#ifndef RAYTRACEGPUTEXTURES_H
#define RAYTRACEGPUTEXTURES_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "render.h"
#include "raytracegpu.h"

/*
 * This class is in charge of sending the information of a spacecraft (mesh & materials) to the GPU.
 */
class RayTraceGPUTextures: public Render, public RayTraceGPU
{
protected:
	virtual void sendFaces();
	void sendVertices();
	void sendNormals();
	void sendMaterials();

public:
	RayTraceGPUTextures();
	virtual void sendTextures();

private:
	GLuint texFaces,texFaces2, texVertices,texNormals, texMaterials1, texMaterials2;
	unsigned int texFacesSize, texVerticesSize, texNormalsSize, texMaterialsSize;

	std::vector<float> facesTexture, verticesTexture;
	std::vector<float> reflectivitiesTexture, materialTypesTexture;
	std::vector<float> normalsTexture, faceNormalsTexture;
};

#endif // RAYTRACEGPUTEXTURES_H
