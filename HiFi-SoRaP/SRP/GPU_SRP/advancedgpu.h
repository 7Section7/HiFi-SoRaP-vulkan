#ifndef ADVANCEDGPU_H
#define ADVANCEDGPU_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "SRP/advancedsrp.h"
#include "MeshObjects/object.h"
#include "MeshObjects/lineobject.h"
#include "GLVisualization/camera.h"
#include "GLVisualization/light.h"
#include <qeventloop.h>
#include <qtimer.h>
#include <unordered_map>
#include <unordered_set>

#include <qfile.h>

enum DebugMode{NoDebug=0,ShowOutputForces=1,ShowFaceNormals=2,ShowLightDir=3, DebugPrecision1=4, DebugPrecision2=5};

/*
 * This class is in charge of managing the communication between the CPU and GPU for the computation of the SRP method.
 */
class AdvancedGPU :  public Object,public AdvancedSRP
{
Q_OBJECT

public:
	struct ForceSRP{
		QTimer * timer;
		QEventLoop * loop;
		Light * light;
		bool isComputed=false;
		QVector3D force;
	};

protected:
	int debugMode=NoDebug;

	std::string fragmentShaderFileGPU;
	std::string fragmentShaderRenderGPU;

	unsigned int rboDepth;
	unsigned int index;

	unsigned int programId;
	GLuint gBuffer, gPosition, gNormal, gAlbedo;
	dataVisualization::Camera *camera;

	int width, height;
	Object *satellite;
	Light *light;

	QVector3D computedForce;

	std::vector<QEventLoop *>loops;
	std::vector<QTimer *>timers;

public:
	std::unique_ptr<QGLShaderProgram> programGPU;
	std::unique_ptr<QGLShaderProgram> programRenderGPU;
	bool renderNeeded;
	std::vector<unsigned int> forcesToCompute;
	std::vector<unsigned int> computedForces;

	std::unordered_map<unsigned int, ForceSRP*> forces;

	AdvancedGPU();

	static Eigen::Matrix4f getLightView(dataVisualization::Camera &camera, Light &light, Object* satellite);

	QVector3D getComputedForce() const;
	void setComputedForce(const QVector3D &value);

	// Object interface
public:
	void initializeGL(int width, int height, Object *obj);
	virtual void initializeBuffers()=0;
	virtual void draw(std::unique_ptr<QGLShaderProgram> &program, Object * satellite)=0;
	void debugDraw(std::unique_ptr<QGLShaderProgram> &program, Object * satellite);

	void showForces();
	void showNormals();
	void showNothing();

	// SRP interface
public:
	void computeStepSRP(double xs[],QVector3D &force,double RS[3]=DEFAULT_DOUBLE_ARRAY, double V1[3]=DEFAULT_DOUBLE_ARRAY, double V2[3]=DEFAULT_DOUBLE_ARRAY);

	Light *getLight() const;
	void setLight(Light *value);

	int getWidth() const;
	void setWidth(int value);

	int getHeight() const;
	void setHeight(int value);

	dataVisualization::Camera *getCamera() const;

	std::string getFragmentShaderFileGPU() const;
	void setFragmentShaderFileGPU(const std::string &value);

	std::string getFragmentShaderRenderGPU() const;

signals:
	void render();
};

#endif // RAYTRACINGGPU_H
