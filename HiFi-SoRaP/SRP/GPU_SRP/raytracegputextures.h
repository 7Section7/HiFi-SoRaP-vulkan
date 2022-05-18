#ifndef RAYTRACEGPUTEXTURES_H
#define RAYTRACEGPUTEXTURES_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
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
	GLuint texFaces,texFaces2, texVertices,texNormals, texMaterials1, texMaterials2;
	unsigned int texFacesSize, texVerticesSize, texNormalsSize, texMaterialsSize;

	virtual void sendFaces();
	void sendVertices();
	void sendNormals();
	void sendMaterials();
	unsigned int getNextPowerOfTwo(unsigned int value);

public:
	RayTraceGPUTextures();
	virtual void sendTextures();
};

#endif // RAYTRACEGPUTEXTURES_H
