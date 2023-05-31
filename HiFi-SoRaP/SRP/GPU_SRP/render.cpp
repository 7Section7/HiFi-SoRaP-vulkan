#include "render.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include<QElapsedTimer>
#include<iostream>
#include<fstream>
#include <iomanip>

#include <cstdint> // for specific size integers
//#include <fstream> // for file handling
using namespace std;


Render::Render()
{
}

void Render::initializeBuffers()
{
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer); //We select this framebuffer.

	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
	unsigned int attachments[2] = {GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT1};
	glDrawBuffers(1, attachments);
	// create and attach depth & stencil buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	GLenum error=glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(error != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::RENDERBUFFER:: Framebuffer is not complete!" ;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	localForces.reserve(width*height*3);
	hits.reserve(width*height);
}

void Render::draw(std::unique_ptr<QGLShaderProgram> &program,Object * satellite)
{
	TriangleMesh *mesh = this->satellite->getMesh();
	Eigen::Vector3f diff = mesh->max_-mesh->min_;
	float diagonalDiff = diff.norm();
	float errorMargin = 0.1f;
	float distance = diagonalDiff+errorMargin;

	float xAxis = distance/2.0f;
	float yAxis = distance/2.0f;

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_CULL_FACE);


	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	program->bind();

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";

	GLuint nxLocation = program->uniformLocation("Nx");
	glUniform1i(nxLocation,width);

	GLuint xtotLocation = program->uniformLocation("xtot");
	glUniform1f(xtotLocation,distance);

	GLuint nyLocation = program->uniformLocation("Ny");
	glUniform1i(nyLocation,height);

	GLuint ytotLocation = program->uniformLocation("ytot");
	glUniform1f(ytotLocation,distance);

	GLuint distanceLocation = program->uniformLocation("boundingBoxDistance");
	glUniform1f(distanceLocation,diagonalDiff);

	camera->toGPU(program);

	camera->updateProjection(-xAxis,xAxis,-yAxis,yAxis,program);

	Eigen::Matrix4f lightView = getLightView(*camera,*light,this->satellite);
	camera->updateView(lightView,program,diagonalDiff);

	light->toGPU(program);

	satellite->draw(program);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" ;

	// Reading local forces from framebuffer

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);

	glReadBuffer( GL_COLOR_ATTACHMENT2 );
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB16,0,0,width,height,0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, localForces.data());

	// Reading hits from stencil buffer

	glReadBuffer( GL_STENCIL_ATTACHMENT );
	glCopyTexImage2D(GL_FRAMEBUFFER,0,GL_STENCIL_INDEX8,0,0,width,height,0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glReadPixels(0, 0, width, height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, hits.data());

	vector3 force;
	float apix = distance/width * distance/height;
	double PS = PRESSURE;

	vector3 compensationTerm{0.L};
	vector3 previousForce;

	//The texture contains: 0 for the pixels that were not rendered,
	// or a value between (0,1) that has to be converted to (-4,4).

	for(auto idxHeight = 0; idxHeight < height; idxHeight++) //iterating over Nx
	{
		for(auto idxWidth = 0; idxWidth < width; idxWidth++) //iterating over Ny
		{
			const long unsigned int idx = (idxWidth*width+idxHeight)*3; //the force is stored in 3 cells
			precision::value_type fx = localForces[idx+0];
			precision::value_type fy = localForces[idx+1];
			precision::value_type fz = localForces[idx+2];

			const auto stencilValue = hits[idxWidth*width+idxHeight];

			if( stencilValue == 1 )
			{
				fx = 8.f*fx - 4.f;
				fy = 8.f*fy - 4.f;
				fz = 8.f*fz - 4.f;

				const auto fixedForce = vector3(fx,fy,fz) - compensationTerm;
				const auto accumulatedFixedForce = force + fixedForce;
				compensationTerm = (accumulatedFixedForce - force) - fixedForce;
				force = accumulatedFixedForce;

				previousForce = force;

				//Previously:
				//force += vector3(fx,fy,fz);
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	computedForce=PS*apix/msat*(force);

	glEnable(GL_CULL_FACE);
}
