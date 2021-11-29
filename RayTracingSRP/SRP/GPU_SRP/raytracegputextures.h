#ifndef RAYTRACEGPUTEXTURES_H
#define RAYTRACEGPUTEXTURES_H

#include "render.h"
#include "raytracegpu.h"

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
    RayTraceGPUTextures(bool isSingleScattering);
    virtual void sendTextures();
};

#endif // RAYTRACEGPUTEXTURES_H
