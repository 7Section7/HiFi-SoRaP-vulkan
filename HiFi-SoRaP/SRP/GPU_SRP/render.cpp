#include "render.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

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
	unsigned int attachments[2] = {  GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT1};//, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(1, attachments);
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	GLenum error=glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(error != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::RENDERBUFFER:: Framebuffer is not complete!" ;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_CULL_FACE);
	program->bind();

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" ;

	camera->toGPU(program);

	camera->updateProjection(-xAxis,xAxis,-yAxis,yAxis,program);

	Eigen::Matrix4f lightView = getLightView(*camera,*light,this->satellite);
	camera->updateView(lightView,program,diagonalDiff);

	light->toGPU(program);

	satellite->draw(program);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" ;

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);

	GLfloat *pixels = new GLfloat[width*height*3];

	glReadBuffer( GL_COLOR_ATTACHMENT2 );
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB16,0,0,width,height,0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, pixels);

	float forceX=0,forceY=0,forceZ=0;

	float maxValue = 1; //65535.f
	int conter =0;
	for(long int idx=0; idx <width*height*3; idx+=3){

		float fx = pixels[idx+0]/maxValue;
		float fy = pixels[idx+1]/maxValue;
		float fz = pixels[idx+2]/maxValue;

		if(fabs(fx)>1e-7 || fabs(fy)>1e-7 || fabs(fz)>1e-7){
			fx = 8*fx;
			fy = 8*fy;
			fz = 8*fz;

			fx = fx - 4;
			fy = fy - 4;
			fz = fz - 4;
			conter++;
		}
		forceX+= fx;
		forceY+= fy;
		forceZ+= fz;
	}

	QVector3D resultForce = QVector3D(forceX,forceY,forceZ);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float apix = distance/width * distance/height;
	double PS = ctt/msat;
	computedForce=PS*apix*(resultForce);

	delete[] pixels;

	glEnable(GL_CULL_FACE);
}
