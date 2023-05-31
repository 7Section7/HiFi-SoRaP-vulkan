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
using namespace std;

namespace
{

static precision::value_type APIX;

//! Reflects the 'dir' vector along the 'normal' vector as if it was mirror-like.
vector3 reflect(const vector3& dir,const vector3& normal);

//! Returns a random 3D vector of length 1.
vector3 randomInSphere();

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
	APIX = Apix;

	d=diagonalDiff;

	force = vector3{0.L};

	vector3 compensationTerm{0.L};
	vector3 previousForce;

	/* For each pixel in the array */
	for(ix = 2; ix <= nx; ix++)
	{
		for(iy = 3; iy <= ny; iy++)
		{
			vector3 F{0.L,0.L,0.L};
			/* pixel (ix,iy) in the grid */
			const auto pixelPosition = ((ix-0.5L)*xpix - xtot/2.L)*V1 + ((iy-0.5L)*ypix - ytot/2.L)*V2 + d*(-XS); //*RS
			const auto hasHit = computePixelForce(XS,pixelPosition,F);

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
	return rayTrace(pixelPosition,XS,vector3{1.L,1.L,1.L},numSecondaryRays, pixelForce);
}

RayTraceCPU::HasHit RayTraceCPU::rayTrace(const vector3& pixelPosition, const vector3& XS, const vector3& importance,
		const int numSecondaryRays, vector3& pixelForce)
{
	if(numSecondaryRays < 0)
	{
		return NO_HIT;
	}

	if(Common::length(importance)<1.e-5L)
	{
		return NO_HIT;
	}

	vector3 hitPoint;
	const auto hitTriangleIdx = hit(pixelPosition,XS,hitPoint);

	if(hitTriangleIdx != -1)
	{
		auto totalForce = computeForce(hitTriangleIdx,XS)*importance;

		if(numSecondaryRays == 0){
			pixelForce = totalForce;
			return HIT;
		}

		//PS contribution
		vector3 forcePS{0.L,0.L,0.L};
		if(reflectionType!=Lambertian){
			vector3 psImportance, psDir, psHitPoint;
			psDir = XS;
			psHitPoint = hitPoint;

			scatter(psHitPoint,hitTriangleIdx,psDir,psImportance, Reflective);
			psImportance*=importance;

			rayTrace(psHitPoint,psDir,psImportance,numSecondaryRays-1,forcePS);
		}

		//PD contribution
		vector3 forcePD{0.L,0.L,0.L};
		uint kernelSize = numDiffuseRays;
		for(uint i=0; i< kernelSize; i++){
			vector3 pdImportance, pdDir, pdHitPoint;
			pdDir = XS;
			pdHitPoint = hitPoint;

			scatter(pdHitPoint,hitTriangleIdx,pdDir,pdImportance, Lambertian);
			pdImportance*=importance;

			vector3 localForcePD{0.L,0.L,0.L};
			rayTrace(pdHitPoint,pdDir,pdImportance,numSecondaryRays-1, localForcePD);
			forcePD += localForcePD;
		}
		if(kernelSize>0)
			forcePD = 1.0L/kernelSize *forcePD;

		totalForce += forcePS + forcePD;
		pixelForce = totalForce;
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

void RayTraceCPU::scatter(vector3& hitPoint, int triangleIdx, vector3& XS, vector3& importance, Reflectiveness r)
{
	TriangleMesh *mesh = satellite->getMesh();
	Material m = satellite->getMaterial(mesh->faces[triangleIdx].rf);

	const auto& N = mesh->faceNormals[mesh->faces[triangleIdx].nn];

	if(r == Reflective)
	{
		const auto reflected = Common::normalize(reflect(XS, N));

		hitPoint = hitPoint + 1.e-4L*reflected;
		XS = reflected;
		importance = m.ps*vector3{1.L,1.L,1.L};
	}
	else if(r == Lambertian)
	{
		const auto target = hitPoint+N+randomInSphere();
		const auto reflected = Common::normalize(target-hitPoint);

		hitPoint = hitPoint + 1.e-4L*reflected;
		XS = reflected;
		importance = m.pd*vector3{1.L,1.L,1.L};
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

vector3 reflect(const vector3& dir,const vector3& normal)
{
	return Common::normalize(dir-2.0L*Common::dot(dir,normal)*normal);
}

vector3 randomInSphere()
{
	vector3 p;
	do {
#ifdef _WIN32
		p = 2.0L*vector3(qrand(),qrand(),qrand()) - vector3(1.L,1.L,1.L);
#else
		p = 2.0L*vector3(drand48(),drand48(),drand48()) - vector3(1.L,1.L,1.L);
#endif

	} while (Common::length(p) >=  1.0L);
	return p;
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

} //anonymous namespace
