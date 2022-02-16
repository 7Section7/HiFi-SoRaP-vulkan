#include "raytracegputextures.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

RayTraceGPUTextures::RayTraceGPUTextures()
{
	fragmentShaderFileGPU = "://resources/shaders/raytrace_multiple_scattering_fshader.glsl";
}

RayTraceGPUTextures::RayTraceGPUTextures(bool isSingleScattering)
{
	fragmentShaderFileGPU = "://resources/shaders/raytrace_multiple_scattering_fshader.glsl";
}

void RayTraceGPUTextures::sendTextures()
{
	//804 faces -> 1024 or 32x32    -> 1 texture 2D 32x32 rgba with vertices indices and materials indices.
	//408 vertices ->512            -> 1 texture 1D 512 rgb with vertices info
	//6 materials ->8               -> 1 texture 1D 8 rgb with ps, pd and refIdx.
								//  -> 1 texture 1D 8 r with reflectiveness.

	TriangleMesh *mesh = satellite->getMesh();

	texFacesSize = getNextPowerOfTwo(mesh->faces.size());
	texVerticesSize = getNextPowerOfTwo(mesh->vertices.size());
	texMaterialsSize = getNextPowerOfTwo(satellite->getNumMaterials());
	texNormalsSize = getNextPowerOfTwo(mesh->faceNormals.size());

	programGPU->bind();

	GLuint numFacesLocation = programGPU->uniformLocation("numTriangles");
	glUniform1i(numFacesLocation,mesh->faces.size());

	GLuint numVerticesLocation = programGPU->uniformLocation("numVertices");
	glUniform1i(numVerticesLocation,mesh->vertices.size());

	GLuint numNormalsLocation = programGPU->uniformLocation("numNormals");
	glUniform1i(numNormalsLocation,mesh->faceNormals.size());

	GLuint numMaterialsLocation = programGPU->uniformLocation("numMaterials");
	glUniform1i(numMaterialsLocation,satellite->getNumMaterials());

	GLuint facesTextureSizeLocation = programGPU->uniformLocation("facesTextureSize");
	glUniform1i(facesTextureSizeLocation,texFacesSize);

	sendFaces();
	sendVertices();
	sendNormals();
	sendMaterials();
}

void RayTraceGPUTextures::sendFaces(){

	TriangleMesh *mesh = satellite->getMesh();

	glGenTextures(1, &texFaces);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, texFaces);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float color[] = { 0.25f, 0.5f, 0.75f, 1.0f };
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float *pixels = new float[texFacesSize*4];
	for(int i=0; i<mesh->faces.size();i++){
		pixels[4*i]   = ((float)mesh->faces[i].v1) / texVerticesSize;
		pixels[4*i+1] = ((float)mesh->faces[i].v2) / texVerticesSize;
		pixels[4*i+2] = ((float)mesh->faces[i].v3) / texVerticesSize;
		pixels[4*i+3] = ((float)mesh->faces[i].rf) / texMaterialsSize;
	}
	for(int i=mesh->faces.size()*4; i<texFacesSize*4; i++)
		pixels[i] =0;

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, 32, 32, 0, GL_RGBA, GL_FLOAT, pixels);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, texFacesSize, 0, GL_RGBA, GL_FLOAT, pixels);

	glUniform1i(programGPU->uniformLocation( "texTriangles"), 0);

	delete [] pixels;
}

void RayTraceGPUTextures::sendVertices(){
	TriangleMesh *mesh = satellite->getMesh();

	glGenTextures(1, &texVertices);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, texVertices);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	float color[] = { 0.25f, 0.5f, 0.75f, 1.0f };
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float *pixels = new float[texVerticesSize*3];
	for(int i=0; i<mesh->vertices.size();i++){
		pixels[3*i  ] = 0.5f * (mesh->vertices[i].x()+1);
		pixels[3*i+1] = 0.5f * (mesh->vertices[i].y()+1);
		pixels[3*i+2] = 0.5f * (mesh->vertices[i].z()+1);
	}
	for(int i=mesh->vertices.size()*3; i<texVerticesSize*3; i++) pixels[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, texVerticesSize, 0, GL_RGB, GL_FLOAT, pixels);

	glUniform1i(programGPU->uniformLocation( "texVertices"), 1);

	delete [] pixels;
}

