#ifndef GLWIDGET_H
#define GLWIDGET_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <QGLShaderProgram>

#include <QtWidgets>
#include <QGLWidget>
#include <QMouseEvent>

#include <fstream>
#include <iostream>
#include <memory>
#include <QString>

#include "camera.h"
#include "SRP/GPU_SRP/advancedgpu.h"
#include "MeshObjects/object.h"
#include "MeshObjects/lineobject.h"
#include "MeshObjects/quad.h"
#include "light.h"
#include "SRP/GPU_SRP/raytracegpu.h"
#include "SRP/GPU_SRP/raytracegputextures.h"

class QGLShaderProgram;

enum RenderMode{CPU=0, GPU=1};

/*
 * This class does the GPU rendering. Not only for visualizing the satellite, but also for the computation
 * of srp methods.
 */
class GLWidget : public QGLWidget
{
	Q_OBJECT

	struct LineForceSRP{
		LineObject* line;
		vector3 force;
	};

	float minForce, maxForce;
	uint previousNumForces;
	AdvancedGPU *advancedGPU;

	Object *satellite, *Sun, *cube;
	Quad * quad;
	LineObject* SunLine, *SunLine2, *xAxis, *yAxis, *zAxis;
	std::vector<LineForceSRP> lineForces;

	RenderMode renderMode;

    Eigen::Matrix<float,4,4,Eigen::DontAlign> satelliteRotation;
	float previousAxisX,previousAxisY,previousAxisZ;
	bool firstRotation;

	Light light;

	bool initializedGL,initializedBuffers;

	std::unique_ptr<QGLShaderProgram> program;

	QLabel *minForceValue,*maxForceValue;

	/**
	 * @brief camera_ Class that computes the multiple camera transform matrices.
	 */
	dataVisualization::Camera camera;
	QPoint lastPos;
	/**
	 * @brief width_ Viewport current width.
	 */
	float width;

	/**
	 * @brief height_ Viewport current height.
	 */
	float height;

	std::string vertexShaderFile;
	std::string fragmentShaderFile;
	int vertexAttributeIdx;
	int normalAttributeIdx;
	int pdAttributeIdx;
	int psAttributeIdx;

	bool showSatellite;

	void init();

	bool readFile(const std::string filename, std::string *shaderSource);

	/**
	 * @brief initializeGL Initializes OpenGL variables and loads, compiles and
	 * links shaders.
	 */
	void initializeGL();
	void initShaders(const char* vShaderFile, const char* fShaderFile, std::unique_ptr<QGLShaderProgram>& program);
	void initializeShaders(std::string kVertexShaderFile, std::string kFragmentShaderFile,
			std::unique_ptr<QGLShaderProgram>& program);

	void paintGL();

	LineObject* createLineObject(const vector3& initPos, const vector3& endPos);
	LineObject* createBigLineObject(const vector3& initPos, const vector3& endPos, const vector3& axis1, const vector3& axis2);
	void updateBigLineObject(TriangleMesh* line, const vector3& initPos, const vector3& endPos, const vector3& axis1,
			const vector3& axis2);

protected:
	/**
	* @brief resizeGL Resizes the viewport.
	* @param w New viewport width.
	* @param h New viewport height.
	*/
	void resizeGL(int w, int h);

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

	void qNormalizeAngle(double &angle);

public:
	GLWidget(QWidget *parent);
	GLWidget(const QGLFormat &glf, QWidget *parent=0);
	~GLWidget();

	void initializeBuffers();
	void setSatellite(Object *obj);

	void addNewForceSRP(const vector3& dir);
	void clearLineForces();

	void rotateSatellite(float angleX,float angleY,float angleZ);

	const vector3& getLightDir() const;
	void setLightDir(const vector3& value);
	AdvancedGPU *getRayTraceGPU() const;
	void setRayTraceGPU(AdvancedGPU *value);

	bool getShowSatellite() const;
	void setShowSatellite(bool value);

	void sendSatelliteToGPU();

	void setNumSecondaryRays();

	void setLabels(QLabel* minValue, QLabel* maxValue);

	Eigen::Matrix4f getSatelliteRotation() const;

signals:
	void finishedForceComputation();
public slots:
	void renderGL();
};
#endif // GLWIDGET_H
