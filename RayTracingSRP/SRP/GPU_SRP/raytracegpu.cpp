#include "raytracegpu.h"

RayTraceGPU::RayTraceGPU()
{
    reflectionType=Reflective;
}

RayTraceGPU::~RayTraceGPU()
{
}

void RayTraceGPU::setNoiseTexture(int textureId,std::unique_ptr<QGLShaderProgram> &program)
{
    glEnable(GL_TEXTURE_3D);

    program->bind();
    GLuint texNoise;

    glGenTextures(1, &texNoise);
    glActiveTexture(GL_TEXTURE0+textureId);
    glBindTexture(GL_TEXTURE_1D, texNoise);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    float color[] = { 0.25f, 0.5f, 0.75f, 1.0f };
    glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    QImage image = QImage("://resources/images/noiseTexture.png");
    int w = image.width();
    int h = image.height();

    int index=0;
    float *pixels = new float[256*3];
    for(int i=0; i<w && index<256;i++){
        for(int j=1; j<h && index<256; j+=2){
            //index = (i*h/2+(j-1)/2);
            QColor pixel = QColor(image.pixel(i,j));
            float r,g,b;
            r= pixel.redF(); g = pixel.greenF(); b = pixel.blueF();

            pixels[3*index]   = r;
            pixels[3*index+1] = g;
            pixels[3*index+2] = b;
            index++;
        }
    }

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, 256, 0, GL_RGB, GL_FLOAT, pixels);

    glUniform1i(program->uniformLocation( "texNoise"), textureId);
    glUniform1i(program->uniformLocation( "numNoiseValues"), 256);

    delete [] pixels;
}

void RayTraceGPU::setDiffuseRays(std::unique_ptr<QGLShaderProgram> &program)
{
    program->bind();
    GLuint idNumDiffuseRays = program->uniformLocation("numDiffuseRays");
    glUniform1i(idNumDiffuseRays,diffuseRays);

}

int RayTraceGPU::getDiffuseRays() const
{
    return diffuseRays;
}

void RayTraceGPU::setDiffuseRays(int value)
{
    diffuseRays = value;
}

void RayTraceGPU::setReflectionType(std::unique_ptr<QGLShaderProgram> &program)
{
    program->bind();
    GLuint idReflectionType = program->uniformLocation("reflectionType");
    glUniform1i(idReflectionType,reflectionType);
}

int RayTraceGPU::getReflectionType() const
{
    return reflectionType;
}

void RayTraceGPU::setReflectionType(int value)
{
    reflectionType = value;
}

void RayTraceGPU::setNumSecondaryRays( std::unique_ptr<QGLShaderProgram> &program)
{
    program->bind();
    GLuint idNumSecondaryRays = program->uniformLocation("numSecondaryRays");
    glUniform1i(idNumSecondaryRays,numSecondaryRays);

}
int RayTraceGPU::getNumSecondaryRays() const
{
    return numSecondaryRays;
}

void RayTraceGPU::setNumSecondaryRays(int value)
{
    numSecondaryRays = value;
}
