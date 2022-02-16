#include "glwidget.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

namespace {

const float kFieldOfView = 60;
const float kZNear = 0.1;
const float kZFar = 80;

}  // namespace

GLWidget::GLWidget(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers),parent)
{
	init();
}
GLWidget::GLWidget(const QGLFormat &glf, QWidget *parent) : QGLWidget(glf, parent) {
	init();
}

AdvancedGPU *GLWidget::getRayTraceGPU() const
{
	return advancedGPU;
}

void GLWidget::setRayTraceGPU(AdvancedGPU *value)
{
	advancedGPU = value;

	connect(advancedGPU,SIGNAL(render()),this,SLOT(renderGL()));

	renderMode = RenderMode::GPU;
}

bool GLWidget::getShowSatellite() const
{
	return showSatellite;
}

void GLWidget::setShowSatellite(bool value)
{
	showSatellite = value;
}

void GLWidget::sendSatelliteToGPU()
{
	satellite->sendObjectToGPU(advancedGPU->programGPU);
}

void GLWidget::setNumSecondaryRays()
{
	RayTraceGPU *rayT = dynamic_cast<RayTraceGPU*>(advancedGPU);
	rayT->setNumSecondaryRays(advancedGPU->programGPU);
}

void GLWidget::setLabels(QLabel *minValue, QLabel *maxValue)
{
	minForceValue=minValue;
	maxForceValue = maxValue;
}

Eigen::Matrix4f GLWidget::getSatelliteRotation() const
{
	return satelliteRotation;
}

void GLWidget::init(){
	width=0.0;
	height=0.0;
	initializedGL=false;
	initializedBuffers =false;
	setFocusPolicy( Qt::StrongFocus );
	lineForces.clear();
	renderMode = RenderMode::CPU;

	vertexShaderFile   = "://resources/shaders/vshader1.glsl";
	fragmentShaderFile = "://resources/shaders/fshader1.glsl";

	previousNumForces=0;

	vertexAttributeIdx = 0;
	normalAttributeIdx = 1;
	pdAttributeIdx = 2;
	psAttributeIdx = 3;

	showSatellite = true;
	previousAxisX = 0;
	previousAxisY = 0;
	previousAxisZ = 0;

	satelliteRotation << 1, 0, 0, 0,
						 0, 1, 0, 0,
						 0, 0, 1, 0,
						 0, 0, 0, 1;
	firstRotation=true;

	cube = new Object();
	quad = new Quad();

	char * nameCubeObj, *nameCubeMtl;
	std::string cubeObj = "://resources/models/cube0.obj";
	std::string cubeMat = "://resources/models/cube0.mtl";
	nameCubeObj = (char*)cubeObj.c_str();
	nameCubeMtl = (char*)cubeMat.c_str();

	cube->loadOBJ(nameCubeObj,nameCubeMtl);

	Sun = new Object();

	char * nameObj, *nameMtl;
	std::string obj = "://resources/sphere1.obj";
	std::string mat = "://resources/sphere1.mtl";
	nameObj = (char*)obj.c_str();
	nameMtl = (char*)mat.c_str();

	Sun->loadOBJ(nameObj,nameMtl);
	Sun->setDiffuseColor(QVector3D(1,1,0));
}

GLWidget::~GLWidget() {

}

QVector3D GLWidget::getLightDir() const
{
	return light.getLightDir();
}

void GLWidget::setLightDir(const QVector3D &value)
{
	light.setLightDir(value);
}

bool GLWidget::readFile(const std::string filename, std::string *shader_source)
{
	std::ifstream infile(filename.c_str());

	if (!infile.is_open() || !infile.good()) {
		std::cerr << "Error " + filename + " not found." << std::endl;
		return false;
	}

	std::stringstream stream;
	stream << infile.rdbuf();
	infile.close();

	*shader_source = stream.str();
	return true;
}

