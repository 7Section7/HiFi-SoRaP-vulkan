#ifndef RAYTRACE_H
#define RAYTRACE_H

#include "MeshObjects/object.h"
//#include <QOpenGLTexture>
#include "SRP/GPU_SRP/advancedgpu.h"
class RayTraceGPU
{
    int numSecondaryRays;
    int diffuseRays;
    int reflectionType;

public:
    RayTraceGPU();
    virtual ~RayTraceGPU()=0;

    void setNumSecondaryRays(std::unique_ptr<QGLShaderProgram> &program);
    int getNumSecondaryRays() const;
    void setNumSecondaryRays(int value);

    void setNoiseTexture(int textureId,std::unique_ptr<QGLShaderProgram> &program);

    void setDiffuseRays(std::unique_ptr<QGLShaderProgram> &program);
    int getDiffuseRays() const;
    void setDiffuseRays(int value);

    void setReflectionType(std::unique_ptr<QGLShaderProgram> &program);
    int getReflectionType() const;
    void setReflectionType(int value);
};

#endif // RAYTRACE_H
