#include "vulkanwindow.h"
#include "vulkanrenderer.h"

QVulkanWindowRenderer* VulkanWindow::createRenderer() {
    renderer = new VulkanRenderer(this);
    return renderer;
}

namespace {

const float kFieldOfView = 60;
const float kZNear = 0.0001;
const float kZFar = 800;

}  // namespace

VulkanWindow::VulkanWindow(QWidget *parent) : QVulkanWindow()
{
    QSize viewportSize = this->swapChainImageSize();
    camera.setViewport(0, 0, viewportSize.width(), viewportSize.height());
    camera.setProjection(kFieldOfView, kZNear, kZFar);
    init();
}

void VulkanWindow::init() {
    width=0.0;
    height=0.0;
    initializedGL=false;
    initializedBuffers =false;
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

const vector3& VulkanWindow::getLightDir() const
{
    return light.getLightDir();
}

void VulkanWindow::setLightDir(const vector3& value)
{
    light.setLightDir(value);
}


void VulkanWindow::setSatellite(Object *obj)
{
    satellite=obj;
    TriangleMesh *mesh = obj->getMesh();
    camera.updateModel(mesh->min_,mesh->max_);

    /*
    TriangleMesh *cubeMesh = cube->getMesh();

    cubeMesh->vertices[3] = vector3(mesh->max_[0],mesh->max_[1],mesh->max_[2]);
    cubeMesh->vertices[1] = vector3(mesh->max_[0],mesh->min_[1],mesh->max_[2]);
    cubeMesh->vertices[2] = vector3(mesh->min_[0],mesh->max_[1],mesh->max_[2]);
    cubeMesh->vertices[5] = vector3(mesh->max_[0],mesh->max_[1],mesh->min_[2]);

    cubeMesh->vertices[6] = vector3(mesh->min_[0],mesh->min_[1],mesh->min_[2]);
    cubeMesh->vertices[0] = vector3(mesh->min_[0],mesh->min_[1],mesh->max_[2]);
    cubeMesh->vertices[4] = vector3(mesh->min_[0],mesh->max_[1],mesh->min_[2]);
    cubeMesh->vertices[7] = vector3(mesh->max_[0],mesh->min_[1],mesh->min_[2]);

    cubeMesh->computeBoundingBox();
    cubeMesh->prepareDataToGPU();
    cube->initializeBuffers();

    // raytracing rendering, render color into quad texture
    if(renderMode == RenderMode::GPU && advancedGPU)
    {
        const auto extremeValues = std::vector<float>{mesh->max_.x(),mesh->max_.y(),mesh->max_.z(),
                                                      mesh->min_.x(),mesh->min_.y(),mesh->min_.z()};
        const auto minPos = std::min_element(extremeValues.begin(),extremeValues.end());
        const auto maxPos = std::max_element(extremeValues.begin(),extremeValues.end());
        auto maxValue = *maxPos;
        auto minValue = *minPos;

        auto *quadMesh = quad->getMesh();
        quadMesh->replicatedVertices[0] = vector4(minValue,maxValue,0.0L,1.0L);
        quadMesh->replicatedVertices[1] = vector4(minValue,minValue,0.0L,1.0L);
        quadMesh->replicatedVertices[2] = vector4(maxValue,maxValue,0.0L,1.0L);
        quadMesh->replicatedVertices[3] = vector4(maxValue,maxValue,0.0L,1.0L);
        quadMesh->replicatedVertices[4] = vector4(minValue,minValue,0.0L,1.0L);
        quadMesh->replicatedVertices[5] = vector4(maxValue,minValue,0.0L,1.0L);

        quadMesh->max_=Eigen::Vector3f(maxValue,maxValue,0.0L);
        quadMesh->min_=Eigen::Vector3f(minValue,minValue,0.0L);
        quad->initializeBuffers();
    }

    if(renderMode == RenderMode::GPU){

        advancedGPU->initializeGL(width,height,satellite);
        advancedGPU->getCamera()->updateModel(mesh->min_,mesh->max_);
    }

    const auto& lightDir = light.getLightDir();
    Eigen::Vector3f center = (mesh->max_+mesh->min_)/2.0f;
    Eigen::Vector3f diff = mesh->max_-mesh->min_;

    float diagonalDiff = 1.3*diff.norm();
    const vector3 centerPos(center[0],center[1],center[2]);

    const auto newCenter = centerPos+vector3(0,0.2,0);
    const auto direction = diagonalDiff*lightDir;
    SunLine = createBigLineObject(newCenter-direction,newCenter+direction,light.getRightDir(),light.getUpDir());

    auto SunMesh = Sun->getMesh();
    const double scale = 0.3*diagonalDiff;
    for(unsigned int i = 0; i < SunMesh->vertices.size(); i++)
    {
        SunMesh->vertices[i] = scale*SunMesh->vertices[i] - direction + vector3(0,0.2,0);;
    }
    SunMesh->prepareDataToGPU();
    Sun->initializeBuffers();

    doneCurrent();

    xAxis = createBigLineObject(centerPos,centerPos+diff.norm()*vector3(1,0,0),vector3(0,1,0),vector3(0,0,1));
    xAxis->setDiffuseColor(QVector3D(1,0.2,0.2));
    yAxis = createBigLineObject(centerPos,centerPos+diff.norm()*vector3(0,1,0),vector3(1,0,0),vector3(0,0,1));
    yAxis->setDiffuseColor(QVector3D(0,1,0));
    zAxis = createBigLineObject(centerPos,centerPos+diff.norm()*vector3(0,0,1),vector3(1,0,0),vector3(0,1,0));
    zAxis->setDiffuseColor(QVector3D(0.2,0.2,1));

    */
}


void VulkanWindow::rotateSatellite(float angleX,float angleY,float angleZ) {
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


}

Eigen::Matrix4f VulkanWindow::getSatelliteRotation() const {
    return satelliteRotation;
}

/*
QVector3D getColour(float intensity)
{
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
*/

void VulkanWindow::setLabels(QLabel *minValue, QLabel *maxValue)
{
    minForceValue=minValue;
    maxForceValue = maxValue;
}

VulkanWindow::~VulkanWindow() {

}



