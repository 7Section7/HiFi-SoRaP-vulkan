#include "raytracecpu.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include<QElapsedTimer>
#include<iostream>
#include<fstream>
#include <iomanip>

#include <cstdint> // for specific size integers
//#include <fstream> // for file handling

#include <chrono>
#include <ctime>

#define FLOAT_EPSILON 1.192093e-7f

// Random number constants:
#define A 16807
#define M 2147483647
#define Q 127773
#define R 2836

using namespace std;

namespace
{

//! Reflects the 'dir' vector along the 'normal' vector as if it was mirror-like.
vector3 reflect(const vector3& dir,const vector3& normal);

//methods to compute SRP in the same way as RayTraceGPU
vector3 product(vector3 a, vector3 b);
bool isnan_float (float* f);

/*
//! Refracts the 'dir' vector along the 'normal' vector by considering the refractive index
bool refract(const vector3& dir, const vector3& normal, const precision::value_type& refractiveIndex,
		vector3& refracted);
*/

} //anonymous namespace

RayTraceCPU::RayTraceCPU()
{
	reflectionType=Reflective;
}

void RayTraceCPU::computeStepSRP(const vector3& XS, vector3 &force, const vector3& V1, const vector3& V2)
{
	precision::value_type xtot, ytot, xpix, ypix, d, Apix;
	int ix,iy;

	const precision::value_type PS = PRESSURE;

	auto *mesh = satellite->getMesh();
	Eigen::Vector3f diff = mesh->max_-mesh->min_;
	float diagonalDiff = diff.norm();
	float errorMargin = 0.1f;
	float distanceWindow = diagonalDiff+errorMargin;

	xtot = distanceWindow;
	ytot = xtot;
	xpix = xtot/nx;
	ypix = ytot/ny;
	Apix = xpix * ypix;

	d=diagonalDiff;

	force = vector3{0.L};

	vector3 compensationTerm{0.L};
	vector3 previousForce;

	const auto now = std::chrono::system_clock::now();
	//const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
	const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	const auto fraction = now - seconds;
	const auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(fraction).count();

	// http://en.cppreference.com/w/cpp/chrono/c/time
	const std::time_t currentNow = std::time(nullptr) ; // get the current time point
	// convert it to (local) calendar time
	// http://en.cppreference.com/w/cpp/chrono/c/localtime
	const std::tm calendarTime = *std::localtime( std::addressof(currentNow) ) ;
	const auto secs      = calendarTime.tm_sec;
	const auto mins      = calendarTime.tm_min;
	const auto hours     = calendarTime.tm_hour;
	const int time_seed = millisecs + 1000*(secs + 60*(mins + 60*(hours)));

	/* For each pixel in the array */
	for(ix = 1; ix <= nx; ix++)
	{
		for(iy = 1; iy <= ny; iy++)
		{ 
			int idx_seed = (((ix-1)*nx + iy)*1000) % (M/100);
			seed = time_seed + idx_seed;

			vector3 F{0.L,0.L,0.L};
			/* pixel (ix,iy) in the grid */
			const auto pixelPosition = ((ix-0.5L)*xpix - xtot/2.L)*V1 + ((iy-0.5L)*ypix - ytot/2.L)*V2 + d*(-XS); //*RS

			//int triangleIdx; F = computeFinalForce(pixelPosition, XS, triangleIdx);
			//if(triangleIdx != -1)
			const auto hasHit = computePixelForce(XS,pixelPosition,F);
            std::cout << F << " ";
			if(hasHit)
			{
				const auto fixedForce = F - compensationTerm;
				const auto accumulatedFixedForce = force + fixedForce;
				compensationTerm = (accumulatedFixedForce - force) - fixedForce;
				force = accumulatedFixedForce;

				previousForce = force;

				// previously
				//force += F;
			}
		}
	}

	// In this approach 'Apix' corresponds already to the projected area.
	force *= PS*Apix/msat;
}