void GLWidget::initializeGL() {

	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_RGBA);
	glEnable(GL_DOUBLE);

	initShaders(vertexShaderFile.c_str(),fragmentShaderFile.c_str(),program);

	if(renderMode == RenderMode::GPU){
		initShaders(vertexShaderFile.c_str(),advancedGPU->getFragmentShaderFileGPU().c_str(),advancedGPU->programGPU);
		initShaders(vertexShaderFile.c_str(),advancedGPU->getFragmentShaderRenderGPU().c_str(),advancedGPU->programRenderGPU);

		advancedGPU->initializeGL(width,height,satellite); //,&camera

		if(RayTraceGPUTextures* rayTriT = dynamic_cast<RayTraceGPUTextures*>(advancedGPU)){
			rayTriT->sendTextures();
		}

		if(dynamic_cast<RayTraceGPU*>(advancedGPU)){
			setNumSecondaryRays();

			RayTraceGPU *rayT = dynamic_cast<RayTraceGPU*>(advancedGPU);
			rayT->setNoiseTexture(10,advancedGPU->programGPU);
			rayT->setDiffuseRays(advancedGPU->programGPU);
			rayT->setReflectionType(advancedGPU->programGPU);
		}
	}

	satelliteRotation.setIdentity();
	initializedGL=true;
	if(satellite && !initializedBuffers)
		initializeBuffers();
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void GLWidget::initializeShaders(std::string kVertexShaderFile, std::string kFragmentShaderFile,
			std::unique_ptr<QGLShaderProgram>& program)
{
	std::string vertex_shader, fragment_shader;
	bool res = readFile(kVertexShaderFile, &vertex_shader) &&
			   readFile(kFragmentShaderFile, &fragment_shader);

	if (!res) exit(0);

	program = make_unique<QGLShaderProgram>();
	program->addShaderFromSourceCode(QGLShader::Vertex,
									  vertex_shader.c_str());
	program->addShaderFromSourceCode(QGLShader::Fragment,
									  fragment_shader.c_str());
	program->link();
}

void GLWidget::initShaders(const char* vShaderFile, const char* fShaderFile,std::unique_ptr<QGLShaderProgram>& program)
{
	QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
	QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);

	vshader->compileSourceFile(vShaderFile);
	fshader->compileSourceFile(fShaderFile);

	program = make_unique<QGLShaderProgram>();
	program->addShader(vshader);
	program->addShader(fshader);
	program->bindAttributeLocation("vertex", vertexAttributeIdx);
	program->bindAttributeLocation("normal", normalAttributeIdx);
	program->bindAttributeLocation("pd", pdAttributeIdx);
	program->bindAttributeLocation("ps", psAttributeIdx);
	program->link();
	program->bind();
}

void GLWidget::resizeGL(int w, int h) {

	if (h == 0) h = 1;
	width = w;
	height = h;

	camera.setViewport(0, 0, w, h);
	camera.setProjection(kFieldOfView, kZNear, kZFar);

	if(renderMode == RenderMode::GPU && advancedGPU){
		dataVisualization::Camera * camera2 = advancedGPU->getCamera();
		camera2->setViewport(0, 0, w, h);
		camera2->setProjection(kFieldOfView, kZNear, kZFar);
		advancedGPU->setWidth(w);
		advancedGPU->setHeight(h);
		makeCurrent();
		advancedGPU->initializeBuffers();
		doneCurrent();
	}

	updateGL();
}

void GLWidget::initializeBuffers(){

	if(!initializedGL || !satellite)
		return;

	makeCurrent();

	if(renderMode == RenderMode::GPU && advancedGPU){
		advancedGPU->programRenderGPU->bind();
		quad->initializeBuffers();
	}
	program->bind();
	satellite->initializeBuffers();
	SunLine->initializeBuffers();
	xAxis->initializeBuffers();
	yAxis->initializeBuffers();
	zAxis->initializeBuffers();
	Sun->initializeBuffers();

	light.toGPU(program);

	doneCurrent();

	initializedBuffers=true;
}

