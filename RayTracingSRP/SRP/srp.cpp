#include "srp.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

int SRP::getAzimuthStep() const
{
	return step_AZ;
}

void SRP::setAzimuthStep(int value)
{
	step_AZ = value;
}

int SRP::getElevationStep() const
{
	return step_EL;
}

void SRP::setElevationStep(int value)
{
	step_EL = value;
}

std::string SRP::getOutput() const
{
	return output;
}

void SRP::setOutput(const std::string &value)
{
	output = value;
}

float SRP::getMsat() const
{
	return msat;
}

void SRP::setMsat(float value)
{
	msat = value;
}

Object *SRP::getSatellite() const
{
	return satellite;
}

void SRP::setSatellite(Object *value)
{
	satellite = value;
}

SRP::SRP()
{
	stopExecution = false;
}

void SRP::saveResults(Grid *results){
	QVector3D origin(0,0,0);
	saveResultsToFile(-1,origin,results);
}

QProgressBar *SRP::getProgressBar() const
{
	return progressBar;
}

void SRP::setProgressBar(QProgressBar *value)
{
	progressBar = value;
}

bool SRP::getStopExecution() const
{
	return stopExecution;
}

void SRP::setStopExecution(bool value)
{
	stopExecution = value;
}

void SRP::saveResultsToFile(float xpix, QVector3D& cm, Grid* results){
	int AZstep, ELstep;
	FILE *fo;


	char * fileName =(char *)output.c_str();
	fo = fopen(fileName, "w");
	if(fo == NULL)
	{	printf("Problems opening output file %s\n", fileName);
		return;
	}

	AZstep = step_AZ;//atoi(argv[3]);
	ELstep = step_EL;//atoi(argv[4]);

	fprintf(fo, "Version: Cannonball\n");
	//fprintf(fo, "System: %s \n", output);
	fprintf(fo, "Analysis Type      : Area\n");
	fprintf(fo, "Reflections        : 1\n");
	fprintf(fo, "Pixel Size    : %lf \n",xpix);
	fprintf(fo, "Spacecraft Size : NAN \n");
	fprintf(fo, "Pressure           : 1\n");
	fprintf(fo, "Center of Mass     :  (%lf, %lf, %lf)\n",cm.x(),cm.y(),cm.z());
	fprintf(fo, "Current time       : xx xx xx xxxxx\n");

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

	fprintf(fo, "Record count       : %d\n", (360/AZstep+1)*(180/ELstep+1) );
	fprintf(fo, "\n");

	fprintf(fo, " AzimuthElevatio  Force(X)  Force(Y)  Force(Z) \n");
	fprintf(fo, " degrees degrees       m^2       m^2       m^2  \n");
	fprintf(fo, " ------- ------- --------- --------- --------- \n");

	results->save(fo);

	fprintf(fo, " ------- ------- --------- --------- --------- \n");

	fclose(fo);
}
