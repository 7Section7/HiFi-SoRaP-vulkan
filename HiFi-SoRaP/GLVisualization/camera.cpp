#include "camera.h"

//#include <GL/glew.h>

#define GLM_FORCE_RADIANS

//QT_WARNING_PUSH
//QT_WARNING_DISABLE_GCC("-Wexpansion-to-defined")

//MODIFY_WARNINGS( ignored )
#include "Lib/glm/gtc/matrix_transform.hpp"
//MODIFY_WARNINGS( warning )

//QT_WARNING_POP

#include <algorithm>
#include <cmath>

namespace dataVisualization {

namespace {

const Eigen::Vector3f vra(0.0, 1.0, 0.0);
const Eigen::Vector3f hra(1.0, 0.0, 0.0);

}  //  namespace

Camera::Camera()
	: distance(2.0f), //2.0f
	  step(0.05f),
	  rotating(false),
	  zooming(false),
	  panning(false),
	  rotationY(0.0f),
	  rotationX(0.0f),
	  panX(0.0f),
	  panY(0.0f),
	  currentX(-1.0f),
	  currentY(-1.0f),
	  viewportX(0.0f),
	  viewportY(0.0f),
	  viewportWidth(0.0f),
	  viewportHeight(0.0f),
	  centeringX(0.0f),
	  centeringY(0.0f),
	  centeringZ(0.0f),
	  scaling(1.0f),
	  fieldOfView(0.0f),
	  zNear(0.0f),
	  zFar(0.0f) {}

Eigen::Matrix4f Camera::lookAt(Eigen::Vector3f const & eye, Eigen::Vector3f const & center, Eigen::Vector3f const & up )
{
	Eigen::Vector3f f = (center - eye).normalized();
	Eigen::Vector3f u = up.normalized();
	Eigen::Vector3f s = f.cross(u).normalized();
	u = s.cross(f);

	Eigen::Matrix4f res;
	res <<  s.x(),s.y(),s.z(),-s.dot(eye),
			u.x(),u.y(),u.z(),-u.dot(eye),
			-f.x(),-f.y(),-f.z(),f.dot(eye),
			0,0,0,1;

	return res;
}

Eigen::Matrix4f Camera::getProjection(double ang, double vp, double zn, double zf) {
	glm::dmat4x4 glmPerspective = glm::perspective((ang * M_PI / 180.0), vp,  zn, zf);

	Eigen::Matrix4f eigen_perspective;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) eigen_perspective(i, j) = glmPerspective[j][i];

	return eigen_perspective;
}

Eigen::Matrix4f Camera::getOrthoProjection(double left, double right, double bottom, double top, double zn, double zf){
	glm::dmat4x4 glmOrtho = glm::ortho(left,right,bottom,top, zn, zf);

	Eigen::Matrix4f eigen_ortho;
	for (int i = 0; i < 4; ++i)
	  for (int j = 0; j < 4; ++j) eigen_ortho(i, j) = glmOrtho[j][i];

	return eigen_ortho;
}

void Camera::setViewport(double x, double y, double w, double h) {
	viewportX = x;
	viewportY = y;
	viewportWidth = w;
	viewportHeight = h;

    //glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
}

void Camera::setViewport() const {
    //glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
}

Eigen::Matrix4f Camera::setIdentity() const {
	Eigen::Matrix4f identity;
	identity << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	return identity;
}

Eigen::Matrix4f Camera::setModel() const {
	const Eigen::Affine3f kScaling(
		Eigen::Scaling(Eigen::Vector3f(scaling, scaling, scaling)));
	const Eigen::Affine3f kTranslation(Eigen::Translation3f(
		Eigen::Vector3f(centeringX, centeringY, centeringZ)));

	return kScaling.matrix() * kTranslation.matrix();
}

Eigen::Matrix4f Camera::setView() const {
	const Eigen::Affine3f kTranslation(
		Eigen::Translation3f(Eigen::Vector3f(panX, panY, -distance)));
	const Eigen::Affine3f kRotationA(Eigen::AngleAxisf(rotationX, hra));
	const Eigen::Affine3f kRotationB(Eigen::AngleAxisf(rotationY, vra));

	return kTranslation.matrix() * kRotationA.matrix() * kRotationB.matrix();
}

Eigen::Matrix4f Camera::setProjection(double fov, double znear, double zfar) {
	fieldOfView = fov;
	zNear = znear;
	zFar = zfar;

	return setProjection();
}

Eigen::Matrix4f Camera::setProjection() const {
	glm::dmat4x4 glmPerspective = glm::perspective((fieldOfView * M_PI / 180.0),
					   (viewportWidth / viewportHeight), zNear, zFar);

	Eigen::Matrix4f eigenPerspective;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) eigenPerspective(i, j) = glmPerspective[j][i];

	return eigenPerspective;
}

void Camera::zoom(double modifier) {
	distance += step * modifier;

	if (distance < kMinCameraDistance) distance = kMinCameraDistance;
	if (distance > kMaxCameraDistance) distance = kMaxCameraDistance;
}

void Camera::safeZoom(double modifier) {
	if (zooming) {
		zoom(modifier - currentY < 0 ? -1 : 1);
		currentY = modifier;
	}
}

void Camera::safePan(double x, double y) {
	if (panning) {
		panX += (x - currentX) / 10.0f * step;
		panY -= (y - currentY) / 10.0f * step;
		currentY = y;
		currentX = x;
	}
}

void Camera::rotate(double modifier) {
	rotationY += kAngleIncrement * modifier;
}

