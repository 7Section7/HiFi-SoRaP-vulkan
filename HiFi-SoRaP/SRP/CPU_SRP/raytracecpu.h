#ifndef RAYTRACECPU_H
#define RAYTRACECPU_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "SRP/advancedsrp.h"

struct HitInfo
{
	vector3 currentPoint;
	vector3 currentL;
	vector3 currentN;
	vector3 currentImportance;
	float currentPS;
	float currentPD;
	float currentRefIdx;
	int currentReflectiveness;
};

/*
 * This class implements the RayTrace (CPU) method.
 */
class RayTraceCPU: public AdvancedSRP
{
	uint numSecondaryRays;
	uint numDiffuseRays;
	uint reflectionType;
	int seed;

	enum HasHit : bool {NO_HIT=false, HIT=true};

public:
	RayTraceCPU();
	void computeStepSRP(const vector3& XS, vector3& force, const vector3& V1, const vector3& V2);

	uint getNumSecondaryRays() const;
	void setNumSecondaryRays(const uint value);

	uint getNumDiffuseRays() const;
	void setNumDiffuseRays(const uint value);

	uint getReflectionType() const;
	void setReflectionType(const uint value);

private:
	HasHit computePixelForce(const vector3& XS, const vector3& pixelPosition, vector3& pixelForce);
	HasHit rayTrace(const vector3& pixelPosition, const vector3& XS, const int numSecondaryRays, vector3& pixelForce);
	vector3 computeForce(const int triangleIdx, const vector3& XS);

	void scatter(vector3& hitPoint, int triangleIdx, vector3& XS, Reflectiveness r);
	int hit(const vector3& pixelPosition, const vector3& XS, vector3& closestHitPoint);

	//Computing SRP force in the same way as RayTraceGPU:
	vector3 computeFinalForce(const vector3& point, const vector3& L, int& triangleIdx);
	vector3 computeScatteredForce(HitInfo& hitInfo);
	void scattering(HitInfo& hitInfo);

	vector3 randomInSphere();
	vector3 getRandomVector();
	float getRandomValue();
};

#endif // RAYTRACECPU_H
