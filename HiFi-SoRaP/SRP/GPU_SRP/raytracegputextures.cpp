#include "raytracegputextures.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

namespace
{
	unsigned int getNextPowerOfTwo(unsigned int value);

	static float color[] = { 0.25f, 0.5f, 0.75f, 1.0f };
}


RayTraceGPUTextures::RayTraceGPUTextures()
{
	//fragmentShaderFileGPU = "://resources/shaders/raytrace_multiple_scattering_fshader.glsl";
	fragmentShaderFileGPU = "://resources/shaders/raytrace_improved_fshader.glsl";
	//fragmentShaderFileGPU   = "://resources/shaders/raytrace_rand_xorshift_fshader.glsl";
}

void RayTraceGPUTextures::sendTextures()
{
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
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	facesTexture.reserve(texFacesSize*4);
	for(uint i=0; i<mesh->faces.size();i++){
		facesTexture[4*i]   = ((float)mesh->faces[i].v1) / texVerticesSize;
		facesTexture[4*i+1] = ((float)mesh->faces[i].v2) / texVerticesSize;
		facesTexture[4*i+2] = ((float)mesh->faces[i].v3) / texVerticesSize;
		facesTexture[4*i+3] = ((float)mesh->faces[i].rf) / texMaterialsSize;
	}
	for(uint i=mesh->faces.size()*4; i<texFacesSize*4; i++)
		facesTexture[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, texFacesSize, 0, GL_RGBA, GL_FLOAT, facesTexture.data());

	glUniform1i(programGPU->uniformLocation( "texTriangles"), 0);
}

void RayTraceGPUTextures::sendVertices(){
	TriangleMesh *mesh = satellite->getMesh();

	glGenTextures(1, &texVertices);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, texVertices);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	const auto extremeValues = std::vector<float>{mesh->max_.x(),mesh->max_.y(),mesh->max_.z(),
			mesh->min_.x(),mesh->min_.y(),mesh->min_.z()};
	const auto minPos = std::min_element(extremeValues.begin(),extremeValues.end());
	const auto maxPos = std::max_element(extremeValues.begin(),extremeValues.end());

	GLuint minVertexComponentLocation = programGPU->uniformLocation("minVertexComponent");
	glUniform1f(minVertexComponentLocation,*minPos);

	GLuint maxVertexComponentLocation = programGPU->uniformLocation("maxVertexComponent");
	glUniform1f(maxVertexComponentLocation,*maxPos);

	verticesTexture.reserve(texVerticesSize*3);
	for(uint i=0; i<mesh->vertices.size();i++)
	{
		verticesTexture[3*i  ] = (mesh->vertices[i].x - *minPos)/(*maxPos - *minPos);
		verticesTexture[3*i+1] = (mesh->vertices[i].y - *minPos)/(*maxPos - *minPos);
		verticesTexture[3*i+2] = (mesh->vertices[i].z - *minPos)/(*maxPos - *minPos);
	}
	for(uint i=mesh->vertices.size()*3; i<texVerticesSize*3; i++) verticesTexture[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, texVerticesSize, 0, GL_RGB, GL_FLOAT, verticesTexture.data());

	glUniform1i(programGPU->uniformLocation( "texVertices"), 1);
}

void RayTraceGPUTextures::sendMaterials(){

	glGenTextures(1, &texMaterials1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D, texMaterials1);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	reflectivitiesTexture.reserve(texMaterialsSize*3);
	uint numMaterials = satellite->getNumMaterials();

	for(uint i=0; i<numMaterials;i++){
		Material m = satellite->getMaterial(i);
		reflectivitiesTexture[3*i  ] = m.ps;
		reflectivitiesTexture[3*i+1] = m.pd;
		reflectivitiesTexture[3*i+2] = 1.0f / m.refIdx;
	}
	for(uint i=numMaterials*3; i<texMaterialsSize*3; i++) reflectivitiesTexture[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, texMaterialsSize, 0, GL_RGB, GL_FLOAT, reflectivitiesTexture.data());

	glUniform1i(programGPU->uniformLocation( "texMaterials1"), 2);

	glGenTextures(1, &texMaterials2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_1D, texMaterials2);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	materialTypesTexture.reserve(texMaterialsSize);
	for(uint i=0; i<numMaterials;i++){
		Material m = satellite->getMaterial(i);
		float f=0;

		if(m.r==Reflective) f=0;
		else if(m.r == Transparent) f=0.5f;
		else if(m.r == Lambertian) f=1;

		materialTypesTexture[3*i] = f;
	}
	for(uint i=numMaterials; i<texMaterialsSize; i++) materialTypesTexture[i] = 0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, texMaterialsSize, 0, GL_R, GL_FLOAT, materialTypesTexture.data());

	glUniform1i(programGPU->uniformLocation( "texMaterials2"), 3);
}

void RayTraceGPUTextures::sendNormals(){
	TriangleMesh *mesh = satellite->getMesh();

	glGenTextures(1, &texNormals);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_1D, texNormals);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	normalsTexture.reserve(texNormalsSize*3);
	for(uint i=0; i<mesh->faceNormals.size();i++){
		normalsTexture[3*i  ] = 0.5f * (mesh->faceNormals[i].x+1);
		normalsTexture[3*i+1] = 0.5f * (mesh->faceNormals[i].y+1);
		normalsTexture[3*i+2] = 0.5f * (mesh->faceNormals[i].z+1);
	}
	for(uint i=mesh->faceNormals.size()*3; i<texNormalsSize*3; i++) normalsTexture[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, texNormalsSize, 0, GL_RGB, GL_FLOAT, normalsTexture.data());

	glUniform1i(programGPU->uniformLocation( "texNormals"), 4);

	glGenTextures(1, &texFaces2);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_1D, texFaces2);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_1D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	faceNormalsTexture.reserve(texFacesSize);
	for(uint i=0; i<mesh->faces.size();i++){
		faceNormalsTexture[i] = ((float)mesh->faces[i].nn) / texNormalsSize;
	}
	for(uint i=mesh->faces.size(); i<texFacesSize; i++) faceNormalsTexture[i] =0;

	glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, texFacesSize, 0, GL_R, GL_FLOAT, faceNormalsTexture.data());

	glUniform1i(programGPU->uniformLocation( "texTrianglesNormals"), 5);
}

namespace
{

unsigned int getNextPowerOfTwo(unsigned int value)
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

}
