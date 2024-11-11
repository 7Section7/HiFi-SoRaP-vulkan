#include "vkvisualization.h"

void VkVisualization::setModel(SRP *value)
{
    model = value;
    // TODO:
    /*
    if(AdvancedGPU* m=dynamic_cast<AdvancedGPU*>(model)){
        glWidget->setRayTraceGPU(m);
    }
    */
}

void VkVisualization::setSatellite(Object *satellite)
{
    vk_window->setSatellite(satellite);
}

VkVisualization::VkVisualization(VulkanWindow* w) : QWidget(), vk_window(w)
{
    QWidget* vkWidget = QWidget::createWindowContainer(w);

    this->setObjectName(QStringLiteral("centralWidget"));
    this->setWindowTitle(QString("Satellite Visualization"));

    this->setMinimumHeight(560);
    this->setMaximumHeight(560);

    this->setMaximumWidth(1250);

    QGroupBox *gridGroupBox = new QGroupBox(this);
    gridGroupBox->setObjectName(QStringLiteral("gridGroupBox"));
    gridGroupBox->setGeometry(QRect(0, 0, 1000, 550));
    gridGroupBox->setMinimumWidth(1050);
    gridGroupBox->setMaximumWidth(1200);
    gridGroupBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridGroupBox->setLayout(gridLayout);


    vkWidget->setGeometry(QRect(0, 0, 512, 512));
    vkWidget->setMaximumSize(512, 512);
    vkWidget->setMinimumSize(512, 512);
    gridLayout->addWidget(vkWidget, 0, 0);


    QSpacerItem *spacerh = new QSpacerItem(30,0,QSizePolicy::Fixed,QSizePolicy::Fixed);
    gridLayout->addItem(spacerh,0,1);
    QGridLayout * gridLayout2 = new QGridLayout;
    gridLayout->addLayout(gridLayout2,0,2);

    QSpacerItem *spacerForText = new QSpacerItem(0,10);
    gridLayout2->addItem(spacerForText,1,0);

    QFont font1;
    font1.setBold(false);
    font1.setWeight(50);

    QFont font2;
    font2.setBold(true);
    font2.setWeight(70);

    QLabel *labelInfoSpinners = new QLabel();
    labelInfoSpinners->setFont(font2);
    labelInfoSpinners->setObjectName(QStringLiteral("labelInfoSpinners"));
    labelInfoSpinners->setGeometry(QRect(160, 0, 31, 17));
    labelInfoSpinners->setText(QString("1. Set the initial rotation of the satellite by using the next 3 sliders:"));
    labelInfoSpinners->setContentsMargins(0,0,0,0);
    gridLayout2->addWidget(labelInfoSpinners,2,0);

    QHBoxLayout *xAxisLayoutContainer = new QHBoxLayout();

    axes[_X_].checkBox = new QCheckBox();
    QHBoxLayout *xAxisLayout = new QHBoxLayout();

    QLabel *xLabelAxisName = new QLabel();
    axes[_X_].spinBox = new QSpinBox();
    axes[_X_].slider = new QSlider();
    generateAxisInformation(xAxisLayoutContainer,xAxisLayout,xLabelAxisName,QString("X: "),font1,axes[_X_]);
    xLabelAxisName->setStyleSheet("color:red;");
    gridLayout2->addLayout(xAxisLayoutContainer,3,0);

    QHBoxLayout *yAxisLayoutContainer = new QHBoxLayout();

    axes[_Y_].checkBox = new QCheckBox();

    QHBoxLayout *yAxisLayout = new QHBoxLayout();
    QLabel *yLabelAxisName = new QLabel();
    axes[_Y_].spinBox = new QSpinBox();
    axes[_Y_].slider = new QSlider();
    generateAxisInformation(yAxisLayoutContainer,yAxisLayout,yLabelAxisName,QString("Y: "),font1,axes[_Y_]);
    yLabelAxisName->setStyleSheet("color:green;");
    gridLayout2->addLayout(yAxisLayoutContainer,4,0);

    QHBoxLayout *zAxisLayoutContainer = new QHBoxLayout();

    axes[_Z_].checkBox= new QCheckBox();

    QHBoxLayout *zAxisLayout = new QHBoxLayout();
    QLabel *zLabelAxisName = new QLabel();
    axes[_Z_].spinBox = new QSpinBox();
    axes[_Z_].slider = new QSlider();
    generateAxisInformation(zAxisLayoutContainer,zAxisLayout,zLabelAxisName,QString("Z: "),font1,axes[_Z_]);
    zLabelAxisName->setStyleSheet("color:blue;");
    gridLayout2->addLayout(zAxisLayoutContainer,5,0);

    QSpacerItem *spacer = new QSpacerItem(0,20,QSizePolicy::Fixed,QSizePolicy::Fixed);
    gridLayout2->addItem(spacer,6,0);

    QLabel *labelInfoButtons = new QLabel();
    labelInfoButtons->setFont(font2);
    labelInfoButtons->setObjectName(QStringLiteral("labelInfoButtons"));
    labelInfoButtons->setGeometry(QRect(160, 0, 31, 17));
    labelInfoButtons->setText(QString("2. Visualization of the accelerations."));
    labelInfoButtons->setContentsMargins(0,30,0,0);
    gridLayout2->addWidget(labelInfoButtons,7,0);

    QLabel *labelInfoButtons1 = new QLabel();
    labelInfoButtons1->setFont(font1);
    labelInfoButtons1->setStyleSheet("margin-left:10px;");
    labelInfoButtons1->setObjectName(QStringLiteral("labelInfoButtons"));
    labelInfoButtons1->setGeometry(QRect(160, 0, 31, 17));
    labelInfoButtons1->setText(QString("2.1 Click on the Start button."));
    labelInfoButtons1->setContentsMargins(30,30,0,0);
    gridLayout2->addWidget(labelInfoButtons1,8,0);

    QLabel *labelInfoButtons2 = new QLabel();
    labelInfoButtons2->setFont(font1);
    labelInfoButtons2->setStyleSheet("margin-left:10px;");
    labelInfoButtons2->setObjectName(QStringLiteral("labelInfoButtons"));
    labelInfoButtons2->setGeometry(QRect(160, 0, 51, 17));
    labelInfoButtons2->setText(QString("2.2 Rotate the satellite with the sliders."));
    labelInfoButtons2->setContentsMargins(30,30,0,0);
    gridLayout2->addWidget(labelInfoButtons2,9,0);

    QSpacerItem *spacer2 = new QSpacerItem(0,30,QSizePolicy::Fixed,QSizePolicy::Fixed);
    gridLayout2->addItem(spacer2,10,0);

    QLabel *labelInfoButtons3 = new QLabel();
    labelInfoButtons3->setFont(font1);
    labelInfoButtons3->setStyleSheet("margin-left:10px;");
    labelInfoButtons3->setObjectName(QStringLiteral("labelInfoButtons"));
    labelInfoButtons3->setGeometry(QRect(160, 0, 51, 17));
    labelInfoButtons3->setText(QString("SRP acceleration values:"));
    labelInfoButtons3->setContentsMargins(30,30,0,0);
    gridLayout2->addWidget(labelInfoButtons3,11,0);

    QLinearGradient gradient(0,50,0,0);

    gradient.setColorAt(0.0f,Qt::blue);
    gradient.setColorAt(1.0f,Qt::red);

    QPalette palette;
    palette.setBrush(QPalette::Background,gradient);

    QWidget* gradientWidget = new QWidget();
    gradientWidget->setGeometry(QRect(0, 0, 25, 50));
    gradientWidget->setMinimumSize(25, 50);
    gradientWidget->setMaximumSize(25, 50);
    gradientWidget->setContentsMargins(60,0,0,0);
    gradientWidget->setPalette(palette);
    gradientWidget->setAutoFillBackground(true);

    QHBoxLayout *layouth = new QHBoxLayout;

    gridLayout2->addLayout(layouth,12,0);

    QSpacerItem *spacer6 = new QSpacerItem(10,0,QSizePolicy::Fixed,QSizePolicy::Fixed);
    layouth->addItem(spacer6);
    layouth->addWidget(gradientWidget);

    QVBoxLayout* layoutv = new QVBoxLayout;
    layouth->addLayout(layoutv);

    QLabel *labelMaxValue = new QLabel();
    labelMaxValue->setFont(font1);
    labelMaxValue->setStyleSheet("margin-left:10px; color:red;");
    labelMaxValue->setObjectName(QStringLiteral("labelInfoButtons"));
    labelMaxValue->setGeometry(QRect(0, 0, 31, 17));
    labelMaxValue->setText(QString("The max length of the SRP accelerations is: "));
    labelMaxValue->setContentsMargins(0,0,0,0);
    layoutv->addWidget(labelMaxValue);

    QLabel *labelMinValue = new QLabel();
    labelMinValue->setFont(font1);
    labelMinValue->setStyleSheet("margin-left:10px; color:blue;");
    labelMinValue->setObjectName(QStringLiteral("labelInfoButtons"));
    labelMinValue->setGeometry(QRect(0, 0, 31, 17));
    labelMinValue->setText(QString("The min length of the SRP accelerations is:"));
    labelMinValue->setContentsMargins(0,0,0,0);
    layoutv->addWidget(labelMinValue);

    QSpacerItem *spacer3 = new QSpacerItem(0,30,QSizePolicy::Fixed,QSizePolicy::Fixed);
    gridLayout2->addItem(spacer3,13,0);

    QToolButton *lockButton, *unlockButton;

    lockButton = new QToolButton();
    lockButton->setIcon(QIcon("://resources/images/startButton.png"));
    lockButton->setIconSize(QSize(80,80));
    gridLayout2->addWidget(lockButton,14,0);

    unlockButton = new QToolButton();
    unlockButton->setToolTip(QString("Reset the visualization. Delete all the forces computed."));
    unlockButton->setIcon(QIcon("://resources/images/stop.png"));
    unlockButton->setIconSize(QSize(80,80));
    unlockButton->setDisabled(true);
    gridLayout2->addWidget(unlockButton,14,1);

    minValue=labelMinValue;
    maxValue =labelMaxValue;
    vk_window->setLabels(labelMinValue,labelMaxValue);

    QCheckBox * xCheckBox = axes[_X_].checkBox;
    QCheckBox * yCheckBox = axes[_Y_].checkBox;
    QCheckBox * zCheckBox = axes[_Z_].checkBox;

    QObject::connect(lockButton, &QToolButton::pressed, [this]() {
        this->lockAxis();
    });

    QObject::connect(unlockButton, &QToolButton::pressed, [xCheckBox]() {
        xCheckBox->setEnabled(true);
    });
    QObject::connect(unlockButton, &QToolButton::pressed, [yCheckBox]() {
        yCheckBox->setEnabled(true);
    });
    QObject::connect(unlockButton, &QToolButton::pressed, [zCheckBox]() {
        zCheckBox->setEnabled(true);
    });
    QObject::connect(unlockButton, &QToolButton::pressed, [lockButton]() {
        lockButton->setEnabled(true);
    });
    QObject::connect(unlockButton, &QToolButton::pressed, [unlockButton]() {
        unlockButton->setDisabled(true);
    });

    QObject::connect(lockButton, &QToolButton::pressed, [this]() {
        this->lockAxis();
    });

    QObject::connect(unlockButton, &QToolButton::pressed, [this]() {
        this->clearLineForces();
        maxValue->setText(QString("The max length of the SRP forces is: "));
        minValue->setText(QString("The min length of the SRP forces is: "));
    });

    this->lockButton = lockButton;
    this->unlockButton = unlockButton;
}


