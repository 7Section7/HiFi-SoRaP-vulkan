#ifndef QUAD_H
#define QUAD_H

#include "object.h"

/*
 * This class is a mesh with 2 triangles.
 */
class Quad : public Object
{
public:
    Quad();

    void initializeBuffers();
};

#endif // QUAD_H
