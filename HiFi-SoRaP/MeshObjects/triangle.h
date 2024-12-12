#ifndef TRIANGLE_H
#define TRIANGLE_H

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

#include "Lib/common.h"

/*
 * This class contains the information of a triangle .
 */
class Triangle
{
public:
    int v1, v2, v3;  // vertex index
    int rf;  // material index
    int nn;  // normal index
    precision::value_type A;  // face area

	Triangle();
};

#endif // TRIANGLE_H
