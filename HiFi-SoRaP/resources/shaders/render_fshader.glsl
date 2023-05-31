#version 330

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

uniform sampler2D gAlbedo;
uniform sampler2D gNormal;

smooth in vec4 worldNormal;
smooth in vec4 worldVertex;

out vec4 fragColor;

in float fragmentPD;
in float fragmentPS;

uniform vec3 lightDirection;

uniform vec3 worldCamPos;
uniform mat4 model;

uniform int debugMode;

uniform vec3 diffuse;


//This shader is used for rendering textures.
void main (void)
{
/*
	float ps=fragmentPS;
	float pd=fragmentPD;
	vec3 N = normalize(worldNormal.xyz);
	vec3 L = normalize(lightDirection.xyz);
	float costh = dot(N, L);
	if(costh>0){
		N=-N;
		costh=-costh;
	}

	fragColor =vec4(0.5f*vec3(worldVertex.x+1,worldVertex.y+1,worldVertex.z+1),1);
*/
	vec2 texCoords = 0.5f*(vec2(worldVertex.x+1,worldVertex.y+1));
	vec3 result = texture(gAlbedo,texCoords).xyz;
	vec3 normal = texture(gNormal,texCoords).xyz;

	fragColor = vec4(result,1.0f);
	if(debugMode==2) fragColor = vec4(normal,1.0f);

}
