#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <QtDataVisualization>
#include <QHBoxLayout>
#include <QWidget>
#include <vector>
#include "grid.h"
#include <QLabel>
#include <QToolButton>
#include <QFileDialog>

#define MAX_SUFACES 4

using namespace QtDataVisualization;
using namespace std;

enum SRPModel{CannonBallModel,NPlateModel,RayTraceCPUModel,AdvancedGPUModel};
enum VisualizationMode{MeanF=0,Fx=1,Fy=2,Fz=3};

/*
 * This class creates a window that shows the 4 3D viewers that explain the obtained forces.
 */

class VisualizationWindow: public QWidget
{
	struct Surface{
		Q3DSurface surface;
		QWidget *container;
		QVBoxLayout *vLayout;
	};

	QWidget *widget;
	QGridLayout * gLayout;
	QVBoxLayout *vLayout;

	int ELstep, AZstep;

	Grid *results;

	void plotSurface(Grid * results,  VisualizationMode mode);

public:
	VisualizationWindow(QWidget *parent = 0);
	~VisualizationWindow();
	void plotResults(Grid *results,  QString name);

	Surface surfaces[MAX_SUFACES];

	void setAZstep(int value);

	void setELstep(int value);

public slots:
	void saveResults();
};

#endif // VISUALIZATIONWINDOW_H
