#ifndef RAYCASTGPU_H
#define RAYCASTGPU_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

#include "MeshObjects/lineobject.h"

#include "advancedgpu.h"

/*
 * This class is in charge of the rendering of the SRP method on the GPU. It sends the information needed to perform the
 * method to the GPU memory and activate the process on the GPU to perform the computation of the SRP force. Later on,
 * it retrives the computed forces from the GPU.
 */
class Render : public AdvancedGPU
{
public:
	Render();
	virtual void initializeBuffers();
	void draw(std::unique_ptr<QGLShaderProgram> &program,Object * satellite);
};

#endif // RAYCASTGPU_H
