
/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "mainwindow.h"
#include <QApplication>

// Qt Vulkan dependencies
#include <QVulkanInstance>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    QVulkanInstance inst;

	MainWindow w;
	w.setWindowTitle(QString("HiFi-SoRaP"));

    // supported layers
    qInfo("Supported Vulkan instance layers");
    for(auto layer : inst.supportedLayers()) {
        qInfo(layer.name);
    }

    QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

    // set vulkan desired layers
    // VK_LAYER_KHRONOS_validation for debug purposes
    inst.setLayers(QByteArrayList() << "VK_LAYER_KHRONOS_validation");
    //inst.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");

    if (!inst.create()) {
        qFatal("Failed to create Vulkan instance: %d", inst.errorCode());
    }

    qInfo("Created vulkan instance: %d", inst.isValid());

    // initialized layers
    qInfo("Supported Vulkan instance layers");
    for(auto layer : inst.layers()) {
        qInfo(layer);
    }


    w.setVulkanInstance(&inst);

	w.show();

	return a.exec();
}
