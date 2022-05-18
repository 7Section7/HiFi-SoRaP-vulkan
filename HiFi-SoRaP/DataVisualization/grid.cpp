#include "grid.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

Grid::Grid(int sizeX, int sizeZ):sizeX(sizeX),sizeZ(sizeZ)
{
	grid.resize(sizeX*sizeZ);
	minF =  std::numeric_limits<double>::infinity();
	minFx = std::numeric_limits<double>::infinity();
	minFy = std::numeric_limits<double>::infinity();
	minFz = std::numeric_limits<double>::infinity();

	maxF =  -std::numeric_limits<double>::infinity();
	maxFx = -std::numeric_limits<double>::infinity();
	maxFy = -std::numeric_limits<double>::infinity();
	maxFz = -std::numeric_limits<double>::infinity();
}

Output Grid::operator ()(int i, int j) const
{
	return grid[i*sizeZ+j];
}

Output &Grid::operator ()(int i, int j)
{
	return grid[i*sizeZ+j];
}

QVector2D Grid::getSizes()
{
	return QVector2D(sizeX,sizeZ);
}

void Grid::saveData(int AZstep, int ELstep, std::string output)
{
	FILE *fo;

	char * fileName =(char *)output.c_str();
	fo = fopen(fileName, "w");
	if(fo == NULL)
	{	printf("Problems opening output file %s\n", fileName);
		return;
	}

	int xpix =-1;
	QVector3D cm(0,0,0);
	fprintf(fo, "Version: Cannonball\n");
	//fprintf(fo, "System: %s \n", output);
	fprintf(fo, "Analysis Type      : Area\n");
	fprintf(fo, "Reflections        : 1\n");
	fprintf(fo, "Pixel Size    : %lf \n",xpix);
	fprintf(fo, "Spacecraft Size : 1 \n");
	fprintf(fo, "Pressure           : 1\n");
	fprintf(fo, "Center of Mass     :  (%lf, %lf, %lf)\n",cm.x(),cm.y(),cm.z());
	fprintf(fo, "Current time       : 00 00 00 00000\n");

	fprintf(fo, "\n");

	fprintf(fo, "Motion    : 1\n");
	fprintf(fo, "  Name    : Azimuth\n");
	fprintf(fo, "  Method  : Step\n");
	fprintf(fo, "  Minimum : -180\n");
	fprintf(fo, "  Maximum : 180\n");
	fprintf(fo, "  Step    : %d\n", AZstep);

	fprintf(fo, "Motion    : 2\n");
	fprintf(fo, "  Name    : Elevation\n");
	fprintf(fo, "  Method  : Step\n");
	fprintf(fo, "  Minimum : -90\n");
	fprintf(fo, "  Maximum : 90\n");
	fprintf(fo, "  Step    : %d\n", ELstep);
	fprintf(fo, ": END\n");
	fprintf(fo, "\n");

	fprintf(fo, "Record count       : %d\n", (AZstep>0?360/AZstep:0+1)*(ELstep>0?180/ELstep:0+1) );
	fprintf(fo, "\n");

	fprintf(fo, " AzimuthElevatio  Force(X)  Force(Y)  Force(Z) \n");
	fprintf(fo, " degrees degrees       m^2       m^2       m^2  \n");
	fprintf(fo, " ------- ------- --------- --------- --------- \n");

	save(fo);

	fprintf(fo, " ------- ------- --------- --------- --------- \n");

	fclose(fo);
}

void Grid::save(FILE *fo)
{
	for(int i=0; i< sizeX; i++){
		for(int j=0; j< sizeZ; j++){
			(*this)(i,j).save(fo);
		}
	}
}

void Grid::updateExtremeValues()
{
	for(int i=0; i< sizeX; i++){
		for(int j=0; j< sizeZ; j++){
			QVector3D force =(*this)(i,j).getForces();

			if(force.x() < minFx) minFx = force.x();
			if(force.y() < minFy) minFy = force.y();
			if(force.z() < minFz) minFz = force.z();

			if(force.x() > maxFx) maxFx = force.x();
			if(force.y() > maxFy) maxFy = force.y();
			if(force.z() > maxFz) maxFz = force.z();

			if(force.length() < minF) minF = force.length();
			if(force.length() > maxF) maxF = force.length();
		}
	}
}

Grid *Grid::getNormalizedOutput()
{
	Grid *normGrid = new Grid(sizeX,sizeZ);
	for(int i=0; i< sizeX; i++){
		for(int j=0; j< sizeZ; j++){
			Output o= (*this)(i,j);
			QVector3D f = o.getForces();
			double x = (maxFx-f.x())/(maxFx-minFx);
			double y = (maxFy-f.y())/(maxFy-minFy);
			double z = (maxFz-f.z())/(maxFz-minFz);
			Output newOutput(o.getAzimuth(),o.getElevation(),x,y,z);
			(*normGrid)(i,j) = newOutput;
		}
	}
	return normGrid;
}
