#ifndef GLWINDOW_H
#define GLWINDOW_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <QWidget>
#include <QtWidgets>
#include "glwidget.h"
#include "SRP/srp.h"
#include "unordered_set"
#define MAX_AXIS 3

enum Axis{_X_=0,_Y_=1,_Z_=2};

/*
 * This class creates a window that allows the user interact with the satellite and individual forces.
 */
class GLWindow: public QWidget
{
	struct AxisWidgets{
		QCheckBox *checkBox;
		QSpinBox *spinBox;
		QSlider *slider;
	};

	GLWidget *glWidget;
	AxisWidgets axes[MAX_AXIS];
	QToolButton *lockButton, *unlockButton;
	QLabel* minValue,*maxValue;

	std::unordered_set<long int> createdLineForces;

	SRP* model;

public:
	GLWindow(QWidget *parent = 0);
	virtual ~GLWindow();
	void setSatellite(Object* satellite);
	void drawSatellite();

	SRP *getModel() const;
	void setModel(SRP *value);

private:
	void generateAxisInformation(QHBoxLayout *axisLayoutContainer,QHBoxLayout *axisLayout,QLabel *labelAxisName,
			QString text, QFont font1,AxisWidgets axis);

private slots:
	void lockAxis();
	void addForceSRP();
	void rotateSatellite();
	void clearLineForces();
};

#endif // GLWINDOW_H
