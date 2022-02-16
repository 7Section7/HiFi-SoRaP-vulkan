#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

#include <QtWidgets>
//#include "glwidget.h"
#include "GLVisualization/glwindow.h"

#include "DataVisualization/visualizationwindow.h"
//#include <QMainWindow>
#include <QFileDialog>
#include <QProgressBar>

//#include "SRP/srp.h"
#include "SRP/CPU_SRP/canball.h"
#include "SRP/CPU_SRP/nplate.h"
#include "SRP/CPU_SRP/raytracecpu.h"

#include "SRP/GPU_SRP/render.h"
#include "SRP/GPU_SRP/raytracegputextures.h"

#include "Comparison/comparisonwindow.h"

#define MAX_SRP_MODELS 4

QT_BEGIN_NAMESPACE
class QSlider;
class GLWidget;
QT_END_NAMESPACE

namespace Ui {
	class MainWindow;
}

/*
 * This class creates the main window for this application.
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	void setVisualizations(std::vector<VisualizationWindow *> &vis);

private slots:
	void on_bttn_loadOBJ_clicked();

	void on_bttn_loadMAT_clicked();

	void bttn_loadSatelliteInfo_clicked();

	void on_bttn_generateOutput_clicked();

	void on_bttn_visualizeSatellite_clicked();

	void on_actionCompare_computed_results_triggered();

	void on_pushButton_clicked();

	void on_comboBox_highlighted(const QString &arg1);

	void on_comboBox_currentIndexChanged(const QString &arg1);

	void showCPUMethodInformation(const QString &arg1);

	void clearLayout(QLayout* layout, bool deleteWidgets = true);

	void on_pushButton_2_clicked(bool checked);

private:
	Ui::MainWindow *ui;

	QLineEdit * infoSatellite;
	QDoubleSpinBox *cr, *area;
	QSpinBox *nx,*ny,*numSecondaryRays, *numDiffuseRays;
	QRadioButton *specularReflection, *diffuseReflection;

	QString nameMtl,nameObj;

	CategoryResult fieldResults[MAX_SRP_MODELS];

	SRP *srp[MAX_SRP_MODELS];
	SRPModel model;

	std::vector<GLWindow*> glVisualizations;
	std::vector<VisualizationWindow *> dataVisualizations;
	std::vector<ComparisonWindow*> comparisonVisualizations;

	QString getNameFromModel(SRPModel model);
	void loadUserParameters();
	void hideAdvanceSettings();
	int errorCode;
};

#endif // MAINWINDOW_H