void Camera::updateModel(Eigen::Vector3f min, Eigen::Vector3f max) {
	Eigen::Vector3f center = (min + max) / 2.0;
	centeringX = -center[0];
	centeringY = -center[1];
	centeringZ = -center[2];

	double longestEdge =
			std::max(max[0] - min[0], std::max(max[1] - min[1], max[2] - min[2]));
	scaling = 1.0 / longestEdge;
}

void Camera::setRotationX(double y) {
	if (rotating) {
		rotationX += (y - currentY) * step;
		currentY = y;
		if (rotationX < kMinRotationX) rotationX = kMinRotationX;
		if (rotationX > kMaxRotationX) rotationX = kMaxRotationX;
	}
}

void Camera::setRotationY(double x) {
	if (rotating) {
		rotationY += (x - currentX) * step;
		currentX = x;
	}
}

void Camera::startRotating(double x, double y) {
	currentX = x;
	currentY = y;
	rotating = true;
}

void Camera::stopRotating(double x, double y) {
	currentX = x;
	currentY = y;
	rotating = false;
}

void Camera::startZooming(double x, double y) {
	currentX = x;
	currentY = y;
	zooming = true;
}

void Camera::stopZooming(double x, double y) {
	currentX = x;
	currentY = y;
	zooming = false;
}

void Camera::startPanning(double x, double y) {
	currentX = x;
	currentY = y;
	panning = true;
}

void Camera::stopPanning(double x, double y) {
	currentX = x;
	currentY = y;
	panning = false;
}

void Camera::setCameraStep(double step) {
	this->step = step;
}


Eigen::Matrix4f Camera::getMVP() {
    if(abs(zNear - zFar) < 0.2){
        zNear = 0.0001;
        zFar = 800;
        fieldOfView = 60;
    }

    // setViewport();

    Eigen::Matrix4f projection = setProjection();
    Eigen::Matrix4f view = setView();
    Eigen::Matrix4f model = setModel();

    Eigen::Matrix4f mvp = projection * view * model;

    return mvp;
}

void Camera::toGPU(std::unique_ptr<QGLShaderProgram> &program)
{
	if(abs(zNear - zFar) < 0.2){
		zNear = 0.0001;
		zFar = 800;
		fieldOfView = 60;
	}

    // setViewport();

	Eigen::Matrix4f projection = setProjection();
	Eigen::Matrix4f view = setView();
	Eigen::Matrix4f model = setModel();

	Eigen::Matrix4f t = view * model;
	Eigen::Matrix3f normal;
	for (int i = 0; i < 3; ++i)
	  for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

	normal = normal.inverse().transpose();

	Eigen::Vector4f camPos = view.inverse().col(3);

    /*
	GLuint projectionLocation, viewLocation, modelLocation,
		normalMatrixLocation, camPositionLocation;

	program->bind();

	projectionLocation =program->uniformLocation("projection");
	viewLocation = program->uniformLocation("view");
	modelLocation = program->uniformLocation("model");
	normalMatrixLocation =program->uniformLocation("normalMatrix");

	camPositionLocation=program->uniformLocation("worldCamPos");

	glUniform3f(camPositionLocation,camPos[0],camPos[1],camPos[2]);

	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projection.data());
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, view.data());
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model.data());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, normal.data());
    */
}

void Camera::updateModel(Eigen::Matrix4f rotationMatrix, std::unique_ptr<QGLShaderProgram> &program)
{
	Eigen::Matrix4f model = setModel();
	model = rotationMatrix*model;
    /*
	program->bind();
	GLuint modelLocation = program->uniformLocation("model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model.data());
    */
}
void Camera::updateModel2(Eigen::Matrix4f rotationMatrix, std::unique_ptr<QGLShaderProgram> &program)
{
	Eigen::Matrix4f model = setModel();
	model = model*rotationMatrix;

    /*
	program->bind();
	GLuint modelLocation = program->uniformLocation("model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, model.data());
    */
}

void Camera::updateView(Eigen::Matrix4f rotationMatrix, std::unique_ptr<QGLShaderProgram> &program,float distance)
{
	Eigen::Matrix4f view = setView();

	const Eigen::Affine3f kTranslation(
		Eigen::Translation3f(Eigen::Vector3f(panX, panY, -distance)));
	const Eigen::Affine3f kRotationA(Eigen::AngleAxisf(rotationX, hra));
	const Eigen::Affine3f kRotationB(Eigen::AngleAxisf(rotationY, vra));

	view = kTranslation.matrix() * kRotationA.matrix() * kRotationB.matrix()* rotationMatrix;

    /*
	program->bind();
	GLuint viewLocation = program->uniformLocation("view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, view.data());
    */

	//update info

	Eigen::Matrix4f model = setModel();
	Eigen::Matrix4f t = view * model;
	Eigen::Matrix3f normal;
	for (int i = 0; i < 3; ++i)
	  for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

	normal = normal.inverse().transpose();

	Eigen::Vector4f camPos = view.inverse().col(3);

    /*
	GLuint normalMatrixLocation, camPositionLocation;

	normalMatrixLocation =program->uniformLocation("normalMatrix");

	camPositionLocation=program->uniformLocation("worldCamPos");

	glUniform3f(camPositionLocation,camPos[0],camPos[1],camPos[2]);
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, normal.data());
    */

}

void Camera::updateProjection(double left, double right, double bottom, double top, std::unique_ptr<QGLShaderProgram> &program)
{
	Eigen::Matrix4f projectionMatrix = getOrthoProjection(left,right,bottom,top,zNear,zFar);
    /*
	program->bind();
	GLuint projectionLocation = program->uniformLocation("projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projectionMatrix.data());
    */
}

}  //  namespace dataVisualization
