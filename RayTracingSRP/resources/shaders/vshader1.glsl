#version 330

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 normal;
layout (location = 2) in float pd;
layout (location = 3) in float ps;
layout (location = 4) in float refIdx;
layout (location = 5) in int reflectiveness;

smooth out vec4 worldVertex;
smooth out vec4 worldNormal;

out float fragmentPS;
out float fragmentPD;
out float fragmentRefIdx;
flat out int fragmentReflectiveness;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;


void main()
{
    gl_Position = projection*view*model*vertex;
    gl_Position = gl_Position/gl_Position.w;

    worldVertex = vertex;
    vec3 newNormal = normalMatrix*normal.xyz;
    vec4 newNormal2 = vec4(newNormal,0);
    worldNormal = normal;

    fragmentPD=pd;
    fragmentPS=ps;
    fragmentRefIdx=refIdx;
    fragmentReflectiveness=reflectiveness;


}