void GLWidget::setSatellite(Object *obj)
{
	satellite=obj;
	TriangleMesh *mesh = obj->getMesh();
	camera.updateModel(mesh->min_,mesh->max_);

	makeCurrent();
	TriangleMesh *cubeMesh = cube->getMesh();

	cubeMesh->vertices[3] = QVector3D(mesh->max_[0],mesh->max_[1],mesh->max_[2]);
	cubeMesh->vertices[1] = QVector3D(mesh->max_[0],mesh->min_[1],mesh->max_[2]);
	cubeMesh->vertices[2] = QVector3D(mesh->min_[0],mesh->max_[1],mesh->max_[2]);
	cubeMesh->vertices[5] = QVector3D(mesh->max_[0],mesh->max_[1],mesh->min_[2]);

	cubeMesh->vertices[6] = QVector3D(mesh->min_[0],mesh->min_[1],mesh->min_[2]);
	cubeMesh->vertices[0] = QVector3D(mesh->min_[0],mesh->min_[1],mesh->max_[2]);
	cubeMesh->vertices[4] = QVector3D(mesh->min_[0],mesh->max_[1],mesh->min_[2]);
	cubeMesh->vertices[7] = QVector3D(mesh->max_[0],mesh->min_[1],mesh->min_[2]);

	cubeMesh->computeBoundingBox();
	cubeMesh->prepareDataToGPU();
	cube->initializeBuffers();
	doneCurrent();

	QVector3D lightDir = light.getLightDir();
	Eigen::Vector3f center = (mesh->max_+mesh->min_)/2.0f;
	Eigen::Vector3f diff = mesh->max_-mesh->min_;

	float diagonalDiff = 1.3*diff.norm();
	QVector3D centerPos =QVector3D(center[0],center[1],center[2]);

	QVector3D newCenter = centerPos+QVector3D(0,0.2,0);
	QVector3D direction = diagonalDiff*lightDir;
	SunLine = createBigLineObject(newCenter,newCenter+direction,light.getRightDir(),light.getUpDir());
	updateBigLineObject(SunLine,newCenter,newCenter-direction,light.getRightDir(),light.getUpDir());

	xAxis = createBigLineObject(centerPos,centerPos+diff.norm()*QVector3D(1,0,0),QVector3D(0,1,0),QVector3D(0,0,1));
	xAxis->setDiffuseColor(QVector3D(1,0.2,0.2));
	yAxis = createBigLineObject(centerPos,centerPos+diff.norm()*QVector3D(0,1,0),QVector3D(1,0,0),QVector3D(0,0,1));
	yAxis->setDiffuseColor(QVector3D(0,1,0));
	zAxis = createBigLineObject(centerPos,centerPos+diff.norm()*QVector3D(0,0,1),QVector3D(1,0,0),QVector3D(0,1,0));
	zAxis->setDiffuseColor(QVector3D(0.2,0.2,1));

	if(renderMode == RenderMode::GPU){

		advancedGPU->initializeGL(width,height,satellite); //,&camera
		advancedGPU->getCamera()->updateModel(mesh->min_,mesh->max_);
	}
}

void GLWidget::addNewForceSRP(QVector3D dir)
{
	QVector3D normDir = dir.normalized();

	TriangleMesh *mesh = satellite->getMesh();
	Eigen::Vector3f center = (mesh->max_+mesh->min_)/2.0f;

	Eigen::Vector3f diff = mesh->max_-mesh->min_;
	float diagonalDiff = diff.norm();
	float lambda= diagonalDiff*0.8;//2.5f;

	QVector3D initDir = QVector3D(center[0],center[1],center[2]);
	QVector3D endDir = initDir+lambda*normDir;

	LineForceSRP lineForce;
	lineForce.force = dir;
	lineForce.line = createLineObject(initDir,endDir);

	lineForces.push_back(lineForce);

	lineForce.line->initializeBuffers();
	lineForce.line->setDiffuseColor(QVector3D(1,1,1));
}

void GLWidget::clearLineForces()
{
	lineForces.clear();
	previousNumForces=0;
	minForce = std::numeric_limits<float>::infinity();
	maxForce = -std::numeric_limits<float>::infinity();
}

