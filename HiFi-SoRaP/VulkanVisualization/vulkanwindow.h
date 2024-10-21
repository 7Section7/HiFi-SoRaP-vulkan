#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include "vulkanrenderer.h"
#include <QtWidgets>


//#include <QGLShaderProgram>

#include <QtWidgets>
#include <QMouseEvent>

#include <fstream>
#include <iostream>
#include <memory>
#include <QString>

#include "GLVisualization/camera.h"
#include "SRP/GPU_SRP/advancedgpu.h"
#include "MeshObjects/object.h"
#include "MeshObjects/lineobject.h"
#include "MeshObjects/quad.h"
#include "GLVisualization/light.h"
#include "SRP/GPU_SRP/raytracegpu.h"
#include "SRP/GPU_SRP/raytracegputextures.h"



enum RenderMode{CPU=0, GPU=1};


/*
 * This class creates a window that allows the user interact with the satellite and individual forces.
 * Similar to GLWidget, this class will be embeded inside VkVisualization (which is analogus to GLWindow)
 */
class VulkanWindow : public QVulkanWindow
{
    Q_OBJECT
public:

    QVulkanWindowRenderer* createRenderer() override;

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

    void initShaders(const char* vShaderFile, const char* fShaderFile, std::unique_ptr<QGLShaderProgram>& program);
    void initializeShaders(std::string kVertexShaderFile, std::string kFragmentShaderFile,
                           std::unique_ptr<QGLShaderProgram>& program);


    LineObject* createLineObject(const vector3& initPos, const vector3& endPos);
    LineObject* createBigLineObject(const vector3& initPos, const vector3& endPos, const vector3& axis1, const vector3& axis2);
    void updateBigLineObject(TriangleMesh* line, const vector3& initPos, const vector3& endPos, const vector3& axis1,
                             const vector3& axis2);

protected:

    // void resizeGL(int w, int h);

    // void mousePressEvent(QMouseEvent *event) override;
    // void mouseMoveEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;
    // void keyPressEvent(QKeyEvent *event) override;

    // void qNormalizeAngle(double &angle);

public:
    VulkanWindow(QWidget *parent=0);
    ~VulkanWindow();

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


};

#endif // VULKANWINDOW_H