RayTraceCPU::HasHit RayTraceCPU::computePixelForce(const vector3& XS, const vector3& pixelPosition, vector3& pixelForce)
{
	return rayTrace(pixelPosition,XS,numSecondaryRays, pixelForce);
}

RayTraceCPU::HasHit RayTraceCPU::rayTrace(const vector3& pixelPosition, const vector3& XS, const int numSecondaryRays,
		vector3& pixelForce)
{
	vector3 hitPoint;
	const auto hitTriangleIdx = hit(pixelPosition,XS,hitPoint);

	if(hitTriangleIdx != -1)
	{
		const auto totalForce = computeForce(hitTriangleIdx,XS);

		if(numSecondaryRays == 0){
			pixelForce = totalForce;
			return HIT;
		}
		const auto triangle = getSatellite()->getMesh()->faces[hitTriangleIdx];
		const auto material = getSatellite()->getMaterial(triangle.rf);

		//PS contribution
		vector3 forcePS{0.L,0.L,0.L};
		if(reflectionType!=Lambertian){
			vector3 psDir, psHitPoint, localPS{0.L,0.L,0.L};

			psDir = XS;
			psHitPoint = hitPoint;

			scatter(psHitPoint,hitTriangleIdx,psDir, Reflective);
			rayTrace(psHitPoint,psDir,numSecondaryRays-1,localPS);

			forcePS = material.ps * localPS;
		}

		//PD contribution
		vector3 forcePD{0.L,0.L,0.L};
		uint kernelSize = numDiffuseRays;
		for(uint i=0; i< kernelSize; i++){
			vector3 pdDir, pdHitPoint, localForcePD{0.L,0.L,0.L};
			pdDir = XS;
			pdHitPoint = hitPoint;

			scatter(pdHitPoint,hitTriangleIdx,pdDir, Lambertian);
			rayTrace(pdHitPoint,pdDir,numSecondaryRays-1, localForcePD);

			forcePD += localForcePD;
		}
		if(kernelSize>0)
			forcePD = (material.pd/kernelSize) * forcePD;

		pixelForce = totalForce + (forcePS + forcePD);
		return HIT;
	}
	else{
		return NO_HIT;
	}
}

vector3 RayTraceCPU::computeForce(const int triangleIdx, const vector3& XS)
{
	auto *mesh = satellite->getMesh();
	precision::value_type ps, pd;
	const auto& N = mesh->faceNormals[mesh->faces[triangleIdx].nn];

	const auto costh = Common::dot(XS,N);

	ps = satellite->getMaterial(mesh->faces[triangleIdx].rf).ps;
	pd = satellite->getMaterial(mesh->faces[triangleIdx].rf).pd;

	auto F = ((1.L - ps)*XS - 2.L*(ps*fabs(costh) + pd/3.L)*N);
	return F;
}

void RayTraceCPU::scatter(vector3& hitPoint, int triangleIdx, vector3& XS, Reflectiveness r)
{
	TriangleMesh *mesh = satellite->getMesh();
	Material m = satellite->getMaterial(mesh->faces[triangleIdx].rf);

	const auto& N = mesh->faceNormals[mesh->faces[triangleIdx].nn];

	if(r == Reflective)
	{
		const auto reflected = Common::normalize(reflect(XS, N));

		hitPoint = hitPoint + 1.e-4L*reflected;
		XS = reflected;
	}
	else if(r == Lambertian)
	{
		const auto target = hitPoint+N+randomInSphere();
		const auto reflected = Common::normalize(target-hitPoint);

		hitPoint = hitPoint + 1.e-4L*reflected;
		XS = reflected;
	}
}

