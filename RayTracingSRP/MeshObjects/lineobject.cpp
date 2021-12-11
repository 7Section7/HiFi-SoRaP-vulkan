#include "lineobject.h"

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

LineObject::LineObject()
{
    diffuseColor = QVector3D(1,1,0);
}

void LineObject::initializeBuffers()
{
    uint size = mesh->replicatedVertices.size();

    glGenVertexArrays( 1, &vao );

    // (VBO)
    glGenBuffers( 1, &buffer );

    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    glBufferData( GL_ARRAY_BUFFER, sizeof(vec4)*size , NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vec4)*size, mesh->replicatedVertices.data() );

    // set up vertex arrays
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,  0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void LineObject::draw(std::unique_ptr<QGLShaderProgram> &program)
{
    if(mesh){

        GLuint diffuseLocation=program->uniformLocation("diffuse");

        glUniform3f(diffuseLocation,diffuseColor.x(),diffuseColor.y(),diffuseColor.z());

        glBindVertexArray( vao );

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays( GL_LINES, 0, mesh->replicatedVertices.size());

        glBindVertexArray(0);
    }
}
