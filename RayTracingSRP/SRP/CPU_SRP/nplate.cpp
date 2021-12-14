#include "nplate.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 * Version: 2.0.1
 *
 ***********************************************************************/

NPlate::NPlate()
{
}

void NPlate::computeStepSRP(double xs[], QVector3D &force, double RS[], double V1[], double V2[])
{
	double fs[3];
	double PS = ctt/msat;

	fsrp_nplate(np, xs, fs);
	force = PS*QVector3D(fs[0],fs[1],fs[2]); //PS*
}

void NPlate::fsrp_nplate(int N, double xs[], double fs[])
/*
   SRP force for the N-plate model

  INPUT:
		N 			- integer with number of plates
	  n[][]  	-  N x 3 table with normal vector to each plate
	  ps[]   	-  N array with coefficients for secular reflection
	  pd[]   	-  N array with coefficients for difusive reflection
	  A[]    	-  N array with Area of each plate
	  xs[]  	-  3 array with sun-satellite direction

   OUTPUT:
	  fs[]  - resultat srp force
*/
{
	int i, ip;
	double cosTH;

	fs[0] = 0., fs[1] = 0., fs[2] = 0.;
	for(ip = 0; ip < N; ip++)
	{
		cosTH = n[ip][0]*xs[0] + n[ip][1]*xs[1] + n[ip][2]*xs[2];

		/* ilumination condition */
		if(cosTH < 0)
		{
			for(i = 0; i < 3; i++)
			fs[i] += A[ip]*( (1.-ps[ip])*xs[i] + 2.*(ps[ip]*cosTH + pd[ip]/3.)*n[ip][i] )*cosTH;
		}
	}
	return ;
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
		printf("problems opening input spacecraft file (check file %s) \n", fileInput);
		return;
	}

	qq = fscanf(fi, "%d", &np);
	if(qq != 1){ printf(" incompatible input format (1)\n"); exit(1); }

	for(i = 0; i < np; i++){

		double aux_A, aux_ps, aux_pd, aux_n0, aux_n1, aux_n2;
		// IMPORTANT: Use ',' (not '.') to describe decimals
		int qq2 = fscanf(fi, "%lf %lf %lf %lf %lf %lf", &aux_A, &aux_ps, &aux_pd, &aux_n0, &aux_n1, &aux_n2);

		A.push_back(aux_A);
		ps.push_back(aux_ps);  pd.push_back(aux_pd);
		n.push_back(QVector3D(aux_n0,aux_n1,aux_n2));

		if(qq2 != 6){
			printf(" incompatible input format (2)\n");
			exit(1);
		}
	}
	fclose(fi);
}
