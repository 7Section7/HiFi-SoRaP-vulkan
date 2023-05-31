#include "nplate.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

NPlate::NPlate()
{
}

void NPlate::computeStepSRP(const vector3& XS, vector3 &force, const vector3& V1, const vector3& V2)
{
	Q_UNUSED(V1);
	Q_UNUSED(V2);

	precision::value_type PS = PRESSURE;

	fsrp_nplate(XS, force);
	force = PS/msat*force;
}

void NPlate::fsrp_nplate(const vector3& XS, vector3 &force)
/*
   SRP force for the N-plate model

  INPUT:
	np       - integer with number of plates
	n[][]   -  N x 3 table with normal vector to each plate
	ps[]   	-  N array with coefficients for secular reflection
	pd[]   	-  N array with coefficients for difusive reflection
	A[]    	-  N array with Area of each plate
	xs[]  	-  3 array with sun-satellite direction

   OUTPUT:
	fs[]    - resultat srp force
*/
{
	int ip;
	precision::value_type cosTH;

	force = vector3();
	for(ip = 0; ip < np; ip++)
	{
		cosTH = Common::dot(n[ip],XS);

		/* ilumination condition */
		if(cosTH < 0)
			force += A[ip]*fabs(cosTH)*((1.L-ps[ip])*XS - 2.L*(ps[ip]*fabs(cosTH) + pd[ip]/3.L)*n[ip]);
	}
	return;
}

bool NPlate::isSatelliteInfoLoaded()
{
	return A.size() > 0;
}

void NPlate::loadSatelliteInfo()
{
	FILE *fi;
	int qq, i;
	A.clear();
	ps.clear();
	pd.clear();
	n.clear();

	std::string file = satelliteInfoFile.toStdString();
	char * fileInput=(char*)file.c_str();

	fi = fopen(fileInput, "r");
	if(fi == NULL)
	{
		throw std::invalid_argument(std::string("Problems opening input spacecraft file (check file ")+ fileInput +" ) \n");
	}

	qq = fscanf(fi, "%d", &np);
	if(qq != 1)
	{
		throw std::invalid_argument("Problems loading number of plates.");
	}

	for(i = 0; i < np; i++)
	{
		double aux_A, aux_ps, aux_pd, aux_n0, aux_n1, aux_n2;
		// IMPORTANT: Use ',' (not '.') to describe decimals
		int qq2 = fscanf(fi, "%lf %lf %lf %lf %lf %lf", &aux_A, &aux_ps, &aux_pd, &aux_n0, &aux_n1, &aux_n2);

		A.push_back(aux_A);
		ps.push_back(aux_ps);  pd.push_back(aux_pd);
		n.push_back(vector3(aux_n0,aux_n1,aux_n2));

		if(qq2 != 6)
		{
			throw std::invalid_argument("Depending on your OS, you may need to use ',' instead of '.' to describe "
										"decimals (or viceversa).");
		}
	}
	fclose(fi);
}