int RayTraceCPU::hit(const vector3& pixelPosition, const vector3& XS, vector3& closestHitPoint)
{
	precision::value_type depmin, dephit;
	uint triangleIdx;
	int triangleIdxHit = -1; //If the ray doesn't hit any triangle, the returned triangle index is -1.

	auto *mesh = satellite->getMesh();
	const auto numFaces = mesh->faces.size();

	depmin = std::numeric_limits<precision::value_type>::infinity();

	const auto L = Common::normalize(XS);
	vector3 hitPoint;

	for(triangleIdx = 0; triangleIdx < numFaces; triangleIdx++)
	{
		if( mesh->hitTriangle(pixelPosition, L, triangleIdx, hitPoint) )
		{
			dephit = Common::length(pixelPosition-hitPoint);

			if(dephit<depmin)
			{
				depmin=dephit;
				triangleIdxHit = triangleIdx;
				closestHitPoint = hitPoint;
			}
		}
	}
	return triangleIdxHit;
}

struct RayTraceStep
{
	HitInfo hitInfo;

	vector3 force, forcePS, forcePD, localForcePD;
	int currentStep;
	int currentComputation;
};

void copyHitInfo(const HitInfo& hitInfo, HitInfo& outHitInfo)
{
	outHitInfo.currentImportance=hitInfo.currentImportance;
	outHitInfo.currentL = hitInfo.currentL;
	outHitInfo.currentN = hitInfo.currentN;
	outHitInfo.currentPS = hitInfo.currentPS;
	outHitInfo.currentPD = hitInfo.currentPD;
	outHitInfo.currentRefIdx = hitInfo.currentRefIdx;
	outHitInfo.currentReflectiveness= hitInfo.currentReflectiveness;
	outHitInfo.currentPoint= hitInfo.currentPoint;
}

//Check if the ray intersects any triangle and get the closest one in order
//to compute the force on the intersection.
vector3 RayTraceCPU::computeScatteredForce(HitInfo& hitInfo)
{
	vector3 totalForce = vector3(0,0,0);
	vector3 importance = hitInfo.currentImportance;
	scattering(hitInfo);
	vector3 hitPoint;

	int triangleIdx = hit(hitInfo.currentPoint,hitInfo.currentL,hitPoint);
	if(triangleIdx!=-1)
	{
		//hitInfo.currentImportance = product(hitInfo.currentImportance,importance);
		//vector3 localForce = computeForce(triangleIdx,hitInfo.currentL);
		//totalForce+= product(localForce,hitInfo.currentImportance);
		totalForce = computeForce(triangleIdx,hitInfo.currentL);

		const auto mesh = getSatellite()->getMesh();
		Triangle t = mesh->faces[triangleIdx];

		hitInfo.currentN = mesh->faceNormals[t.nn];

		Material m = getSatellite()->getMaterial(t.rf);
		hitInfo.currentPS = m.ps;
		hitInfo.currentPD = m.pd;
		hitInfo.currentRefIdx = m.refIdx;

		hitInfo.currentPoint = hitPoint;
	}
	else
	{
		hitInfo.currentImportance=importance;
	}

	return totalForce;
}