void GLWidget::rotateSatellite(float angleX, float angleY, float angleZ)
{
	if(fabs(angleX-previousAxisX)>0){
		Eigen::Matrix4f rotationX;
		Eigen::Vector4f axisX;
		if(firstRotation){
			axisX= Eigen::Vector4f(1,0,0,0);
			rotationX= Eigen::Affine3f(Eigen::AngleAxisf(angleX,Eigen::Vector3f(axisX[0],axisX[1],axisX[2]))).matrix();
			firstRotation=false;
		}
		else{
			axisX= satelliteRotation*Eigen::Vector4f(1,0,0,0);
			rotationX= Eigen::Affine3f(Eigen::AngleAxisf(angleX-previousAxisX,Eigen::Vector3f(axisX[0],axisX[1],axisX[2]))).matrix() * satelliteRotation;
		}

		float sizeX = QVector3D(rotationX(0,0),rotationX(1,0),rotationX(2,0)).length();
		float sizeY = QVector3D(rotationX(0,1),rotationX(1,1),rotationX(2,1)).length();
		float sizeZ = QVector3D(rotationX(0,2),rotationX(1,2),rotationX(2,2)).length();
		rotationX(0,0) /= sizeX; rotationX(1,0) /= sizeX; rotationX(2,0) /= sizeX;
		rotationX(0,1) /= sizeY; rotationX(1,1) /= sizeY; rotationX(2,1) /= sizeY;
		rotationX(0,2) /= sizeZ; rotationX(1,2) /= sizeZ; rotationX(2,2) /= sizeZ;

		previousAxisX = angleX;
		satelliteRotation = rotationX;
	}
	else if(fabs(angleY-previousAxisY)>0){
		Eigen::Matrix4f rotationY;
		Eigen::Vector4f axisY;
		if(firstRotation){
			axisY= Eigen::Vector4f(0,1,0,0);
			rotationY= Eigen::Affine3f(Eigen::AngleAxisf(angleY,Eigen::Vector3f(axisY[0],axisY[1],axisY[2]))).matrix();
			firstRotation=false;
		}
		else{
			axisY= satelliteRotation*Eigen::Vector4f(0,1,0,0);
			rotationY= Eigen::Affine3f(Eigen::AngleAxisf(angleY-previousAxisY,Eigen::Vector3f(axisY[0],axisY[1],axisY[2]))).matrix() * satelliteRotation;
		}

		float sizeX = QVector3D(rotationY(0,0),rotationY(1,0),rotationY(2,0)).length();
		float sizeY = QVector3D(rotationY(0,1),rotationY(1,1),rotationY(2,1)).length();
		float sizeZ = QVector3D(rotationY(0,2),rotationY(1,2),rotationY(2,2)).length();
		rotationY(0,0) /= sizeX; rotationY(1,0) /= sizeX; rotationY(2,0) /= sizeX;
		rotationY(0,1) /= sizeY; rotationY(1,1) /= sizeY; rotationY(2,1) /= sizeY;
		rotationY(0,2) /= sizeZ; rotationY(1,2) /= sizeZ; rotationY(2,2) /= sizeZ;

		previousAxisY = angleY;
		satelliteRotation = rotationY;
	}
	else if(fabs(angleZ-previousAxisZ)>0){
		Eigen::Matrix4f rotationZ;
		Eigen::Vector4f axisZ;
		if(firstRotation){
			axisZ= Eigen::Vector4f(0,0,1,0);
			rotationZ= Eigen::Affine3f(Eigen::AngleAxisf(angleZ,Eigen::Vector3f(axisZ[0],axisZ[1],axisZ[2]))).matrix();
			firstRotation=false;
		}
		else{
			axisZ= satelliteRotation*Eigen::Vector4f(0,0,1,0);
			rotationZ= Eigen::Affine3f(Eigen::AngleAxisf(angleZ-previousAxisZ,Eigen::Vector3f(axisZ[0],axisZ[1],axisZ[2]))).matrix() * satelliteRotation;
		}

		float sizeX = QVector3D(rotationZ(0,0),rotationZ(1,0),rotationZ(2,0)).length();
		float sizeY = QVector3D(rotationZ(0,1),rotationZ(1,1),rotationZ(2,1)).length();
		float sizeZ = QVector3D(rotationZ(0,2),rotationZ(1,2),rotationZ(2,2)).length();
		rotationZ(0,0) /= sizeX; rotationZ(1,0) /= sizeX; rotationZ(2,0) /= sizeX;
		rotationZ(0,1) /= sizeY; rotationZ(1,1) /= sizeY; rotationZ(2,1) /= sizeY;
		rotationZ(0,2) /= sizeZ; rotationZ(1,2) /= sizeZ; rotationZ(2,2) /= sizeZ;

		previousAxisZ = angleZ;
		satelliteRotation = rotationZ;
	}

	updateGL();
}