void VkVisualization::generateAxisInformation(QHBoxLayout *axisLayoutContainer,QHBoxLayout *axisLayout,QLabel *labelAxisName, QString text, QFont font1,AxisWidgets axis)
{
    axisLayoutContainer->setContentsMargins(0,0,0,0);

    axis.checkBox->setChecked(false);

    axisLayout->setContentsMargins(15,0,0,0);

    labelAxisName->setFont(font1);
    labelAxisName->setText(text);
    labelAxisName->setContentsMargins(0,0,20,0);
    axisLayout->addWidget(labelAxisName);

    axis.spinBox->setFont(font1);
    axis.spinBox->setValue(0);
    axis.spinBox->setMinimum(-180);
    axis.spinBox->setMaximum(180);
    axis.spinBox->setSingleStep(10);
    axis.spinBox->setSuffix(QString(" º"));
    axis.spinBox->setContentsMargins(0,0,10,0);

#ifdef _WIN32
    axis.spinBox->setGeometry(10,0,10,10);
#endif

    axisLayout->addWidget(axis.spinBox);

    axis.slider->setFont(font1);
    axis.slider->setValue(0);
    axis.slider->setMinimum(-180);
    axis.slider->setMaximum(180);
    axis.slider->setSingleStep(10);
    axis.slider->setOrientation(Qt::Horizontal);
    axis.slider->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

#ifdef _WIN32
    axis.slider->setGeometry(10,0,50,10);
    axis.slider->setMinimumWidth(30);
#else
    axis.slider->setGeometry(10,0,500,10);
    axis.slider->setMinimumWidth(350);
#endif

    axis.slider->setGeometry(10,0,40,10);
    axis.slider->setMinimumWidth(30);
    axisLayout->addWidget(axis.slider);
    axisLayoutContainer->addLayout(axisLayout);


    VulkanWindow::connect(axis.slider, SIGNAL(valueChanged(int)), axis.spinBox, SLOT(setValue(int)));
    VulkanWindow::connect(axis.spinBox, SIGNAL(valueChanged(int)), axis.slider, SLOT(setValue(int)));

    VulkanWindow::connect(axis.checkBox,SIGNAL(toggled(bool)),labelAxisName,SLOT(setDisabled(bool)));
    VulkanWindow::connect(axis.checkBox,SIGNAL(toggled(bool)),axis.spinBox,SLOT(setDisabled(bool)));
    VulkanWindow::connect(axis.checkBox,SIGNAL(toggled(bool)),axis.slider,SLOT(setDisabled(bool)));

    QObject::connect(axis.slider, &QSlider::valueChanged, [this]() {
        this->rotateSatellite();
        this->addForceSRP();
    });

}

