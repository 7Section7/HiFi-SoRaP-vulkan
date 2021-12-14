#include "advancedgpu.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

#include <QCoreApplication>

Light *AdvancedGPU::getLight() const
{
	return light;
}

void AdvancedGPU::setLight(Light *value)
{
	light = value;
}

int AdvancedGPU::getWidth() const
{
	return width;
}

void AdvancedGPU::setWidth(int value)
{
	width = value;
}

int AdvancedGPU::getHeight() const
{
	return height;
}

void AdvancedGPU::setHeight(int value)
{
	height = value;
}

dataVisualization::Camera *AdvancedGPU::getCamera() const
{
	return camera;
}

std::string AdvancedGPU::getFragmentShaderFileGPU() const
{
	return fragmentShaderFileGPU;
}

void AdvancedGPU::setFragmentShaderFileGPU(const std::string &value)
{
	fragmentShaderFileGPU = value;
}

void AdvancedGPU::debugDraw(std::unique_ptr<QGLShaderProgram> &program, Object * satellite)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(debugMode == NoDebug) return;

#ifdef CULL_FRONT_FACES
	glCullFace(GL_FRONT);
	//satellite->invertNormals(program,true);
//#else
  //  satellite->invertNormals(program,false);
#endif

	program->bind();
	GLuint debugModeLocation = program->uniformLocation("debugMode");
	glUniform1i(debugModeLocation,debugMode);

	camera->toGPU(program);

	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	GLuint gAlbedoLocation = program->uniformLocation("gAlbedo");
	glUniform1i(gAlbedoLocation,11);

	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	GLuint gNormalLocation = program->uniformLocation("gNormal");
	glUniform1i(gNormalLocation,12);

	satellite->draw(program);

#ifdef SHOW_LIGHT_AXIS
	Eigen::Vector3f center = (satellite->getMesh()->max_+satellite->getMesh()->min_)/2.0f;
	QVector3D centerPos =QVector3D(center[0],center[1],center[2]);
	QVector3D initDir = centerPos;//-4*light->getRightDir();
	QVector3D endDir = centerPos +4*light->getRightDir();

	LineObject* line = new LineObject();
	TriangleMesh *mesh2 = new TriangleMesh();
	mesh2->replicatedVertices.clear();
	mesh2->replicatedVertices.push_back(vec4(initDir.x(),initDir.y(),initDir.z(),1));
	mesh2->replicatedVertices.push_back(vec4(endDir.x(),endDir.y(),endDir.z(),1));

	line->setMesh(mesh2);

	line->initializeBuffers();
	line->draw(program);

	line = new LineObject();
	mesh2 = new TriangleMesh();
	initDir = centerPos;//-4*light->getUpDir();
	endDir = centerPos +4*light->getUpDir();
	mesh2->replicatedVertices.clear();
	mesh2->replicatedVertices.push_back(vec4(initDir.x(),initDir.y(),initDir.z(),1));
	mesh2->replicatedVertices.push_back(vec4(endDir.x(),endDir.y(),endDir.z(),1));

	line->setMesh(mesh2);

	line->initializeBuffers();
	line->draw(program);
#endif

	glCullFace(GL_BACK);
	glUniform1i(debugModeLocation,NoDebug);
}

void AdvancedGPU::showForces()
{
	debugMode = ShowOutputForces;
}

void AdvancedGPU::showNormals()
{
	debugMode = ShowFaceNormals;
}

void AdvancedGPU::showNothing()
{
	debugMode = NoDebug;
}

std::string AdvancedGPU::getFragmentShaderRenderGPU() const
{
	return fragmentShaderRenderGPU;
}

AdvancedGPU::AdvancedGPU()
{
	renderNeeded=false;
	index =0;

	fragmentShaderFileGPU = "://resources/shaders/zbuffer_fshader.glsl";
	fragmentShaderRenderGPU = "://resources/shaders/render_fshader.glsl";
}

void AdvancedGPU::initializeGL(int width, int height,Object *obj) //, dataVisualization::Camera *camera
{
	this->width=width;
	this->height=height;
	satellite=obj;
	this->camera = new dataVisualization::Camera();//camera;
}

void AdvancedGPU::computeStepSRP(double xs[], QVector3D &force, double RS[], double V1[], double V2[])
{
	ForceSRP f;
	QEventLoop *loop = new QEventLoop();
	QTimer *timer = new QTimer();
	Light *light = new Light();
	light->setLightDir(QVector3D(xs[0],xs[1],xs[2]));
	light->setRightDir(QVector3D(V1[0],V1[1],V1[2]));
	light->setUpDir(   QVector3D(V2[0],V2[1],V2[2]));

	f.loop=loop;
	f.timer = timer;
	f.isComputed=false;
	f.force = QVector3D(0,0,0);
	f.light = light;

	forces[index] = &f;
	forcesToCompute.push_back(index);

	index++;

	timer->setSingleShot(true);

	connect(timer, SIGNAL(timeout()), loop, SLOT(quit()),Qt::QueuedConnection);

	timer->start(10000);
	emit render();
	loop->exec();

	if(f.isComputed){
		force=f.force;
	}
	else{
		qDebug("timeout");
	}
}

Eigen::Matrix4f AdvancedGPU::getLightView(dataVisualization::Camera &camera, Light &light, Object* satellite)
{
	QVector3D lightDir = light.getLightDir();
	QVector3D upDir = light.getUpDir();

	TriangleMesh *mesh = satellite->getMesh();
	Eigen::Vector3f center = (mesh->max_+mesh->min_)/2.0f;

	Eigen::Vector3f eye(center.x() - lightDir.x(),center.y() - lightDir.y(),center.z() - lightDir.z());
	Eigen::Vector3f up(upDir.x(),upDir.y(),upDir.z());

	return camera.lookAt(eye,center,up);
}

QVector3D AdvancedGPU::getComputedForce() const
{
	return computedForce;
}

void AdvancedGPU::setComputedForce(const QVector3D &value)
{
	computedForce = value;
}