LineObject* GLWidget::createLineObject(QVector3D initDir, QVector3D endDir)
{
	LineObject* line = new LineObject();
	TriangleMesh *mesh2 = new TriangleMesh();
	mesh2->replicatedVertices.clear();
	mesh2->replicatedVertices.push_back(vec4(initDir.x(),initDir.y(),initDir.z(),1));
	mesh2->replicatedVertices.push_back(vec4(endDir.x(),endDir.y(),endDir.z(),1));

	line->setMesh(mesh2);

	return line;
}

LineObject* GLWidget::createBigLineObject(QVector3D initDir,QVector3D endDir,QVector3D axis1, QVector3D axis2){
	LineObject* line = new LineObject();
	TriangleMesh *mesh2 = new TriangleMesh();
	mesh2->replicatedVertices.clear();
	for(int i=-2; i<=2; i++){
		for(int j=-2; j<=2; j++){
			QVector3D d = i*0.005f*axis1 +j*0.005f*axis2;
			mesh2->replicatedVertices.push_back(vec4(d.x()+initDir.x(),d.y()+initDir.y(),d.z()+initDir.z(),1));
			mesh2->replicatedVertices.push_back(vec4(d.x()+endDir.x(),d.y()+endDir.y(),d.z()+endDir.z(),1));

		}
	}

	line->setMesh(mesh2);

	return line;
}

void GLWidget::updateBigLineObject(LineObject* line,QVector3D initDir,QVector3D endDir,QVector3D axis1, QVector3D axis2){

	TriangleMesh *mesh2 = line->getMesh();
	for(int i=-2; i<=2; i++){
		for(int j=-2; j<=2; j++){
			QVector3D d = i*0.005f*axis1 +j*0.005f*axis2;
			mesh2->replicatedVertices.push_back(vec4(d.x()+initDir.x(),d.y()+initDir.y(),d.z()+initDir.z(),1));
			mesh2->replicatedVertices.push_back(vec4(d.x()+endDir.x(),d.y()+endDir.y(),d.z()+endDir.z(),1));

		}
	}
}

void emitTimeout(QTimer * timer) {
  Q_ASSERT(timer);
  QTimerEvent event{timer->timerId()};
  QCoreApplication::sendEvent(timer, &event);
}

QVector3D getColour(float intensity){
	QVector3D color;
	if(intensity<0.34){
		color = (1-intensity/0.34f)*QVector3D(0.0,0.0,0.6) + (intensity/0.34f)*QVector3D(0.4,0.4,1);
	}
	else if(intensity<0.68){
		color = (1-(intensity-0.34)/0.34f)*QVector3D(0.4,0.4,1) + (intensity-0.34f)/0.34f*QVector3D(1,1,0);
	}
	else{
		color = (1-(intensity-0.68)/0.32f)*QVector3D(1,1,0) + (intensity-0.68f)/0.32f*QVector3D(1,0,0);
	}
	return color;
}