void RayTraceGPUTextures::sendMaterials(){

	glGenTextures(1, &texMaterials1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D, texMaterials1);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	float color[] = { 0.25f, 0.5f, 0.75f, 1.0f };
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float *pixels = new float[texMaterialsSize*3];
	int numMaterials = satellite->getNumMaterials();

	for(int i=0; i<numMaterials;i++){
		Material m = satellite->getMaterial(i);
		pixels[3*i  ] = m.ps;
		pixels[3*i+1] = m.pd;
		pixels[3*i+2] = 1.0f / m.refIdx;
	}
	for(int i=numMaterials*3; i<texMaterialsSize*3; i++) pixels[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, texMaterialsSize, 0, GL_RGB, GL_FLOAT, pixels);

	glUniform1i(programGPU->uniformLocation( "texMaterials1"), 2);

	delete [] pixels;

	glGenTextures(1, &texMaterials2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_1D, texMaterials2);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//float color[] = { 0.25f, 0.5f, 0.75f, 1.0f };
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float *pixels2 = new float[texMaterialsSize];
	for(int i=0; i<numMaterials;i++){
		Material m = satellite->getMaterial(i);
		float f=0;

		if(m.r==Reflective) f=0;
		else if(m.r == Transparent) f=0.5f;
		else if(m.r == Lambertian) f=1;

		pixels2[3*i] = f;
	}
	for(int i=numMaterials; i<texMaterialsSize; i++) pixels2[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, texMaterialsSize, 0, GL_R, GL_FLOAT, pixels2);

	glUniform1i(programGPU->uniformLocation( "texMaterials2"), 3);

	delete [] pixels2;
}

void RayTraceGPUTextures::sendNormals(){
	TriangleMesh *mesh = satellite->getMesh();

	glGenTextures(1, &texNormals);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_1D, texNormals);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	float color[] = { 0.25f, 0.5f, 0.75f, 1.0f };
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float *pixels = new float[texNormalsSize*3];
	for(int i=0; i<mesh->faceNormals.size();i++){
		pixels[3*i  ] = 0.5f * (mesh->faceNormals[i].x()+1);
		pixels[3*i+1] = 0.5f * (mesh->faceNormals[i].y()+1);
		pixels[3*i+2] = 0.5f * (mesh->faceNormals[i].z()+1);
	}
	for(int i=mesh->faceNormals.size()*3; i<texNormalsSize*3; i++) pixels[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, texNormalsSize, 0, GL_RGB, GL_FLOAT, pixels);

	glUniform1i(programGPU->uniformLocation( "texNormals"), 4);

	delete [] pixels;

	glGenTextures(1, &texFaces2);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_1D, texFaces2);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float *pixels2 = new float[texFacesSize];
	for(int i=0; i<mesh->faces.size();i++){
		pixels2[i] = ((float)mesh->faces[i].nn) / texNormalsSize;
	}
	for(int i=mesh->faces.size(); i<texFacesSize; i++) pixels2[i] =0;

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, 32, 32, 0, GL_RGBA, GL_FLOAT, pixels);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, texFacesSize, 0, GL_R, GL_FLOAT, pixels2);

	glUniform1i(programGPU->uniformLocation( "texTrianglesNormals"), 5);

	delete [] pixels2;
}

unsigned int RayTraceGPUTextures::getNextPowerOfTwo(unsigned int value)
{
	unsigned int v = value; // compute the next highest power of 2 of 32-bit v

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	v+= (v==0);

	return v;

}
