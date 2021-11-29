#ifndef RAYCASTGPU_H
#define RAYCASTGPU_H

#include "MeshObjects/lineobject.h"

#include "advancedgpu.h"

class Render : public AdvancedGPU
{
public:
    Render();
    virtual void initializeBuffers();
    void draw(std::unique_ptr<QGLShaderProgram> &program,Object * satellite);

};

#endif // RAYCASTGPU_H
