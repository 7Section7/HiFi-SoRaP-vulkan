#ifndef RESULT_H
#define RESULT_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "DataVisualization/grid.h"
#include <QString>

/*
 * This class stores the output of a method for each azimuth and elevation.
 */

class Result
{
	Grid *output;
	QString name;

public:
	Result();

	QString getName() const;
	void setName(const QString &value);
	Grid *getOutput() const;
	void setOutput(Grid *value);
};

#endif // RESULT_H
