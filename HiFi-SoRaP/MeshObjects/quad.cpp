#include "quad.h"

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

Quad::Quad()
{
	mesh = new TriangleMesh();
	mesh->replicatedVertices.clear();
	mesh->replicatedNormals.clear();
	// first triangle
	mesh->replicatedVertices.push_back(vector4(-1.0L,1.0L,0.0L,1.0L)); // Top left
	mesh->replicatedVertices.push_back(vector4(-1.0L,-1.0L,0.0L,1.0L)); // Bottom left
	mesh->replicatedVertices.push_back(vector4(1.0L,1.0L,0.0L,1.0L)); // Top right
	// second triangle
	mesh->replicatedVertices.push_back(vector4(1.0L,1.0L,0.0L,1.0L)); // Top right
	mesh->replicatedVertices.push_back(vector4(-1.0L,-1.0L,0.0L,1.0L)); // Bottom left
	mesh->replicatedVertices.push_back(vector4(1.0L,-1.0L,0.0L,1.0L)); // Bottom right

	const vector4 normal(0.L,0.L,1.L,0.L);
	for(uint i=0; i< mesh->replicatedVertices.size(); ++i)
		mesh->replicatedNormals.push_back(normal);
}

void Quad::initializeBuffers()
{
	uint size = mesh->replicatedVertices.size();

	glGenVertexArrays( 1, &vao );

	//  vertex buffer object (VBO)
	glGenBuffers( 1, &buffer );

	glBindBuffer( GL_ARRAY_BUFFER, buffer );

	glBufferData( GL_ARRAY_BUFFER, 2*sizeof(vector4)*size , NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vector4)*size, mesh->replicatedVertices.data() );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vector4)*size, sizeof(vector4)*size, mesh->replicatedNormals.data() );

	// set up vertex arrays
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,  0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0,  (void*)(sizeof(vector4)*size));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER,0);
}


