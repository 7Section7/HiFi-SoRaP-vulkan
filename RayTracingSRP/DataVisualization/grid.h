#ifndef GRID_H
#define GRID_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include <vector>
#include <QVector3D>
#include <QVector2D>
#include "output.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * This class stores the resultant SRP force for each azimuth and elevation.
 */
class Grid
{
	std::vector<Output> grid;
	int sizeX,sizeZ;

public:
	Grid(int sizeX, int sizeZ);
	Output operator ()(int i,int j) const;
	Output & operator ()(int i,int j);

	QVector2D getSizes();

	void saveData(int AZstep, int ELstep,std::string output);

	void save(FILE* fo);

	void updateExtremeValues();

	Grid *getNormalizedOutput();

	double minFx,minFy,minFz,minF;
	double maxFx,maxFy,maxFz,maxF;
};

#endif // GRID_H
