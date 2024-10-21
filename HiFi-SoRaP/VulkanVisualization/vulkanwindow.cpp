#include "vulkanwindow.h"

QVulkanWindowRenderer* VulkanWindow::createRenderer() {
    return new VulkanRenderer(this);
}

namespace {

const float kFieldOfView = 60;
const float kZNear = 0.0001;
const float kZFar = 800;

}  // namespace

VulkanWindow::VulkanWindow(QWidget *parent) : QVulkanWindow()
{
}

void VulkanWindow::setLabels(QLabel *minValue, QLabel *maxValue)
{
    minForceValue=minValue;
    maxForceValue = maxValue;
}

VulkanWindow::~VulkanWindow() {

}



