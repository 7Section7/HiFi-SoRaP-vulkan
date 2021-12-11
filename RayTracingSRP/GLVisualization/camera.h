#ifndef CAMERAH
#define CAMERAH

#include "Lib/common.h"
#include <Lib/eigen3/Eigen/Geometry>
#include <QGLShaderProgram>

#include <memory>

namespace dataVisualization {

const double kMaxCameraDistance = 3;//3.0;
const double kMinCameraDistance = 1.5;//-kMaxCameraDistance;

const double kMinRotationX = -M_PI / 2;
const double kMaxRotationX = M_PI / 2;

const double kAngleIncrement = 0.01;

/*
 * This class contains the information of the camera where the observer is watching.
 */
class Camera {
 private:
  /**
   * @brief distance Zoom distance.
   */
  double distance;

  /**
   * @brief step A parameter that determines the influence of the mouse
   * movement over the camera.
   */
  double step;

  /**
   * @brief rotating Indicates whether the camera is rotating.
   */
  bool rotating;

  /**
   * @brief zooming Indicates whether the camera is zooming.
   */
  bool zooming;

  /**
   * @brief zooming Indicates whether the camera is panning.
   */
  bool panning;

  /**
   * @brief rotationy Camera rotation around the Y axis.
   */
  double rotationY;

  /**
   * @brief rotationy Camera rotation around the X axis.
   */
  double rotationX;

  /**
   * @brief panx Camera pan in the horizontal Viewport direction.
   */
  double panX;

  /**
   * @brief pany Camera pan in the vertical Viewport direction.
   */
  double panY;

  /**
   * @brief currentx Should store the mouse current X to compute
   * displacements.
   */
  double currentX;

  /**
   * @brief currenty Should store the mouse current Y to compute
   * displacements.
   */
  double currentY;

  /**
   * @brief viewportx Viewport X position.
   */
  double viewportX;

  /**
   * @brief viewporty Viewport Y position.
   */
  double viewportY;

  /**
   * @brief viewportwidth Viewport width;
   */
  double viewportWidth;

  /**
   * @brief viewportheight Viewport height;
   */
  double viewportHeight;

  /**
   * @brief centeringx Translation applied to the modeling transform. Tries to
   * center a model bounding box.
   */
  double centeringX;

  /**
   * @brief centeringy Translation applied to the modeling transform. Tries to
   * center a model bounding box.
   */
  double centeringY;

  /**
   * @brief centeringz Translation applied to the modeling transform. Tries to
   * center a model bounding box.
   */
  double centeringZ;

  /**
   * @brief scaling Scaling applied to the modeling transform. Tries to make
   * unit length the longest edge of a model bounding box.
   */
  double scaling;

  /**
   * @brief fieldofview Field of view for a perspective camera.
   */
  double fieldOfView;

  /**
   * @brief znear Near plane Z for a perspective camera.
   */
  double zNear;

  /**
   * @brief zfar Far plane Z for a perspective camera.
   */
  double zFar;

 public:
  /**
   * @brief Camera Constructor of the class.
   */
  Camera();

  /**
   * @brief Camera Destructor of the class.
   */
  ~Camera() {}

  Eigen::Matrix4f lookAt(Eigen::Vector3f const & eye, Eigen::Vector3f const & center, Eigen::Vector3f const & up );

  Eigen::Matrix4f getProjection(double ang, double vp, double zn, double zf);
  Eigen::Matrix4f getOrthoProjection(double left, double right, double bottom, double top, double zn, double zf);

  /**
   * @brief SetViewport Calls glViewport with the current viewport position,
   * width and height and stores these values.
   * @param x Viewport position x.
   * @param y Viewport position y.
   * @param w Viewport width-
   * @param h Viewport height.
   */
  void setViewport(double x, double y, double w, double h);

  /**
   * @brief SetViewport Calls glViewport with the current viewport position,
   * width and height.
   */
  void setViewport() const;

  /**
   * @brief SetIdentity Returns an identity matrix.
   * @return An identity matrix.
   */
  Eigen::Matrix4f setIdentity() const;

