#ifndef VKVISUALIZATION_H
#define VKVISUALIZATION_H
#include <QWidget>
#include <QtWidgets>
#include "vulkanwindow.h"
#include "SRP/srp.h"
#include "unordered_set"
#define MAX_AXIS 3

enum Axis{_X_=0,_Y_=1,_Z_=2};

/*
 * This class creates a window that allows the user interact with the satellite and individual forces.
 */
class VkVisualization: public QWidget
{
    Q_OBJECT

    struct AxisWidgets{
        QCheckBox *checkBox;
        QSpinBox *spinBox;
        QSlider *slider;
    };

    VulkanWindow* vk_window;
    AxisWidgets axes[MAX_AXIS];
    QToolButton *lockButton, *unlockButton;
    QLabel* minValue,*maxValue;

    std::unordered_set<long int> createdLineForces;

    SRP* model;

public:

    explicit VkVisualization(VulkanWindow* w);
    //virtual ~VkVisualization();

    // void setSatellite(Object* satellite);
    // void drawSatellite();

    // SRP *getModel() const;
    // void setModel(SRP *value);

private:
    void generateAxisInformation(QHBoxLayout *axisLayoutContainer,QHBoxLayout *axisLayout,QLabel *labelAxisName,
                                 QString text, QFont font1,AxisWidgets axis);

private slots:
    void lockAxis();
    void addForceSRP();
    void rotateSatellite();
    void clearLineForces();
};

#endif // VKVISUALIZATION_H
