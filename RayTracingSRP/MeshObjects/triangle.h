#ifndef TRIANGLE_H
#define TRIANGLE_H

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

/*
 * This class contains the information of a triangle .
 */
class Triangle
{
public:
	int v1, v2, v3;
	int rf;
	int nn;
	double A;

	Triangle();
};

#endif // TRIANGLE_H