void GLWidget::paintGL() {
	//If the buffers are not initialized then we do not render.

	if (initializedBuffers) {
		program->bind();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if(renderMode == RenderMode::GPU && advancedGPU->forcesToCompute.size()>0){
			unsigned int id = advancedGPU->forcesToCompute[advancedGPU->forcesToCompute.size()-1];
			AdvancedGPU::ForceSRP &f = *advancedGPU->forces[id];
			advancedGPU->forcesToCompute.erase(advancedGPU->forcesToCompute.begin());

			advancedGPU->setLight(f.light);
			if(RayTraceGPUTextures* rayTriT = dynamic_cast<RayTraceGPUTextures*>(advancedGPU)){
				advancedGPU->draw(advancedGPU->programGPU,cube);
				advancedGPU->debugDraw(advancedGPU->programRenderGPU,quad);
				//advancedGPU->debugDraw(advancedGPU->programGPU,cube);
			}
			else{
				advancedGPU->draw(advancedGPU->programGPU,advancedGPU->getSatellite());
				//advancedGPU->debugDraw(advancedGPU->programGPU,advancedGPU->getSatellite());
				advancedGPU->debugDraw(advancedGPU->programRenderGPU,quad);
			}

			f.force = advancedGPU->getComputedForce();
			f.isComputed=true;
			emitTimeout(f.timer);
		}

		if(showSatellite){

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_CULL_FACE);

			camera.toGPU(program);

			SunLine->draw(program);

			TriangleMesh *mesh = satellite->getMesh();
			QVector3D lightDir = light.getLightDir();
			Eigen::Vector3f center = (mesh->max_+mesh->min_)/2.0f;
			Eigen::Vector3f differ = mesh->max_-mesh->min_;

			float diagonalDiff = differ.norm();
			QVector3D centerPos =QVector3D(center[0],center[1],center[2]);
			QVector3D initPos = QVector3D(0,0.4,0)-2*diagonalDiff*lightDir;

			TriangleMesh *mesh2 = Sun->getMesh();
			differ = mesh2->max_-mesh2->min_;
			float scale= std::max(std::max(differ[0],differ[1]),differ[2]);
			scale = 0.3;

			Eigen::Affine3f kScaling(
				  Eigen::Scaling(Eigen::Vector3f(scale,scale,scale)));
			Eigen::Affine3f kTranslation(Eigen::Translation3f(
				  Eigen::Vector3f(initPos.x(),initPos.y(),initPos.z())));

			camera.updateModel(kScaling.matrix()*kTranslation.matrix(),program);
			Sun->draw(program);

			camera.toGPU(program);
			camera.updateModel(satelliteRotation,program);

			satellite->draw(program);
			xAxis->draw(program);
			yAxis->draw(program);
			zAxis->draw(program);

			if(lineForces.size()>0 && lineForces.size()!=previousNumForces ){
				if(lineForces.size()==1) {

					maxForce = lineForces[0].force.length()+1;
					minForce = lineForces[0].force.length()-1;
					QString force = QString::number(lineForces[0].force.length());
					minForceValue->setText("The min length of the SRP forces is: "+force);
					maxForceValue->setText("The max length of the SRP forces is: "+force);
				}
				else{
					float min = std::numeric_limits<float>::infinity();
					float max = -std::numeric_limits<float>::infinity();

					for(uint i=0; i<lineForces.size(); i++){
						if(lineForces[i].force.length()<min)min = lineForces[i].force.length();
						if(lineForces[i].force.length()>max)max = lineForces[i].force.length();
					}
					minForce=min;
					maxForce=max;
					minForceValue->setText("The min length of the SRP forces is: "+QString::number(min));
					maxForceValue->setText("The max length of the SRP forces is: "+QString::number(max));

				}
				previousNumForces = lineForces.size();
			}
			float diff = maxForce-minForce;
			if(fabs(diff)<1.e-8) diff=1;
			for(uint i=0; i<lineForces.size(); i++){
				lineForces[i].line->setDiffuseColor(getColour((lineForces[i].force.length()-minForce)/diff));
				lineForces[i].line->draw(program);
			}
			glEnable(GL_CULL_FACE);
		}
	}
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  //lastPos = event->pos();
	if (event->button() == Qt::RightButton) {
	  camera.startRotating(event->x(), event->y());
	}
	if (event->button() == Qt::LeftButton) {
	  camera.startZooming(event->x(), event->y());
	}

  updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {

	if(!showSatellite) return;

	camera.setRotationX(event->y());
	camera.setRotationY(event->x());
	camera.safeZoom(event->y());
	updateGL();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
  if(!showSatellite) return;

  if (event->button() == Qt::RightButton) {
	camera.stopRotating(event->x(), event->y());
  }
  if (event->button() == Qt::LeftButton) {
	camera.stopZooming(event->x(), event->y());
  }
  updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  if(!showSatellite) return;

  if (event->key() == Qt::Key_Up) camera.zoom(-1);
  if (event->key() == Qt::Key_Down) camera.zoom(1);

  if (event->key() == Qt::Key_Left) camera.rotate(-1);
  if (event->key() == Qt::Key_Right) camera.rotate(1);

  if (event->key() == Qt::Key_W) camera.zoom(-1);
  if (event->key() == Qt::Key_S) camera.zoom(1);

  if (event->key() == Qt::Key_A) camera.rotate(-1);
  if (event->key() == Qt::Key_D) camera.rotate(1);

  updateGL();
}

void GLWidget::qNormalizeAngle(double &angle)
{
	while (angle < 0)
		angle += 360*16;
	while (angle > 360)
		angle -= 360*16;
}

void GLWidget::renderGL(){
	updateGL();
}
