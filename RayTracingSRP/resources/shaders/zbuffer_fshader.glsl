#version 330

/***********************************************************************
 +
 * Project: RayTracingSRP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

layout (location = 0)out vec4 gAlbedo;
layout (location = 1) out vec3 gNormal;

smooth in vec4 worldNormal;
smooth in vec4 worldVertex;

in float fragmentPD;
in float fragmentPS;

uniform vec3 lightDirection;

uniform vec3 worldCamPos;
uniform mat4 model;

uniform int debugMode;

uniform vec3 diffuse;

void main (void) {

	float ps=fragmentPS;
	float pd=fragmentPD;
	vec3 N = normalize(worldNormal.xyz);
	vec3 L = normalize(lightDirection.xyz);
	float costh = dot(N, L);
	if(costh>0){
		N=-N;
		costh=-costh;
	}

	//Compute SRP force on this point.
	float Apix = 1.0f;
	float Apix2 = 1.0f;
	float FSRP0 = -Apix*( (1. - ps)*L.x + 2.*( ps*costh + pd/3. )*N.x );
	float FSRP1 = -Apix*( (1. - ps)*L.y + 2.*( ps*costh + pd/3. )*N.y );
	float FSRP2 = -Apix*( (1. - ps)*L.z + 2.*( ps*costh + pd/3. )*N.z );


	vec3 normalizedForce = 0.125f*vec3(FSRP0+4,FSRP1+4,FSRP2+4);
	gAlbedo = vec4(Apix2*normalizedForce,1.0f);

	gNormal = 0.5f*(N+vec3(1.0f,1.0f,1.0f));

	if(debugMode == 2){ //Show face normals
		N=normalize(worldNormal.xyz);
		if(N.x <0)
			gAlbedo = vec4(1,0,0,1);
		else if(N.x >0)
			gAlbedo = vec4(1,1,0,1);
		else if(N.y <0)
			gAlbedo = vec4(0,1,0,1);
		else if(N.y >0)
			gAlbedo = vec4(0,1,1,1);
		else if(N.z <0)
			gAlbedo = vec4(0,0,1,1);
		else if(N.z >0)
			gAlbedo = vec4(1,0,1,1);

	}
	else if(debugMode == 3){ //Debug light direction
		gAlbedo = vec4(L,1);
	}

	//Drawing the light axes as yellow.
	if(length(worldNormal.xyz)<0.1f)
		gAlbedo = vec4(1.0f,1.0f,0.0f,1.0f);

}