void VkVisualization::lockAxis()
{
    lockButton->setDisabled(true);
    unlockButton->setEnabled(true);
}

void VkVisualization::addForceSRP()
{
    if(lockButton->isEnabled())
        return;

    long int xAngle, yAngle,zAngle;

    xAngle = axes[_X_].spinBox->value();
    yAngle = axes[_Y_].spinBox->value();
    zAngle = axes[_Z_].spinBox->value();

    xAngle+=180;
    yAngle+=180;
    zAngle+=180;

    long int id = xAngle*360*360 + yAngle*360 + zAngle;

    std::unordered_set<long int>::const_iterator it = createdLineForces.find(id);

    if (it == createdLineForces.end()) {

        const auto& lightDir = vk_window->getLightDir();
        Eigen::Matrix4f rotationMatrix = vk_window->getSatelliteRotation();

        const auto force = model->computeSRP(lightDir,rotationMatrix);


#ifdef QT_DEBUG
        qDebug("Calculated SRP force:%f,%f,%f", force.x, force.y, force.z);
#endif

        // glWidget->addNewForceSRP(force);
        // createdLineForces.insert(id);
    }

    //glWidget->updateGL();
}

void VkVisualization::rotateSatellite()
{
    float angleX,angleY,angleZ;
    angleX = axes[_X_].spinBox->value()/180.0f*M_PI;
    angleY = axes[_Y_].spinBox->value()/180.0f*M_PI;
    angleZ = axes[_Z_].spinBox->value()/180.0f*M_PI;

    vk_window->rotateSatellite(angleX, angleY, angleZ);
}

void VkVisualization::clearLineForces()
{
    // glWidget->clearLineForces();
    // glWidget->updateGL();
    createdLineForces.clear();
}

// void VkVisualization::drawSatellite()
// {
//     //glWidget->initializeBuffers();
//     //glWidget->updateGL();
// }
