#version 330

/***********************************************************************
 +
 * Project: HiFi-SoRaP
 * Created by: Leandro Zardaín Rodríguez (leandrozardain@gmail.com)
 * Created on: 30 Nov 2021
 *
 ***********************************************************************/

smooth in vec4 worldNormal;
smooth in vec4 worldVertex;

in float fragmentPD;
in float fragmentPS;

uniform vec3 lightDirection;

uniform vec3 worldCamPos;
uniform mat4 model;
out vec4 fragColor;

uniform vec3 diffuse;

/*
  This computes which color assign to the pixel. It is based on the formula of blinn-phong.
  */
void main (void) {
	vec3 V = normalize(worldCamPos-worldVertex.xyz);

	vec3 lightPos = lightDirection;
	vec3 L = normalize(worldVertex.xyz-lightPos);
	vec3 E = normalize(-V);
	vec3 N = normalize(worldNormal.xyz);
	vec3 R = normalize(-reflect(L, N));
	vec3 H = normalize(L+V);
	vec3 Iamb = vec3(1.0, 1.0, 1.0);

	float scale;

	//When we have several materials in a object (in order to differentiate each material).
	if(fragmentPS <0.003){
		scale = 0.25f;
	}else if(fragmentPS < 0.03){
		scale = 0.40f;
	}else if(fragmentPS < 0.07 && fragmentPD < 0.005){
		scale = 0.55f;
	}else if(fragmentPS < 0.07){
		scale = 0.70f;
	}else if(fragmentPS < 0.3){
		scale = 0.85f;
	}else{
		scale= 1.f;
	}

	vec3 newDiffuse = scale * diffuse;

	if(length(worldNormal.xyz)<0.1f) //line objects
		newDiffuse=diffuse;

	vec3 Idiff = diffuse * vec3(max(dot(N, L),0.0));

	vec3 Ispec = vec3(1.0, 1.0, 1.0) * pow(max(dot(R,E), 0.0), 2.2);
	Ispec = clamp(Ispec, 0.0, 1.0);

	fragColor = vec4(newDiffuse*0.7+Idiff*0.3,1.0);

}
