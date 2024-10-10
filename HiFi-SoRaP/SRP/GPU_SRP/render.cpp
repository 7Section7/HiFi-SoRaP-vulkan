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

////////////////////// TO REMOVE
#include <stdio.h>

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

unsigned char* createBitmapFileHeader (int height, int stride)
{
	int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

	static unsigned char fileHeader[] = {
		0,0,     /// signature
		0,0,0,0, /// image file size in bytes
		0,0,0,0, /// reserved
		0,0,0,0, /// start of pixel array
	};

	fileHeader[ 0] = (unsigned char)('B');
	fileHeader[ 1] = (unsigned char)('M');
	fileHeader[ 2] = (unsigned char)(fileSize      );
	fileHeader[ 3] = (unsigned char)(fileSize >>  8);
	fileHeader[ 4] = (unsigned char)(fileSize >> 16);
	fileHeader[ 5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

	return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width)
{
	static unsigned char infoHeader[] = {
		0,0,0,0, /// header size
		0,0,0,0, /// image width
		0,0,0,0, /// image height
		0,0,     /// number of color planes
		0,0,     /// bits per pixel
		0,0,0,0, /// compression
		0,0,0,0, /// image size
		0,0,0,0, /// horizontal resolution
		0,0,0,0, /// vertical resolution
		0,0,0,0, /// colors in color table
		0,0,0,0, /// important color count
	};

	infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
	infoHeader[ 4] = (unsigned char)(width      );
	infoHeader[ 5] = (unsigned char)(width >>  8);
	infoHeader[ 6] = (unsigned char)(width >> 16);
	infoHeader[ 7] = (unsigned char)(width >> 24);
	infoHeader[ 8] = (unsigned char)(height      );
	infoHeader[ 9] = (unsigned char)(height >>  8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	infoHeader[12] = (unsigned char)(1);
	infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

	return infoHeader;
}

void generateBitmapImage (unsigned char* image, int height, int width, char* imageFileName)
{
	int widthInBytes = width * BYTES_PER_PIXEL;

	unsigned char padding[3] = {0, 0, 0};
	int paddingSize = (4 - (widthInBytes) % 4) % 4;

	int stride = (widthInBytes) + paddingSize;

	FILE* imageFile = fopen(imageFileName, "wb");

	unsigned char* fileHeader = createBitmapFileHeader(height, stride);
	fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

	unsigned char* infoHeader = createBitmapInfoHeader(height, width);
	fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

	int i;
	for (i = 0; i < height; i++) {
		fwrite(image + (i*widthInBytes), BYTES_PER_PIXEL, width, imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	fclose(imageFile);
}

/////////////////////


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

	const auto now = std::chrono::system_clock::now();
	//const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
	const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	const auto fraction = now - seconds;
	const auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(fraction).count();

	// http://en.cppreference.com/w/cpp/chrono/c/time
	const std::time_t currentNow = std::time(nullptr) ; // get the current time point
	// convert it to (local) calendar time
	// http://en.cppreference.com/w/cpp/chrono/c/localtime
	const std::tm calendarTime = *std::localtime( std::addressof(currentNow) ) ;
	const auto secs      = calendarTime.tm_sec;
	const auto mins      = calendarTime.tm_min;
	const auto hours     = calendarTime.tm_hour;
	const int time_seed = millisecs + 1000*(secs + 60*(mins + 60*(hours)));

	//program->bind();
	glUniform1i(program->uniformLocation("timeSeed"), time_seed);

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

	////// TO REMOVE
/*
	unsigned char image[width][height][BYTES_PER_PIXEL];
	char* imageFileName = (char*) "output.bmp";
*/
	/////////

	//The texture contains: 0 for the pixels that were not rendered,
	// or a value between (0,1) that has to be converted to (-4,4).

	for(auto idxWidth = 0; idxWidth < width; idxWidth++) //iterating over Nx
	{
		for(auto idxHeight = 0; idxHeight < height; idxHeight++) //iterating over Ny
		{
			//The image formed by the pixels is translated. Next formula doesn't work:
			const long unsigned int idx = (idxWidth*width+idxHeight)*3; //the force is stored in 3 cells
			//const long unsigned int idx = (idxHeight*height+idxWidth)*3; //the force is stored in 3 cells

			precision::value_type fx = localForces[idx+0];
			precision::value_type fy = localForces[idx+1];
			precision::value_type fz = localForces[idx+2];

			const auto stencilValue = hits[idxWidth*width+idxHeight];

			if( stencilValue == 1 )
			{

				////// TO REMOVE
				///
/*
				unsigned int i = idxHeight, j = idxWidth;
				image[j][i][2] = (unsigned char) ( fz*255 ); ///red
				image[j][i][1] = (unsigned char) ( fy*255 ); ///green
				image[j][i][0] = (unsigned char) ( fx*255 ); ///blue
*/
				///////

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
			////// TO REMOVE
/*
			else{

				unsigned int i = idxHeight, j = idxWidth;
				image[j][i][2] = (unsigned char) ( 255 ); ///red
				image[j][i][1] = (unsigned char) ( 255 ); ///green
				image[j][i][0] = (unsigned char) ( 255 ); ///blue

			}
*/
			///////

		}
	}

	////// TO REMOVE
//  generateBitmapImage((unsigned char*) image, width, height, imageFileName);
	///////


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	computedForce=PS*apix/msat*(force);

	glEnable(GL_CULL_FACE);
}