//This computes the SRP force for the fragment.
//In particular, it casts several additional rays when one of the primary
//rays intersects with the mesh.
vector3 RayTraceCPU::computeFinalForce(const vector3& point, const vector3& L, int& triangleIdx)
{
	vector3 totalForce;

	vector3 currentN;
	vector3 currentL = normalize(L);
	vector3 currentPoint = point;
	float currentPs,currentPd,currentRefIdx;
	int currentReflectiveness;
	vector3 hitPoint;

	vector3 importance = vector3(1.f,1.f,1.f);

	vector3 pointAux = point;
	triangleIdx = hit(pointAux,currentL,currentPoint);
	if(triangleIdx==-1) return vector3(0,0,0);

	const auto mesh = getSatellite()->getMesh();

	Triangle t = mesh->faces[triangleIdx];
	Material m = getSatellite()->getMaterial(t.rf);

	currentN = mesh->faceNormals[t.nn];

	currentPs = m.ps;
	currentPd = m.pd;
	currentRefIdx = m.refIdx;
	currentReflectiveness = m.r;

	totalForce = computeForce(triangleIdx,currentL);

	if(numSecondaryRays == 0)
		return totalForce;

	//For optimization, we decided to create a stack 'steps'
	//in order to save the accumulated forces of each iteration.
    //RayTraceStep steps[numSecondaryRays];  // C++ variable arrays not portable (MSVC fails to compile)
    std::vector<RayTraceStep> steps(numSecondaryRays);


	for(uint i=0; i< numSecondaryRays; i++)
	{
		steps[i].currentStep=i;
		steps[i].currentComputation=0;
		steps[i].force =vector3(0,0,0);
		steps[i].hitInfo.currentReflectiveness=0;
		steps[i].forcePS = vector3(0,0,0);
		steps[i].forcePD = vector3(0,0,0);
		steps[i].localForcePD = vector3(0,0,0);
	}
	steps[0].hitInfo.currentImportance=importance;
	steps[0].hitInfo.currentL=L;
	steps[0].hitInfo.currentPoint=currentPoint;
	steps[0].hitInfo.currentPS=currentPs;
	steps[0].hitInfo.currentPD=currentPd;
	steps[0].hitInfo.currentN =currentN;

	int diffuseRays = numDiffuseRays;

	bool stop = false;
	int i=0;
	int currentStep=0;
	while(!stop && i<1000) //i is a timeout counter.
	{
		if(steps[currentStep].currentComputation<1+diffuseRays)
		{
			//Get the information from the previous iteration.
			HitInfo hitInfoAux;
			copyHitInfo(steps[currentStep].hitInfo,hitInfoAux);

			//Depending on the current step of computation we are in,
			//we compute a secondary ray based on specular or diffuse reflection.
			int r;
			if(steps[currentStep].currentComputation<1) { r=0; }
			else { r=2; }

			//Compute force on the intersection of the ray and add it to the accumulated force.
			hitInfoAux.currentReflectiveness=r;
			vector3 f = computeScatteredForce(hitInfoAux);

			if(steps[currentStep].currentComputation<1)
			{ steps[currentStep].forcePS = f; }
			else
			{ steps[currentStep].localForcePD = f;	}

			if( length(f)<FLOAT_EPSILON || currentStep == numSecondaryRays-1 )
			{
				//There was no hit for this secondary ray or we reach the maximum deep raytracing level,
				//so we move forward on the next secondary ray computation. (next iteration).
				if(steps[currentStep].currentComputation >= 1)
				{ steps[currentStep].forcePD += steps[currentStep].localForcePD; }

				steps[currentStep].currentComputation++;
			}
			else
			{
                //Or, the secondary ray hit a triangle, then we need to compute new secondary rays from this intersected point.
                copyHitInfo(hitInfoAux, steps[currentStep+1].hitInfo);
				steps[currentStep+1].currentComputation=0;
				steps[currentStep+1].force=vector3(0,0,0);
				steps[currentStep+1].forcePS=vector3(0,0,0);
				steps[currentStep+1].forcePD=vector3(0,0,0);
				steps[currentStep+1].localForcePD=vector3(0,0,0);
				steps[currentStep+1].hitInfo.currentReflectiveness=0;

				currentStep++;
			}
		}
		else//Move backward.
		{
			//Once all the secondary rays were computed, we move backward in the stack.
			float ps = steps[currentStep].hitInfo.currentPS;
			float pd = steps[currentStep].hitInfo.currentPD;
			steps[currentStep].force = ps*steps[currentStep].forcePS;
			if(diffuseRays > 0) steps[currentStep].force += (pd/diffuseRays)*steps[currentStep].forcePD;

			if(currentStep==0)  stop=true;
			else
			{
				if(steps[currentStep-1].currentComputation<1)
				{ steps[currentStep-1].forcePS += steps[currentStep].force; }
				else
				{
					steps[currentStep-1].localForcePD += steps[currentStep].force;
					steps[currentStep-1].forcePD += steps[currentStep-1].localForcePD;
				}

				currentStep--;
				steps[currentStep].currentComputation++;
			}
		}

		i++;
	}

	totalForce += steps[0].force;

	return totalForce;
}

