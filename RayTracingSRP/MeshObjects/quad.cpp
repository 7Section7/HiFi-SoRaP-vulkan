#include "quad.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

Quad::Quad()
{
	mesh = new TriangleMesh();
	mesh->replicatedVertices.clear();
	mesh->replicatedVertices.push_back(vec4(-1.0f,1.0f,0.0f,1.0f));
	mesh->replicatedVertices.push_back(vec4(-1.0f,-1.0f,0.0f,1.0f));
	mesh->replicatedVertices.push_back(vec4(1.0f,1.0f,0.0f,1.0f));

	mesh->replicatedVertices.push_back(vec4(1.0f,1.0f,0.0f,1.0f));
	mesh->replicatedVertices.push_back(vec4(-1.0f,-1.0f,0.0f,1.0f));
	mesh->replicatedVertices.push_back(vec4(1.0f,-1.0f,0.0f,1.0f));
}

void Quad::initializeBuffers()
{
	uint size = mesh->replicatedVertices.size();

	glGenVertexArrays( 1, &vao );

	//  vertex buffer object (VBO)
	glGenBuffers( 1, &buffer );

	glBindBuffer( GL_ARRAY_BUFFER, buffer );

	glBufferData( GL_ARRAY_BUFFER, sizeof(vec4)*size , NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vec4)*size, mesh->replicatedVertices.data() ); //mesh->replicatedVertices.data()

	// set up vertex arrays
	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,  0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER,0);
}


