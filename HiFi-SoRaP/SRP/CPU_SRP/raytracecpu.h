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

/*
 * This class implements the RayTrace (CPU) method.
 */
class RayTraceCPU: public AdvancedSRP
{
	uint numSecondaryRays;
	uint numDiffuseRays;
	uint reflectionType;
	float seed;

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
	HasHit rayTrace(const vector3& pixelPosition, const vector3& XS, const vector3& importance,
			const int numSecondaryRays, vector3& pixelForce);
	vector3 computeForce(const int triangleIdx, const vector3& XS);

	void scatter(vector3& hitPoint, int triangleIdx, vector3& XS, vector3& importance, Reflectiveness r);
	int hit(const vector3& pixelPosition, const vector3& XS, vector3& closestHitPoint);
};

#endif // RAYTRACECPU_H