  /**
   * @brief SetModel Returns a model transform matrix that centers the last
   * "updated" model and scales its bounding box longest edge to unit length.
   * @return A modeling transform.
   */
  Eigen::Matrix4f setModel() const;

  /**
   * @brief SetView Computed the viewing matrix given the rotationx around the
   * x axis, rotationy around the y axis, a pan with translation (panx,
   * pany) and a zoom out of distance.
   * @return A viewing transform.
   */
  Eigen::Matrix4f setView() const;

  /**
   * @brief SetProjection Computes the projection matrix for a perspective
   * camera, and stores its parameters.
   * @param fov Field of view for a perspective camera.
   * @param znear Near plane Z for a perspective camera.
   * @param zfar Far plane Z for a perspective camera.
   * @return A perspective camera matrix transform.
   */
  Eigen::Matrix4f setProjection(double fov, double znear, double zfar);

  /**
   * @brief SetProjection Computes the projection matrix for a perspective
   * camera, using the stored parameters.
   * @return A perspective camera matrix transform.
   */
  Eigen::Matrix4f setProjection() const;

  /**
   * @brief Zoom Zooms the camera in the direction given by the modifier.
   * @param modifier Sign of the zooming direction.
   */
  void zoom(double modifier);

  /**
   * @brief Zoom If zooming is active, zooms the camera in the direction given
   * by the modifier.
   * @param modifier Sign of the zooming direction.
   */
  void safeZoom(double modifier);

  /**
   * @brief SafePan If panning is active, pans the camera by the displaced mouse
   * position.
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void safePan(double x, double y);

  /**
   * @brief Rotate Rotates the camera around the Y axis.
   * @param modifier Sign of the rotation direction.
   */
  void rotate(double modifier);

  /**
   * @brief UpdateModel Updates the intrinsic parameters to compute a modeling
   * transform that centers the bounding box of the model and makes its longest
   * edge unit length.
   * @param min Minimum point of the model bounding box.
   * @param max Maximum point of the model bounding box.
   */
  void updateModel(Eigen::Vector3f min, Eigen::Vector3f max);

  /**
   * @brief SetRotationX If rotating is active, rotates the camera around the X
   * axis.
   * @param y New mouse Y position.
   */
  void setRotationX(double y);

  /**
   * @brief SetRotationY If rotating is active, Rotates the camera around the Y
   * axis.
   * @param x New mouse X position.
   */
  void setRotationY(double x);

  /**
   * @brief StartRotating Allows camera rotation with starting mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void startRotating(double x, double y);

  /**
   * @brief StopRotating Finishes camera rotation with ending mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void stopRotating(double x, double y);

  /**
   * @brief StartZooming Allows camera zooming with starting mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void startZooming(double x, double y);

  /**
   * @brief StopZooming Finishes camera zooming with ending mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void stopZooming(double x, double y);

  /**
   * @brief StartPanning Allows camera panning with starting mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void startPanning(double x, double y);

  /**
   * @brief StopPanning Finishes camera panning with ending mouse position
   * (x,y).
   * @param x New mouse X position.
   * @param y New mouse Y position.
   */
  void stopPanning(double x, double y);

  /**
   * @brief SetCameraStep Modifies the camera step.
   * @param step New camera step.
   */
  void setCameraStep(double step);

  void toGPU(std::unique_ptr<QGLShaderProgram> &program);

  void updateModel(Eigen::Matrix4f rotationMatrix,std::unique_ptr<QGLShaderProgram> &program);
  void updateModel2(Eigen::Matrix4f rotationMatrix,std::unique_ptr<QGLShaderProgram> &program);

  void updateView(Eigen::Matrix4f rotationMatrix,std::unique_ptr<QGLShaderProgram> &program, float distance=3.0f);

  void updateProjection(double left, double right, double bottom, double top, std::unique_ptr<QGLShaderProgram> &program);
};

}  //  namespace datavisualization

#endif  //  CAMERAH