void RayTraceCPU::scattering(HitInfo& hitInfo)
{
	vector3 point = hitInfo.currentPoint;
	vector3 dir = hitInfo.currentL;
	float ps=hitInfo.currentPS;
	float pd = hitInfo.currentPD;
	float refIdx = hitInfo.currentRefIdx;
	int reflectiveness = hitInfo.currentReflectiveness;
	vector3 N = hitInfo.currentN;

	if(reflectiveness == 0) //Reflective
	{
		vector3 reflected = normalize(reflect(dir,N));

		point = point + 1.e-4f*reflected;

		hitInfo.currentPoint = point;
		hitInfo.currentL=reflected;
		hitInfo.currentImportance = vector3(ps,ps,ps);
	}
	else //Lambertian
	{
		vector3 target = point + N + randomInSphere();
		vector3 reflected = normalize(target - point);

		point = point + 1.e-4f*reflected;

		hitInfo.currentPoint = point;
		hitInfo.currentL=reflected;
		hitInfo.currentImportance = vector3(pd,pd,pd);
	}
}

vector3 RayTraceCPU::randomInSphere()
{
	vector3 p;

	/* OPTION 1: discarding til norm < 1
	do {
		p = 2.0L*getRandomVector() - vector3(1.L,1.L,1.L);
	} while (Common::length(p) >=  1.0L);
	*/

	//OPTION 2: Polar Coordinates Improved
	double theta = 2 * M_PI * getRandomValue();
	// wrong: double phi = M_PI * uniform01(generator);
	double phi = acos(1 - 2 * getRandomValue());
	double x = sin(phi) * cos(theta);
	double y = sin(phi) * sin(theta);
	double z = cos(phi);

	p = vector3(x,y,z);

	return p;
}

vector3 RayTraceCPU::getRandomVector()
{
	const vector3 randomVector(getRandomValue(),getRandomValue(),getRandomValue());
	return randomVector;
}

float RayTraceCPU::getRandomValue()
{
	int newSeed = A*(seed % Q) - R*(seed/Q);
	if(newSeed < 0) newSeed += M;

	seed = newSeed;

	float output = float(newSeed)/float(M);

	return output;
}

uint RayTraceCPU::getNumSecondaryRays() const
{
	return numSecondaryRays;
}

void RayTraceCPU::setNumSecondaryRays(const uint value)
{
	numSecondaryRays = value;
}

uint RayTraceCPU::getNumDiffuseRays() const
{
	return numDiffuseRays;
}

void RayTraceCPU::setNumDiffuseRays(const uint value)
{
	numDiffuseRays = value;
}

uint RayTraceCPU::getReflectionType() const
{
	return reflectionType;
}

void RayTraceCPU::setReflectionType(const uint value)
{
	reflectionType = value;
}

namespace
{

bool isnan_float (float* f) { return (f != f); };

vector3 reflect(const vector3& dir,const vector3& normal)
{
	return Common::normalize(dir-2.0L*Common::dot(dir,normal)*normal);
}

/*bool refract(const vector3& dir, const vector3& normal, const precision::value_type& refractiveIndex,
		vector3& refracted)
{
	const auto uv = Common::normalize(dir);
	const auto dt = Common::dot(uv, normal);
	const auto discriminant = 1.0L - refractiveIndex*refractiveIndex*(1-dt*dt);
	if (discriminant > 0) {
		refracted = refractiveIndex*(uv - normal*dt) - normal*sqrt(discriminant);
		return true;
	}
	else
		return false;
}*/

vector3 product(vector3 a, vector3 b)
{
	return vector3(a.x*b.x,a.y*b.y,a.z*b.z);
}

} //anonymous namespace
