#include "output.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

QVector3D Output::getForces() const
{
	return forces;
}

double Output::getAzimuth() const
{
	return azimuth;
}

double Output::getElevation() const
{
	return elevation;
}

Output::Output()
{
}

Output::Output(double az, double el, double ffx, double ffy, double ffz)
{
	azimuth = az;
	elevation = el;
	forces = QVector3D(ffx,ffy,ffz);
}

QVector3D Output::getOutputWithMeanForces()
{
	return QVector3D(elevation,forces.length(),azimuth);
}

QVector3D Output::getOutputWithForceX()
{
	return QVector3D(elevation,forces.x(),azimuth);
}

QVector3D Output::getOutputWithForceY()
{
	return QVector3D(elevation,forces.y(),azimuth);
}

QVector3D Output::getOutputWithForceZ()
{
	return QVector3D(elevation,forces.z(),azimuth);
}

void Output::save(FILE *fo)
{
	fprintf(fo, " %7.2lf %7.2lf  %22.10e %22.10e %22.10e\n", azimuth, elevation, forces.x(), forces.y(), forces.z());
}
